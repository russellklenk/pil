/**
 * @summary Implement the platform-independent portions of the memory management
 * APIs, along with routines for hashing memory blocks. 
 */
#include <string.h>

#include "pil.h"
#include "memmgr.h"

#ifdef _MSC_VER
#   include <intrin.h>
#   define XXH_Rotl32(_x, _r)    _rotl((_x),(_r))
#   define XXH_Rotl64(_x, _r)    _rotl64((_x),(_r))
#else
#   define XXH_Rotl32(_x, _r)    (((_x) << (_r)) | ((_x) >> (32 - (_r))))
#   define XXH_Rotl64(_x, _r)    (((_x) << (_r)) | ((_x) >> (64 - (_r))))
#endif

/* @summary Portably read a 32-bit value from a memory location which may or may not be properly aligned.
 * @param mem The memory location to read.
 * @return A 32-bit unsigned integer representing the value at address mem.
 */
static PIL_INLINE uint32_t
XXH_ReadU32
(
    void const *mem
)
{
    uint32_t val;
    memcpy(&val, mem, sizeof(val));
    return val;
}

/* @summary Portably read a 64-bit value from a memory location which may or may not be properly aligned.
 * @param mem The memory location to read.
 * @return A 64-bit unsigned integer representing the value at address mem.
 */
static PIL_INLINE uint64_t
XXH_ReadU64
(
    void const *mem
)
{
    uint64_t val;
    memcpy(&val, mem, sizeof(val));
    return val;
}

/* @summary XXH32_Round from xxHash.
 * @param acc The 32-bit accumulator.
 * @param val The input value.
 * @return The updated accumulator.
 */
static PIL_INLINE uint32_t
XXH32_Round
(
    uint32_t acc, 
    uint32_t val
)
{
    acc += val * 2246822519U;
    acc  = XXH_Rotl32(acc, 13);
    acc *= 2654435761U;
    return acc;
}

/* @summary XXH64_round from xxHash.
 * @param acc The 64-bit accumulator.
 * @param val The input value.
 * @return The updated accumulator.
 */
static PIL_INLINE uint64_t
XXH64_Round
(
    uint64_t acc, 
    uint64_t val
)
{
    acc += val * 14029467366897019727ULL;
    acc  = XXH_Rotl64(acc, 31);
    acc *= 11400714785074694791ULL;
    return acc;
}

/* @summary XXH64_mergeRound from xxHash.
 * @param acc The 64-bit accumulator.
 * @param val The input value.
 * @return The updated accumulator.
 */
static PIL_INLINE uint64_t
XXH64_Merge
(
    uint64_t acc, 
    uint64_t val
)
{
    val  = XXH64_Round(0, val);
    acc ^= val;
    acc  = acc * 11400714785074694791ULL + 9650029242287828579ULL;
    return acc;
}

PIL_API(uint32_t)
BitsMix32
(
    uint32_t input
)
{   /* the finalizer from 32-bit MurmurHash3 */
    input ^= input >> 16;
    input *= 0x85EBCA6BU;
    input ^= input >> 13;
    input *= 0xC2B2AE35U;
    input ^= input >> 16;
    return input;
}

PIL_API(uint64_t)
BitsMix64
(
    uint64_t input
)
{   /* the finalizer from x64 MurmurHash3 */
    input ^= input >> 33;
    input *= 0xFF51AFD7ED558CCDULL;
    input ^= input >> 33;
    input *= 0xC4CEB9FE1A85EC53ULL;
    input ^= input >> 33;
    return input;
}

