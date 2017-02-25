/* executehelper.c - functions for execution
 * vim:ts=4 noexpandtab
 */
#include "types.h"
#include "lib.h"
#include "pcb.h"
#include "fs.h"
#include "executehelper.h"

extern pcb_t * pcb;

/* void parser(const uint8_t * command, uint8_t * cmd, uint8_t * args);
 * Inputs:
 * 		uint8_t * command (the full string entered into shell)
 * 		uint8_t * cmd (buffer to write the command name into e.g. "cat")
 * 		uint8_t * args (buffer to write the arguments into e.g. "frame0.txt")
 * Return value: void
 * Function: parses the command string by splitting it into the program to run
 * 		(cmd) and its arguments (args), and writes them into their respective
 * 		buffers
*/
void parser(const uint8_t* command, uint8_t* cmd, uint8_t* args)
{
	uint32_t i = 0;
	uint32_t t = 0;
	while(command[i] != ' ' && command[i] != '\n' && command[i] != '\0'){
		cmd[i] = command[i];
		i++;
	}
	if(command[i] == ' ') i++;
	while(command[i] != '\n' && command[i] != '\0'){
		args[t] = command[i];
		t++;
		i++;
		if(t==(ARGSIZE-1)) break;
	}
	if(t==(ARGSIZE-1)) args[t] = '\0';
	//if(t>0)
	//pcb->arguments = args;
}

/* uint32_t checkExe(uint8_t* cmd)
 * Inputs: uint8_t * cmd (buffer that stores the command to try to run
 * Return value: uint32_t (0 if a valid executable, -1 otherwise)
 * Function: finds the associated executable file for the command cmd, if it
 * 		exists, and determines if it is ELF
*/
uint32_t checkExe(uint8_t* cmd)
{
	dentry_t prog;
	if (read_dentry_by_name(cmd, &prog) == -1)
	{
		printf("%s is an invalid filename! ", cmd);
		return -1;
	}
	uint8_t elf[4];
	if(read_data(prog.inode_num, 0, elf, 4)==-1){
		printf("Could not read file:%s\n", cmd);
		return -1;
	}
	if(elf[0]==0x7F&&elf[1]=='E'&&elf[2]=='L'&&elf[3]=='F'){
		//printf("%s is an ELF file!\n", cmd);
		return 0;
	}
	else{
		printf("%s is not an ELF file :(", cmd);
		return -1;
	}
	return 0;
}
