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
#   ifndef __PIL_MEMMGR_H__
#       include "memmgr.h"
#   endif
#   ifndef __PIL_TABLE_H__
#       include "table.h"
#   endif
#endif

struct GPU_PROGRAM_TABLE;
struct GPU_PROGRAM_BYTECODE;
struct GPU_PROGRAM_TABLE_INIT;

/* @summary Define constants related to GPU program objects.
 * GPU_PROGRAM_BYTECODE_STREAM_INDEX: The zero-based index of the data stream of the GPU_PROGRAM_BYTECODE.
 * GPU_PROGRAM_TABLE_STREAM_COUNT   : The number of data streams in a GPU_PROGRAM_TABLE.
 * GPU_PROGRAM_COUNT_MAX            : The maximum number of programs that can be loaded into a single GPU_PROGRAM_TABLE.
 */
#ifndef GPU_PROGRAM_CONSTANTS
#   define GPU_PROGRAM_CONSTANTS
#   define GPU_PROGRAM_BYTECODE_STREAM_INDEX                                   0
#   define GPU_PROGRAM_TABLE_STREAM_COUNT                                      1
#   define GPU_PROGRAM_COUNT_MAX                                               8192
#endif

/* @summary Retrieve the number of loaded GPU programs.
 * @param _t A pointer to a GPU_PROGRAM_TABLE instance.
 * @return The number of GPU program bytecode objects loaded into the table.
 */
#ifndef GpuProgramTableCount
#define GpuProgramTableCount(_t)                                               \
    Table_GetCount(&(_t)->TableDesc)
#endif

/* @summary Retrieve the maximum number of loaded GPU programs.
 * @param _t A pointer to a GPU_PROGRAM_TABLE instance.
 * @return The maximum number of loaded GPU programs.
 */
#ifndef GpuProgramTableCapacity
#define GpuProgramTableCapacity(_t)                                            \
    Table_GetCapacity(&(_t)->TableDesc)
#endif

/* @summary Retrieve a pointer to the first element in the active handle stream for the set of GPU program bytecode.
 * @param _t A pointer to a GPU_PROGRAM_TABLE instance.
 * @return A pointer (HANDLE_BITS*) to the first active identifier in the table. If equal to the address returned by GpuProgramTableBytecodeHandleEnd(_t), the table is empty and the returned pointer must not be dereferenced.
 */
#ifndef GpuProgramTableProgramHandleBegin
#define GpuProgramTableProgramHandleBegin(_t)                                  \
    Table_GetHandleBegin(&(_t)->TableDesc)
#endif

/* @summary Retrieve a pointer to one-past the last element in the active handle stream for the set of GPU program bytecode.
 * @param _t A pointer to a GPU_PROGRAM_TABLE instance.
 * @return A pointer (HANDLE_BITS*) to one-past the last active identifier. Do not dereference the returned pointer.
 */
#ifndef GpuProgramTableProgramHandleEnd
#define GpuProgramTableProgramHandleEnd(_t)                                    \
    Table_GetHandleEnd(&(_t)->TableDesc)
#endif

/* @summary Retrieve the _i'th handle of a GPU program.
 * @param _t A pointer to a GPU_PROGRAM_TABLE instance.
 * @param _i The zero-based index of the program handle to retrieve.
 * @return The _i'th program handle.
 */
#ifndef GpuProgramTableHandleAt
#define GpuProgramTableHandleAt(_t, _i)                                        \
    Table_GetHandle(&(_t)->TableDesc, _i)
#endif

/* @summary Retrieve a pointer to the first GPU_PROGRAM_BYTECODE data record.
 * @param _t A pointer to a GPU_PROGRAM_TABLE instance.
 * @return A pointer to the first active GPU_PROGRAM_BYTECODE record. If the returned pointer is the same as GpuProgramTableBytecodeStreamEnd, the table is empty and the returned pointer must not be dereferenced.
 */
#ifndef GpuProgramTableBytecodeStreamBegin
#define GpuProgramTableBytecodeStreamBegin(_t)                                 \
    Table_GetStreamBegin(GPU_PROGRAM_BYTECODE, &(_t)->TableDesc, GPU_PROGRAM_BYTECODE_STREAM_INDEX)
#endif

/* @summary Retrieve a pointer to one-past the last GPU_PROGRAM_BYTECODE data record.
 * @param _t A pointer to a GPU_PROGRAM_TABLE instance.
 * @return A pointer to one-past the last active GPU_PROGRAM_BYTECODE data record. Do not dereference the returned pointer.
 */