PIL_API(uint32_t)
HashData32
(
    void const *data, 
    size_t    length, 
    uint32_t    seed
)
{   /* xxHash XXH32 */
    uint8_t  const *p_itr = (uint8_t const*) data;
    uint8_t  const *p_end = (uint8_t const*) data + length;
    uint32_t const     c1 =  2654435761U;
    uint32_t const     c2 =  2246822519U;
    uint32_t const     c3 =  3266489917U;
    uint32_t const     c4 =   668265263U;
    uint32_t const     c5 =   374761393U;
    uint32_t          h32;   /* output */

    if (data == NULL) {
        length = 0;
        p_itr  = p_end = (uint8_t const*) (size_t) 16;
    }
    if (length > 16) {
        uint8_t const * const p_limit = p_end - 16;
        uint32_t                   v1 = seed  + c1 + c2;
        uint32_t                   v2 = seed  + c2;
        uint32_t                   v3 = seed  + 0;
        uint32_t                   v4 = seed  - c1;
        do {
            v1 = XXH32_Round(v1, XXH_ReadU32(p_itr)); p_itr += 4;
            v2 = XXH32_Round(v2, XXH_ReadU32(p_itr)); p_itr += 4;
            v3 = XXH32_Round(v3, XXH_ReadU32(p_itr)); p_itr += 4;
            v4 = XXH32_Round(v4, XXH_ReadU32(p_itr)); p_itr += 4;
        } while (p_itr <= p_limit);
        h32 = XXH_Rotl32(v1, 1) + XXH_Rotl32(v2, 7) + XXH_Rotl32(v3, 12) + XXH_Rotl32(v4, 18);
    } else {
        h32 = seed + c5;
    }

    h32 += (uint32_t) length;

    while (p_itr + 4 <= p_end) {
        h32   += XXH_ReadU32(p_itr)  * c3;
        h32    = XXH_Rotl32(h32, 17) * c4;
        p_itr += 4;
    }
    while (p_itr < p_end) {
        h32   += (*p_itr) * c5;
        h32    = XXH_Rotl32(h32, 11) * c1;
        p_itr++;
    }

    h32 ^= h32 >> 15;
    h32 *= c2;
    h32 ^= h32 >> 13;
    h32 *= c3;
    h32 ^= h32 >> 16;
    return h32;
}

PIL_API(uint64_t)
HashData64
(
    void const *data, 
    size_t    length, 
    uint64_t    seed 
)
{   /* xxHash XXH64 */
    uint8_t  const *p_itr = (uint8_t const*) data;
    uint8_t  const *p_end = (uint8_t const*) data + length;
    uint64_t const     c1 =  11400714785074694791ULL;
    uint64_t const     c2 =  14029467366897019727ULL;
    uint64_t const     c3 =   1609587929392839161ULL;
    uint64_t const     c4 =   9650029242287828579ULL;
    uint64_t const     c5 =   2870177450012600261ULL;
    uint64_t          h64;   /* output */

    if (data == NULL) {
        length = 0;
        p_itr  = p_end = (uint8_t const*) (size_t) 32;
    }
    if (length > 32) {
        uint8_t const * const p_limit = p_end - 32;
        uint64_t                   v1 = seed  + c1 + c2;
        uint64_t                   v2 = seed  + c2;
        uint64_t                   v3 = seed  + 0;
        uint64_t                   v4 = seed  - c1;
        do {
            v1 = XXH64_Round(v1, XXH_ReadU64(p_itr)); p_itr += 8;
            v2 = XXH64_Round(v2, XXH_ReadU64(p_itr)); p_itr += 8;
            v3 = XXH64_Round(v3, XXH_ReadU64(p_itr)); p_itr += 8;
            v4 = XXH64_Round(v4, XXH_ReadU64(p_itr)); p_itr += 8;
        } while (p_itr <= p_limit);

        h64 = XXH_Rotl64(v1, 1) + XXH_Rotl64(v2, 7) + XXH_Rotl64(v3, 12) + XXH_Rotl64(v4, 18);
        h64 = XXH64_Merge(h64, v1);
        h64 = XXH64_Merge(h64, v2);
        h64 = XXH64_Merge(h64, v3);
        h64 = XXH64_Merge(h64, v4);
    } else {
        h64 = seed + c5;
    }

    h64 += (uint64_t) length;

    while (p_itr + 8 <= p_end) {
        uint64_t const k1 = XXH64_Round(0, XXH_ReadU64(p_itr));
        h64   ^= k1;
        h64    = XXH_Rotl64(h64, 27) * c1 + c4;
        p_itr += 8;
    }
    if (p_itr + 4 <= p_end) {
        h64   ^= (uint64_t)(XXH_ReadU32(p_itr)) * c1;
        h64    = XXH_Rotl64(h64, 23) * c2 + c3;
        p_itr += 4;
    }
    while (p_itr < p_end) {
        h64 ^= (*p_itr) * c5;
        h64  = XXH_Rotl64(h64, 11) * c1;
        p_itr++;
    }

    h64 ^= h64 >> 33;
    h64 *= c2;
    h64 ^= h64 >> 29;
    h64 *= c3;
    h64 ^= h64 >> 32;
    return h64;
}

