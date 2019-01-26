/**
 * @summary gpu_program_d3d12.h: Define the data structures and functions 
 * relating to compiling GPU programs at runtime and managing cached GPU 
 * program bytecode.
 */
#ifndef __PIL_GPU_PROGRAM_D3D12_H__
#define __PIL_GPU_PROGRAM_D3D12_H__

#pragma once

#ifndef PIL_NO_INCLUDES
#   ifndef __PIL_H__
#       include "pil.h"
#   endif
#   ifndef __PIL_STRLIB_H__
#       include "strlib.h"
#   endif
#   ifndef __PIL_D3DCOMPILERAPI_WIN32_H__
#       include "win32/d3dcompilerapi_win32.h"
#   endif
#endif

struct D3DCOMPILERAPI_DISPATCH;
struct GPU_PROGRAM_BYTECODE_D3D12;
struct GPU_PROGRAM_DEBUGINFO_D3D12;
struct GPU_PROGRAM_TABLE_D3D12;
struct GPU_PROGRAM_CACHE_D3D12;
struct GPU_PROGRAM_CACHE_INIT_D3D12;

/* @summary Define constants related to table access and the maximum number of program objects.
 * D3D12_GPU_PROGRAM_BYTECODE_STREAM_INDEX: The zero-based index of the data stream of GPU_PROGRAM_BYTECODE_D3D12 records.
 * D3D12_GPU_PROGRAM_COUNT_MAX            : The maximum number of programs that can be loaded into a single GPU_PROGRAM_CACHE_D3D12.
 */
#ifndef GPU_PROGRAM_D3D12_CONSTANTS
#   define GPU_PROGRAM_D3D12_CONSTANTS
#   define D3D12_GPU_PROGRAM_BYTECODE_STREAM_INDEX                             0
#   define D3D12_GPU_PROGRAM_DEBUGINFO_STREAM_INDEX                            2
#   define D3D12_GPU_PROGRAM_COUNT_MAX                                         1024
#endif

/* @summary Retrieve the number of active GPU programs in a program cache.
 * @param _t A pointer to a GPU_PROGRAM_CACHE_D3D12 instance.
 * @return The number of active GPU programs in the cache.
 */
#ifndef D3D12_ProgramCacheCount
#define D3D12_ProgramCacheCount(_t)                                            \
    Table_GetCount(&(_t)->TableDesc)
#endif

/* @summary Retrieve the maximum number of active GPU programs in a program cache.
 * @param _t A pointer to a GPU_PROGRAM_CACHE_D3D12 instance.
 * @return The maximum number of active GPU programs in the cache.
 */
#ifndef D3D12_ProgramCacheCapacity
#define D3D12_ProgramCacheCapacity(_t)                                         \
    Table_GetCapacity(&(_t)->TableDesc)
#endif

/* @summary Retrieve a pointer to the first element in the active handle stream for the set of GPU programs in a program cache.
 * @param _t A pointer to a GPU_PROGRAM_CACHE_D3D12 instance.
 * @return A pointer (HANDLE_BITS*) to the first active identifier in the table. If equal to the address returned by D3D12_ProgramCacheHandleEnd(_t), the table is empty and the returned pointer must not be dereferenced.
 */
#ifndef D3D12_ProgramCacheHandleBegin
#define D3D12_ProgramCacheHandleBegin(_t)                                      \
    Table_GetHandleBegin(&(_t)->TableDesc)
#endif

/* @summary Retrieve a pointer to one-past the last element in the active handle stream for the set of GPU programs in a program cache.
 * @param _t A pointer to a GPU_PROGRAM_CACHE_D3D12 instance.
 * @return A pointer (HANDLE_BITS*) to one-past the last active identifier. Do not dereference the returned pointer.
 */
#ifndef D3D12_ProgramCacheHandleEnd
#define D3D12_ProgramCacheHandleEnd(_t)                                        \
    Table_GetHandleEnd(&(_t)->TableDesc)
#endif

/* @summary Retrieve the _i'th handle of a GPU program in a program cache.
 * @param _t A pointer to a GPU_PROGRAM_CACHE_D3D12 instance.
 * @param _i The zero-based index of the program to retrieve.
 * @return The _i'th program handle.
 */
#ifndef D3D12_ProgramCacheHandleAt
#define D3D12_ProgramCacheHandleAt(_t, _i)                                     \
    Table_GetHandle(&(_t)->TableDesc, _i)
#endif

/* @summary Define the data used to store the compiled GPU program bytecode.
 * GPU program bytecode is converted into an architecture-specific representation by the GPU driver.
 */
typedef struct GPU_PROGRAM_BYTECODE_D3D12 {
    ID3DBlob                       *ByteCode;                                  /* The blob containing the compiled program bytecode. */
} GPU_PROGRAM_BYTECODE_D3D12;

