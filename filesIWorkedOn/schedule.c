/* schedule.c - Functions for scheduling
* vim:ts=4 noexpandtab
*/
#include "syscalls.h"
#include "pcb.h"
#include "schedule.h"
#include "paging.h"
#include "x86_desc.h"
#include "pit.h"
#include "i8259.h"

static int32_t curProc = 0;
static pcb_t * old_pcb;

extern pcb_t * pcb;

void
schedule(void)
{
	int8_t activeTerminals = getBitMap();
	old_pcb = pcb;
	// while((activeTerminals&(0x01<<curProc))==0){
	// 	curProc++;
	// 	if(curProc==6) curProc = 0;
	// }
	pcb = (pcb_t *)(KERNEL_STACK_BASE - ((curProc+1) * TASK_STACK_SIZE));
	while(((activeTerminals&(0x01<<curProc))==0)||(pcb->child_pid!=-1)){
		curProc++;
		if(curProc==6) {
			curProc = 0;
		}
		pcb = (pcb_t *)(KERNEL_STACK_BASE - ((curProc+1) * TASK_STACK_SIZE));
	}
	if(curProc == 5) curProc = 0;
	else curProc++;

	/* Save old pcb esp, ebp restore current pcb esp, ebp */

	if(old_pcb == pcb){
		asm volatile (
			"movl %%esp, %0;"
			"movl %%ebp, %1;"
			: "=a" (old_pcb->esp_schedule), "=b" (old_pcb->ebp_schedule)
		);
		return;
	}
	else {
		asm volatile (
			"movl %%esp, %0;"
			"movl %%ebp, %1;"
			"movl %2, %%esp;"
			"movl %3, %%ebp;"
			: "=a" (old_pcb->esp_schedule), "=b" (old_pcb->ebp_schedule)
			: "c" (pcb->esp_schedule), "d" (pcb->ebp_schedule)
		);
	}
	/* restore parent's paging */
	update_paging(pcb->pid);
	tss.esp0 = KERNEL_STACK_BASE - (pcb->pid * TASK_STACK_SIZE);
	tss.ss0 = KERNEL_DS;
	send_eoi(PIT_IRQ_LINE);
}
