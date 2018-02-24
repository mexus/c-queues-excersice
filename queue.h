#pragma once

#include <inttypes.h>

#include "configure.h"

/// A double-ended continuous storage.
///
/// Internally the queue is stored in a continuous vector, but there might be a
/// discontinuity in the queue itself. For better understanding please look at
/// the following internal representation example.
///
/// ```
/// QUEUE_MAX_LENGTH = 7
/// queue.begin      = 4
/// queue.size       = 5
///                                           #0        #1        #2
///   #3        #4         *         *
///    ^         ^         ^         ^         ^         ^         ^
///    |         |         |         |         |         |         |
/// array[0]  array[1]  array[2]  array[3]  array[4]  array[5]  array[6]
/// ```
///
/// This storage scheme has been selected in order to avoid allocations/moving
/// memory when adding or removing data to/from the both ends.
///
/// For example, to add an element to the fron we simply move the `begin` value
/// to the left, and if it is already zero, we just swap it by moving to the end
/// of the array.
struct Queue {
    /// Number of the front element in the array.
    unsigned begin;
    /// Current size of the array.
    unsigned size;
    /// The storage array.
    uint32_t array[QUEUE_MAX_LENGTH];
};

/// Initializes an empty queue.
void queue_init(struct Queue *queue);

/// Pushes a value to the 'back' (i.e. 'begin') of a queue.
int queue_push_back(struct Queue *queue, uint32_t value);

/// Pops the 'back' (i.e. 'first') element of the queue.
int queue_pop_back(struct Queue *queue, uint32_t *value);

/// Pops the 'front' (i.e. 'last') element from the queue.
int queue_pop_front(struct Queue *queue, uint32_t *value);

/// Finds an element in a queue.
int queue_find(const struct Queue *queue, uint32_t value, unsigned *index);

/// Removes a given index from a queue. The index is expected to lie withint the
/// bounds of the queue.
void queue_remove(struct Queue *queue, unsigned index);

/// Merges two queues into the first one. Their combined size should not be
/// greater than `QUEUE_MAX_LENGTH`. The second queue will be emptied after the
/// merge.
void queue_merge(struct Queue *queue_into, struct Queue *queue2);

/// Returns a value stored at a given index. Index should lie within the bounds
/// of the queue.
uint32_t queue_get_value(const struct Queue *queue, unsigned index);

/// Copies the contents of a queue into an array. Size of the array should match
/// the size of the queue.
///
/// The method is here mostly for the test purposes.
void queue_copy_to(const struct Queue *queue, uint32_t *destination);
