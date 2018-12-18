/* Simple task system which uses multiple queues, two threads are assigned to each queue */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include "task_system.h"
#include "queue.h"


/* Task system structure */
struct task_system {
    Queue **work_q; // the work queue
    pthread_t *threads; // threads used to start tasks 
    unsigned int NUM_QUEUES; // number of queues
    sig_atomic_t index; // index used to try and push to queue
    struct func_args *args; // array of arguments to functions
}; 

/* Arguments passed into run */
struct func_args {
    TaskSystem *ts; // the task systsem itself
    int queue; // index to the queue the thread operates on
};

/* Runs the processes */
void *run(void *arg) {

    struct func_args *args; 
    unsigned int index, i;  
    void (*fnPtr)(void);
    TaskSystem *ts;

    args = (struct func_args *) arg;
    index = args -> queue;
    ts = args -> ts;
    /* Loop forever until notified we are done */
    while (true) {

        for (i = 0; i != (ts -> NUM_QUEUES); i++) {
            fnPtr = q_try_pop(ts -> work_q[(index + i) % (ts -> NUM_QUEUES)]);
            if (fnPtr) break;
        }
        if (!fnPtr) {
            fnPtr = q_pop(ts -> work_q[index]);
            if (!fnPtr) pthread_exit(NULL);
        }

        fnPtr();
    }

    pthread_exit(NULL);
}

/* Returns a pointer to a task system object if successfull, NULL otherwise */
TaskSystem *ts_init(unsigned int numQueues) {

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
    
    ts -> NUM_QUEUES = numQueues;
    ts -> index = 0;
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
    ts -> threads = (pthread_t *) malloc(sizeof(pthread_t) * numQueues);
    if (!(ts -> threads)) {
        fprintf(stderr, "Error: memory allocation failed\n");
        ts_delete(ts);
        return ts;
    }

    /* Create arguments to pass into run */
    ts -> args = (struct func_args *) malloc(sizeof(struct func_args) * numQueues);
    if (!ts -> args) {
        fprintf(stderr, "Error: memory allocation failed\n");
        free((void *) ts -> args);
        ts_delete(ts);
        return ts;
    }
    /* Fill arguments */
    for (i = 0; i < numQueues; i++) {
        ts -> args[i].ts = ts;
        ts -> args[i].queue = i;
    }
    /* Spawn threads */
    for (i = 0; i < (ts -> NUM_QUEUES); i++) 
        /* Make sure the correct arguments are being passed to the relevant thread */
        if (pthread_create(&(ts -> threads[i]), NULL, run, (void *) &(ts -> args[i]))) {
            fprintf(stderr, "Error: failed to create thread %d\n", i+1);
            ts_delete(ts);
            return ts;
        }
    

    return ts;
}

/* Adds a task to one of the queues */
void ts_asynch(TaskSystem *ts, void (*fn)(void)) { 
    
    int i, n; 

    i = (ts -> index)++;
    for (n = 0; n < ts -> NUM_QUEUES; n++) 
        if (q_try_push(ts -> work_q[(i + n) % (ts -> NUM_QUEUES)], fn)) 
            return;
    
    q_insert(ts -> work_q[i % (ts -> NUM_QUEUES)], fn); 

}

/* Deletes a task system object */
void ts_delete(TaskSystem *ts) {
    
    int i;

    /* Notify the task system we are done */
    for (i = 0; i < (ts -> NUM_QUEUES); i++) q_set_done(ts -> work_q[i]);
    /* Wait for threads to finish */
    for (i = 0; i < (ts -> NUM_QUEUES); i++) pthread_join(ts -> threads[i], NULL);
    /* Free heap memory */
    for (i = 0; i < (ts -> NUM_QUEUES); i++) q_delete(ts -> work_q[i]);

    free((void *) ts -> work_q);
    free((void *) ts -> threads);
    free((void *)ts->args);
    free((void *) ts);
    ts = NULL;

}

/* Tester function */
void test() { printf("Hello world\n"); }

int main(int argc, char **argv) {

    TaskSystem *ts;
    int i;

    ts = ts_init(4);
    for (i = 0; i < 20; i++) ts_asynch(ts, test);
    ts_delete(ts);

    return 0;
}
