
#include "syscalls.h"
#include "fs.h"
#include "pcb.h"
#include "rtc.h"
#include "paging.h"
#include "terminal.h"
#include "lib.h"
#include "x86_desc.h"
#include "types.h"
#include "executehelper.h"
#include "idt.h"

/* flag set when an exception happens in a user program */
static uint32_t program_exception = 0;

static int first = 0;
static int attemptExit = 0;
extern pcb_t * pcb;

#define EIGHT_KB (8 * 1024)
#define EIGHT_MB (8 * 1024 * 1024)
#define PROG_PAGE_SIZE 0x400000
#define PROG_VIR_ADDR_START 0x8000000
#define PCB_ADDR(PID) (KERNEL_STACK_BASE - ((PID + 1) * TASK_STACK_SIZE))


extern pcb_t * pcb;
extern uint8_t whichTerm;

uint8_t pid_bitmap = 0;
file_operations_table_t fops_rtc = {
	.open = rtc_open,
	.read = rtc_read,
	.write = rtc_write,
	.close = rtc_close,
};

file_operations_table_t fops_dir = {
	.open = dir_open,
	.read = dir_read,
	.write = dir_write,
	.close = dir_close,
};

file_operations_table_t fops_file = {
	.open = reg_file_open,
	.read = reg_file_read,
	.write = reg_file_write,
	.close = reg_file_close,
};

file_operations_table_t fops_term = {
	.open = tOpen,
	.read = tRead,
	.write = tWrite,
	.close = tClose,
};

int8_t
getBitMap(void)
{
	return pid_bitmap;
}

