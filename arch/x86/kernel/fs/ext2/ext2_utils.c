#include "ext2.h"

#include "ext2_utils.h"
#include "ext2_struct.h"

#include "printk.h"

#include <string.h>

PathParts_ext format_folder_path_ext(const char *in)
{
	PathParts_ext result = {0};

	while (*in == '/')
		in++;

	while (*in && result.count < MAX_PARTS && *in != '\0')
	{
		const char *end = in;
		while (*end && *end != '/' && *end != '\0')
			end++;

		int len = end - in;
		if (len > 0)
		{
			char name[256] = {0};
			strncpy(name, in, len);

			result.parts[result.count].name = kmalloc(len + 1);
			memcpy(result.parts[result.count].name, name, len);
			result.parts[result.count].name[len] = '\0';

			result.count++;
		}

		in = end;
		while (*in == '/' && *in != '\0')
			in++;
	}
	return result;
}

int IS_DIR(uint16_t mode)
{
	return (mode & 0xF000) == 0x4000;
}

// read and write
int ext2_read_block(EXT2_FS *fs, uint32_t block_number, void *buf)
{
	uint32_t sectors_per_block = fs->block_size / 512;
	uint32_t lba = fs->first_lba + block_number * sectors_per_block;
	// printk("lba: %d sectors_per_block: %d block_size: %d\n", lba, sectors_per_block, fs->block_size);
	for (uint32_t i = 0; i < sectors_per_block; i++)
	{
		if (fs->read_sector(fs->device, lba + i, ((uint8_t *)buf) + i * 512) < 0)
		{
			return -1;
		}
	}
	return 0;
}
int ext2_write_block(EXT2_FS *fs, uint32_t block_number, void *buf)
{
	uint32_t sectors_per_block = fs->block_size / 512;
	uint32_t lba = fs->first_lba + block_number * sectors_per_block;
	printk("lba: %d sectors_per_block: %d block_size: %d\n", lba, sectors_per_block, fs->block_size);
	for (uint32_t i = 0; i < sectors_per_block; i++)
	{
		if (fs->write_sector(fs->device, lba + i, ((uint8_t *)buf) + i * 512) < 0)
		{
			return -1;
		}
	}
	return 0;
}

// directory
uint32_t ext2_find_dir_entry(EXT2_FS *fs, uint32_t inode, const char *name)
{
	Ext2Inode dir_inode;
	ext2_read_inode(fs, inode, &dir_inode);

	uint8_t *block_buf = kmalloc(fs->block_size);

	for (int i = 0; i < 12 && dir_inode.block[i]; i++) // цикл по прямих блоках
	{
		ext2_read_block(fs, dir_inode.block[i], block_buf);

		uint32_t offset = 0;
		while (offset < fs->block_size)
		{
			Ext2DirEntry *entry = (Ext2DirEntry *)(block_buf + offset);
			if (entry->inode == 0)
				break;

			if (entry->name_len == strlen(name) &&
			    memcmp(entry->name, name, entry->name_len) == 0)
			{
				printk("Found: %s\n", entry->name);
				uint32_t found_inode = entry->inode;
				kfree(block_buf);
				return found_inode;
			}
			printk("%s ", entry->name);

			offset += entry->rec_len;
			if (entry->rec_len == 0)
				break;
		}
	}

	kfree(block_buf);
	return 0;
}

uint32_t ext2_parse_path(EXT2_FS *fs, uint32_t inode, const char *path)
{
	PathParts_ext parts = format_folder_path_ext(path);
	printk("parsing path: %s, inode: %d\n", path, inode);
	for (int i = 0; i < parts.count; i++)
	{
		inode = ext2_find_dir_entry(fs, inode, parts.parts[i].name);
		printk("inode: %d, name: %s\n", inode, parts.parts[i].name);
		if (inode == 0)
		{
			printk("File not found(when parsing): %s\n", parts.parts[i].name);
			return 0;
		}
	}
	return inode;
}

// block free
void ext2_free_block(EXT2_FS *fs, uint32_t block_number)
{
	printk("free block: %u\n", block_number);

	uint32_t rel_block = block_number - fs->first_data_block;
	uint32_t group = rel_block / fs->blocks_per_group;
	uint32_t index = rel_block % fs->blocks_per_group;

	uint32_t bitmap_block = fs->groups[group].block_bitmap;

	uint8_t *bitmap_buf = kmalloc(fs->block_size);
	if (!bitmap_buf)
		return;

	// Зчитуємо bitmap
	if (ext2_read_block(fs, bitmap_block, bitmap_buf) != 0)
	{
		kfree(bitmap_buf);
		return;
	}

	uint8_t mask = 1 << (index % 8);
	uint8_t *byte = &bitmap_buf[index / 8];

	if (!(*byte & mask))
	{
		printk("Warning: double free of block %u\n", block_number);
		kfree(bitmap_buf);
		return;
	}

	*byte &= ~mask;

	ext2_write_block(fs, bitmap_block, bitmap_buf);
	kfree(bitmap_buf);

	// Оновлюємо лічильники
	fs->groups[group].free_blocks_count++;
	fs->superblock->s_free_blocks_count++;
}

void ext2_free_inode(EXT2_FS *fs, uint32_t inode_number)
{
	Ext2Inode inode;
	if (ext2_read_inode(fs, inode_number, &inode) != 0)
		return;

	for (int i = 0; i < 12; i++)
	{
		if (inode.block[i])
		{
			ext2_free_block(fs, inode.block[i]);
			inode.block[i] = 0;
		}
	}
	inode.dtime = inode.ctime;
	ext2_write_inode(fs, inode_number, &inode);

	uint32_t group = (inode_number - 1) / fs->inodes_per_group;
	uint32_t index = (inode_number - 1) % fs->inodes_per_group;

	uint32_t bitmap_block = fs->groups[group].inode_bitmap;

	uint8_t *bitmap_buf = kmalloc(fs->block_size);
	ext2_read_block(fs, bitmap_block, bitmap_buf);

	bitmap_buf[index / 8] &= ~(1 << (index % 8));

	ext2_write_block(fs, bitmap_block, bitmap_buf);
	kfree(bitmap_buf);

	fs->groups[group].free_inodes_count += 1;
	fs->superblock->s_free_inodes_count += 1;

	printk("free inode: %d\n", fs->superblock->s_free_inodes_count);
	printk("free blocks: %d\n", fs->superblock->s_free_blocks_count);

	ext2_write_group_desc(fs, group, &fs->groups[group]);
	ext2_write_superblock(fs);
}
