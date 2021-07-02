#include "slab.h"
#include <stdbool.h>
#include <memory.h>
#include <stdio.h>
#include <unistd.h>
#include <inttypes.h>
#include <stdlib.h>

#define OFTYPE(t) sizeof(t)

struct Slab slab_new(size_t etype) {
    return (struct Slab) {
        .el_size = etype,
        .capacity = 32,
        .elements = malloc(etype*32)
    };
}

bool slab_write(
    struct Slab *cont,
    size_t at, 
    void *val
) {
    if (at > cont->capacity) return false;
    memcpy(cont->elements+(cont->el_size*at), val, cont->el_size);
    return true;
}

void* slab_read(
    struct Slab *cont,
    size_t at
) {
    if (at >= cont->capacity) return NULL;
    return cont->elements+(cont->el_size*at);
}



bool slab_resize(
    struct Slab *cont,
    size_t capacity)
{
    if (capacity < cont->capacity)
        return false;
    printf("Resizing for size %d, previously %d\n", capacity*cont->el_size, cont->capacity*cont->el_size);
    cont->elements = realloc(cont->elements, capacity*cont->el_size);
    cont->capacity = capacity;
    return true;
}

void slab_drop(struct Slab *cont) {
    free(cont->elements);
    cont->elements = NULL;
}
