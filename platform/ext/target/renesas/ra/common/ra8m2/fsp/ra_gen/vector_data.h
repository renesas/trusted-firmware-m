/*
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef VECTOR_DATA_H
#define VECTOR_DATA_H

#ifdef __cplusplus
extern "C" {
#endif

void ipc_isr(void);
void agt_int_isr(void);

/* The number of entries required for the ICU vector table. */
#define BSP_ICU_VECTOR_NUM_ENTRIES (96)

#ifdef RA_NS_INTERFACE_MAILBOX
/* CPU1 NVIC/IELSR slot of the SPE-to-NSPE mailbox reply notification (IPC IRQ0). */
#define RA_NS_MAILBOX_IRQ    0
#endif

#define IRQ_HANDLER_NAME(n)  irq_##n##_handler

#define IRQ_HANDLER(num)           \
   void IRQ_HANDLER_NAME(num)(void)

#define WEAK_IRQ_HANDLER(num)              \
__WEAK IRQ_HANDLER(num)                    \
{                                          \
    BSP_CFG_HANDLE_UNRECOVERABLE_ERROR(0); \
}

#define IRQ_VECTOR_ENTRY(n)[n] = IRQ_HANDLER_NAME(n),

#define IRQ_FOREACH(X) \
	X(0)  X(1)  X(2)  X(3)  X(4)  X(5)  X(6)  X(7)  X(8)  X(9)  X(10) X(11) X(12) X(13) \
	X(14) X(15) X(16) X(17) X(18) X(19) X(20) X(21) X(22) X(23) X(24) X(25) X(26) X(27) \
	X(28) X(29) X(30) X(31) X(32) X(33) X(34) X(35) X(36) X(37) X(38) X(39) X(40) X(41) \
	X(42) X(43) X(44) X(45) X(46) X(47) X(48) X(49) X(50) X(51) X(52) X(53) X(54) X(55) \
	X(56) X(57) X(58) X(59) X(60) X(61) X(62) X(63) X(64) X(65) X(66) X(67) X(68) X(69) \
	X(70) X(71) X(72) X(73) X(74) X(75) X(76) X(77) X(78) X(79) X(80) X(81) X(82) X(83) \
	X(84) X(85) X(86) X(87) X(88) X(89) X(90) X(91) X(92) X(93) X(94) X(95)

#ifdef __cplusplus
}
#endif
#endif /* VECTOR_DATA_H */