#ifndef GpuProgramTableBytecodeStreamEnd
#define GpuProgramTableBytecodeStreamEnd(_t)                                   \
    Table_GetStreamEnd  (GPU_PROGRAM_BYTECODE, &(_t)->TableDesc, GPU_PROGRAM_BYTECODE_STREAM_INDEX)
#endif

/* @summary Retrieve a pointer to the _i'th GPU_PROGRAM_BYTECODE record.
 * @param _t A pointer to a GPU_PROGRAM_TABLE instance.
 * @param _i The zero-based index of the item to retrieve.
 * @return A pointer to the _i'th data record.
 */
#ifndef GpuProgramTableBytecodeStreamAt
#define GpuProgramTableBytecodeStreamAt(_t, _i)                                \
    Table_GetStreamElement(GPU_PROGRAM_BYTECODE, &(_t)->TableDesc, GPU_PROGRAM_BYTECODE_STREAM_INDEX, _i)
#endif

/* @summary Define the data describing compiled GPU program bytecode.
 * GPU program bytecode is converted into an architecture-specific representation by the GPU driver.
 */
typedef struct GPU_PROGRAM_BYTECODE {
    ADDRESS_OR_OFFSET               ByteCode;                                  /* Pointer to the buffer containing the compiled bytecode, or the offset of the compiled bytecode within the buffer. */
    uint64_t                        SizeBytes;                                 /* The number of bytes in the ByteCode buffer that are valid. */
    uint32_t                        ProgramType;                               /* A value of the GPU_PROGRAM_TYPE enumeration indicating the type of GPU program. */
    uint32_t                        ProgramFlags;                              /* One or more bitwise ORd values of the GPU_PROGRAM_FLAGS enumeration specifying usage flags for the GPU program. */
} GPU_PROGRAM_BYTECODE;

/* @summary Define the data required to create a table of GPU programs.
 */
typedef struct GPU_PROGRAM_TABLE_INIT {
    uint64_t                        CommitBytes;                               /* The number of bytes of GPU program bytecode storage that should initially be committed. */
    uint64_t                        CapacityBytes;                             /* The maximum number of bytes of GPU program bytecode that can be loaded into the table. */
    uint32_t                        ProgramCapacity;                           /* The maximum number of GPU program objects that can be stored in the table. */
} GPU_PROGRAM_TABLE_INIT;

/* @summary Define the data associated with a set of GPU program objects.
 * At a minimum, the table stores the bytecode for each GPU program.
 */
typedef struct GPU_PROGRAM_TABLE {
    MEMORY_ARENA                    Allocator;                                 /* The arena allocator used to sub-allocate bytecode blobs. */
    TABLE_DESC                      TableDesc;                                 /* The table descriptor, used for calling the data table functions. */
    TABLE_INDEX                     TableIndex;                                /* The table index used to map GPU_PROGRAM_HANDLE to a dense array index. */
    TABLE_DATA                      BytecodeData;                              /* The table data storing the stream of GPU_PROGRAM_BYTECODE instances. */
    TABLE_DATA                     *TableStreams[1];                           /* An array of pointers to the table data streams. */
    MEMORY_BLOCK                    MemoryBlock;                               /* The memory block used to store GPU program bytecode. */
} GPU_PROGRAM_TABLE;

/* @summary Define the supported types of GPU programs.
 */
typedef enum GPU_PROGRAM_TYPE {
    GPU_PROGRAM_TYPE_UNKNOWN                  =  0,                            /* The program type is not known, and the program is not valid. */
    GPU_PROGRAM_TYPE_COMPUTE                  =  1,                            /* The program cache contains general-purpose compute programs. */
    GPU_PROGRAM_TYPE_VERTEX                   =  2,                            /* The program cache contains programs used for processing vertex data. */
    GPU_PROGRAM_TYPE_FRAGMENT                 =  3,                            /* The program cache contains programs used for performing per-fragment computation. */
    GPU_PROGRAM_TYPE_GEOMETRY                 =  4,                            /* The program cache contains programs for emitting or processing geometric primitives. */
} GPU_PROGRAM_TYPE;

/* @summary Define flags tht can be bitwise ORd to describe GPU program usage.
 */
typedef enum GPU_PROGRAM_FLAGS {
    GPU_PROGRAM_FLAGS_NONE                    = (0UL <<  0),                   /* No program usage flags are specified. */
} GPU_PROGRAM_FLAGS;

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}; /* extern "C" */
#endif

#endif /* __PIL_GPU_PROGRAM_H__ */

