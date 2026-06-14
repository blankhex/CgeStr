#ifndef VALUELIST_H
#define VALUELIST_H

#include <stddef.h>
#include <string.h>
#include <stdlib.h>

struct ValueList {
    long* data;
    size_t size;
    struct ValueList* next;
};

static long valueListFind(struct ValueList* list, long* data, size_t size) {
    long index = 0;

    while (list) {
        if (list->size == size && !memcmp(list->data, data, size * sizeof(long)))
            return index;
        index++, list = list->next;
    }
    return -1;
}

static long valueListIntern(struct ValueList** list, long* data, size_t size) {
    long index = 0;

    while (*list) ++index, list = &(*list)->next;
    if (!(*list = malloc(sizeof(struct ValueList))))
        abort();
    if (!((*list)->data = malloc(size * sizeof(long))))
        abort();

    memcpy((*list)->data, data, size * sizeof(long));
    (*list)->size = size;
    (*list)->next = NULL;

    return index;
}

#endif /* VALUELIST_H */