PIL_API(bool)
MemoryBlockIsValid
(
    struct MEMORY_BLOCK const *block
)
{
    return (block->BytesCommitted > 0 || block->BytesReserved > 0);
}

PIL_API(bool)
MemoryBlockDidMove
(
    struct MEMORY_BLOCK const *old_block, 
    struct MEMORY_BLOCK const *new_block
)
{
    return (old_block->HostAddress != new_block->HostAddress);
}

PIL_API(int)
MemoryArenaCreate
(
    struct MEMORY_ARENA         *o_arena, 
    struct MEMORY_ARENA_INIT const *init
)
{
    uint64_t       start = init->MemoryStart.BaseOffset;
    uint64_t   nb_commit = init->CommittedSize;
    uint64_t  nb_reserve = init->ReserveSize;
    uint32_t alloc_flags = init->AllocationFlags;

    if (init == NULL || o_arena == NULL) {
        assert(init != NULL);
        assert(o_arena != NULL);
        return -1;
    }
    if (init->AllocatorType == MEMORY_ALLOCATOR_TYPE_INVALID) {
        assert(init->AllocatorType != MEMORY_ALLOCATOR_TYPE_INVALID);
        return -1;
    }
    if (init->ArenaFlags == MEMORY_ARENA_FLAGS_NONE) {
        assert(init->ArenaFlags != MEMORY_ARENA_FLAGS_NONE);
        return -1;
    }
    if (init->ArenaFlags & MEMORY_ARENA_FLAG_EXTERNAL) {
        if (init->ArenaFlags & MEMORY_ARENA_FLAG_INTERNAL) {
            assert(0 && "ArenaFlags cannot specify both INTERNAL and EXTERNAL");
            return -1;
        }
        if (init->AllocatorType != MEMORY_ALLOCATOR_TYPE_DEVICE) {
            if (init->MemoryStart.HostAddress == NULL) {
                assert(init->MemoryStart.HostAddress != NULL);
                return -1;
            }
        }
    }
    if (init->ArenaFlags & MEMORY_ARENA_FLAG_INTERNAL) {
        if (init->AllocatorType == MEMORY_ALLOCATOR_TYPE_DEVICE) {
            assert(0 && "MEMORY_ALLOCATOR_TYPE_DEVICE cannot specify MEMORY_ARENA_FLAG_INTERNAL");
            return -1;
        }
    }
    if (init->ReserveSize == 0 || init->CommittedSize == 0) {
        assert(init->ReserveSize > 0);
        assert(init->CommittedSize > 0);
        return -1;
    }
    if (init->ReserveSize > init->CommittedSize) {
        assert(init->CommittedSize <= init->ReserveSize);
        return -1;
    }
    if (init->ArenaFlags & MEMORY_ARENA_FLAG_INTERNAL) {
        MEMORY_BLOCK block;
        void    *host_addr;

        if (init->AllocatorType == MEMORY_ALLOCATOR_TYPE_HOST_HEAP) {
            if ((host_addr = HostMemoryAllocateHeap(&block, nb_commit, 16)) == NULL) {
                return -1;
            }  alloc_flags = HOST_MEMORY_ALLOCATION_FLAGS_READWRITE;
        } else if (init->AllocatorType == MEMORY_ALLOCATOR_TYPE_HOST_VMM) {
            if ((host_addr = HostMemoryReserveAndCommit(&block, nb_reserve, nb_commit, init->AllocationFlags)) == NULL) {
                return -1;
            }  alloc_flags = init->AllocationFlags;
        } else {
            assert(init->AllocatorType == MEMORY_ALLOCATOR_TYPE_HOST_HEAP || init->AllocatorType == MEMORY_ALLOCATOR_TYPE_HOST_VMM);
            return -1;
        }
        start      =(uint64_t) block.HostAddress;
        nb_commit  = block.BytesCommitted;
        nb_reserve = block.BytesReserved;
    }
    o_arena->AllocatorName  = init->AllocatorName;
    o_arena->MemoryStart    = start;
    o_arena->NextOffset     = 0;
    o_arena->MaximumOffset  = nb_commit;
    o_arena->NbReserved     = nb_reserve;
    o_arena->NbCommitted    = nb_commit;
    o_arena->AllocatorType  = init->AllocatorType;
    o_arena->AllocatorTag   = init->AllocatorTag;
    o_arena->AllocationFlags= alloc_flags;
    o_arena->ArenaFlags     = init->ArenaFlags;
    return 0;
}

