
#include "printk.h"

#include <mm/kmalloc.h>

#include <fs/gpt/gpt_struct.h>

#include <fs/ata/ata.h>
#include <fs/vfs/vfs.h>
#include <fs/vfs/vfs_standart_struct.h>

#include "ext2.h"
#include "ext2_struct.h"
#include "ext2_utils.h"

#include <string.h>
#include <errno.h>

int ext2_read_group_desc(EXT2_FS *fs)
{
	uint32_t desc_block = (fs->block_size == 1024) ? 2 : 1;

	uint32_t groups_count = (fs->blocks_count + fs->blocks_per_group - 1) / fs->blocks_per_group;
	uint32_t desc_size = sizeof(Ext2GroupDesc) * groups_count;

	uint32_t blocks_needed = (desc_size + fs->block_size - 1) / fs->block_size;

	uint8_t *buf = kmalloc(blocks_needed * fs->block_size);
	if (!buf)
	{
		printk("EXT2: failed to alloc group desc buffer\n");
		return -1;
	}

	for (uint32_t i = 0; i < blocks_needed; i++)
	{
		ext2_read_block(fs, desc_block + i, buf + i * fs->block_size);
	}
	if (fs->groups)
	{
		kfree(fs->groups);
	}
	fs->groups = kmalloc(sizeof(Ext2GroupDesc) * groups_count);
	if (!fs->groups)
	{
		kfree(buf);
		printk("EXT2: failed to alloc fs->groups\n");
		return -1;
	}

	memcpy(fs->groups, buf, desc_size);
	kfree(buf);

	printk("EXT2: read %u group descriptors\n", groups_count);
	for (uint32_t i = 0; i < groups_count; i++)
		printk("Group %u: inode_table=%u block_bitmap=%u inode_bitmap=%u; ",
		       i, fs->groups[i].inode_table,
		       fs->groups[i].block_bitmap,
		       fs->groups[i].inode_bitmap);

	return 0;
}

int ext2_write_group_desc(EXT2_FS *fs, uint32_t group, Ext2GroupDesc *desc)
{

	uint32_t block_group_table = (fs->block_size == 1024) ? 2 : 1;
	uint32_t desc_per_block = fs->block_size / sizeof(Ext2GroupDesc);

	uint32_t block_index = group / desc_per_block;
	uint32_t offset = group % desc_per_block;

	uint8_t *buf = kmalloc(fs->block_size);
	if (!buf)
		return -1;

	if (ext2_read_block(fs, block_group_table + block_index, buf) != 0)
	{
		kfree(buf);
		return -1;
	}

	memcpy(buf + offset * sizeof(Ext2GroupDesc), desc, sizeof(Ext2GroupDesc));

	int res = ext2_write_block(fs, block_group_table + block_index, buf);
	// update group descriptor

	ext2_read_group_desc(fs);

	// print free blocks
	uint32_t free_blocks = 0;
	for (int i = 0; i < 5; i++)
	{

		free_blocks += fs->groups[group].free_blocks_count;
		printk("%u ", fs->groups[group].free_blocks_count);
		printk("%u ", fs->groups[group].free_inodes_count);
	}
	printk("Group %u: %u free blocks\n", group, free_blocks);

	kfree(buf);
	return res;
}

