/* Simple task system which uses multiple queues, two threads are assigned to each queue */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include "task_system.h"
#include "queue.h"

#define THREADS_PER_QUEUE 2

/* Task system structure */
struct task_system {
    Queue **work_q; // the work queue
    pthread_t *threads; // threads used to start tasks 
    unsigned int NUM_THREADS; // the number of threads in the task system  
    unsigned int NUM_QUEUES; // number of queues
}; 

/* Arguments passed into run */
struct func_args {
    TaskSystem *ts; // the task systsem itself
    int queue; // index to the queue the thread operates on
};

/* Runs the processes */
void *run(void *arg) {

    struct func_args *args;
    void (*fnPtr)(void);
    unsigned int index;
    TaskSystem *ts;

    args = (struct func_args *) arg;
    index = args -> queue;
    ts = args -> ts;
    /* Loop forever until notified we are done */
    while (true) {
        fnPtr = q_pop(ts -> work_q[index]);    
        if (!fnPtr) pthread_exit(NULL);
        fnPtr();
    }

    pthread_exit(NULL);
}

/* Returns a pointer to a task system object if successfull, NULL otherwise */
TaskSystem *ts_init(unsigned int numQueues) {

    struct func_args *args;
    unsigned int i, j;
    TaskSystem *ts; 

    ts = (TaskSystem *) malloc(sizeof(TaskSystem));
    /* Print error message and return NULL if memory allocation fails */
    if (!ts) {
        fprintf(stderr, "Error: memory allocation failed\n");
        free((void *) ts);
        ts = NULL;
        return ts;
    } 
    
    ts -> NUM_THREADS = numQueues * THREADS_PER_QUEUE;
    ts -> NUM_QUEUES = numQueues;
    ts -> work_q = (Queue **) malloc(sizeof(Queue *) * numQueues);
    
    if (!(ts -> work_q)) {
        fprintf(stderr, "Error: memory allocation failed\n");
        free((void *) ts);
        free((void *) ts -> work_q);
        ts = NULL;
        return ts;
    }
    /* Create the queues */
    for (i = 0; i < numQueues; i++) {
        ts -> work_q[i] = q_init();
        if (!(ts -> work_q[i])) {
            free((void *) ts);
            ts = NULL;
            return ts;
        }    
    }
    ts -> threads = (pthread_t *) malloc(sizeof(pthread_t) * ts -> NUM_THREADS);
    if (!(ts -> threads)) {
        fprintf(stderr, "Error: memory allocation failed\n");
        ts_delete(ts);
        return ts;
    }

    /* Create arguments to pass into run */
    args = (struct func_args *) malloc(sizeof(struct func_args) * numQueues);
    if (!args) {
        fprintf(stderr, "Error: memory allocation failed\n");
        free((void *) args);
        ts_delete(ts);
        return ts;
    }
    /* Fill arguments */
    for (i = 0; i < numQueues; i++) {
        args[i].ts = ts;
        args[i].queue = i;
    }
    /* Spawn threads */
    for (i = 0, j = 0; i < (ts -> NUM_THREADS); i++) {
        /* Make sure the correct arguments are being passed to the relevant thread */
        if (!(i % THREADS_PER_QUEUE) && i) j++;
        if (pthread_create(&(ts -> threads[i]), NULL, run, (void *) &(args[j]))) {
            fprintf(stderr, "Error: failed to create thread %d\n", i+1);
            ts_delete(ts);
            return ts;
        }

    }

    return ts;
}

/* Adds a task to one of the queues */
void ts_asynch(TaskSystem *ts, void (*fn)(void)) { 
    static int queue = 0;
    q_insert(ts -> work_q[queue++ % (ts -> NUM_QUEUES)], fn); 
}

/* Deletes a task system object */
void ts_delete(TaskSystem *ts) {
    
    int i;

    /* Notify the task system we are done */
    for (i = 0; i < (ts -> NUM_QUEUES); i++) q_set_done(ts -> work_q[i]);
    /* Wait for threads to finish */
    for (i = 0; i < (ts -> NUM_THREADS); i++) pthread_join(ts -> threads[i], NULL);
    /* Free heap memory */
    for (i = 0; i < (ts -> NUM_QUEUES); i++) q_delete(ts -> work_q[i]);

    free((void *)ts -> work_q);
    free((void *) ts);
    free((void *) ts -> threads);
    ts = NULL;

}

/* Tester function */
void test() { printf("Hello world\n"); }

int main() {

    TaskSystem *ts;

    ts = ts_init(2);
    ts_asynch(ts, test);
    ts_asynch(ts, test);
    ts_asynch(ts, test);
    ts_asynch(ts, test);
    ts_asynch(ts, test);
    ts_asynch(ts, test);

    ts_delete(ts);

    return 0;
}
