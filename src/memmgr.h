/**
 * @summary memmgr.h: Define types and functions related to internal memory 
 * management. Within the PIL, most memory is managed by directly interacting 
 * with the virtual memory interface to the host operating system.
 */
#ifndef __PIL_MEMMGR_H__
#define __PIL_MEMMGR_H__

#pragma once

#ifndef PIL_NO_INCLUDES
#   ifndef __PIL_H__
#       include "pil.h"
#   endif
#endif

/* @summary Allocate host memory with the correct size and alignment for an instance of a given type from a memory arena.
 * @param _arena The PIL_MEMORY_ARENA from which the allocation is being made.
 * @param _type A typename, such as int, specifying the type being allocated.
 * @return A pointer to the start of the allocated memory block, or NULL.
 */
#ifndef MemoryArenaAllocateHostType
#define MemoryArenaAllocateHostType(_arena, _type)                             \
    ((_type*) MemoryArenaAllocateHost(NULL, (_arena), sizeof(_type), PIL_ALIGN_OF(_type)))
#endif

/* @summary Allocate memory with the correct size and alignment for an array of instance of a given type from a memory arena.
 * @param _arena The PIL_MEMORY_ARENA from which the allocation is being made.
 * @param _type A typename, such as int, specifying the type being allocated.
 * @param _count The number of elements in the array.
 * @return A pointer to the start of the allocated memory block, or NULL.
 */
#ifndef MemoryArenaAllocateHostArray
#define MemoryArenaAllocateHostArray(_arena, _type, _count)                    \
    ((_type*) MemoryArenaAllocateHost(NULL, (_arena), sizeof(_type) * (_count), PIL_ALIGN_OF(_type)))
#endif

/* @summary Allocate memory with the given object size and alignment for an array of object data from a memory arena.
 * @param _arena The PIL_MEMORY_ARENA from which the allocation is being made.
 * @param _objsize The object size, in bytes.
 * @param _count The number of elements in the array.
 * @param _align The object alignment, in bytes.
 * @return A pointer to the start of the allocated memory block, or NULL.
 */
#ifndef MemoryArenaAllocateHostArrayRaw
#define MemoryArenaAllocateHostArrayRaw(_arena, _objsize, _align, _count)  \
    ((uint8_t*) MemoryArenaAllocateHost(NULL, (_arena), (_objsize) * (_count), (_align)))
#endif

/* @summary Forward-declare the types exported by this module.
 */
struct  MEMORY_BLOCK;
struct  MEMORY_ARENA;
struct  MEMORY_ARENA_INIT;
struct  MEMORY_ARENA_MARKER;

/* @summary A union used to specify an offset (for device memory allocations) or a base address (for host memory allocations).
 */
typedef union ADDRESS_OR_OFFSET {
    uint64_t                BaseOffset;                                        /* Specifies the base offset from some starting address which may or may not be visible to the host. */
    void                   *HostAddress;                                       /* Specifies the base address in host-visible memory. */
} ADDRESS_OR_OFFSET;

/* @summary Define the data associated with a host or device memory allocation.
 */
typedef struct MEMORY_BLOCK {
    uint64_t                BytesCommitted;                                    /* The number of bytes that can be accessed by the application. */
    uint64_t                BytesReserved;                                     /* The number of bytes of process address space reserved by the allocation. */
    uint64_t                BlockOffset;                                       /* The allocation offset. This field is set for both host and device memory allocations. */
    uint8_t                *HostAddress;                                       /* The host-visible memory address. This field is set to NULL for device memory allocations. */
    uint32_t                AllocationFlags;                                   /* One or more bitwise-OR'd values of the HOST_MEMORY_ALLOCATION_FLAGS enumeration. */
    uint32_t                AllocationTag;                                     /* The allocation tag associated with the memory allocator that returned the memory block. */
} MEMORY_BLOCK;