int ext2_read_superblock(EXT2_FS *fs)
{
	uint8_t buf[1024];
	printk("Reading superblock\n");
	uint32_t superblock_lba = fs->first_lba + 2; // 1024 / 512 = 2 секторa
	printk("Superblock LBA: %u\n", superblock_lba);
	if (!fs->read_sector)
	{
		printk("EXT2: read_sector is NULL\n");
		return -1;
	}
	for (int i = 0; i < 2; i++)
		fs->read_sector(fs->device, superblock_lba + i, buf + i * 512);
	Ext2Superblock *sb = (Ext2Superblock *)buf;

	if (sb->s_magic != 0xEF53)
	{
		printk("EXT2: invalid magic 0x%x (expected 0xEF53)\n", sb->s_magic);
		printk("first_lba=%u superblock_lba=%u\n", fs->first_lba, superblock_lba);
		return -1;
	}
	printk("EXT2: magic ok 0x%x\n", sb->s_magic);
	fs->inodes_count = sb->s_inodes_count;
	fs->blocks_count = sb->s_blocks_count;
	fs->first_data_block = sb->s_first_data_block;
	fs->log_block_size = sb->s_log_block_size;
	fs->blocks_per_group = sb->s_blocks_per_group;
	fs->inodes_per_group = sb->s_inodes_per_group;
	fs->block_size = 1024 << sb->s_log_block_size;
	fs->magic = sb->s_magic;

	fs->inode_size = (sb->s_inode_size && sb->s_inode_size >= 128) ? sb->s_inode_size : 128;
	if (fs->superblock)
	{
		kfree(fs->superblock);
	}

	fs->superblock = kmalloc(sizeof(Ext2Superblock));
	memcpy(fs->superblock, sb, sizeof(Ext2Superblock));

	printk("EXT2: magic ok 0x%x, inodes_count=%u, blocks_count=%u, block_size=%u, free_blocks_count=%u, free_inodes_count=%u\n", sb->s_magic, sb->s_inodes_count, sb->s_blocks_count, 1024 << sb->s_log_block_size, sb->s_free_blocks_count, sb->s_free_inodes_count);

	return 0;
}

int ext2_write_superblock(EXT2_FS *fs)
{
	uint32_t superblock_lba = fs->first_lba + 2;
	uint8_t *buf = kmalloc(fs->block_size);
	if (!buf)
		return -1;

	memset(buf, 0, fs->block_size);
	memcpy(buf, fs->superblock, sizeof(Ext2Superblock));

	uint32_t sectors_per_block = fs->block_size / 512;

	for (uint32_t i = 0; i < sectors_per_block; i++)
		fs->write_sector(fs->device, superblock_lba + i, buf + i * 512);

	// update superblock

	ext2_read_superblock(fs);

	kfree(buf);
	return 0;
}

Directory ext2_list_dir(EXT2_FS *fs, Ext2Inode *dir_inode)
{
	if (!IS_DIR(dir_inode->mode))
	{
		printk("Not a directory\n");
		return Directory_init((Directory){.entries = NULL, .count = 0});
	}

	uint8_t *block_buf = kmalloc(fs->block_size);

	printk("Listing directory (size=%u bytes)\n", dir_inode->size);

	Directory dir = Directory_init((Directory){.entries = kmalloc(1024), .count = 0});

	for (int i = 0; i < 12 && dir_inode->block[i]; i++) // тільки прямі блоки для простої версії
	{
		ext2_read_block(fs, dir_inode->block[i], block_buf);
		uint32_t offset = 0;
		while (offset < fs->block_size)
		{
			Ext2DirEntry *entry = (Ext2DirEntry *)(block_buf + offset);

			if (entry->inode == 0)
				break;
			dir.entries[dir.count].name = kmalloc(entry->name_len + 1);
			if (!dir.entries[dir.count].name)
			{
				printk("failed");
			}
			memcpy(dir.entries[dir.count].name, entry->name, entry->name_len);
			dir.entries[dir.count]
			    .name[entry->name_len] = '\0';
			dir.entries[dir.count].is_dir = entry->file_type == 0x10;
			dir.entries[dir.count].cluster = entry->inode;
			dir.count++;

			// printk(" - %s (inode=%u, type=%u)\n", name, entry->inode, entry->file_type);

			offset += entry->rec_len;
			if (entry->rec_len == 0)
			{
				break;
			}
		}
	}
	kfree(block_buf);
	return dir;
}
uint32_t ext2_allocate_inode(EXT2_FS *fs, uint32_t group)
{
	uint8_t *bitmap = kmalloc(fs->block_size);
	if (!bitmap)
		return 0;
	if (ext2_read_block(fs, fs->groups[group].inode_bitmap, bitmap) != 0)
	{
		kfree(bitmap);
		return 0;
	}

	for (uint32_t i = 0; i < fs->inodes_per_group; i++)
	{
		uint32_t byte = i / 8;
		uint8_t bit = 1 << (i % 8);
		if (!(bitmap[byte] & bit))
		{
			bitmap[byte] |= bit; // помічаємо inode як зайнятий

			if (ext2_write_block(fs, fs->groups[group].inode_bitmap, bitmap) != 0)
			{
				kfree(bitmap);
				return 0;
			}

			/* Оновлюємо лічильники */
			if (fs->groups[group].free_inodes_count > 0)
				fs->groups[group].free_inodes_count--;
			if (fs->superblock->s_free_inodes_count > 0)
				fs->superblock->s_free_inodes_count--;
			printk("\nallocating inode %d\n", i + 1 + group * fs->inodes_per_group);
			/* Записуємо назад group descriptor і суперблок */
			ext2_write_group_desc(fs, group, &fs->groups[group]);
			ext2_write_superblock(fs);

			kfree(bitmap);
			return i + 1 + group * fs->inodes_per_group; // глобальний номер inode
		}
	}

	kfree(bitmap);
	return 0; // немає вільних inode
}

