#include "queue.h"

#include <stdlib.h>
#include <assert.h>

// TODO any way to have better error checking when user passes only a
// TODO pointer? (and not pointer to pointer?)
void *rl_pop(rl_queue **queue)
{
    if (*queue == NULL)
        return NULL;

    rl_queue *oldHead = *queue;
    void *data = (*queue)->data;
    *queue = (*queue)->next;

    free(oldHead);

    return data;
}

void rl_push(rl_queue **queue, void *data, int priority)
{
    if (queue == NULL)
        return;

    if (*queue == NULL) {
        (*queue) = malloc(sizeof(rl_queue));
        (*queue)->data = data;
        (*queue)->priority = priority;
        (*queue)->next = NULL;
        return;
    }

    rl_queue *newNode = malloc(sizeof(rl_queue));
    if (newNode == NULL) return;
    newNode->data = data;
    newNode->priority = priority;
    newNode->next = NULL;

    // if newNode is more urgent then head, replace head
    if (priority <= (*queue)->priority) {
        newNode->next = *queue;
        *queue = newNode;
        return;
    }

    // TODO opposite order?
    rl_queue *current = *queue;
    while (current->next != NULL) {
        // found the spot to insert
        if (current->next->priority > priority)
            break;

        current = current->next;
    }

    newNode->next = current->next;
    current->next = newNode;
}

void *rl_peek(const rl_queue *queue)
{
    if (queue == NULL)
        return NULL;

    return queue->data;
}

int rl_queue_size(const rl_queue *queue)
{
    // get size of queue
    int length = 0;
    rl_queue *tip = queue;
    while (tip) {
        ++length;
        tip = tip->next;
    }

    return length;
}

void **rl_queue_to_array(rl_queue **queue)
{
    // get size of queue
    int data_size = 0;
    rl_queue *tip = *queue;
    while (tip) {
        data_size += sizeof(tip->data);
        tip = tip->next;
    }

    if (data_size <= 0) {
        // free queue
        while (rl_pop(queue)) {}

        return NULL;
    }

    void **arr = malloc(data_size);

    if (arr == NULL) {
        // free queue
        while (rl_pop(queue)) {}

        return NULL;
    }

    void **current = arr;
    void *data;
    int i = 0;
    while (data = rl_pop(queue)) {
        *current = data;
        ++current;
    }

    return arr;
}

void rl_array_to_queue(rl_queue **queue, void **data, int length)
{
    for (int i = 0; i < length; ++i) {
        rl_push(queue, data[i], i);
    }

    free(data);

    return queue;
}
