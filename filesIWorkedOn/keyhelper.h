/* keyhelper.h - Defines for useful keyboard functions
 * vim:ts=4 noexpandtab
 */

#ifndef _KEYHELPER_H
#define _KEYHELPER_H

#include "types.h"

/*Help check if caps lock is on or off*/
#define CAPS			0x3A

/*Get the character of the scancode*/
uint8_t scan2Let(uint8_t scancode);
/*Set caps lock on or off*/
void setCaps(uint8_t set);

#endif /* _KEYHELPER_H_ */
