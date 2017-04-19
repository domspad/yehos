#include "kernel.h"
#include "asmhelpers.h"

#define PAGEDIR_ADDR 0x80000
#define PT0_ADDR 0x81000
#define PRESENT_AND_RW 0x03

physaddr_t g_nextPage = 0x100000;

void
setup_paging()
{
  uint32_t *pagedir = (void *) PAGEDIR_ADDR;
  memset(pagedir, 0, 4096);
  pagedir[0] = PT0_ADDR | PRESENT_AND_RW;
  pagedir[1023] = PAGEDIR_ADDR | PRESENT_AND_RW;

  uint32_t *pt0 = (void *) PT0_ADDR;
  memset(pt0, 0, 4096);

  // identity-map first 1MB
  for (unsigned int i=0; i<256; ++i) {
      pt0[i] = (i << 12) | PRESENT_AND_RW;
  }

  set_cr3(PAGEDIR_ADDR);

  set_cr0_paging();
}

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
  ptable[pfaddr >> 12] = page | PRESENT_AND_RW;
  kprintf("Page fault at 0x%x, replacing with phys page at 0x%x\n", pfaddr, page);
}
