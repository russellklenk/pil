/**
 * @summary table.h: Defines data structures and functions for working with 
 * data tables, where items in the table are identified by opaque 32-bit 
 * integers. Items in the table are stored densely-packed like in an array.
 * Memory management for the table utilizes the host virtual memory manager.
 */
#ifndef __PIL_TABLE_H__
#define __PIL_TABLE_H__

#pragma once

#ifndef PIL_NO_INCLUDES
#   ifndef __PIL_H__
#       include "pil.h"
#   endif
#endif

/* @summary Define various shifts and masks used when working with item handles.
 * These values are used when constructing and breaking apart HANDLE_BITS values.
 * A HANDLE_BITS value consists of the following packed into a 32-bit unsigned integer:
 * 31.|.......|....................|....0
 *   F|SSSSSSS|IIIIIIIIIIIIIIIIIIII|GGGG
 * Starting from the most signficant bit:
 * - F is set if the handle was valid at some point (used to distinguish valid and invalid handles).
 * - S is a salt value used to differentiate handles coming from different tables.
 * - I is an index value specifying the SparseIndex array index, which is then used to look up the corresponding dense index.
 * - G is a generation counter used to differentiate handle slots that have been recycled.
 * The bits are ordered so that if you have a collection of handles, and then sort them, they will be grouped first by type, then by chunk, then by index within chunk.
 */
#ifndef HANDLE_CONSTANTS
#   define HANDLE_CONSTANTS
#   define HANDLE_BITS_INVALID       0UL
#   define HANDLE_GENER_BITS         11
#   define HANDLE_INDEX_BITS         20
#   define HANDLE_FLAG_BITS          1

#   define HANDLE_GENER_SHIFT        0
#   define HANDLE_INDEX_SHIFT       (HANDLE_GENER_SHIFT + HANDLE_GENER_BITS)
#   define HANDLE_FLAG_SHIFT        (HANDLE_INDEX_SHIFT + HANDLE_INDEX_BITS)

#   define HANDLE_GENER_MASK       ((1UL << HANDLE_GENER_BITS) - 1)
#   define HANDLE_INDEX_MASK       ((1UL << HANDLE_INDEX_BITS) - 1)
#   define HANDLE_FLAG_MASK        ((1UL << HANDLE_FLAG_BITS ) - 1)

#   define HANDLE_GENER_MASK_PACKED (HANDLE_GENER_MASK << HANDLE_GENER_SHIFT)
#   define HANDLE_INDEX_MASK_PACKED (HANDLE_INDEX_MASK << HANDLE_INDEX_SHIFT)
#   define HANDLE_FLAG_MASK_PACKED  (HANDLE_FLAG_MASK  << HANDLE_FLAG_SHIFT )
#   define HANDLE_GENER_ADD_PACKED  (1UL << HANDLE_GENER_SHIFT)
#endif

/* @summary Define various constants related to the data table implementation.
 * TABLE_MIN_OBJECT_COUNT: The minimum capacity for a table.
 * TABLE_MAX_OBJECT_COUNT: The maximum capacity for a table.
 */
#ifndef TABLE_CONSTANTS
#   define TABLE_CONSTANTS
#   define TABLE_MIN_OBJECT_COUNT    1UL
#   define TABLE_MAX_OBJECT_COUNT   (1UL << HANDLE_INDEX_BITS)
#   define TABLE_CHUNK_SIZE          1024
#endif

/* @summary Read the number of live items in the table from the TABLE_INDEX.
 * @param _x A pointer to a TABLE_INDEX structure.
 * @return The number of live items in the table.
 */
#ifndef TableIndex_GetCount
#define TableIndex_GetCount(_x)                                                \
    (_x)->ActiveCount
#endif

/* @summary Read the maximum capacity of a table from the TABLE_INDEX.
 * @param _x A pointer to a TABLE_INDEX structure.
 * @return The capacity of the table, in items.
 */
#ifndef TableIndex_GetCapacity
#define TableIndex_GetCapacity(_x)                                             \
    (_x)->TableCapacity
#endif

/* @summary Read a handle value from a TABLE_INDEX.
 * @param _x A pointer to a TABLE_INDEX structure.
 * @param _i The zero-based dense index of the handle to read. The dense index is extracted from the sparse index value.
 * @return The corresponding HANDLE_BITS.
 */
