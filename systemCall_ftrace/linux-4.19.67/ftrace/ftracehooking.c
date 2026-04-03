#include "ftracehooking.h"
#include <linux/sched.h>
#include <linux/init_task.h>
#include <linux/slab.h>
MODULE_LICENSE("GPL");

void **syscall_table;
asmlinkage int (*original_ftrace)(pid_t);

//global variables that will be exported
pid_t tracePID = 0;
char *ftraceFName = NULL;	//memory allocated is needed
size_t readByte = 0;
size_t writeByte = 0;
int openCnt = 0;
int closeCnt = 0;
int readCnt = 0;
int writeCnt = 0;
int lseekCnt = 0;

EXPORT_SYMBOL(tracePID);
EXPORT_SYMBOL(ftraceFName);
EXPORT_SYMBOL(readByte);
EXPORT_SYMBOL(writeByte);
EXPORT_SYMBOL(openCnt);
EXPORT_SYMBOL(closeCnt);
EXPORT_SYMBOL(readCnt);
EXPORT_SYMBOL(writeCnt);
EXPORT_SYMBOL(lseekCnt);

//hooking function
static asmlinkage int ftraceHooking(const struct pt_regs *regs)
{	
	struct task_struct *task;	// for find pid's process name
	int usrPid = regs->di;		// di is the first parameter of this function
	if(!tracePID)		// case: tracing nothing
	{
		//initialize global variables
		tracePID = usrPid;
		if(ftraceFName == NULL)
		{
			ftraceFName = kzalloc(256, GFP_ATOMIC);
		}
		readByte = 0;
		writeByte = 0;
		openCnt = 0;
		closeCnt = 0;
		readCnt = 0;
		writeCnt = 0;
		lseekCnt = 0;
		printk("OS Assignment 2 ftrace [%d] Start\n", usrPid);	// print that file tracing is started
		return 0;	// stop system call routine
	}
	if(usrPid) return 0;	// case: already tracing something -> just return
	// case: the end of tracing
	// find task struct that fits with tracing pid
	for_each_process(task)
	{
		if(task->pid == tracePID) break;
	}
	// print the result of tracing
	printk("[2019202053] /%s file[%s] stats [x] read - %lu / written - %lu\n", task->comm, ftraceFName, readByte, writeByte);
	printk("open[%d] close[%d] read[%d] write[%d] lseek[%d]\n", openCnt, closeCnt, readCnt, writeCnt, lseekCnt); 
	printk("OS Assignment 2 ftrace [%d] End\n", tracePID);
	// reset tracePID so that next system call works start to trace
	tracePID = 0;
	if(ftraceFName != NULL)
	{
		kfree(ftraceFName);
		ftraceFName = NULL;
	}
	return 0;
}

static int __init hooking_init(void)
{
	syscall_table = (void **) kallsyms_lookup_name("sys_call_table");

	make_rw(syscall_table);
	original_ftrace = syscall_table[__NR_ftrace];	// get original ftrace function
	syscall_table[__NR_ftrace] = (void*)(&ftraceHooking);	// set new ftrace function
	make_ro(syscall_table);

	return 0;
}

static void __exit hooking_exit(void)
{
	if(ftraceFName != NULL)
		kfree(ftraceFName);	//if the variable has already alloced, free it
	make_rw(syscall_table);
	syscall_table[__NR_ftrace] = original_ftrace;	// set original ftrace function
	make_ro(syscall_table);
}

module_init(hooking_init);
module_exit(hooking_exit);
