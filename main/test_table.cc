#include <stdio.h>
#include "pil.h"
#include "table.h"

#define CONTAINER_ITEM_STREAM_INDEX    0

#define Container_GetCount(_t)                                                 \
    Table_GetCount(&(_t)->TableDesc)

#define Container_GetCapacity(_t)                                              \
    Table_GetCapacity(&(_t)->TableDesc)

#define Container_HandleBegin(_t)                                              \
    Table_GetHandleBegin(&(_t)->TableDesc)

#define Container_HandleEnd(_t)                                                \
    Table_GetHandleEnd(&(_t)->TableDesc)

#define Container_HandleAt(_t, _i)                                             \
    Table_GetHandle(&(_t)->TableDesc, _i)

#define Container_ItemStreamBegin(_t)                                          \
    Table_GetStreamBegin(ITEM, &(_t)->TableDesc, CONTAINER_ITEM_STREAM_INDEX)

#define Container_ItemStreamEnd(_t)                                            \
    Table_GetStreamEnd(ITEM, &(_t)->TableDesc, CONTAINER_ITEM_STREAM_INDEX)

#define Container_ItemStreamAt(_t, _i)                                         \
    Table_GetStreamElement(ITEM, &(_t)->TableDesc, CONTAINER_ITEM_STREAM_INDEX, _i)

typedef struct ITEM {
    int            Value;
} ITEM;

typedef struct CONTAINER {
    TABLE_DESC     TableDesc;
    TABLE_INDEX    TableIndex;
    TABLE_DATA     ItemData;
    TABLE_DATA    *TableStreams[1];
} CONTAINER;

static void
DebugHandleBits
(
    HANDLE_BITS bits
)
{
    printf("%u|", HandleBitsExtractLive(bits));
    printf("%07u|", HandleBitsExtractSparseIndex(bits));
    printf("%02u\n", HandleBitsExtractGeneration(bits));
}

static void
CreateContainer
(
    CONTAINER *c, 
    uint32_t num
)
{
    uint32_t const          stream_count = 1;
    uint32_t                      result = 0;
    TABLE_INIT                table_init = {};
    TABLE_DATA_STREAM_DESC table_data[1] = {
        { &c->ItemData, sizeof(ITEM) }
    };

    table_init.Index         = &c->TableIndex;
    table_init.Streams       = table_data;
    table_init.StreamCount   = stream_count;
    table_init.TableCapacity = num;
    table_init.InitialCommit = num;
    if ((result = TableCreate(&table_init)) != 0) {
        assert(0 && "TableCreate failed");
        return;
    }
    c->TableStreams[0]       = &c->ItemData;
    c->TableDesc.Index       = &c->TableIndex;
    c->TableDesc.Streams     = c->TableStreams;
    c->TableDesc.StreamCount = stream_count;
}

static void
DeleteContainer
(
    CONTAINER *c
)
{
    TableDelete(&c->TableDesc);
}

static HANDLE_BITS
ContainerPush
(
    CONTAINER *c, 
    int    value
)
{
    ITEM       *item;
    HANDLE_BITS bits;
    uint32_t       i;
    if((bits = TableCreateId(&i, &c->TableDesc)) != HANDLE_BITS_INVALID) {
        item = Container_ItemStreamAt(c, i);
        item->Value = value;
    } return bits;
}

static ITEM*
ContainerLookUp
(
    CONTAINER   *c, 
    HANDLE_BITS id
)
{
    uint32_t i;
    if (TableResolve(&i, &c->TableDesc, id) != 0) {
        return Container_ItemStreamAt(c, i);
    } else {
        return nullptr;
    }
}

static int
ContainerDel1
(
    CONTAINER   *c, 
    HANDLE_BITS id
)
{
    HANDLE_BITS moved = HANDLE_BITS_INVALID;
    ITEM        *item = ContainerLookUp(c, id);
    int         value = -1;
    if (item) {
        value = item->Value; /* must access BEFORE calling TableDeleteId */
        moved = TableDeleteId(&c->TableDesc, id); (void) moved;
        return value;
    } else {
        return -1;
    }
}

static void
ContainerDelN
(
    CONTAINER     *c, 
    HANDLE_BITS *ids,
    uint32_t       n
)
{
    TableDeleteIds(&c->TableDesc, ids, n);
}

static void
PrintContainer
(
    CONTAINER *c
)
{
    HANDLE_BITS *hand_itr = Container_HandleBegin(c);
    HANDLE_BITS *hand_end = Container_HandleEnd(c);
    ITEM        *item_itr = Container_ItemStreamBegin(c);
    ITEM        *item_end = Container_ItemStreamEnd(c);
    uint32_t          num = Container_GetCount(c);

    printf("CONTAINER CONTENTS (%u items):\n", num);
    
    printf("HAND: [");
    while (hand_itr != hand_end) {
        printf("%08X", *hand_itr);
        if ((hand_itr + 1) != hand_end) {
            printf(", ");
        } hand_itr++;
    }
    printf("]\n");

    printf("STR0: [");
    while (item_itr != item_end) {
        printf("%08u", item_itr->Value);
        if ((item_itr + 1) != item_end) {
            printf(", ");
        } item_itr++;
    }
    printf("]\n");
    printf("\n");
}

