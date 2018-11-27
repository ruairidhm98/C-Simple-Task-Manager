#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include "queue.h"

/* Node stored at each entry in the queue */
struct q_elt {
    void (*fn)(void); // data stored at node
    struct q_elt *next; // pointer to the next node
};

/* Queue implemented as a singly linked list */
struct queue {
    bool done; // if done is false, then there is still work on the queue
    Node *head; // pointer to the head of the list
    Node *tail; // pointer to the tail of the list
    unsigned long size; // size counter
    pthread_mutex_t mutex; // mutex used to ensure thread safety
    pthread_cond_t delete; // condition variable used to notify threads when deleting
};

/* Arguments used in insert */
struct args {
    Queue *queue; // queue being accessed
    void (*fn)(void); // function pointer being inserted
};

/* Returns a pointer to an empty queue if successfull, NULL otherwise */
Queue *q_init() {

    Queue *q;

    q = (Queue *) malloc(sizeof(Queue));
    /* Print error message and return failure if memory allocation failed */
    if (!q) {
        fprintf(stderr, "Error: memory allocation failed\n");
        free((void *) q);
        q = NULL;
        return NULL;
    }
    q -> done = false;
    q -> head = NULL;
    q -> tail = NULL;
    q -> size = 0;
    /* Print error message and return failure if mutex fails to create */
    if (pthread_mutex_init(&(q -> mutex), NULL)) {
        fprintf(stderr, "Error: mutex failed to create\n");
        free((void *) q);
        q = NULL;
        return q;
    }
    /* Print error message and return NULL if condition variable fails to create */
    if (pthread_cond_init(&(q -> delete), NULL)) {
        fprintf(stderr, "Error: condition variable failed to create\n");
        free((void *) q);
        pthread_mutex_destroy(&(q -> mutex));
        q = NULL;
    }

    return q;
}

/* Returns 1/0 if item was counted/not counted */
int q_insert(Queue *queue, void (*fn)(void)) {

    Node *newNode;

    newNode = (Node *) malloc(sizeof(Node));
    if (!newNode) {
        fprintf(stderr, "Error: memory allocation failed\n");
        free((void *) newNode);
        return 0;
    }
    newNode -> fn = fn;
    newNode -> next = NULL;
    /* Critical region */
    pthread_mutex_lock(&(queue -> mutex));
    /* Insertion at the head if queue is empty */
    if (!queue -> head) 
        queue -> head = newNode;
    /* This will only be executed after the 2nd attempt to insert */
    else if (!queue -> tail) {
        queue -> head -> next = newNode;
        queue -> tail = newNode;
    }
    /* Insert at the tail */
    else {
        queue -> tail -> next = newNode;
        queue -> tail = newNode;
    }
    queue -> size++;
    pthread_mutex_unlock(&(queue -> mutex));
    /* Notify threads that are sleeping */
    pthread_cond_signal(&(queue -> delete));

    return 1;
} 

/* Returns and removes the front of the queue */
void (*q_pop(Queue *queue))(void) {

    void (*result)(void);
    Node *temp;
    
    pthread_mutex_lock(&(queue -> mutex));
    /* Make the thread sleep until there is more work */
    if (!(queue -> head) || queue -> done) {
        printf("== Queue is empty ==\n");
        pthread_cond_wait(&(queue -> delete), &(queue -> mutex));;
    }
    /* If done was set to false, then deal with the case of the queue being empty */
    if (!(queue -> size)) return NULL;
    temp = queue -> head;
    result = temp -> fn;
    queue -> head = temp -> next;
    queue -> size--;
    free((void *) temp);
    pthread_mutex_unlock(&(queue -> mutex));

    return result;
}

/* Returns the front of the queue without removing it */
void (*q_front(Queue *queue))(void) {

    void (*fn)(void);

    fn = NULL;
    if (queue -> size) {
        pthread_mutex_lock(&(queue -> mutex));
        fn = queue -> head -> fn;
        pthread_mutex_unlock(&(queue -> mutex));
    }

    return fn;
}

/* Deletes a queue object */
void q_delete(Queue *queue) {

    Node *cursor, *tmp;

    pthread_mutex_lock(&(queue -> mutex));
    if (!queue) {
        pthread_mutex_unlock(&(queue -> mutex));
        return;
    }
    cursor = queue -> head;
    /* Iterate through the list deleting each node */
    while (cursor) {
        tmp = cursor;
        cursor = cursor -> next;
        free((void *) tmp);
    }
    free((void *) queue);
    queue -> head = NULL;
    queue -> tail = NULL;
    pthread_cond_destroy(&(queue -> delete));
    pthread_mutex_unlock(&(queue -> mutex));
    pthread_mutex_destroy(&(queue -> mutex));

}

/* Prints the contents of the queue */
void q_print(Queue *queue) {

    Node *cursor;

    pthread_mutex_lock(&(queue -> mutex));
    cursor = queue -> head;
    /* 
     * Iterate through the queue, printing the pointer to a function stored
     * at each node
     */
    while (cursor) {
        /* Reached the end of the queue */
        if (!(cursor -> next)) {
            printf("%p\n", cursor -> fn);
            break;
        }
        printf("%p -> ", cursor -> fn);
        cursor = cursor -> next;
    }
    pthread_mutex_unlock(&(queue -> mutex));

}

/* Prints the contents of the queue */
unsigned long q_size(Queue *queue) { return queue -> head ? queue -> size : -1; }

/* Set's done to true, to let threads know we are finished */
void q_set_done(Queue *queue) {
    pthread_mutex_lock(&(queue -> mutex));
    queue -> done = true;
    pthread_mutex_unlock(&(queue -> mutex));
    pthread_cond_broadcast(&(queue -> delete));  
}

/* Safe insert */
void *insert(void *arg) {

    Queue *queue;
    void (*fn)(void);
    struct args *arguments;

    arguments = (struct args *) arg;
    queue = arguments -> queue;
    fn = arguments -> fn;
    q_insert(queue, fn);

    pthread_exit(NULL);
}

/* Safe pop */
void *pop(void *arg) {

    Queue *queue;
    void (*result)(void);

    queue = (Queue *) arg;
    result = q_pop(queue);

    pthread_exit((void *) result);
}
