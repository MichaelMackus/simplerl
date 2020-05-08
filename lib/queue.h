#ifndef RL_QUEUE_H
#define RL_QUEUE_H

struct RL_queue {
    void *data;
    int priority;
    struct RL_queue *next;
};
typedef struct RL_queue RL_queue_t;

// push to queue - if (*queue) == NULL this allocates & initializes the queue
void rl_push(RL_queue_t **queue, void *data, int priority);

// pop element from queue and free the old queue node
void *rl_pop(RL_queue_t **queue);

// peek at the data in the queue
void *rl_peek(const RL_queue_t *queue);

#endif