/*
* 	int32_t sys_execute(const uint8_t* command)
*   Inputs: command -- the input to tell what program to run with the arguments after a space
*   Return Value: int32_t (0 if success, -1 if failure)
*	Function: Updates the PCB with relevant information get the command and arguments needed for program to return
*			  sets up the paging for the program and context switches for the program to run properly then perform
*			  the artificial IRET to go to the right location for the program
*/
int32_t
sys_execute(const uint8_t* command)
{
	uint32_t i;
	uint32_t size = 1024;//strlen((int8_t*)command);
	uint8_t cmd[size+1];
	uint8_t args[ARGSIZE];
	for(i=0; i<size; i++){
		cmd[i]=0;
	}
	for(i=0; i<ARGSIZE; i++){
		args[i]=0;
	}
	parser(command, cmd, args);
	if(checkExe(cmd) == -1){
		return -1;
	}
	int32_t free_pid = 0;
	while (free_pid < 6) {
		if ((pid_bitmap & (1 << free_pid)) == 0) {
			break;
		}
		free_pid++;
		if(free_pid >= NUM_PROCESS){
			printf("Cannot run more than %d processes!\n", NUM_PROCESS);
			return -1;
		}
	}

	/* copy file contents */

	//set up paging
	setup_paging(free_pid);

	dentry_t prog;

	if (read_dentry_by_name(cmd, &prog) == -1)
		return -1;
	uint8_t prog_img[63*4096];
	int32_t num_byte = read_data(prog.inode_num, 0, prog_img, 63*4096);
	if(num_byte == -1)
		return -1;
	memcpy((void*)( IMG_OFFSET + ENTRY_POINT_START), (void*)prog_img, num_byte);

	pcb_t * old_pcb = pcb;
	pcb = (pcb_t*)(PCB_ADDR(free_pid));
	pcb->pid = free_pid;
	pcb->child_pid = -1;

	switch(whichTerm){
		case 0x01:
		{
			pcb->parent_pid = -1;
			pcb->curTerminal = 1;
			break;
		}
		case 0x02:
		{
			pcb->parent_pid = -1;
			pcb->curTerminal = 2;
			break;
		}
		case 0x04:
		{
			pcb->parent_pid = -1;
			pcb->curTerminal = 3;
			break;
		}
		default:
		{
			pcb->parent_pid = old_pcb->pid;
			break;
		}
	}
	whichTerm = 0x00;
	if (attemptExit){
		attemptExit = 0;
		pcb->parent_pid = -1;
		pcb->curTerminal = old_pcb->curTerminal;
	}

	if(pcb->parent_pid!=-1) {
		old_pcb->child_pid = pcb->pid;
		pcb->curTerminal = old_pcb->curTerminal;
	}

	pcb->file_array[0].fops_table = &fops_term;
	pcb->file_array[0].flags = TYPE_TERM | OPEN;

	pcb->file_array[1].fops_table = &fops_term;
	pcb->file_array[1].flags = TYPE_TERM | OPEN;

	pid_bitmap |= (1 << free_pid);

	 // calculate entry point: 128 MB + offset, where offset = buf[24-27]
	uint32_t entry_point = ENTRY_POINT_START;
	uint32_t * buf_bytes = (uint32_t *)prog_img;
	entry_point = buf_bytes[6];
	// printf("%x", entry_point);
	tss.eip = entry_point;

	//set up tss for context switch
	tss.esp0 = KERNEL_STACK_BASE - (pcb->pid * TASK_STACK_SIZE);
	tss.ss0 = KERNEL_DS;

	/* save stuff into pcb */
	pcb->arguments = args;
	if(!first) {
		first=1;
		screenInit();
	}
	int32_t child_esp = 0x8400000 - 4;

	asm volatile(
		"movl %%esp, %0;"
		"movl %%ebp, %1;"
		: "=r" ( pcb->parent_esp ), "=r" ( pcb->parent_ebp )
	);

	/* push artificial IRET context and return value onto stack;
	 * set IF to 1 and DPL to 3 then push eflags */
	asm volatile (
		"pushl $0x002B;"
		"pushl %1;"
		"pushf;"
		"popl %%ecx;"
		"orl $0x0200, %%ecx;"
		"pushl %%ecx;"
		"pushl $0x0023;"
		"pushl %0;"
		"movw $0x002B, %%dx;"
		"pushw %%dx;"
		"popw %%ds;"
		"iret;"
		".globl halt_ret_label ;"
		"halt_ret_label: ;"
		"cmpl $1, %2;"
		"jnz skip_set_retval;"
		"movl $256, %%eax;"
		"skip_set_retval: ;"
		"leave;"
		"ret"
		:
		: "r" (entry_point), "r" (child_esp), "r" (program_exception)
		: "%eax", "%ecx", "%dx"
	);

	return 0;
}

/*
* 	int32_t sys_halt(uint8_t status)
*   Inputs: status -- the return values
*   Return Value: status
*	Function: Updates our PCB to parent so that we are able to return correctly while updating the esp and ebp
*			  registers to be able to return to the correct location on the stack also updating our tss to work
*			  correctly
*/
int32_t
sys_halt(uint8_t status)
{
	/* check if the return value of the program is 256, meaning an exception
	 * happened, and set the flag (program_exception)
	 */
	uint32_t ret_val;
	asm volatile (
		"movl %%ebx, %0;"
		: "=r" (ret_val)
	);

	program_exception = 0;
	if (ret_val == 256) {
		program_exception = 1;
	}

	int i;
	for (i = 0; i < MAX_OPEN_FILES; i++){
		pcb->file_array[i].flags = CLOSE;
	}
	pcb_t * parent_pcb = (pcb_t *)(PCB_ADDR(pcb->parent_pid));
	pcb_t * old_pcb = pcb;
	// if (old_pcb->parent_pid == -1) {
	// 	printf("sys_halt: can't halt if pid is 0!\n");
	// 	asm volatile (
	// 		"movl $-1, %%eax;"
	// 		"jmp sys_halt_error ;"
	// 		:
	// 		:
	// 		: "%eax"
	// 	);
	// }

	/* free pid */
	pid_bitmap -= (1 << pcb->pid);

	if (old_pcb->parent_pid == -1){
		attemptExit = 1;
		return sys_execute((uint8_t*)"shell");
	}

	pcb = parent_pcb;
	pcb->child_pid = -1;

	/* restore old esp, ebp, return status */
	asm volatile (
		"movl %0, %%esp;"
		"movl %1, %%ebp;"
		:
		: "r" ( old_pcb->parent_esp ), "r" ( old_pcb->parent_ebp )
	);

	/* restore parent's paging */
	update_paging(pcb->pid);

	tss.esp0 = KERNEL_STACK_BASE - (pcb->pid * TASK_STACK_SIZE);
	tss.ss0 = KERNEL_DS;

	asm volatile (
		"xorl %%eax, %%eax;"
		"movb %%bl, %%al;"
		"sys_halt_error: ;"
		"jmp halt_ret_label;"
		:
		:
		: "%eax"
	);

	return 0;
}

