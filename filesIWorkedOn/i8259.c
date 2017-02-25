/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts
 * are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7 */
uint8_t slave_mask; /* IRQs 8-15 */


/*
 * void i8259_init(void)
 *   Inputs: void
 *   Return Value: void
 *	Function: Initializes the 8259 PIC
 */
void
i8259_init(void)
{
	/*first create a variable to hold the flags then lock by using the given ways to stops interrupts*/
	uint32_t flags;
	cli_and_save(flags);

	/*mask all interrupts by setting all bit in IMR for both master and slave PIC*/
	outb(DISABLE, MASTER_PORT_DATA);
	outb(DISABLE, SLAVE_PORT_DATA);

	/*send the ICWs to the PIC for init*/
	//send first word
	outb(ICW1, MASTER_8259_PORT);
	outb(ICW1, SLAVE_8259_PORT);
	//send secondword
	outb(ICW2_MASTER, MASTER_PORT_DATA);
	outb(ICW2_SLAVE, SLAVE_PORT_DATA);
	//send third word
	outb(ICW3_MASTER, MASTER_PORT_DATA);
	outb(ICW3_SLAVE, SLAVE_PORT_DATA);
	//send 4th word
	outb(ICW4, MASTER_PORT_DATA);
	outb(ICW4, SLAVE_PORT_DATA);

	/*mask all interrupts by setting all bit in IMR for both master and slave PIC*/
	outb(DISABLE, MASTER_PORT_DATA);
	outb(DISABLE, SLAVE_PORT_DATA);	

	//sti();
	restore_flags(flags);
	
}

/*
 * void enable_irq(uint32_t irq_num)
 *   Inputs: IRQ line to enable (unmask)
 *   Return Value: void
 *	Function: Enable (unmask) the specified IRQ
 */
void
enable_irq(uint32_t irq_num)
{
	uint8_t tmp;
	uint8_t unmask;
	if((irq_num >= 8)&&(irq_num < 16)) {		//check if irq_num is within the bounds of 0-15 (Master: lines 0-7, Slave: lines 8-15)
		irq_num -= 8;							//get the correct IRQ on the IMR for the slave PIC
		tmp = 1<<irq_num;						//left shift the amount needed to make to correct bit unmask the IRQ on the IMR
		tmp = ~tmp;								//must do this to make that bit zero and the rest one to not change the rest of the IMR
		unmask = inb(SLAVE_PORT_DATA)&tmp;		//get original IMR from slave and bitwise AND with the bit we want to enable
		outb(unmask, SLAVE_PORT_DATA);
	}
	else if((irq_num >= 0)&&(irq_num < 8)){		//make sure the irq_num is within the bounds
		/*do not need to subtract here since it is already between 0-7 also same steps as above*/
		tmp = 1<<irq_num;
		tmp = ~tmp;
		unmask = inb(MASTER_PORT_DATA)&tmp;		//same but for master PIC
		outb(unmask, MASTER_PORT_DATA);
	}
}

/*
 * void disable_irq(uint32_t irq_num)
 *   Inputs: IRQ line to disable (mask)
 *   Return Value: void
 *	Function: Disable (mask) the specified IRQ
 */
void
disable_irq(uint32_t irq_num)
{
	uint8_t mask;
	if((irq_num >= 8)&&(irq_num < 16)){				//check if irq_num is within the bounds of 0-15 (Master: lines 0-7, Slave: lines 8-15)
		irq_num -= 8;								//get correct IRQ num for slave
		mask = inb(SLAVE_PORT_DATA)|(1<<irq_num);	//get IMR currently on slave and bitwise OR with the bit we want to mask
		outb(mask, SLAVE_PORT_DATA);
	}
	else if((irq_num >= 0)&&(irq_num < 8)){			//make sure the irq_num is within the bounds
		/*Do not to change irq_num and everything here is the same as that in the first if statement*/
		mask = inb(MASTER_PORT_DATA)|(1<<irq_num);	//get IMR currently on master and bitwise OR with the bit we want to mask
		outb(mask, MASTER_PORT_DATA);
	}
}

/*
 * void send_eoi(uint32_t irq_num)
 *   Inputs: IRQ line to send EOI
 *   Return Value: void
 *	Function: Send end-of-interrupt signal for the specified IRQ
 */
void
send_eoi(uint32_t irq_num)
{
	uint8_t eoiSig;
	if((irq_num >= 0)&&(irq_num < 16)){	//check if irq_num is within the bounds of 0-15 (Master: lines 0-7, Slave: lines 8-15)
		if(irq_num >= 8){				// check if slave (lines 8-15)
			irq_num -= 8;				//get correct IRQ num for slave
			eoiSig = EOI|irq_num;
			outb(eoiSig, SLAVE_8259_PORT);
			eoiSig = EOI|2; // now send EOI to the master for IR 2
		} else {
			//have to send to both when in cascade mode
			eoiSig = EOI|irq_num;
		}

		outb(eoiSig, MASTER_8259_PORT);
	}
}
