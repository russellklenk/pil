/**
 * @summary strlib_win32.cc: Implement the string library entry points specific 
 * to the Microsoft Windows platform.
 */
#include "memmgr.h"
#include "strlib.h"

/* @summary Define the data associated with the memory requirements for a string table.
 */
typedef struct STRING_TABLE_SIZE_INFO {
    uint32_t   BucketCount;                                                    /* The number of hash buckets required for the hash table. */
    uint32_t   DataCommitSize;                                                 /* The number of bytes to commit for the string data storage block. */
    uint32_t   DataReserveSize;                                                /* The number of bytes to reserve for the string data storage block. */
    uint32_t   HashCommitSize;                                                 /* The number of bytes to commit for the hash chunk storage block. */
    uint32_t   HashReserveSize;                                                /* The number of bytes to reserve for the hash chunk storage block. */
    uint32_t   TableCommitSize;                                                /* The number of bytes to commit from the main table storage block. */
    uint32_t   TableReserveSize;                                               /* The number of bytes to reserve for the main table storage block. */
    uint32_t   HashCommitCount;                                                /* The number of STRING_HASH_CHUNK records committed in the initial allocation. */
    uint32_t   HashReserveCount;                                               /* The number of STRING_HASH_CHUNK records reserved in the initial allocation. */
    uint32_t   StringCommitCount;                                              /* The number of STRING_DATA_ENTRY records committed in the initial allocation. */
    uint32_t   StringReserveCount;                                             /* The number of STRING_DATA_ENTRY records reserved in the initial allocation. */
} STRING_TABLE_SIZE_INFO;

/* @summary Calculate the next power-of-two value greater than or equal to a given value.
 * @param n The input value.
 * @return The next power-of-two value greater than or equal to n.
 */
static PIL_INLINE uint32_t
StringNextPow2GreaterOrEqual
(
    uint32_t n
)
{
    uint32_t i, k;
    --n;
    for (i = 1, k = sizeof(uint32_t) * 8; i < k; i <<= 1)
    {
        n |= n >> i;
    }
    return n+1;
}

/* @summary Given a value of the STRING_CHAR_TYPE enumeration, return the corresponding function for producing a 32-bit hash of a nul-terminated string.
 * @param char_type One of the values of the STRING_CHAR_TYPE enumeration.
 * @return The corresponding hash function.
 */
static PIL_INLINE PFN_StringHash32
StringHashForCharType
(
    uint32_t char_type
)
{
    switch (char_type) {
        case STRING_CHAR_TYPE_UNKNOWN: return StringHash32_Utf8;
        case STRING_CHAR_TYPE_UTF8   : return StringHash32_Utf8;
        case STRING_CHAR_TYPE_UTF16  : return StringHash32_Utf16;
        case STRING_CHAR_TYPE_UTF32  : return StringHash32_Utf32;
        default                      : break;
    } return StringHash32_Utf8;
}

/* @summary Determine the memory allocation attributes for a given string table configuration.
 * @param size On return, this structure stores the memory allocation attributes for the string table.
 * @param init Configuration data specifying the attributes of the string table.
 */
