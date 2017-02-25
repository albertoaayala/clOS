/* keyboard.c - Functions to init and deal with keyboard interrupts
 * vim:ts=4 noexpandtab
 */
#include "keyboard.h"
#include "lib.h"
#include "i8259.h"
#include "keyhelper.h"
#include "terminal.h"

static int back = 0;

/*
* void keyboard_init(void)
*   Inputs: void
*   Return Value: void
*	Function: Initializes the keyboard
*/
void
keyboard_init(void)
{
	/*uint8_t check;
	check = (inb(PORT_DATA)&0x01);
	if(!check){

	}*/									//maybe possible to just turn on interrupt line on PIC

	uint8_t check;
	uint8_t curCmd;
	uint32_t flags;

	cli_and_save(flags);

	check = (inb(KB_STAT)&0x01);			//check to see if the buffer is full if not then we can write to it
	if(!check){
		outb(RD_CMD, KB_STAT);				//tell keyboard we want to read the current command bits
		curCmd = inb(KB_PORT);				//get the current command bits
		outb(WRT_CMD, KB_STAT);				//tell the keyboard we want to write command bits
		outb((curCmd|SET_BITS), KB_PORT);	//enable the interrupt bit and the scancode set 1 transformation
	}

	enable_irq(KB_IR_LINE);					//enable the interrupt line
	keyInt = 0;

	// sti();
	restore_flags(flags);

}

/*
* void setback(void)
*   Inputs: None
*   Return Value: None
*	Function: Initialize the back variable to know where we are in the screen and not delete to far back in buf
*/
void
setback(void)
{
	back = 0;
}