uint32_t ext2_allocate_block(EXT2_FS *fs, uint32_t group)
{
	uint8_t *bitmap = kmalloc(fs->block_size);
	if (!bitmap)
		return 0;
	if (ext2_read_block(fs, fs->groups[group].block_bitmap, bitmap) != 0)
	{
		kfree(bitmap);
		return 0;
	}

	for (uint32_t i = 0; i < fs->blocks_per_group; i++)
	{
		uint32_t byte = i / 8;
		uint8_t bit = 1 << (i % 8);
		if (!(bitmap[byte] & bit))
		{
			bitmap[byte] |= bit;
			if (ext2_write_block(fs, fs->groups[group].block_bitmap, bitmap) != 0)
			{
				kfree(bitmap);
				return 0;
			}

			/* Оновлюємо лічильники */
			if (fs->groups[group].free_blocks_count > 0)
				fs->groups[group].free_blocks_count--;
			if (fs->superblock->s_free_blocks_count > 0)
				fs->superblock->s_free_blocks_count--;

			/* Записуємо назад group descriptor і суперблок */
			ext2_write_group_desc(fs, group, &fs->groups[group]);
			ext2_write_superblock(fs);

			kfree(bitmap);

			/* повертаємо глобальний номер блоку в простих блоках FS */
			uint32_t global_block = fs->first_data_block + i + group * fs->blocks_per_group;
			return global_block;
		}
	}

	kfree(bitmap);
	return 0;
}

int ext2_write_inode(EXT2_FS *fs, uint32_t inode_num, Ext2Inode *inode)
{
	uint32_t group = (inode_num - 1) / fs->inodes_per_group;
	uint32_t index = (inode_num - 1) % fs->inodes_per_group;
	uint32_t table_block = fs->groups[group].inode_table;

	uint32_t inode_size = fs->inode_size;
	uint32_t inodes_per_block = fs->block_size / inode_size;
	uint32_t block_offset = index / inodes_per_block;
	uint32_t offset = index % inodes_per_block;

	uint8_t *buf = kmalloc(fs->block_size);
	if (!buf)
		return -1;

	ext2_read_block(fs, table_block + block_offset, buf);

	memcpy(buf + offset * inode_size, inode, sizeof(Ext2Inode));

	ext2_write_block(fs, table_block + block_offset, buf);

#ifdef EXT2_DEBUG
	uint8_t *check = kmalloc(fs->block_size);
	ext2_read_block(fs, table_block + block_offset, check);
	printk("verify inode:\n");
	for (int i = 0; i < 128; i++)
		printk("%u ", check[offset * inode_size + i]);
	kfree(check);
#endif

	kfree(buf);
	return 0;
}
int ext2_read_inode(EXT2_FS *fs, uint32_t inode_number, Ext2Inode *out_inode)
{
	uint32_t group = (inode_number - 1) / fs->inodes_per_group;
	uint32_t index = (inode_number - 1) % fs->inodes_per_group;
	uint32_t table_block = fs->groups[group].inode_table;

	uint32_t inode_size = fs->inode_size;
	uint32_t inodes_per_block = fs->block_size / inode_size;
	uint32_t block_offset = index / inodes_per_block;
	uint32_t offset = index % inodes_per_block;

	printk("inode_number=%u group=%u index=%u table_block=%u block_offset=%u offset=%u\n", inode_number, group, index, table_block, block_offset, offset);

	uint8_t *block_buf = kmalloc(fs->block_size);
	if (!block_buf)
		return -1;

	ext2_read_block(fs, table_block + block_offset, block_buf);

	memcpy(out_inode, block_buf + offset * inode_size, sizeof(Ext2Inode));
	kfree(block_buf);
	return 0;
}

