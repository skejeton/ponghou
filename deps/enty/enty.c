#include "enty.h"
#include <math.h>
#include <stdio.h>

struct Enty enty_new(usize type_size) {
    return (struct Enty) {
        .slab = slab_new(type_size),
        .last_index = -1,
        .free_capacity = 1,
        .frees = calloc(1, sizeof(u32))
    };
}

// Returns rightmost set bit
u32 rmsb(u32 num) {
    return (num << 16 ? (num << 24 ? (num << 28 ? (num << 30 ? (num << 31 ? 0 : 1) : (num << 29 ? 2 : 3)) : (num << 26 ? (num << 27 ? 4 : 5) : (num << 25 ? 6 : 7))) : (num << 20 ? (num << 22 ? (num << 23 ? 8 : 9) : (num << 21 ? 10 : 11)) : (num << 18 ? (num << 19 ? 12 : 13) : (num << 17 ? 14 : 15)))) : (num << 8 ? (num << 12 ? (num << 14 ? (num << 15 ? 16 : 17) : (num << 13 ? 18 : 19)) : (num << 10 ? (num << 11 ? 20 : 21) : (num << 9 ? 22 : 23))) : (num << 4 ? (num << 6 ? (num << 7 ? 24 : 25) : (num << 5 ? 26 : 27)) : (num << 2 ? (num << 3 ? 28 : 29) : (num << 1 ? 30 : (num << 0 ? 31 : 32))))));
}

usize enty_first_index(struct Enty *self) {
    usize index = 0;
    while (enty_is_free(self, index) && (isize)index < self->last_index) index += 1;
    return index;
}

usize enty_advance_index(struct Enty *self, usize index) {
    index += 1;
    while (enty_is_free(self, index) && (isize)index < self->last_index) index += 1;
    return index;
}

usize enty_retreat_index(struct Enty *self, usize index) {
    if (index == 0) return 0;
    index -= 1;
    while (enty_is_free(self, index) && index > 0) index -= 1;
    return index;
}

usize enty_insert(struct Enty *self, void *value) {
    for (usize i = 0; i < self->free_capacity; i += 1) {
        u32 bitmap = self->frees[i];
        if (bitmap != 0xFFFFFFFF) {
            usize unset_pos = (usize) rmsb(~bitmap);
            usize offset = (i * FREED_BITS) + unset_pos;
            if (self->slab.capacity <= offset) {
                if (!slab_resize(&self->slab, self->slab.capacity * 2)) {
                    fprintf(stderr, "sus\n");
                    exit(-1);
                }
            }
            if ((isize)offset > self->last_index) {
                self->last_index = (isize) offset;
            }
            slab_write(&self->slab, offset, value);
            self->frees[i] |= (1 << unset_pos);
            return offset;
        }
    }

    // Resize and repeat
    void *old_frees = self->frees;
    void *new_frees = calloc(self->free_capacity*2, sizeof(u32));
    memcpy(new_frees, self->frees, self->free_capacity*sizeof(u32));
    free(old_frees);
    self->frees = new_frees;
    self->free_capacity *= 2;


    return enty_insert(self, value);
}

void* enty_get(struct Enty *self, usize index) {
    usize bitmap_offset = index / FREED_BITS;
    u32 flag = 1 << (index-bitmap_offset*FREED_BITS);

    if (bitmap_offset > self->free_capacity) {
        fprintf(stderr, "Enty container invalid bitmap offset, maximum %d got %d, requested via index %d\n", self->free_capacity, bitmap_offset, index);
        exit(-1);
    };
    if ((self->frees[bitmap_offset] & flag) == 0) {
        fprintf(stderr, "Enty container, attempt to access freed area\n");
        exit(-1);       
    }

    return slab_read(&self->slab, index);
}

bool enty_is_free(struct Enty *self, usize index) {
    usize bitmap_offset = index / FREED_BITS;
    u32 flag = 1 << (index-bitmap_offset*FREED_BITS);
    if (bitmap_offset >= self->free_capacity) return true;
    if ((self->frees[bitmap_offset] & flag) == 0) {
        return true;
    }

    return false;
}

void enty_free_index(struct Enty *self, usize index) {
    usize bitmap_offset = index / FREED_BITS;
    u32 flag = 1 << (index-bitmap_offset*FREED_BITS);

    if (bitmap_offset > self->free_capacity) {
        fprintf(stderr, "Enty container invalid bitmap offset, maximum %d got %d, requested via index %d", self->free_capacity, bitmap_offset, index);
        exit(-1);
    };
    if ((self->frees[bitmap_offset] & flag) == 0) {
        fprintf(stderr, "Enty container, attempt to free already freed area");
        exit(-1);       
    }

    // Mark free
    self->frees[bitmap_offset] &= ~flag;
}

void enty_drop(struct Enty *self) {
    slab_drop(&self->slab);
    free(self->frees);
}
