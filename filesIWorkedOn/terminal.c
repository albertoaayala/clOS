#include "terminal.h"
#include "lib.h"
#include "i8259.h"
#include "pcb.h"
#include "keyboard.h"
#include "syscalls.h"

#define VIDEO 0xB8000
#define NUM_COLS 80
#define NUM_ROWS 25

extern file_struct_t fs_test;

// static int idx1 = 0;
// static int idx2 = 0;
// static int idx3 = 0;
static int idx[3] ={0,0,0};
static int set = 1;
// static int8_t tbuf1[BUFSIZE];
// static int8_t tbuf2[BUFSIZE];
// static int8_t tbuf3[BUFSIZE];
static int8_t tbuf[3][BUFSIZE];

extern pcb_t * pcb;



// void
// updateOldBuffers(int32_t curTerm)
// {
// 	int i = 0;
// 	switch(curTerm){
// 		case 1:
// 			for(i=0; i<BUFSIZE; i++){
// 				tbuf[0][i] = tbuf[i];
// 			}
// 			idx1 = idx;
// 			break;
// 		case 2:
// 			for(i=0; i<BUFSIZE; i++){
// 				tbuf[1][i] = tbuf[i];
// 			}
// 			idx2 = idx;
// 			break;
// 		case 3:
// 			for(i=0; i<BUFSIZE; i++){
// 				tbuf[2][i] = tbuf[i];
// 			}
// 			idx3 = idx;
// 			break;
// 		default:
// 			break;
// 	}
// }
//
// void
// updateBuffer(int32_t newTerm)
// {
// 	int i = 0;
// 	switch(newTerm){
// 		case 1:
// 			for(i=0; i<BUFSIZE; i++){
// 				tbuf[i] = tbuf1[i];
// 			}
// 			idx = idx1;
// 			break;
// 		case 2:
// 			for(i=0; i<BUFSIZE; i++){
// 				tbuf[i] = tbuf2[i];
// 			}
// 			idx = idx2;
// 			break;
// 		case 3:
// 			for(i=0; i<BUFSIZE; i++){
// 				tbuf[i] = tbuf3[i];
// 			}
// 			idx = idx3;
// 			break;
// 		default:
// 			break;
// 	}
// }
/*
* 	void insert(uint8_t letter)
*   Inputs: letter -- the letter we want to add to tbuf
*   Return Value: None
*	Function: Updates tbuf by adding the letter into it
*/
int32_t
insert(uint8_t letter)
{
	if(idx[whatTerm()-1]>127) return idx[whatTerm()-1];
	tbuf[whatTerm()-1][idx[whatTerm()-1]] = letter;
	idx[whatTerm()-1]++;
	return idx[whatTerm()-1];
}

/*
* 	void clearbuf(void)
*   Inputs: None
*   Return Value: None
*	Function: Makes tbuf an empty string and sets idx to 0
*/
void
clearbuf(void)
{
	int i;
	for(i=0; i<BUFSIZE; i++){
		tbuf[pcb->curTerminal-1][i] = '\0';
	}
	idx[pcb->curTerminal-1] = 0;
}

/*
* 	int32_t backspacebuf(void)
*   Inputs: None
*   Return Value: The current location in tbuf we are at
*	Function: Makes sure we do not try to access outside of tbuf
*/
int32_t
backspacebuf(void)
{
	tbuf[whatTerm()-1][idx[whatTerm()-1]-1] = 0;
	if(idx[whatTerm()-1] > 0) idx[whatTerm()-1]--;
	else return -1;
	return idx[whatTerm()-1];
}

/*
* 	int32_t tRead(int32_t fd, void* buf, int32_t nbytes)
*   Inputs: fd -- Tells us the file descriptor
			buf -- The empty string we want to put the terminal read into
			nbytes -- the size of the buf
*   Return Value: int32_t (-1 on failure; How many bytes read on success)
*	Function: Gets input from user so that the user program may use it to run
*/
int32_t
tRead(int32_t fd, void* buf, int32_t nbytes)
{
	int32_t i;
	for(i = 0; i<=nbytes; i++){
		((uint8_t *)(buf))[i] = '\0';
	}

	updatecursor();
	setback();
	clearbuf();
	uint8_t scancode;
	while(idx[pcb->curTerminal-1]<BUFSIZE){
		cli();
		if(idx[pcb->curTerminal-1]==0);
		else if(tbuf[pcb->curTerminal-1][idx[pcb->curTerminal-1]-1]=='\n') {
			sti();
			break;
		}
		sti();
	}
	cli();
	if(idx[whatTerm()-1]==BUFSIZE){
		while(scancode!=ENTR){
			if(tbuf[pcb->curTerminal-1][idx[pcb->curTerminal-1]-1]=='\n') break;
			if(idx[pcb->curTerminal-1]==BUFSIZE){
				if(set==0){
					cli();
					set++;
				}
				while(!(inb(KB_STAT)&MAKE)){
				}
				scancode = inb(KB_PORT);
				if(scancode==BKSP||scancode==CNTRL||scancode==ALT){
					set=0;
					sti();
				}
			}
		}
		if(idx[pcb->curTerminal-1]==BUFSIZE){
			tbuf[pcb->curTerminal-1][BUFSIZE-1] = '\n';
			printf("\n");
		}
	}
	if(nbytes<=idx[pcb->curTerminal-1]){
		strncpy(buf, tbuf[pcb->curTerminal-1], nbytes);
		sti();
		return nbytes;
	}
	else{
		strncpy(buf, tbuf[pcb->curTerminal-1],idx[pcb->curTerminal-1]);
		sti();
		return idx[pcb->curTerminal-1];
	}
	clearbuf();
	sti();
	return -1;
}


/*
* int32_t tWrite (int32_t fd, const void* buf, int32_t nbytes)
*   Inputs: fd -- File descriptor of file to write to. (Terminal)
*			buf -- Data to write to the terminal.
*			nbytes -- Number of bytes to write to the terminal.
*   Return Value: int32_t (number of bytes written)
*	Function: Writes nbytes from the buf to the terminal.
*/
int32_t
tWrite (int32_t fd, const void* buf, int32_t nbytes)
{
	int i;
	int32_t bytesWritten = 0;
	uint8_t * buf_ptr = (uint8_t*)buf;
	for(i = 0; i < nbytes; i++)
	{
		if(buf_ptr[i]){
			printf("%c", buf_ptr[i]);
			bytesWritten++;
		}
		else if((i==(nbytes-1))/*&&(nbytes <= 1024)*/&&(buf_ptr[nbytes-1]=='\0')){
			printf("\n");
		}
	}
	// if((i==(nbytes-1))/*&&(nbytes <= 1024)*/&&(buf_ptr[nbytes-1]=='\0')){
	// 	printf("\n");
	// }
	return bytesWritten;
}

/*
* int32_t tOpen(const uint8_t* filename)
*   Inputs: filename -- File to open. (Terminal)
*   Return Value: int32_t (0 if success, -1 if failure)
*	Function: Opens the terminal by settting up the variables used by terminal.
*/
int32_t
tOpen(const uint8_t* filename)
{
	clearbuf();
	return 0;
}

/*
* int32_t tClose(int32_t fd)
*   Inputs: fd -- File descriptor of file to close.
*   Return Value: int32_t (0 if success, -1 if failure)
*	Function: Closes the terminal in the PCB array and makes it available.
*/
int32_t
tClose(int32_t fd)
{
	return 0;
}
