#include <linux/kernel.h>
#include <linux/syscalls.h>

// origianl ftrace function call
SYSCALL_DEFINE1(ftrace, pid_t, usrPid)
{
	printk("ftrace function is called! the parameter is %d\n", usrPid);	// it just prints that it's called
	return 0;
}
