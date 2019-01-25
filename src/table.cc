/**
 * @summary table.cc: Implement the platform-agnostic components of the 
 * data table module.
 */
#include <string.h>
#include "table.h"

/* @summary Construct a HANDLE_BITS from its constituient parts.
 * @param _sparse_index The zero-based index within the sparse portion of the TABLE_INDEX that is allocated to the item.
 * @param _generation The generation value of the data slot allocated to the item.
 * @return The HANDLE_BITS identifying the item.
 */
#ifndef Table_MakeHandleBits
#define Table_MakeHandleBits(_sparse_index, _generation)                       \
     (HANDLE_FLAG_MASK_PACKED |                                                \
    (((_sparse_index) & HANDLE_INDEX_MASK) << HANDLE_INDEX_SHIFT) |            \
    (((_generation  ) & HANDLE_GENER_MASK) << HANDLE_GENER_SHIFT))
#endif

/* @summary Extract whether or not a HANDLE_BITS represents a possibly-valid item.
 * @param _bits The HANDLE_BITS value.
 * @return Non-zero if the HANDLE_BITS identifies an item that was valid at some point.
 */
#ifndef Table_HandleBitsExtractLive
#define Table_HandleBitsExtractLive(_bits)                                     \
    (((_bits) & HANDLE_FLAG_MASK_PACKED) >> HANDLE_FLAG_SHIFT)
#endif

/* @summary Extract the generation value of the data slot associated with a HANDLE_BITS.
 * @param _bits The HANDLE_BITS value.
 * @return The generation value portion of the handle.
 */
#ifndef Table_HandleBitsExtractGeneration
#define Table_HandleBitsExtractGeneration(_bits)                               \
    (((_bits) & HANDLE_GENER_MASK_PACKED) >> HANDLE_GENER_SHIFT)
#endif

/* @summary Extract the sparse slot index encoded within a HANDLE_BITS.
 * @param _bits The HANDLE_BITS value.
 * @return The zero-based index within the sparse portion of the TABLE_INDEX allocated to the item.
 */
#ifndef Table_HandleBitsExtractSparseIndex
#define Table_HandleBitsExtractSparseIndex(_bits)                              \
    (((_bits) & HANDLE_INDEX_MASK_PACKED) >> HANDLE_INDEX_SHIFT)
#endif

/* @summary Extract a value indicating whether or not a sparse index slot represents a valid item.
 * @param _word The word read from the sparse index.
 * @return Non-zero if the slot is associated with a valid item.
 */
#ifndef Table_SparseIndexExtractLive
#define Table_SparseIndexExtractLive(_word)                                    \
    (((_word) & HANDLE_FLAG_MASK_PACKED) >> HANDLE_FLAG_SHIFT)
#endif

/* @summary Extract the generation value of the data slot associated with a sparse index slot.
 * @param _word The word read from the sparse index.
 * @return The generation value portion of the index value.
 */
#ifndef Table_SparseIndexExtractGeneration
#define Table_SparseIndexExtractGeneration(_word)                              \
    (((_word) & HANDLE_GENER_MASK_PACKED) >> HANDLE_GENER_SHIFT)
#endif

/* @summary Extract the dense array index encoded within a sparse index slot.
 * @param _word The word read from the sparse index.
 * @return The dense index portion of the index value.
 */
#ifndef Table_SparseIndexExtractDenseIndex
#define Table_SparseIndexExtractDenseIndex(_word)                              \
    (((_word) & HANDLE_INDEX_MASK_PACKED) >> HANDLE_INDEX_SHIFT)
#endif

/* @summary Move the data for a given table slot from one location to another.
 * @param desc Pointer to a TABLE_DESC describing the table data streams.
 * @param dst_index The destination index.
 * @param src_index The source index.
 */
static inline void
MoveTableItemData
(
    struct TABLE_DESC *desc, 
    uint32_t      dst_index, 
    uint32_t      src_index
)
{
    TABLE_DATA **streams = desc->Streams;
    void          *src_p;
    void          *dst_p;
    uint32_t        i, n;
    for (i = 0, n = desc->StreamCount; i < n; ++i) {
        src_p = TableData_GetElementPointer(void, streams[i], src_index);
        dst_p = TableData_GetElementPointer(void, streams[i], dst_index);
        memcpy(dst_p, src_p, streams[i]->ElementSize);
    }
}

