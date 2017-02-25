#ifndef _IDT_H_
#define _IDT_H_

#include "syscalls.h"

#ifndef ASM
extern void exception_print0 ();
extern void exception_print1 ();
extern void exception_print2 ();
extern void exception_print3 ();
extern void exception_print4 ();
extern void exception_print5 ();
extern void exception_print6 ();
extern void exception_print7 ();
extern void exception_print8 ();
extern void exception_print9 ();
extern void exception_print10 ();
extern void exception_print11 ();
extern void exception_print12 ();
extern void exception_print13 ();
extern void exception_print14 ();
extern void exception_print16 ();
extern void exception_print17 ();
extern void exception_print18 ();
extern void exception_print19 ();
extern void exception_print_general ();
extern void exception_user_program ();
extern void keyboard_handler_wrapper ();
extern void rtc_handler_wrapper ();
extern void pit_handler_wrapper ();
extern void exception_print (int num);
extern void syscall_handler ();

extern int32_t sys_halt (uint8_t status);
extern int32_t sys_execute (const uint8_t* command);
extern int32_t sys_read (int32_t fd, void* buf, int32_t nbytes);
extern int32_t sys_write (int32_t fd, const void* buf, int32_t nbytes);
extern int32_t sys_open (const uint8_t* filename);
extern int32_t sys_close (int32_t fd);
extern int32_t sys_getargs (uint8_t* buf, int32_t nbytes);
extern int32_t vidmap (uint8_t** screen_start);
extern int32_t set_handler (int32_t signum, void* handler_address);
extern int32_t sigreturn (void);

#endif /* ASM */
#endif /* _IDT_H_ */
