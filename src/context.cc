/**
 * context.cc: Implement the PIL context management functions.
 */
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "pil.h"
#include "memmgr.h"

/* @summary Define the "global application data" carried throughout the application.
 * This represents the application's connection to the Platform Interface Layer.
 */
typedef struct PIL_CONTEXT {
    MEMORY_ARENA GlobalArena;                                                  /* The memory arena used for allocating application-lifetime memory. */
    MEMORY_ARENA ScratchArena;                                                 /* The memory arena used for function-lifetime scratch memory. */
    char         AppName[64];                                                  /* The name of the hosting application, terminated with a nul. */
} PIL_CONTEXT;

/* @summary Copy at most nmax bytes from one string buffer to another, ensuring the destination buffer is nul-terminated.
 * @param dst A pointer to the first byte to write.
 * @param src A pointer to the first byte to read.
 * @param nmax The maximum number of bytes that can be written to dst, including the nul terminator.
 */
static PIL_INLINE char*
PIL_Strncpy
(
    char       * PIL_RESTRICT dst, 
    char const * PIL_RESTRICT src, 
    size_t                   nmax
)
{
    char       *d = dst;
    char const *s = src;
    size_t      n = 0;
    size_t      m = nmax - 1;

    assert(nmax > 0);
    assert(src != NULL);
    assert(dst != NULL);
    
    while (n != m && *s) {
        d[n++] = *s++;
    } d[n] = 0;
    return dst;
}

PIL_API(struct PIL_CONTEXT*)
PIL_ContextCreate
(
    struct PIL_CONTEXT_INIT *init
)
{
    PIL_CONTEXT            *ctx = NULL;
    MEMORY_ARENA_INIT gmem_init;
    MEMORY_ARENA_INIT smem_init;

    if (init == NULL) {
        assert(init != NULL);
        return NULL;
    }
    if (init->ApplicationName == NULL) {
        assert(init->ApplicationName != NULL);
        return NULL;
    }

    /* allocate and zero the main context object */
    if ((ctx = (PIL_CONTEXT*) HostMemoryAllocateHeap(NULL, sizeof(PIL_CONTEXT), PIL_AlignOf(PIL_CONTEXT))) == NULL) {
        goto cleanup_and_fail;
    }
    memset(ctx, 0, sizeof(PIL_CONTEXT));

    /* specify global memory attributes.
     * global memory allocations persist for the lifetime of the context.
     */
    memset(&gmem_init, 0, sizeof(MEMORY_ARENA_INIT));
    gmem_init.AllocatorName    = "PIL Global Memory";
    gmem_init.ReserveSize      = 64ULL * 1024ULL * 1024ULL; /* 64MB */
    gmem_init.CommittedSize    =  4ULL * 1024ULL * 1024ULL; /*  4MB */
    gmem_init.AllocatorType    = MEMORY_ALLOCATOR_TYPE_HOST_VMM;
    gmem_init.AllocatorTag     = MakeAllocatorTag('G','M','E','M');
    gmem_init.AllocationFlags  = HOST_MEMORY_ALLOCATION_FLAGS_READWRITE;
    gmem_init.ArenaFlags       = MEMORY_ARENA_FLAG_INTERNAL;

    /* specify scratch memory attributes.
     * scratch memory allocations persiste for the lifetime of a single function call.
     */
    memset(&smem_init, 0, sizeof(MEMORY_ARENA_INIT));
    smem_init.AllocatorName    = "PIL Scratch Memory";
    smem_init.ReserveSize      =  4ULL * 1024ULL * 1024ULL; /*  4MB */
    smem_init.CommittedSize    = 64ULL * 1024ULL;           /* 64KB */
    smem_init.AllocatorType    = MEMORY_ALLOCATOR_TYPE_HOST_VMM;
    smem_init.AllocatorTag     = MakeAllocatorTag('S','M','E','M');
    smem_init.AllocationFlags  = HOST_MEMORY_ALLOCATION_FLAGS_READWRITE;
    smem_init.ArenaFlags       = MEMORY_ARENA_FLAG_INTERNAL;

    /* begin initializing the context object */
    PIL_Strncpy(ctx->AppName, init->ApplicationName, PIL_CountOf(ctx->AppName));
    if (MemoryArenaCreate(&ctx->GlobalArena, &gmem_init) != 0) {
        goto cleanup_and_fail;
    }
    if (MemoryArenaCreate(&ctx->ScratchArena, &gmem_init) != 0) {
        goto cleanup_and_fail;
    }
    /* TODO:
     * Load Vulkan runtime
     * Create Vulkan instance
     * Enumerate Vulkan physical devices
     * Create notification window (this is platform-specific)
     * Enumerate displays
     * Enumerate input devices
     * Enumerate audio devices
     */
    return ctx;

cleanup_and_fail:
    if (ctx) {
        MemoryArenaDelete(&ctx->ScratchArena);
        MemoryArenaDelete(&ctx->GlobalArena);
        HostMemoryFreeHeap(ctx);
    }
    return NULL;
}

PIL_API(void)
PIL_ContextDelete
(
    struct PIL_CONTEXT *context
)
{
    if (context) {
        MemoryArenaDelete(&context->ScratchArena);
        MemoryArenaDelete(&context->GlobalArena);
        HostMemoryFreeHeap(context);
    }
}

