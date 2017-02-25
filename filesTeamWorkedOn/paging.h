

#ifndef _PAGING_H_
#define _PAGING_H_

#define NUM_PAGES 1024
#define NUM_PAGE_TABLES 1024
#define NUM_PROCESS 6
#define PAGE_SIZE 4096

#ifndef ASM

extern void load_CR3 (uint32_t *ptr);
extern void set_cr0 ();
extern void set_pse ();

#endif

extern void setup_paging(int pid );

extern void update_paging(int pid);

extern void add_vidmap(uint32_t virt_vmem);

#endif /* _PAGING_H_ */
