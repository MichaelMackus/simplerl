#include "queue.h"

#include <stdlib.h>

void *rl_pop(rl_queue_t **queue)
{
    if (*queue == NULL)
        return NULL;

    rl_queue_t *oldHead = *queue;
    void *data = (*queue)->data;
    *queue = (*queue)->next;

    free(oldHead);

    return data;
}

void rl_push(rl_queue_t **queue, void *data, int priority)
{
    if (queue == NULL)
        return;

    if (*queue == NULL) {
        (*queue) = malloc(sizeof(rl_queue_t));
        (*queue)->data = data;
        (*queue)->priority = priority;
        (*queue)->next = NULL;
        return;
    }

    rl_queue_t *newNode = malloc(sizeof(rl_queue_t));
    if (newNode == NULL) return;
    newNode->data = data;
    newNode->priority = priority;
    newNode->next = NULL;

    // if newNode is more urgent then head, replace head
    if (priority < (*queue)->priority) {
        newNode->next = *queue;
        *queue = newNode;
        return;
    }

    rl_queue_t *current = *queue;
    while (current->next != NULL) {
        // found the spot to insert
        if (current->next->priority > priority)
            break;

        current = current->next;
    }

    newNode->next = current->next;
    current->next = newNode;
}

void *rl_peek(const rl_queue_t *queue)
{
    if (queue == NULL)
        return NULL;

    return queue->data;
}
