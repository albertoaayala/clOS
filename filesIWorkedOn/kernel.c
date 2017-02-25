/* kernel.c - the C part of the kernel
 * vim:ts=4 noexpandtab
 */

/* reference: http://wiki.osdev.org/Setting_Up_Paging
 			http://www.osdever.net/tutorials/view/implementing-basic-paging

 */
#include "multiboot.h"
#include "x86_desc.h"
#include "lib.h"
#include "i8259.h"
#include "idt.h"
#include "paging.h"
#include "debug.h"
#include "rtc.h"
#include "keyboard.h"
#include "pit.h"
#include "fs.h"
#include "pcb.h"
#include "terminal.h"
#include "syscalls.h"

 //(VIDEO>>12)
#define VIDEO 0xB8000
#define VIDEO_TERM1 0x2401000
#define VIDEO_TERM2 0x2402000
#define VIDEO_TERM3 0x2403000
#define video_table_entry 184
#define kernel_start 0x400000
#define kernel_hardcode 0x400183

#define R_W_BIT_ENABLE	0x02

#define READWRITE_PRESENT_USER 7
#define SHELL_HARDCODE 0x800187
#define physical_page_offset 0x400000

//36MB
#define EMPTY_PHYS_MEM 0x2400000
#define FOUR_MB (4*1024*1024)



/* Macros. */
/* Check if the bit BIT in FLAGS is set. */
#define CHECK_FLAG(flags,bit)   ((flags) & (1 << (bit)))

extern uint8_t whichTerm;

bootblock_t * bblock;

//declare page directory array(allign to 4kb)
static uint32_t page_dir[NUM_PROCESS][NUM_PAGE_TABLES] __attribute__((aligned (PAGE_SIZE)));

//declare page table for 0-4 mb
static uint32_t page_table_0[NUM_PAGES] __attribute__((aligned(PAGE_SIZE)));//0-4mb table

//declare page table for 4kb for mapping video to user space
static uint32_t page_table_vidmap[NUM_PAGES] __attribute__((aligned(PAGE_SIZE)));

static uint32_t page_table_term[NUM_PAGES] __attribute__((aligned(PAGE_SIZE)));

void initilize_paging();

void setup_paging(int pid );

void add_vidmap(uint32_t virt_vmem);

void test_reg_file_open (uint8_t * file_name);

void test_reg_file_read (file_struct_t * fs_test, int32_t nbytes);

void test_reg_file_write (file_struct_t * fs_test, char * string, int32_t nbytes);

void test_reg_file_close (int32_t fd);

void test_dir_open (uint8_t * file_name);

void test_dir_read (file_struct_t * fs_test, int32_t nbytes, int32_t num_files);

void test_dir_write (file_struct_t * fs_test, char * string, int32_t nbytes);

void test_dir_close (int32_t fd);

void unit_test_read_dentry_filename(const uint8_t * file_name);

void unit_test_read_dentry_inode(int32_t inode_num);

void unit_test_read_data(uint32_t index);

void test_fs_print_filenames (int32_t print_inodes);

void test_fs_print_filesize (int32_t inode);

void test_terminal(void);


/* Check if MAGIC is valid and print the Multiboot information structure
   pointed by ADDR. */