/* @summary Define the data used for debugging purposes and to support runtime change detection and re-compilation of GPU programs.
 */
typedef struct GPU_PROGRAM_DEBUGINFO_D3D12 {
    char_native_t                  *SourcePath;                                /* Pointer to an interned, nul-terminated string specifying the path of the main HLSL source file. This may point to an empty string. */
    char_native_t                  *EntryPoint;                                /* Pointer to an interned, nul-terminated string specifying the path of the program entry point. This may point to an empty string. */
    FILETIME                        LastWriteTime;                             /* The last write time of the most recently modified HLSL source file(s). */
    FILETIME                        LastBuildTime;                             /* The last write time of the bytecode file or of the last compilation. */
} GPU_PROGRAM_DEBUGINFO_D3D12;

/* @summary Define the data associated with a cache of compiled GPU program bytecode used for creating Direct3D 12 shaders.
 * All GPU programs in a program cache instance are of the same type (for example, all compute shaders).
 * TODO: This does not need to use data tables. Instead, just use a single array.
 * The GPU_PROGRAM_TABLE will use data tables to map GPU_PROGRAM_HANDLE to a {CACHE*, index}.
 * The serialized form of a program cache consists of the set of bytecode and the string tables.
 */
typedef struct GPU_PROGRAM_CACHE_D3D12 {
    char                          **DefineSymbols;                             /* An array of nul-terminated strings, interned in the string table, specifying the preprocessor symbols to define when compiling GPU program source code. */
    char                          **DefineValues;                              /* An array of nul-terminated strings, interned in the string table, specifying the values of the preprocessor symbols when compiling GPU program source code. */
    char_native_t                 **IncludePaths;                              /* An array of nul-terminated strings, interned in the string table, specifying the absolute paths of the directories to search for includes. */
    uint32_t                        IncludePathCount;                          /* The number of nul-terminated path strings in the IncludePaths array. */
    uint32_t                        DefineCount;                               /* The number of nul-terminated symbols and values in the DefineSymbols and DefineValues arrays. */
    uint32_t                        ProgramType;                               /* One of the values of the GPU_PROGRAM_TYPE enumeration specifying the type of GPU programs in the cache. All programs in the cache must be of the same type. */
    TABLE_DESC                      TableDesc;                                 /* The table descriptor, used for calling the data table functions. */
    TABLE_INDEX                     TableIndex;                                /* The table index used to map a GPU_PROGRAM_HANDLE to a dense array index. */
    TABLE_DATA                      ByteCodeData;                              /* The table data storing the stream of GPU_PROGRAM_BYTECODE_D3D12 instances. */
    TABLE_DATA                      DebugInfoData;                             /* The table data storing the stream of GPU_PROGRAM_DEBUGINFO_D3D12 instances. */
    TABLE_DATA                     *TableStreams[2];                           /* An array of pointers to the table data streams. */
    STRING_TABLE                   *DebugInfoStrings;                          /* A string table storing the UTF-8 strings referenced by the GPU_PROGRAM_DEBUGINFO_D3D12 records.*/
    STRING_TABLE                   *CompilationStrings;                        /* A string table storing the strings referenced by the DefineSymbols, DefineValues and IncludePaths arrays. */
} GPU_PROGRAM_CACHE_D3D12;

/* @summary Define the data used to configure the creation of a GPU program cache.
 */
typedef struct GPU_PROGRAM_CACHE_INIT_D3D12 {
    struct D3DCOMPILERAPI_DISPATCH *D3DCompilerDispatch;                       /* The dispatch table used to access D3D compiler functions used for runtime GPU program compilation. */
    char const                    **DefineSymbols;                             /* An array of nul-terminated strings specifying the preprocessor symbols to define when compiling GPU program source code. */
    char const                    **DefineValues;                              /* An array of nul-terminated strings specifying the values of the preprocessor symbols when compiling GPU program source code. */
    char_native_t const           **IncludePaths;                              /* An array of nul-terminated strings specifying the absolute paths of the directories to search for includes. */
    uint32_t                        IncludePathCount;                          /* The number of nul-terminated path strings in the IncludePaths array. */
    uint32_t                        DefineCount;                               /* The number of nul-terminated symbols and values in the DefineSymbols and DefineValues arrays. */
    uint32_t                        ProgramCapacity;                           /* The maximum number of GPU programs that can be stored in the cache. */
    uint32_t                        ProgramType;                               /* One of the values of the GPU_PROGRAM_TYPE enumeration specifying the type of GPU programs in the cache. All programs in the cache must be of the same type. */
} GPU_PROGRAM_CACHE_INIT_D3D12;

#endif /* __PIL_GPU_PROGRAM_D3D12_H__ */

