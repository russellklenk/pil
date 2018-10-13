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
#ifndef PIL_MemoryArenaAllocateHostType
#define PIL_MemoryArenaAllocateHostType(_arena, _type)                         \
    ((_type*) PIL_MemoryArenaAllocateHostNoBlock((_arena), sizeof(_type), PIL_ALIGN_OF(_type)))
#endif

/* @summary Allocate memory with the correct size and alignment for an array of instance of a given type from a memory arena.
 * @param _arena The PIL_MEMORY_ARENA from which the allocation is being made.
 * @param _type A typename, such as int, specifying the type being allocated.
 * @param _count The number of elements in the array.
 * @return A pointer to the start of the allocated memory block, or NULL.
 */
#ifndef PIL_MemoryArenaAllocateHostArray
#define PIL_MemoryArenaAllocateHostArray(_arena, _type, _count)                \
    ((_type*) PIL_MemoryArenaAllocateHostNoBlock((_arena), sizeof(_type) * (_count), PIL_ALIGN_OF(_type)))
#endif

/* @summary Allocate memory with the given object size and alignment for an array of object data from a memory arena.
 * @param _arena The PIL_MEMORY_ARENA from which the allocation is being made.
 * @param _objsize The object size, in bytes.
 * @param _count The number of elements in the array.
 * @param _align The object alignment, in bytes.
 * @return A pointer to the start of the allocated memory block, or NULL.
 */
#ifndef PIL_MemoryArenaAllocateHostArrayRaw
#define PIL_MemoryArenaAllocateHostArrayRaw(_arena, _objsize, _align, _count)  \
    ((uint8_t*) PIL_MemoryArenaAllocateHostNoBlock((_arena), (_objsize) * (_count), (_align)))
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
    uint64_t                SizeInBytes;                                       /* The size of the memory block, in bytes. */
    uint64_t                BlockOffset;                                       /* The allocation offset. This field is set for both host and device memory allocations. */
    uint8_t                *HostAddress;                                       /* The host-visible memory address. This field is set to NULL for device memory allocations. */
    uint32_t                AllocatorType;                                     /* One of the values of the MEMORY_ALLOCATOR_TYPE enumeration specifying whether the memory block represents a host or device memory allocation. */
    uint32_t                AllocationTag;                                     /* The allocation tag associated with the memory allocator that returned the memory block. */
} MEMORY_BLOCK;

/* @summary Define the data associated with an arena-style memory allocator.
 * The allocator is double-ended, and allocations can be made from either the permanent end or the temporary end.
 */
typedef struct MEMORY_ARENA {
    char const             *AllocatorName;                                     /* A nul-terminated string specifying the name of the allocator. Used for debugging. */
    uint32_t                AllocatorType;                                     /* One of the values of the MEMORY_ALLOCATOR_TYPE enumeration specifying whether the memory allocator allocates host or device memory. */
    uint32_t                AllocatorTag;                                      /* An opaque 32-bit value used to tag allocations from the arena. */
    uint64_t                MemoryStart;                                       /* The address or offset of the start of the memory block from which sub-allocations are returned. */
    uint64_t                NextOffsetPerm;                                    /* The byte offset of the next permanent allocation to return. */
    uint64_t                NextOffsetTemp;                                    /* The byte offset of the start of the most recent temporary allocation. */
    uint64_t                MaximumOffset;                                     /* The maximum value of NextOffsetPerm/NextOffsetTemp. */
    uint64_t                NbReserved;                                        /* The number of bytes of reserved address space. */
    uint64_t                NbCommitted;                                       /* The number of bytes of committed address space. */
} MEMORY_ARENA;

/* @summary Define the data used to configure an arena-style memory allocator.
 */
typedef struct MEMORY_ARENA_INIT {
    char const             *AllocatorName;                                     /* A nul-terminated string specifying the name of the allocator. Used for debugging. */
    uint64_t                MemorySize;                                        /* The size of the memory block, in bytes. */
    ADDRESS_OR_OFFSET       MemoryStart;                                       /* The offset or host address of the start of the allocated memory block. */
    uint32_t                AllocatorType;                                     /* One of the values of the MEMORY_ALLOCATOR_TYPE enumeration specifying whether the memory allocator allocates host or device memory. */
    uint32_t                AllocatorTag;                                      /* An opaque 32-bit value used to tag allocations from the arena. */
} MEMORY_ARENA_INIT;

/* @summary Define the data associated with an arena marker, which represents the state of an arena allocator at a specific point in time.
 * The arena can be reset back to a previously defined marker, invalidating all allocations made since the marked point in time.
 * Markers can only be used to mark the state of permanent allocations.
 */
typedef struct MEMORY_ARENA_MARKER {
    struct MEMORY_ARENA    *Arena;
    uint64_t                Offset;
} MEMORY_ARENA_MARKER;

/* @summary Define the allowed values for memory allocator type. 
 * An allocator can manage either host or device memory. Device memory may not be visible to the host CPU.
 */
typedef enum MEMORY_ALLOCATOR_TYPE {
    MEMORY_ALLOCATOR_TYPE_INVALID           =  0UL,                            /* This value is invalid and should not be used. */
    MEMORY_ALLOCATOR_TYPE_HOST              =  1UL,                            /* The allocator is a host memory allocator. */
    MEMORY_ALLOCATOR_TYPE_DEVICE            =  2UL,                            /* The allocator is a device memory allocator. */
} MEMORY_ALLOCATOR_TYPE;

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

#ifndef __cplusplus
}; /* extern "C" */
#endif

#endif /* __PIL_MEMMGR_H__ */

