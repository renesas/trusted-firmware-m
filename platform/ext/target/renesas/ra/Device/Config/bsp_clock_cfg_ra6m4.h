/* ${REA_DISCLAIMER_PLACEHOLDER} */

/*******************************************************************************************************************//**
 * @ingroup BSP_CONFIG_DK9M
 * @defgroup BSP_CONFIG_DK9M_CLOCKS Build Time Configurations - Clock
 *
 * This file contains build-time clock configuration options. The BSP will use these macros to setup the MCU's clocks
 * for the user before main() is executed.
 *
 * Clock configuration options.
 * The input clock frequency is specified and then the system clocks are set by specifying the multipliers used. The
 * multiplier settings are used to set the clock registers. An example is shown below for a board with a 24MHz
 * XTAL and CPU clock of 240MHz: (NOTE that this is just an example. The RA6M4 has a mximum ICLK of 120 MHz).
 *
 * <PRE>
 * BSP_CFG_XTAL_HZ = 24000000
 * BSP_CFG_PLL_DIV = 1  (no division)
 * BSP_CFG_PLL_MUL = 10 (24MHz x 10 = 240MHz)
 *
 * BSP_CFG_ICLK_DIV =  1      : System Clock (ICLK)        =
 *                             (((BSP_CFG_XTAL_HZ/BSP_CFG_PLL_DIV) * BSP_CFG_PLL_MUL) / BSP_CFG_ICLK_DIV)  = 240MHz
 * BSP_CFG_PCLKA_DIV = 2      : Peripheral Clock A (PCLKA) =
 *                             (((BSP_CFG_XTAL_HZ/BSP_CFG_PLL_DIV) * BSP_CFG_PLL_MUL) / BSP_CFG_PCLKA_DIV) = 120MHz
 * BSP_CFG_PCLKB_DIV = 4      : Peripheral Clock B (PCLKB) =
 *                             (((BSP_CFG_XTAL_HZ/BSP_CFG_PLL_DIV) * BSP_CFG_PLL_MUL) / BSP_CFG_PCLKB_DIV) = 60MHz
 * BSP_CFG_PCLKC_DIV = 4      : Peripheral Clock C (PCLKC) =
 *                             (((BSP_CFG_XTAL_HZ/BSP_CFG_PLL_DIV) * BSP_CFG_PLL_MUL) / BSP_CFG_PCLKC_DIV) = 60MHz
 * BSP_CFG_PCLKD_DIV = 2      : Peripheral Clock D (PCLKD) =
 *                             (((BSP_CFG_XTAL_HZ/BSP_CFG_PLL_DIV) * BSP_CFG_PLL_MUL) / BSP_CFG_PCLKD_DIV) = 120MHz
 * BSP_CFG_FCLK_DIV =  4      : Flash IF Clock (FCLK)      =
 *                             (((BSP_CFG_XTAL_HZ/BSP_CFG_PLL_DIV) * BSP_CFG_PLL_MUL) / BSP_CFG_FCLK_DIV)  = 60MHz
 * BSP_CFG_BCLK_DIV =  2      : External Bus Clock (BCK)   =
 *                             (((BSP_CFG_XTAL_HZ/BSP_CFG_PLL_DIV) * BSP_CFG_PLL_MUL) / BSP_CFG_BCLK_DIV)  = 120MHz
 * BSP_CFG_UCK_DIV =  5      : USB Clock (UCLK)           =
 *                             (((BSP_CFG_XTAL_HZ/BSP_CFG_PLL_DIV) * BSP_CFG_PLL_MUL) / BSP_CFG_UCK_DIV)  = 48MHz
 * </PRE>
 *
 * @{
 ***********************************************************************************************************************/

#ifndef BSP_CLOCK_CFG_RA6M4_H
#define BSP_CLOCK_CFG_RA6M4_H

/***********************************************************************************************************************
 * Configuration Options
 ***********************************************************************************************************************/

/* Still not sure why the RA6M4 takes more cycles than other MCUs. */ // ask hw team about this, and test it out more
#define BSP_DELAY_LOOP_CYCLES      (9U)

/** Clock source select (CKSEL). The chosen clock will be the base clock provided for the system clock and all
 * peripheral clocks. It is also used for the flash clock and the external bus clocks.
 *
 * Set the macro to one of the enumerations shown.
 *
 * <PRE>
 * Clock                                        -   Enumeration to use for macro
 * High Speed On-Chip Oscillator   (HOCO)       -   BSP_CLOCKS_SOURCE_CLOCK_HOCO
 * Middle Speed On-Chip Oscillator (MOCO)       -   BSP_CLOCKS_SOURCE_CLOCK_MOCO
 * Low Speed On-Chip Oscillator    (LOCO)       -   BSP_CLOCKS_SOURCE_CLOCK_LOCO
 * Main Clock Oscillator                        -   BSP_CLOCKS_SOURCE_CLOCK_MAIN_OSC
 * Sub-Clock Oscillator                         -   BSP_CLOCKS_SOURCE_CLOCK_SUBCLOCK
 * PLL Circuit                                  -   BSP_CLOCKS_SOURCE_CLOCK_PLL
 * </PRE>
 */
