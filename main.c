#include "struct.h"


void * consumer_func(void * args);
void * producer_func(void * args);
void * queue_balance_func(void * args);

void generate_processes();
void add_process(queue_struct *queue, task_struct *task);
void add_process_by_priority(task_struct *process, int cpu_id);
task_struct remove_process_from_rq(queue_struct *queue);
int check_queue(queue_struct *ready_q, int id, int rqlevel);

int count_processes_in_cpu(int id);
int calculate_ticks(struct timeval t1, struct timeval t2);
int calculate_turnaround(struct timeval t1, struct timeval t2);
int calculate_dp(int previous_dp, int bonus);
int calculate_time_slice(int sp);

int min(int a, int b);
int max(int a, int b);

int producer_created = 0;
int consumers_created = 0;
int processes_completed = 0;

pthread_mutex_t processes_completed_mutex;
pthread_mutex_t cpu_mutex[NUM_OF_CON];

queue_struct rq0[NUM_OF_CON];
queue_struct rq1[NUM_OF_CON];
queue_struct rq2[NUM_OF_CON];


int main(){
    int res;
    pthread_t consumer_threads[NUM_OF_CON];
    pthread_t producer_thread;
    pthread_t balancer_thread;
    void *thread_result;
    pthread_attr_t thread_attr;
    
    srand(time(NULL)); // A random random!
    
    //Consumer
    for(int i = 0; i<NUM_OF_CON;i++){
        rq0[i].first_entry_loc = 0;
        rq0[i].last_entry_loc = 0;
        rq1[i].first_entry_loc = 0;
        rq1[i].last_entry_loc = 0;
        rq2[i].first_entry_loc = 0;
        rq2[i].last_entry_loc = 0;
        res = pthread_attr_init(&thread_attr);
        if (res != 0){
            perror("Attribute creation failed for consumer");
            exit(EXIT_FAILURE); 
        }
        res = pthread_attr_setschedpolicy(&thread_attr,SCHED_RR);
        if (res != 0){
            perror("Setting policy failed for consumer");
            exit(EXIT_FAILURE);
        }
        res = pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);
        if (res != 0){
            perror("Setting detached attribute failed for consumer");
            exit(EXIT_FAILURE);
        }
        res = pthread_create(&consumer_threads[i],&thread_attr, consumer_func, (void *)(intptr_t)i);
        if (res != 0){
            perror("Thread creation failed for consumer.");
            exit(EXIT_FAILURE);
        }
    }
    while(consumers_created < NUM_OF_CON);
    
    //Producer
    res = pthread_create(&producer_thread,&thread_attr, producer_func, (void *) NULL);
    if (res != 0){
        perror("Thread creation failed for Producer.");
        exit(EXIT_FAILURE);
    }
    while(!producer_created);
    
    //Balancer
    res = pthread_create(&balancer_thread, &thread_attr, queue_balance_func, (void *) NULL);
    if (res != 0){
        perror("Thread creation failed for queue balancer.");
        exit(EXIT_FAILURE);
    }
    while(processes_completed < 20){
        sleep(1);
    };
    
    exit(EXIT_SUCCESS);
}

/**
 * Thread Function for Consumer(s)
 */
void * consumer_func(void * args){
    int id = (intptr_t) args;
    
    queue_struct *ready_q0 = &rq0[id];
    queue_struct *ready_q1 = &rq1[id];
    queue_struct *ready_q2 = &rq2[id];

    printf("This is Consumer %d.\n",id);
    consumers_created++;

    while(processes_completed<20){
        if (check_queue(ready_q0, id, 0));
        else if (check_queue(ready_q1, id, 1));
        else if (check_queue(ready_q2, id, 2));
        else {
            sleep(1);
        }
    }
}

/**
 * Function to run a queue's process (if the process exists)
 */
