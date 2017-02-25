#ifndef _FS_H
#define _FS_H

#include "types.h"
#include "multiboot.h"
#include "pcb.h"

#define MAX_FILES 63
#define BYTES_PER_BLOCK 4096

#define FILE_OPEN 1
#define FILE_CLOSE 0
#define OPEN 0x00000010
#define CLOSE 0x00000000
#define TYPE_DIR 0x00000001
#define TYPE_FILE 0x00000002
#define TYPE_RTC 0x00000000
#define TYPE_TERM 0x00000004
#define DIR_OPEN 1
#define DIR_CLOSE 0
#define FILE_POS 0
#define DIR_POS 0
#define DIR_INODE 0
// #define TYPE_DIR 1
// #define TYPE_FILE 2

#define GET_INODE_ADRS(index) \
	((inode_t *) (bblock + 1 + (index)))

#define GET_DATABLOCK_ADRS(index) \
	((uint8_t *) bblock + BYTES_PER_BLOCK * (bblock->inode_cnt + (index) + 1))

typedef struct __attribute__((packed)) inode_struct {
	int32_t length;
	int32_t data_block [63];
} inode_t;

typedef struct __attribute__((packed)) direntry_struct {
	char file_name [32];
	int32_t file_type;
	int32_t inode_num;
	int32_t pad[6];
} dentry_t;

typedef struct __attribute__((packed)) bootblock_struct {
	int32_t dir_entries;
	int32_t inode_cnt;
	int32_t data_block_cnt;
	int32_t pad[13];
	dentry_t dentry [63];
} bootblock_t;

extern bootblock_t * bblock;

int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry);

int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry);

int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

int32_t reg_file_open(const uint8_t* filename);

int32_t reg_file_read (int32_t fd, void* buf, int32_t nbytes);

int32_t reg_file_write (int32_t fd, const void* buf, int32_t nbytes);

int32_t reg_file_close(int32_t fd);

int32_t dir_open(const uint8_t* filename);

int32_t dir_read (int32_t fd, void* buf, int32_t nbytes);

int32_t dir_write (int32_t fd, const void* buf, int32_t nbytes);

int32_t dir_close(int32_t fd);


#endif /* _FS_H_ */
