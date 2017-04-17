#include "kernel.h"
#include "asmhelpers.h"

void
dump_regs(const struct exc_registers *regs)
{
    DPRINT(0, "eax=%08X  ebx=%08X  ecx=%08X  edx=%08X",
              regs->eax, regs->ebx, regs->ecx, regs->edx);
    DPRINT(0, "esi=%08X  edi=%08X  ebp=%08X  esp=%08X",
              regs->esi, regs->edi, regs->ebp, regs->esp);
    DPRINT(0, "eflags=%08X  CS:EIP=%02X:%08X",
              regs->eflags, regs->cs, regs->eip);


    void **ebp = (void **) regs->ebp;

    DPRINT(0, "[%08X] eip=%08X", ebp, ebp[-1]);
    ebp = (void **) ebp[1]; // why do we need to skip a frame like this??

    while ((u32) ebp > 0x1000 && (u32) ebp < 0x7000)
    {
        DPRINT(0, "[%08X] eip=%08X", ebp, ebp[1]);
        ebp = (void **) ebp[0];
    }
}
