#pragma once

#include <stdint.h>

#include <fs/ata/ata.h>
#include <fs/vfs/vfs.h>
#include <fs/vfs/vfs_standart_struct.h>

#include "ext2_struct.h"

PathParts_ext format_folder_path_ext(const char *in);
int IS_DIR(uint16_t mode);
int ext2_read_block(EXT2_FS *fs, uint32_t block_number, void *buf);
int ext2_write_block(EXT2_FS *fs, uint32_t block_number, void *buf);
uint32_t ext2_find_dir_entry(EXT2_FS *fs, uint32_t inode, const char *name);
uint32_t ext2_parse_path(EXT2_FS *fs, uint32_t inode, const char *path);
int ext2_write_inode(EXT2_FS *fs, uint32_t inode_num, Ext2Inode *inode);
int ext2_read_inode(EXT2_FS *fs, uint32_t inode_number, Ext2Inode *out_inode);
int ext2_write_group_desc(EXT2_FS *fs, uint32_t group, Ext2GroupDesc *desc);
int ext2_write_superblock(EXT2_FS *fs);
void ext2_free_block(EXT2_FS *fs, uint32_t block_number);
