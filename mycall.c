#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/sched.h>
#include<linux/syscalls.h>
#include "mycall.h"

asmlinkage long sys_sayhello(unsigned int userID, int SyscallOperation, int *process_counter_interface)
{

	if (SyscallOperation == 0) //return count of processes
	{
		int ProcessCounter = 0; //initialize the counter of processes to 0
		struct task_struct* task; //declare pointer of struct task list
		for_each_process(task) //loops through all processes of the kernel whether they belong to the userID or not
		{
			if ((task->cred->uid.val)==userID) //tests to see if process belongs to user
			{
				ProcessCounter++;
			}
		}
		put_user(ProcessCounter, process_counter_interface);
	}
	else if (SyscallOperation == 1) //return list of processes & attributes
	{
		/*
		/	possibly allocate memory for upcoming execution
		/
		*/
		for_each_process(task)
		{
			if ((task->cred->uid.val)==userID)
			{
				put_user(task->pid, process_counter_interface+1);
				put_user(task->comm, process_counter_interface+2);
				put_user(task->prio, process_counter_interface+3);
			}
		}
		/*
		/	possibly free memory for previous execution
		/
		*/
	}
	return 0;
	
}


