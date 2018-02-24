/// Queues managing program. For usage details please refer to the `cli_help`
/// function.
///
/// The program could be configured during the compilation with the following
/// options:
///
/// * QUEUE_MAX_LENGTH: a maximum length of the queues (defaults to 10). Please
///                     don't set it to be more than UINT_MAX, otherwise the
///                     program will misbehave.
/// * QUEUE_MODE: queues operation mode: FIFO (1) or LIFO (2).
///
/// # Building
///
/// To build the main program, compile `queue.c` with `cli.c` into a binary,
/// like
///
/// ```
/// $ clang queue.c cli.c -ocli
/// ```

#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "configure.h"
#include "queue.h"

/// Returns the current queue mode as a string.
static const char *cli_queue_mode_string();

/// Converts a hexadecimal string representation into a command id.
static int cli_command_from_arg(const char *arg);

/// A helper function to extract a queue number from the arguments.
static int cli_get_queue_number(const char *arg);

/// Adds an element to a queue.
///
/// The functions expects 3 arguments to be presented in the `argv` array (hence
/// `argc` is expected to be equal to 3):
///
/// * command id,
/// * queue number,
/// * element.
static int cli_add_to_queue(int argc, char **argv, struct Queue queues[2]);

/// Finds and removes an element from a queue.
///
/// The functions expects 3 arguments to be presented in the `argv` array (hence
/// `argc` is expected to be equal to 3):
///
/// * command id,
/// * queue number,
/// * element.
static int cli_remove_from_queue(int argc, char **argv, struct Queue queues[2]);

/// Prints out a length and contents of a queue.
static int cli_print_size_and_contents(int argc, char **argv,
                                       struct Queue queues[2]);

/// Prints out contents of a queue.
static int cli_print_contents(int argc, char **argv, struct Queue queues[2]);

/// Dequeues an element out of a queue.
static int cli_dequeue(int argc, char **argv, struct Queue queues[2]);

/// Finds all elements in a queue which have a specified bit set.
static int cli_find_bit(int argc, char **argv, struct Queue queues[2]);

/// Merges both queues into the first one.
static int cli_merge_queues(struct Queue queues[2]);

/// Prints a help message.
static void cli_help(const char *cmd_name);

/// Loads a queue from a file.
static void load_queue(struct Queue *queue, const char *file_name);

/// Saves a queue into a given file. For reasons of simplicity the function
/// silently ignores I/O errors.
static void save_queue(const struct Queue *queue, const char *file_name);

const char *cli_queue_mode_string() {
#if QUEUE_MODE == QUEUE_MODE_FIFO
    return "FIFO";
#elif QUEUE_MODE == QUEUE_MODE_LIFO
    return "LIFO";
#endif
}

int cli_command_from_arg(const char *arg) {
    char *endptr;
    long command_id = strtol(arg, &endptr, 16);
    if (command_id < 0 || command_id > 0x06) {
        fprintf(stderr,
                "Command should be a positive integer not greater than 5\n");
        return -1;
    }
    if (*endptr == '\0') {
        // This means the string has been fully converted into a value.
        return command_id;
    }
    fprintf(stderr, "Can't convert an argument [%s] into a hex\n", arg);
    return -1;
}

int cli_get_queue_number(const char *arg) {
    int queue_number = strtol(arg, NULL, 0);
    if (queue_number != 1 && queue_number != 2) {
        fprintf(stderr, "Number of the queue should be either 1 or 2\n");
        return -1;
    }
    return queue_number - 1;
}

int cli_add_to_queue(int argc, char **argv, struct Queue queues[2]) {
    if (argc < 3) {
        fprintf(stderr, "Command '%s' expects 2 args: <queue> <element>\n",
                argv[0]);
        return -1;
    }
    int queue_number = cli_get_queue_number(argv[1]);
    if (queue_number == -1) {
        return -1;
    }
    struct Queue *queue = &queues[queue_number];
    uint32_t element = strtoull(argv[2], NULL, 0);
    return queue_push_back(queue, element);
}

