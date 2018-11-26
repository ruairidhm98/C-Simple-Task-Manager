#ifndef __TASK_SYSTEM_H__
#define __TASK_SYSTEM_H__

typedef struct task_system TaskSystem;

/* Returns a pointer to an empty task system if successfull, NULL otherwise */
TaskSystem *ts_init(unsigned int numThreads);
/* Launches a process on the queue */
void ts_asynch(void (*fn)(void));
/* Deletes a task system object */
void ts_delete(TaskSystem *ts);

#endif