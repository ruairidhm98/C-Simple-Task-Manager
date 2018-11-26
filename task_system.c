#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include "task_system.h"
#include "queue.h"

/* Task system structure */
struct task_system {
    bool done; // if done is false, then there is still work on the queue
    Queue *work_q; // the work queue
    pthread_t *threads; // threads used to start tasks 
    unsigned int NUM_THREADS; // the number of threads in the task system  
}; 

/* Runs the processes */
void *run(void *arg) {

    void (*fnPtr)(void);
    TaskSystem *ts;

    ts = (TaskSystem *) arg;
    /* Loop forever until notified we are done */
    while (true) {
        break;
        fnPtr = q_pop(ts -> work_q);    
        if (!fnPtr) pthread_exit(NULL);
        fnPtr();
    }

    pthread_exit(NULL);
}

/* Returns a pointer to a task system object if successfull, NULL otherwise */
TaskSystem *ts_init(unsigned int numThreads) {

    TaskSystem *ts;
    int i;

    ts = (TaskSystem *) malloc(sizeof(TaskSystem));
    /* Print error message and return NULL if memory allocation fails */
    if (!ts) {
        fprintf(stderr, "Error: memory allocation failed\n");
        free((void *) ts);
        ts = NULL;
        return ts;
    }   
    ts -> done = false;
    ts -> NUM_THREADS = numThreads;
    ts -> work_q = q_init();
    if (!(ts -> work_q)) {
        free((void *) ts);
        ts = NULL;
        return ts;
    }    
    ts -> threads = (pthread_t *) malloc(sizeof(pthread_t) * numThreads);
    if (!(ts -> threads)) {
        fprintf(stderr, "Error: memory allocation failed\n");
        ts_delete(ts);
        return ts;
    }
    /* Spawn threads */
    for (i = 0; i < numThreads; i++) 
        if (pthread_create(&(ts -> threads[i]), NULL, run, NULL)) {
            fprintf(stderr, "Error: failed to create thread %d\n", i+1);
            ts_delete(ts);
            return ts;
        }

    return ts;
}

/* Deletes a task system object */
void ts_delete(TaskSystem *ts) {
    
    int i;

    /* Wait for threads to finish */
    for (i = 0; i < ts -> NUM_THREADS; i++) 
        pthread_join(ts -> threads[i], NULL);
    /* Free heap memory */
    q_delete(ts -> work_q);
    free((void *) ts);
    free((void *) ts -> threads);
    ts = NULL;

}
