#include <stdio.h>
#include <pwd.h>
#include <sys/types.h>
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

//define structe type ProcSchedIndoType
	
typedef struct
	{
		unsigned int PID;
		char CMD[16];
		int PRIO;
		unsigned long POLICY;
	} ProcSchedInfoType;

int main(int argc, char **argv)
{    
	//variables to retrieve and store the User Id of the User Name
	struct passwd *pwd;
	unsigned int UidOfUser;
	
	//Process Count Interface between User Space and Kernel
	int *ProcessCountIF = NULL;
	int ProcessCountIFU, TotalProcesses;
	ProcessCountIF = &ProcessCountIFU;
	
	int *ResultIF = NULL;
	int ResultIFU = 0;
	ResultIF = &ResultIFU;
	
	ProcSchedInfoType ProcSchedinfo;
	char * ReturnArray;
	ReturnArray = (char *) &ProcSchedinfo;
	long int ret_status;
	char policyAnagram[2];

//debug - value to be printed to kernel log to ensure it is retrievable by Syscall on Kernel side
ProcessCountIFU = 1;


//test the number of command line parameters passed
if (argc >2) printf("Ignoring extra parameters \n");
if (argc ==1)
{
	printf("User name is required. \n");
	return 0;	// alert and do not do anything else...
}

/* Lookup UID of user passed as first parameter */
pwd = getpwnam(argv[1]);
if (pwd == NULL)
{
			printf("User '%s' is unknown.\n",argv[1]);
			return 0; // alert and do not do anything else...
}

UidOfUser = pwd->pw_uid;

/* STEP 1: FIRST SYSCALL TO RETRIEVE THE NUMBER OF PROCESSES FOR THE USER */

ret_status = syscall(398, UidOfUser, 0, ProcessCountIF, ReturnArray, ResultIF);

if(ret_status != 0)
{
		printf("System call - SyscallOperation 0 - 398 did not execute as expected. \n");
		return ret_status;
}

//debug
//printf("System call 398 executed correctly. \n");

TotalProcesses = *ProcessCountIF;

//debug
printf("Op 0 result: user '%s' currently has %d processes\n", argv[1], *ProcessCountIF);

//if there are no processes found, exit
if (TotalProcesses == 0) return 0;

/* STEP 2 : ALLOCATING ENOUGH MEMORY TO ARRAY OF ProcessSchedInfo Structure types */

//adding 10% memory more memory processes just in case of new processes 
//being created between System Call Operation 0 and System Call Operation 1
int MarginMemorySpace;
MarginMemorySpace = TotalProcesses/10;
if (MarginMemorySpace < 10)
{
		MarginMemorySpace = 10;
}

//Allocate memory for number of processes already plus the margin
ProcSchedInfoType *ProcSchedInfoArray = (ProcSchedInfoType *) calloc(TotalProcesses + MarginMemorySpace, sizeof(ProcSchedInfoType));
if (ProcSchedInfoArray == NULL)
{
	// Not enough memory
	printf("Not enough memory to hold all processes\n");
	// 
	// Aborting for now under this condition - future handling strategies include
	// iteratively reducing size of array to N, until Malloc does NOT fail
	// and then retrieving only the first N processes using the Syscall OR making multiple calls to Syscall, retrieving no more than N
	// processes at a time until all have been retrieved
	//  
	return 0;
}

//debug
//printf("Memory Allocation Complete\n");


/* STEP 3 : RETRIEVING ALL PROCESSES IN ProcSchedInfoArray AND LISTING THEM*/

ret_status = syscall(398, UidOfUser, 1 , ProcessCountIF, (char *) ProcSchedInfoArray, ResultIF);

if(ret_status != 0)
{
		printf("System call 398 - SyscallOperation 1 - did not execute as expected. \n");
		return ret_status;
}


printf("   PID COMMAND          PRI   CLS\n");
for (int i = 0; i < ProcessCountIFU; i++)
{
	switch(ProcSchedInfoArray[i].POLICY)
	{
			case 0:
				strcpy(policyAnagram, "TS");
				break;
			case 1:
				strcpy(policyAnagram, "FF");
				break;
			case 2:
				strcpy(policyAnagram, "RR");
				break;
			case 3:
				strcpy(policyAnagram,"B");
				break;
	}
	printf("%6d %-16s %3d %5s\n", ProcSchedInfoArray[i].PID, ProcSchedInfoArray[i].CMD, ProcSchedInfoArray[i].PRIO, policyAnagram);
}

printf("\n Op 1 Result Code: %d    Processes Returned: %d \n", ResultIFU, ProcessCountIFU);
if (ResultIFU != 0)
{
		printf ("Warning! Not all processes have been returned.\n\n");
}


free(ProcSchedInfoArray);

//debug
//printf("Memory freed correctly\n");


return 0;
}
