#ifndef RL_QUEUE_H
#define RL_QUEUE_H

struct _RL_Queue {
    void *data;
    int priority;
    struct _RL_Queue *next;
};
typedef struct _RL_Queue RL_Queue;

// push to queue - if (*queue) == NULL this allocates & initializes the queue
void rl_push(RL_Queue** queue, void *data, int priority);

// pop element from queue and free the old queue node
void* rl_pop(RL_Queue** queue);

// peek at the data in the queue
void* rl_peek(const RL_Queue* queue);

#endif