static void
StringTableQueryMemorySize
(
    struct STRING_TABLE_SIZE_INFO *size, 
    struct STRING_TABLE_INIT      *init
)
{
    uint32_t   num_buckets_base =(init->MaxStringCount + (STRING_HASH_CHUNK_CAPACITY-1)) / STRING_HASH_CHUNK_CAPACITY;
    uint32_t   num_buckets_pow2 = StringNextPow2GreaterOrEqual(num_buckets_base);
    uint32_t   data_size_commit = 0;
    uint32_t  data_size_reserve = 0;
    uint32_t   hash_size_commit = 0;
    uint32_t  hash_size_reserve = 0;
    uint32_t  table_size_commit = 0;
    uint32_t table_size_reserve = 0;
    uint32_t      commit_chunks = 0;
    uint32_t     reserve_chunks = 0;
    uint32_t     commit_entries = 0;
    uint32_t    reserve_entries = 0;
    uint32_t       entry_offset = 0;
    SYSTEM_INFO         sysinfo;

    /* retrieve operating system page size and allocation granularity */
    GetNativeSystemInfo(&sysinfo);

    /* calculate sizes and offsets for the main table storage block */
    table_size_commit       += PIL_AllocationSizeType (STRING_TABLE);
    table_size_commit       += PIL_AllocationSizeArray(STRING_HASH_CHUNK*, num_buckets_pow2);
    entry_offset             = table_size_commit;
    table_size_reserve       = table_size_commit;
    table_size_commit       += PIL_AllocationSizeArray(STRING_INFO, init->InitialCapacity);
    table_size_reserve      += PIL_AllocationSizeArray(STRING_INFO, init->MaxStringCount);
    table_size_commit        = PIL_AlignUp(table_size_commit , sysinfo.dwPageSize);
    table_size_reserve       = PIL_AlignUp(table_size_reserve, sysinfo.dwPageSize);
    commit_entries           =(table_size_commit  - entry_offset) / sizeof(STRING_INFO);
    reserve_entries          =(table_size_reserve - entry_offset) / sizeof(STRING_INFO);
    
    /* calculate the reservation and initial commit size for the hash block */
    hash_size_commit         = PIL_AllocationSizeArray(STRING_HASH_CHUNK, num_buckets_pow2);
    hash_size_reserve        = PIL_AllocationSizeArray(STRING_HASH_CHUNK, num_buckets_pow2*2);
    hash_size_commit         = PIL_AlignUp(hash_size_commit , sysinfo.dwPageSize);
    hash_size_reserve        = PIL_AlignUp(hash_size_reserve, sysinfo.dwPageSize);
    commit_chunks            = hash_size_commit  / sizeof(STRING_HASH_CHUNK);
    reserve_chunks           = hash_size_reserve / sizeof(STRING_HASH_CHUNK);

    /* calculate the reservation and initial commit size for the data block */
    data_size_commit         = PIL_AlignUp(init->DataCommitSize, sysinfo.dwPageSize);
    data_size_reserve        = PIL_AlignUp(init->MaxDataSize   , sysinfo.dwPageSize);

    size->BucketCount        = num_buckets_pow2;
    size->DataCommitSize     = data_size_commit;
    size->DataReserveSize    = data_size_reserve;
    size->HashCommitSize     = hash_size_commit;
    size->HashReserveSize    = hash_size_reserve;
    size->TableCommitSize    = table_size_commit;
    size->TableReserveSize   = table_size_reserve;
    size->HashCommitCount    = commit_chunks;
    size->HashReserveCount   = reserve_chunks;
    size->StringCommitCount  = commit_entries;
    size->StringReserveCount = reserve_entries;
}

PIL_API(size_t)
NativeStringLengthBytes
(
    char_native_t const *str
)
{
    size_t len = 0;

    if (str == nullptr) {
        return 0;
    }
    if (SUCCEEDED(StringCbLengthW(str, STRSAFE_MAX_CCH * sizeof(char_native_t), &len))) {
        return (len + sizeof(char_native_t));
    } return 0;
}

PIL_API(size_t)
NativeStringLengthChars
(
    char_native_t const *str
)
{
    size_t len = 0;

    if (str == nullptr) {
        return 0;
    }
    if (SUCCEEDED(StringCchLengthW(str, STRSAFE_MAX_CCH, &len))) {
        return (len);
    }
    return 0;
}

PIL_API(int)
NativeStringCompareCs
(
    char_native_t const *str1, 
    char_native_t const *str2
)
{
    if (str1 == str2) {
        return 0;
    }
    /* lstrcmpW is exported by KERNEL32.DLL - uses CompareStringEx */
    return lstrcmpW(str1, str2);
}

PIL_API(int)
NativeStringCompareCi
(
    char_native_t const *str1, 
    char_native_t const *str2
)
{
    if (str1 == str2) {
        return 0;
    }
    /* lstrcmpiW is exported by KERNEL32.DLL - uses CompareStringEx */
    return lstrcmpiW(str1, str2);
}

PIL_API(int)
StringConvertUtf8ToNative
(
    char_utf8_t const *utf8_str, 
    char_native_t   *native_buf, 
    size_t           native_max, 
    size_t          *native_len
)
{
    char_native_t *output = (char_native_t*) native_buf;
    int            outcch = (int           )(native_max / sizeof(char_native_t));
    int            nchars = 0;

    if (utf8_str == nullptr) {
        if (output && native_max >= sizeof(char_native_t)) {
            *output = L'\0';
        } *native_len = sizeof(char_native_t);
        return 0;
    }
    if ((nchars = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8_str, -1, output, outcch)) == 0) {
        if (output && native_max >= sizeof(char_native_t)) {
            *output = L'\0';
        } *native_len = sizeof(char_native_t);
        return -1;
    } *native_len = (size_t)(nchars * sizeof(char_native_t));
    return 0;
}