void
entry (unsigned long magic, unsigned long addr)
{
	multiboot_info_t *mbi;

	/* Clear the screen. */
	clear();

	/* Am I booted by a Multiboot-compliant boot loader? */
	if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
	{
		printf ("Invalid magic number: 0x%#x\n", (unsigned) magic);
		return;
	}

	/* Set MBI to the address of the Multiboot information structure. */
	mbi = (multiboot_info_t *) addr;

	/* Print out the flags. */
	printf ("flags = 0x%#x\n", (unsigned) mbi->flags);

	/* Are mem_* valid? */
	if (CHECK_FLAG (mbi->flags, 0))
		printf ("mem_lower = %uKB, mem_upper = %uKB\n",
				(unsigned) mbi->mem_lower, (unsigned) mbi->mem_upper);

	/* Is boot_device valid? */
	if (CHECK_FLAG (mbi->flags, 1))
		printf ("boot_device = 0x%#x\n", (unsigned) mbi->boot_device);

	/* Is the command line passed? */
	if (CHECK_FLAG (mbi->flags, 2))
		printf ("cmdline = %s\n", (char *) mbi->cmdline);

	if (CHECK_FLAG (mbi->flags, 3)) {
		int mod_count = 0;
		int i;
		module_t* mod = (module_t*)mbi->mods_addr;
		while(mod_count < mbi->mods_count) {
			printf("string in module %d is : %s\n", mod_count, mod->string);
			printf("Module %d loaded at address: 0x%#x\n", mod_count, (unsigned int)mod->mod_start);
			printf("Module %d ends at address: 0x%#x\n", mod_count, (unsigned int)mod->mod_end);
			printf("First few bytes of module:\n");
			for(i = 0; i<16; i++) {
				printf("0x%x ", *((char*)(mod->mod_start+i)));
			}
			printf("\n");
			mod_count++;
			mod++;
		}
	}

	/* Start of fs test functions */

	// clear();
	// test_reg_file_open((uint8_t *)"shell");

	// WARNING: test_reg_file_open must return successfully before running this function"
	//clear();
	// printf("%c\n", 0x01);
	// test_reg_file_read (&fs_test, 100000);

	// WARNING: test_reg_file_open must return successfully before running this function"
	// clear();
	// test_reg_file_write (&fs_test, "test!", 1);

	// WARNING: test_reg_file_open must return successfully before running this function"
	// clear();
	// test_reg_file_close (2);

	// clear();
	// test_dir_open((uint8_t *)".");

	// WARNING: test_dir_open must return successfully before running this function"
	// clear();
	// test_dir_read (&fs_test, 32, 20);

	// WARNING: test_dir_open must return successfully before running this function"
	// clear();
	// test_dir_write (&fs_test, "test!", 1);

	// WARNING: test_dir_open must return successfully before running this function"
	// clear();
	// test_dir_close (2);

	// clear();
	// unit_test_read_dentry_filename((uint8_t *)"shell");

	// clear();
	// unit_test_read_dentry_inode(10000);
	// unit_test_read_dentry_inode(0);

	// clear();
	// unit_test_read_data(0);

	// /* print all files in the directory, plus their inodes */
	// clear();
	// test_fs_print_filenames(1);

	// /* print the size of sigtest */
	// clear();
	// printf("size of sigtest:\n");
	// test_fs_print_filesize(1);
	// asm volatile(".2: hlt; jmp .2;"); // comment this after testing fs
	/* End of fs test functions */

	/*Test terminal*/
	//printf("Testing Terminal Read");
	//test_term_read();

	/* Bits 4 and 5 are mutually exclusive! */
	if (CHECK_FLAG (mbi->flags, 4) && CHECK_FLAG (mbi->flags, 5))
	{
		printf ("Both bits 4 and 5 are set.\n");
		return;
	}

	/* Is the section header table of ELF valid? */
	if (CHECK_FLAG (mbi->flags, 5))
	{
		elf_section_header_table_t *elf_sec = &(mbi->elf_sec);

		printf ("elf_sec: num = %u, size = 0x%#x,"
				" addr = 0x%#x, shndx = 0x%#x\n",
				(unsigned) elf_sec->num, (unsigned) elf_sec->size,
				(unsigned) elf_sec->addr, (unsigned) elf_sec->shndx);
	}

	/* Are mmap_* valid? */
	if (CHECK_FLAG (mbi->flags, 6))
	{
		memory_map_t *mmap;

		printf ("mmap_addr = 0x%#x, mmap_length = 0x%x\n",
				(unsigned) mbi->mmap_addr, (unsigned) mbi->mmap_length);
		for (mmap = (memory_map_t *) mbi->mmap_addr;
				(unsigned long) mmap < mbi->mmap_addr + mbi->mmap_length;
				mmap = (memory_map_t *) ((unsigned long) mmap
					+ mmap->size + sizeof (mmap->size)))
			printf (" size = 0x%x,     base_addr = 0x%#x%#x\n"
					"     type = 0x%x,  length    = 0x%#x%#x\n",
					(unsigned) mmap->size,
					(unsigned) mmap->base_addr_high,
					(unsigned) mmap->base_addr_low,
					(unsigned) mmap->type,
					(unsigned) mmap->length_high,
					(unsigned) mmap->length_low);
	}

	/* Construct an LDT entry in the GDT */
	{
		seg_desc_t the_ldt_desc;
		the_ldt_desc.granularity    = 0;
		the_ldt_desc.opsize         = 1;
		the_ldt_desc.reserved       = 0;
		the_ldt_desc.avail          = 0;
		the_ldt_desc.present        = 1;
		the_ldt_desc.dpl            = 0x0;
		the_ldt_desc.sys            = 0;
		the_ldt_desc.type           = 0x2;

		SET_LDT_PARAMS(the_ldt_desc, &ldt, ldt_size);
		ldt_desc_ptr = the_ldt_desc;
		lldt(KERNEL_LDT);
	}

	/* Construct a TSS entry in the GDT */
	{
		seg_desc_t the_tss_desc;
		the_tss_desc.granularity    = 0;
		the_tss_desc.opsize         = 0;
		the_tss_desc.reserved       = 0;
		the_tss_desc.avail          = 0;
		the_tss_desc.seg_lim_19_16  = TSS_SIZE & 0x000F0000;
		the_tss_desc.present        = 1;
		the_tss_desc.dpl            = 0x0;
		the_tss_desc.sys            = 0;
		the_tss_desc.type           = 0x9;
		the_tss_desc.seg_lim_15_00  = TSS_SIZE & 0x0000FFFF;

		SET_TSS_PARAMS(the_tss_desc, &tss, tss_size);

		tss_desc_ptr = the_tss_desc;

		tss.ldt_segment_selector = KERNEL_LDT;
		tss.ss0 = KERNEL_DS;
		tss.esp0 = 0x800000;
		ltr(KERNEL_TSS);
	}

	// /* load the IDT and add some entries */
	lidt(idt_desc_ptr);

	/* exceptions, numbered 0 to 20 */
	unsigned int i;
	for (i = 0; i < 32; i++) {

			idt[i].present = 1;
			idt[i].dpl = 0;
			idt[i].seg_selector = KERNEL_CS;
			idt[i].reserved4 = 0;
			idt[i].reserved3 = 0;			/* interrupt gate */
			idt[i].reserved2 = 1;
			idt[i].reserved1 = 1;
			idt[i].size = 1;				/* gate is 32 bits */
			idt[i].reserved0 = 0;

	}

	SET_IDT_ENTRY(idt[0], exception_print0);
	SET_IDT_ENTRY(idt[1], exception_print1);
	SET_IDT_ENTRY(idt[2], exception_print2);
	SET_IDT_ENTRY(idt[3], exception_print3);
	SET_IDT_ENTRY(idt[4], exception_print4);
	SET_IDT_ENTRY(idt[5], exception_print5);
	SET_IDT_ENTRY(idt[6], exception_print6);
	SET_IDT_ENTRY(idt[7], exception_print7);
	SET_IDT_ENTRY(idt[8], exception_print8);
	SET_IDT_ENTRY(idt[9], exception_print9);
	SET_IDT_ENTRY(idt[10], exception_print10);
	SET_IDT_ENTRY(idt[11], exception_print11);
	SET_IDT_ENTRY(idt[12], exception_print12);
	SET_IDT_ENTRY(idt[13], exception_print13);
	SET_IDT_ENTRY(idt[14], exception_print14);
	SET_IDT_ENTRY(idt[16], exception_print16);
	SET_IDT_ENTRY(idt[17], exception_print17);
	SET_IDT_ENTRY(idt[18], exception_print18);
	SET_IDT_ENTRY(idt[19], exception_print19);

	SET_IDT_ENTRY(idt[15], exception_print_general);

	for (i = 20; i < 32; i++){

		SET_IDT_ENTRY(idt[i], exception_print_general);
	}

	/* devices: PIT on interrupt vector 0x20 */
	idt[0x20].present = 1;
	idt[0x20].dpl = 0;
	idt[0x20].seg_selector = KERNEL_CS;
	idt[0x20].reserved4 = 0;
	idt[0x20].reserved3 = 0;		/* interrupt gate */
	idt[0x20].reserved2 = 1;
	idt[0x20].reserved1 = 1;
	idt[0x20].size = 1;				/* gate is 32 bits */
	idt[0x20].reserved0 = 0;

	SET_IDT_ENTRY(idt[0x20], pit_handler_wrapper);


	/* devices: keyboard on interupt vector 0x21 */
	idt[0x21].present = 1;
	idt[0x21].dpl = 0;
	idt[0x21].seg_selector = KERNEL_CS;
	idt[0x21].reserved4 = 0;
	idt[0x21].reserved3 = 0;		/* interrupt gate */
	idt[0x21].reserved2 = 1;
	idt[0x21].reserved1 = 1;
	idt[0x21].size = 1;				/* gate is 32 bits */
	idt[0x21].reserved0 = 0;

	SET_IDT_ENTRY(idt[0x21], keyboard_handler_wrapper);

	/* devices: RTC on interupt vector 0x28 */
	idt[0x28].present = 1;
	idt[0x28].dpl = 0;
	idt[0x28].seg_selector = KERNEL_CS;
	idt[0x28].reserved4 = 0;
	idt[0x28].reserved3 = 0;		/* interrupt gate */
	idt[0x28].reserved2 = 1;
	idt[0x28].reserved1 = 1;
	idt[0x28].size = 1;
	idt[0x28].reserved0 = 0;

	SET_IDT_ENTRY(idt[0x28], rtc_handler_wrapper);

	/* syscalls on trap vector 0x80 */
	idt[0x80].present = 1;
	idt[0x80].dpl = 3;
	idt[0x80].seg_selector = KERNEL_CS;
	idt[0x80].reserved4 = 0;
	idt[0x80].reserved3 = 1;		/* trap gate */
	idt[0x80].reserved2 = 1;
	idt[0x80].reserved1 = 1;
	idt[0x80].size = 1;				/* gate is 32 bits */
	idt[0x80].reserved0 = 0;

	SET_IDT_ENTRY(idt[0x80], syscall_handler);

	/* init devices */
	i8259_init();
	rtc_init();
	pit_init();
	keyboard_init();

	/* Initialize devices, memory, filesystem, enable device interrupts on the
	 * PIC, any other initialization stuff... */

	initilize_paging();

	/* test various exceptions */
	/*
	int * ptrr;
	int b;
	for(i = kernel_start; i<kernel_start+(PAGE_SIZE*1024)-1;i++) {
		ptrr = i;
		printf("pass kernel memory %x \n",i);
		b  = *ptrr;}

		printf("pass all kernel memory\n");

		ptrr = i;

		b = *ptrr;
		int32_t aa;
		while(1){
		aa += 0x0ffffff;
		printf("num is %x \n",aa);
		asm volatile ("into");
	}
	*/

	/* Enable interrupts */
	/* test rtc_write */

	/* Execute the first program (`shell') ... */

	/* Enable interrupts */
	//clear();
	//printf("Enabling Interrupts\n");
	sti();


	/* test rtc_open

	uint8_t rtc_name[] = "rtc_test_filename";
	printf("running rtc_open with filename %s\n", rtc_name);
	rtc_open(rtc_name);

	rtc_wait(5, 0);
	clear();

	*/

	/* test rtc_write

	rtc_write(300); // freq not a power of 2
	rtc_write(RTC_DEFAULT_FREQ * 2); // freq out of range
	rtc_write(RTC_DEFAULT_FREQ / 2); // freq out of range

	rtc_wait(5, 1);
	clear();

	rtc_write(4);

	rtc_wait(5, 1);
	clear();

	rtc_write(1024);

	rtc_wait(5, 1);
	clear();

	*/

	/* rtc_read in an infinite loop

	rtc_write(2);
	while (1) {
		rtc_read();
	} */

	/*Testing terminal*/
	//test_terminal();

	/* test PIT
	pit_set_freq(3, 50);
	pit_set_freq(50, 50);
	pit_set_freq(0, 1);
	pit_set_freq(0, 18);
	pit_set_freq(0, PIT_FREQ_MAX + 5);
	pit_set_freq(0, 19);
	*/

	/* set the PIT to the frequency we will use for scheduling */
	pit_set_freq(0, PIT_FREQ_SCHEDULE);

	bblock = (bootblock_t *)((module_t *)(mbi->mods_addr))->mod_start;

	/* squash exceptions in user programs */
	SET_IDT_ENTRY(idt[0], exception_user_program);
	SET_IDT_ENTRY(idt[1], exception_user_program);
	SET_IDT_ENTRY(idt[2], exception_user_program);
	SET_IDT_ENTRY(idt[3], exception_user_program);
	SET_IDT_ENTRY(idt[4], exception_user_program);
	SET_IDT_ENTRY(idt[5], exception_user_program);
	SET_IDT_ENTRY(idt[6], exception_user_program);
	SET_IDT_ENTRY(idt[7], exception_user_program);
	SET_IDT_ENTRY(idt[8], exception_user_program);
	SET_IDT_ENTRY(idt[9], exception_user_program);
	SET_IDT_ENTRY(idt[10], exception_user_program);
	SET_IDT_ENTRY(idt[11], exception_user_program);
	SET_IDT_ENTRY(idt[12], exception_user_program);
	SET_IDT_ENTRY(idt[13], exception_user_program);
	SET_IDT_ENTRY(idt[14], exception_user_program);
	SET_IDT_ENTRY(idt[16], exception_user_program);
	SET_IDT_ENTRY(idt[17], exception_user_program);
	SET_IDT_ENTRY(idt[18], exception_user_program);
	SET_IDT_ENTRY(idt[19], exception_user_program);

	/* Execute the first program (`shell') ... */
	clear();
	whichTerm = 0x01;
	sys_execute((uint8_t*)"shell\n");

	/* Spin (nicely, so we don't chew up cycles) */
	asm volatile(".1: hlt; jmp .1;");
}