PIL_API(void)
TableDeleteAllIds
(
    struct TABLE_DESC *table
)
{
    TABLE_INDEX     *index = table->Index;
    uint32_t *sparse_array = index->SparseIndex;
    uint32_t *handle_array = index->HandleArray;
    uint32_t  handle_value;
    uint32_t  sparse_index;
    uint32_t    generation;
    uint32_t          i, n;
    for (i = 0, n = index->ActiveCount; i < n; ++i) {
        handle_value = handle_array[i];
        generation   = Table_HandleBitsExtractGeneration(handle_value);
        sparse_index = Table_HandleBitsExtractSparseIndex(handle_value);
        sparse_array[sparse_index] = ((generation + 1) & HANDLE_GENER_MASK) << HANDLE_GENER_SHIFT;
        handle_array[i]  = sparse_index;
    } index->ActiveCount = 0;
}

PIL_API(void)
TableRemoveAllIds
(
    struct TABLE_DESC *table
)
{
    TABLE_INDEX     *index = table->Index;
    uint32_t *sparse_array = index->SparseIndex;
    size_t    sparse_bytes = index->TableCapacity * sizeof(uint32_t);
    memset(sparse_array, 0 , sparse_bytes);
    index->ActiveCount = 0;
}

PIL_API(HANDLE_BITS)
TableDeleteId
(
    struct TABLE_DESC *table, 
    HANDLE_BITS         bits
)
{
    TABLE_INDEX     *index = table->Index;
    uint32_t *sparse_array = index->SparseIndex;
    uint32_t *handle_array = index->HandleArray;
    uint32_t    last_dense = index->ActiveCount - 1;
    uint32_t   moved_value = HANDLE_BITS_INVALID;
    uint32_t  sparse_index = Table_HandleBitsExtractSparseIndex(bits);
    uint32_t  sparse_value;
    uint32_t   moved_gener;
    uint32_t   moved_index;
    uint32_t   dense_index;
    uint32_t    generation;

    if (sparse_index < index->TableCapacity) {
        sparse_value = sparse_array[sparse_index];
        generation   = Table_SparseIndexExtractGeneration(sparse_value);
        dense_index  = Table_SparseIndexExtractDenseIndex(sparse_value);
        sparse_array[sparse_index] = ((generation + 1) & HANDLE_GENER_MASK) << HANDLE_GENER_SHIFT;
        /* if the deleted item is not the last slot in the dense array, 
         * swap the last live item into the slot vacated by the deleted 
         * item in order to keep the handle and data arrays densely packed.
         */
        if (dense_index != last_dense) {
            moved_value  = handle_array[last_dense];
            moved_gener  = Table_HandleBitsExtractGeneration(moved_value);
            moved_index  = Table_HandleBitsExtractSparseIndex(moved_value);
            MoveTableItemData(table, dense_index, last_dense);
            sparse_array[moved_index] = HANDLE_FLAG_MASK_PACKED | (dense_index << HANDLE_INDEX_SHIFT) | (moved_gener << HANDLE_GENER_SHIFT);
            handle_array[dense_index] = moved_value;
        }
        /* return sparse_index to the free list */
        handle_array[last_dense] = sparse_index;
        index->ActiveCount = last_dense;
    }
    return moved_value;
}

