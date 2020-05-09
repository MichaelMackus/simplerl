#ifndef RL_QUEUE_H
#define RL_QUEUE_H

struct rl_queue {
    void *data;
    int priority;
    struct rl_queue *next;
};
typedef struct rl_queue rl_queue_t;

// push to queue - if (*queue) == NULL this allocates & initializes the queue
void rl_push(rl_queue_t **queue, void *data, int priority);

// pop element from queue and free the old queue node
void *rl_pop(rl_queue_t **queue);

// peek at the data in the queue
void *rl_peek(const rl_queue_t *queue);

#endif
