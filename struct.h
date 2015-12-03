#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <sched.h>

#define SCHED_NORMAL 0

#define NUM_OF_CON  4
#define NUM_OF_PRC 20

#define MILLI_SEC_IN_SEC 1000
#define MAX_SLEEP_AVG 10

// Used for 1:1:3 distribution of FIFO:RR:NORMAL Scheduling Types
const int schedule_type[5] = {SCHED_FIFO, SCHED_RR, SCHED_NORMAL, SCHED_NORMAL, SCHED_NORMAL};

typedef struct {
    int pid;
    int static_priority;
    int dynamic_priority;
    int time_remaining;
    int time_slice;
    int accu_time_slice;
    int lastCPU;
    int sched_type;
    int sleep_avg;
    struct timeval last_time_used;
} task_struct;

typedef struct {
    int count;
    int last_entry_loc;
    int first_entry_loc;
    task_struct circ_buff[NUM_OF_PRC]; //just made sufficiently large to never overflow
 } queue_struct;