#ifndef TableIndex_GetHandle
#define TableIndex_GetHandle(_x, _i)                                           \
    (_x)->HandleArray[(_i)]
#endif

/* @summary Retrieve the base address of the storage buffer for a TABLE_DATA.
 * @param _type The typename to return.
 * @param _d The TABLE_DATA structure to query.
 * @return A pointer (_type*) to the first element of the storage buffer.
 */
#ifndef TableData_GetBuffer
#define TableData_GetBuffer(_type, _d)                                         \
    (_type*)((_d)->StorageBuffer)
#endif

/* @summary Retrieve the number of bytes between elements in a TABLE_DATA.
 * @param _d The TABLE_DATA structure to query.
 * @return The number of bytes between elements.
 */
#ifndef TableData_GetElementSize
#define TableData_GetElementSize(_d)                                           \
    (_d)->ElementSize
#endif

/* @summary Retrieve a pointer to the _i'th element in a TABLE_DATA.
 * @param _type The typename to return.
 * @param _d The TABLE_DATA structure to query.
 * @param _i The zero-based index of the item to retrieve.
 * @return A pointer (_type*) to the given element.
 */
#ifndef TableData_GetElementPointer
#define TableData_GetElementPointer(_type, _d, _i)                             \
    (_type*)(((uint8_t*)(_d)->StorageBuffer) +((_i) * (_d)->ElementSize))
#endif

/* @summary Retrieve the number of active items in a table.
 * @param _td A pointer to a TÁBLE_DESC structure.
 * @return The number of active items in the table.
 */
#ifndef Table_GetCount
#define Table_GetCount(_td)                                                    \
    (_td)->Index->ActiveCount
#endif

/* @summary Retrieve the capacity of a table.
 * @param _td A pointer to a TABLE_DESC structure.
 * @return The maximum number of items that can be stored in the table.
 */
#ifndef Table_GetCapacity
#define Table_GetCapacity(_td)                                                 \
    (_td)->Index->TableCapacity
#endif

/* @summary Retrieve the _ix'th handle for a table.
 * @param _td A pointer to a TABLE_DESC structure.
 * @param _ix The zero-based index of the item to return.
 * @return The HANDLE_BITS for the given item.
 */
#ifndef Table_GetHandle
#define Table_GetHandle(_td, _ix)                                              \
    (_td)->Index->HandleArray[(_ix)]
#endif

/* @summary Retrieve a pointer to the first element in the active handle stream for a table.
 * @param _td A pointer to a TABLE_DESC structure.
 * @return A pointer (HANDLE_BITS*) to the first active identifier in the table. If equal to the address returned by Table_GetHandleEnd(_td), the table is empty.
 */
#ifndef Table_GetHandleBegin
#define Table_GetHandleBegin(_td)                                              \
    (_td)->Index->HandleArray
#endif

/* @summary Retrieve a pointer to one-past the last element in the active handle stream for a table.
 * @param _td A pointer to a TABLE_DESC structure.
 * @return A pointer (HANDLE_BITS*) to one-past the last active identifier. Do not dereference the returned pointer.
 */
#ifndef Table_GetHandleEnd
#define Table_GetHandleEnd(_td)                                                \
   ((_td)->Index->HandleArray + (_td)->Index->ActiveCount)
#endif

/* @summary Retrieve a pointer to the first element in a table data stream.
 * @param _type The typename to return.
 * @param _td A pointer to a TABLE_DESC structure.
 * @param _si The zero-based index of the data stream to query.
 * @return A pointer to the first active element in the data stream. If equal to the address returned by Table_GetStreamEnd(_type, _td, _si), the table is empty.
 */
#ifndef Table_GetStreamBegin
#define Table_GetStreamBegin(_type, _td, _si)                                  \
    ((_type*)((_td)->Streams[(_si)]->StorageBuffer))
#endif

/* @summary Retrieve a pointer to one-past the last element in a table data stream.
 * @param _type The typename to return.
 * @param _td A pointer to a TABLE_DESC structure.
 * @param _si The zero-based index of the data stream to query.
 * @return A pointer to one-past the last active element in the data stream. Do not dereference the returned pointer.
 */
#ifndef Table_GetStreamEnd
#define Table_GetStreamEnd(_type, _td, _si)                                    \
    ((_type*)(((uint8_t*)((_td)->Streams[(_si)]->StorageBuffer)) + ((_td)->Streams[(_si)]->ElementSize * (_td)->Index->ActiveCount)))