/*
* void keyboard_handler(void)
*   Inputs: void
*   Return Value: void
*	Function: prints out the character preseed when we get a keyboard interrupt
*/
void
keyboard_handler(void)
{
	uint8_t letter;
	uint8_t check;
	uint8_t scancode;

	scancode = inb(KB_PORT);			//get the info from the keyboard_handler
	//printf("%x", scancode);
	check = scancode >> 7;				//use check to see if the key is pressed or realesed

	/*Check to see if it is a key we do not want to implement*/
	if(scan2Let(scancode)==0xFF) {
	}
	/*Set up wether the caps is on or off*/
	else if((scancode==CAPS)&&(scan2Let(scancode)==0x00)){
		setCaps(0x01);
	}
	else if((scancode==CAPS)&&(scan2Let(scancode)==0x01)){
		setCaps(0x00);
	}
	// else if(scancode==ENTR){
	// 	if(insert(scan2Let(scancode))<128){
	// 		printf("%c", scan2Let(scancode));		//print the character if it is being pressed
	// 		updatecursor();
	// 		back++;
	// 	}
	// 	letter = scan2Let(scancode);
	// 	printf("%c", letter);
	// 	updatecursor();
	// 	back++;
	// }
	/*See if the user is trying to clear the screen*/
	else if(scancode==CNTRL){
		/*Wait until another key is pressed or control key is released*/
		while(!(inb(KB_STAT)&MAKE)){
		}
		/*Check if the key is l then clear the screen if it is*/
		if(inb(KB_PORT)==CLR){
			/*wait until control key released to clear screen*/
			while(inb(KB_PORT)!=0x9D){
			}
			clear();					//clear the video mem
			clearbuf();					//clear the terminal buffer
			/*try to put cursor in top left corner*/
			updatecursor();
		}
	}
	else if(scancode==ALT){
		while(!(inb(KB_STAT)&MAKE)){
		}
		scancode = inb(KB_PORT);
		//printf("%x", scancode);
		if(scancode==F1){
			//printf("You are switching to terminal 1");
			screenChange(1);
		}
		else if(scancode==F2){
			//printf("You are switching to terminal 2");
			screenChange(2);
		}
		else if(scancode==F3){
			//printf("You are switching to terminal 3");
			screenChange(3);
		}
	}
	else if(scancode==BKSP) {
		int i = backspacebuf();
		if(i>-1){
			backspace();
			updatecursor();
		}
		// else if(back>0){
		// 	backspace();
		// 	updatecursor();
		// 	back = 0;
		// }
	}
	/*if special key not pressed print out the key being pressed*/
	else if(!(check&MAKE)){
		/*Checks to see if caps lock is off*/
		if(!scan2Let(CAPS)){
			if(scan2Let(scancode)==LSHFT){
				while(inb(KB_PORT)!=LUSHFT){
					while(!(inb(KB_STAT)&MAKE)){
					}
					scancode = inb(KB_PORT);
					if(scancode==LUSHFT) break;
					letter = scan2Let(scancode);
					if(scancode<SCANLIMIT){
						if((letter>LOWERLET)&&(letter<UPPERLET)){
							if(insert(letter-CAPCHANGE)<128){
								keyInt = 1;
								printf("%c", letter-CAPCHANGE);
								keyInt = 0;
								updatecursor();
								back++;
							}
						}
						else {
							letter = getShift(letter);
							if(insert(letter)<128){
								keyInt = 1;
								printf("%c", letter);
								keyInt = 0;
								updatecursor();
								back++;
							}
						}
					}
				}
			}
			else if(scan2Let(scancode)==RSHFT){
				//printf("%x", scancode);
				while(inb(KB_PORT)!=RUSHFT){
					while(!(inb(KB_STAT)&MAKE)){
					}
					scancode = inb(KB_PORT);
					if(scancode==RUSHFT) break;
					letter = scan2Let(scancode);
					if(scancode<SCANLIMIT){
						if((letter>LOWERLET)&&(letter<UPPERLET)){
							if(insert(letter-CAPCHANGE)<128){
								keyInt = 1;
								printf("%c", letter-CAPCHANGE);
								keyInt = 0;
								updatecursor();
								back++;
							}
						}
						else {
							letter = getShift(letter);
							if(insert(letter)<128){
								keyInt = 1;
								printf("%c", letter);
								keyInt = 0;
								updatecursor();
								back++;
							}
						}
					}
				}
			}
			else {
				if(insert(scan2Let(scancode))<128){
					keyInt = 1;
					printf("%c", scan2Let(scancode));		//print the character if it is being pressed
					keyInt = 0;
					updatecursor();
					back++;
				}
			}
		}
		else if(scan2Let(CAPS)){
			if(scan2Let(scancode)==LSHFT){
				while(inb(KB_PORT)!=LUSHFT){
					while(!(inb(KB_STAT)&MAKE)){
					}
					if(inb(KB_PORT)==LUSHFT) break;
					scancode = inb(KB_PORT);
					letter = scan2Let(scancode);
					if(scancode<SCANLIMIT){
						if((letter>LOWERLET)&&(letter<UPPERLET)){
							if(insert(letter)<128){
								keyInt = 1;
								printf("%c", letter);
								keyInt = 0;
								updatecursor();
								back++;
							}
						}
						else {
							letter = getShift(letter);
							if(insert(letter)<128){
								keyInt = 1;
								printf("%c", letter);
								keyInt = 0;
								updatecursor();
								back++;
							}
						}
					}
				}
			}
			else if(scan2Let(scancode)==RSHFT){
				//printf("%x", scancode);
				while(inb(KB_PORT)!=RUSHFT){
					while(!(inb(KB_STAT)&MAKE)){
					}
					if(inb(KB_PORT)==RUSHFT) break;
					scancode = inb(KB_PORT);
					letter = scan2Let(scancode);
					if(scancode<SCANLIMIT){
						if((letter>LOWERLET)&&(letter<UPPERLET)){
							if(insert(letter)<128){
								keyInt = 1;
								printf("%c", letter);
								keyInt = 0;
								updatecursor();
								back++;
							}
						}
						else {
							letter = getShift(letter);
							if(insert(letter)<128){
								keyInt = 1;
								printf("%c", letter);
								keyInt = 0;
								updatecursor();
								back++;
							}
						}
					}
				}
			}
			else
			{
				letter = scan2Let(scancode);
				if((letter>LOWERLET)&&(letter<UPPERLET)){
					if(insert(letter-CAPCHANGE)<128){
						keyInt = 1;
			 			printf("%c", letter-CAPCHANGE);		//print the character if it is being pressed
			 			keyInt = 0;
						updatecursor();
						back++;
					}
				}
				else {
					if(insert(letter)<128){
						keyInt = 1;
						printf("%c", letter);
						keyInt = 0;
						updatecursor();
						back++;
					}
				}
			}
		}
	}
	send_eoi(KB_IR_LINE);					//must send eoi to PIC when done
}

uint8_t
/*Get the shifted keys for non alphabet keys(could not find an easy way to convert from one to the other)*/
getShift(uint8_t letter)
{
	switch (letter) {
		case '`':
			return '~';
		case '1':
			return '!';
		case '2':
			return '@';
		case '3':
			return '#';
		case '4':
			return '$';
		case '5':
			return '%';
		case '6':
			return '^';
		case '7':
			return '&';
		case '8':
			return '*';
		case '9':
			return '(';
		case '0':
			return ')';
		case '-':
			return '_';
		case '=':
			return '+';
		case '[':
			return '{';
		case ']':
			return '}';
		case 0x5C:
			return '|';
		case ';':
			return ':';
		case 0x27:
			return '"';
		case ',':
			return '<';
		case '.':
			return '>';
		case '/':
			return '?';
		default:
			return -1;
	}
}
