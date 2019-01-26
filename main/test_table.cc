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

int main
(
    int    argc, 
    char **argv
)
{
    CONTAINER        c;
    HANDLE_BITS ids[4];
    HANDLE_BITS del[2];

    (void) argc;
    (void) argv;

    CreateContainer(&c, 4);
    ids[0] = ContainerPush(&c, 1);
    ids[1] = ContainerPush(&c, 2);
    ids[2] = ContainerPush(&c, 3);
    ids[3] = ContainerPush(&c, 4);
    PrintContainer(&c);
    ContainerDel1(&c, ids[1]);
    PrintContainer(&c);
    del[0] = ids[2];
    del[1] = ids[0];
    ContainerDelN(&c, del, 2);
    PrintContainer(&c);
    DeleteContainer(&c);
    return 0;
}

