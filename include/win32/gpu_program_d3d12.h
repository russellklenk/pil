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
#   ifndef __PIL_GPU_PROGRAM_H__
#       include "gpu_program.h"
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

/* @summary Define constants related to GPU program objects.
 * D3D12_GPU_PROGRAM_COUNT_MAX: The maximum number of programs that can be loaded into a single GPU_PROGRAM_CACHE_D3D12.
 */
#ifndef GPU_PROGRAM_D3D12_CONSTANTS
#   define GPU_PROGRAM_D3D12_CONSTANTS
#endif

/* @summary Define the data used to store the compiled GPU program bytecode.
 * GPU program bytecode is converted into an architecture-specific representation by the GPU driver.
 * Alongside the bytecode, additional data is stored for debugging purporses and to support runtime change detection.
 */
typedef struct GPU_PROGRAM_BYTECODE_D3D12 {
    ID3DBlob                       *ByteCode;                                  /* The blob containing the compiled program bytecode. */
    char_native_t                  *SourcePath;                                /* Pointer to an interned, nul-terminated string specifying the path of the main HLSL source file. This may point to an empty string. */
    char                           *EntryPoint;                                /* Pointer to an interned, nul-terminated string specifying the program entry point. This may point to an empty string. */
    FILETIME                        LastWriteTime;                             /* The last write time of the most recently modified HLSL source file(s). */
    FILETIME                        LastBuildTime;                             /* The last write time of the bytecode file or of the last compilation. */
} GPU_PROGRAM_BYTECODE_D3D12;

/* @summary Define the data associated with a cache of compiled GPU program bytecode used for creating Direct3D 12 shaders.
 * All GPU programs in a program cache instance are of the same type (for example, all compute shaders).
 */
typedef struct GPU_PROGRAM_CACHE_D3D12 {
    GPU_PROGRAM_BYTECODE_D3D12     *BytecodeList;                              /* An array of ProgramCapacity program bytecode records, of which the first ProgramCount are valid. */
    uint32_t                        ProgramType;                               /* One of the values of the GPU_PROGRAM_TYPE enumeration specifying the type of GPU programs in the cache. All programs in the cache must be of the same type. */
    uint32_t                        ProgramCount;                              /* The number of valid program bytecode records in the BytecodeList array. */
    uint32_t                        ProgramCapacity;                           /* The maximum number of program bytecode records that can be stored in this cache. */
    uint32_t                        CacheFlags;                                /* One or more bitwise-OR'd values of the GPU_PROGRAM_CACHE_FLAGS enumeration. */
    FILETIME                        LastWriteTime;                             /* A timestamp value set to the most recent LastWriteTime of any programs stored in the cache. */
    FILETIME                        LastUpdateTime;                            /* A timestamp value set to the time of the last program cache modification. */
    D3D_SHADER_MACRO               *CompilerDefines;                           /* A {NULL, NULL} terminated array of conditional compilation symbols and values. */
    char                           *TargetModel;
    char                          **DefineSymbols;                             /* An array of nul-terminated strings, interned in the string table, specifying the preprocessor symbols to define when compiling GPU program source code. */
    char                          **DefineValues;                              /* An array of nul-terminated strings, interned in the string table, specifying the values of the preprocessor symbols when compiling GPU program source code. */
    char_native_t                 **IncludePaths;                              /* An array of nul-terminated strings, interned in the string table, specifying the absolute paths of the directories to search for includes. */
    uint32_t                        IncludePathCount;                          /* The number of nul-terminated path strings in the IncludePaths array. */
    uint32_t                        DefineCount;                               /* The number of nul-terminated symbols and values in the DefineSymbols and DefineValues arrays. */
    STRING_TABLE                   *DebugInfoStrings;                          /* A string table storing the UTF-8 strings referenced by the bytecode records.*/
    STRING_TABLE                   *CompilationStrings;                        /* A string table storing the strings referenced by the DefineSymbols, DefineValues and IncludePaths arrays. */
    D3DCOMPILERAPI_DISPATCH         D3DCompilerDispatch;                       /* The dispatch table used to access D3D compiler functions used for runtime GPU program compilation. */
} GPU_PROGRAM_CACHE_D3D12;

#endif /* __PIL_GPU_PROGRAM_D3D12_H__ */

