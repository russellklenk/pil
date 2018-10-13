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
            o_block->BytesCommitted = n_bytes;
            o_block->BytesReserved  = n_bytes;
            o_block->BlockOffset    = 0;
            o_block->HostAddress    =(uint8_t*) p;
            o_block->AllocatorType  = MEMORY_ALLOCATOR_TYPE_HOST_HEAP;
            o_block->AllocationTag  = MAKEFOURCC('H','E','A','P');
        } else {
            ZeroMemory(o_block, sizeof(MEMORY_BLOCK));
        }
    } return p;
}

