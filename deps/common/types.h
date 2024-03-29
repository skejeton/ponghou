/**
 * Type aliases
 * ishidex2 2021, MIT license
 */

#pragma once

#include <inttypes.h>
#include <unistd.h>

typedef uint64_t u64;
typedef int64_t i64;

typedef uint32_t u32;
typedef int32_t i32;

typedef uint16_t u16;
typedef int16_t i16;

typedef uint8_t u8;
typedef int8_t i8;

typedef size_t usize;
typedef ssize_t isize;

typedef float f32;
typedef double f64;

typedef float f32;
typedef double f64;

#define nullable /* @nullable */
#define pub /* @public */
#define FORRANGE(T, handle, from, to) for (T handle = from; handle < to; handle += 1)
