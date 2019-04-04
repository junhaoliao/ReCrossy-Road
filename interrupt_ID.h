/* This file provides interrupt IDs */

/* FPGA interrupts (there are 64 in total; only a few are defined below) */
#define	INTERVAL_TIMER_IRQ				72
#define	KEYS_IRQ		 						73
#define	FPGA_IRQ2	 						74
#define	FPGA_IRQ3	 						75
#define	FPGA_IRQ4	 						76
#define	FPGA_IRQ5	 						77
#define	AUDIO_IRQ							78
#define	PS2_IRQ		 						79
#define	JTAG_IRQ		 						80
#define	IrDA_IRQ		 						81
#define	FPGA_IRQ10							82
#define	JP1_IRQ								83
#define	JP2_IRQ								84
#define	FPGA_IRQ13							85
#define	FPGA_IRQ14							86
#define	FPGA_IRQ15							87
#define	FPGA_IRQ16							88
#define	PS2_DUAL_IRQ						89
#define	FPGA_IRQ18							90
#define	FPGA_IRQ19							91

/* ARM A9 MPCORE devices (there are many; only a few are defined below) */
#define	MPCORE_GLOBAL_TIMER_IRQ			27
#define	MPCORE_PRIV_TIMER_IRQ			29
#define	MPCORE_WATCHDOG_IRQ				30

/* HPS devices (there are many; only a few are defined below) */
#define	HPS_UART0_IRQ	   				194
#define	HPS_UART1_IRQ	   				195
#define	HPS_GPIO0_IRQ	        		  	196
#define	HPS_GPIO1_IRQ	        		  	197
#define	HPS_GPIO2_IRQ	        		  	198
#define	HPS_TIMER0_IRQ	        		 	199
#define	HPS_TIMER1_IRQ	        		 	200
#define	HPS_TIMER2_IRQ	        		 	201
#define	HPS_TIMER3_IRQ	        		 	202
#define	HPS_WATCHDOG0_IRQ	     			203
#define	HPS_WATCHDOG1_IRQ	     			204