int ext2_add_dir_entry(EXT2_FS *fs, uint32_t dir_inode_num, const char *name, uint32_t inode_num, uint8_t file_type)
{
	Ext2Inode dir_inode;
	if (ext2_read_inode(fs, dir_inode_num, &dir_inode) != 0)
		return -1;

	uint8_t *block = kmalloc(fs->block_size);
	if (!block)
		return -1;

	if (ext2_read_block(fs, dir_inode.block[0], block) != 0)
	{
		kfree(block);
		return -1;
	}

	printk("ADD inode=%u name=%s dir_inode=%u \n", inode_num, name, dir_inode_num);
	uint32_t offset = 0;
	while (offset < fs->block_size)
	{
		Ext2DirEntry *entry = (Ext2DirEntry *)(block + offset);
		if (offset + entry->rec_len >= fs->block_size)
		{
			uint16_t actual_len = 8 + ((entry->name_len + 3) & ~3);
			uint16_t new_len = fs->block_size - offset - actual_len;
			if (new_len < (8 + ((strlen(name) + 3) & ~3)))
			{
				// не вистачає місця для нового запису в цьому блоці (хоча такий випадок рідкісний для прямо заповненого блоку)
				kfree(block);
				return -1;
			}

			entry->rec_len = actual_len;

			Ext2DirEntry *new_entry = (Ext2DirEntry *)(block + offset + actual_len);
			new_entry->inode = inode_num;
			new_entry->rec_len = new_len;
			new_entry->name_len = strlen(name);
			new_entry->file_type = file_type;
			memcpy(new_entry->name, name, new_entry->name_len);

			if (ext2_write_block(fs, dir_inode.block[0], block) != 0)
			{
				kfree(block);
				return -1;
			}

			dir_inode.mtime = 0;
			ext2_write_inode(fs, dir_inode_num, &dir_inode);

			kfree(block);
			return 0;
		}
		offset += entry->rec_len;
	}

	kfree(block);
	return -1;
}
int ext2_remove_dir_entry(EXT2_FS *fs, uint32_t dir_inode_num, uint32_t inode_num)
{
	Ext2Inode dir_inode;
	if (ext2_read_inode(fs, dir_inode_num, &dir_inode) != 0)
		return -1;

	uint8_t *block = kmalloc(fs->block_size);
	if (!block)
		return -1;

	for (uint32_t i = 0; i < 15 && dir_inode.block[i]; i++)
	{
		if (ext2_read_block(fs, dir_inode.block[i], block) != 0)
			continue;

		uint32_t offset = 0;
		Ext2DirEntry *prev = NULL;
		uint32_t prev_offset = 0;

		while (offset < fs->block_size)
		{
			Ext2DirEntry *entry = (Ext2DirEntry *)(block + offset);

			if (entry->inode == inode_num)
			{
				if (prev)
				{
					prev->rec_len += entry->rec_len;
				}
				else
				{
					// зсунути всі записи назад
					uint32_t move_size = fs->block_size - (offset + entry->rec_len);
					memmove(entry, (uint8_t *)entry + entry->rec_len, move_size);
				}

				ext2_write_block(fs, dir_inode.block[i], block);

				dir_inode.mtime = dir_inode.ctime = 0; // TODO: set time
				ext2_write_inode(fs, dir_inode_num, &dir_inode);

				// зменшити link_count
				Ext2Inode victim;
				if (ext2_read_inode(fs, inode_num, &victim) == 0)
				{
					if (victim.links_count > 0)
						victim.links_count--;
					ext2_write_inode(fs, inode_num, &victim);
				}

				kfree(block);
				return 0;
			}

			prev_offset = offset;
			prev = entry;
			offset += entry->rec_len;
		}
	}

	kfree(block);
	return -1;
}