#ifndef BSP_CFG_CLOCK_SOURCE
 #define BSP_CFG_CLOCK_SOURCE      (BSP_CLOCKS_SOURCE_CLOCK_PLL)
#endif

/** XTAL - Input clock frequency in Hz */
#ifndef BSP_CFG_XTAL_HZ
 #define BSP_CFG_XTAL_HZ           (24000000)
#endif

/** The HOCO can operate at several different frequencies. Choose which one using the macro below. The frequency
 * used out of reset depends upon the OFS1.HOCOFRQ0 bits.
 *
 * <PRE>
 * Available frequency settings:
 * 0 = 16MHz
 * 1 = 18MHz
 * 2 = 20MHz
 * </PRE>
 */
#ifndef BSP_CFG_HOCO_FREQUENCY
 #define BSP_CFG_HOCO_FREQUENCY    (0)
#endif

/** PLL clock source (PLLSRCEL). Choose which clock source to input to the PLL circuit.
 *
 * <PRE>
 * Available clock sources:     -   Enumeration to use for macro
 * 0 = Main clock (default)     -   BSP_CLOCKS_SOURCE_CLOCK_MAIN_OSC
 * 1 = HOCO                     -   BSP_CLOCKS_SOURCE_CLOCK_HOCO
 * </PRE>
 */
#ifndef BSP_CFG_PLL_SOURCE
 #define BSP_CFG_PLL_SOURCE        (BSP_CLOCKS_SOURCE_CLOCK_MAIN_OSC)
#endif

/** PLL Input Frequency Division Ratio Select (PLIDIV).
 *
 * Available divisors = - /1 (no division), - /2, - /3
 *
 * @note Set macro definition to 'BSP_CLOCKS_PLL_DIV_' + your divider selection.
 */
#ifndef BSP_CFG_PLL_DIV
 #define BSP_CFG_PLL_DIV           (BSP_CLOCKS_PLL_DIV_3)
#endif

/** PLL Frequency Multiplication Factor Select (PLLMUL).
 *
 * Available multipliers = x10.0 to x30.0 in 0.5 increments (e.g. 10.0, 10.5, 11.0, 11.5, ..., 29.0, 29.5, 30.0)
 */
#ifndef BSP_CFG_PLL_MUL
 #define BSP_CFG_PLL_MUL           (BSP_CLOCKS_PLL_MUL_25_0)
#endif

/** System Clock Divider (ICK).
 *
 * Available divisors = /1 (no division), /2, /4, /8, /16, /32, /64
 *
 * @note Set macro definition to 'BSP_CLOCKS_SYS_CLOCK_DIV_' + your divider selection.
 */
#ifndef BSP_CFG_ICLK_DIV
 #define BSP_CFG_ICLK_DIV          (BSP_CLOCKS_SYS_CLOCK_DIV_1)
#endif

/** Peripheral Module Clock A Divider (PCKA).
 *
 * Available divisors = /1 (no division), /2, /4, /8, /16, /32, /64
 *
 * @note Set macro definition to 'BSP_CLOCKS_SYS_CLOCK_DIV_' + your divider selection.
 */
#ifndef BSP_CFG_PCLKA_DIV              // PCLKH
 #define BSP_CFG_PCLKA_DIV         (BSP_CLOCKS_SYS_CLOCK_DIV_2)
#endif

/** Peripheral Module Clock B Divider (PCKB).
 *
 * Available divisors = /1 (no division), /2, /4, /8, /16, /32, /64
 *
 * @note Set macro definition to 'BSP_CLOCKS_SYS_CLOCK_DIV_' + your divider selection.
 */
#ifndef BSP_CFG_PCLKB_DIV              // PCLKL
 #define BSP_CFG_PCLKB_DIV         (BSP_CLOCKS_SYS_CLOCK_DIV_4)
#endif

/** Peripheral Module Clock C Divider (PCKC).
 *
 * Available divisors = /1 (no division), /2, /4, /8, /16, /32, /64
 *
 * @note Set macro definition to 'BSP_CLOCKS_SYS_CLOCK_DIV_' + your divider selection.
 */
#ifndef BSP_CFG_PCLKC_DIV              // PCLKADC
 #define BSP_CFG_PCLKC_DIV         (BSP_CLOCKS_SYS_CLOCK_DIV_4)
#endif

