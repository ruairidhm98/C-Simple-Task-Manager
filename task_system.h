#ifndef __TASK_SYSTEM_H__
#define __TASK_SYSTEM_H__

typedef struct task_system TaskSystem;

/* Returns a pointer to an empty task system if successfull, NULL otherwise */
TaskSystem *ts_init(int numThreads);
/*  */


#endif