/*
* void initilize_paging(void)
*   Inputs: none
*   Return Value: void
*	Function: Initializes paging
*/
void
initilize_paging()
{
	//static uint32_t page_table_0[NUM_PAGES] __attribute__((aligned(PAGE_SIZE)));//0-4mb table
	int i, j;

	//initialize all page directory entry to 010, that is read/write, supervisor mode and nonpresent
	for(i=0;i<NUM_PROCESS;i++){
		for(j=0;j<NUM_PAGE_TABLES;j++){
			page_dir[i][j] = R_W_BIT_ENABLE;
		}
	}

	//fill first table entry as nonpresent
	uint32_t map_to_addr = 0x0;
	for(i=0;i<NUM_PAGES;i++){
		page_table_0[i] = map_to_addr | R_W_BIT_ENABLE;
		map_to_addr += PAGE_SIZE;
	}

	//set page table entry for video memory;
	//set attribute  to user level, read/write, present(111 in binary)
	page_table_0[video_table_entry] = VIDEO|7;
	for(i=0;i<NUM_PROCESS;i++){
		page_dir[i][0] = (uint32_t)page_table_0 | 0x07;
		page_dir[i][34] = ((uint32_t) page_table_term) | 7;
	}

	//set page directory entry 1 to point to kernel memory

	//set page size bit and the pde points to
	// page_dir[1] = kernel_start | (0X01<<7) | 0x03;
	for(i=0;i<NUM_PROCESS;i++){
		page_dir[i][1] = kernel_hardcode;
	}
	///////////////////////////////////
	//fill video mapping page table entry as non-present

	map_to_addr = EMPTY_PHYS_MEM;
	for(i=0;i<NUM_PAGES;i++){
		page_table_vidmap[i] = map_to_addr | R_W_BIT_ENABLE;
		page_table_term[i] = map_to_addr | R_W_BIT_ENABLE;
		map_to_addr += PAGE_SIZE;
	}

	//set page table entry for video memory;
	//set attribute  to user level, read/write, present(111 in binary)
	page_table_vidmap[0] = VIDEO|7;
	page_table_term[1] = VIDEO_TERM1|7;
	page_table_term[2] = VIDEO_TERM2|7;
	page_table_term[3] = VIDEO_TERM3|7;

	//set PSE to allow 4mb page
 	set_pse();
}