PIL_API(int)
StringConvertNativeToUtf8
(
    char_native_t const *native_str, 
    char_utf8_t           *utf8_buf, 
    size_t                 utf8_max, 
    size_t                *utf8_len
)
{
    char_native_t const *native = (char_native_t const*) native_str;
    char_utf8_t         *output = (char_utf8_t        *) utf8_buf;
    int                   outcb = (int                 )(utf8_max / sizeof(char_utf8_t));
    int                  nbytes = 0;

    if (native_str == nullptr) {
        if (output && utf8_max >= sizeof(char_utf8_t)) {
            *output = '\0';
        } *utf8_len = sizeof(char_utf8_t);
        return 0;
    }
    if ((nbytes = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, native, -1, output, outcb, nullptr, nullptr)) == 0) {
        if (output && utf8_max >= sizeof(char_utf8_t)) {
            *output = '\0';
        } *utf8_len = sizeof(char_utf8_t);
        return -1;
    } *utf8_len = (size_t) nbytes;
    return 0;
}

/* @summary Compute the length of a UTF-8 encoded, nul-terminated string.
 * @param str The string.
 * @param cb On return, this location stores the string length, including the nul, in bytes.
 * @param cc On return, this location stores the string length, not including the nul, in characters.
 */
PIL_API(void)
StringLengthUtf8
(
    void const   *str, 
    uint32_t *n_bytes, 
    uint32_t *n_chars
)
{
    uint8_t const *s =(uint8_t const*) str;
    uint32_t      nb = 0;
    uint32_t      nc = 0;
    uint8_t       cp;

    if (str != nullptr) {
        while ((cp = *s++) != 0) {
            if((cp & 0xC0) != 0x80) {
                nc++;
            } nb++;
        }
        PIL_Assign(n_bytes, nb+1);
        PIL_Assign(n_chars, nc);
    } else {
        PIL_Assign(n_bytes, 0);
        PIL_Assign(n_chars, 0);
    }
}

/* @summary Compute the length of a UTF-16 encoded, nul-terminated string.
 * @param str The string.
 * @param cb On return, this location stores the string length, including the nul, in bytes.
 * @param cc On return, this location stores the string length, not including the nul, in characters.
 */
PIL_API(void)
StringLengthUtf16
(
    void const   *str, 
    uint32_t *n_bytes, 
    uint32_t *n_chars
)
{
    uint16_t const *s =(uint16_t const*) str;
    uint32_t       nb = 0;
    uint32_t       nc = 0;
    uint16_t       cp;

    if (str != nullptr) {
        while ((cp = *s++) != 0) {
            nb += 2; nc++;
            if (cp >= 0xD800 && cp <= 0xDBFF) { 
                cp  =*s;
                if (cp >= 0xDC00 && cp <= 0xDFFF) {
                    nb += 2;
                    s++;
                }
            }
        }
        PIL_Assign(n_bytes, nb+2);
        PIL_Assign(n_chars, nc);
    } else {
        PIL_Assign(n_bytes, 0);
        PIL_Assign(n_chars, 0);
    }
}

/* @summary Compute the length of a UTF-32 encoded, nul-terminated string.
 * @param str The string.
 * @param cb On return, this location stores the string length, including the nul, in bytes.
 * @param cc On return, this location stores the string length, not including the nul, in characters.
 */
PIL_API(void)
StringLengthUtf32
(
    void const   *str, 
    uint32_t *n_bytes, 
    uint32_t *n_chars
)
{
    uint32_t const *s =(uint32_t const*) str;
    uint32_t       nb = 0;
    uint32_t       nc = 0;
    uint32_t       cp;

    if (str != nullptr) {
        while ((cp = *s++) != 0) {
            nb += 4; nc++;
        }
        PIL_Assign(n_bytes, nb+4);
        PIL_Assign(n_chars, nc);
    } else {
        PIL_Assign(n_bytes, 0);
        PIL_Assign(n_chars, 0);
    }
}

