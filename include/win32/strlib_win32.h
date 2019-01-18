/**
 * @summary strlib_win32.h: Define platform-specific implementation details 
 * relating to strings for Microsoft Windows platforms.
 */
#ifndef __PIL_STRLIB_WIN32_H__
#define __PIL_STRLIB_WIN32_H__

#ifndef PIL_NO_INCLUDES
#   include <Windows.h>
#   include <strsafe.h>
#endif

/* @summary Define the native character type for the host OS.
 */
#ifndef STRING_CHAR_TYPE_NATIVE
#   define STRING_CHAR_TYPE_NATIVE       STRING_CHAR_TYPE_UTF16
#endif

/* @summary Define the capacity of a STRING_HASH_CHUNK.
 */
#ifndef STRING_HASH_CHUNK_CAPACITY
#   define STRING_HASH_CHUNK_CAPACITY    30
#endif

/* @summary Define the data associated with a single string interned in a string table.
 */
typedef struct STRING_DATA_ENTRY {
    STRING_INFO                StringInfo;                                     /* Size and character type information about the string. */
    uint32_t                   ByteOffset;                                     /* The byte offset of the start of the string data within the data chunk. */
} STRING_DATA_ENTRY;

/* @summary Define the data associated with a chunk in the hash list.
 * The hash table used to speed lookups within a string table consists of a fixed-length array of buckets.
 * Each bucket is simply a pointer to the head of a singly-linked list of STRING_HASH_CHUNK instances. 
 * Each bucket is anticipated to have either zero, one or two hash chunks, though the actual number per-bucket is not limited.
 * The data entries themselves are stored in a separate contiguous array maintained by the string table.
 * The Lookup(hash) consists of:
 * 1. Map hash to a bucket index.
 * 2. for each STRING_HASH_CHUNK in the bucket, perform a linear search of EntryHash.
 *    if a match is found at position i, check the STRING_TABLE::EntryList[bucket->EntryIndex[i]].
 *    if no match is found, check the next STRING_HASH_CHUNK in the bucket.
 */
typedef struct STRING_HASH_CHUNK {                                             /* 256 bytes */
#   define CAP STRING_HASH_CHUNK_CAPACITY
    struct STRING_HASH_CHUNK  *NextChunk;                                      /* The next chunk in the list. */
    uint32_t                   ItemCount;                                      /* The number of items stored in the chunk. */
    uint32_t                   Reserved;                                       /* Reserved for future use. Set to zero. */
    uint32_t                   EntryHash [CAP];                                /* An array of 32-bit hash values for each item in the chunk. */
    uint32_t                   EntryIndex[CAP];                                /* An array of 32-bit index values into the string table entry list. */
#   undef  CAP
} STRING_HASH_CHUNK;

/* @summary Define the data associated with a string table.
 * A string table stores a single unique copy of a string. Strings are always stored nul-terminated.
 */
typedef struct STRING_TABLE {
    PFN_StringHash32           HashString;                                     /* The callback function used to hash a string. */
    struct STRING_DATA_ENTRY  *StringList;                                     /* An array of StringCapacity string descriptors. */
    struct STRING_HASH_CHUNK **HashBuckets;                                    /* An array of BucketCount pointers to hash chunks storing the content of each hash bucket. */
    uint8_t                   *StringDataBase;                                 /* The base address of the memory block used to store string data. DataCommitSize bytes are valid. */
    uint32_t                   StringDataNext;                                 /* The offset of the next byte to return from the string data block. */
    uint32_t                   StringCount;                                    /* The number of strings interned in the table (also the number of valid entries in the StringList array). */
    uint32_t                   DataCommitSize;                                 /* The number of bytes committed for storing string data. */
    uint32_t                   DataReserveSize;                                /* The number of bytes reserved for storing string data. */
    uint32_t                   HashBucketCount;                                /* The number of hash buckets. This defines the dimension of the HashBuckets array. */
    uint32_t                   StringCommitCount;                              /* The number of committed STRING_DATA_ENTRY items in the StringList array. */
    uint32_t                   StringReserveCount;                             /* The maximum number of STRING_DATA_ENTRY items in the StringList array. */
    uint32_t                   HashCommitCount;                                /* The number of STRING_HASH_CHUNK items committed. */
    uint32_t                   HashReserveCount;                               /* The number of STRING_HASH_CHUNK items */
    uint8_t                   *HashDataBase;                                   /* The base address of the memory block used to store hash bucket data. */
    struct STRING_HASH_CHUNK  *HashFreeList;                                   /* A pointer to the head of the hash chunk free list, or NULL if the free list is empty. */
} STRING_TABLE;

#endif /* __PIL_STRLIB_WIN32_H__ */

