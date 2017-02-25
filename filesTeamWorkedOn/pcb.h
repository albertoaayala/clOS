#ifndef _PCB_H
#define _PCB_H

#include "types.h"
#include "fs.h"
#include "terminal.h"
#include "rtc.h"

#define MAX_OPEN_FILES 8

typedef struct __attribute__((packed)) file_operations_table_struct {
	int32_t (*open) (const uint8_t * filename);
	int32_t (*read) (int32_t fd, void* buf, int32_t nbytes);
	int32_t (*write) (int32_t fd, const void* buf, int32_t nbytes);
	int32_t (*close) (int32_t fd);
} file_operations_table_t;

typedef struct __attribute__((packed)) file_struct {
	file_operations_table_t * fops_table;
	int32_t inode;
	int32_t file_position;
	int32_t flags;
} file_struct_t;

typedef struct __attribute__((packed)) pcb_struct {
	file_struct_t file_array[MAX_OPEN_FILES];
	int32_t curTerminal;
	int32_t child_pid;
	int32_t pid;
	int32_t esp_screen;
	int32_t ebp_screen;
	int32_t esp_schedule;
	int32_t ebp_schedule;
	int32_t parent_pid;
	int32_t parent_esp;
	int32_t parent_ebp;
	uint32_t rtc_freq;
	uint8_t * arguments;

} pcb_t;

pcb_t * pcb;

#endif /* _PCB_H_ */
