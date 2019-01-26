/**
 * @summary gpu_program.h: 
 */
#ifndef __PIL_GPU_PROGRAM_H__
#define __PIL_GPU_PROGRAM_H__

#pragma once

#ifndef PIL_NO_INCLUDES
#   ifndef __PIL_H__
#       include "pil.h"
#   endif
#endif

struct GPU_PROGRAM_DESC;
struct GPU_PROGRAM_CACHE;
struct GPU_PROGRAM_BYTECODE;
struct GPU_PROGRAM_DEBUGINFO;

typedef enum GPU_PROGRAM_TYPE {
    GPU_PROGRAM_TYPE_UNKNOWN  = 0, 
    GPU_PROGRAM_TYPE_COMPUTE  = 1, 
    GPU_PROGRAM_TYPE_VERTEX   = 2, 
    GPU_PROGRAM_TYPE_FRAGMENT = 3, 
    GPU_PROGRAM_TYPE_GEOMETRY = 4, 
    /* ... */
} GPU_PROGRAM_TYPE;

#ifdef __cplusplus
extern "C" {
#endif

PIL_API(struct GPU_PROGRAM_CACHE*)
GpuProgramCacheCreate
(
    struct GPU_PROGRAM_CACHE_INIT *init
);

PIL_API(void)
GpuProgramCacheDelete
(
    struct GPU_PROGRAM_CACHE *cache
);

PIL_API(uint32_t)
GpuProgramCacheGetProgramCount
(
    struct GPU_PROGRAM_CACHE *cache
);

PIL_API(int)
GpuProgramCacheGetProgram
(
    struct GPU_PROGRAM_BYTECODE *desc, 
    struct GPU_PROGRAM_CACHE   *cache, 
    uint32_t            program_index
);

PIL_API(int)
GpuProgramCachePutProgram
(
    uint32_t       *o_program_index, 
    struct GPU_PROGRAM_DESC   *desc, 
    struct GPU_PROGRAM_CACHE *cache
);

// save/load needs to follow request/complete pattern so it can run async.
// compile needs to follow request/complete pattern so it can run async.
// two ways to put something into the cache - store precompiled bytecode, or runtime compile.
// support a GetCacheInfo like we do for string tables?

#ifdef __cplusplus
}; /* extern "C" */
#endif

#endif /* __PIL_GPU_PROGRAM_H__ */

