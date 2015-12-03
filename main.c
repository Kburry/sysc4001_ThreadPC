#include "struct.h"


void * consumer_func(void * args);
void * producer_func(void * args);
void * queue_balance_func(void * args);

void generate_processes();
void add_process(queue_struct *queue, task_struct task);
int check_queue(queue_struct *ready_q, int id);

int calculate_ticks(struct timeval t1, struct timeval t2);
int calculate_dp(int previous_dp, int bonus);

int min(int a, int b);
int max(int a, int b);

int producer_created = 0;
int consumers_created = 0;
int processes_completed = 0;

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
    
    srand(time(NULL)); // Random random!
    
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
        perror("Thread creation failed for  Producer.");
        exit(EXIT_FAILURE);
    }
    
    while(!producer_created);
    //balancer
    res = pthread_create(&balancer_thread, &thread_attr, queue_balance_func, (void *) NULL);
    if (res != 0){
        perror("Thread creation failed for queue balancer.");
        exit(EXIT_FAILURE);
    }
    while(1){
    };
}

/**
 * Thread Function for Consumer(s)
 */
void * consumer_func(void * args){
    int id = (intptr_t) args;
    
    queue_struct *ready_q0 = &rq0[id];
    queue_struct *ready_q1 = &rq1[id];
    queue_struct *ready_q2 = &rq2[id];

    printf("This is the consumer %d.\n",id);
    consumers_created++;
    while(processes_completed<20){
        if (check_queue(ready_q0, id)) printf("In rq0\n");
        else if (check_queue(ready_q1, id)) printf("In rq1\n");
        else if (check_queue(ready_q2, id)) printf("In rq2\n");
        else{
            sleep(1);
        }
    }
}

int check_queue(queue_struct *ready_q, int id) {
    int ticks;
    if(ready_q->count != 0){
        struct timeval t1, t2;
        
        gettimeofday(&t2, NULL);

        task_struct *task = &ready_q->circ_buff[ready_q->first_entry_loc];
        
        // Increment sleep_avg (bonus)
        ticks = calculate_ticks(task->last_time_used, t2);
        task->sleep_avg = min(task->sleep_avg + ticks, MAX_SLEEP_AVG);
        
        task->dynamic_priority = calculate_dp(task->dynamic_priority, task->sleep_avg);

        gettimeofday(&t1, NULL);
        usleep(task->time_remaining * 1000);
        gettimeofday(&t2, NULL);
 
        // Decrement sleep_avg (bonus)
        ticks = calculate_ticks(t1, t2);
        task->sleep_avg = max(task->sleep_avg - ticks, 0);
        
        processes_completed++;
        printf("handled process number %d in consumer %d. ",processes_completed,id);
        
        ready_q->first_entry_loc++;
        ready_q->count--;
        return 1;
    }
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
 * Thread Function for Queue Balance Function
 */
void * queue_balance_func(void * args){
    printf("This is the queue balancer.\n");
}


/**
 * Generate processes and add them to their specific queue
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
        process.time_remaining = rand() % (MILLI_SEC_IN_SEC*10);
        if (process.sched_type == SCHED_FIFO) {
            process.time_slice = process.time_remaining;
        }
        else {
            process.time_slice = rand() % MILLI_SEC_IN_SEC;
        }
        process.sleep_avg = 0;
        
        // Add to Queue
        if (process.sched_type == SCHED_NORMAL && process.static_priority >= 130) {
            add_process(&rq2[cpu], process);
        }
        else if (process.sched_type == SCHED_NORMAL) {
            add_process(&rq1[cpu], process);
        }
        else {
            add_process(&rq0[cpu], process);
        }
        
        printf("Process added to CPU: %d\n",cpu);
        cpu++;
        if(cpu==NUM_OF_CON) cpu=0;
    }
}

void add_process(queue_struct *queue, task_struct task){
    queue->last_entry_loc++; 
    queue->circ_buff[queue->last_entry_loc] = task;
    if(queue->last_entry_loc >= NUM_OF_PRC)queue->last_entry_loc = 0;
    queue->count++;
}



int calculate_time_slice(int sp) {
    if (sp < 120) return (140 - sp) * 20;
    else return (140 - sp) * 5;
}

int calculate_ticks(struct timeval t1, struct timeval t2) {
    int time1 = (t1.tv_sec*MILLI_SEC_IN_SEC) + (t1.tv_usec/1000);
    int time2 = (t2.tv_sec*MILLI_SEC_IN_SEC) + (t2.tv_usec/1000);
    return (time2-time1) / 100;
}

int calculate_dp(int previous_dp, int bonus) {
    return max(100, min(previous_dp - bonus + 5, 139));
}

int min(int a, int b) {return (a<=b ? a:b);}
int max(int a, int b) {return (a>b ? a:b);}