/* @summary Define the data associated with an arena-style memory allocator.
 */
typedef struct MEMORY_ARENA {
    char const             *AllocatorName;                                     /* A nul-terminated string specifying the name of the allocator. Used for debugging. */
    uint64_t                MemoryStart;                                       /* The address or offset of the start of the memory block from which sub-allocations are returned. */
    uint64_t                NextOffset;                                        /* The byte offset of the next permanent allocation to return. */
    uint64_t                MaximumOffset;                                     /* The maximum value of NextOffsetPerm/NextOffsetTemp. */
    uint64_t                NbReserved;                                        /* The number of bytes of reserved address space. */
    uint64_t                NbCommitted;                                       /* The number of bytes of committed address space. */
    uint32_t                AllocatorType;                                     /* One of the values of the MEMORY_ALLOCATOR_TYPE enumeration specifying whether the memory allocator allocates host or device memory. */
    uint32_t                AllocatorTag;                                      /* An opaque 32-bit value used to tag allocations from the arena. */
    uint32_t                AllocationFlags;                                   /* One or more bitwise-OR'd values of the HOST_MEMORY_ALLOCATION_FLAGS or DEVICE_MEMORY_ALLOCATION_FLAGS enumeration. */
    uint32_t                ArenaFlags;                                        /* One or more bitwise-OR'd values of the MEMORY_ARENA_FLAGS enumeration. */
} MEMORY_ARENA;

/* @summary Define the data used to configure an arena-style memory allocator.
 */
typedef struct MEMORY_ARENA_INIT {
    char const             *AllocatorName;                                     /* A nul-terminated string specifying the name of the allocator. Used for debugging. */
    uint64_t                ReserveSize;                                       /* The number of bytes of address space reserved for the memory block. */
    uint64_t                CommittedSize;                                     /* The number of bytes of address space committed in the memory block.  */
    ADDRESS_OR_OFFSET       MemoryStart;                                       /* The offset or host address of the start of the allocated memory block. */
    uint32_t                AllocatorType;                                     /* One of the values of the MEMORY_ALLOCATOR_TYPE enumeration specifying whether the memory allocator allocates host or device memory. */
    uint32_t                AllocatorTag;                                      /* An opaque 32-bit value used to tag allocations from the arena. */
    uint32_t                AllocationFlags;                                   /* One or more bitwise-OR'd values of the HOST_MEMORY_ALLOCATION_FLAGS or DEVICE_MEMORY_ALLOCATION_FLAGS enumeration. */
    uint32_t                ArenaFlags;                                        /* One or more bitwise-OR'd values of the MEMORY_ARENA_FLAGS enumeration. */
} MEMORY_ARENA_INIT;

/* @summary Define the data associated with an arena marker, which represents the state of an arena allocator at a specific point in time.
 * The arena can be reset back to a previously defined marker, invalidating all allocations made since the marked point in time.
 */
typedef struct MEMORY_ARENA_MARKER {
    struct MEMORY_ARENA    *Arena;                                             /* The MEMORY_ARENA from which the marker was obtained. */
    uint64_t                State;                                             /* A value encoding the state of the memory arena when the marker was obtained. */
} MEMORY_ARENA_MARKER;

/* @summary Define the allowed values for memory allocator type. 
 * An allocator can manage either host or device memory. Device memory may not be visible to the host CPU.
 */
typedef enum MEMORY_ALLOCATOR_TYPE {
    MEMORY_ALLOCATOR_TYPE_INVALID           =  0UL,                            /* This value is invalid and should not be used. */
    MEMORY_ALLOCATOR_TYPE_HOST_VMM          =  1UL,                            /* The allocator is a host memory allocator, returning address space from the system virtual memory manager. */
    MEMORY_ALLOCATOR_TYPE_HOST_HEAP         =  2UL,                            /* The allocator is a host memory allocator, returning address space from the system heap. */
    MEMORY_ALLOCATOR_TYPE_DEVICE            =  3UL,                            /* The allocator is a device memory allocator. */
} MEMORY_ALLOCATOR_TYPE;

