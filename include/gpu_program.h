/**
 * @summary gpu_program.h: Define the types and functions used for working with 
 * compiled GPU programs. Compiled GPU programs are stored in caches of program
 * bytecode, which can then be used to create runtime programs for executing 
 * work on the GPU. GPU programs can either be compiled at runtime, or the 
 * pre-compiled bytecode can be loaded from disk or memory into a program cache.
 */
#ifndef __PIL_GPU_PROGRAM_H__
#define __PIL_GPU_PROGRAM_H__

#pragma once

#ifndef PIL_NO_INCLUDES
#   ifndef __PIL_H__
#       include "pil.h"
#   endif
#endif

/* You don't need a device to compile programs or create a program cache */

struct GPU_PROGRAM_DESC;
struct GPU_PROGRAM_CACHE;
struct GPU_PROGRAM_BYTECODE;
struct GPU_PROGRAM_CACHE_INIT;

/* @summary Define the data describing compiled GPU program bytecode.
 * GPU program bytecode is converted into an architecture-specific representation by the GPU driver.
 */
typedef struct GPU_PROGRAM_BYTECODE {
    uint8_t                        *ByteCode;                                  /* Pointer to the buffer containing the compiled bytecode. */
    uint64_t                        SizeBytes;                                 /* The number of bytes in the ByteCode buffer that are valid. */
} GPU_PROGRAM_BYTECODE;

/* @summary Define additional data optionally supplied with GPU_PROGRAM_BYTECODE for debugging purposes.
 */
typedef struct GPU_PROGRAM_DEBUG_INFO {
    char_native_t                  *SourcePath;                                /* Pointer to a nul-terminated string specifying the path of the main HLSL source file. This may point to an empty string. */
    char                           *EntryPoint;                                /* Pointer to a nul-terminated string specifying the the program entry point. This may point to an empty string. */
    int64_t                         LastWriteTime;                             /* The last write time of the most recently modified HLSL source file(s), specified in UNIX timestamp format. */
    int64_t                         LastBuildTime;                             /* The last write time of the bytecode file or of the last compilation, specified in UNIX timestamp format. */
} GPU_PROGRAM_DEBUG_INFO;

/* @summary Define the data required to create a GPU program cache.
 * All GPU programs in a program cache are of the same type (for example, all fragment programs).
 * Several fields are required only if programs need to be compiled at runtime. 
 * If runtime compilation is not needed, the DefineSymbols, DefineValues, and IncludePaths fields can all be NULL, and DefineCount and IncludeCount should be set to 0.
 * All programs in the cache share the same compilation attributes.
 * Programs can be added to the cache one-by-one, or in batches, but the entire cache must be dropped at once.
 */
typedef struct GPU_PROGRAM_CACHE_INIT {
    char const                     *TargetModel;                               /* A nul-terminated string specifying the shader model target. This may */
    char const                    **DefineSymbols;                             /* An array of nul-terminated strings specifying the preprocessor symbols to define when compiling GPU program source code. */
    char const                    **DefineValues;                              /* An array of nul-terminated strings specifying the values of the preprocessor symbols when compiling GPU program source code. */
    char_native_t const           **IncludePaths;                              /* An array of nul-terminated strings specifying the absolute paths of the directories to search for includes. */
    uint32_t                        IncludePathCount;                          /* The number of nul-terminated path strings in the IncludePaths array. */
    uint32_t                        DefineCount;                               /* The number of nul-terminated symbols and values in the DefineSymbols and DefineValues arrays. */
    uint32_t                        ProgramCapacity;                           /* The maximum number of GPU programs that can be stored in the cache. */
    uint32_t                        ProgramType;                               /* One of the values of the GPU_PROGRAM_TYPE enumeration specifying the type of GPU programs in the cache. All programs in the cache must be of the same type. */
    uint32_t                        ProgramCacheFlags;                         /* One or more bitwise-OR'd values of the GPU_PROGRAM_CACHE_FLAGS enumeration. */
} GPU_PROGRAM_CACHE_INIT;

/* @summary Define the supported types of GPU programs.
 */
typedef enum GPU_PROGRAM_TYPE {
    GPU_PROGRAM_TYPE_UNKNOWN                  =  0,                            /* The program type is not known, and the program is not valid. */
    GPU_PROGRAM_TYPE_COMPUTE                  =  1,                            /* The program cache contains general-purpose compute programs. */
    GPU_PROGRAM_TYPE_VERTEX                   =  2,                            /* The program cache contains programs used for processing vertex data. */
    GPU_PROGRAM_TYPE_FRAGMENT                 =  3,                            /* The program cache contains programs used for performing per-fragment computation. */
    GPU_PROGRAM_TYPE_GEOMETRY                 =  4,                            /* The program cache contains programs for emitting or processing geometric primitives. */
} GPU_PROGRAM_TYPE;

/* @summary Define a series of flags that can be bitwise-OR'd to specify special program cache attributes.
 */
typedef enum GPU_PROGRAM_CACHE_FLAGS {
    GPU_PROGRAM_CACHE_FLAGS_NONE              = (0UL << 0),                    /* No special flags are set. */
    GPU_PROGRAM_CACHE_BUILD_ONLY              = (1UL << 0),                    /* The program cache is being used to store compiled program code as part of a build process. */
    GPU_PROGRAM_CACHE_FLAGS_COMPILER          = (1UL << 1),                    /* Require runtime compilation support. */
    GPU_PROGRAM_CACHE_FLAGS_DEBUG             = (1UL << 2),                    /* Enable debugging features. */
    GPU_PROGRAM_CACHE_FLAGS_SKIP_OPTIMIZATION = (1UL << 3),                    /* Do not optimize compiled programs. */
} GPU_PROGRAM_CACHE_FLAGS;

#ifdef __cplusplus
extern "C" {
#endif

/* @summary Create a new program cache into which bytecode will be loaded.
 * The program cache supports explicit storing of externally-compiled bytecode as well as runtime compilation.
 * @param init Data used to configure creation of the cache.
 * @return A pointer to the empty program cache object, or NULL.
 */
PIL_API(struct GPU_PROGRAM_CACHE*)
GpuProgramCacheCreate
(
    struct GPU_PROGRAM_CACHE_INIT *init
);

/* @summary Delete a program cache and unload all of its stored bytecode.
 * @param cache The program cache to delete.
 */
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
GpuProgramCacheStoreProgram
(
    uint32_t             *o_program_index, 
    struct GPU_PROGRAM_BYTECODE *bytecode, 
    struct GPU_PROGRAM_DEBUG_INFO  *debug, 
    struct GPU_PROGRAM_CACHE       *cache
);

// save/load needs to follow request/complete pattern so it can run async.
// compile needs to follow request/complete pattern so it can run async.
// two ways to put something into the cache - store precompiled bytecode, or runtime compile.
// support a GetCacheInfo like we do for string tables?

#ifdef __cplusplus
}; /* extern "C" */
#endif

#endif /* __PIL_GPU_PROGRAM_H__ */