PIL_API(void)
MemoryArenaDelete
(
    struct MEMORY_ARENA *arena
)
{
    if (arena) {
        if (arena->ArenaFlags & MEMORY_ARENA_FLAG_INTERNAL) {
            if (arena->AllocatorType == MEMORY_ALLOCATOR_TYPE_HOST_HEAP) {
                HostMemoryFreeHeap((void*) arena->MemoryStart);
                arena->MemoryStart = NULL;
            } else if (arena->AllocatorType == MEMORY_ALLOCATOR_TYPE_HOST_VMM) {
                HostMemoryRelease((void*) arena->MemoryStart);
                arena->MemoryStart = NULL;
            } else {
                assert(0 && "Unknown allocator type for INTERNAL arena (memory leak)");
                return;
            }
        }
        arena->MaximumOffset = 0;
        arena->NbReserved    = 0;
        arena->NbCommitted   = 0;
    }
}

PIL_API(int)
MemoryArenaAllocate
(
    struct MEMORY_BLOCK *o_block, 
    struct MEMORY_ARENA   *arena, 
    size_t                  size, 
    size_t             alignment
)
{
    uint64_t    base_address = arena->MemoryStart + arena->NextOffset;
    uint64_t aligned_address = base_address != 0 ? PIL_AlignUp(base_address, alignment) : 0;
    uint64_t     align_bytes = aligned_address - base_address;
    uint64_t     alloc_bytes = size + align_bytes;
    uint64_t      new_offset = arena->NextOffset + alloc_bytes;

    assert(o_block != NULL);

    if (new_offset >= arena->MaximumOffset) {
        /* internal VMM arenas can increase commit */
        if (arena->NbCommitted  != arena->NbReserved) {
            uint64_t def_amount  = 128 * 1024; /* grow by 128KB at a time */
            uint64_t min_amount  = new_offset - arena->NbCommitted;
            uint64_t max_amount  = arena->NbReserved - arena->NbCommitted;
            uint64_t new_amount;
            MEMORY_BLOCK  block;

            block.BytesCommitted = arena->NbCommitted;
            block.BytesReserved  = arena->NbReserved;
            block.BlockOffset    = 0;
            block.HostAddress    =(uint8_t*) arena->MemoryStart;
            block.AllocationFlags= arena->AllocationFlags;
            block.AllocatorTag    = arena->AllocatorTag;

            if (min_amount <= max_amount) {
                if (arena->NbCommitted + def_amount <= max_amount && def_amount > min_amount) {
                    new_amount = arena->NbCommitted  + def_amount; /* grow by the default amount */
                } else {
                    new_amount = arena->NbCommitted  + min_amount; /* grow by the amount we're short */
                }
                if (HostMemoryIncreaseCommitment(&block, &block, new_amount)) {
                    arena->MaximumOffset = block.BytesCommitted;
                    arena->NbCommitted   = block.BytesCommitted;
                    assert(arena->NbCommitted <= arena->NbReserved);
                } else goto allocation_failed; /* commit increase failed */
            } else goto allocation_failed; /* need more than we can commit */
        } else goto allocation_failed; /* not enough space */
    }

    o_block->BytesCommitted  = size;
    o_block->BytesReserved   = size;
    o_block->BlockOffset     = arena->NextOffset;
    o_block->HostAddress     =(uint8_t*) (uintptr_t) aligned_address;
    o_block->AllocationFlags = arena->AllocationFlags;
    o_block->AllocatorTag    = arena->AllocatorTag;
    arena->NextOffset        = new_offset;
    return  0;

allocation_failed:
    memset(o_block, 0, sizeof(MEMORY_BLOCK));
    return -1;
}