PIL_API(void)
TableDeleteIds
(
    struct TABLE_DESC *table, 
    HANDLE_BITS  *delete_ids, 
    uint32_t    delete_count
)
{
#   define              HC   64
#   define              HM  (HC-1)
    TABLE_INDEX     *index = table->Index;
    uint32_t *sparse_array = index->SparseIndex;
    uint32_t *handle_array = index->HandleArray;
    uint32_t  active_count = index->ActiveCount;
    uint32_t    last_dense = index->ActiveCount - 1;
    uint32_t    move_count = 0;
    uint32_t   state_value; /* read from sparse_array  */
    uint32_t   state_index; /* index into sparse_array */
    uint32_t   dense_index; /* index into handle_array */
    uint32_t   moved_index; /* index into sparse_array */
    uint32_t   moved_value; /* read from handle_array  */
    uint32_t   moved_gener;
    uint32_t   history[HC];
    uint32_t         found;
    uint32_t       i, j, n;

    if (delete_count > active_count) {
        assert(delete_count <= active_count);
        return;
    }
    if (delete_count == active_count) {
        /* the entire table contents is being deleted */
        TableDeleteAllIds(table);
        return;
    }
    /* only part of the table is being deleted. 
     * for large deletions, there can be significant overhead due to repeatedly moving elements.
     * optimize for this case by tracking which items are moved and where they were moved to. 
     * then, perform a second pass so that any moved item gets moved at most once.
     */
    for (i = 0; i < delete_count; ++i, --last_dense) {
        state_index = Table_HandleBitsExtractSparseIndex(delete_ids[i]);
        state_value = sparse_array[state_index];
        dense_index = Table_SparseIndexExtractDenseIndex(state_value);
        moved_value = handle_array[last_dense];
        moved_index = Table_HandleBitsExtractSparseIndex(moved_value);
        moved_gener = Table_HandleBitsExtractGeneration (moved_value);
        /* invalidate the deleted handle */
        sparse_array[state_index] = (state_value + HANDLE_GENER_ADD_PACKED) & HANDLE_GENER_MASK_PACKED;
        if (dense_index != last_dense) { /* update the index arrays */
            sparse_array[moved_index] = HANDLE_FLAG_MASK_PACKED | (dense_index & HANDLE_INDEX_MASK) | (moved_gener & HANDLE_GENER_MASK);
            handle_array[dense_index] = handle_array[last_dense];
        }
    } index->ActiveCount -= delete_count;
    /* second pass, move all of the actual data.
     * the items in handle_array [index->ActiveCount, index->ActiveCount+delete_count) 
     * can be used to retrieve the sparse index value stored at the given dense index.
     */
    for (i = index->ActiveCount, n = index->ActiveCount + delete_count; i < n; ++i) {
        state_index = Table_HandleBitsExtractSparseIndex(handle_array[i]);
        state_value = sparse_array[state_index];
        dense_index = Table_SparseIndexExtractDenseIndex(state_value);
        /* item at state_index was moved in handle_array from location i to location dense_index.
         * if the state_index doesn't appear in the move history buffer, move the associated data.
         */
        for (j = 0, found = 0; j < HC && j < move_count; ++j) {
            if (history[j] == state_index) {
                found = 1;
                break;
            }
        }
        if (found == 0) {
            MoveTableItemData(table  , dense_index, i);
            history[move_count & HM] = state_index;
            move_count++;
        }
    }
#   undef HM
#   undef HC
}

PIL_API(HANDLE_BITS)
TableRemoveId
(
    struct TABLE_DESC *table, 
    HANDLE_BITS         bits
)
{
    TABLE_INDEX     *index = table->Index;
    uint32_t *sparse_array = index->SparseIndex;
    uint32_t *handle_array = index->HandleArray;
    uint32_t    last_dense = index->ActiveCount - 1;
    uint32_t   moved_value = HANDLE_BITS_INVALID;
    uint32_t  sparse_index = Table_HandleBitsExtractSparseIndex(bits);
    uint32_t  sparse_value;
    uint32_t   moved_gener;
    uint32_t   moved_index;
    uint32_t   dense_index;

    if (sparse_index < index->TableCapacity) {
        sparse_value = sparse_array[sparse_index];
        dense_index  = Table_SparseIndexExtractDenseIndex(sparse_value);
        sparse_array[sparse_index] = 0;
        /* if the deleted item is not the last slot in the dense array, 
         * swap the last live item into the slot vacated by the deleted 
         * item in order to keep the handle and data arrays densely packed.
         */
        if (dense_index != last_dense) {
            moved_value  = handle_array[last_dense];
            moved_gener  = Table_HandleBitsExtractGeneration(moved_value);
            moved_index  = Table_HandleBitsExtractSparseIndex(moved_value);
            MoveTableItemData(table, dense_index, last_dense);
            sparse_array[moved_index] = HANDLE_FLAG_MASK_PACKED | (dense_index << HANDLE_INDEX_SHIFT) | (moved_gener << HANDLE_GENER_SHIFT);
            handle_array[dense_index] = moved_value;
        }
        index->ActiveCount = last_dense;
    }
    return moved_value;
}

PIL_API(int)
TableResolve
(
    uint32_t *o_record_index, 
    struct TABLE_DESC *table, 
    HANDLE_BITS         bits
)
{
    TABLE_INDEX      *index = table->Index;
    uint32_t  *sparse_array = index->SparseIndex;
    uint32_t        is_live = Table_HandleBitsExtractLive(bits);
    uint32_t   generation_h = Table_HandleBitsExtractGeneration(bits);
    uint32_t   sparse_index = Table_HandleBitsExtractSparseIndex(bits);
    uint32_t   sparse_value;
    uint32_t   generation_i;

    if (is_live) {
        sparse_value   = sparse_array[sparse_index];
        generation_i   = Table_SparseIndexExtractGeneration(sparse_value);
       *o_record_index = Table_SparseIndexExtractDenseIndex(sparse_value);
        assert (generation_h == generation_i);
        return (generation_h == generation_i);
    } else {
        return 0;
    }
}

