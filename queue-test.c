/// Testing the `queue` module.
///
/// To run the tests, first compile this file with the `queue.c`, while passing
/// a `-DQUEUE_MAX_LENGTH=5` flag to the compiler:
///
/// ```
/// $ clang -DQUEUE_MAX_LENGTH=5 queue-test.c queue.c -oqueue-test
/// ```
///
/// ... and the run it:
///
/// ```
/// $ ./queue-test
/// ```
///
/// On successful execution the return code will be zero; some output is
/// expected.

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

#if QUEUE_MAX_LENGTH != 5
#error Max queue length should be 5
#endif

/// This macro expects a pointer to a `Queue` object and a `reference` array,
/// which should be a stack-allocated array (like `uint32_t ref[] = {...};`).
#define CHECK_QUEUE(queue, reference)                                       \
    {                                                                       \
        unsigned len = sizeof(reference) / sizeof(reference[0]);            \
        assert(len == (queue)->size &&                                      \
               "Wrong array size passed to CHECK_QUEUE");                   \
        uint32_t* test_array = malloc(len * sizeof(uint32_t));              \
        queue_copy_to((queue), test_array);                                 \
        assert(memcmp(test_array, reference, len * sizeof(uint32_t)) == 0); \
        free(test_array);                                                   \
    }

static void make_initial(struct Queue* queue) {
    uint32_t initialize[] = {3, 4, 0, 1, 2};
    memcpy(queue->array, initialize, QUEUE_MAX_LENGTH * sizeof(uint32_t));
    queue->begin = 3;
    queue->size = 4;
    {
        uint32_t reference_array[] = {1, 2, 3, 4};
        CHECK_QUEUE(queue, reference_array);
    }
}

static void make_initial2(struct Queue* queue, uint32_t* initial,
                          unsigned len) {
    uint32_t zeroes[] = {0, 0, 0, 0, 0};
    memcpy(queue->array, zeroes, QUEUE_MAX_LENGTH * sizeof(uint32_t));
    memcpy(queue->array, initial, len * sizeof(uint32_t));
    queue->begin = 0;
    queue->size = len;
}

static void test_push_back() {
    struct Queue queue;
    make_initial(&queue);
    assert(queue_push_back(&queue, 15) == 0);
    {
        uint32_t reference_array[] = {15, 1, 2, 3, 4};
        CHECK_QUEUE(&queue, reference_array);
    }

    // Can't insert any other element since the queue is already full.
    assert(queue_push_back(&queue, 10) == -1);
    {
        // Check that nothing changed.
        uint32_t reference_array[] = {15, 1, 2, 3, 4};
        CHECK_QUEUE(&queue, reference_array);
    }

    // Test array wrapping.
    queue.begin = 0;
    queue.size -= 1;
    {
        uint32_t reference_array[] = {3, 4, 15, 1};
        CHECK_QUEUE(&queue, reference_array);
    }
    assert(queue_push_back(&queue, 24) == 0);
    assert(queue.begin == 4);
    {
        uint32_t reference_array[] = {24, 3, 4, 15, 1};
        CHECK_QUEUE(&queue, reference_array);
    }
}

static void test_pop_back() {
    struct Queue queue;
    make_initial(&queue);
    uint32_t value;
    assert(queue_pop_back(&queue, &value) == 0);
    assert(value == 1);
    assert(queue_pop_back(&queue, &value) == 0);
    assert(value == 2);
    assert(queue_pop_back(&queue, &value) == 0);
    assert(value == 3);
    assert(queue_pop_back(&queue, &value) == 0);
    assert(value == 4);
    // No more elements.
    assert(queue_pop_back(&queue, &value) == -1);
}

static void test_pop_front() {
    struct Queue queue;
    make_initial(&queue);
    uint32_t value;
    assert(queue_pop_front(&queue, &value) == 0);
    assert(value == 4);
    assert(queue_pop_front(&queue, &value) == 0);
    assert(value == 3);
    assert(queue_pop_front(&queue, &value) == 0);
    assert(value == 2);
    assert(queue_pop_front(&queue, &value) == 0);
    assert(value == 1);
    // No more elements.
    assert(queue_pop_front(&queue, &value) == -1);
}

static void test_find() {
    struct Queue queue;
    make_initial(&queue);
    unsigned index;
    assert(queue_find(&queue, 3, &index) == 0);
    assert(index == 2);
    assert(queue_find(&queue, 0, &index) == -1);
    assert(queue_find(&queue, 2, &index) == 0);
    assert(index == 1);
}

static void test_remove() {
    struct Queue queue;
    make_initial(&queue);
    queue_remove(&queue, 3);  // Removing the last one
    {
        uint32_t reference_array[] = {1, 2, 3};
        CHECK_QUEUE(&queue, reference_array);
    }
    queue_remove(&queue, 0);  // Removing the first one
    {
        uint32_t reference_array[] = {2, 3};
        CHECK_QUEUE(&queue, reference_array);
    }
    // Removing the rest
    queue_remove(&queue, 1);
    queue_remove(&queue, 0);
    assert(queue.size == 0);
}

static void test_merge() {
    struct Queue queue1, queue2;
    uint32_t initial1[] = {1, 3, 5};
    uint32_t initial2[] = {2, 4};
    make_initial2(&queue1, initial1, 3);
    make_initial2(&queue2, initial2, 2);

    queue_merge(&queue1, &queue2);
    {
        uint32_t reference_array[] = {1, 2, 3, 4, 5};
        CHECK_QUEUE(&queue1, reference_array);
    }
    assert(queue2.size == 0);
}

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    test_push_back();
    test_pop_back();
    test_pop_front();
    test_find();
    test_remove();
    test_merge();
}
