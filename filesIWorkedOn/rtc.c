/* rtc.c - Functions to init and deal with RTC interrupts
* vim:ts=4 noexpandtab
*/
#include "rtc.h"
#include "lib.h"
#include "i8259.h"

/* the flag that the interrupt handler changes when it runs so rtc_read knows
 * when to return
 */
volatile int32_t rtc_interrupt_happened = 0;

/* for testing rtc_write and rtc_read, keeps track of how many rtc interrupts
 * have happened
*/
volatile uint32_t rtc_interrupt_count = 0;

/* keeps track of the current rtc frequency */
int32_t rtc_cur_freq;

/*
* void rtc_init(void)
*   Inputs: void
*   Return Value: void
*	Function: Initializes the RTC
*/
void
rtc_init(void)
{
	uint8_t curReg;
	// uint32_t flags;
	// cli_and_save(flags);
	cli();
	/*Process to turn on interrupts while disabling NMI*/
	outb(REGB_NONMI, RTC_PORT);		//must set the register we want to read from on
	curReg = inb(PORT_DATA);			//get the registers original state
	outb(REGB_NONMI, RTC_PORT);		//when you read from this port it resets the register to D so have to set it back to B
	outb((curReg|RTC_ENABLE),PORT_DATA);	//set the register with PIE set to RTC_ENABLE interrupts

	/*Must RTC_ENABLE the interrupt line on the PIC*/
	enable_irq(IR_LINE);
	/* re-enable the slave pic's pin */
	enable_irq(2);

	/*Turn the NMI back on*/
	outb(NMI_ON, RTC_PORT);

	sti();
	// restore_flags(flags);
}

/*
* void rtc_handler(void)
*   Inputs: void
*   Return Value: void
*	Function: prints string and executes test_interrupts function when there is
*	an interrupt from the RTC
*/
void
rtc_handler(void)
{
	/* DEBUG */
	rtc_interrupt_count++;
	// printf("received!\n");

	outb(REGC, RTC_PORT);
	inb(PORT_DATA);
	send_eoi(IR_LINE);
	rtc_interrupt_happened = 1;
}

/*
* int32_t rtc_read (void)
*   Inputs: none
*   Return Value: int32_t (0 if success)
*	Function: returns 0 after an rtc interrupt is handled by rtc_handler
*/
int32_t
rtc_read (int32_t fd, void* buf, int32_t nbytes)
{
	/* DEBUG */
	// printf("rtc_read: waiting for interrupt #%u...", rtc_interrupt_count);

	/* set flag and wait for it to be changed by rtc_handler, then return */
	// cli();
	rtc_interrupt_happened = 0;
	// sti();
	while (!rtc_interrupt_happened) {}

	return 0;
}

/*
* int32_t rtc_write (int32_t freq)
*   Inputs: freq (frequency that the rtc chip will send interrupts at)
*   Return Value: int32_t (0 if success, -1 if failure)
*	Function: sets the new rtc frequency
*/
int32_t
rtc_write (int32_t fd, const void* buf, int32_t nbytes)
{
	int32_t freq = *(uint32_t*)buf;
	/* DEBUG
	printf("rtc_write: attempting to set rtc frequency to %u hz\n", freq);
	*/

	/* check if freq is a power of 2 */
	int32_t tmp = freq;
	while ((tmp > 1) && (tmp % 2 == 0)) {
		tmp = tmp >> 1;
	}

	if (tmp != 1) {
		/* DEBUG */
		printf("rtc_write: ERROR - new frequency not a power of 2\n");
		return -1;
	}

	/* check if freq is outside of valid range */
	if (freq > RTC_PRGM_MAX_FREQ || freq <= 0) {
		/* DEBUG */
		printf("rtc_write: ERROR - frequency power of 2 but not in valid range\n");
		return -1;
	}

	/* calculate rate based on freq */
	int32_t rate = 1;
	while ((RTC_DEFAULT_FREQ >> (rate - 1)) != freq) {
		rate++;
	}

	/* is rate in the valid range, since rate must fit in 4 bytes */
	if (rate < RTC_RATE_LOW || rate > RTC_RATE_HIGH) {
		/* DEBUG */
		printf("rtc_write: ERROR - new rate out of range\n");
		return -1;
	}


	/* DEBUG
	printf("rtc_write: set rtc frequency to %u hz\n", freq);
	*/

	rtc_cur_freq = freq;

	/* disable all interrupts */
	cli();
	outb(REGB_NONMI, RTC_PORT);

	rtc_interrupt_count = 0;
	/* modify the A register to set the new rate */

	/* set index to REGA */
	outb(REGA, RTC_PORT);
	/* get previous value of REGA */
	char reg_prev = inb(PORT_DATA);
	/* reset index to REGA */
	outb(REGA, RTC_PORT);
	/* write rate to the bottom 4 bits */
	reg_prev = (reg_prev & 0xF0) | rate;
	outb(reg_prev, PORT_DATA);

	/* enable all interrupts */
	outb(NMI_ON, RTC_PORT);
	sti();

	return 0;

}

