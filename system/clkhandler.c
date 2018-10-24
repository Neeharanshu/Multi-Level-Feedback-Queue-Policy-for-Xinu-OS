/* clkhandler.c - clkhandler */

#include <xinu.h>
uint32 boost_time;
/*------------------------------------------------------------------------
 * clkhandler - high level clock interrupt handler
 *------------------------------------------------------------------------
 */
void	clkhandler()
{
	static	uint32	count1000 = 1000;	/* Count to 1000 ms	*/
	//kprintf("%d\n", count1000);
	/* Decrement the ms counter, and see if a second has passed */
	struct procent *ptr = &proctab[currpid];
	if(ptr->sys_call == 0 && ptr->prstate == PR_CURR)
	{
		ptr->burst_duration--;
		if(ptr->burst_duration == 0)
			ptr->burst_done_flag = 1;
		if(ptr->time_allot >= 0)
			ptr->time_allot--;
			//kill(currpid);
	}
	boost_time++;
	//kprintf("%d\n", count1000);
	if((--count1000) <= 0) {

		/* One second has passed, so increment seconds count */

		clktime++;

		/* Reset the local ms counter for the next second */

		count1000 = 1000;
	}

	/* Handle sleeping processes if any exist */

	if(!isempty(sleepq)) {

		/* Decrement the delay for the first process on the	*/
		/*   sleep queue, and awaken if the count reaches zero	*/

		if((--queuetab[firstid(sleepq)].qkey) <= 0) {
			wakeup();
		}
	}

	/* Decrement the preemption counter, and reschedule when the */
	/*   remaining time reaches zero			     */
//kprintf("Came here for %d\n", preempt);
	if((--preempt) <= 0) {
		//preempt = TIME_SLICE;
		resched();
	}
}
