/**
 * @summary Implement the platform-specific components of the memory management
 * APIs for the Windows desktop platform.
 */
#include <Windows.h>
#include <malloc.h>
#include "memmgr.h"

PIL_API(void*)
HostMemoryAllocateHeap
(
    struct MEMORY_BLOCK *o_block, 
    size_t               n_bytes, 
    size_t             alignment
)
{
    void *p = _aligned_malloc(n_bytes, alignment);
    if (o_block) {
        if (p) {
            o_block->BytesCommitted  = n_bytes;
            o_block->BytesReserved   = n_bytes;
            o_block->BlockOffset     = 0;
            o_block->HostAddress     =(uint8_t*) p;
            o_block->AllocationFlags = HOST_MEMORY_ALLOCATION_FLAGS_READWRITE | HOST_MEMORY_ALLOCATION_FLAG_NOGUARD;
            o_block->AllocationTag   = MAKEFOURCC('H','E','A','P');
        } else {
            ZeroMemory(o_block, sizeof(MEMORY_BLOCK));
        }
    } return p;
}

PIL_API(void)
HostMemoryFreeHeap
(
    void *host_addr
)
{
    _aligned_free(host_addr);
}

PIL_API(void*)
HostMemoryReserveAndCommit
(
    struct MEMORY_BLOCK *o_block, 
    size_t         reserve_bytes, 
    size_t          commit_bytes, 
    uint32_t         alloc_flags
)
{
    SYSTEM_INFO sysinfo;
    void          *base = NULL;
    size_t    page_size = 0;
    size_t        extra = 0;
    size_t  min_reserve = 0;
    DWORD        access = 0;
    DWORD         flags = MEM_RESERVE;
    DWORD         error = ERROR_SUCCESS;

    GetNativeSystemInfo(&sysinfo);
    min_reserve = sysinfo.dwPageSize;
    page_size   = sysinfo.dwPageSize;
    if (reserve_bytes < min_reserve) {
        reserve_bytes = min_reserve;
    }
    if (commit_bytes > reserve_bytes) {
        assert(commit_bytes <= reserve_bytes);
        SetLastError(ERROR_INVALID_PARAMETER);
        return NULL;
    }

    /* VMM allocations are rounded up to the next even multiple of the system 
     * page size, and have a starting address that is an even multiple of the 
     * system allocation granularity (usually 64KB) */
    reserve_bytes = PIL_AlignUp(reserve_bytes, page_size);
    if (alloc_flags == HOST_MEMORY_ALLOCATION_FLAGS_DEFAULT)
        alloc_flags  = HOST_MEMORY_ALLOCATION_FLAGS_READWRITE;
    if (alloc_flags  & HOST_MEMORY_ALLOCATION_FLAG_READ)
        access = PAGE_READONLY;
    if (alloc_flags  & HOST_MEMORY_ALLOCATION_FLAG_WRITE)
        access = PAGE_READWRITE;
    if (alloc_flags & HOST_MEMORY_ALLOCATION_FLAG_EXECUTE) {
        access = PAGE_EXECUTE_READWRITE;
        commit_bytes = reserve_bytes;
    }
    if((alloc_flags & HOST_MEMORY_ALLOCATION_FLAG_NOGUARD) == 0) {
        /* commit an extra page as a guard page */
        extra = page_size;
    }

    if ((base = VirtualAlloc(NULL, reserve_bytes + extra, flags, access)) == NULL) {
        /* reservation failed */
        goto cleanup_and_fail;
    }
    if (commit_bytes > 0) {
        commit_bytes = PIL_AlignUp(commit_bytes, page_size);
        if (VirtualAlloc(base, commit_bytes, MEM_COMMIT, access) != base) {
            /* commit failed */
            goto cleanup_and_fail;
        }
    }
    if (extra > 0) {
        if (VirtualAlloc((uint8_t*)base + reserve_bytes, page_size, MEM_COMMIT, access|PAGE_GUARD) == NULL) {
            /* commit failed for guard page */
            goto cleanup_and_fail;
        }
    }
    if (o_block) {
        o_block->BytesCommitted  = commit_bytes;
        o_block->BytesReserved   = reserve_bytes;
        o_block->BlockOffset     = 0;
        o_block->HostAddress     =(uint8_t*) base;
        o_block->AllocationFlags = alloc_flags;
        o_block->AllocationTag   = MAKEFOURCC('V','M','E','M');
    }
    return base;

cleanup_and_fail:
    error = GetLastError();
    VirtualFree(base, 0, MEM_RELEASE);
    if (o_block) {
        ZeroMemory(o_block, sizeof(MEMORY_BLOCK));
    } SetLastError(error);
    return NULL;
}

