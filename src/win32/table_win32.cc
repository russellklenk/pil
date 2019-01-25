/**
 * @summary table_win32.cc: Implement the Windows platform-specific components 
 * of the data table API.
 */
#include <Windows.h>
#include "table.h"

PIL_API(int)
TableCreate
(
    struct TABLE_INIT *init
)
{
    uint8_t              *index_ptr = nullptr;
    uint32_t            *sparse_ptr = nullptr;
    uint32_t            *handle_ptr = nullptr;
    void                *stream_ptr = nullptr;
    size_t            sparse_commit = 0;
    size_t            handle_commit = 0;
    size_t           handle_reserve = 0;
    size_t            index_reserve = 0;
    TABLE_INDEX              *index = init->Index;
    TABLE_DATA_STREAM_DESC *streams = init->Streams;
    uint32_t           stream_count = init->StreamCount;
    uint32_t                      i;

    if (init->Index == nullptr) {
        assert(init->Index != nullptr);
        return -1;
    }
    if (init->TableCapacity < TABLE_MIN_OBJECT_COUNT) {
        assert(init->TableCapacity >= TABLE_MIN_OBJECT_COUNT);
        return -1;
    }
    if (init->TableCapacity > TABLE_MAX_OBJECT_COUNT) {
        assert(init->TableCapacity <= TABLE_MAX_OBJECT_COUNT);
        return -1;
    }
    for (i = 0; i < stream_count; ++i) {
        if (streams[i].Data == nullptr) {
            assert(streams[i].Data != nullptr);
            return -1;
        }
        if (streams[i].Size == 0) {
            assert(streams[i].Size != 0);
            return -1;
        }
    }

    /* reserve process address space for the index & data */
    sparse_commit  = init->TableCapacity * sizeof(uint32_t);
    handle_commit  = init->InitialCommit * sizeof(uint32_t);
    handle_reserve = init->TableCapacity * sizeof(uint32_t);
    index_reserve  = sparse_commit + handle_reserve;
    if ((index_ptr =(uint8_t*) VirtualAlloc(nullptr, index_reserve, MEM_RESERVE, PAGE_NOACCESS)) == nullptr) {
        goto cleanup_and_fail;
    }
    sparse_ptr = (uint32_t*)(index_ptr + 0);
    handle_ptr = (uint32_t*)(index_ptr + sparse_commit);
    for (i = 0 ; i < stream_count; ++i) {
        if ((stream_ptr = VirtualAlloc(nullptr, init->TableCapacity * streams[i].Size, MEM_RESERVE, PAGE_NOACCESS)) == NULL) {
            goto cleanup_and_fail;
        }
        streams[i].Data->StorageBuffer = stream_ptr;
        streams[i].Data->ElementSize   = streams[i].Size;
    }
    /* the sparse portion of the index is always fully committed */
    if (VirtualAlloc(sparse_ptr, sparse_commit, MEM_COMMIT, PAGE_READWRITE) == nullptr) {
        goto cleanup_and_fail;
    }
    if (init->InitialCommit > 0) {
        /* the dense portion of the index and the data streams are committed on-demand */
        if (VirtualAlloc(handle_ptr, handle_commit, MEM_COMMIT, PAGE_READWRITE) == nullptr) {
            goto cleanup_and_fail;
        }
        for (i = 0; i < stream_count; ++i) {
            size_t     stream_commit  = init->InitialCommit * streams[i].Size;
            if (VirtualAlloc(streams[i].Data->StorageBuffer , stream_commit, MEM_COMMIT, PAGE_READWRITE) == nullptr) {
                goto cleanup_and_fail;
            }
        }
    }
    index->SparseIndex   = sparse_ptr;
    index->HandleArray   = handle_ptr;
    index->ActiveCount   = 0;
    index->HighWatermark = 0;
    index->CommitCount   = init->InitialCommit;
    index->TableCapacity = init->TableCapacity;
    return 0;

cleanup_and_fail:
    for (i = 0; i < stream_count; ++i) {
        if (streams[i].Data->StorageBuffer != nullptr) {
            VirtualFree(streams[i].Data->StorageBuffer, 0, MEM_RELEASE);
            streams[i].Data->StorageBuffer = nullptr;
        }
    }
    if (index_ptr != nullptr) {
        VirtualFree(index_ptr, 0, MEM_RELEASE);
    }
    return -1;
}

PIL_API(int)
TableEnsure
(
    struct TABLE_DESC *table, 
    uint32_t      total_need, 
    uint32_t      chunk_size
)
{
    TABLE_INDEX      *index = table->Index;
    TABLE_DATA    **streams = table->Streams;
    size_t    handle_commit;
    size_t    stream_commit;
    uint32_t    chunk_count;
    uint32_t new_item_count;
    uint32_t           i, n;

    if (index->CommitCount  >= total_need) {
        return 0;
    }
    chunk_count    = (total_need + (chunk_size-1)) / chunk_size;
    new_item_count = (chunk_size * chunk_count);
    if (new_item_count > index->TableCapacity) {
        new_item_count = index->TableCapacity;
    }
    if (new_item_count < total_need) {
        return -1;
    }
    handle_commit = new_item_count * sizeof(uint32_t);
    if (VirtualAlloc(index->HandleArray, handle_commit, MEM_COMMIT, PAGE_READWRITE) == nullptr) {
        return -1;
    }
    for (i = 0, n = table->StreamCount; i < n; ++i) {
        stream_commit = new_item_count * streams[i]->ElementSize;
        if (VirtualAlloc(streams[i]->StorageBuffer, stream_commit, MEM_COMMIT, PAGE_READWRITE) == nullptr) {
            return -1;
        }
    }
    index->CommitCount = new_item_count;
    return 0;
}

PIL_API(void)
TableDelete
(
    struct TABLE_DESC *table
)
{
    TABLE_INDEX   *index = table->Index;
    TABLE_DATA **streams = table->Streams;
    uint32_t        i, n;
    for (i = 0, n = table->StreamCount; i < n; ++i) {
        if (streams[i]->StorageBuffer) {
            VirtualFree(streams[i]->StorageBuffer, 0, MEM_RELEASE);
            streams[i]->StorageBuffer = nullptr;
        }
    }
    if (index && index->SparseIndex) {
        VirtualFree(index->SparseIndex, 0, MEM_RELEASE);
        index->SparseIndex   = nullptr;
        index->HandleArray   = nullptr;
        index->ActiveCount   = 0;
        index->CommitCount   = 0;
        index->TableCapacity = 0;
    }
 }

