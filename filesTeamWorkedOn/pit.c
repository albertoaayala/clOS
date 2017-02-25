/* pit.c - Functions to init and deal with PIT interrupts
* vim:ts=4 noexpandtab
*/

#include "i8259.h"
#include "lib.h"
#include "pit.h"
#include "schedule.h"
#include "syscalls.h"

/*	void pit_init();
 *	Inputs: void
 *	Return: void
 *	Function: initializes the PIT
 */
void pit_init() {
	cli();
	enable_irq(PIT_IRQ_LINE);
	sti();
}

/*	void pit_handler();
 *	Inputs: void
 *	Return: void
 *	Function: the interrupt handler for the PIT
 */
void pit_handler() {
	/* DEBUG
	printf("PIT interrupt !\n");
	*/
	if (getBitMap())
		schedule();

	send_eoi(PIT_IRQ_LINE);
}

/*	int32_t pit_set_freq(uint8_t channel, uint32_t freq);
 *	Inputs: uint8_t channel, uint32_t freq
 *	Return: 0 if success, -1 if failed
 *	Function: sets the PIT channel frequency to freq
 */
int32_t pit_set_freq(uint8_t channel, uint32_t freq) {
	/* bad channel number */
	if (channel > 2 || channel == 1) {
		return -1;
	}

	/* invalid frequency */
	if (freq > PIT_FREQ_MAX || freq < PIT_FREQ_MIN) {
		return -1;
	}

	cli();
	/* set the new frequency */
	uint16_t divider = PIT_FREQ_MAX / freq;
	/* tell PIT what channel to set, use square wave mode */
	outb(PIT_SET_RATE_CMD + (channel << 6), PIT_CMD_PORT);
	/* send low byte */
	outb(divider & 0xFF, PIT_CH0_PORT + channel);
	/* send high byte */
	outb(divider >> 8, PIT_CH0_PORT + channel);
	sti();

	return 0;
}