PIL_API(uint32_t)
StringHash32_Utf8
(
    void const *str, 
    uint32_t *len_b, 
    uint32_t *len_c
)
{   /* FNV1 with MurmurHash3 finalizer */
    uint8_t const    *s =(uint8_t const*) str;
    uint32_t        h32 = 2166136261U;
    uint32_t         cc = 0;
    uint32_t         cb = 0;
    uint8_t          cp;
    while ((cp = *s++) != 0) {
        h32 =(16777619U * h32) + cp;
        cb += sizeof(uint8_t);
        if ((cp & 0xC0) != 0x80) {
            cc++;
        }
    }
    PIL_Assign(len_b, cb+sizeof(uint8_t));
    PIL_Assign(len_c, cc);
    h32 ^= h32 >> 16;
    h32 *= 0x85EBCA6BU;
    h32 ^= h32 >> 13;
    h32 *= 0xC2B2AE35U;
    h32 ^= h32 >> 16;
    return h32;
}

PIL_API(uint32_t)
StringHash32_Utf16
(
    void const *str, 
    uint32_t *len_b, 
    uint32_t *len_c
)
{   /* FNV1 with MurmurHash3 finalizer */
    uint16_t const *s =(uint16_t const*) str;
    uint32_t      h32 = 2166136261U;
    uint32_t       cc = 0;
    uint32_t       cb = 0;
    uint16_t       cp;
    while ((cp = *s++) != 0) {
        h32 =(16777619U * h32) + cp;
        cb += sizeof(uint16_t);
        cc++;
        if (cp >= 0xD800 && cp <= 0xDBFF) {
            cp  =*s;
            if (cp >= 0xDC00 && cp <= 0xDFFF) {
                cb += sizeof(uint16_t);
                s++;
            }
        }
    }
    PIL_Assign(len_b, cb+sizeof(uint16_t));
    PIL_Assign(len_c, cc);
    h32 ^= h32 >> 16;
    h32 *= 0x85EBCA6BU;
    h32 ^= h32 >> 13;
    h32 *= 0xC2B2AE35U;
    h32 ^= h32 >> 16;
    return h32;
}

PIL_API(uint32_t)
StringHash32_Utf32
(
    void const *str, 
    uint32_t *len_b, 
    uint32_t *len_c
)
{   /* FNV1 with MurmurHash3 finalizer */
    uint32_t const *s =(uint32_t const*) str;
    uint32_t      h32 = 2166136261U;
    uint32_t       cc = 0;
    uint32_t       cb = 0;
    uint32_t       cp;
    while ((cp = *s++) != 0) {
        h32 =(16777619U * h32) + cp;
        cb += sizeof(uint32_t);
        cc++;
    }
    PIL_Assign(len_b, cb+sizeof(uint32_t));
    PIL_Assign(len_c, cc);
    h32 ^= h32 >> 16;
    h32 *= 0x85EBCA6BU;
    h32 ^= h32 >> 13;
    h32 *= 0xC2B2AE35U;
    h32 ^= h32 >> 16;
    return h32;
}

PIL_API(uint32_t)
StringHash32_Range
(
    void const *strbeg, 
    void const *strend
)
{   /* FNV1 with MurmurHash3 finalizer */
    uint8_t const *s =(uint8_t const*) strbeg;
    uint8_t const *e =(uint8_t const*) strend;
    uint32_t     h32 = 2166136261U;
    while (s < e) {
        h32 =(16777619U * h32) + *s++;
    }
    h32 ^= h32 >> 16;
    h32 *= 0x85EBCA6BU;
    h32 ^= h32 >> 13;
    h32 *= 0xC2B2AE35U;
    h32 ^= h32 >> 16;
    return h32;
}

