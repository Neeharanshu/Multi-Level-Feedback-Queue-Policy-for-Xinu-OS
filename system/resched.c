/* resched.c - resched, resched_cntl */

#include <xinu.h>

struct	defer	Defer;

/*------------------------------------------------------------------------
 *  resched  -  Reschedule processor to highest priority eligible process
 *------------------------------------------------------------------------
 */
void	resched(void)		/* Assumes interrupts are disabled	*/
{
	struct procent *ptold;	/* Ptr to table entry for old process	*/
	struct procent *ptnew;	/* Ptr to table entry for new process	*/
	int pid_deq;
	struct procent *dequeue_pid;
	int old_pid = currpid;

	/* If rescheduling is deferred, record attempt and return */

	if (Defer.ndefers > 0) {
		Defer.attempt = TRUE;
		return;
	}

	ptold = &proctab[currpid];
	if(boost_time == PRIORITY_BOOST_PERIOD)
	{
		if(nonempty(userlist1))
		{
			pid_deq = firstid(userlist1);
			kprintf("pid_deq is %d", pid_deq);
			while(pid_deq != EMPTY)
			{
				dequeue_pid = &proctab[pid_deq];
				dequeue_pid->time_allot = TIME_ALLOTMENT;
				pid_deq = queuetab[pid_deq].qnext;
			}
		}
		while(nonempty(userlist2))
		{
			kprintf("Lslsdbhrfhubrusdfjgbkrsuhgri\n");
			pid_deq = dequeue(userlist2);
			dequeue_pid = &proctab[pid_deq];
			dequeue_pid->time_allot = TIME_ALLOTMENT;
			enqueue(pid_deq, userlist1);	
		}
		while(nonempty(userlist3))
		{
			kprintf("userlist3s\n");	
			pid_deq = dequeue(userlist3);
			dequeue_pid = &proctab[pid_deq];
			dequeue_pid->time_allot = TIME_ALLOTMENT;
			enqueue(pid_deq, userlist1);	
		}
	}

/*IF this was a system call*/
	if((strncmp(ptold->prname, "prnull",6)==0))
		{
			if(nonempty(readylist))
			{
				if (ptold->prstate == PR_CURR) { 
					ptold->prstate = PR_READY;
					//kprintf("I am inserting here1");
					insert(currpid, readylist, ptold->prprio);
				}
				currpid = dequeue(readylist);
				ptnew = &proctab[currpid];
				ptnew->prstate = PR_CURR;
				preempt = TIME_SLICE;		/* Reset time slice for process	*/
			}
			else
			{
				if(nonempty(userlist1) || nonempty(userlist2) || nonempty(userlist3)){
					if(ptold->prstate == PR_CURR)
					{
						ptold->prstate = PR_READY;
						insert(currpid, readylist, ptold->prprio);
					}
					//kprintf("I came here \n");
					if(nonempty(userlist1))
					{
						currpid = dequeue(userlist1);
						//kprintf("%d in userlist1", currpid);
						preempt = TIME_SLICE;
					}
					else if (nonempty(userlist2))
					{
						currpid = dequeue(userlist2);
						//kprintf("%d in userlist2", currpid);
						preempt = TIME_SLICE*2;
					}
					else
					{
						currpid = dequeue(userlist3);
						//kprintf("%d in userlist1", currpid);
						preempt = TIME_SLICE*4;
					}
					ptnew = &proctab[currpid];
					ptnew->prstate = PR_CURR;
					
				}
				else
				{
					preempt = TIME_SLICE;
					return;
				}
			}
		}
	else if(ptold->sys_call == 1)
	{
		if (ptold->prstate == PR_CURR){  /* Process remains eligible */
			if (ptold->prprio > firstkey(readylist)){
				preempt = TIME_SLICE;	
				return;
			}
			ptold->prstate = PR_READY;
			insert(currpid, readylist, ptold->prprio);
			currpid = dequeue(readylist);
			ptnew = &proctab[currpid];
			ptnew->prstate = PR_CURR;
			preempt = TIME_SLICE;	
		}
		else if (firstid(readylist) != NULLPROC){
					currpid = dequeue(readylist);
					ptnew = &proctab[currpid];
					ptnew->prstate = PR_CURR;
					preempt = TIME_SLICE;		/* Reset time slice for process	*/
				}
		else if(nonempty(userlist1) || nonempty(userlist2) || nonempty(userlist3))
		{
			if(ptold->prstate == PR_CURR)
			{
				ptold->prstate = PR_READY;
				insert(currpid, readylist, ptold->prprio);
			}
			if(nonempty(userlist1)){
				currpid = dequeue(userlist1);
				preempt = TIME_SLICE;
			}
			else if (nonempty(userlist2)){
				currpid = dequeue(userlist2);
				preempt = TIME_SLICE*2;
			}
			else{
				currpid = dequeue(userlist3);
				preempt = TIME_SLICE*4;
			}
			ptnew = &proctab[currpid];
			ptnew->prstate = PR_CURR;
			
		}
		else
		{
			if (ptold->prstate == PR_CURR) { 
				if(ptold->prprio > firstkey(readylist)){
					preempt = TIME_SLICE;
					return;
				}
				ptold->prstate = PR_READY;
					//kprintf("I am inserting here4");
				insert(currpid, readylist, ptold->prprio);
			}
			currpid = dequeue(readylist);
			ptnew = &proctab[currpid];
			ptnew->prstate = PR_CURR;
			preempt = TIME_SLICE;		/* Reset time slice for process	*/
		}

			/* Reset time slice for process	*/
	}
	else
	{
		if(ptold->prstate == PR_CURR && ptold->number_bursts>0)
		{
			ptold->prstate = PR_READY;
			if(ptold->time_allot <= 0){
				switch(ptold->qnum){
					case 1: enqueue(currpid, userlist2);
						//	kprintf("I enqueued %d in uerlist coz numer of bursts is %d\n", currpid, ptold->number_bursts);
							ptold->time_allot = TIME_ALLOTMENT;
							ptold->qnum = 2;
							break;
					case 2: 
					case 3: enqueue(currpid, userlist3);
						//	kprintf("I enqueued %d in uerlist3 coz numer of bursts is %d\n", currpid, ptold->number_bursts);
							ptold->time_allot = TIME_ALLOTMENT;
							ptold->qnum = 3;
							break;
					default: kprintf("Qnum %d not found", ptold->qnum);
							 break;
				}
				//kprintf("Did this succesfully\n");
			}
			else{
				switch(ptold->qnum){
					case 1: enqueue(currpid, userlist1);
							break;
					case 2: enqueue(currpid, userlist2);
							break;
					case 3: enqueue(currpid, userlist3);
							break;
					default: kprintf("Qnum %d not found", ptold->qnum);
							 break;
				}
			}
		}
		if(firstid(readylist) != NULLPROC)
		{
			currpid = dequeue(readylist);
			ptnew = &proctab[currpid];
			ptnew->prstate = PR_CURR;
			preempt = TIME_SLICE;
		}
		else if(nonempty(userlist1) || nonempty(userlist2)||nonempty(userlist3))
		{
			if(nonempty(userlist1)){
				currpid = dequeue(userlist1);
				preempt = TIME_SLICE;
			}
			else if (nonempty(userlist2)){
				//kprintf("Time allotment for %d is %d\n", currpid, ptold->time_allot);
				currpid = dequeue(userlist2);
				preempt = TIME_SLICE*2;	
			}
			else{
				currpid = dequeue(userlist3);
				preempt = TIME_SLICE*4;
			}
			ptnew = &proctab[currpid];
			ptnew->prstate = PR_CURR;
			
			//kprintf("Context switching from here");	
		}
		else
		{
			currpid = dequeue(readylist);
			ptnew = &proctab[currpid];
			ptnew->prstate = PR_CURR;
			preempt = TIME_SLICE;
		}
	}
	if(old_pid!=currpid)
		kprintf("Old process: %d\tnew process: %d\n", old_pid,currpid);
	ctxsw(&ptold->prstkptr, &ptnew->prstkptr);

	/* Old process returns here when resumed */

	return;
}

/*------------------------------------------------------------------------
 *  resched_cntl  -  Control whether rescheduling is deferred or allowed
 *------------------------------------------------------------------------
 */
status	resched_cntl(		/* Assumes interrupts are disabled	*/
	  int32	defer		/* Either DEFER_START or DEFER_STOP	*/
	)
{
	switch (defer) {

	    case DEFER_START:	/* Handle a deferral request */

		if (Defer.ndefers++ == 0) {
			Defer.attempt = FALSE;
		}
		return OK;

	    case DEFER_STOP:	/* Handle end of deferral */
		if (Defer.ndefers <= 0) {
			return SYSERR;
		}
		if ( (--Defer.ndefers == 0) && Defer.attempt ) {
			resched();
		}
		return OK;

	    default:
		return SYSERR;
	}
}
