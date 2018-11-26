#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "task_system.h"
#include "queue.h"

/* Task system structure */
struct task_system {
    pthread_t *threads; // threads used to start tasks 
    const int NUM_THREADS; // the number of threads in the task system  
}; 


/* Runs the processes */
void run() {



}

/* Returns a pointer to a task system object if successfull, NULL otherwise */
