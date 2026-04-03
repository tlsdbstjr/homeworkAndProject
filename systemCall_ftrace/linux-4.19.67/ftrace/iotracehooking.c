#include "ftracehooking.h"
#include <asm/uaccess.h>

// system call numbers
#define __NR_open 2
#define __NR_read 0
#define __NR_write 1
#define __NR_lseek 8
#define __NR_close 3

void **syscall_table;
// origianl system call function pointers
asmlinkage long(*ori_open)(const struct pt_regs *regs);
asmlinkage long(*ori_read)(const struct pt_regs *regs);
asmlinkage long(*ori_write)(const struct pt_regs *regs);
asmlinkage long(*ori_lseek)(const struct pt_regs *regs);
asmlinkage long(*ori_close)(const struct pt_regs *regs);

// global variables from ftracinghooking.c
extern pid_t tracePID;
extern char *ftraceFName;
extern size_t readByte;
extern size_t writeByte;
extern int openCnt;
extern int closeCnt;
extern int readCnt;
extern int writeCnt;
extern int lseekCnt;

// hooking functions for ftrace
static asmlinkage long ftrace_open(const struct pt_regs *regs)
{
	if(current->pid == tracePID)
	{
		long strCnt = strnlen_user((char *)regs->di, 256);	// di is the first parameter
		strncpy_from_user(ftraceFName, (char *)regs->di, strCnt);	//get open file name from user space
		openCnt++;
	}
	return (ori_open(regs));
}

static asmlinkage long ftrace_read(const struct pt_regs *regs)
{	
	if(current->pid == tracePID)
	{
		readCnt++;
		readByte += (size_t)regs->dx;	// dx is the third parameter, which is count
	}
	return (ori_read(regs));
}

static asmlinkage long ftrace_write(const struct pt_regs *regs)
{
	if(current->pid == tracePID)
	{
		writeCnt++;
		writeByte += (size_t)regs->dx;	// dx is the third parameter, which is count
	}
	return (ori_write(regs));
}

static asmlinkage long ftrace_close(const struct pt_regs *regs)
{
	if(current->pid == tracePID)
		closeCnt++;

	return (ori_close(regs));
}

static asmlinkage long ftrace_lseek(const struct pt_regs *regs)
{
	if(current->pid == tracePID)
		lseekCnt++;
	
	return (ori_lseek(regs));
}




static int __init hooking_init(void)
{
	syscall_table = (void **) kallsyms_lookup_name("sys_call_table");

	make_rw(syscall_table);
	// get original system call functions
	ori_open = syscall_table[__NR_open];
	ori_read = syscall_table[__NR_read];
	ori_write = syscall_table[__NR_write];
	ori_lseek = syscall_table[__NR_lseek];
	ori_close = syscall_table[__NR_close];
	
	// set hooking system call functions
	syscall_table[__NR_open] = (void*)&ftrace_open;
	syscall_table[__NR_read] = (void*)&ftrace_read;
	syscall_table[__NR_write] = (void*)&ftrace_write;
	syscall_table[__NR_lseek] = (void*)&ftrace_lseek;
	syscall_table[__NR_close] = (void*)&ftrace_close;
	make_ro(syscall_table);
	return 0;
}

static void __exit hooking_exit(void)
{
	make_rw(syscall_table);
	// set origianl system call functions
	syscall_table[__NR_open] = ori_open;
	syscall_table[__NR_read] = ori_read;
	syscall_table[__NR_write] = ori_write;
	syscall_table[__NR_lseek] = ori_lseek;
	syscall_table[__NR_close] = ori_close;
	make_ro(syscall_table);
}

module_init(hooking_init);
module_exit(hooking_exit);