/*
* int32_t rtc_open (const uint8_t * filename)
*   Inputs: filename
*   Return Value: int32_t (0 if success, -1 if failure)
*	Function: initializes the rtc
*/
int32_t
rtc_open (const uint8_t * filename)
{
	int32_t buf = 2;
	return rtc_write(0, &buf, 0);
}

/*
* int32_t rtc_close (int32_t fd)
*   Inputs: fd, the file descriptor
*   Return Value: int32_t (0 if success, -1 if failure)
*	Function: closes the rtc
*/
int32_t
rtc_close (int32_t fd)
{
	return 0;
}

/*
* void rtc_wait (uint32_t seconds, uint32_t print)
*   Inputs: seconds: number of seconds to wait
*			print: 0 to not print the debug statement(s), 1 if yes
*   Return Value: none
*	Function: uses rtc interrupts to wait for an integer number of seconds
*/
void
rtc_wait (uint32_t seconds, uint32_t print)
{
	/* DEBUG */
	if (print) {
		printf("rtc_wait: waiting for %u seconds...\n", seconds);
	}

	cli();
	rtc_interrupt_count = 0;
	sti();
	uint32_t ticks = seconds * rtc_cur_freq;
	while (ticks) {
		rtc_read(0,0,0);
		ticks--;
	}

}

// /* rtc.c - Functions to init and deal with RTC interrupts
// * vim:ts=4 noexpandtab
// */
// #include "rtc.h"
// #include "lib.h"
// #include "i8259.h"

// /* keeps track of how many rtc interrupts have happened, useful for rtc
//  * virtualization
// */
// volatile uint32_t rtc_interrupt_count = 0;

// /*
// * void rtc_init(void)
// *   Inputs: void
// *   Return Value: void
// *	Function: Initializes the RTC
// */
// void
// rtc_init(void)
// {
// 	uint8_t curReg;
// 	cli();
// 	/*Process to turn on interrupts while disabling NMI*/
// 	outb(REGB_NONMI, RTC_PORT);		//must set the register we want to read from on
// 	curReg = inb(PORT_DATA);			//get the registers original state
// 	outb(REGB_NONMI, RTC_PORT);		//when you read from this port it resets the register to D so have to set it back to B
// 	outb((curReg|RTC_ENABLE),PORT_DATA);	//set the register with PIE set to RTC_ENABLE interrupts

// 	/*Must RTC_ENABLE the interrupt line on the PIC*/
// 	enable_irq(IR_LINE);
// 	/* re-enable the slave pic's pin */
// 	enable_irq(2);

// 	/*Turn the NMI back on*/
// 	outb(NMI_ON, RTC_PORT);

// 	sti();

// 	/* set the rtc to the virtualization frequenty, 1024 Hz, which is the lowest
// 	 * rate
// 	 */
// 	rtc_set_freq(RTC_RATE_LOW);
// }

// /*
// * void rtc_handler(void)
// *   Inputs: void
// *   Return Value: void
// *	Function: prints string and executes test_interrupts function when there is
// *	an interrupt from the RTC
// */
// void
// rtc_handler(void)
// {
// 	/* DEBUG */
// 	// printf("received!\n");

// 	outb(REGC, RTC_PORT);
// 	inb(PORT_DATA);
// 	send_eoi(IR_LINE);
// 	// printf("rtc_interrupt_count: %u\n", rtc_interrupt_count);
// 	rtc_interrupt_count++;
// }

// /*
// * int32_t rtc_read (void)
// *   Inputs: none
// *   Return Value: int32_t (0 if success)
// *	Function: returns 0 after an rtc interrupt is handled by rtc_handler
// */
// int32_t
// rtc_read (int32_t fd, void* buf, int32_t nbytes)
// {
// 	/* DEBUG */
// 	// printf("rtc_read: waiting for interrupt #%u...\n", rtc_interrupt_count);

