#include "keyhelper.h"

/*Got scancode array from Bran's Kernel Development Tutorial
* added 0xFF to all the scancodes we are not implementing
*  By:   Brandon F. (friesenb@gmail.com)
*/
static uint8_t getChar[128] =
{
    0xFF,  0xFF/*ESC*/, '1', '2', '3', '4', '5', '6', '7', '8',	/* 9 */
  '9', '0', '-', '=', '\b',	/* Backspace */
  0xFF/*implement tab?*/,			/* Tab */
  'q', 'w', 'e', 'r',	/* 19 */
  't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',		/* Enter key */
    0,			/* 29   - Control */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',	/* 39 */
 '\'', '`',  0x80,		/* Left shift */
 '\\', 'z', 'x', 'c', 'v', 'b', 'n',			/* 49 */
  'm', ',', '.', '/',   0x81,					/* Right shift */
  '*',
    0,	/* Alt */
  ' ',	/* Space bar */
    0x00,	/* Caps lock */
    0xFF,	/* 59 - F1 key ... > */
    0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,
    0xFF,	/* < ... F10 */
    0xFF,	/* 69 - Num lock*/
    0xFF,	/* Scroll Lock */
    0xFF,	/* Home key */
    0xFF,	/* Up Arrow */
    0xFF,	/* Page Up */
  '-',
    0xFF,	/* Left Arrow */
    0xFF,
    0xFF,	/* Right Arrow */
  '+',
    0xFF,	/* 79 - End key*/
    0xFF,	/* Down Arrow */
    0xFF,	/* Page Down */
    0xFF,	/* Insert Key */
    0xFF,	/* Delete Key */
    0xFF,   0xFF,   0xFF,
    0xFF,	/* F11 Key */
    0xFF,	/* F12 Key */
    0xFF,	/* All other keys are undefined */
};


uint8_t
scan2Let(uint8_t scancode)
{
	return getChar[scancode];
}

void
setCaps(uint8_t set)
{
	getChar[CAPS] = set;
}
