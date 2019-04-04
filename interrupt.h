//
// Created by junha on 4/3/2019.
//

#ifndef REFURBSOUP_INTERRUPT_H
#define REFURBSOUP_INTERRUPT_H

#include "interrupt_ID.h"
#include "defines.h"
#include "address_map_arm.h"

extern volatile int key_dir;
extern volatile int pattern;



extern volatile int *LEDR_ptr;
extern volatile int *SW_ptr;
extern volatile int *KEY_EDGE_ptr;
extern volatile char *character_buffer;
extern volatile int *pixel_ctrl_ptr;

extern bool KEYBOARD_UP;
extern bool KEYBOARD_DOWN;
extern bool KEYBOARD_LEFT;
extern bool KEYBOARD_RIGHT;
extern bool KEYBOARD_RESTART;

unsigned char byte1 = 0;
unsigned char byte2 = 0;
unsigned char byte3 = 0;

void config_interrupt (int, int);
void hw_write_bits(volatile int *, volatile int, volatile int);

void set_A9_IRQ_stack(void);

void enable_A9_interrupts(void);

void config_GIC(void);

void config_KEYs();

void config_PS2();
void pushbutton_ISR(void);

void PS2_ISR();

// Define the IRQ exception handler
void __attribute__ ((interrupt)) __cs3_isr_irq (void)
{
    // Read the ICCIAR from the processor interface
    int address = MPCORE_GIC_CPUIF + ICCIAR;
    int int_ID = *((int *) address);

    if (int_ID == PS2_IRQ)				// check if interrupt is from the PS/2
        PS2_ISR ();
    else
        while (1);									// if unexpected, then halt

    // Write to the End of Interrupt Register (ICCEOIR)
    address = MPCORE_GIC_CPUIF + ICCEOIR;
    *((int *) address) = int_ID;

    return;
}

// Define the remaining exception handlers
void __attribute__((interrupt)) __cs3_reset(void) {
    while (1);
}

void __attribute__((interrupt)) __cs3_isr_undef(void) {
    while (1);
}

void __attribute__((interrupt)) __cs3_isr_swi(void) {
    while (1);
}

void __attribute__((interrupt)) __cs3_isr_pabort(void) {
    while (1);
}

void __attribute__((interrupt)) __cs3_isr_dabort(void) {
    while (1);
}

void __attribute__((interrupt)) __cs3_isr_fiq(void) {
    while (1);
}

/*
 * Initialize the banked stack pointer register for IRQ mode
*/

void set_A9_IRQ_stack(void)
{
    int stack, mode;
    stack = A9_ONCHIP_END - 7;		// top of A9 onchip memory, aligned to 8 bytes
    /* change processor to IRQ mode with interrupts disabled */
    mode = INT_DISABLE | IRQ_MODE;
    asm("msr cpsr, %[ps]" : : [ps] "r" (mode));
    /* set banked stack pointer */
    asm("mov sp, %[ps]" : : [ps] "r" (stack));

    /* go back to SVC mode before executing subroutine return! */
    mode = INT_DISABLE | SVC_MODE;
    asm("msr cpsr, %[ps]" : : [ps] "r" (mode));
}

/*
 * Turn on interrupts in the ARM processor
*/
void enable_A9_interrupts(void)
{
    int status = SVC_MODE | INT_ENABLE;
    asm("msr cpsr,%[ps]" : : [ps]"r"(status));
}

/*
 * Configure the Generic Interrupt Controller (GIC)
*/
void config_GIC(void)
{
    int address;	// used to calculate register addresses

    /* enable some examples of interrupts */
    config_interrupt (PS2_IRQ, CPU0);

    // Set Interrupt Priority Mask Register (ICCPMR). Enable interrupts for lowest priority
    address = MPCORE_GIC_CPUIF + ICCPMR;
    *((int *) address) = 0xFFFF;

    // Set CPU Interface Control Register (ICCICR). Enable signaling of interrupts
    address = MPCORE_GIC_CPUIF + ICCICR;
    *((int *) address) = ENABLE;

    // Configure the Distributor Control Register (ICDDCR) to send pending interrupts to CPUs
    address = MPCORE_GIC_DIST + ICDDCR;
    *((int *) address) = ENABLE;
}

