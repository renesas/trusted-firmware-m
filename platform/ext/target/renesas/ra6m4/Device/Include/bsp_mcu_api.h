/* ${REA_DISCLAIMER_PLACEHOLDER} */

#ifndef BSP_MCU_API_H
#define BSP_MCU_API_H

/** Common macro for FSP header files. There is also a corresponding FSP_FOOTER macro at the end of this file. */
FSP_HEADER

void      R_BSP_RegisterProtectEnable(bsp_reg_protect_t regs_to_protect);
void      R_BSP_RegisterProtectDisable(bsp_reg_protect_t regs_to_unprotect);
fsp_err_t R_BSP_VersionGet(fsp_version_t * p_version);

/** Common macro for FSP header files. There is also a corresponding FSP_HEADER macro at the top of this file. */
FSP_FOOTER

#endif