int check_queue(queue_struct *ready_q, int id, int rqlevel) {
    int ticks;
    pthread_mutex_lock(&cpu_mutex[id]); 
    
    if(ready_q->count != 0){
        struct timeval t1, t2;
        gettimeofday(&t2, NULL);

        task_struct *task = &ready_q->circ_buff[ready_q->first_entry_loc];
        
        // Increment sleep_avg (bonus)
	    if(task->sched_type == SCHED_NORMAL){
            ticks = calculate_ticks(task->last_time_used, t2)*3;//  Multiply by 3 for I/O operation.
            task->sleep_avg = min(task->sleep_avg + ticks, MAX_SLEEP_AVG);
            task->dynamic_priority = calculate_dp(task->dynamic_priority, task->sleep_avg);
		    task->time_slice = calculate_time_slice(task->dynamic_priority);
	    }
	    
	    //This is what the prof wanted! Sorry Mr. TA
	    printf("Process: %d with Priority-%d SELECTED in CPU %d from Queue RQ%d\n\n",
	            task->pid,
	            task->sched_type,
	            id,
	            rqlevel);
        
        gettimeofday(&t1, NULL);
        usleep(min(task->time_remaining,task->time_slice) * 1000);
        gettimeofday(&t2, NULL);
        
        task->accu_time_slice += min(task->time_remaining,task->time_slice);
        task->time_remaining -= task->time_slice;
        
        ready_q->count--;
        if(ready_q->count != 0)ready_q->first_entry_loc++;
        if(ready_q->first_entry_loc >= NUM_OF_PRC)ready_q->first_entry_loc = 0;
        if(task->time_remaining <= 0){
            pthread_mutex_lock(&processes_completed_mutex);
            processes_completed++;
            int tat = calculate_turnaround(task->turnaround_time,t2);
            printf("Process: %d Completed in CPU: %d\nPriority: %d in Queue: rq%d\nService Time: %dms\nAccumulated Service Time: %dms\nTurnaround time:%dms\n\n\n",
                    task->pid,
                    id,
                    task->sched_type,
                    rqlevel,
                    task->time_slice,
                    task->accu_time_slice,
                    tat);
                    
            pthread_mutex_unlock(&processes_completed_mutex);

        }
        else {
            // Decrement sleep_avg (the "bonus")
            printf("Process: %d with Priority-%d BLOCKED in CPU %d from Queue RQ%d\n\n",
	            task->pid,
	            task->sched_type,
	            id,
	            rqlevel);
	        if(task->sched_type == SCHED_NORMAL){
                ticks = calculate_ticks(t1, t2);
                task->sleep_avg = max(task->sleep_avg - ticks, 0);
                task->dynamic_priority = calculate_dp(task->dynamic_priority, task->sleep_avg);
                if(task->dynamic_priority >= 130){
                    add_process(&rq2[id], task);
                }
                else{
                    add_process(&rq1[id], task);
                }
            }
            else{
                add_process(ready_q, task);
            }  
        }
        pthread_mutex_unlock(&cpu_mutex[id]); 
        return 1;
    }
    pthread_mutex_unlock(&cpu_mutex[id]);
    return 0;
}

/**
 * Thread Function for Producer
 */
void * producer_func(void * args){
    printf("This is the producer.\n");
    producer_created++;
    generate_processes();
}

/**
 * Generate processes and add them to their specific queue
 * NOTE: We changed time_remaining from 5-50 seconds to 2-10 seconds
 * NOTE: This ^ was done to prevent a possible 20 minute wait time (which would suck)
 */
void generate_processes(){
    int cpu = 0;
    for(int i = 0; i<NUM_OF_PRC;i++){
        task_struct process;

        process.sched_type = schedule_type[rand() % 5];
        process.pid = i;
       
        // Priority
        if (process.sched_type == SCHED_FIFO || process.sched_type == SCHED_RR) {
            process.static_priority = rand() % 100;
        }
        else if (process.sched_type == SCHED_NORMAL) {
            process.static_priority = (rand() % 40) + 100;
        }
        process.dynamic_priority = process.static_priority;
        
        // Time
        process.time_remaining = (rand() % (MILLI_SEC_IN_SEC*8)) + (MILLI_SEC_IN_SEC*2);
        if (process.sched_type == SCHED_FIFO) {
            process.time_slice = process.time_remaining;
        }
        else {
            process.time_slice = calculate_time_slice(process.dynamic_priority);
        }
        process.sleep_avg = 0;
        process.accu_time_slice = 0;
        
        gettimeofday(&process.turnaround_time,NULL);
        
        pthread_mutex_lock(&cpu_mutex[cpu]);

        // Add to Queue
        add_process_by_priority(&process, cpu);
        
        pthread_mutex_unlock(&cpu_mutex[cpu]); 
        printf("Process added to CPU: %d\n",cpu);
        cpu++;
        if(cpu==NUM_OF_CON) cpu=0;
    }
}

/**
 * Removes a process from the desired Queue (queue)
 */
task_struct remove_process_from_rq(queue_struct *queue) {
   task_struct task = queue->circ_buff[queue->last_entry_loc];
   queue->count--;
   queue->last_entry_loc--;
   if(queue->last_entry_loc < 0)queue->last_entry_loc = NUM_OF_PRC-1;
   return task;
}