PIL_API(struct STRING_TABLE*)
StringTableCreate
(
    struct STRING_TABLE_INIT *init
)
{
    STRING_TABLE          *table = nullptr;
    STRING_INFO         *strings = nullptr;
    STRING_HASH_CHUNK    *chunks = nullptr;
    STRING_HASH_CHUNK      *head = nullptr;
    STRING_HASH_CHUNK      *next = nullptr;
    STRING_HASH_CHUNK  **buckets = nullptr;
    uint8_t          *table_addr = nullptr;
    uint8_t           *hash_addr = nullptr;
    uint8_t           *data_addr = nullptr;
    DWORD             error_code = ERROR_SUCCESS;
    uint32_t                i, n;
    MEMORY_ARENA           arena;
    MEMORY_ARENA_INIT arena_init;
    STRING_TABLE_SIZE_INFO sizes;

    if (init->DataCommitSize >= init->MaxDataSize) {
        assert(init->MaxDataSize >= init->DataCommitSize);
        error_code = ERROR_INVALID_PARAMETER;
        return nullptr;
    }
    if (init->InitialCapacity >= init->MaxStringCount) {
        assert(init->MaxStringCount >= init->InitialCapacity);
        error_code = ERROR_INVALID_PARAMETER;
        return nullptr;
    }

    /* initialize the three primary memory allocations */
    StringTableQueryMemorySize(&sizes, init);
    if ((table_addr =(uint8_t *) VirtualAlloc(nullptr, sizes.TableReserveSize, MEM_RESERVE, PAGE_NOACCESS)) == nullptr) {
        error_code  = GetLastError();
        goto cleanup_and_fail;
    }
    if ((hash_addr  =(uint8_t *) VirtualAlloc(nullptr, sizes.HashReserveSize , MEM_RESERVE, PAGE_NOACCESS)) == nullptr) {
        error_code  = GetLastError();
        goto cleanup_and_fail;
    }
    if ((data_addr  =(uint8_t *) VirtualAlloc(nullptr, sizes.DataReserveSize , MEM_RESERVE, PAGE_NOACCESS)) == nullptr) {
        error_code  = GetLastError();
        goto cleanup_and_fail;
    }

    /* commit memory for the table - committed pages are zero-initialized */
    if (sizes.TableCommitSize > 0) {
        if (VirtualAlloc(table_addr, sizes.TableCommitSize, MEM_COMMIT, PAGE_READWRITE) != table_addr) {
            error_code  = GetLastError();
            goto cleanup_and_fail;
        }
    }
    if (sizes.HashCommitSize > 0) {
        if (VirtualAlloc(hash_addr , sizes.HashCommitSize , MEM_COMMIT, PAGE_READWRITE) != hash_addr) {
            error_code  = GetLastError();
            goto cleanup_and_fail;
        }
    }
    if (sizes.DataCommitSize > 0) {
        if (VirtualAlloc(data_addr , sizes.DataCommitSize , MEM_COMMIT, PAGE_READWRITE) != data_addr) {
            error_code  = GetLastError();
            goto cleanup_and_fail;
        }
    }
    
    /* initialize a memory arena to sub-allocate from the table allocation */
    arena_init.AllocatorName           = __FUNCTION__;
    arena_init.ReserveSize             = sizes.TableReserveSize;
    arena_init.CommittedSize           = sizes.TableReserveSize;
    arena_init.MemoryStart.HostAddress = table_addr;
    arena_init.AllocatorType           = MEMORY_ALLOCATOR_TYPE_HOST_VMM;
    arena_init.AllocatorTag            = MakeAllocatorTag('S','T','V','M');
    arena_init.AllocationFlags         = HOST_MEMORY_ALLOCATION_FLAGS_READWRITE;
    arena_init.ArenaFlags              = MEMORY_ARENA_FLAG_EXTERNAL;
    if (MemoryArenaCreate(&arena, &arena_init) != 0) {
        error_code = ERROR_ARENA_TRASHED;
        goto cleanup_and_fail;
    }
    table   = MemoryArenaAllocateHostType (&arena, STRING_TABLE);
    buckets = MemoryArenaAllocateHostArray(&arena, STRING_HASH_CHUNK*, sizes.BucketCount);
    strings = MemoryArenaAllocateHostArray(&arena, STRING_INFO, sizes.StringReserveCount);
    table->StringList         = strings;
    table->HashBuckets        = buckets;
    table->StringDataBase     = data_addr;
    table->StringDataNext     = 0;
    table->StringCount        = 0;
    table->DataCommitSize     = sizes.DataCommitSize;
    table->DataReserveSize    = sizes.DataReserveSize;
    table->HashBucketCount    = sizes.BucketCount;
    table->StringCommitCount  = sizes.StringCommitCount;
    table->StringReserveCount = sizes.StringReserveCount;
    table->HashCommitCount    = sizes.HashCommitCount;
    table->HashReserveCount   = sizes.HashReserveCount;
    table->HashDataBase       = hash_addr;

    /* initialize hash chunk free list */
    for (i = 0, n = sizes.HashCommitCount, head = nullptr, chunks = (STRING_HASH_CHUNK*) hash_addr; i < n; ++i) {
        next = &chunks[n-i-1];
        next->NextChunk = head;
        next->ItemCount = 0;
        head = next;
    }
    table->HashFreeList = head;
    return table;

cleanup_and_fail:
    if (data_addr) {
        VirtualFree(data_addr , 0, MEM_RELEASE);
    }
    if (hash_addr) {
        VirtualFree(hash_addr , 0, MEM_RELEASE);
    }
    if (table_addr) {
        VirtualFree(table_addr, 0, MEM_RELEASE);
    }
    return nullptr;
}

