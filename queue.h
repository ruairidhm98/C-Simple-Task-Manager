#ifndef __QUEUE_H__
#define __QUEUE_H__

typedef struct q_elt Node;
typedef struct queue Queue;

/* Returns a pointer to an empty queue if successfull, NULL otherwise */
Queue *q_init();
/* Returns 1/0 if item was counted/not counter */
int q_insert(Queue *queue, void (*f)(void));
/* Returns the item popped off the front of the queue */
void (*q_pop(Queue *queue))(void);
/* Returns the front of the queue without removing it */
void (*q_front(Queue *queue))(void);
/* Deletes a queue object */
void q_delete(Queue *queue); 
/* Prints the contents of the queue */
void q_print(Queue *queue);
/* Returns the size of the queue */
unsigned long q_size(Queue *queue);
/* Sets done to true, so we know we are finished */
void q_set_done(Queue *queue);

#endif 