/**
 * Adds a process (task) to the desired Queue (queue)
 */
void add_process(queue_struct *queue, task_struct *task){
    if(queue->count !=0) queue->last_entry_loc++;
    if(queue->last_entry_loc >= NUM_OF_PRC)queue->last_entry_loc = 0;
    queue->circ_buff[queue->last_entry_loc] = *task;
    queue->count++;
}

/**
 * Add a Process to it's appropriate Priority Queue
 */
void add_process_by_priority(task_struct *process, int cpu_id) {
    if (process->sched_type == SCHED_NORMAL && process->static_priority >= 130) {
        add_process(&rq2[cpu_id], process);
    }
    else if (process->sched_type == SCHED_NORMAL) {
        add_process(&rq1[cpu_id], process);
    }
    else {
        add_process(&rq0[cpu_id], process);
    }
}

/**
 * Thread Function for the Queue Balancer
 */
void * queue_balance_func(void * args){
    printf("This is the queue balancer.\n");
    int process_count_in_cpu[NUM_OF_CON];
    int min, max;
    task_struct t;
    
    for (int i = 0; i < NUM_OF_CON; i++) {
        pthread_mutex_lock(&cpu_mutex[i]);
    }
    
    while(processes_completed < NUM_OF_PRC) {
        for (int i = 0; i<NUM_OF_CON; i++) {
            process_count_in_cpu[i] = count_processes_in_cpu(i);
        }
        min = 0;
        max = 0;
        for (int i = 1; i<NUM_OF_CON; i++) {
            if (process_count_in_cpu[i] > process_count_in_cpu[max]) max = i;
            if (process_count_in_cpu[i] < process_count_in_cpu[min]) min = i;
        }
        
        //printf("About to enter while\n");
        if((process_count_in_cpu[max] - process_count_in_cpu[min]) >= 2) {            
            if (rq0[max].count > 0) {
                t = remove_process_from_rq(&rq0[max]);
                printf("Load Balancer took a process from rq0\n");
            }
            else if (rq1[max].count > 0) {
                t = remove_process_from_rq(&rq1[max]);
                printf("Load Balancer took a process from rq1\n");
            }
            else if (rq2[max].count > 0) {
                t = remove_process_from_rq(&rq2[max]);
                printf("Load Balancer took a process from rq2\n");
            }

            add_process_by_priority(&t, min);
            printf("Load Balancer moved one process from CPU-%d to CPU-%d\n\n", max, min);
        }
        
        for (int i = 0; i < NUM_OF_CON; i++) {
            pthread_mutex_unlock(&cpu_mutex[i]);
        }
        sleep(1);
    }
}

/**
 * Count the number of processes in a desired CPU
 */
int count_processes_in_cpu(int id) {
    return rq0[id].count+rq1[id].count+rq2[id].count;
}

/**
 * Calculate the Time Slice
 * NOTE: We have modified this calculation (Professor mentioned that this was allowed)
 * NOTE: We changed (140-sp) to (240-sp) in both lines
 */
int calculate_time_slice(int sp) {
    if (sp < 120) return (240 - sp) * 20 ;
    else return (240 - sp) * 5 ;
}

/**
 * Calculate the ticks, which is used for the sleep_avg
 * Times 3 for I/O operation.
 */
int calculate_ticks(struct timeval t1, struct timeval t2) {
    int time1 = (t1.tv_sec*MILLI_SEC_IN_SEC) + (t1.tv_usec/1000);
    int time2 = (t2.tv_sec*MILLI_SEC_IN_SEC) + (t2.tv_usec/1000);
    return (time2-time1) / 100;
}

/**
 * Calculate the Turnaround value
 */
int calculate_turnaround(struct timeval t1, struct timeval t2){
    int time1 = (t1.tv_sec*MILLI_SEC_IN_SEC) + (t1.tv_usec/1000);
    int time2 = (t2.tv_sec*MILLI_SEC_IN_SEC) + (t2.tv_usec/1000);
    return time2-time1;
}

/**
 * Calculate the Dynamic Priority value
 */
int calculate_dp(int previous_dp, int bonus) {
    return max(100, min(previous_dp - bonus + 5, 139));
}


/* Helper Functions (self-explanatory) */
int min(int a, int b) {return (a<=b ? a:b);}
int max(int a, int b) {return (a>b ? a:b);}