PIL_API(bool)
HostMemoryIncreaseCommitment
(
    struct MEMORY_BLOCK *o_block, 
    struct MEMORY_BLOCK   *block, 
    size_t          commit_bytes
)
{
    uint8_t      *address;
    uint64_t       blkofs;
    uint64_t  num_reserve;
    uint64_t   old_commit;
    uint64_t   new_commit;
    uint32_t  alloc_flags;
    uint32_t    alloc_tag;
    uint64_t max_increase;
    uint64_t req_increase;

    if (block == NULL) {
        assert(block != NULL);
        goto cleanup_and_fail;
    }
    if (block->BytesReserved == 0 || block->HostAddress == NULL) {
        assert(block->BytesReserved != 0);
        assert(block->HostAddress != NULL);
        goto cleanup_and_fail;
    }

    /* copy values out of block to avoid aliasing */
    old_commit   = block->BytesCommitted;
    new_commit   = block->BytesCommitted;
    num_reserve  = block->BytesReserved;
    blkofs       = block->BlockOffset;
    address      = block->HostAddress;
    alloc_flags  = block->AllocationFlags;
    alloc_tag    = block->AllocationTag;
    max_increase = num_reserve  - old_commit;
    req_increase = commit_bytes - old_commit;

    if (block->BytesCommitted < commit_bytes) {
        SYSTEM_INFO sysinfo;
        size_t    page_size = 0;
        DWORD        access = 0;

        if (req_increase > max_increase) {
            assert(req_increase <= max_increase);
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            goto cleanup_and_fail;
        }

        GetNativeSystemInfo(&sysinfo);
        page_size  = sysinfo.dwPageSize;
        new_commit = PIL_AlignUp(old_commit+req_increase, page_size);
        if (alloc_flags & HOST_MEMORY_ALLOCATION_FLAG_READ)
            access = PAGE_READONLY;
        if (alloc_flags & HOST_MEMORY_ALLOCATION_FLAG_WRITE)
            access = PAGE_READWRITE;
        if (alloc_flags & HOST_MEMORY_ALLOCATION_FLAG_EXECUTE)
            access = PAGE_EXECUTE_READWRITE;
        if (VirtualAlloc(address, new_commit, MEM_COMMIT, access) == NULL) {
            goto cleanup_and_fail;
        }
    }
    if (o_block) {
        o_block->BytesCommitted  = new_commit;
        o_block->BytesReserved   = num_reserve;
        o_block->BlockOffset     = blkofs;
        o_block->HostAddress     = address;
        o_block->AllocationFlags = alloc_flags;
        o_block->AllocationTag   = alloc_tag;
    }
    return true;

cleanup_and_fail:
    if (o_block) {
        if (block) {
            CopyMemory(o_block, block, sizeof(MEMORY_BLOCK));
        } else {
            ZeroMemory(o_block, sizeof(MEMORY_BLOCK));
        }
    }
    return false;
}

PIL_API(void)
HostMemoryFlush
(
    struct MEMORY_BLOCK const *block
)
{
    (void) FlushInstructionCache(GetCurrentProcess(), block->HostAddress, block->BytesCommitted);
}

PIL_API(void)
HostMemoryRelease
(
    void *host_addr
)
{
    VirtualFree(host_addr, 0, MEM_RELEASE);
}

#if 0
PIL_API(int)
MemoryArenaCreate
(
    struct MEMORY_ARENA         *o_arena, 
    struct MEMORY_ARENA_INIT const *init
);

PIL_API(void)
MemoryArenaDelete
(
    struct MEMORY_ARENA *arena
);

PIL_API(void*)
MemoryArenaAllocateHost
(
    struct MEMORY_BLOCK *o_block, 
    struct MEMORY_ARENA   *arena, 
    size_t                  size, 
    size_t             alignment
);
#endif

