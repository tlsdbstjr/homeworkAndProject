#include <linux/module.h>
#include <linux/highmem.h>
#include <linux/kallsyms.h>
#include <linux/syscalls.h>
#include <asm/syscall_wrapper.h>

#define __NR_ftrace 336

// the function that set parameter's page writalbe
void make_rw(void *addr)
{
	unsigned int level;
	pte_t *pte = lookup_address((u64)addr, &level);
	if(!(pte->pte & _PAGE_RW))
		pte->pte |= _PAGE_RW;
}

// the function that set parameter's page unwritable
void make_ro(void *addr)
{
	unsigned int level;
	pte_t *pte = lookup_address((u64)addr, &level);
	
	pte->pte = pte->pte &~ _PAGE_RW;
}

MODULE_LICENSE("GPL");