PIL_API(void)
StringTableDelete
(
    struct STRING_TABLE *table
)
{
    if (table) {
        VirtualFree(table->StringDataBase, 0, MEM_RELEASE);
        VirtualFree(table->HashDataBase, 0, MEM_RELEASE);
        VirtualFree(table, 0, MEM_RELEASE);
    }
}

PIL_API(void)
StringTableReset
(
    struct STRING_TABLE *table
)
{
    STRING_HASH_CHUNK **buckets = table->HashBuckets;
    STRING_HASH_CHUNK    *chunk;
    uint32_t               i, n;

    table->StringDataNext = 0;
    table->StringCount    = 0;
    for (i = 0, n = table->HashBucketCount; i < n; ++i) {
        while ((chunk = buckets[i]) != nullptr) {
            buckets[i]          = chunk->NextChunk;
            chunk->NextChunk    = table->HashFreeList;
            chunk->ItemCount    = 0;
            table->HashFreeList = chunk;
        }
    }
}

PIL_API(int)
StringTableRebuild
(
    struct STRING_TABLE *table, 
    uint32_t      string_count, 
    uint32_t        data_bytes
)
{
    uint32_t const      GROW_SIZE = STRING_BUFFER_GROW_SIZE;
    uint32_t const      ALIGNMENT = STRING_DATA_ALIGNMENT;
    PFN_StringHash32    hash_func = StringHash32_Utf8;
    STRING_INFO          *ent_itr = table->StringList;
    STRING_INFO          *ent_end = table->StringList + string_count;
    uint8_t            *data_base = table->StringDataBase;
    uint32_t            char_type = STRING_CHAR_TYPE_UNKNOWN;
    STRING_HASH_CHUNK   **buckets = table->HashBuckets;
    STRING_HASH_CHUNK     *bucket = nullptr;
    uint32_t          bucket_mask = table->HashBucketCount - 1;
    uint32_t               bindex = 0;
    uint32_t              ent_idx = 0;
    uint32_t                 hash = 0;
    uint8_t                  *str;
    uint32_t               nb, nc;
    uint32_t                 i, n;

    if (data_bytes > table->DataCommitSize) {
        assert(table->DataCommitSize >= data_bytes);
        return -1;
    }
    if (string_count > table->StringCommitCount) {
        assert(table->StringCommitCount >= string_count);
        return -1;
    }

    while (ent_itr < ent_end) {
        /* validate the entry and hash the string */
        if (char_type != ent_itr->CharacterType) {
            hash_func  = StringHashForCharType(ent_itr->CharacterType);
            char_type  = ent_itr->CharacterType;
        }
        str    = data_base + ent_itr->ByteOffset;
        hash   = hash_func(str, &nb, &nc);
        bindex = hash & bucket_mask;
        bucket = buckets[bindex];
        assert(ent_itr->ByteLength == nb);
        assert(ent_itr->CharLength == nc);

        /* get the hash chunk where the item will be inserted */
        if (bucket == nullptr || bucket->ItemCount == STRING_HASH_CHUNK_CAPACITY) {
            if (table->HashFreeList == nullptr) {
                /* commit an additional block of hash chunks */
                uint32_t   commit_size = GROW_SIZE;
                uint32_t    num_commit = commit_size / sizeof(STRING_HASH_CHUNK);
                if((table->HashCommitCount + num_commit) > table->HashReserveCount) {
                    num_commit = table->HashReserveCount - table->HashCommitCount;
                    commit_size= num_commit * sizeof(STRING_HASH_CHUNK);
                }
                if (VirtualAlloc(&table->HashDataBase[table->HashCommitCount*sizeof(STRING_HASH_CHUNK)], commit_size, MEM_COMMIT, PAGE_READWRITE) == nullptr) {
                    return -1;
                }
                for (i = table->HashCommitCount, n = num_commit; i < n; ++i) {
                    STRING_HASH_CHUNK *chunk =(STRING_HASH_CHUNK*) &table->HashDataBase[i * sizeof(STRING_HASH_CHUNK)];
                    chunk->NextChunk         = table->HashFreeList;
                    chunk->ItemCount         = 0;
                    table->HashFreeList      = chunk;
                }
                table->HashCommitCount += num_commit;
            }
            /* take a chunk from the head of the free list.
             * insert it at the head of the hash bucket list. */
            bucket = table->HashFreeList;
            table->HashFreeList        = bucket->NextChunk;
            bucket->NextChunk          = table->HashBuckets[bindex];
            bucket->ItemCount          = 0;
            table->HashBuckets[bindex] = bucket;
        }

        /* append the item to the hash chunk */
        bucket->EntryHash [bucket->ItemCount] = hash;
        bucket->EntryIndex[bucket->ItemCount] = ent_idx;
        bucket->ItemCount++;
        ent_itr++; ent_idx++;
    }
    table->StringDataNext = PIL_AlignUp(data_bytes, ALIGNMENT);
    table->StringCount    = string_count;
    return 0;
}