#endif

/* @summary Retrieve the number of bytes between elements in a table data stream.
 * @param _td A pointer to a TABLE_DESC structure.
 * @param _si The zero-based index of the data stream to query.
 * @return The number of bytes between elements in the data stream.
 */
#ifndef Table_GetStreamElementSize
#define Table_GetStreamElementSize(_td, _si)                                   \
    ((_td)->Streams[(_si)]->ElementSize)
#endif

/* @summary Retrieve a pointer to the _ei'th element in a table data stream.
 * @param _type The typename to return.
 * @param _td A pointer to a TABLE_DESC structure.
 * @param _si The zero-based index of the data stream to access.
 * @param _ei The zero-based index of the item to retrieve.
 * @return A pointer (_type*) to the given element.
 */
#ifndef Table_GetStreamElement
#define Table_GetStreamElement(_type, _td, _si, _ei)                           \
    ((_type*)(((uint8_t*)((_td)->Streams[(_si)]->StorageBuffer)) + ((_td)->Streams[(_si)]->ElementSize * (_ei))))
#endif

/* @summary Items are identified by a handle, which is represented by a bit-packed 32-bit integer.
 */
typedef uint32_t HANDLE_BITS;

/* @summary Define the data associated with the index used to map a 32-bit integer item ID to a dense array index.
 */
typedef struct TABLE_INDEX {
    uint32_t                      *SparseIndex;                                /* The fully committed sparse array used to map item handles to indices in TABLE_DATA and the HandleArray. */
    uint32_t                      *HandleArray;                                /* A partially committed, densely-packed array of the handles associated with each item in the table. */
    uint32_t                       ActiveCount;                                /* The number of items in the table that are valid. */
    uint32_t                       HighWatermark;                              /* The maximum number of items observed in the table since it was created or reset. */
    uint32_t                       CommitCount;                                /* The maximum number of items that can be stored in the table without committing additional memory. */
    uint32_t                       TableCapacity;                              /* The maximum capacity of the table, in items. */
} TABLE_INDEX;

/* @summary Define the structure describing the buffer used to store tightly-packed item data records in a table.
 * All items in a TABLE_DATA have the same type, but a table may have more than one associated TABLE_DATA.
 */
typedef struct TABLE_DATA {
    void                          *StorageBuffer;                              /* A pointer to the start of the storage buffer used for storing table records. */
    uint32_t                       ElementSize;                                /* The size of the record type stored in the table data. */
} TABLE_DATA;

/* @summary Define a structure used to describe a TABLE_DATA representing a data stream.
 * This structure is used during table creation only.
 */
typedef struct TABLE_DATA_STREAM_DESC {
    struct TABLE_DATA             *Data;                                       /* A pointer to the TABLE_DATA to populate. */
    uint32_t                       Size;                                       /* The size of the record type stored in the table data. */
} TABLE_DATA_STREAM_DESC;

/* @summary Define the data used to construct a new data table.
 */
typedef struct TABLE_INIT {
    struct TABLE_INDEX            *Index;                                      /* The TABLE_INDEX associated with the table to initialize. */
    struct TABLE_DATA_STREAM_DESC *Streams;                                    /* An array of StreamCount descriptors of the TABLE_DATA objects to initialize. */
    uint32_t                       StreamCount;                                /* The number of valid entries in the Streams array. */
    uint32_t                       TableCapacity;                              /* The maximum number of items that can be stored in the table. */
    uint32_t                       InitialCommit;                              /* The initial table committment, in items. */
} TABLE_INIT;

/* @summary Define the data used to describe an existing data table.
 */
typedef struct TABLE_DESC {
    struct TABLE_INDEX            *Index;                                      /* The TABLE_INDEX used to map handles to their corresponding records. */
    struct TABLE_DATA            **Streams;                                    /* An array of StreamCount densely-packed data streams representing the table records. */
    uint32_t                       StreamCount;                                /* The number of valid entries in the Streams array. */
} TABLE_DESC;

