/**
 * @summary gpu_program_d3d12.h: Define the data structures and functions 
 * relating to management of compiled GPU program bytecode on Direct3D 12
 * devices.
 */
#ifndef __PIL_GPU_PROGRAM_D3D12_H__
#define __PIL_GPU_PROGRAM_D3D12_H__

#pragma once

struct GPU_PROGRAM_BYTECODE_D3D12;
struct GPU_PROGRAM_TABLE_D3D12;

/* @summary Define constants related to GPU program objects.
 * D3D12_PROGRAM_BYTECODE_STREAM_INDEX: The zero-based index of the data stream of the GPU_PROGRAM_BYTECODE_D3D12.
 * D3D12_PROGRAM_TABLE_STREAM_COUNT   : The number of data streams in a GPU_PROGRAM_TABLE_D3D12.
 * D3D12_GPU_PROGRAM_COUNT_MAX        : The maximum number of programs that can be loaded into a single GPU_PROGRAM_TABLE_D3D12.
 */
#ifndef GPU_PROGRAM_D3D12_CONSTANTS
#   define GPU_PROGRAM_D3D12_CONSTANTS
#   define D3D12_PROGRAM_BYTECODE_STREAM_INDEX                                 0
#   define D3D12_PROGRAM_TABLE_STREAM_COUNT                                    1
#   define D3D12_PROGRAM_COUNT_MAX                                             8192
#endif

/* @summary Retrieve the number of loaded GPU programs.
 * @param _t A pointer to a GPU_PROGRAM_TABLE_D3D12 instance.
 * @return The number of GPU program bytecode objects loaded into the table.
 */
#ifndef D3D12_ProgramTableCount
#define D3D12_ProgramTableCount(_t)                                            \
    Table_GetCount(&(_t)->TableDesc)
#endif

/* @summary Retrieve the maximum number of loaded GPU programs.
 * @param _t A pointer to a GPU_PROGRAM_TABLE_D3D12 instance.
 * @return The maximum number of loaded GPU programs.
 */
#ifndef D3D12_ProgramTableCapacity
#define D3D12_ProgramTableCapacity(_t)                                         \
    Table_GetCapacity(&(_t)->TableDesc)
#endif

/* @summary Retrieve a pointer to the first element in the active handle stream for the set of GPU program bytecode.
 * @param _t A pointer to a GPU_PROGRAM_TABLE_D3D12 instance.
 * @return A pointer (HANDLE_BITS*) to the first active identifier in the table. If equal to the address returned by D3D12_ProgramTableBytecodeHandleEnd(_t), the table is empty and the returned pointer must not be dereferenced.
 */
#ifndef D3D12_ProgramTableProgramHandleBegin
#define D3D12_ProgramTableProgramHandleBegin(_t)                               \
    Table_GetHandleBegin(&(_t)->TableDesc)
#endif

/* @summary Retrieve a pointer to one-past the last element in the active handle stream for the set of GPU program bytecode.
 * @param _t A pointer to a GPU_PROGRAM_TABLE_D3D12 instance.
 * @return A pointer (HANDLE_BITS*) to one-past the last active identifier. Do not dereference the returned pointer.
 */
#ifndef D3D12_ProgramTableProgramHandleEnd
#define D3D12_ProgramTableProgramHandleEnd(_t)                                 \
    Table_GetHandleEnd(&(_t)->TableDesc)
#endif

/* @summary Retrieve the _i'th handle of a GPU program.
 * @param _t A pointer to a GPU_PROGRAM_TABLE_D3D12 instance.
 * @param _i The zero-based index of the program handle to retrieve.
 * @return The _i'th program handle.
 */
#ifndef D3D12_ProgramTableHandleAt
#define D3D12_ProgramTableHandleAt(_t, _i)                                     \
    Table_GetHandle(&(_t)->TableDesc, _i)
#endif

/* @summary Retrieve a pointer to the first GPU_PROGRAM_BYTECODE_D3D12 data record.
 * @param _t A pointer to a GPU_PROGRAM_TABLE_D3D12 instance.
 * @return A pointer to the first active GPU_PROGRAM_BYTECODE_D3D12 record. If the returned pointer is the same as D3D12_ProgramTableBytecodeStreamEnd, the table is empty and the returned pointer must not be dereferenced.
 */
#ifndef D3D12_ProgramTableBytecodeStreamBegin
#define D3D12_ProgramTableBytecodeStreamBegin(_t)                              \
    Table_GetStreamBegin(GPU_PROGRAM_BYTECODE_D3D12, &(_t)->TableDesc, D3D12_PROGRAM_BYTECODE_STREAM_INDEX)
#endif

/* @summary Retrieve a pointer to one-past the last GPU_PROGRAM_BYTECODE_D3D12 data record.
 * @param _t A pointer to a GPU_PROGRAM_TABLE_D3D12 instance.
 * @return A pointer to one-past the last active GPU_PROGRAM_BYTECODE_D3D12 data record. Do not dereference the returned pointer.
 */
#ifndef D3D12_ProgramTableBytecodeStreamEnd
#define D3D12_ProgramTableBytecodeStreamEnd(_t)                                \
    Table_GetStreamEnd  (GPU_PROGRAM_BYTECODE_D3D12, &(_t)->TableDesc, D3D12_PROGRAM_BYTECODE_STREAM_INDEX)
#endif

/* @summary Retrieve a pointer to the _i'th GPU_PROGRAM_BYTECODE_D3D12 record.
 * @param _t A pointer to a GPU_PROGRAM_TABLE_D3D12 instance.
 * @param _i The zero-based index of the item to retrieve.
 * @return A pointer to the _i'th data record.
 */
#ifndef D3D12_ProgramTableBytecodeStreamAt
#define D3D12_ProgramTableBytecodeStreamAt(_t, _i)                             \
    Table_GetStreamElement(GPU_PROGRAM_BYTECODE_D3D12, &(_t)->TableDesc, D3D12_PROGRAM_BYTECODE_STREAM_INDEX, _i)
#endif

/* @summary Define the data used to store the compiled GPU program bytecode.
 * GPU program bytecode is converted into an architecture-specific representation by the GPU driver.
 * Memory for the bytecode is allocated from an arena maintained by the table.
 */
typedef struct GPU_PROGRAM_BYTECODE_D3D12 {
    uint8_t                        *ByteCode;                                  /* The blob containing the compiled program bytecode. */
    size_t                          CodeSize;                                  /* The size of the bytecode data buffer, in bytes. */
    uint32_t                        ProgramType;                               /* One of the values of the GPU_PROGRAM_TYPE enumeration. */
} GPU_PROGRAM_BYTECODE_D3D12;

/* @summary Define the data associated with a table of Direct3D 12 GPU program bytecode.
 */
typedef struct GPU_PROGRAM_TABLE_D3D12 {
    MEMORY_ARENA                    Allocator;                                 /* */
    TABLE_DESC                      TableDesc;                                 /* The table descriptor, used for calling the data table functions. */
    TABLE_INDEX                     TableIndex;                                /* The table index used to map GPU_PROGRAM_HANDLE to a dense array index. */
    TABLE_DATA                      BytecodeData;                              /* The table data storing the stream of GPU_PROGRAM_BYTECODE_D3D12 instances. */
    TABLE_DATA                     *TableStreams[1];                           /* An array of pointers to the table data streams. */
    MEMORY_BLOCK                    MemoryBlock;                               /* */
} GPU_PROGRAM_TABLE_D3D12;

PIL_API(int)
D3D12_CreateGpuProgramTable
(
    //
)

#endif /* __PIL_GPU_PROGRAM_D3D12_H__ */