// 	/* return when the program's rtc frequency is met */
// 	uint32_t when_to_stop = RTC_PRGM_MAX_FREQ/pcb->rtc_freq;
// 	while ((rtc_interrupt_count % when_to_stop) != when_to_stop-1) {}

// 	return 0;
// }

// /*
// * int32_t rtc_write (int32_t freq)
// *   Inputs: freq (frequency that the rtc chip will send interrupts at)
// *   Return Value: int32_t (0 if success, -1 if failure)
// *	Function: sets the new rtc frequency
// */
// int32_t
// rtc_write (int32_t fd, const void* buf, int32_t nbytes)
// {
// 	uint32_t freq = *(uint32_t*)buf;

// 	/* check if freq is a power of 2 */
// 	uint32_t tmp = freq;
// 	while ((tmp > 1) && (tmp % 2 == 0)) {
// 		tmp = tmp >> 1;
// 	}

// 	if (tmp != 1) {
// 		return -1;
// 	}

// 	/* check if freq is outside of valid range */
// 	if (freq > RTC_PRGM_MAX_FREQ || freq <= 0) {
// 		return -1;
// 	}

// 	/* calculate rate based on freq */
// 	uint32_t rate = 1;
// 	while ((RTC_DEFAULT_FREQ >> (rate - 1)) != freq) {
// 		rate++;
// 	}

// 	/* is rate in the valid range, since rate must fit in 4 bytes */
// 	if (rate < RTC_RATE_LOW || rate > RTC_RATE_HIGH) {
// 		return -1;
// 	}

// 	/* frequency is valid, so save the program's freq in the pcb */
// 	pcb->rtc_freq = freq;

// 	/* set rtc to RTC_PRGM_MAX_FREQ for virtualization */
// 	rtc_set_freq(RTC_RATE_LOW);

// 	return 0;

// }

// /*
//  * void rtc_set_freq (uint32_t rate);
//  *
// */
// void rtc_set_freq (uint32_t rate) {
// 	/* disable all interrupts */
// 	cli();
// 	outb(REGB_NONMI, RTC_PORT);

// 	rtc_interrupt_count = 0;
// 	/* modify the A register to set the new rate */

// 	/* set index to REGA */
// 	outb(REGA, RTC_PORT);
// 	/* get previous value of REGA */
// 	char reg_prev = inb(PORT_DATA);
// 	/* reset index to REGA */
// 	outb(REGA, RTC_PORT);
// 	/* write rate to the bottom 4 bits */
// 	reg_prev = (reg_prev & 0xF0) | rate;
// 	outb(reg_prev, PORT_DATA);

// 	/* enable all interrupts */
// 	outb(NMI_ON, RTC_PORT);
// 	sti();
// }

// /*
// * int32_t rtc_open (const uint8_t * filename)
// *   Inputs: filename
// *   Return Value: int32_t (0 if success, -1 if failure)
// *	Function: initializes the rtc
// */
// int32_t
// rtc_open (const uint8_t * filename)
// {
// 	int32_t buf = 2;
// 	return rtc_write(0, &buf, 0);
// }

// /*
// * int32_t rtc_close (int32_t fd)
// *   Inputs: fd, the file descriptor
// *   Return Value: int32_t (0 if success, -1 if failure)
// *	Function: closes the rtc
// */
// int32_t
// rtc_close (int32_t fd)
// {
// 	return 0;
// }

// /*
// * void rtc_wait (uint32_t seconds, uint32_t print)
// *   Inputs: seconds: number of seconds to wait
// *			print: 0 to not print the debug statement(s), 1 if yes
// *   Return Value: none
// *	Function: uses rtc interrupts to wait for an integer number of seconds
// */
// void
// rtc_wait (uint32_t seconds, uint32_t print)
// {
// 	/* DEBUG */
// 	if (print) {
// 		printf("rtc_wait: waiting for %u seconds...\n", seconds);
// 	}

// 	cli();
// 	rtc_interrupt_count = 0;
// 	sti();
// 	uint32_t ticks = seconds * RTC_PRGM_MAX_FREQ;
// 	while (ticks) {
// 		rtc_read(0,0,0);
// 		ticks--;
// 	}

// }
