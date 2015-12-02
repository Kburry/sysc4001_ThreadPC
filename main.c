#include "struct.h"


void * consumer_func(void * args);
void * producer_func(void * args);
void * queue_balance_func(void * args);
void generate_processes();
void add_process(queue_struct *queue, task_struct task);

int producer_created = 0;
int consumers_created = 0;

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

void * consumer_func(void * args){
    int id = (intptr_t) args;
    queue_struct *queue = &rq0[id];
    int handled = 0;
    printf("This is the consumer %d.\n",id);
    consumers_created++;
    while(handled<5){
        if(queue->count != 0){
            handled++;
            queue->first_entry_loc++;
            queue->count--;
            task_struct task = queue->circ_buff[queue->first_entry_loc];
            usleep(task.time_remaining * 1000000);
            printf("handled process number %d in consumer %d.\n",handled,id);
        }else{
            sleep(1);
        }
    }
}
void * producer_func(void * args){
    printf("This is the producer.\n");
    producer_created++;
    generate_processes();
}
void * queue_balance_func(void * args){
    printf("This is the queue balancer.\n");
}

void generate_processes(){
    int rq = 0;
    for(int i = 0; i<NUM_OF_PRC;i++){
        task_struct process;
        process.pid = i;
        process.static_priority = rand() % 140;
        process.dynamic_priority = process.static_priority;
        process.time_remaining = rand() % 10;
        process.time_slice = rand() % 100000;
        
        add_process(&rq0[rq],process);
        printf("process added to %d\n",rq);
        rq++;
        if(rq==NUM_OF_CON) rq=0;
    }
}

void add_process(queue_struct *queue, task_struct task){
    queue->last_entry_loc++; 
    queue->circ_buff[queue->last_entry_loc] = task;
    if(queue->last_entry_loc >= NUM_OF_PRC)queue->last_entry_loc = 0;
    queue->count++;
}