/*
* 	int32_t sys_open(const uint8_t* filename)
*   Inputs: filename -- the name of the file that we want to be opend
*   Return Value: int32_t (-1 on failure; File Descriptor on success)
*	Function: Checks to make sure it is a valid file name then updates the PCB with the correct values depending
*			  on what file type it is
*/
int32_t
sys_open(const uint8_t* filename)
{
	int i = 2;
	while(pcb->file_array[i].flags != CLOSE){
		if (i >= MAX_OPEN_FILES - 1){
			return -1;
		}
		i++;
	}

	dentry_t open_file;
	if (read_dentry_by_name(filename, &open_file) == -1)		//check if valid filename
	{
		// printf("read_dentry_by_name failed!\n");
		return -1;
	}

	if (strncmp((int8_t*)filename, (int8_t*)"terminal", 8) == 0)
		open_file.file_type = TYPE_TERM;

	switch (open_file.file_type){			//check if it is a regular file type
		case TYPE_DIR:
		{
			pcb->file_array[i].fops_table = &fops_dir;
			pcb->file_array[i].inode = DIR_INODE;
			pcb->file_array[i].file_position = DIR_POS;
			pcb->file_array[i].flags = OPEN | TYPE_DIR;
			break;
		}
		case TYPE_FILE:
		{
			pcb->file_array[i].fops_table = &fops_file;
			pcb->file_array[i].inode = open_file.inode_num;		//update file system struct with proper values
			pcb->file_array[i].file_position = FILE_POS;
			pcb->file_array[i].flags = OPEN | TYPE_FILE;
			break;
		}
		case TYPE_RTC:
		{
			pcb->file_array[i].fops_table = &fops_rtc;
			pcb->file_array[i].inode = 0;
			pcb->file_array[i].file_position = 0;
			pcb->file_array[i].flags = OPEN | TYPE_RTC;
			break;
		}
		case TYPE_TERM:
		{
			pcb->file_array[i].fops_table = &fops_term;
			pcb->file_array[i].inode = 0;
			pcb->file_array[i].file_position = 0;
			pcb->file_array[i].flags = OPEN | TYPE_TERM;
			break;
		}
		default:
			break;
	}
	return i;
}


/*
* 	int32_t sys_read (int32_t fd, void* buf, int32_t nbytes)
*   Inputs: fd -- the file descriptor of the what must be read
*			buf -- Gets what ever is read
*			nbytes -- how many bytes we want read
*   Return Value: int32_t (-1 on failure; Number of bytes read on success)
*	Function: Checks to make sure we have a valid file descriptor if so then it chooses the PCB that corresponds
*			  with the file descriptor given and does the correct read
*/
int32_t
sys_read (int32_t fd, void* buf, int32_t nbytes)
{
	if (fd == 0 || (fd >=2 && fd < 8)){
		if (pcb->file_array[fd].flags & OPEN)
		{
			return pcb->file_array[fd].fops_table->read(fd, buf, nbytes);
		}
	}
	return -1;
}


