#include "vfs.h"
#include "vfs_standart_struct.h"
#include "printk.h"

#include <fs/ext2/ext2.h>
#include <fs/ext2/ext2_struct.h>
#include <fs/ext2/ext2_utils.h>

#include <mm/kmalloc.h>

#include <string.h>
#include <errno.h>

#define EXT2_ATTR_READ_ONLY 0x01
#define EXT2_ATTR_HIDDEN 0x02
#define EXT2_ATTR_SYSTEM 0x04
#define EXT2_ATTR_VOLUME_ID 0x08
#define EXT2_ATTR_DIRECTORY 0x10
#define EXT2_ATTR_ARCHIVE 0x20

#define EXT2_BLOCK_SIZE 1024

static void build_parent_path_from_parts(char *out, size_t out_len, PathParts_ext *parts)
{
	if (!out || out_len == 0)
		return;
	out[0] = '\0';

	if (parts->count <= 1)
	{
		// файл у корені
		strncpy(out, "/", out_len - 1);
		out[out_len - 1] = '\0';
		return;
	}

	size_t pos = 0;
	for (int i = 0; i < parts->count - 1; i++)
	{
		const char *pname = parts->parts[i].name;
		size_t needed = strlen(pname) + (i == 0 && pname[0] == '/' ? 0 : 1); // slash if not leading
		if (pos + needed + 1 >= out_len)
			break; // захист від переповнення

		if (pos == 0)
		{
			// перший компонент — просто копіюємо (можливо починається без '/')
			if (pname[0] != '/')
			{
				out[pos++] = '/';
			}
		}
		size_t n = strlen(pname);
		if (pos + n < out_len)
		{
			memcpy(out + pos, pname, n);
			pos += n;
		}
		if (i != parts->count - 2 && pos + 1 < out_len)
		{
			out[pos++] = '/';
		}
	}
	out[pos] = '\0';
}

static const char *get_leaf_name(PathParts_ext *parts)
{
	if (!parts || parts->count == 0)
		return NULL;
	return parts->parts[parts->count - 1].name;
}

bool ext2_vfs_init(VFS_FS *fs, VFS_Device *device, uint32_t start_lba)
{
	printk("Initializing device\n");
	EXT2_FS *ext2_fs = kmalloc(sizeof(EXT2_FS));
	ext2_fs->device = device->device;
	ext2_fs->read_sector = device->read;
	if (!ext2_fs->read_sector)
	{
		if (device->read == NULL)
		{
			printk("EXT2: read_sector is NULL from param\n");
		}
		printk("EXT2: read_sector is NULL\n");
		return 0;
	}
	ext2_fs->write_sector = device->write;
	ext2_fs->first_lba = start_lba;
	printk("Initializing EXT2 on partition starting at LBA %u\n", start_lba);
	if (ext2_init(ext2_fs) != 0)
	{
		printk("EXT2: failed to initialize\n");
		kfree(ext2_fs);
		return false;
	}

	printk("EXT2 Superblock OK (magic 0x%x)\n", ext2_fs->magic);
	fs->fs = ext2_fs;
	return 1;
}

static VFS_Node *ext2_vfs_open(VFS_FS *fs, const char *path)
{
	if (!fs || !path)
		return ERR_PTR(-EINVAL);

	PathParts_ext parts = format_folder_path_ext(path);

	char parent_path[512];
	build_parent_path_from_parts(parent_path, sizeof(parent_path), &parts);

	uint32_t parent_inode = ext2_parse_path(fs->fs, 2, parent_path);
	if (parent_inode == 0)
	{
		printk("EXT2: parent not found: %s (for %s)\n", parent_path, path);
		return ERR_PTR(-ENOENT);
	}

	uint32_t inode_number = ext2_parse_path(fs->fs, 2, path);
	if (inode_number == 0)
	{
		// файл не знайдено
		return ERR_PTR(-ENOENT);
	}

	Ext2File *file = kmalloc(sizeof(Ext2File));
	if (!file)
		return ERR_PTR(-ENOMEM);
	file->inode = inode_number;
	file->parent_inode = parent_inode;

	Ext2Inode inode_meta;
	if (ext2_read_inode(fs->fs, inode_number, &inode_meta) != 0)
	{
		kfree(file);
		return ERR_PTR(-EIO);
	}

	VFS_Node *node = kmalloc(sizeof(VFS_Node));
	if (!node)
	{
		kfree(file);
		return ERR_PTR(-ENOMEM);
	}

	node->fs_node = file;
	node->fs = fs;
	node->size = inode_meta.size;
	node->mode = inode_meta.mode;
	node->is_dir = (inode_meta.mode & 0x4000) != 0; // S_IFDIR

	const char *leaf = get_leaf_name(&parts);
	if (leaf)
	{
		strncpy(node->name, leaf, sizeof(node->name) - 1);
		node->name[sizeof(node->name) - 1] = '\0';
	}
	else
	{
		node->name[0] = '\0';
	}

	return node;
}

int ext2_vfs_read(VFS_File *node, void *buffer, uint32_t size)
{
	return ext2_read_file(node, buffer, size);
}

int ext2_vfs_write(VFS_File *node, const void *buffer, uint32_t size)
{
	return ext2_write_file(node, (uint8_t *)buffer, size);
}

int ext2_vfs_close(VFS_File *file)
{
	if (!file || !file->node->fs || !file->node->fs->close)
		return -EIO;
	kfree(file->node->fs_node);
	kfree(file->node);
	return 0;
}

