/**
 * @summary gpu_program_d3d12.cc: Implement the functions from gpu_program.h 
 * for Windows platforms utilizing the Direct3D 12 API.
 */
#include "memmgr.h"
#include "win32/gpu_program_d3d12.h"

static inline int64_t
FILETIMEToUnixTime
(
    FILETIME filetime
)
{   /* 10000000 is the number of 100ns intervals in one second.
     * 11644473600 is the number of seconds between Jan 1 1601 00:00 and 
     * Jan 1 1970 00:00 UTC (the epoch difference.) */
    ULARGE_INTEGER ui;
    ui.LowPart  = filetime.dwLowDateTime;
    ui.HighPart = filetime.dwHighDateTime;
    return (int64_t)((ui.QuadPart / 10000000ULL) - 11644473600ULL);
}

static inline FILETIME
UnixTimeToFILETIME
(
    int64_t unixtime
)
{   /* 10000000 is the number of 100ns intervals in one second.
     * 116444736000000000 is the number of 100ns intervals between 
     * Jan 1 1601 00:00 and Jan 1 1970 00:00 UTC (the epoch difference.) */
    ULARGE_INTEGER ui;
    FILETIME filetime;
    ui.QuadPart = (uint64_t)((unixtime * 10000000ULL) + 116444736000000000ULL);
    filetime.dwLowDateTime  = ui.LowPart;
    filetime.dwHighDateTime = ui.HighPart;
    return filetime;
}

