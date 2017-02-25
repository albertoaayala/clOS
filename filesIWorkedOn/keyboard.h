/* keyboard.h - Defines for useful keyboard functions
 * vim:ts=4 noexpandtab
 */

#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#include "types.h"

/*Ports the keyboard is on*/
#define KB_PORT			0x60
#define KB_STAT			0x64

/*Get and set commands*/
#define RD_CMD			0x20
#define WRT_CMD			0x60
#define	SET_BITS		0x41

/*Interrupt line to enable on the PIC*/
#define KB_IR_LINE			1

/*Help check if the key was just pressed or realesed*/
#define MAKE			0x01

/*Help check if special function keys are pressed and released*/
#define LSHFT			0x80
#define RSHFT			0x81
#define LUSHFT			0xAA
#define RUSHFT			0xB6
#define CAPS			0x3A
#define CNTRL			0x1D
#define BKSP			0x0E
#define CLR				0x26		//the scancode for l and lets us clear the screen
#define ENTR			0x1C
#define ALT				0x38
#define F1				0x3B
#define F2				0x3C
#define F3				0x3D

/*Check bounds*/
#define SCANLIMIT		128
#define LOWERLET		0x60
#define UPPERLET		0x7B
#define CAPCHANGE		0x20

uint8_t keyInt;

/*Initialize the Keyboard*/
void keyboard_init(void);
/*Keyboard interrupt handler*/
void keyboard_handler();
/*Get the shifted version of the key*/
uint8_t getShift(uint8_t letter);
/*Lets the tRead work properly*/
void setback(void);

#endif /* _KEYBOARD_H_ */
