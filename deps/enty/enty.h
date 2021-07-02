/**
 * Homogenous data allocator
 * ishidex2 2021, MIT license
 */

#pragma once
#include "../slab/slab.h"
#include "../common/types.h"

#define FREED_BITS 32L

struct Enty {
    struct Slab slab;
    // Each element in the list will represent if some element is free in the slab
    // We'll hide the implementation detail that the size is limited,
    // It will **panic** when the slab size exceeds 32 bits * 32 cells,
    // However, that will change as the demands will be growing
    // 1 = occupied
    // 0 = freed
    u32 *frees;
    usize free_capacity;
    isize last_index;
};

struct Enty enty_new(usize type_size);
usize enty_first_index(struct Enty *self);
usize enty_advance_index(struct Enty *self, usize index);
usize enty_retreat_index(struct Enty *self, usize index);
void enty_free_index(struct Enty *self, usize index);
usize enty_insert(struct Enty *self, void *value);
bool enty_is_free(struct Enty *self, usize index);
void* enty_get(struct Enty *self, usize index);
void enty_drop(struct Enty *self);