PIL_API(void*)
StringTableIntern
(
    struct STRING_TABLE *table, 
    void const            *str, 
    uint32_t         char_type, 
    PFN_StringHash32   hash_fn
)
{
    uint32_t const  GROW_SIZE = STRING_BUFFER_GROW_SIZE;
    uint32_t const  ALIGNMENT = STRING_DATA_ALIGNMENT;
    uint32_t             i, n;
    uint32_t             hash;
    uint32_t            len_b;
    uint32_t            len_c;
    uint32_t          nb_need;
    uint32_t           nb_pad;
    uint32_t           eindex;
    uint32_t           bindex;
    uint32_t          eoffset;
    uint32_t          *hashes;
    STRING_HASH_CHUNK *bucket;
    STRING_INFO        *entry;
    uint8_t             *sptr;

    if (str != nullptr) {
        hash    = hash_fn(str, &len_b, &len_c);
        eoffset = table->StringDataNext;
        eindex  = table->StringCount;
        bindex  = hash &(table->HashBucketCount-1);
        bucket  = table->HashBuckets[bindex];
        while (bucket != nullptr) {
            for (i = 0, n = bucket->ItemCount, hashes = bucket->EntryHash; i < n; ++i) {
                if (hashes[i] == hash) {
                    entry = &table->StringList[bucket->EntryIndex[i]];
                    if (entry->ByteLength    == len_b && 
                        entry->CharLength    == len_c && 
                        entry->CharacterType == char_type) {
                        sptr = table->StringDataBase + entry->ByteOffset;
                        if (memcmp(str, sptr, len_b) == 0) {
                            return sptr;
                        }
                    }
                }
            } bucket = bucket->NextChunk;
        }

        /* the string doesn't exist in the table - intern it */
        bucket   = table->HashBuckets[bindex];
        eoffset  = table->StringDataNext; /* byte offset in storage block */
        eindex   = table->StringCount;    /* index in StringList array    */
        nb_need  = sizeof(uint32_t);      /* entry index in storage block */
        nb_need += len_b;                 /* string data, including nul   */
        nb_pad   = PIL_AlignUp(eoffset + nb_need, ALIGNMENT) - (eoffset + nb_need);
        nb_need += nb_pad;

        if (table->StringCount == table->StringCommitCount) {
            /* commit an additional 64KB of string entry data */
            uint32_t commit_size = GROW_SIZE;
            uint32_t  num_commit = table->StringReserveCount - table->StringCommitCount;
            uint32_t   nb_commit = num_commit * sizeof(STRING_INFO);
            if (nb_commit > commit_size) { /* commit a max of 64KB at once */
                nb_commit = commit_size;
                num_commit= nb_commit / sizeof(STRING_INFO);
            }
            if (VirtualAlloc(&table->StringList[table->StringCount], nb_commit, MEM_COMMIT, PAGE_READWRITE) == nullptr) {
                return nullptr;
            }
            table->StringCommitCount += num_commit;
        }
        if ((eoffset + nb_need) >= table->DataCommitSize) {
            /* commit additional storage block data */
            uint32_t  commit_size = GROW_SIZE;
            if (nb_need > commit_size) {
                commit_size = nb_need;
                commit_size = PIL_AlignUp(commit_size, GROW_SIZE);
            }
            if((table->DataCommitSize + commit_size) > table->DataReserveSize) {
                /* clamp to maximum data size */
                commit_size = table->DataReserveSize - table->DataCommitSize;
            }
            if (commit_size < nb_need) {
                /* not enough data storage */
                return nullptr;
            }
            if (VirtualAlloc(&table->StringDataBase[table->DataCommitSize], commit_size, MEM_COMMIT, PAGE_READWRITE) == nullptr) {
                return nullptr;
            }
            table->DataCommitSize += commit_size;
        }

        /* get the hash chunk where the item will be inserted */
        if (bucket == nullptr || bucket->ItemCount == STRING_HASH_CHUNK_CAPACITY) {
            if (table->HashFreeList == nullptr) {
                /* commit an additional block of hash chunks */
                uint32_t   commit_size = GROW_SIZE;
                uint32_t    num_commit = commit_size / sizeof(STRING_HASH_CHUNK);
                if((table->HashCommitCount + num_commit) > table->HashReserveCount) {
                    num_commit = table->HashReserveCount - table->HashCommitCount;
                    commit_size= num_commit * sizeof(STRING_HASH_CHUNK);
                }
                if (VirtualAlloc(&table->HashDataBase[table->HashCommitCount*sizeof(STRING_HASH_CHUNK)], commit_size, MEM_COMMIT, PAGE_READWRITE) == nullptr) {
                    return nullptr;
                }
                for (i = table->HashCommitCount, n = num_commit; i < n; ++i) {
                    STRING_HASH_CHUNK *chunk =(STRING_HASH_CHUNK*) &table->HashDataBase[i * sizeof(STRING_HASH_CHUNK)];
                    chunk->NextChunk         = table->HashFreeList;
                    chunk->ItemCount         = 0;
                    table->HashFreeList      = chunk;
                }
                table->HashCommitCount += num_commit;
            }
            /* take a chunk from the head of the free list.
             * insert it at the head of the hash bucket list. */
            bucket = table->HashFreeList;
            table->HashFreeList        = bucket->NextChunk;
            bucket->NextChunk          = table->HashBuckets[bindex];
            bucket->ItemCount          = 0;
            table->HashBuckets[bindex] = bucket;
        }

        /* append the item to the hash chunk */
        bucket->EntryHash [bucket->ItemCount] = hash;
        bucket->EntryIndex[bucket->ItemCount] = eindex;
        bucket->ItemCount++;

        /* cache the string information in the table */
        entry = &table->StringList[eindex];
        entry->ByteLength    = len_b;
        entry->CharLength    = len_c;
        entry->CharacterType = char_type;
        entry->ByteOffset    = eoffset + sizeof(uint32_t);
        table->StringCount++;

        /* copy the string data to the storage block */
        table->StringDataNext = eoffset + nb_need;
        sptr  = table->StringDataBase + eoffset;
        *(uint32_t*) sptr = eindex;
        sptr += sizeof(uint32_t );
        memcpy (sptr, str, len_b);
        sptr += len_b;
        for(i = 0; i < nb_pad; ++i) {
            *sptr++ = 0;
        }
        return table->StringDataBase + eoffset + sizeof(uint32_t);
    }
    return nullptr;
}

