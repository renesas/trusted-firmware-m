/* generated vector header file - do not edit */
#ifndef VECTOR_DATA_H
#define VECTOR_DATA_H

/* Number of interrupts allocated */
#ifndef VECTOR_DATA_IRQ_COUNT
 #define VECTOR_DATA_IRQ_COUNT    (0)
#endif

/* ISR prototypes */

/* Vector table allocations */
typedef enum IRQn
{
    Reset_IRQn            = -15,
    NonMaskableInt_IRQn         = -14,  /* Non Maskable Interrupt */
    HardFault_IRQn              = -13,  /* HardFault Interrupt */
    MemoryManagement_IRQn       = -12,  /* Memory Management Interrupt */
    BusFault_IRQn               = -11,  /* Bus Fault Interrupt */
    UsageFault_IRQn             = -10,  /* Usage Fault Interrupt */
    SecureFault_IRQn            = -9,   /* Secure Fault Interrupt */
    SVCall_IRQn                 = -5,   /* SV Call Interrupt */
    DebugMonitor_IRQn           = -4,   /* Debug Monitor Interrupt */
    PendSV_IRQn                 = -2,   /* Pend SV Interrupt */
    SysTick_IRQn                = -1,   /* System Tick Interrupt */
	SCI7_RXI_IRQn = 0, /* SCI7 RXI (Received data full) */
	SCI7_TXI_IRQn = 1, /* SCI7 TXI (Transmit data empty) */
	SCI7_TEI_IRQn = 2, /* SCI7 TEI (Transmit end) */
	SCI7_ERI_IRQn = 3, /* SCI7 ERI (Receive error) */
	SCI0_RXI_IRQn = 4, /* SCI0 RXI (Received data full) */
	SCI0_TXI_IRQn = 5, /* SCI0 TXI (Transmit data empty) */
	SCI0_TEI_IRQn = 6, /* SCI0 TEI (Transmit end) */
	SCI0_ERI_IRQn = 7, /* SCI0 ERI (Receive error) */
} IRQn_Type;
#endif                                 /* VECTOR_DATA_H */