void
setup_paging(int pid)
{
	//map 128 mb virtual addr to 8mb
	page_dir[pid][32] = SHELL_HARDCODE + pid * physical_page_offset;
	set_pse();
	load_CR3(page_dir[pid]);
	//set paging bit
	set_cr0();
}

void
update_paging(int pid)
{
	load_CR3(page_dir[pid]);
}



void
add_vidmap(uint32_t virt_vmem)
{
	uint32_t index = virt_vmem / FOUR_MB;

	page_dir[pcb->pid][index] = ((uint32_t) page_table_vidmap) | 7;

 	load_CR3(page_dir[pcb->pid]);
}

// /*
// * void test_reg_file_open (uint8_t * file_name)
// *   Inputs: filename (the name of the file to open)
// *   Return Value: none
// *	Function: tries to open a filename and prints out success or failure
// */
// void
// test_reg_file_open (uint8_t * file_name)
// {
// 	if (reg_file_open(file_name) == -1)
// 	{
// 		printf("%s is an invalid filename!\n", file_name);
// 		return;
// 	}
// 	printf("%s successfully opened!\n", file_name);
// // 	printf("fs_test.inode = %d\n", fs_test.inode);
// // 	printf("fs_test.file_position = %d\n", fs_test.file_position);
// // 	printf("fs_test.flags = %d\n", fs_test.flags);
// }

