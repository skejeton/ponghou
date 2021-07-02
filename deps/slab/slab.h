/**
 * Homogenous data container
 * ishidex2 2021, MIT license
 */

#pragma once

#include <stdbool.h>
#include <memory.h>
#include <unistd.h>
#include <inttypes.h>
#include <stdlib.h>

#define OFTYPE(t) sizeof(t)

struct Slab {
    size_t el_size;
    size_t capacity;
    uint8_t *elements;
};

struct Slab slab_new(size_t etype);

bool slab_write(
    struct Slab *cont,
    size_t at, 
    void *val
);

void* slab_read(
    struct Slab *cont,
    size_t at
);

bool slab_resize(
    struct Slab *cont,
    size_t capacity
);

void slab_drop(struct Slab *cont);