uint32_t ext2_create_file(EXT2_FS *fs, uint32_t parent_inode, const char *name)
{
	uint32_t new_inode = ext2_allocate_inode(fs, 0);
	if (!new_inode)
	{
		printk("ext2_create_file: ext2_allocate_inode failed\n");
		return 0;
	}

	uint32_t new_block = ext2_allocate_block(fs, 0);
	if (!new_block)
	{
		printk("ext2_create_file: ext2_allocate_block failed\n");
		// // rollback: звільнити inode (треба реалізувати ext2_free_inode)
		// ext2_free_inode(fs, new_inode);
		return 0;
	}

	Ext2Inode inode;
	memset(&inode, 0, sizeof(Ext2Inode));
	inode.mode = 0x8000 | 0644;
	inode.size = 0;
	inode.blocks = (fs->block_size / 512);
	inode.block[0] = new_block;
	inode.links_count = 1;
	inode.ctime = inode.mtime = 0;

	if (ext2_write_inode(fs, new_inode, &inode) != 0)
	{
		printk("ext2_create_file: ext2_write_inode failed\n");
		ext2_free_block(fs, new_block);
		ext2_free_inode(fs, new_inode);
		return 0;
	}

	if (ext2_add_dir_entry(fs, parent_inode, name, new_inode, 1) != 0)
	{
		printk("ext2_create_file: ext2_add_dir_entry failed\n");
		ext2_free_block(fs, new_block);
		ext2_free_inode(fs, new_inode);
		return 0;
	}

	Ext2Inode parent;
	ext2_read_inode(fs, parent_inode, &parent);
	parent.mtime = 0;
	ext2_write_inode(fs, parent_inode, &parent);

	printk("Created file %s with inode %u\n", name, new_inode);
	return new_inode;
}

uint32_t ext2_create_directory(EXT2_FS *fs, uint32_t parent_inode, const char *name)
{
	uint32_t new_inode = ext2_allocate_inode(fs, 0);
	uint32_t new_block = ext2_allocate_block(fs, 0);

	Ext2Inode inode = {0};
	inode.mode = 0x4000 | 0755; // директорій
	inode.size = 0;
	inode.blocks = 2; // у 512-блоках
	inode.block[0] = new_block;

	ext2_write_inode(fs, new_inode, &inode);
	ext2_add_dir_entry(fs, parent_inode, name, new_inode, 2); // 2 = директорій
	return new_inode;
}
int ext2_read_file(VFS_File *file, uint8_t *buf, uint32_t size)
{
	EXT2_FS *fs = file->node->fs->fs;
	Ext2File *file_data = file->node->fs_node;

	uint32_t inode_number = file_data->inode;
	printk("ext2_read_file: inode_number=%u\n", inode_number);

	Ext2Inode *inode = kmalloc(sizeof(Ext2Inode));
	if (!inode)
	{
		printk("ext2_read_file: failed to allocate inode\n");
		return -ENOMEM;
	}

	if (ext2_read_inode(fs, inode_number, inode) != 0)
	{
		printk("ext2_read_file: failed to read inode, inode_number=%d\n", inode_number);
		kfree(inode);
		return -EIO;
	}

	uint32_t offset = file->pos;
	uint32_t block_size = fs->block_size;

	if (offset >= inode->size)
	{
		printk("ext2_read_file: offset >= inode->size, offset=%u inode->size=%u, inode_number=%u\n", offset, inode->size, inode_number);
		kfree(inode);
		return 0;
	}
	if (offset + size > inode->size)
		size = inode->size - offset;

	uint64_t total_read = 0;
	uint32_t start_block = offset / block_size;
	uint32_t block_offset = offset % block_size;
	uint8_t *tmp = kmalloc(block_size);

	if (!tmp)
	{
		printk("ext2_read_file: failed to allocate buffer\n");
		kfree(inode);
		return -ENOMEM;
	}

	while (total_read < size)
	{
		uint32_t logical_block = start_block + (total_read / block_size);
		uint32_t phys_block = inode->block[logical_block];

		if (phys_block == 0)
			break;

		ext2_read_block(fs, phys_block, tmp);

		uint32_t copy_offset = (total_read == 0) ? block_offset : 0;
		uint32_t copy_len = block_size - copy_offset;
		if (copy_len > (size - total_read))
			copy_len = size - total_read;

		memcpy(buf + total_read, tmp + copy_offset, copy_len);
		total_read += copy_len;
	}

	kfree(tmp);
	kfree(inode);

	file->pos += total_read;
	return total_read;
}

