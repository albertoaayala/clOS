#ifndef _PIT_H_
#define _PIT_H_

#include "types.h"

/* IRQ 0 */
#define PIT_IRQ_LINE 0

/* relevant frequencies */
#define PIT_FREQ_MAX 1193182
#define PIT_FREQ_MIN (PIT_FREQ_MAX / 65536.0)
/* set this frequency using pit_set_freq for scheduling */
#define PIT_FREQ_SCHEDULE 25

/* I/O ports */
#define PIT_CH0_PORT 0x40
#define PIT_CH1_PORT 0x41
#define PIT_CH2_PORT 0x42
#define PIT_CMD_PORT 0x43

/* magic number for setting rate in pit_set_freq */
#define PIT_SET_RATE_CMD 0x36

void pit_init();
void pit_handler();

void schedule();

int32_t pit_set_freq(uint8_t channel, uint32_t freq);

#endif /* _PIT_H_ */