static int
Test_Generation
(
    void
)
{   /* create and delete one item at a time so that the item gets created in the same slot.
     * ensure that the generation portion of the handle is set correctly. */
    int     res = 1;
    uint32_t  i;
    CONTAINER c;

    CreateContainer(&c, 4);
    for (i = 0; i < HANDLE_GENER_MASK+1; ++i) {
        HANDLE_BITS h = ContainerPush(&c,  i);
        if (HandleBitsExtractGeneration(h) != (i & HANDLE_GENER_MASK)) {
            assert(HandleBitsExtractGeneration(h) == (i & HANDLE_GENER_MASK));
            res  = 0; goto end;
        } ContainerDel1(&c, h);
    } res = VerifyTableIndex(&c.TableIndex);

end:
    DeleteContainer(&c);
    return res;
}

static int
Test_FullStateValidationOne
(
    void
)
{   /* allocate items, one by one, validating after each allocation.
     * then delete each item, one at a time, and validate after each deletion. */
#   define C    1024
    HANDLE_BITS *handles =(HANDLE_BITS*) malloc(C * sizeof(HANDLE_BITS));
    int              res = 1;
    CONTAINER          c;
    int             i, j;

    CreateContainer(&c, C);
    for (j = 0; j < 64; ++j) {
        for (i = 0; i < C; ++i) { /* allocate one at a time */
            if ((handles[i] = ContainerPush(&c, i)) == HANDLE_BITS_INVALID) {
                assert(handles[i] != HANDLE_BITS_INVALID);
                res  = 0; goto end;
            }
            if (VerifyTableIndex(&c.TableIndex) == 0) {
                assert(0 && "Table index verification failed");
                res  = 0; goto end;
            }
        }
        for (i = 0; i < C; ++i) { /* delete each even-indexed item */
            if ((i & 1) == 0) {
                if (ContainerDel1(&c, handles[i]) != i) {
                    assert(0 && "ContainerDel1(&c, handles[i]) == i");
                    res  = 0; goto end;
                }
                if (VerifyTableIndex(&c.TableIndex) == 0) {
                    assert(0 && "Table index verification failed (delete even)");
                    res  = 0; goto end;
                }
            }
        }
        for (i = 0; i < C; ++i) { /* delete each odd-indexed item */
            if ((i & 1) == 1) {
                if (ContainerDel1(&c, handles[i]) != i) {
                    assert(0 && "ContainerDel1(&c, handles[i]) == i");
                    res  = 0; goto end;
                }
                if (VerifyTableIndex(&c.TableIndex) == 0) {
                    assert(0 && "Table index verification failed (delete even)");
                    res  = 0; goto end;
                }
            }
        }
    }

end:
    DeleteContainer(&c);
    free(handles);
    return res;
#   undef  C
}

static int
Test_FullStateValidationMany
(
    void
)
{   /* allocate items, one by one, validating after each allocation.
     * then delete each item, one at a time, and validate after each deletion. */
#   define C    1024
    HANDLE_BITS *handles =(HANDLE_BITS*) malloc(C * sizeof(HANDLE_BITS));
    int              res = 1;
    CONTAINER          c;
    int             i, j;

    assert(C%4 == 0);

    CreateContainer(&c, C);
    for (j = 0; j < 64; ++j) {
        for (i = 0; i < C; ++i) { /* allocate one at a time */
            if ((handles[i] = ContainerPush(&c, i)) == HANDLE_BITS_INVALID) {
                assert(handles[i] != HANDLE_BITS_INVALID);
                res  = 0; goto end;
            }
            if (VerifyTableIndex(&c.TableIndex) == 0) {
                assert(0 && "Table index verification failed");
                res  = 0; goto end;
            }
        }
        for (i = 0; i < 4; ++i) { /* delete C/4 items at a time */
            ContainerDelN(&c, &handles[i * (C/4)], C/4);
            if (VerifyTableIndex(&c.TableIndex) == 0) {
                assert(0 && "Table index verification failed (many)");
                res  = 0; goto end;
            }
        }
    }

end:
    DeleteContainer(&c);
    free(handles);
    return res;
#   undef  C
}

int main
(
    int    argc, 
    char **argv
)
{
    (void) argc;
    (void) argv;

    Test_Generation();
    Test_FullStateValidationOne();
    Test_FullStateValidationMany();

    return 0;
}