int cli_remove_from_queue(int argc, char **argv, struct Queue queues[2]) {
    if (argc < 3) {
        fprintf(stderr, "Command '%s' expects 2 args: <queue> <element>\n",
                argv[0]);
        return -1;
    }
    int queue_number = cli_get_queue_number(argv[1]);
    if (queue_number == -1) {
        return -1;
    }
    struct Queue *queue = &queues[queue_number];
    uint32_t value = strtoull(argv[2], NULL, 0);
    unsigned index;
    if (queue_find(queue, value, &index) == -1) {
        fprintf(stderr, "Command '%s': can't find %" PRIi32 " in the queue %i",
                argv[0], value, queue_number);
        return -1;
    }
    queue_remove(queue, index);
    return 0;
}

int cli_print_size_and_contents(int argc, char **argv, struct Queue queues[2]) {
    if (argc < 2) {
        fprintf(stderr, "Command '%s' expects 1 arg: <queue>\n", argv[0]);
        return -1;
    }
    int queue_number = cli_get_queue_number(argv[1]);
    if (queue_number == -1) {
        return -1;
    }
    const struct Queue *queue = &queues[queue_number];
    printf("Queue size: %u\nContents:", queue->size);
    for (unsigned i = 0; i != queue->size; ++i) {
        printf(" %" PRIi32, queue_get_value(queue, i));
    }
    printf("\n");
    return 0;
}

int cli_print_contents(int argc, char **argv, struct Queue queues[2]) {
    if (argc < 2) {
        fprintf(stderr, "Command '%s' expects 1 arg: <queue>\n", argv[0]);
        return -1;
    }
    int queue_number = cli_get_queue_number(argv[1]);
    if (queue_number == -1) {
        return -1;
    }
    const struct Queue *queue = &queues[queue_number];
    for (unsigned i = 0; i != queue->size; ++i) {
        printf("%" PRIi32 " ", queue_get_value(queue, i));
    }
    printf("\n");
    return 0;
}

int cli_dequeue(int argc, char **argv, struct Queue queues[2]) {
    if (argc < 2) {
        fprintf(stderr, "Command '%s' expects 1 arg: <queue>\n", argv[0]);
        return -1;
    }
    int queue_number = cli_get_queue_number(argv[1]);
    if (queue_number == -1) {
        return -1;
    }
    struct Queue *queue = &queues[queue_number];
    uint32_t value;
    int rc =
#if QUEUE_MODE == QUEUE_MODE_FIFO
        queue_pop_back(queue, &value);
#elif QUEUE_MODE == QUEUE_MODE_LIFO
        queue_pop_front(queue, &value);
#endif
    if (rc == -1) {
        return -1;
    }
    printf("%" PRIi32 "\n", value);
    return 0;
}

int cli_find_bit(int argc, char **argv, struct Queue queues[2]) {
    if (argc < 3) {
        fprintf(stderr, "Command '%s' expects 2 arg: <queue> <bit>\n", argv[0]);
        return -1;
    }
    int queue_number = cli_get_queue_number(argv[1]);
    if (queue_number == -1) {
        return -1;
    }
    const struct Queue *queue = &queues[queue_number];
    unsigned long long bit_number = strtoull(argv[2], NULL, 0);
    if (bit_number > 32) {
        fprintf(stderr,
                "Command '%s': bit number should be no greater than 32\n",
                argv[0]);
        return -1;
    }
    uint32_t mask = ((uint32_t)1) << bit_number;
    for (unsigned i = 0; i != queue->size; ++i) {
        uint32_t item = queue_get_value(queue, i);
        if (item & mask) {
            printf("%" PRIi32 " ", item);
        }
    }
    printf("\n");
    return 0;
}

static int cli_merge_queues(struct Queue queues[2]) {
    if (queues[0].size + queues[1].size >= QUEUE_MAX_LENGTH) {
        fprintf(
            stderr,
            "Can't merge queues since their combined size exceeds the limit\n");
        return -1;
    }
    queue_merge(&queues[0], &queues[1]);
    return 0;
}

