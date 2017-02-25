/* rtc.h - Defines for useful RTC functions
 * vim:ts=4 noexpandtab
 */

#ifndef _RTC_H
#define _RTC_H

#include "pcb.h"
#include "types.h"

/*Ports that the RTC is on*/
#define RTC_PORT	0x70
#define PORT_DATA	0x71

/*Disable/Enable NMI when reading and writing to the registers on RTC*/
#define REGB_NONMI	0x8B
#define NMI_ON		0x7F

#define REGA        0x8A
#define REGC        0x0C

/*Enable PIE bit on register B to turn on RTC interrupts*/
#define RTC_ENABLE	0x40

/*The interrupt line to enable on the PIC*/
#define IR_LINE	8

/* The default frequency of the rtc chip */
#define RTC_DEFAULT_FREQ 32768

/* The max frequency user programs can use */
#define RTC_PRGM_MAX_FREQ 1024

/* lower and upper bounds for valid rtc rates */
/* RTC_PRGM_MAX_FREQ hz */
#define RTC_RATE_LOW 6
/* 2 hz */
#define RTC_RATE_HIGH 15

/* Externally-visible functions */

/*Initialize the RTC*/
void rtc_init(void);
/*RTC interrupt handler*/
void rtc_handler(void);

int32_t rtc_read (int32_t fd, void* buf, int32_t nbytes);
int32_t rtc_write (int32_t fd, const void* buf, int32_t nbytes);
int32_t rtc_open (const uint8_t * filename);
int32_t rtc_close (int32_t fd);

void rtc_set_freq (uint32_t rate);
void rtc_wait (uint32_t seconds, uint32_t print);

#endif /* _RTC_H_ */