uint32_t allocate_data_block(EXT2_FS *fs, uint32_t logical_block, uint32_t phys_block, uint32_t inode_number, uint32_t *blocks, uint32_t inderect_block)
{
	if (phys_block == 0)
	{
		phys_block = ext2_allocate_block(fs, 0);
		blocks[logical_block] = phys_block;
		// ext2_write_inode(fs, inode_number, inode);
		uint8_t *buf = kmalloc(fs->block_size);
		memset(buf, 0, fs->block_size);
		ext2_write_block(fs, phys_block, buf);
		kfree(buf);
	}

	if (inderect_block)
	{
		ext2_write_block(fs, inderect_block, blocks);
	}

	return phys_block;
}

int ext2_write_file(VFS_File *file, uint8_t *buf, uint32_t size)
{
	EXT2_FS *fs = file->node->fs->fs;
	Ext2File *file_data = file->node->fs_node;
	uint32_t inode_number = file_data->inode;
	Ext2Inode *inode = kmalloc(sizeof(Ext2Inode));
	ext2_read_inode(fs, inode_number, inode);

	uint32_t offset = file->pos;
	uint32_t block_size = 1024;
	uint32_t to_write = offset + size;

	uint64_t total_write = 0;
	uint8_t *tmp = kmalloc(block_size);
	printk("offset=%u to_write=%u cluster_size=%u inode_number=%u sizeof_inode=%u\n", offset, to_write, inode->size, inode_number, sizeof(Ext2Inode));
	while (offset < to_write)
	{
		uint32_t logical_block = offset / block_size;

		uint32_t phys_block = 0;
		if (logical_block >= 12)
		{
			int pointers_per_block = block_size / 4;
			int offset_in_block = logical_block - 12;
			if (offset_in_block < pointers_per_block)
			{
				phys_block = inode->block[12];

				phys_block = allocate_data_block(fs, 12, phys_block, inode_number, inode->block, 0);

				uint32_t *tmp_direct_block = kmalloc(block_size);
				ext2_read_block(fs, phys_block, tmp_direct_block);
				phys_block = tmp_direct_block[offset_in_block];
				phys_block = allocate_data_block(fs, offset_in_block, phys_block, inode_number, tmp_direct_block, inode->block[12]);
				kfree(tmp_direct_block);
			}
		}
		else
		{
			phys_block = inode->block[logical_block];
			phys_block = allocate_data_block(fs, logical_block, phys_block, inode_number, inode->block, 0);
		}
		ext2_read_block(fs, phys_block, tmp);

		uint32_t block_offset = offset % block_size;
		uint32_t copy_offset = block_offset;

		uint32_t copy_len = block_size - copy_offset;
		if (copy_len > (size - total_write))
			copy_len = size - total_write;

		memcpy(tmp + copy_offset, buf + total_write, copy_len);
		ext2_write_block(fs, phys_block, tmp);

		total_write += copy_len;
		offset += copy_len;
	}
	inode->size = inode->size > file->pos + total_write ? inode->size : file->pos + total_write;

	if (inode->size < file->pos + total_write)
	{
		printk("inode->size < file->pos + total_write\n");
		inode->size = file->pos + total_write;
	}

	ext2_write_inode(fs, inode_number, inode);

	file->pos += total_write;
	file->node->size = inode->size;
	printk("wrote %u bytes, size = %u\n", total_write, inode->size);
	kfree(inode);
	kfree(tmp);

	return 0;
}