PIL_API(void)
StringTableGetTableInfo
(
    struct STRING_TABLE_INFO *info, 
    struct STRING_TABLE     *table
)
{
    info->StringInfo  = table->StringList;
    info->StringData  = table->StringDataBase;
    info->StringCount = table->StringCount;
    info->DataBytes   = table->StringDataNext;
}

PIL_API(int)
StringTableGetStringInfo
(
    struct STRING_INFO   *info, 
    struct STRING_TABLE *table, 
    void const            *str
)
{
    uint8_t const *addr =(uint8_t const*)str;
    uint8_t   *addr_min = table->StringDataBase;
    uint8_t   *addr_max = table->StringDataBase + table->StringDataNext;
    uint32_t      index;
    STRING_INFO      *i;
    if (addr  >= addr_min && addr < addr_max) {
        index  =*(uint32_t*)(addr-sizeof(uint32_t));
        assert(index < table->StringCount); 
        i = &table->StringList[index];
        info->ByteOffset     = i->ByteOffset;
        info->ByteLength     = i->ByteLength;
        info->CharLength     = i->CharLength;
        info->CharacterType  = i->CharacterType;
        return  0;
    } else {
        info->ByteOffset     = 0;
        info->ByteLength     = 0;
        info->CharLength     = 0;
        info->CharacterType  = STRING_CHAR_TYPE_UNKNOWN;
        return -1;
    }
}