// /*
// * void test_reg_file_read (file_struct_t * fs_test, int32_t nbytes)
// *   Inputs: fs_test (the dummy file descriptor), nbytes (num bytes to read)
// *   Return Value: none
// *	Function: uses helper function reg_file_read to read nbytes from a file,
// *			prints out the return value of reg_file_read
// */
// void
// test_reg_file_read (file_struct_t * fs_test, int32_t nbytes)
// {

// 	/* test read_data */
// 	uint8_t buf[64 * 4096 + 1];
// 	buf[64 * 4096] = '\0';

// 	int32_t ret_val = reg_file_read(fs_test, buf, nbytes);

// 	printf("reg_file_read returned value %d\n", ret_val);
// 	printf("%s\n", buf);
// }

// /*
// * void test_reg_file_write (file_struct_t * fs_test, char * string, int32_t nbytes)
// *   Inputs: fs_test (the dummy file descriptor), string (the string to write),
// *			nbytes (number of bytes to write)
// *   Return Value: none
// *	Function: tests reg_file_write, prints its return value
// */
// void
// test_reg_file_write (file_struct_t * fs_test, char * string, int32_t nbytes)
// {
// 	printf("reg_file_write returned %d\n", reg_file_write(fs_test, string, nbytes));
// }

// /*
// * void test_reg_file_close (int32_t fd)
// *	Inputs: fd (file index)
// *	Return Value: none
// *	Function: tests reg_file_close
// */
// void
// test_reg_file_close (int32_t fd)
// {
// 	reg_file_close(fd);
// }