void cli_help(const char *cmd_name) {
    printf(
        "Queues manager\n"
        "\n"
        "Usage: %s <command> [<args>...]\n"
        "\n"
        "The available commands are:\n"
        "    0x00 <queue> <element>  Add an <element> to a <queue>\n"
        "    0x01 <queue> <element>  Remove an <element> from a <queue>\n"
        "    0x02 <queue>            Print size and contents of a <queue>\n"
        "    0x03 <queue>            Print contents of a <queue>\n"
        "    0x04                    Merge the queues in a check pattern, see\n"
        "                            details below\n"
        "    0x05 <queue> <bit>      Find elements in a <queue> which have a "
        "bit\n"
        "                            number <bit> set to 1\n"
        "    0x06 <queue>            Dequeue a <queue>\n"
        "\n"
        "Where\n"
        "  * <queue>   is a queue number. Available options are 1 and 2.\n"
        "  * <element> is a 32bit unsigned integer, which represents an item\n"
        "              in a queue. If a passed integer lies outside of the\n"
        "              unsigned-32-bit range, it will be trimmed.\n"
        "  * <bit>     is a number of bit, starting from 1 and ending at 32.\n"
        "\n"
        "# Merging queues\n"
        "\n"
        "The queues are merged in a chess pattern, like a zipper slider brings "
        "\n"
        "together the two sides. For example, queues of `1, 2, 3` and `4, 5, "
        "6`\nwill be merged into `1, 4, 2, 5, 3, 6`.\n"
        "After the merge the second queue will be emptied, and the first one "
        "will\n"
        "contain the merge result.\n"
        "\n"
        "# Queues\n"
        "\n"
        "The queues have a maximum length of %i and are operated in a %s\n"
        "mode. The queues are loaded into memory from the files `.queue1` and\n"
        "`.queue2` respectively, and are saved to the same files if the\n"
        "program terminates corretly, with a success exit code.\n",
        cmd_name, QUEUE_MAX_LENGTH, cli_queue_mode_string());
}

static void load_queue(struct Queue *queue, const char *file_name) {
    FILE *f = fopen(file_name, "r");
    if (f == NULL) {
        // Can't read the file.
        return;
    }
    unsigned counter = 0;
    for (;;) {
        uint32_t value;
        size_t rc = fread(&value, sizeof(uint32_t), 1, f);
        if (rc != 1) {
            // Read failed, most probably because hit the EOF.
            break;
        }
        queue->array[counter] = value;
        counter += 1;
    }
    queue->size = counter;
    queue->begin = 0;
    fclose(f);
}

void save_queue(const struct Queue *queue, const char *file_name) {
    FILE *f = fopen(file_name, "w");
    if (f == NULL) {
        // Can't read the file.
        return;
    }
    for (unsigned i = 0; i != queue->size; ++i) {
        uint32_t value = queue_get_value(queue, i);
        size_t rc = fwrite(&value, sizeof(uint32_t), 1, f);
        if (rc != 1) {
            // Write failed.
            break;
        }
    }
    fclose(f);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        cli_help(argv[0]);
        return 1;
    }
    long command_id = cli_command_from_arg(argv[1]);
    if (command_id == -1) {
        cli_help(argv[0]);
        return 1;
    }
    struct Queue queues[2];
    load_queue(&queues[0], ".queue1");
    load_queue(&queues[1], ".queue2");
    int rc;
    switch (command_id) {
        case 0x00:
            rc = cli_add_to_queue(argc - 1, argv + 1, queues);
            break;
        case 0x01:
            rc = cli_remove_from_queue(argc - 1, argv + 1, queues);
            break;
        case 0x02:
            rc = cli_print_size_and_contents(argc - 1, argv + 1, queues);
            break;
        case 0x03:
            rc = cli_print_contents(argc - 1, argv + 1, queues);
            break;
        case 0x04:
            rc = cli_merge_queues(queues);
            break;
        case 0x05:
            rc = cli_find_bit(argc - 1, argv + 1, queues);
            break;
        case 0x06:
            rc = cli_dequeue(argc - 1, argv + 1, queues);
            break;
        default:
            fprintf(stderr, "Unknown command '%s'\n", argv[1]);
            cli_help(argv[0]);
            return 1;
    }
    if (rc == -1) {
        return 1;
    }
    save_queue(&queues[0], ".queue1");
    save_queue(&queues[1], ".queue2");
    return 0;
}
