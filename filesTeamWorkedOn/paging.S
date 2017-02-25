#define ASM 1

//put pde address into CR3

.globl load_CR3

/*
* void load_CR3 (uint32_t *ptr)
*   Inputs: Pointer to page directory
*   Return Value: void
*	Function: Puts the page directory pointer into CR3
*/
load_CR3:
		//set up stack frame
		  push %ebp
		  mov %esp, %ebp

		  //load parameter(address of pde) and put it in crs

		  mov 8(%ebp), %ecx
		  mov %ecx, %cr3
		 leave
		  ret

/*
* void set_cr0 ()
*   Inputs: void
*   Return Value: none
*	Function: Set paging bit to enable paging
*/
.globl set_cr0
set_cr0:
				//set up stack frame
					push %ebp
					mov %esp, %ebp

					mov %cr0, %eax

					//set paging bit(32th)
					or $0x80000000, %eax
					mov %eax, %cr0

					leave
					ret

/*
* void set_pse()
*   Inputs: void
*   Return Value: none
*	Function: Sets the PSE to allow 4MB pages
*/
.globl set_pse
set_pse:
				//set up stack frame
					push %ebp
					mov %esp, %ebp

					mov %cr4, %eax
					//magic number need to fix

					//set physical size extension bit(4th)
					or $0x00000010, %eax
					mov %eax, %cr4
					leave
					ret
