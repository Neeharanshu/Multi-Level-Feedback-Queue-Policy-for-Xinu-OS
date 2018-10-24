#include <xinu.h>
#include <stdio.h>

int prA, prB, prC;
int proc_a(), proc_b(), proc_c();
volatile int a_cnt = 0;
volatile int b_cnt = 0;
volatile int c_cnt = 0;

//TIME_SLICE            2
//TIME_ALLOTMENT        400
//PRIORITY_BOOST_PERIOD 2200

int main() {
    double total_cnt, a_percent, b_percent, c_percent;
    int i;

    kprintf("\n\n########## Test Case 4, MLFQ scheduling (3 processes):\n");
    resume( prA = create_user_proc(proc_a, 1024, 10000, "proc A", 3, 1, 10000, 0) );
    sleepms(1000);
    for(i=0;i<100000000;i++);
    resume( prB = create_user_proc(proc_b, 1024, 10000, "proc B", 3, 1, 10000, 0) );
    resume( prC = create_user_proc(proc_c, 1024, 10000, "proc C", 3, 1, 10000, 0) );
    sleepms(2100);
    for(i=0;i<100000000;i++);
    kill(prA);
    kill(prB);
    kill(prC);
    total_cnt = a_cnt + b_cnt + c_cnt;
    a_percent = (double) a_cnt / total_cnt * 100;
    b_percent = (double) b_cnt / total_cnt * 100;
    c_percent = (double) c_cnt / total_cnt * 100;
    kprintf("Test RESULT: A : B : C (%d : %d : %d)\n", (int) a_percent, (int) b_percent, (int) c_percent);


    return OK;
}


proc_a(c)
char c; {
    int i;
    while (1) {
        for (i = 0; i < 100; i++)
            ;
        a_cnt++;
    }
}

proc_b(c)
char c; {
    int i;
    while (1) {
        for (i = 0; i < 100; i++)
            ;
        b_cnt++;
    }
}

proc_c(c)
char c; {
    int i;
    
    while (1) {
        for (i = 0; i < 100; i++)
            ;
        c_cnt++;
    }
}
