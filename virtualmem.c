#include "kernel.h"
#include "asmhelpers.h"

physaddr_t g_nextPage = 0x100000;

physaddr_t
get_unused_page()
{
    physaddr_t ret = g_nextPage;
    g_nextPage += 0x1000;
    return ret;
}

void
handle_page_fault()
{
  uint32_t pfaddr = get_cr2();
  uint32_t *ptable = (uint32_t *) 0xffc00000;
  physaddr_t page = get_unused_page();
  ptable[pfaddr >> 12] = page | 0x3;
  kprintf("Page fault at 0x%x, replacing with phys page at 0x%x\n", pfaddr, page);
}
