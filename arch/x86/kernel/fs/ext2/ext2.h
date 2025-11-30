#pragma once

#include <stdint.h>
#include <stddef.h>
#include <fs/ext2/ext2_struct.h>
#include <fs/vfs/vfs.h>

int ext2_init(EXT2_FS *fs);
uint32_t ext2_parse_path(EXT2_FS *fs, uint32_t inode, const char *path);
uint32_t ext2_find_dir_entry(EXT2_FS *fs, uint32_t inode, const char *name);
uint32_t ext2_create_file(EXT2_FS *fs, uint32_t parent_inode, const char *name);
uint32_t ext2_create_directory(EXT2_FS *fs, uint32_t parent_inode, const char *name);
Directory ext2_list_dir(EXT2_FS *fs, Ext2Inode *dir_inode);
int ext2_read_inode(EXT2_FS *fs, uint32_t inode_number, Ext2Inode *out_inode);
int ext2_read_file(VFS_File *file, uint8_t *buf, uint32_t size);
int ext2_write_file(VFS_File *file, uint8_t *buf, uint32_t size);
void ext2_free_inode(EXT2_FS *fs, uint32_t inode_number);
int ext2_remove_dir_entry(EXT2_FS *fs, uint32_t dir_inode_num, uint32_t inode_num);