/* setup the KEY interrupts in the FPGA */
void config_KEYs() {
    volatile int *KEY_ptr = (int *) KEY_BASE; // pushbutton KEY address
    *(KEY_ptr + 2) = 0x3;                    // enable interrupts for KEY[1]
}
/* setup the PS/2 interrupts */
void config_PS2() {
    volatile int * PS2_ptr = (int *)PS2_BASE; // PS/2 port address

    *(PS2_ptr) = 0xFF; /* reset */
    *(PS2_ptr + 1) =
            0x1; /* write to the PS/2 Control register to enable interrupts */
}
void pushbutton_ISR(void) {
    volatile int *KEY_ptr = (int *) KEY_BASE;
    int press;
    press = *(KEY_ptr + 3); // read the pushbutton interrupt register
    *(KEY_ptr + 3) = press; // Clear the interrupt
    key_dir ^= 1; // Toggle key_dir value

    return;
}


void PS2_ISR(void) {
    volatile int * PS2_ptr = (int *) 0xFF200100;		// PS/2 port address
    int PS2_data, RAVAIL;

    PS2_data = *(PS2_ptr);									// read the Data register in the PS/2 port
    RAVAIL = (PS2_data & 0xFFFF0000) >> 16;			// extract the RAVAIL field
    if (RAVAIL > 0)
    {
        /* always save the last three bytes received */
        byte1 = byte2;
        byte2 = byte3;
        byte3 = PS2_data & 0xFF;
        if ((byte2 == (char) 0xE0) && (byte3 == (char) 0x75)) {
            KEYBOARD_UP = true;
        } else if((byte2 == (char) 0xE0) && (byte3 == (char) 0x72)){
            KEYBOARD_DOWN = true;
        } else if((byte2 == (char) 0xE0) && (byte3 == (char) 0x6B)){
            KEYBOARD_LEFT = true;
        } else if((byte2 == (char) 0xE0) && (byte3 == (char) 0x74)){
            KEYBOARD_RIGHT = true;
        } else if((byte2 == (char) 0xE0) && (byte3 == (char) 0x69)){
            KEYBOARD_RESTART = true;
        }
    }
}

/*
 * Configure registers in the GIC for individual interrupt IDs.
*/
void config_interrupt (int int_ID, int CPU_target)
{
    int n, addr_offset, value, address;
    /* Set Interrupt Processor Targets Register (ICDIPTRn) to cpu0.
     * n = integer_div(int_ID / 4) * 4
     * addr_offet = #ICDIPTR + n
     * value = CPU_target << ((int_ID & 0x3) * 8)
     */
    n = (int_ID >> 2) << 2;
    addr_offset = ICDIPTR + n;
    value = CPU_target << ((int_ID & 0x3) << 3);

    /* Now that we know the register address and value, we need to set the correct bits in
     * the GIC register, without changing the other bits */
    address = MPCORE_GIC_DIST + addr_offset;
    hw_write_bits((int *) address, 0xff << ((int_ID & 0x3) << 3), value);

    /* Set Interrupt Set-Enable Registers (ICDISERn).
     * n = (integer_div(in_ID / 32) * 4
     * addr_offset = 0x100 + n
     * value = enable << (int_ID & 0x1F) */
    n = (int_ID >> 5) << 2;
    addr_offset = ICDISER + n;
    value = 0x1 << (int_ID & 0x1f);
    /* Now that we know the register address and value, we need to set the correct bits in
     * the GIC register, without changing the other bits */
    address = MPCORE_GIC_DIST + addr_offset;
    hw_write_bits((int *) address, 0x1 << (int_ID & 0x1f), value);
}

void hw_write_bits(volatile int * addr, volatile int unmask, volatile int value)
{
    *addr = ((~unmask) & *addr) | value;
}
#endif //REFURBSOUP_INTERRUPT_H
