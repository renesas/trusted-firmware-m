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
} IRQn_Type;
#endif                                 /* VECTOR_DATA_H */