int ext2_init(EXT2_FS *fs)
{
	printk("Initializing EXT2\n");
	printk("Reading superblock\n");
	if (ext2_read_superblock(fs) < 0)
		return -1;
	printk("Reading group descriptors\n");
	if (ext2_read_group_desc(fs) < 0)
		return -1;
	Ext2Inode root_inode;
	printk("Reading root inode\n");
	ext2_read_inode(fs, 2, &root_inode);

	printk("Root inode size = %u bytes\n", root_inode.size);
	printk("Root inode first block = %u\n", root_inode.block[0]);
	// display all info
	printk("inode blocks count = %u\n", root_inode.blocks);
	ext2_list_dir(fs, &root_inode);
	return 0;
}

// void ext2_init(gpt_partition_t part)
// {
// 	printk("Initializing EXT2 on partition starting at LBA %llu\n", part.first_lba);

// 	VFS_Device *device = part.device;

// 	uint8_t *buf = kmalloc(1024 * 2);

// 	device->read(device->device, part.first_lba, buf);
// 	device->read(device->device, part.first_lba + 1, buf + 512);
// 	device->read(device->device, part.first_lba + 2, buf + 1024);

// 	Ext2Superblock *superblock = (Ext2Superblock *)(buf + 1024);

// 	if (superblock->s_magic != 0xEF53)
// 	{
// 		printk("Superblock magic number: 0x%x (invalid)\n", superblock->s_magic);
// 		return;
// 	}

// 	printk("EXT2 Superblock OK (magic 0x%x)\n", superblock->s_magic);
// 	printk("Block size: %u\n", 1024 << superblock->s_log_block_size);
// 	printk("Inodes count: %u\n", superblock->s_inodes_count);
// 	printk("Blocks count: %u\n", superblock->s_blocks_count);
// 	printk("Blocks per group: %u\n", superblock->s_blocks_per_group);
// 	printk("Inodes per group: %u\n", superblock->s_inodes_per_group);
// 	printk("First data block: %u\n", superblock->s_first_data_block);
// 	printk("Volume name: %.16s\n", superblock->s_volume_name);

// 	EXT2_FS *fs = kmalloc(sizeof(EXT2_FS));
// 	if (!fs)
// 	{
// 		printk("Failed to allocate EXT2_FS\n");
// 		return;
// 	}

// 	fs->device = device->device;
// 	fs->start_lba = part.first_lba;
// 	fs->read_sector = device->read;
// 	fs->write_sector = device->write;

// 	fs->block_size = 1024 << superblock->s_log_block_size;
// 	fs->blocks_per_group = superblock->s_blocks_per_group;
// 	fs->inodes_per_group = superblock->s_inodes_per_group;
// 	fs->first_data_block = superblock->s_first_data_block;
// 	fs->blocks_count = superblock->s_blocks_count;
// 	fs->inodes_count = superblock->s_inodes_count;

// 	uint32_t group_desc_lba = (fs->first_data_block + 1) * (fs->block_size / 512);

// 	uint8_t *buffer = kmalloc(fs->block_size);

// 	device->read(device->device, group_desc_lba, buffer);

// 	for (int i = 0; i < fs->inodes_per_group; i++)
// 	{
// 		Ext2Inode *inode = (Ext2Inode *)(buffer + i * sizeof(Ext2Inode));

// 		if (inode->i_mode == 0)
// 		{
// 			continue;
// 		}

// 		fs->inodes[i] = inode;
// 	}

// 	kfree(buf);
// }