/** Peripheral Module Clock D Divider (PCKD).
 *
 * Available divisors = /1 (no division), /2, /4, /8, /16, /32, /64
 *
 * @note Set macro definition to 'BSP_CLOCKS_SYS_CLOCK_DIV_' + your divider selection.
 */
#ifndef BSP_CFG_PCLKD_DIV              // PCLKGPT
 #define BSP_CFG_PCLKD_DIV         (BSP_CLOCKS_SYS_CLOCK_DIV_2)
#endif

/** External Bus Clock Divider (BCLK).
 *
 * Available divisors = /1 (no division), /2, /4, /8, /16, /32, /64
 *
 * @note Set macro definition to 'BSP_CLOCKS_SYS_CLOCK_DIV_' + your divider selection.
 */
#ifndef BSP_CFG_BCLK_DIV
 #define BSP_CFG_BCLK_DIV          (BSP_CLOCKS_SYS_CLOCK_DIV_4)
#endif

/** Flash IF Clock Divider (FCK).
 *
 * Available divisors = /1 (no division), /2, /4, /8, /16, /32, /64
 *
 * @note Set macro definition to 'BSP_CLOCKS_SYS_CLOCK_DIV_' + your divider selection.
 */
#ifndef BSP_CFG_FCLK_DIV
 #define BSP_CFG_FCLK_DIV          (BSP_CLOCKS_SYS_CLOCK_DIV_4)
#endif

/** Configure BCLK output pin
 *
 * Available options:
 * - 0 = No output
 * - 1 = BCK frequency
 * - 2 = BCK/2 frequency
 *
 * @note This macro is only effective when the external bus is enabled
 */
#ifndef BSP_CFG_BCLK_OUTPUT
 #define BSP_CFG_BCLK_OUTPUT       (2)
#endif

/** PLL2 clock source (PLLSRCEL). Choose which clock source to input to the PLL2 circuit.
 *
 * <PRE>
 * Available clock sources:     -   Enumeration to use for macro
 * 0 = Main clock (default)     -   BSP_CLOCKS_SOURCE_CLOCK_MAIN_OSC
 * 1 = HOCO                     -   BSP_CLOCKS_SOURCE_CLOCK_HOCO
 * </PRE>
 */
#ifndef BSP_CFG_PLL2_SOURCE
 #define BSP_CFG_PLL2_SOURCE       (BSP_CLOCKS_CLOCK_DISABLED)
#endif

/** PLL2 Input Frequency Division Ratio Select (PLIDIV).
 *
 * Available divisors = - /1 (no division), - /2, - /3
 *
 * @note Set macro definition to 'BSP_CLOCKS_PLL_DIV_' + your divider selection.
 */
#ifndef BSP_CFG_PLL2_DIV
 #define BSP_CFG_PLL2_DIV          (BSP_CLOCKS_PLL_DIV_2)
#endif

/** PLL2 Frequency Multiplication Factor Select (PLLMUL).
 *
 * Available multipliers = x10.0 to x30.0 in 0.5 increments (e.g. 10.0, 10.5, 11.0, 11.5, ..., 29.0, 29.5, 30.0)
 */
#ifndef BSP_CFG_PLL2_MUL
 #define BSP_CFG_PLL2_MUL          (BSP_CLOCKS_PLL_MUL_16_0)
#endif

/**
 * The USB Clock source that is configured during startup.
 * Available clock sources:
 *   - BSP_CLOCKS_SOURCE_CLOCK_PLL
 *   - BSP_CLOCKS_SOURCE_CLOCK_PLL2
 */
#ifndef BSP_CFG_UCK_SOURCE
 #define BSP_CFG_UCK_SOURCE        (BSP_CLOCKS_CLOCK_DISABLED)
#endif

/** USB Clock Divider Select.
 *
 * Available divisors = /3, /4, /5
 *
 * @note Set macro definition to 'BSP_CLOCKS_USB_CLOCK_DIV_' + your divider selection.
 */
#ifndef BSP_CFG_UCK_DIV
 #define BSP_CFG_UCK_DIV           (BSP_CLOCKS_USB_CLOCK_DIV_4)
#endif

/**
 * The OCTA Clock source that is configured during startup.
 */
#ifndef BSP_CFG_OCTA_SOURCE
 #define BSP_CFG_OCTA_SOURCE       (BSP_CLOCKS_CLOCK_DISABLED)
#endif

/** OCTA Clock Divider Select.
 *
 * Available divisors = /1, /2, /4, /6, /8
 *
 * @note Set macro definition to 'BSP_CLOCKS_OCTA_CLOCK_DIV_' + your divider selection.
 */
#ifndef BSP_CFG_OCTA_DIV
 #define BSP_CFG_OCTA_DIV          (BSP_CLOCKS_OCTA_CLOCK_DIV_1)
#endif

/** @} (end defgroup BSP_CONFIG_CLOCKS) */

#endif
