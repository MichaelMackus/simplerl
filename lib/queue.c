#include "queue.h"

#include <stdlib.h>

void *rl_pop(RL_Queue **queue)
{
    if (*queue == NULL)
        return NULL;

    RL_Queue *oldHead = *queue;
    void *data = (*queue)->data;
    *queue = (*queue)->next;

    free(oldHead);

    return data;
}

void rl_push(RL_Queue **queue, void *data, int priority)
{
    if (queue == NULL)
        return;

    if (*queue == NULL) {
        (*queue) = malloc(sizeof(RL_Queue));
        (*queue)->data = data;
        (*queue)->priority = priority;
        (*queue)->next = NULL;
        return;
    }

    RL_Queue *newNode = malloc(sizeof(RL_Queue));
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

    RL_Queue *current = *queue;
    while (current->next != NULL) {
        // found the spot to insert
        if (current->next->priority > priority)
            break;

        current = current->next;
    }

    newNode->next = current->next;
    current->next = newNode;
}

void *rl_peek(const RL_Queue *queue)
{
    if (queue == NULL)
        return NULL;

    return queue->data;
}