// /*
// * void test_dir_open (uint8_t * dir_name)
// *	Inputs: dir_name (the directory name to open)
// *	Return Value: none
// *	Function: tests test_dir_open
// */
// void
// test_dir_open (uint8_t * dir_name)
// {
// 	if (dir_open(dir_name) == -1)
// 	{
// 		printf("%s is an invalid directory name!\n", dir_name);
// 		return;
// 	}
// 	printf("%s successfully opened!\n", dir_name);
// // 	printf("fs_test.inode = %d\n", fs_test.inode);
// // 	printf("fs_test.file_position = %d\n", fs_test.file_position);
// // 	printf("fs_test.flags = %d\n", fs_test.flags);
// }

// /*
// * void test_dir_read (file_struct_t * fs_test, int32_t nbytes, int32_t num_files)
// *	Inputs: fs_test (dummy file descriptor), nbytes, num_files
// *	Return Value: none
// *	Function: tests dir_read, prints its return value
// */
// void
// test_dir_read (file_struct_t * fs_test, int32_t nbytes, int32_t num_files)
// {

// 	/* test dir_read by reading num_files times */
// 	uint8_t buf[32];

// 	int32_t ret_val;

// 	while(num_files)
// 	{
// 		ret_val = dir_read(fs_test, buf, nbytes);
// 		if (ret_val <= 0)
// 		{
// 			printf("dir_read returned value %d\n", ret_val);
// 			break;
// 		}
// 		printf("%s\n", buf);
// 		num_files--;
// 	}
// }