PIL_API(HANDLE_BITS)
TableCreateId
(
    uint32_t *o_record_index, 
    struct TABLE_DESC *table
)
{
    TABLE_INDEX     *index = table->Index;
    uint32_t *sparse_array = index->SparseIndex;
    uint32_t *handle_array = index->HandleArray;
    uint32_t  handle_index = index->ActiveCount;
    uint32_t  sparse_index;
    uint32_t    generation;
    uint32_t    slot_value;
    HANDLE_BITS       bits;

    assert(index->ActiveCount < index->CommitCount);
    if (handle_index == index->HighWatermark) {
        index->HighWatermark = handle_index + 1;
        sparse_index         = handle_index;
        generation           = 0;
        slot_value           = 0;
    } else {
        slot_value           = handle_array[handle_index];
        generation           = Table_HandleBitsExtractGeneration(slot_value);
        sparse_index         = Table_HandleBitsExtractSparseIndex(slot_value);
    }
    bits = Table_MakeHandleBits(sparse_index, generation);
    sparse_array[sparse_index] = HANDLE_FLAG_MASK_PACKED | (handle_index << HANDLE_INDEX_SHIFT) | (generation << HANDLE_GENER_SHIFT);
    handle_array[handle_index] = bits;
   *o_record_index = handle_index; 
    index->ActiveCount++;
    return bits;
}

PIL_API(int)
TableInsertId
(
    uint32_t *o_record_index, 
    struct TABLE_DESC *table, 
    HANDLE_BITS         bits
)
{
    TABLE_INDEX     *index = table->Index;
    uint32_t *sparse_array = index->SparseIndex;
    uint32_t *handle_array = index->HandleArray;
    uint32_t  handle_index = index->ActiveCount + 1;
    uint32_t    generation = Table_HandleBitsExtractGeneration(bits);
    uint32_t  sparse_index = Table_HandleBitsExtractSparseIndex(bits);

    assert(index->ActiveCount < index->CommitCount);
    if (sparse_index < index->TableCapacity && sparse_array[sparse_index] == 0) {
        sparse_array[sparse_index] = HANDLE_FLAG_MASK_PACKED | (handle_index << HANDLE_INDEX_SHIFT) | (generation << HANDLE_GENER_SHIFT);
        handle_array[handle_index] = bits;
       *o_record_index = handle_index;
        return 0;
    }
    return -1;
}

PIL_API(HANDLE_BITS)
MakeHandleBits
(
    uint32_t sparse_index, 
    uint32_t   generation
)
{
    return Table_MakeHandleBits(sparse_index, generation);
}

PIL_API(uint32_t)
HandleBitsExtractLive
(
    HANDLE_BITS bits
)
{
    return Table_HandleBitsExtractLive(bits);
}

PIL_API(uint32_t)
HandleBitsExtractGeneration
(
    HANDLE_BITS bits
)
{
    return Table_HandleBitsExtractGeneration(bits);
}

PIL_API(uint32_t)
HandleBitsExtractSparseIndex
(
    HANDLE_BITS bits
)
{
    return Table_HandleBitsExtractSparseIndex(bits);
}

PIL_API(uint32_t)
SparseIndexExtractLive
(
    uint32_t index_value
)
{
    return Table_SparseIndexExtractLive(index_value);
}

PIL_API(uint32_t)
SparseIndexExtractGeneration
(
    uint32_t index_value
)
{
    return Table_SparseIndexExtractGeneration(index_value);
}

PIL_API(uint32_t)
SparseIndexExtractDenseIndex
(
    uint32_t index_value
)
{
    return Table_SparseIndexExtractDenseIndex(index_value);
}

PIL_API(uint32_t)
TableData_GetElementIndex
(
    struct TABLE_DATA *table_data, 
    void             *element_ptr
)
{
    uint8_t *beg = TableData_GetBuffer(uint8_t, table_data);
    uint32_t siz = TableData_GetElementSize(table_data);
    uint32_t idx =(uint32_t)((((uint8_t*) element_ptr) - beg) / siz);
    return   idx;
}

