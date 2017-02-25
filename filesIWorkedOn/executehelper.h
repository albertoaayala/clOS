/* parser.h - Defines for useful keyboard functions
 * vim:ts=4 noexpandtab
 */

#ifndef _EXECUTEHELPER_H
#define _EXECUTEHELPER_H

#include "types.h"
/*New defined arguments size since this is the biggest a name can be in the file directory*/
#define ARGSIZE				 32

void parser(const uint8_t* command, uint8_t* cmd, uint8_t* args);
uint32_t checkExe(uint8_t* cmd);

#endif /* _EXECUTEHELPER_H_ */