/*
* 	int32_t sys_write (int32_t fd, const void* buf, int32_t nbytes)
*   Inputs: fd -- the file descriptor of the what must be written
*			buf -- The string that must be written
*			nbytes -- how many bytes we must be written
*   Return Value: int32_t (-1 on failure; Number of bytes written on success)
*	Function: Checks to make sure we have a valid file descriptor if so then it chooses the PCB that corresponds
*			  with the file descriptor given and does the correct write
*/
int32_t
sys_write (int32_t fd, const void* buf, int32_t nbytes)
{
	if ((fd >= 1 && fd < 8)){
		if (pcb->file_array[fd].flags & OPEN)
		{
			return pcb->file_array[fd].fops_table->write(fd, buf, nbytes);
		}
	}
	return -1;
}

/*
* 	int32_t sys_close (int32_t fd)
*   Inputs: fd -- the file descriptor of the what must be closed
*   Return Value: int32_t (-1 on failure; 0 on success)
*	Function: Checks to make sure we have a valid file descriptor (only 2-7 since we never close stdin
*			  stdout) if so then it chooses the PCB that corresponds with the file descriptor given and
*			  does the correct close
*/
int32_t
sys_close (int32_t fd)
{
	if (fd >= 2 && fd < 8){
		if (pcb->file_array[fd].flags & OPEN)
		{
			pcb->file_array[fd].flags = CLOSE;
			return pcb->file_array[fd].fops_table->close(fd);
		}
	}
	return -1;
}

/*
* 	int32_t sys_getargs (uint8_t* buf, int32_t nbytes)
*   Inputs: buf -- the empty string to put the arguments in
			nbytes -- the size of the buf
*   Return Value: int32_t (-1 on failure; 0 on success)
*	Function: gets the arguments from the PCB and puts it into the buf
*/
int32_t
sys_getargs (uint8_t* buf, int32_t nbytes)
{
	if (buf == NULL)
		return -1;
	int bufSize = strlen((int8_t*)buf);
	int i;
	for(i=0;i<bufSize;i++){
		buf[i] = '\0';
	}
	uint8_t * arg = pcb->arguments;
	uint32_t arg_length = strlen((int8_t*) arg );

	if(arg_length > nbytes)
		return -1;
	else if(arg_length==0) return -1;

	// memcpy(arg, buf, arg_length);
	strncpy((int8_t*)buf, (int8_t*)arg, arg_length);

	return 0;

}

/*
* 	int32_t vidmap (uint8_t ** screen_start)
*   Inputs: screen_start -- the location in virtual memory that video mem starts
*   Return Value: int32_t (-1 on failure; 0 on success)
*	Function: maps video for a program in user space
*/
int32_t
vidmap (uint8_t ** screen_start)
{

 	//check whether argument addr is valid(within range of the program page)

 	//uint8_t** curr_page_base = PROG_VIR_ADDR_START;

 	if((uint32_t)screen_start < PROG_VIR_ADDR_START || (uint32_t)screen_start >= (PROG_VIR_ADDR_START + PROG_PAGE_SIZE))
  		return -1;

  	*screen_start = (uint8_t*)PROG_VIR_ADDR_START+PROG_PAGE_SIZE;

	add_vidmap(PROG_VIR_ADDR_START+PROG_PAGE_SIZE);

 	return 0;
 }

 /*
 * 	int32_t set_handler (int32_t signum, void* handler_address)
 *  Inputs: signum --
 *			handler_address --
 *  Return Value: int32_t (-1)
 *	Function: Not used at the moment
 */
int32_t
set_handler (int32_t signum, void* handler_address)
{
	return -1;
}

/*
*  int32_t sigreturn (void)
*  Inputs: None
*  Return Value: int32_t (-1)
*  Function: Not used at the moment
*/
int32_t
sigreturn (void)
{
	return -1;
}