PIL_API(struct GPU_PROGRAM_CACHE*)
GpuProgramCacheCreate
(
    struct GPU_PROGRAM_CACHE_INIT *init
)
{
    GPU_PROGRAM_BYTECODE_D3D12 *bytecode = nullptr;
    GPU_PROGRAM_CACHE_D3D12      *cache_ = nullptr;
    D3D_SHADER_MACRO            *defines = nullptr;
    char                       **symbols = nullptr;
    char                        **values = nullptr;
    char_native_t                **paths = nullptr;
    STRING_TABLE     *debug_info_strings = nullptr;
    STRING_TABLE       *compiler_strings = nullptr;
    STRING_TABLE_INIT    debug_info_init = {};
    STRING_TABLE_INIT      compiler_init = {};
    FILETIME               filetime_none = {};
    uint32_t           d3dcompiler_flags = D3DCOMPILERAPI_LOADER_FLAGS_NONE;
    uint32_t                    nb_debug = 0;
    uint32_t             nb_debug_commit = 0;
    uint32_t                 nb_compiler = 0;
    uint32_t                     n_bytes;
    uint32_t                     n_chars;
        
    if ((bytecode = HostMemoryAllocateHeapArray(GPU_PROGRAM_BYTECODE_D3D12, init->ProgramCapacity)) == nullptr) {
        goto cleanup_and_fail;
    }
    if ((defines  = HostMemoryAllocateHeapArray(D3D_SHADER_MACRO, init->DefineCount + 1)) == nullptr) {
        goto cleanup_and_fail;
    }
    ZeroMemory(bytecode, sizeof(GPU_PROGRAM_BYTECODE_D3D12) * init->ProgramCapacity);
    ZeroMemory(defines , sizeof(D3D_SHADER_MACRO) * (init->DefineCount + 1));

    /* Figure out sizing for the string tables.
     * The debug information string table is potentially large, and partially-committed.
     * The compiler information string table is fully-committed.
     * On Windows, the maximum total length of a path string is 32K characters.
     * Assume that the average entry point name is 64 characters or less.
     */
    StringLengthUtf8(init->TargetModel, &n_bytes, &n_chars); nb_compiler = n_bytes;
    for (uint32_t i  = 0, n = init->IncludePathCount; i < n; ++i) {
        nb_compiler +=(uint32_t) NativeStringLengthBytes(init->IncludePaths[i]);
    }
    for (uint32_t i  = 0, n = init->DefineCount; i < n; ++i) {
        StringLengthUtf8(init->DefineSymbols[i], &n_bytes, &n_chars); nb_compiler += n_bytes;
        StringLengthUtf8(init->DefineValues [i], &n_bytes, &n_chars); nb_compiler += n_bytes;
    }
    nb_debug         = (init->ProgramCapacity * 32768 * sizeof(WCHAR)); /* For path information */
    nb_debug        += (init->ProgramCapacity *    64 * sizeof(char )); /* For entry points */
    nb_debug        +=  sizeof(WCHAR); /* an extra nul */
    nb_debug_commit  = (init->ProgramCapacity *   256 * sizeof(WCHAR)); /* For path information */
    nb_debug_commit += (init->ProgramCapacity *    20 * sizeof(char )); /* For entry points */
    nb_debug_commit +=  sizeof(WCHAR); /* an extra nul */

    /* Initialize the string table for storing compilation options.
     * There's an extra string for storing the target shader model, 
     * and one for storing an empty string. */
    compiler_init.MaxDataSize     = nb_compiler;
    compiler_init.DataCommitSize  = nb_compiler;
    compiler_init.MaxStringCount  = init->IncludePathCount + (init->DefineCount * 2) + 2;
    compiler_init.InitialCapacity = init->IncludePathCount + (init->DefineCount * 2) + 2;
    if ((compiler_strings = StringTableCreate(&compiler_init)) == nullptr) {
        goto cleanup_and_fail;
    }

    /* Initialize the string table for storing file paths and entry points.
     * There's an extra string for storing an empty string. */
    debug_info_init.MaxDataSize     = nb_debug;
    debug_info_init.DataCommitSize  = nb_debug_commit;
    debug_info_init.MaxStringCount  =(init->ProgramCapacity * 2) + 1;
    debug_info_init.InitialCapacity =(init->ProgramCapacity * 2) + 1;
    if ((debug_info_strings = StringTableCreate(&debug_info_init)) == nullptr) {
        goto cleanup_and_fail;
    }

    /* Populate the compiler settings string table up front */
    cache_->TargetModel = StringTableInternUtf8(compiler_strings, init->TargetModel);
    for (uint32_t i = 0 , n = init->IncludePathCount; i < n; ++i) {
        paths  [i]  =(char_native_t*) StringTableInternUtf16(compiler_strings, init->IncludePaths[i]);
    }
    for (uint32_t i = 0 , n = init->DefineCount; i < n; ++i) {
        symbols[i]  = StringTableInternUtf8 (compiler_strings, init->DefineSymbols[i]);
        values [i]  = StringTableInternUtf8 (compiler_strings, init->DefineValues [i]);
        defines[i]  = D3D_SHADER_MACRO { symbols[i], values[i] };
    }

    if (init->ProgramCacheFlags & GPU_PROGRAM_CACHE_BUILD_ONLY) {
        /* Enable development-only D3DCompiler APIs */
        d3dcompiler_flags = D3DCOMPILERAPI_LOADER_FLAG_DEVELOPMENT;
    }
    if (D3DCompilerApiPopulateDispatch(&cache_->D3DCompilerDispatch, d3dcompiler_flags) != 0) {
        goto cleanup_and_fail;
    }
    cache_->BytecodeList       = bytecode;
    cache_->ProgramType        = init->ProgramType;
    cache_->ProgramCount       = 0;
    cache_->ProgramCapacity    = init->ProgramCapacity;
    cache_->CacheFlags         = init->ProgramCacheFlags;
    cache_->LastWriteTime      = filetime_none;
    cache_->LastUpdateTime     = filetime_none;
    cache_->CompilerDefines    = defines;
    cache_->DefineSymbols      = symbols;
    cache_->DefineValues       = values;
    cache_->IncludePaths       = paths;
    cache_->DefineCount        = init->DefineCount;
    cache_->IncludePathCount   = init->IncludePathCount;
    cache_->DebugInfoStrings   = debug_info_strings;
    cache_->CompilationStrings = compiler_strings;
    return (struct GPU_PROGRAM_CACHE*) cache_;

cleanup_and_fail:
    StringTableDelete(debug_info_strings);
    StringTableDelete(compiler_strings);
    HostMemoryFreeHeap(bytecode);
    HostMemoryFreeHeap(defines);
    HostMemoryFreeHeap(symbols);
    HostMemoryFreeHeap(values);
    HostMemoryFreeHeap(paths);
    HostMemoryFreeHeap(cache_);
    return nullptr;
}

PIL_API(void)
GpuProgramCacheDelete
(
    struct GPU_PROGRAM_CACHE *cache
)
{
    GPU_PROGRAM_CACHE_D3D12 *cache_ =(GPU_PROGRAM_CACHE_D3D12*) cache;
    if (cache_ && cache_->ProgramType != GPU_PROGRAM_TYPE_UNKNOWN) {
        GPU_PROGRAM_BYTECODE_D3D12 *bytecode = cache_->BytecodeList;
        ID3DBlob                       *blob = nullptr;

        for (uint32_t i = 0, n = cache_->ProgramCount; i < n; ++i) {
            if ((blob = bytecode[i].ByteCode) != nullptr) {
                bytecode[i].ByteCode = nullptr;
                blob->Release();
            }
        }
        if (cache_->IncludePathCount > 0) {
            HostMemoryFreeHeap(cache_->IncludePaths);
            cache_->IncludePathCount = 0;
        }
        if (cache_->DefineCount > 0) {
            HostMemoryFreeHeap(cache_->DefineSymbols);
            HostMemoryFreeHeap(cache_->DefineValues);
            cache_->DefineCount = 0;
        }
        D3DCompilerApiInvalidateDispatch(&cache_->D3DCompilerDispatch);
        StringTableDelete (cache_->CompilationStrings);
        StringTableDelete (cache_->DebugInfoStrings);
        HostMemoryFreeHeap(cache_->CompilerDefines);
        HostMemoryFreeHeap(cache_->BytecodeList);
        HostMemoryFreeHeap(cache_);
    }
}

