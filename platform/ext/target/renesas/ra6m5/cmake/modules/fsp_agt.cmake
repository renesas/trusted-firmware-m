# FSP AGT Module Library
# Asynchronous General-purpose Timer. Needed by the secure image ONLY when the FLIH/SLIH
# interrupt test suites are built: they raise a real secure interrupt, and on RA that means
# a timer the SECURE e2 project owns (DECISIONS D061, D068).
#
# Adding a module: fsp_cmake/TFM_INTEGRATION_COMPLETE.md, "Adding New FSP Modules".
#
# TWO THINGS DEPEND ON THIS BEING LINKED, and the second one is not obvious:
#
#   1. plat_test.c calls R_AGT_Open()/R_AGT_Start()/R_AGT_Stop() to drive the test timer.
#      An earlier revision poked the AGT registers directly to avoid the driver; that is
#      how the AGTMR2-before-AGTMR1 ordering and the compare-match parking got missed.
#
#   2. agt_int_isr(). The generated ra_gen/vector_data.c names it in g_vector_table, and
#      that file has to be linked for its OTHER symbol - the ICU event-link table that
#      bsp_irq_cfg() reads (see target_cfg.c). So pulling in vector_data.o pulls in a
#      reference to FSP's AGT ISR, and without this module the link fails with
#      "Error[Li005]: no definition for agt_int_isr".
#
#      The secure image never DISPATCHES through g_vector_table - TF-M owns the vector
#      table (__VECTOR_TABLE in startup_ra6m5.c, slot 16 -> TFM_TIMER0_IRQ_Handler) - so
#      agt_int_isr() is linked and never called. That is intended, not waste: it is the
#      cost of keeping the generated event-link table authoritative instead of
#      transcribing IELSR values into the port.

fsp_module_library(FSP_AGT_TARGET agt)

fsp_module_glob(_src "ra/fsp/src/r_agt")
if(NOT _src)
    message(FATAL_ERROR
        "RA6M5: the AGT module is not in ${FSP_MODULE_BASE_DIR}.\n"
        "The FLIH/SLIH interrupt suites need a secure timer. Add the Timer (r_agt) module "
        "to the SECURE project in e2 studio with a real Interrupt Priority - not Disabled, "
        "which is also what makes the interrupt secure on RA - regenerate, and build the "
        "project once. See DECISIONS D061.")
endif()

target_sources(${FSP_AGT_TARGET} PRIVATE ${_src})

target_link_libraries(${FSP_AGT_TARGET} PUBLIC fsp_bsp_${FSP_MODULE_ROLE})
