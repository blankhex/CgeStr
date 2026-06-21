#ifndef BLOCKS_H
#define BLOCKS_H

#include <stddef.h>
#include <stdlib.h>
#include "ValueList.h"

struct BlockLevel {
    long *data;
    size_t size;
    size_t capacity;
    struct ValueList *list;
};

struct Blocks {
    struct BlockLevel *levels;
    size_t depth;
};

static int ilog2(unsigned long value) {
    int result = -1;
    while (value) value >>= 1, result++;
    return result;
}

static long blockInsert(struct Blocks *blocks, long value, size_t depth) {
    struct BlockLevel *level = blocks->levels + depth;

    if (blocks->depth - 1 != depth) {
        if ((value = blockInsert(blocks, value, depth + 1)) == -1)
            return -1;
    }

    level->data[level->size++] = value;
    if (level->size >= level->capacity) {
        level->size = 0;

        if (depth == 0 || (value = valueListFind(level->list, level->data, level->capacity)) == -1) {
            value = valueListIntern(&level->list, level->data, level->capacity);
        }
        return value;
    }

    return -1;
}

static long blockFindR(struct Blocks *blocks, long value, long offset, size_t depth) {
    struct BlockLevel *level = &blocks->levels[depth];
    struct ValueList *current;
    size_t i, bits = 0;

    for (i = depth + 1; i < blocks->depth; i++) {
        bits += ilog2(blocks->levels[i].capacity);
    }

    current = level->list;
    while (offset--) current = current->next;

    offset = (value >> bits) & ((unsigned long)level->capacity - 1);
    offset = current->data[offset];

    if (depth != blocks->depth - 1)
        return blockFindR(blocks, value, offset, depth + 1);

    return offset;
}

static long blockFind(struct Blocks *blocks, long value) {
    size_t i, bits = 0;
    long offset;

    for (i = 0; i < blocks->depth; i++) {
        bits += ilog2(blocks->levels[i].capacity);
    }

    offset = value >> bits;
    return blockFindR(blocks, value, offset, 0);
}

static void blockInit(struct Blocks *blocks, size_t depth, ...) {
    va_list args;

    blocks->depth = 0;
    if (!(blocks->levels = malloc(sizeof(struct BlockLevel) * depth)))
        abort();

    va_start(args, depth);

    while (blocks->depth < depth) {
        struct BlockLevel *level = &blocks->levels[blocks->depth];

        level->list = NULL;
        level->size = 0;
        level->capacity = va_arg(args, int);
        if (!(level->data = malloc(level->capacity * sizeof(long))))
            abort();
        blocks->depth++;
    }

    va_end(args);
}

static void blockDump(struct Blocks *blocks, size_t depth, FILE *out,
                      const char *name, const char *type) {
    struct BlockLevel *level = &blocks->levels[depth];
    struct ValueList *current;
    size_t i, j, printed = 0;

    fprintf(out, "static const %s %s[] = {\n    ", type, name);
    current = level->list;
    for (current = level->list; current; current = current->next) {
        for (i = 0; i < level->capacity; i++) {
            if (printed++ >= 15) {
                fprintf(out, "\n    ");
                printed = 1;
            }
            fprintf(out, "%ld, ", current->data[i]);
        }
    }
    fprintf(out, "\n};\n\n");
}

static void blockAccess(struct Blocks *blocks, size_t depth, FILE *out,
                        const char *var, const char *arg, const char *name) {
    struct BlockLevel *level = &blocks->levels[depth];
    long i, bits = 0, offset, mask;

    for (i = depth + 1; i < blocks->depth; i++) {
        bits += ilog2(blocks->levels[i].capacity);
    }

    offset = ilog2(blocks->levels[depth].capacity);
    mask = level->capacity - 1;

    fprintf(out, "    %s = (long)%s", var, name);
    if (depth) {
        if (offset) {
            fprintf(out, "[(%s<<%ld)", var, offset);
        } else {
            fprintf(out, "[%s", var);
        }
    }
    else
        fprintf(out, "[");

    if (mask || !depth) {
        if (depth)
            fprintf(out, "+");

        if (mask)
            fprintf(out, "(");

        if (bits)
            fprintf(out, "(%s>>%ld)", arg, (long)bits);
        else
            fprintf(out, "%s", arg);

        if (depth)
            fprintf(out, "&%ld", mask);

        if (mask)
            fprintf(out, ")");
    }

    fprintf(out, "];\n");
}

#endif /* BLOCKS_H */
