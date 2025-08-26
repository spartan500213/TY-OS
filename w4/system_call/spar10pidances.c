#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/sched.h>
#include <linux/pid.h>

SYSCALL_DEFINE1(spar10pidances, pid_t, pid) {
	struct task_struct *task;
	struct task_struct *parent;

	printk(KERN_INFO "spar10pidances: ancestor/s for PID %d\n", pid);
	
	//find the task_struct for the given PID
	task = pid_task(find_vpid(pid), PIDTYPE_PID);

	//check if the process exists
	if(!task) {
		printk(KERN_ERR "spar10pidances: process with PID %d not found\n", pid);
		return -ESRCH; // return no such process error
	}

	//start with the initial process
	parent = task;

	//loop through the parent processes until we reach the init process
	while(parent->pid != 1 && parent->parent->pid != 0) {
		parent = parent->real_parent;
		if(parent) {
			printk(KERN_INFO "spar10pidances: parent PID: %d, name: %s\n", parent->pid, parent->comm);
		}
		else break;
	}

	printk(KERN_INFO "spar10pidances: syscall complete\n");

	return 0; //success
}