/* @summary Define various flags that can be bitwise OR'd to control the behavior of a memory arena allocator.
 */
typedef enum MEMORY_ARENA_FLAGS {
    MEMORY_ARENA_FLAGS_NONE                 = (0UL <<  0),                     /* No flags are specified. Specifying no flags will cause arena creation to fail. */
    MEMORY_ARENA_FLAG_INTERNAL              = (1UL <<  0),                     /* The memory arena should allocate memory internally, and free the memory when the arena is destroyed. */
    MEMORY_ARENA_FLAG_EXTERNAL              = (1UL <<  1),                     /* The memory arena uses memory supplied and managed by the application. */
} MEMORY_ARENA_FLAGS;

/* @summary Define various flags that can be bitwise OR'd to control the allocation attributes for a single host memory allocation.
 */
typedef enum HOST_MEMORY_ALLOCATION_FLAGS {
    HOST_MEMORY_ALLOCATION_FLAGS_DEFAULT   = (0UL <<  0),                      /* The memory can be read and written by the host, and ends with a guard page. */
    HOST_MEMORY_ALLOCATION_FLAG_READ       = (1UL <<  0),                      /* The memory can be read by the host. */
    HOST_MEMORY_ALLOCATION_FLAG_WRITE      = (1UL <<  1),                      /* The memory can be written by the host. */
    HOST_MEMORY_ALLOCATION_FLAG_EXECUTE    = (1UL <<  2),                      /* The allocation can contain code that can be executed by the host. */
    HOST_MEMORY_ALLOCATION_FLAG_NOGUARD    = (1UL <<  3),                      /* The allocation will not end with a guard page. */
    HOST_MEMORY_ALLOCATION_FLAGS_READWRITE =                                   /* The committed memory can be read and written by the host. */
        HOST_MEMORY_ALLOCATION_FLAG_READ   | 
        HOST_MEMORY_ALLOCATION_FLAG_WRITE
} HOST_MEMORY_ALLOCATION_FLAGS;

