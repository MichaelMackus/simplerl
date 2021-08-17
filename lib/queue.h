#ifndef RL_QUEUE_H
#define RL_QUEUE_H

struct rl_queue {
    void *data;
    int priority;
    struct rl_queue *next;
};
typedef struct rl_queue rl_queue;

// push to queue - if (*queue) == NULL this allocates & initializes the queue
// to free the queue use the "rl_pop" function until it returns NULL
void rl_push(rl_queue **queue, void *data, int priority);

// pop element from queue and free the old queue node
void *rl_pop(rl_queue **queue);

// peek at the data in the queue
void *rl_peek(const rl_queue *queue);

// return length of queue
int rl_queue_size(const rl_queue *queue);

// transform queue to array - make sure you free the array when you are done
void **rl_queue_to_array(rl_queue **queue);

#endif
