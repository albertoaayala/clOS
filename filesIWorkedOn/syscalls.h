#ifndef _SYSCALLS_H
#define _SYSCALLS_H

#include "types.h"

#define KERNEL_STACK_BASE 	 0x800000
#define ENTRY_POINT_START 	 0x8000000
#define TASK_STACK_SIZE   	 8192
#define IMG_OFFSET 		  	 0x00048000
#define PHYSICAL_PROCESS  	 0x800000
#define physical_page_offset 0x400000

int8_t getBitMap(void);

int32_t sys_halt (uint8_t status);

int32_t sys_execute (const uint8_t* command);

int32_t sys_read (int32_t fd, void* buf, int32_t nbytes);

int32_t sys_write (int32_t fd, const void* buf, int32_t nbytes);

int32_t sys_open (const uint8_t* filename);

int32_t sys_close (int32_t fd);

int32_t sys_getargs (uint8_t* buf, int32_t nbytes);

int32_t vidmap (uint8_t** screen_start);

int32_t set_handler (int32_t signum, void* handler_address);

int32_t sigreturn (void);

#endif /* _SYSCALLS_H_ */
