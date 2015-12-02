#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <math.h>
#include <sched.h>


#define NUM_OF_CON  4
#define NUM_OF_PRC 20
#define MICRO_SEC_IN_SEC 1000000

typedef struct {
    int pid;
    int static_priority;
    int dynamic_priority;
    int time_remaining;
    int time_slice;
    int accu_time_slice;
    int lastCPU;
} task_struct;

typedef struct {
    int count;
    int last_entry_loc;
    int first_entry_loc;
    task_struct circ_buff[NUM_OF_PRC]; //just made sufficiently large to never overflow
 } queue_struct;
