/* ready.c - ready */

#include <xinu.h>

qid16	readylist;			/* Index of ready list		*/
qid16	userlist1;			/* Index of ready list		*/
qid16	userlist2;			/* Index of ready list		*/
qid16	userlist3;			/* Index of ready list		*/


/*------------------------------------------------------------------------
 *  ready  -  Make a process eligible for CPU service
 *------------------------------------------------------------------------
 */
status	ready(
	  pid32		pid		/* ID of process to make ready	*/
	)
{
	register struct procent *prptr;

	if (isbadpid(pid)) {
		return SYSERR;
	}

	/* Set process state to indicate ready and add to ready list */
	//kprintf("I came in ready for %d\n", pid);
	prptr = &proctab[pid];
	//if(prptr->prstate == PR_SUSP)
	//	prptr->prstate = PR_READY;
	if(prptr->sys_call == 1)
		insert(pid, readylist, prptr->prprio);
	else if(prptr->number_bursts > 0){
		switch(prptr->qnum){
		case 1: enqueue(pid, userlist1);
				break;
		case 2: //kprintf("I enqueued from ready %d in uerlist coz numer of bursts is %d\n", currpid, prptr->number_bursts);
				enqueue(pid, userlist2);
				break;
		case 3: enqueue(pid, userlist3);
				break;
		}
	}
	//resched();

	return OK;
}
