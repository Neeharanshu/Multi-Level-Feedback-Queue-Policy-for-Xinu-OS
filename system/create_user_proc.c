/* create.c - create, newpid */

#include <xinu.h>
#include <stdarg.h>

//local	int newpid();

/*------------------------------------------------------------------------
 *  create  -  Create a process to start running a function on x86
 *------------------------------------------------------------------------
 */
pid32	create_user_proc(
	  void		*funcaddr,	/* Address of the function	*/
	  uint32	ssize,		/* Stack size in bytes		*/
	  uint32	run_time,	/* Process priority > 0		*/
	  char		*name,		/* Name (for debugging)		*/
	  uint32	nargs,		/* Number of args that follow	*/
	  ...
	)
{
	uint32		savsp, *pushsp;
	intmask 	mask;    	/* Interrupt mask		*/
	pid32		pid;		/* Stores new process id	*/
	struct	procent	*prptr;		/* Pointer to proc. table entry */
	int32		i;
	uint32		*a;		/* Points to list of args	*/
	uint32		*saddr;		/* Stack address		*/
	uint32	number_bursts, burst_duration, sleep_duration;
	mask = disable();
	if (ssize < MINSTK)
		ssize = MINSTK;
	ssize = (uint32) roundmb(ssize);
	if ( ((pid=newpid()) == SYSERR) ||
	     ((saddr = (uint32 *)getstk(ssize)) == (uint32 *)SYSERR) ) {
		restore(mask);
		return SYSERR;
	}

	prcount++;
	prptr = &proctab[pid];
	va_list args;
	va_start(args, nargs);
	number_bursts = va_arg(args, uint32);
	burst_duration = va_arg(args, uint32);
	sleep_duration = va_arg(args, uint32);
	va_end(args); 
	//kprintf("Here 1\n");
	/* Initialize process table entry for new process */
	prptr->prstate = PR_SUSP;	/* Initial state is suspended	*/
	//prptr->prprio = priority;
	prptr->prstkbase = (char *)saddr;
	prptr->prstklen = ssize;
	prptr->number_bursts = number_bursts;
	prptr->burst_duration = burst_duration;
	prptr->sleep_duration = sleep_duration;
	prptr->run_time = run_time;
	prptr->sys_call = 0;
	prptr->qnum = 1;
	prptr->time_allot = TIME_ALLOTMENT;
	prptr->prname[PNMLEN-1] = NULLCH;
	//kprintf("Here 1\n");
	for (i=0 ; i<PNMLEN-1 && (prptr->prname[i]=name[i])!=NULLCH; i++)
		;
	prptr->prsem = -1;
	prptr->prparent = (pid32)getpid();
	prptr->prhasmsg = FALSE;

	/* Set up stdin, stdout, and stderr descriptors for the shell	*/
	prptr->prdesc[0] = CONSOLE;
	prptr->prdesc[1] = CONSOLE;
	prptr->prdesc[2] = CONSOLE;
	//kprintf("Here 1\n");
	/* Initialize stack as if the process was called		*/

	*saddr = STACKMAGIC;
	savsp = (uint32)saddr;

	/* Push arguments */
	a = (uint32 *)(&nargs + 1);	/* Start of args		*/
	a += nargs -1;			/* Last argument		*/
	for ( ; nargs > 0 ; nargs--)	/* Machine dependent; copy args	*/
		*--saddr = *a--;	/* onto created process's stack	*/
	*--saddr = (long)INITRET;	/* Push on return address	*/

	/* The following entries on the stack must match what ctxsw	*/
	/*   expects a saved process state to contain: ret address,	*/
	/*   ebp, interrupt mask, flags, registers, and an old SP	*/

	*--saddr = (long)funcaddr;	/* Make the stack look like it's*/
					/*   half-way through a call to	*/
					/*   ctxsw that "returns" to the*/
					/*   new process		*/
	*--saddr = savsp;		/* This will be register ebp	*/
					/*   for process exit		*/
	savsp = (uint32) saddr;		/* Start of frame for ctxsw	*/
	*--saddr = 0x00000200;		/* New process runs with	*/
					/*   interrupts enabled		*/

	/* Basically, the following emulates an x86 "pushal" instruction*/

	*--saddr = 0;			/* %eax */
	*--saddr = 0;			/* %ecx */
	*--saddr = 0;			/* %edx */
	*--saddr = 0;			/* %ebx */
	*--saddr = 0;			/* %esp; value filled in below	*/
	pushsp = saddr;			/* Remember this location	*/
	*--saddr = savsp;		/* %ebp (while finishing ctxsw)	*/
	*--saddr = 0;			/* %esi */
	*--saddr = 0;			/* %edi */
	*pushsp = (unsigned long) (prptr->prstkptr = (char *)saddr);
	restore(mask);
	return pid;
}

void burst_execution(uint32 number_bursts, uint32 burst_duration, uint32 sleep_duration){
	struct procent *ptr = &proctab[currpid];
	//kprintf("I came here atleast\n");
	while(ptr->number_bursts > 0)
	{
		while(ptr->burst_done_flag != 1);
		if(ptr->number_bursts >= 1 && sleep_duration > 0)
		{
			sleepms(sleep_duration);
			//kprintf("Going to sleep for %d coz %d\n", currpid, ptr->burst_duration);
		}
		while(ptr->prstate != PR_CURR);
		ptr->number_bursts--;
		if(ptr->number_bursts <= 0)
		{
			ptr->prstate = PR_FREE;
			//kprintf("I am breaking now %d\n", currpid);
			//break;
		}
		else
		{
			ptr->burst_duration = burst_duration;
			ptr->burst_done_flag = 0;
		}
	}
	while(1);
}