// /*
// * void test_dir_write (file_struct_t * fs_test, char * string, int32_t nbytes)
// *	Inputs: fs_test (dummy file descriptor), string (stuff to write to the file), nbytes
// *	Return Value: none
// *	Function: tests dir_write, prints its return value
// */
// void
// test_dir_write (file_struct_t * fs_test, char * string, int32_t nbytes)
// {
// 	printf("dir_write returned %d\n", dir_write(fs_test, string, nbytes));
// }

// /*
// * void test_dir_close (int32_t fd)
// *	Inputs: fd (file index)
// *	Return Value: none
// *	Function: tests dir_close, prints its return value
// */
// void
// test_dir_close (int32_t fd)
// {
// 	dir_close(fd);
// }

/*
* void unit_test_read_dentry_filename (const uint8_t * file_name)
*	Inputs: file_name
*	Return Value: none
*	Function: tests read_dentry_by_name, prints debug info
*/
// void
// unit_test_read_dentry_filename (const uint8_t * file_name)
// {
// 	dentry_t test;
// 	if (read_dentry_by_name(file_name, &test) == 0)
// 	{
// 		printf("file_name : %s\n", test.file_name);
// 		printf("file_type : %d\n", test.file_type);
// 		printf("inode_num : %d\n", test.inode_num);
// 	}
// 	else
// 	{
// 		printf("file_name: %s does not exist!\n", file_name);
// 	}
// }

// /*
// * void unit_test_read_dentry_inode (int32_t inode_num)
// *	Inputs: inode_num
// *	Return Value: none
// *	Function: tests read_dentry_by_index, prints debug info
// */
// void
// unit_test_read_dentry_inode(int32_t inode_num)
// {
// 	dentry_t test;
// 	if (read_dentry_by_index(inode_num, &test) == 0)
// 	{
// 		printf("file_name : %s\n", test.file_name);
// 		printf("file_type : %d\n", test.file_type);
// 		printf("inode_num : %d\n", test.inode_num);
// 	}
// 	else
// 	{
// 		printf("inode_num: %d does not exist!\n", inode_num);
// 	}
// }

// /*
// * void unit_test_read_data (uint32_t index)
// *	Inputs: index (the inode number of the file to read)
// *	Return Value: none
// *	Function: tests read_data, prints debug info
// */
// void
// unit_test_read_data(uint32_t index)
// {
// 	uint8_t buf[64 * 4096 + 1];
// 	buf[64 * 4096] = '\0';

// 	int32_t read_data_retval;

// 	/* read more than the filesize of frame1.txt */
// 	read_data_retval = read_data(index, 0, buf, 10000);
// 	printf("read_data returned with value %d\n", read_data_retval);

// 	/* use an invalid offset to read frame1.txt */
// 	// read_data_retval = read_data(index, 1000, buf, 10000);
// 	// printf("read_data returned with value %d\n", read_data_retval);

// 	// /* use an invalid length to read frame1.txt */
// 	// read_data_retval = read_data(index, 10, buf, 0);
// 	// printf("read_data returned with value %d\n", read_data_retval);

// 	// /* start in the middle of frame1.txt */
// 	// read_data_retval = read_data(index, 50, buf, 10000);
// 	// printf("read_data returned with value %d\n", read_data_retval);
// }

// /*
// * void test_fs_print_filenames (int32_t print_inodes)
// *	Inputs: print_inodes (flag, if 0, doesn't print the inode numbers of every
// *			file, else, prints them)
// *	Return Value: none
// *	Function: print all filenames in the filesystem
// */
// void
// test_fs_print_filenames (int32_t print_inodes)
// {
// 	int d;
// 	for(d = 0; d < bblock->dir_entries; d++) {
// 		printf("filename: %s", bblock->dentry[d].file_name);
// 		if (print_inodes) {
// 			printf("        inode_num: %d\n", bblock->dentry[d].inode_num);
// 		} else {
// 			printf("\n");
// 		}
// 	}

// }

// /*
// * void test_fs_print_filesize (int32_t inode)
// *	Inputs: inode (inode num of the file that we want to print the size of)
// *	Return Value: none
// *	Function: prints the filesize of a file
// */
// void
// test_fs_print_filesize (int32_t inode)
// {
// 	inode_t * in = GET_INODE_ADRS(inode);
// 	printf("inode %u:, size %u\n", inode, in->length);
// }