PIL_API(void*)
MemoryArenaAllocateHost
(
    struct MEMORY_BLOCK *o_block, 
    struct MEMORY_ARENA   *arena, 
    size_t                  size, 
    size_t             alignment
)
{
    MEMORY_BLOCK block;
    if (o_block == NULL) {
        o_block = &block;
    }
    if (MemoryArenaAllocate(o_block, arena, size, alignment) == 0) {
        return o_block->HostAddress;
    } else {
        return NULL;
    }
}

PIL_API(struct MEMORY_ARENA_MARKER)
MemoryArenaMark
(
    struct MEMORY_ARENA *arena
)
{
    MEMORY_ARENA_MARKER m;
    m.Arena = arena;
    m.State = arena->NextOffset;
    return  m;
}

PIL_API(uint8_t*)
MemoryArenaMarkerToHostAddress
(
    struct MEMORY_ARENA_MARKER marker
)
{
    assert(marker.Arena);
    assert(marker.Arena->MemoryStart   <= marker.State);
    assert(marker.Arena->MaximumOffset >= marker.State);
    assert(marker.Arena->AllocatorType == MEMORY_ALLOCATOR_TYPE_HOST_HEAP || marker.Arena->AllocatorType == MEMORY_ALLOCATOR_TYPE_HOST_VMM);
    return(uint8_t*) marker.Arena->MemoryStart + marker.State;
}

PIL_API(ptrdiff_t)
MemoryArenaMarkerDifference
(
    struct MEMORY_ARENA_MARKER marker1, 
    struct MEMORY_ARENA_MARKER marker2
)
{
    assert(marker1.Arena != NULL);
    assert(marker2.Arena != NULL);
    assert(marker1.Arena == marker2.Arena);
    assert(marker1.Arena->AllocatorType == MEMORY_ALLOCATOR_TYPE_HOST_HEAP || marker1.Arena->AllocatorType == MEMORY_ALLOCATOR_TYPE_HOST_VMM);
    if (marker1.State < marker2.State) {
        return marker2.State - marker1.State;
    } else {
        return marker1.State - marker2.State;
    }
}

PIL_API(void)
MemoryArenaReset
(
    struct MEMORY_ARENA *arena
)
{
    arena->NextOffset = 0;
}

PIL_API(void)
MemoryArenaResetToMarker
(
    struct MEMORY_ARENA        *arena, 
    struct MEMORY_ARENA_MARKER marker 
)
{
    assert(arena != NULL);
    assert(marker.Arena == arena);
    assert(marker.Arena->NextOffset >= marker.State);
    arena->NextOffset = marker.State;
}

