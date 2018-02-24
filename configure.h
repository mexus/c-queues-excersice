#pragma once

#ifndef QUEUE_MAX_LENGTH
/// A maximum length of a queue.
#define QUEUE_MAX_LENGTH 10
#endif  // QUEUE_MAX_LENGTH

/// First-in-first-out operation mode.
#define QUEUE_MODE_FIFO 1

/// Last-in-first-out operation mode.
#define QUEUE_MODE_LIFO 2

#ifndef QUEUE_MODE
/// Defines a queue mode.
#define QUEUE_MODE QUEUE_MODE_LIFO
#endif  // QUEUE_MODE
