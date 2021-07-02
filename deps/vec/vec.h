/**
 * Size annotated vector type
 * ishidex2 2021, MIT license
 */

#pragma once

#include <stdbool.h>
#include <memory.h>
#include <unistd.h>
#include <inttypes.h>
#include <stdlib.h>
#include "../slab/slab.h"

struct Vec {
    size_t size;
    struct Slab cont;
};

struct Vec vec_new(size_t etype);
void vec_push(struct Vec *vec, void *val);
void* vec_get(struct Vec *vec, size_t at);
void* vec_pop(struct Vec *vec);
void vec_drop(struct Vec *vec);
