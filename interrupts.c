#include "asmhelpers.h"
#include "memlib.h"
#include "vgatext.h"
#include "kb.h"
#include "video.h"

#define NUM_INTERRUPTS 64

static char timer_chars[4] = {'-','/','|','\\'};
static int timer_index = 0;
static int img_number = 0;
static int pause_set =  0;

/*
 * IDT at 0x1000
 *    exceptions at INT 00h-1fh
 *          IRQs at INT 20h-2fh
 *      syscalls at INT 30h-3fh
 * stage 0 handlers at 0x1200 + MAX_S0_LEN*intnum
 */

void isr_timer(){
    char *pic_index = (char *) 0x8da0;

    if( timer_index % 5 == 0 && !pause_set) {
        show_image(pic_index+(img_number%15)*4000, 4000);
        img_number++;
        vga_setchar(79, 0, (char *) img_number+'0', 0x03);
    }
    /*vga_setchar(79, 0, timer_chars[timer_index%4], 0x03);*/
    timer_index ++;
}

void isr_keyboard() {
        u8 scancode = in8(0x60);
        u16 val;
        val = scancode_to_ascii(scancode);
        if(val == ' ') pause_set = 1-pause_set;

        if(val == 0) return;

        vga_putc(val, 0x07);
}

void irq_handler(u32 irq)
{
    /*vga_putc(irq + '0', 0x03);*/
//    vga_setchar(irq, 24, irq + '0', 0x03);
    switch (irq) {
        case 0: isr_timer(); break;
        case 1: isr_keyboard(); break;
    default:
//            kprintf("Unhandled IRQ%d\r\n", irq);
            break;
    };
}

#define MAX_S0_LEN 32

extern u32 irq_stage0_start, irq_stage0_fixup, irq_stage0_end;
extern u32 exc_stage0_start, exc_stage0_fixup, exc_stage0_end;
extern u32 excerr_stage0_start, excerr_stage0_fixup, excerr_stage0_end;
//extern u32 syscall_stage0_start, syscall_stage0_fixup, syscall_stage0_end;
extern u32 asm_halt;

static void
create_handler(u8 *h, void *start, void *fixup, void *end, int intnum)
{
    memcpy(h, (const void *) start, end - start);
    h[fixup - start + 1] = intnum;
}

static inline void
set_idt_entry(u32 *idtentry, void *handler_addr)
{
    idtentry[0] = 0x00080000 + (u32) handler_addr;
    idtentry[1] = 0x00008E00;
}

void
lidt(void *base, unsigned int limit)
{
   volatile u16 idtr[3];

   idtr[0] = limit;
   idtr[1] = (u32) base;
   idtr[2] = (u32) base >> 16;

   asm volatile ("lidt (%0)": :"r" (idtr));
}

void
create_idt(u32 *idt) // and also stage0 interrupt stubs after the IDT
{
    u8 *handler_addr = (u8 *) (idt + 2*NUM_INTERRUPTS);

    int i;
    for (i=0; i < NUM_INTERRUPTS; ++i)
    {
        if (i == 8) { // double fault
            set_idt_entry(&idt[i*2], (u8 *) asm_halt);
        } else {
            set_idt_entry(&idt[i*2], handler_addr);
        }

        if (i == 8 || i == 10 || i == 11 || i == 12 ||
                      i == 13 || i == 14 || i == 17)    // with errcode
        {
            create_handler(handler_addr, &excerr_stage0_start,
                                         &excerr_stage0_fixup,
                                         &excerr_stage0_end,
                                         i);
        }
        else if (i < 0x20) // exception without errcode
        {
            create_handler(handler_addr, &exc_stage0_start,
                                         &exc_stage0_fixup,
                                         &exc_stage0_end,
                                         i);
        }
        else if (i < 0x30) // irq
        {
            create_handler(handler_addr, &irq_stage0_start,
                                         &irq_stage0_fixup,
                                         &irq_stage0_end,
                                         i - 0x20);
        }
#if 0
        else // syscall
        {
            create_handler(handler_addr, &syscall_stage0_start,
                                         &syscall_stage0_fixup,
                                         &syscall_stage0_end,
                                         i - 0x30);
        }
#endif
        handler_addr += MAX_S0_LEN;
    }
}

void
setup_pic(u8 master_int, u8 slave_int)
{
    out8(0x20, 0x11);        // ICW1: init, expect ICW4
    out8(0x21, master_int);  // ICW2: base address (int#) for IRQ0
    out8(0x21, 0x04);        // ICW3: slave is on IRQ2
    out8(0x21, 0x01);        // ICW4: manual EOI
    out8(0x21, 0x0);         // OCW1: unmask all ints

    out8(0xA0, 0x11);        // ICW1: init, expect ICW4
    out8(0xA1, slave_int);   // ICW2: base address (int#) for IRQ8
    out8(0xA1, 0x02);        // ICW3: i am attached to IRQ2
    out8(0xA1, 0x01);        // ICW4: manual EOI
    out8(0xA1, 0x0);         // OCW1: unmask all ints
}

void
setup_interrupts(void *idtaddr)
{
    // set up 8259 PIC for hardware interrupts at 0x20/0x28
    setup_pic(0x20, 0x28);

    // create IDT and handlers
    create_idt(idtaddr);

    // load IDTR
    lidt(idtaddr, 8*NUM_INTERRUPTS-1);

    // enable interrupts on processor
    asm volatile ("sti");
}