#ifdef __cplusplus
extern "C" {
#endif

/* @summary Allocate resources for a data table.
 * The implementation of this function is platform-specific.
 * @param init Pointer to a TABLE_INIT describing the index and data streams to allocate.
 * @return Zero if the table is successfully initialized, or non-zero if an error occurred.
 */
PIL_API(int)
TableCreate
(
    struct TABLE_INIT *init
);

/* @summary Ensure that a data table can accomodate a given number of items.
 * If necessary and possible, the table committment is increased to meet the need.
 * The implementation of this function is platform-specific.
 * @param table Pointer to a TABLE_DESC describing the index and data streams for the table.
 * @param total_need The total number of items the caller needs to store in the table (for example, TableIndex_GetCount() + 1).
 * @param chunk_size The chunk size for the table. The commitment is increased to an even multiple of this value to reduce the overall number of allocations.
 * @return Zero if the table can store at least total_need items, or non-zero if an error occurred.
 */
PIL_API(int)
TableEnsure
(
    struct TABLE_DESC *table, 
    uint32_t      total_need, 
    uint32_t      chunk_size
);

/* @summary Free all resources allocated by a data table.
 * The implementation of this function is platform-specific.
 * @param table Pointer to a TABLE_DESC describing the index and data streams for the table.
 */
PIL_API(void)
TableDelete
(
    struct TABLE_DESC *table
);

/* @summary For a table containing internally-managed identifiers, reset the table back to empty.
 * The table data should have already had any necessary cleanup performed prior to the call.
 * @param table Pointer to a TABLE_DESC describing the table to reset.
 */
PIL_API(void)
TableDeleteAllIds
(
    struct TABLE_DESC *table
);

/* @summary For a table containing externally managed identifiers, reset the table back to empty.
 * The table data should have already had any necessary cleanup performed prior to the call.
 * @param table Pointer to a TABLE_DESC describing the table to reset.
 */
PIL_API(void)
TableRemoveAllIds
(
    struct TABLE_DESC *table
);

/* @summary Invalidate a single table item identifier created by the TableCreateId function.
 * The corresponding table data should have already had any necessary cleanup performed prior to calling this function.
 * The caller is responsible for ensuring that the value supplied for bits represents a valid table entry.
 * @param table Pointer to a TABLE_DESC describing the table that created the item identifier.
 * @param bits The HANDLE_BITS identifying the item to delete.
 * @return The handle of the item that was moved as the result of the deletion, or HANDLE_BITS_INVALID if no item was moved.
 */
PIL_API(HANDLE_BITS)
TableDeleteId
(
    struct TABLE_DESC *table, 
    HANDLE_BITS         bits
);

/* @summary Invalidate multiple table item identifiers created by the TableCreateId function.
 * The corresponding table data should have already had any necessary cleanup performed prior to calling this function.
 * The caller is responsible for ensuring that the values supplied in the delete_ids array represent valid table entries.
 * The caller is responsible for ensuring that the delete_ids array does not contain duplicate values.
 * When deleting large numbers of items, this function may increase the table memory committment.
 * @param table Pointer to a TABLE_DESC describing the table that created the item identifiers.
 * @param delete_ids An array of delete_count HANDLE_BITS identifying the items to delete.
 * @param delete_count The number of item identifiers in the delete_ids array.
 */
PIL_API(void)
TableDeleteIds
(
    struct TABLE_DESC *table, 
    HANDLE_BITS  *delete_ids, 
    uint32_t    delete_count
);

/* @summary Remove a single external table item identifier inserted previously by TableInsertId.
 * The corresponding table data should have already had any necessary cleanup performed prior to calling this function.
 * The caller is responsible for ensuring that the value supplied for bits represents a valid table entry.
 * @param table Pointer to a TABLE_DESC describing the table from which the item will be removed.
 * @param bits The HANDLE_BITS identifying the item to remove.
 * @return The handle of the item that was moved as the result of the removal, or HANDLE_BITS_INVALID if no item was moved.
 */
PIL_API(HANDLE_BITS)
TableRemoveId
(
    struct TABLE_DESC *table, 
    HANDLE_BITS         bits
);

/* @summary Resolve a table item identifier into an array index that can be used with the TableData_GetElementPointer macro.
 * @param o_record_index Pointer to the location to update with the index value to pass to TableData_GetElementPointer.
 * @param table Pointer to a TABLE_DESC describing the table in which the lookup will be performed.
 * @param bits The HANDLE_BITS identifying the item to resolve.
 * @return Non-zero if the item is successfully resolved and o_record_index is updated with the item index value.
 */
PIL_API(int)
TableResolve
(
    uint32_t *o_record_index, 
    struct TABLE_DESC *table, 
    HANDLE_BITS         bits
);

/* @summary Create a single table item identifier.
 * The caller is responsible for ensuring the table has sufficient committed capacity using the TableEnsure function.
 * @param o_record_index Pointer to a location to update with the index value to pass to TableData_GetElementPointer.
 * @param table Pointer to the TABLE_DESC describing the table that owns the item identifier.
 * @return The table item identifier, or HANDLE_BITS_INVALID if the item cannot be created.
 */
PIL_API(HANDLE_BITS)
TableCreateId
(
    uint32_t *o_record_index, 
    struct TABLE_DESC *table
);

/* @summary Insert an existing ID, generated by TableCreateId on a different table, into a data table.
 * The caller is responsible for ensuring the table has sufficient committed capacity using the TableEnsure function.
 * @param o_record_index Pointer to a location to update with the index value to pass to TableData_GetElementPointer.
 * @param table Pointer to the TABLE_DESC describing the table into which the key will be inserted.
 * @param bits The HANDLE_BITS representing the externally-created table item identifier.
 * @return Zero if the item is inserted successfully, or non-zero if the item identifier cannot be inserted.
 */
PIL_API(int)
TableInsertId
(
    uint32_t *o_record_index, 
    struct TABLE_DESC *table, 
    HANDLE_BITS         bits
);

/* @summary Construct a HANDLE_BITS from its constituient parts.
 * @param sparse_index The zero-based index within the sparse portion of the TABLE_INDEX that is allocated to the item.
 * @param generation The generation value of the data slot allocated to the item.
 * @return The HANDLE_BITS identifying the item.
 */
PIL_API(HANDLE_BITS)
MakeHandleBits
(
    uint32_t sparse_index, 
    uint32_t   generation
);

/* @summary Extract whether or not a HANDLE_BITS represents a possibly-valid item.
 * @param bits The HANDLE_BITS value.
 * @return Non-zero if the HANDLE_BITS identifies an item that was valid at some point.
 */
PIL_API(uint32_t)
HandleBitsExtractLive
(
    HANDLE_BITS bits
);

/* @summary Extract the generation value of the data slot associated with a HANDLE_BITS.
 * @param bits The HANDLE_BITS value.
 * @return The generation value portion of the handle.
 */
PIL_API(uint32_t)
HandleBitsExtractGeneration
(
    HANDLE_BITS bits
);

/* @summary Extract the sparse slot index encoded within a HANDLE_BITS.
 * @param bits The HANDLE_BITS value.
 * @return The zero-based index within the sparse portion of the TABLE_INDEX allocated to the item.
 */
PIL_API(uint32_t)
HandleBitsExtractSparseIndex
(
    HANDLE_BITS bits
);

/* @summary Extract a value indicating whether or not a sparse index slot represents a valid item.
 * @param index_value The word read from the sparse index.
 * @return Non-zero if the slot is associated with a valid item.
 */
PIL_API(uint32_t)
SparseIndexExtractLive
(
    uint32_t index_value
);

/* @summary Extract the generation value of the data slot associated with a sparse index slot.
 * @param index_value The word read from the sparse index.
 * @return The generation value portion of the index value.
 */
PIL_API(uint32_t)
SparseIndexExtractGeneration
(
    uint32_t index_value
);

/* @summary Extract the dense array index encoded within a sparse index slot.
 * @param index_value The word read from the sparse index.
 * @return The dense index portion of the index value.
 */
PIL_API(uint32_t)
SparseIndexExtractDenseIndex
(
    uint32_t index_value
);

/* @summary Perform an internal self-consistency check on a TABLE_INDEX structure.
 * Intended to be used for debugging purposes only. Asserts fire for validation errors in debug builds.
 * @param index The TABLE_INDEX to validate.
 * @return Non-zero if the index is valid, or zero if the index is not valid.
 */
PIL_API(int)
VerifyTableIndex
(
    struct TABLE_INDEX *index
);

/* @summary Given a pointer to a particular data element within a TABLE_DATA buffer, retrieve the corresponding dense array index of the element.
 * @param table_data The TABLE_DATA from which element_ptr was obtained.
 * @param element_ptr The address of the element within the TABLE_DATA buffer.
 * @return The zero-based index of the element within the table data.
 */
PIL_API(uint32_t)
TableData_GetElementIndex
(
    struct TABLE_DATA *table_data, 
    void             *element_ptr
);

#ifdef __cplusplus
}; /* extern "C" */
#endif

#endif /* __PIL_TABLE_H__ */