PIL_API(uint32_t)
GpuProgramCacheGetProgramCount
(
    struct GPU_PROGRAM_CACHE *cache
)
{
    GPU_PROGRAM_CACHE_D3D12 *cache_ = (GPU_PROGRAM_CACHE_D3D12*) cache;
    return cache_->ProgramCount;
}

PIL_API(int)
GpuProgramCacheGetProgram
(
    struct GPU_PROGRAM_BYTECODE *desc, 
    struct GPU_PROGRAM_CACHE   *cache, 
    uint32_t            program_index
)
{
    GPU_PROGRAM_CACHE_D3D12    *cache_ = (GPU_PROGRAM_CACHE_D3D12*) cache;
    if (desc && cache && program_index < cache_->ProgramCount) {
        ID3DBlob *blob = cache_->BytecodeList[program_index].ByteCode;
        desc->ByteCode =(uint8_t *) blob->GetBufferPointer();
        desc->SizeBytes=(uint64_t ) blob->GetBufferSize();
        return 0;
    } else {
        assert(desc  != nullptr);
        assert(cache != nullptr);
        assert(program_index < cache_->ProgramCount);
        if (desc != nullptr) {
            desc->ByteCode   = nullptr;
            desc->SizeBytes  = 0;
        } return -1;
    }
}

PIL_API(int)
GpuProgramCacheStoreProgram
(
    uint32_t             *o_program_index, 
    struct GPU_PROGRAM_BYTECODE *bytecode, 
    struct GPU_PROGRAM_DEBUG_INFO  *debug, 
    struct GPU_PROGRAM_CACHE       *cache
)
{
    GPU_PROGRAM_CACHE_D3D12  *cache_ =(GPU_PROGRAM_CACHE_D3D12*) cache;
    GPU_PROGRAM_BYTECODE_D3D12  *rec = nullptr;
    ID3DBlob                   *blob = nullptr;
    uint32_t           program_index = cache_->ProgramCount;
    HRESULT                      res = S_OK;
    SYSTEMTIME                   now = {};
    FILETIME            filetime_now = {};
    GPU_PROGRAM_DEBUG_INFO null_info = {};

    if (bytecode == nullptr || bytecode->ByteCode == nullptr || bytecode->SizeBytes == 0) {
        /* The bytecode is required */
        return -1;
    }
    if (cache_->ProgramCount == cache_->ProgramCapacity) {
        /* The cache is full - need larger capacity */
        assert(cache_->ProgramCount < cache_->ProgramCapacity);
        return -1;
    }
    if (FAILED((res = cache_->D3DCompilerDispatch.D3DCreateBlob((SIZE_T) bytecode->SizeBytes, &blob)))) {
        /* Failed to create bytecode blob */
        return -1;
    }
    if (debug == nullptr) {
        GetSystemTime(&now);
        SystemTimeToFileTime(&now, &filetime_now);
        debug->LastWriteTime = FILETIMEToUnixTime(filetime_now);
        debug->LastBuildTime = FILETIMEToUnixTime(filetime_now);
        debug = &null_info;
    }
    /* TODO: Provide an API to create the blob, and allow the caller to map it */
    CopyMemory(blob->GetBufferPointer(), bytecode->ByteCode, bytecode->SizeBytes);
    /* Intern the debug strings, if any are provided */
    rec = &cache_->BytecodeList[program_index];
    rec->ByteCode      = blob;
    rec->SourcePath    =(char_native_t*) StringTableInternUtf16(cache_->DebugInfoStrings, debug->SourcePath);
    rec->EntryPoint    =(char         *) StringTableInternUtf8 (cache_->DebugInfoStrings, debug->EntryPoint);
    rec->LastWriteTime = UnixTimeToFILETIME(debug->LastWriteTime);
    rec->LastBuildTime = UnixTimeToFILETIME(debug->LastBuildTime);
    if (o_program_index) {
       *o_program_index= program_index;
    } return 0;
}

