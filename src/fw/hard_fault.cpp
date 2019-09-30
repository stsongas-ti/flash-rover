
#include <stdint.h>
#include <string.h>

#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(driverlib/interrupt.h)
#include DeviceFamily_constructPath(inc/hw_ints.h)
#include DeviceFamily_constructPath(inc/hw_cpu_scs.h)
#include DeviceFamily_constructPath(inc/hw_memmap.h)

#include "hard_fault.hpp"

static RegDump_t* pRegDump = nullptr;

extern "C" void debugHardfault(uint32_t *sp)
{
    if (pRegDump)
    {
        memcpy(pRegDump, (RegDump_t *)(sp), sizeof(RegDump_t));
    }

    for(;;);
}

static void hardFaultIsr(void)
{
#if defined(__GNUC__)
    __asm__ __volatile__
    (
        "tst lr, #4        \n"
        "ite eq            \n"
        "mrseq r0, msp     \n"
        "mrsne r0, psp     \n"
        "bx %0             \n"
            : /* output */
            : /* input */
            "r"(debugHardfault)
    );
#elif defined(__TI_COMPILER_VERSION__)
    __asm__ __volatile__
    (
        " tst lr, #4              \n"
        " ite eq                  \n"
        " mrseq r0, msp           \n"
        " mrsne r0, psp           \n"
        " bl debugHardfault \n"
    );
#endif
}

void openHardFaultDebugger(RegDump_t& regDump)
{
    memset(&regDump, 0, sizeof(RegDump_t));
    pRegDump = &regDump;

    HWREG(CPU_SCS_BASE + CPU_SCS_O_ACTLR) = CPU_SCS_ACTLR_DISDEFWBUF;

    IntRegister(INT_HARD_FAULT, hardFaultIsr);
}
