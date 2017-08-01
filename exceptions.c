#include "asmhelpers.h"
#include "memlib.h"
#include "kernel.h"
#include "debug.h"
#include "virtualmem.h"


void exception_handler(u32 exc, u32 errcode,
               u32 edi, u32 esi, u32 ebp, u32 esp,
               u32 ebx, u32 edx, u32 ecx, u32 new_eax,
               u32 eax, u32 eip, u32 cs, u32 eflags)
{
    struct exc_registers *regs = (struct exc_registers *) &errcode;

    switch (exc) {
        case 0: // divide-by-zero
        case 1:  // debug
        case 2:  // nmi
        case 3:  // breakpoint
        case 4:  // overflow
        case 5:  // bounds
        case 6:  // invalid opcode
        case 7:  // device n/a
        case 8:  // double fault
        case 9:  // coproc
        case 10: // invalid tss
        case 11: // segment not present
        case 12: // stack-segment
        case 13: // GPF
        case 14: // page fault
        {
          handle_page_fault();
          break;
        }

        case 16: // x87 fpe
        case 17: // alignment
        case 18: // machine check
        case 19: // simd fpe
        default:
            {
                kprintf("\n\nKERNEL PANIC! This might help: ");
                dump_regs(regs);
                while (1) halt();
            }
    };
}