#ifndef __cplusplus
extern "C" {
#endif

/* @summary Mix the bits in a 32-bit value.
 * @param input The input value.
 * @return The input value with its bits mixed.
 */
PIL_API(uint32_t)
BitsMix32
(
    uint32_t input
);

/* @summary Mix the bits in a 64-bit value.
 * @param input The input value.
 * @return The input value with its bits mixed.
 */
PIL_API(uint64_t)
BitsMix64
(
    uint64_t input
);

/* @summary Compute a 32-bit non-cryptographic hash of some data.
 * @param data The data to hash.
 * @param length The number of bytes of data to hash.
 * @param seed An initial value used to seed the hash.
 * @return A 32-bit unsigned integer computed from the data.
 */
PIL_API(uint32_t)
HashData32
(
    void const *data, 
    size_t    length, 
    uint32_t    seed
);

/* @summary Compute a 64-bit non-cryptographic hash of some data.
 * @param data The data to hash.
 * @param length The number of bytes of data to hash.
 * @param seed An initial value used to seed the hash.
 * @return A 64-bit unsigned integer computed from the data.
 */
PIL_API(uint64_t)
HashData64
(
    void const *data, 
    size_t    length, 
    uint64_t    seed 
);

/* @summary Allocate memory from the system heap.
 * @param o_block Pointer to a MEMORY_BLOCK to populate with information about the allocation.
 * @param n_bytes The minimum number of bytes to allocate.
 * @param alignment The required alignment, in bytes. The PIL_AlignOf(T) macro can be used to obtain the necessary alignment for a given type. 
 * @return A pointer to the start of the aligned memory block, or NULL if the allocation request could not be satisfied.
 */
PIL_API(void*)
HostMemoryAllocateHeap
(
    struct MEMORY_BLOCK *o_block, 
    size_t               n_bytes, 
    size_t             alignment
);

/* @summary Free a memory block returned from the system heap.
 * @param host_addr An address returned by HostMemoryAllocateHeap.
 */
PIL_API(void)
HostMemoryFreeHeap
(
    void *host_addr
);

/* @summary Allocate address space from the host virtual memory manager.
 * The memory block is aligned to at least the operating system page size.
 * @param o_block Pointer to a MEMORY_BLOCK to populate with information about the allocation.
 * @param reserve_bytes The number of bytes of process address space to reserve.
 * @param commit_bytes The number of bytes of process address space to commit. This value can be zero.
 * @param alloc_flags One or more bitwise OR'd values from the HOST_MEMORY_ALLOCATION_FLAGS enumeration.
 * @return A pointer to the start of the reserved address space, or NULL if the allocation could not be satisfied.
 */
PIL_API(void*)
HostMemoryReserveAndCommit
(
    struct MEMORY_BLOCK *o_block, 
    size_t         reserve_bytes, 
    size_t          commit_bytes, 
    uint32_t         alloc_flags
);

/* @summary Increase the number of bytes committed in a memory block allocated from the host virtual memory manager.
 * The commitment is rounded up to the next even multiple of the operating system page size.
 * @param o_block Pointer to a MEMORY_BLOCK describing the memory block attributes after the commitment increase.
 * @param block Pointer to a MEMORY_BLOCK describing the memory block attributes prior to the commitment increase.
 * @param commit_bytes The total amount of address space within the memory block that should be committed, in bytes.
 * @return true if at least commit_bytes are committed within the memory block, or false if the commitment could not be met.
 */
PIL_API(bool)
HostMemoryIncreaseCommitment
(
    struct MEMORY_BLOCK *o_block, 
    struct MEMORY_BLOCK   *block, 
    size_t          commit_bytes
);

/* @summary Flush the host CPU instruction cache after writing dynamically-generated code to a memory block.
 * @param block Pointer to a MEMORY_BLOCK describing the memory block containing the dynamically-generated code.
 */
PIL_API(void)
HostMemoryFlush
(
    struct MEMORY_BLOCK const *block
);

/* @summary Decommit and release a block of memory returned by the system virtual memory manager.
 * @param host_addr An address returned by HostMemoryReserveAndCommit.
 */
PIL_API(void)
HostMemoryRelease
(
    void *host_addr
);

/* @summary Check a MEMORY_BLOCK to determine whether it represents a valid allocation (as opposed to a failed allocation).
 * @param block The MEMORY_BLOCK to inspect.
 * @return false if block describes a failed allocation.
 */
PIL_API(bool)
MemoryBlockIsValid
(
    struct MEMORY_BLOCK const *block
);

/* @summary Compare two MEMORY_BLOCK instances to determine if a reallocation moved the memory block.
 * @param old_block A description of the memory block prior to the reallocation.
 * @param new_block A description of the memory block after the reallocation.
 * @return true if the memory block moved.
 */
PIL_API(bool)
MemoryBlockDidMove
(
    struct MEMORY_BLOCK const *old_block, 
    struct MEMORY_BLOCK const *new_block
);

/* @summary Create a memory arena using the specified configuration.
 * The memory arena can sub-allocate from either an internal or an external block of memory.
 * @param o_arena The MEMORY_ARENA to initialize.
 * @param init Data used to configure the behavior of the memory arena.
 * @return Zero if the memory arena is successfully created, or -1 if an error occurred.
 */
PIL_API(int)
MemoryArenaCreate
(
    struct MEMORY_ARENA         *o_arena, 
    struct MEMORY_ARENA_INIT const *init
);

/* @summary Free resources associated with a memory arena.
 * For a memory arena managing an internal memory block, this frees the memory block.
 * @param arena The memory arena to delete.
 */
PIL_API(void)
MemoryArenaDelete
(
    struct MEMORY_ARENA *arena
);

/* @summary Allocate memory from an arena.
 * @param o_block Pointer to a MEMORY_BLOCK to populate with information about the allocation.
 * @param arena The memory arena from which the memory will be allocated.
 * @param size The minimum number of bytes to allocate.
 * @param alignment The required alignment of the returned address, in bytes.
 * @return Zero if the allocation is successful and o_block is populated with information about the allocation, or -1 if the allocation request could not be satisfied.
 */
PIL_API(int)
MemoryArenaAllocate
(
    struct MEMORY_BLOCK *o_block, 
    struct MEMORY_ARENA   *arena, 
    size_t                  size, 
    size_t             alignment
);

/* @summary Allocate host memory from an arena.
 * The arena must have type MEMORY_ALLOCATOR_TYPE_HOST_HEAP or MEMORY_ALLOCATOR_TYPE_HOST_VMM.
 * @param o_block Pointer to an optional MEMORY_BLOCK to populate with information about the allocation.
 * @param arena The memory arena from which the memory will be allocated.
 * @param size The minimum number of bytes to allocate.
 * @param alignment The required alignment of the returned address, in bytes.
 * @return A pointer to the start of the memory block, or NULL if the allocation request could not be satisfied.
 */
PIL_API(void*)
MemoryArenaAllocateHost
(
    struct MEMORY_BLOCK *o_block, 
    struct MEMORY_ARENA   *arena, 
    size_t                  size, 
    size_t             alignment
);

/* @summary Retrieve a marker representing the state of the memory arena at the time of the call.
 * @param arena The MEMORY_ARENA whose state will be returned.
 * @return An object representing the state of the arena at the time of the call, which can be used to invalidate all allocations made after the call.
 */
PIL_API(struct MEMORY_ARENA_MARKER)
MemoryArenaMark
(
    struct MEMORY_ARENA *arena
);

/* @summary Convert a memory arena marker into a valid address within the memory block managed by the arena.
 * The arena must have type MEMORY_ALLOCATOR_TYPE_HOST_HEAP or MEMORY_ALLOCATOR_TYPE_HOST_VMM.
 * @param marker The MEMORY_ARENA_MARKER to query.
 * @return The address in host memory corresponding to the address, or NULL if the marker or arena is invalid.
 */
PIL_API(uint8_t*)
MemoryArenaMarkerToHostAddress
(
    struct MEMORY_ARENA_MARKER marker
);

/* @summary Calculate the difference, in bytes, between two memory arena markers.
 * The markers must have been obtained from the same arena and the arena must have type MEMORY_ALLOCATOR_TYPE_HOST_HEAP or MEMORY_ALLOCATOR_TYPE_HOST_VMM.
 * @param marker1 A memory arena marker.
 * @param marker2 A memory arena marker.
 * @return The number of bytes between the two markers.
 */
PIL_API(ptrdiff_t)
MemoryArenaMarkerDifference
(
    struct MEMORY_ARENA_MARKER marker1, 
    struct MEMORY_ARENA_MARKER marker2
);

/* @summary Reset the state of the memory arena, invalidating all existing allocations.
 * @param arena The MEMORY_ARENA to reset.
 */
PIL_API(void)
MemoryArenaReset
(
    struct MEMORY_ARENA *arena
);

/* @summary Reset the state of the memory arena back to a previously obtained marker.
 * This invalidates all allocations from the arena made since the marker was obtained.
 * @param arena The MEMORY_ARENA to reset.
 * @param marker THe marker representing the reset point.
 */
PIL_API(void)
MemoryArenaResetToMarker
(
    struct MEMORY_ARENA        *arena, 
    struct MEMORY_ARENA_MARKER marker 
);

#ifndef __cplusplus
}; /* extern "C" */
#endif

#endif /* __PIL_MEMMGR_H__ */

