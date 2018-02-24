#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

#define MIN(a, b) ((a) < (b)) ? (a) : (b)
#define MAX(a, b) ((a) > (b)) ? (a) : (b)

void queue_init(struct Queue *queue) {
    queue->size = 0;
    queue->begin = 0;
    memset(queue->array, 0, QUEUE_MAX_LENGTH * sizeof(uint32_t));
}

int queue_push_back(struct Queue *queue, uint32_t value) {
    if (queue->size == QUEUE_MAX_LENGTH) {
        fprintf(stderr,
                "Can't enqueue an element since the capacity of the queue has "
                "been reached\n");
        return -1;
    }
    if (queue->begin == 0) {
        queue->begin = QUEUE_MAX_LENGTH - 1;
    } else {
        queue->begin -= 1;
    }
    queue->array[queue->begin] = value;
    queue->size += 1;
    return 0;
}

int queue_pop_back(struct Queue *queue, uint32_t *value) {
    if (queue->size == 0) {
        fprintf(stderr, "Can't pop an element: the queue is empty\n");
        return -1;
    }
    *value = queue->array[queue->begin];
    if (queue->begin == QUEUE_MAX_LENGTH - 1) {
        queue->begin = 0;
    } else {
        queue->begin += 1;
    }
    queue->size -= 1;
    return 0;
}

int queue_pop_front(struct Queue *queue, uint32_t *value) {
    if (queue->size == 0) {
        fprintf(stderr, "Can't pop an element: the queue is empty\n");
        return -1;
    }
    *value = queue_get_value(queue, queue->size - 1);
    queue->size -= 1;
    return 0;
}

int queue_find(const struct Queue *queue, uint32_t value, unsigned *index) {
    for (unsigned i = 0; i != queue->size; ++i) {
        if (queue_get_value(queue, i) == value) {
            *index = i;
            return 0;
        }
    }
    return -1;
}

void queue_remove(struct Queue *queue, unsigned index) {
    assert(index < queue->size);
    queue->size -= 1;
    for (unsigned _i = queue->begin + index; _i != queue->begin + queue->size;
         ++_i) {
        unsigned i = _i % QUEUE_MAX_LENGTH;
        queue->array[i] = queue->array[(_i + 1) % QUEUE_MAX_LENGTH];
    }
}

void queue_merge(struct Queue *queue_into, struct Queue *queue2) {
    unsigned total_len = queue_into->size + queue2->size;
    assert(total_len <= QUEUE_MAX_LENGTH);
    const struct Queue *min_queue =
        (queue_into->size < queue2->size) ? queue_into : queue2;
    const struct Queue *max_queue =
        (queue_into->size >= queue2->size) ? queue_into : queue2;
    uint32_t *new_queue_array = malloc(total_len);
    for (unsigned i = 0; i != min_queue->size; ++i) {
        new_queue_array[2 * i] = queue_get_value(queue_into, i);
        new_queue_array[2 * i + 1] = queue_get_value(queue2, i);
    }
    for (unsigned i = min_queue->size; i != max_queue->size; ++i) {
        new_queue_array[min_queue->size + i] = queue_get_value(max_queue, i);
    }
    queue_into->begin = 0;
    queue_into->size = total_len;
    memcpy(queue_into->array, new_queue_array, total_len * sizeof(uint32_t));
    queue2->size = 0;
    free(new_queue_array);
}

uint32_t queue_get_value(const struct Queue *queue, unsigned index) {
    assert(index < queue->size);
    return queue->array[(queue->begin + index) % QUEUE_MAX_LENGTH];
}

void queue_copy_to(const struct Queue *queue, uint32_t *destination) {
    if (queue->begin + queue->size <= QUEUE_MAX_LENGTH) {
        memcpy(destination, queue->array + queue->begin,
               queue->size * sizeof(uint32_t));
        return;
    }

    unsigned first_portion_len = QUEUE_MAX_LENGTH - queue->begin;
    unsigned second_portion_len = queue->size + queue->begin - QUEUE_MAX_LENGTH;

    memcpy(destination, queue->array + queue->begin,
           first_portion_len * sizeof(uint32_t));
    memcpy(destination + first_portion_len, queue->array,
           second_portion_len * sizeof(uint32_t));
}
