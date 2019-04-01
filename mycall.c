#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/sched.h>
#include<linux/syscalls.h>
#include "mycall.h"

typedef struct
	{
	unsigned int PID;
	char CMD[16];
	int PRIO;
	unsigned long POLICY;
	} ProcSchedInfoType;

asmlinkage long sys_sayhello(unsigned int userID, int SyscallOperation, int *CounterInterface, char *ReturnArray, int *ResultInterface)
{

	int ProcessCounter = 0;	//initialize the counter of processes to 0
	int BytesNotCopied = 0;
	struct task_struct* task; //declare pointer of struct task list
	ProcSchedInfoType CurrentTaskParameters;

	if (SyscallOperation < 0 || SyscallOperation > 1 || CounterInterface == NULL || ReturnArray == NULL || ResultInterface == NULL)
	{
		printk("Invalid parameters. Aborting.\n");
		return 1;
	}

/* debug in kernel log

	printk("UserID: %d \n", userID);
	printk("SyscallOperation: %d \n", SyscallOperation);
	printk("CounterInterface VALUE: %d \n", *CounterInterface);
	printk("ResultInterface initial value: %d\n", *ResultInterface);

*/

	if (SyscallOperation == 0) //return count of processes
	{
		ProcessCounter = 0; //redundant but just in case
		for_each_process(task) //loops through all processes of the kernel whether they belong to the userID or not
		{
			if ((task->cred->uid.val)==userID) //tests to see if process belongs to user
			{
				ProcessCounter++;
			}
		}
		put_user(ProcessCounter, CounterInterface); // Writes number of processes back to user space through CounterInterface
	}
	else if (SyscallOperation == 1) //return list of processes & attributes
	{
		/*	CounterInterface (CI for short) is an array of integers used to hold and exchange number of processes:
				CI[0] is the Total Number of Processes to return.
					Is is written/returned by SyscallOperation 0, and then kept constant as a reference.
					This will be the total number of processes returned by the successive calls to SyscallOperation 1
				CI[1] is the Number of Processes already returned in previous calls.
					Should be 0 on the first call to SyscallOperation 1
				CI[2] is the Number of Processes to return in a single call.
				CI[3] is the Number of Processes returned during this call.



		int Total,ToReturn, StartingAt;
		get_user(Total,CounterInterface);
		get_user(ToReturn,CounterInterface+1);
		get_user(StartingAt,CounterInterface+2);

		//debug
		printk("Total to return: %d. For this call, return %d starting at %d\n",Total,ToReturn, StartingAt);  

		if (StartingAt >= Total || ToReturn <= 0)
		{
			printk("Nothing to return\n")
			put_user(0,CounterInterface+3) // number of processes returned = 0
			return 0
		}

		*/

		ProcessCounter = 0; //redundant but just in case
		for_each_process(task)
		{
			if ((task->cred->uid.val)==userID) // a process is only counted and possibly returned if it matches the userID
			{
				//Copy task parameters to structure variable
				CurrentTaskParameters.PID = task->pid;
				strcpy(CurrentTaskParameters.CMD,task->comm);
				CurrentTaskParameters.PRIO = task->prio;
				CurrentTaskParameters.POLICY = task->policy;
					
				BytesNotCopied = copy_to_user (	ReturnArray + ProcessCounter * sizeof(ProcSchedInfoType),
								&CurrentTaskParameters, sizeof(ProcSchedInfoType));
						
				if (BytesNotCopied != 0)
				{
					//debug in case a process was not fully copied back
					printk("Process #: %d - Bytes not copied by copy_to_user: %d", ProcessCounter+1, BytesNotCopied);

					put_user(ProcessCounter, CounterInterface); // Write number of processes actually returned
					put_user(1, ResultInterface); // 1 means at least one process failed to be sent
					return 0;
				}

				ProcessCounter++;
			}
		}
		put_user(ProcessCounter, CounterInterface); // Write number of processes actually returned
		put_user(0, ResultInterface); // 0 means at least one process failed to be sent

	}
return 0;	
}
