#define ASM 1

.data
	# put useful constants here: LABEL = VALUE

.text
.globl exception_print0
.globl exception_print1
.globl exception_print2
.globl exception_print3
.globl exception_print4
.globl exception_print5
.globl exception_print6
.globl exception_print7
.globl exception_print8
.globl exception_print9
.globl exception_print10
.globl exception_print11
.globl exception_print12
.globl exception_print13
.globl exception_print14
.globl exception_print16
.globl exception_print17
.globl exception_print18
.globl exception_print19

.globl exception_print_general
.globl exception_print_handler
.globl exception_user_program

.globl syscall_handler
.globl rtc_handler_wrapper
.globl keyboard_handler_wrapper
.globl pit_handler_wrapper

exception_print0:
	movl $0, %eax
	jmp exception_print_handler

exception_print1:
	movl $1, %eax
	jmp exception_print_handler

exception_print2:
	movl $2, %eax
	jmp exception_print_handler

exception_print3:
	movl $3, %eax
	jmp exception_print_handler

exception_print4:
	movl $4, %eax
	jmp exception_print_handler

exception_print5:
	movl $5, %eax
	jmp exception_print_handler

exception_print6:
	movl $6, %eax
	jmp exception_print_handler

exception_print7:
	movl $7, %eax
	jmp exception_print_handler

exception_print8:
	movl $8, %eax
	jmp exception_print_handler

exception_print9:
	movl $9, %eax
	jmp exception_print_handler

exception_print10:
	movl $10, %eax
	jmp exception_print_handler

exception_print11:
	movl $11, %eax
	jmp exception_print_handler

exception_print12:
	movl $12, %eax
	jmp exception_print_handler

exception_print13:
	movl $13, %eax
	jmp exception_print_handler

exception_print14:
	movl $14, %eax
	jmp exception_print_handler

exception_print16:
	movl $16, %eax
	jmp exception_print_handler

exception_print17:
	movl $17, %eax
	jmp exception_print_handler

exception_print18:
	movl $18, %eax
	jmp exception_print_handler

exception_print19:
	movl $19, %eax
	jmp exception_print_handler

exception_print_general:
	movl $20, %eax
	jmp exception_print_handler

exception_print_handler:
	pushl %eax
	call print_exceptions

exception_user_program:
	movl $256, %ebx
	jmp sys_halt

	# spin indefinitely
spin:
	hlt
	jmp spin

rtc_handler_wrapper:
	cli
	pusha
	call rtc_handler
	popa
	sti
	iret

pit_handler_wrapper:
	cli
	pusha
	call pit_handler
	popa
	sti
	iret

keyboard_handler_wrapper:
	cli
	pusha
	call keyboard_handler
	popa
	sti
	iret

syscall_handler:

	pushl %esp
	pushl %ebp
	pushl %esi
	pushl %edi
	#put arguments on stack
	pushl %edx
	pushl %ecx
	pushl %ebx

	# check if the syscall number is valid, 1 <= eax <= 10
	cmpl $1, %eax
	jl syscall_invalid_number
	cmpl $10, %eax
	jg syscall_invalid_number

	# use eax as an index into syscall_table, so 0-9 instead of 1-10
	decl %eax
	movl $syscall_table, %edi

	call *(%edi, %eax, 4)
	jmp syscall_exit

syscall_invalid_number:
	movl $-1, %eax

syscall_exit:
	popl %ebx
	popl %ecx
	popl %edx
	popl %edi
	popl %esi
	popl %ebp
	popl %esp

	iret

.data

syscall_table:
	# TODO: replace these with our halt, execute, etc.
	.long sys_halt
	.long sys_execute
	.long sys_read
	.long sys_write
	.long sys_open
	.long sys_close
	.long sys_getargs
	.long vidmap
	.long set_handler
	.long sigreturn
