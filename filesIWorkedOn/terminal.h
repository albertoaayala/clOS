/* terminal.h - Defines for useful keyboard functions
 * vim:ts=4 noexpandtab
 */

#ifndef _TERMINAL_H
#define _TERMINAL_H

#include "types.h"

#define BUFSIZE			128
#define ATTRIB 			0x7
/*Ports the keyboard is on*/
#define KB_PORT			0x60
#define KB_STAT			0x64
/*Help check if the key was just pressed or realesed*/
#define MAKE			0x01
/*Interrupt line to enable on the PIC*/
#define KB_IR_LINE			1

/*Scroll screen vertically*/
// void tScroll(int32_t terminal);

void updateOldBuffers(int32_t curTerm);

void updateBuffer(int32_t newTerm);
/*Fill the buffer when needed*/
int32_t insert(uint8_t letter);
/*Clear the buffer*/
void clearbuf(void);
/*update buffer for backspace*/
int32_t backspacebuf(void);
/*Read the user input*/
int32_t tRead(int32_t fd, void* buf, int32_t nbytes);

int32_t tWrite(int32_t fd, const void* buf, int32_t nbytes);

int32_t tOpen(const uint8_t* filename);

int32_t tClose(int32_t fd);

#endif /* _KEYHELPER_H_ */
