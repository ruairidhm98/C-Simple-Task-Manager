#include "task_system.h"
#include "queue.h"
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

/* Task system structure */
struct task_system {
  Queue **work_q; // the work queue
  pthread_t *threads; // threads used to start tasks
  unsigned int NUM_QUEUES; // number of queues
  volatile sig_atomic_t index; // index used to try and push to queue
  struct func_args *args; // array of arguments to functions
  void (*ts_asynch)(TaskSystem *ts, void (*fn)(void)); // function pointer to insert into work queue
  void (*ts_delete)(TaskSystem *ts); // function pointer to delete task system
  pthread_mutex_t mutex; // mutex used to protect shared data
};

/* Arguments passed into run */
struct func_args {
  TaskSystem *ts; // the task systsem itself
  int queue; // index to the queue the thread operates on
};

/* Runs the processes */
void *run(void *arg) {

  struct func_args *args = (struct func_args *)arg;
  unsigned int index = args->queue, i;
  void (*fnPtr)(void) = NULL;
  TaskSystem *ts = args->ts;

  /* Loop forever until notified we are done */
  while (true) {
    for (i = index; i != (ts->NUM_QUEUES); i++) {
      fnPtr = q_try_pop(ts->work_q[(index + i) % (ts->NUM_QUEUES)]);
      if (fnPtr)
        break;
    }
    if (!fnPtr) {
      fnPtr = q_pop(ts->work_q[index]);
      if (!fnPtr)
        pthread_exit(NULL);
    }
    fnPtr();
  }

  pthread_exit(NULL);
}

/* Returns a pointer to a task system object if successfull, NULL otherwise */
TaskSystem *ts_init(unsigned int numQueues) {

  unsigned int i;
  TaskSystem *ts;

  ts = (TaskSystem *)malloc(sizeof(TaskSystem));
  /* Print error message and return NULL if memory allocation fails */
  if (!ts) {
    fprintf(stderr, "Error: memory allocation failed\n");
    free((void *)ts);
    ts = NULL;
    return ts;
  }
  /* Make sure mutex creation was successfull */
  
  ts->NUM_QUEUES = numQueues;
  ts->index = 0;
  ts->work_q = (Queue **)malloc(sizeof(Queue *) * numQueues);
  if (!(ts->work_q)) {
    fprintf(stderr, "Error: memory allocation failed\n");
    free((void *)ts->work_q);
    free((void *)ts);
    ts = NULL;
    return NULL;
  }
  /* Create the queues */
  for (i = 0; i < numQueues; i++) {
    ts->work_q[i] = q_init();
    if (!(ts->work_q[i])) {
      free((void *)ts);
      ts = NULL;
      return NULL;
    }
  }
  ts->threads = (pthread_t *)malloc(sizeof(pthread_t) * numQueues);
  if (!(ts->threads)) {
    fprintf(stderr, "Error: memory allocation failed\n");
    ts_delete(ts);
    return NULL;
  }
  /* Create arguments to pass into run */
  ts->args = (struct func_args *)malloc(sizeof(struct func_args) * numQueues);
  if (!ts->args) {
    fprintf(stderr, "Error: memory allocation failed\n");
    free((void *)ts->args);
    ts_delete(ts);
    return NULL;
  }
  /* Set function pointers */
  ts->ts_asynch = ts_asynch;
  ts->ts_delete = ts_delete;
  /* Fill arguments */
  for (i = 0; i < numQueues; i++) {
    ts->args[i].ts = ts;
    ts->args[i].queue = i;
  }
  /* Spawn threads */
  for (i = 0; i < (ts->NUM_QUEUES); i++)
    /* Make sure the correct arguments are being passed to the relevant thread
     */
    if (pthread_create(&(ts->threads[i]), NULL, run, (void *)&(ts->args[i]))) {
      fprintf(stderr, "Error: failed to create thread %d\n", i + 1);
      ts_delete(ts);
      return NULL;
    }

  return ts;
}

/* Adds a task to one of the queues */
void ts_asynch(TaskSystem *ts, void (*fn)(void)) {

  unsigned int i, n;

  i = (ts->index)++  % ts->NUM_QUEUES;
  for (n = 0; n < ts->NUM_QUEUES; n++)
    if (q_try_push(ts->work_q[(i + n) % (ts->NUM_QUEUES)], fn))
      return;

  q_insert(ts->work_q[i % (ts->NUM_QUEUES)], fn);
}

/* Deletes a task system object */
void ts_delete(TaskSystem *ts) {

  unsigned int i;

  /* Notify the task system we are done */
  for (i = 0; i < (ts->NUM_QUEUES); i++)
    q_set_done(ts->work_q[i]);
  /* Wait for threads to finish */
  for (i = 0; i < (ts->NUM_QUEUES); i++)
    pthread_join(ts->threads[i], NULL);
  /* Free heap memory */
  for (i = 0; i < (ts->NUM_QUEUES); i++)
    q_delete(ts->work_q[i]);

  free((void *)ts->work_q);
  free((void *)ts->threads);
  free((void *)ts->args);
  free((void *)ts);
  ts = NULL;
}

/* Tester function */
void test() { printf("Hello world\n"); }