// int ext2_vfs_lseek(VFS_File *node, int offset, int whence)
// {
// 	if (!node || !node->node->fs || !node->node->fs->lseek)
// 		return -EIO;
// 	return 0;
// }

uint8_t ext2_vfs_unlink(VFS_FS *fs, const char *path)
{
	if (!fs || !path)
		return false;

	VFS_Node *node = ext2_vfs_open(fs, path);
	if (IS_ERR(node))
		return false;

	Ext2File *file_meta = node->fs_node;
	if (!file_meta)
	{
		// cleanup
		kfree(node);
		return false;
	}

	uint32_t inode_number = file_meta->inode;
	uint32_t parent_inode = file_meta->parent_inode;

	// 1) Видалити запис з директорії
	if (ext2_remove_dir_entry(fs->fs, parent_inode, inode_number) != 0)
	{
		printk("EXT2: failed to remove dir entry for inode %u from parent %u\n", inode_number, parent_inode);
		// cleanup
		kfree(file_meta);
		kfree(node);
		return false;
	}

	Ext2Inode victim;
	if (ext2_read_inode(fs->fs, inode_number, &victim) == 0)
	{
		if (victim.links_count > 0)
		{
			victim.links_count--;
			ext2_write_inode(fs->fs, inode_number, &victim);
		}
		if (victim.links_count == 0)
		{
			// видалити дані/блоки і зарезервувати inode
			ext2_free_inode(fs->fs, inode_number);
			printk("EXT2: inode %u freed\n", inode_number);
		}
		else
		{
			printk("EXT2: inode %u link_count decreased to %u\n", inode_number, victim.links_count);
		}
	}
	else
	{
		printk("EXT2: cannot read inode %u to decrement links\n", inode_number);
	}

	// cleanup
	kfree(file_meta);
	kfree(node);
	return true;
}

uint8_t ext2_vfs_mkdir(VFS_FS *fs, const char *path)
{
	if (!fs || !path)
		return -EINVAL;

	PathParts_ext parts = format_folder_path_ext(path);
	char parent_path[512];
	build_parent_path_from_parts(parent_path, sizeof(parent_path), &parts);

	uint32_t parent_inode = ext2_parse_path(fs->fs, 2, parent_path);
	if (parent_inode == 0)
	{
		printk("EXT2: parent not found for create: %s\n", parent_path);
		return -ENOENT;
	}

	const char *name = get_leaf_name(&parts);
	if (!name || strlen(name) == 0)
	{
		return -EINVAL;
	}

	uint32_t inode_number = ext2_create_file(fs->fs, parent_inode, name);
	if (inode_number == 0)
	{
		printk("EXT2: failed to create file %s in parent inode %u\n", name, parent_inode);
		return -EIO;
	}

	return 0;
}

VFS_Node *ext2_vfs_create_file(VFS_FS *fs, const char *path)
{
	if (!fs || !path)
		return ERR_PTR(-EINVAL);

	PathParts_ext parts = format_folder_path_ext(path);
	char parent_path[512];
	build_parent_path_from_parts(parent_path, sizeof(parent_path), &parts);

	uint32_t parent_inode = ext2_parse_path(fs->fs, 2, parent_path);
	if (parent_inode == 0)
	{
		printk("EXT2: parent not found for create: %s\n", parent_path);
		return ERR_PTR(-ENOENT);
	}

	const char *name = get_leaf_name(&parts);
	if (!name || strlen(name) == 0)
	{
		return ERR_PTR(-EINVAL);
	}

	uint32_t inode_number = ext2_create_file(fs->fs, parent_inode, name);
	if (inode_number == 0)
	{
		printk("EXT2: failed to create file %s in parent inode %u\n", name, parent_inode);
		return ERR_PTR(-EIO);
	}

	// Відразу відкрити новий файл і повернути VFS_Node
	VFS_Node *node = ext2_vfs_open(fs, path);
	if (IS_ERR(node))
	{
		// створено inode, але не змогли відкрити (не повинно бути)
		return ERR_PTR(-EIO);
	}

	return node;
}

Directory ext2_vfs_readdir(VFS_FS *fs, const char *path)
{

	printk("path: %s\n", path);
	Ext2Inode *inode = kmalloc(sizeof(Ext2Inode));
	// inode = ext2_find_dir_entry(fs->fs, 2, path);
	ext2_read_inode(fs->fs, ext2_parse_path(fs->fs, 2, path), inode);
	return ext2_list_dir(fs->fs, inode);
}

// void fat32_init_vfs(VFS_FS *fs)
// {
// 	fs->mount = fat32_mount_wrapper;
// 	fs->unmount = fat32_unmount_wrapper;
// 	fs->open = fat32_open_wrapper;
// 	fs->read = fat32_read_wrapper;
// 	fs->write = fat32_write_wrapper;
// 	fs->create_file = fat32_create_file_wrapper;
// 	fs->mkdir = fat32_mkdir_wrapper;
// 	fs->unlink = fat32_unlink_wrapper;
// 	fs->readdir = fat32_readdir_wrapper;
// 	fs->close = fat32_close_wrapper;
// }

void ext2_init_vfs(VFS_FS *fs)
{
	fs->mount = ext2_vfs_init;
	fs->open = ext2_vfs_open;
	fs->read = ext2_vfs_read;
	fs->write = ext2_vfs_write;
	fs->close = ext2_vfs_close;
	fs->unlink = ext2_vfs_unlink;
	fs->mkdir = ext2_vfs_mkdir;
	fs->readdir = ext2_vfs_readdir;
	fs->create_file = ext2_vfs_create_file;
}
