/**
 * @summary display_device_d3d12.cc: Implement the D3D12DeviceManager class and 
 * any internal helper functions for dealing with device-related functionality.
 */
#include "display.h"
#include "win32/display_system_d3d12.h"

/* @summary Convert a LUID (Locally-Unique IDentifier) to a 64-bit integer value.
 * @param luid The input value.
 * @return The LUID, represented as a 64-bit unsigned integer.
 */
static PIL_INLINE uint64_t
LuidToUint64
(
    LUID luid
)
{
    return (((uint64_t) luid.HighPart) << 32) | ((uint64_t) luid.LowPart);
}

/* @summary Convert a Win32 HMONITOR (monitor handle) to a 64-bit integer value.
 * @param monitor The Win32 monitor handle.
 * @return The HMONITOR, cast to a 64-bit unsigned integer.
 */
static PIL_INLINE uint64_t
HMONITORToUint64
(
    HMONITOR monitor
)
{
    return ((uint64_t) monitor);
}

static void
ReleaseDeviceInterfaces
(
    struct DISPLAY_DEVICE_TABLE_D3D12 *table
)
{
    DISPLAY_DEVICE_D3D12   *device_itr = D3D12_DeviceTableDeviceStreamBegin(table);
    DISPLAY_DEVICE_D3D12   *device_end = D3D12_DeviceTableDeviceStreamEnd  (table);
    ID3D12Device         *d3d12_device;
    ID3D12CommandQueue  *compute_queue;
    ID3D12CommandQueue *graphics_queue;
    ID3D12CommandQueue *transfer_queue;
    while (device_itr != device_end) {
        /* release retained COM interfaces */
        if ((transfer_queue = device_itr->TransferCommandQueue) != nullptr) {
            transfer_queue->Release();
        }
        if ((graphics_queue = device_itr->GraphicsCommandQueue) != nullptr) {
            graphics_queue->Release();
        }
        if ((compute_queue  = device_itr->ComputeCommandQueue ) != nullptr) {
            compute_queue->Release();
        }
        if ((d3d12_device   = device_itr->D3D12Device) != nullptr) {
            d3d12_device->Release();
        }
        device_itr->TransferCommandQueue = nullptr;
        device_itr->GraphicsCommandQueue = nullptr;
        device_itr->ComputeCommandQueue  = nullptr;
        device_itr->D3D12Device          = nullptr;
        /* ... */
        device_itr->PhysicalDeviceId = 0;
        device_itr->ReferenceCount = 0;
        device_itr++;
    }
}

static void
EnumerateAdapterOutputs
(
    struct DISPLAY_OUTPUT_TABLE_D3D12 *table, 
    uint32_t                  *o_num_outputs, 
    IDXGIAdapter1                   *adapter
)
{
    IDXGIOutput               *output = nullptr;
    HRESULT                    result = S_OK;
    DISPLAY_OUTPUT_D3D12 *output_list = D3D12_OutputTableStreamBegin(table);
    uint64_t             *output_keys = D3D12_OutputTableKeysBegin(table);
    uint32_t const        max_outputs = D3D12_OutputTableCapacity(table);
    uint32_t              total_count = D3D12_OutputTableCount(table);
    UINT                        index;
    uint32_t                    count;
    uint64_t                      key;
    DISPLAY_OUTPUT_D3D12        *item;
    DXGI_OUTPUT_DESC             desc;

    for (index = 0, count = 0; /* empty */; ++index, ++count) {
        if (SUCCEEDED((result = adapter->EnumOutputs(index, &output)))) {
            if (SUCCEEDED(output->GetDesc(&desc))) {
                key = HMONITORToUint64(desc.Monitor);
                if ((item = D3D12_ResolveOutputForKey(table, key)) == nullptr) {
                    if (total_count != max_outputs) {
                        /* populate the new entry */
                        item = &output_list[total_count];
                        item->DxgiOutput  = output;
                        item->OutputDesc  = desc;
                        /* make the new entry visible */
                        output_keys[total_count++] = key;
                    } else { /* increase _D3D12_MAX_OUTPUTS */
                        assert(total_count < max_outputs);
                        if (o_num_outputs)*o_num_outputs = count;
                        break;
                    }
                }
            }
        } else if (result == DXGI_ERROR_NOT_FOUND) { /* no more results */
            if (o_num_outputs) *o_num_outputs = count;
            break;
        } else { /* an actual error occurred */
            if (o_num_outputs) *o_num_outputs = count;
            break;
        }
    } table->OutputCount = total_count;
}

PIL_API(int)
D3D12_CreateDisplayAdapterTable
(
    struct DISPLAY_ADAPTER_TABLE_D3D12 *table, 
    uint32_t                         capacity
)
{
    uint64_t              *keys = nullptr;
    DISPLAY_ADAPTER_D3D12 *list = nullptr;

    if ((keys = (uint64_t *) malloc(capacity * sizeof(uint64_t))) == nullptr) {
        goto cleanup_and_fail;
    }
    if ((list = (DISPLAY_ADAPTER_D3D12 *) malloc(capacity * sizeof(DISPLAY_ADAPTER_D3D12))) == nullptr) {
        goto cleanup_and_fail;
    }
    memset(keys, 0, capacity * sizeof(uint64_t));
    memset(list, 0, capacity * sizeof(DISPLAY_ADAPTER_D3D12));
    table->AdapterCount      = 0;
    table->AdapterCapacity   = capacity;
    table->AdapterKeys       = keys;
    table->AdapterList       = list;
    return 0;

cleanup_and_fail:
    if (list) {
        free(list);
    }
    if (keys) {
        free(keys);
    } memset(table, 0, sizeof(DISPLAY_ADAPTER_TABLE_D3D12));
    return -1;
}

PIL_API(void)
D3D12_DeleteDisplayAdapterTable
(
    struct DISPLAY_ADAPTER_TABLE_D3D12 *table
)
{
    uint64_t               *keys = table->AdapterKeys;
    DISPLAY_ADAPTER_D3D12  *list = table->AdapterList;
    D3D12_ClearDisplayAdapterTable(table);
    if (list) {
        free(list);
    }
    if (keys) {
        free(keys);
    } memset(table, 0, sizeof(DISPLAY_ADAPTER_TABLE_D3D12));
}

PIL_API(void)
D3D12_ClearDisplayAdapterTable
(
    struct DISPLAY_ADAPTER_TABLE_D3D12 *table
)
{
    DISPLAY_ADAPTER_D3D12 *list_itr = D3D12_AdapterTableStreamBegin(table);
    DISPLAY_ADAPTER_D3D12 *list_end = D3D12_AdapterTableStreamEnd(table);
    IDXGIAdapter1          *adapter = nullptr;
    while (list_itr != list_end) {
        if ((adapter = list_itr->DxgiAdapter1) != nullptr) {
            list_itr->DxgiAdapter1 = nullptr;
            adapter->Release();
        } /* ... */
        list_itr++;
    } table->AdapterCount = 0;
}

PIL_API(int)
D3D12_CreateDisplayOutputTable
(
    struct DISPLAY_OUTPUT_TABLE_D3D12 *table, 
    uint32_t                        capacity
)
{
    uint64_t             *keys = nullptr;
    DISPLAY_OUTPUT_D3D12 *list = nullptr;

    if ((keys = (uint64_t*) malloc(capacity * sizeof(uint64_t))) == nullptr) {
        goto cleanup_and_fail;
    }
    if ((list = (DISPLAY_OUTPUT_D3D12*) malloc(capacity * sizeof(DISPLAY_OUTPUT_D3D12))) == nullptr) {
        goto cleanup_and_fail;
    }
    memset(keys, 0, capacity * sizeof(uint64_t));
    memset(list, 0, capacity * sizeof(DISPLAY_OUTPUT_D3D12));
    table->OutputCount       = 0;
    table->OutputCapacity    = capacity;
    table->OutputKeys        = keys;
    table->OutputList        = list;
    return 0;

cleanup_and_fail:
    if (list) {
        free(list);
    }
    if (keys) {
        free(keys);
    } memset(table, 0, sizeof(DISPLAY_OUTPUT_TABLE_D3D12));
    return -1;
}

PIL_API(void)
D3D12_DeleteDisplayOutputTable
(
    struct DISPLAY_OUTPUT_TABLE_D3D12 *table
)
{
    uint64_t              *keys = table->OutputKeys;
    DISPLAY_OUTPUT_D3D12  *list = table->OutputList;
    D3D12_ClearDisplayOutputTable(table);
    if (list) {
        free(list);
    }
    if (keys) {
        free(keys);
    } memset(table, 0, sizeof(DISPLAY_OUTPUT_TABLE_D3D12));
}

PIL_API(void)
D3D12_ClearDisplayOutputTable
(
    struct DISPLAY_OUTPUT_TABLE_D3D12 *table
)
{
    DISPLAY_OUTPUT_D3D12 *list_itr = D3D12_OutputTableStreamBegin(table);
    DISPLAY_OUTPUT_D3D12 *list_end = D3D12_OutputTableStreamEnd(table);
    IDXGIOutput            *output = nullptr;
    while (list_itr != list_end) {
        if ((output  = list_itr->DxgiOutput) != nullptr) {
            list_itr->DxgiOutput = nullptr;
            output->Release();
        } /* ... */
        list_itr++;
    } table->OutputCount = 0;
}

PIL_API(int)
D3D12_CreateDisplayDeviceTable
(
    struct DISPLAY_DEVICE_TABLE_D3D12 *table, 
    uint32_t                        capacity
)
{
    uint32_t const          stream_count = 1;
    uint32_t                      result = 0;
    TABLE_INIT                table_init = {};
    TABLE_DATA_STREAM_DESC table_data[1] = {
        { &table->DeviceData, sizeof(DISPLAY_DEVICE_D3D12) }
    };

    table_init.Index         =&table->TableIndex;
    table_init.Streams       = table_data;
    table_init.StreamCount   = stream_count;
    table_init.TableCapacity = capacity;
    table_init.InitialCommit = capacity;
    if ((result = TableCreate(&table_init)) != 0) {
        return result;
    }
    table->TableStreams[0]       =&table->DeviceData;
    table->TableDesc.Index       =&table->TableIndex;
    table->TableDesc.Streams     = table->TableStreams;
    table->TableDesc.StreamCount = stream_count;
    return 0;
}

PIL_API(void)
D3D12_DeleteDisplayDeviceTable
(
    struct DISPLAY_DEVICE_TABLE_D3D12 *table
)
{
    ReleaseDeviceInterfaces(table);
    TableDelete(&table->TableDesc);
}

PIL_API(struct DISPLAY_ADAPTER_D3D12*)
D3D12_ResolveAdapterForKey
(
    struct DISPLAY_ADAPTER_TABLE_D3D12 *table, 
    uint64_t                      adapter_key
)
{
    uint64_t const *keys_itr = D3D12_AdapterTableKeysBegin(table);
    uint64_t const *keys_end = D3D12_AdapterTableKeysEnd(table);
    uint32_t           index = 0;
    while (keys_itr != keys_end) {
        if (*keys_itr != adapter_key) {
            keys_itr++; index++;
        } else {
            return D3D12_AdapterTableStreamAt(table, index);
        }
    } return nullptr;
}

PIL_API(struct DISPLAY_OUTPUT_D3D12*)
D3D12_ResolveOutputForKey
(
    struct DISPLAY_OUTPUT_TABLE_D3D12 *table, 
    uint64_t                      output_key
)
{
    uint64_t const *keys_itr = D3D12_OutputTableKeysBegin(table);
    uint64_t const *keys_end = D3D12_OutputTableKeysEnd(table);
    uint32_t           index = 0;
    while (keys_itr != keys_end) {
        if (*keys_itr != output_key) {
            keys_itr++; index++;
        } else {
            return D3D12_OutputTableStreamAt(table, index);
        }
    } return nullptr;
}

PIL_API(struct DISPLAY_OUTPUT_D3D12*)
D3D12_ResolveOutputForHMONITOR
(
    struct DISPLAY_OUTPUT_TABLE_D3D12 *table, 
    HMONITOR                         monitor
)
{
    return D3D12_ResolveOutputForKey(table, HMONITORToUint64(monitor));
}

PIL_API(struct DISPLAY_DEVICE_D3D12*)
D3D12_ResolveDeviceForHandle
(
    struct DISPLAY_DEVICE_TABLE_D3D12 *table, 
    GPU_DEVICE_HANDLE                 handle
)
{
    uint32_t record;
    if (TableResolve(&record, &table->TableDesc, handle.Bits)) {
        return D3D12_DeviceTableDeviceStreamAt(table, record);
    } else {
        return nullptr;
    }
}

PIL_API(uint32_t)
D3D12_DescribeDisplayAdapters
(
    struct DISPLAY_ADAPTER_TABLE_D3D12 *table, 
    struct DISPLAY_ADAPTER_DESC      *results, 
    uint32_t                      start_index, 
    uint32_t                      max_results
)
{
    uint64_t const        *keys_itr = D3D12_AdapterTableKeysAt(table, start_index);
    DISPLAY_ADAPTER_D3D12 *list_itr = D3D12_AdapterTableStreamAt(table, start_index);
    DISPLAY_ADAPTER_D3D12 *list_end = D3D12_AdapterTableStreamEnd(table);
    DISPLAY_ADAPTER_DESC  *dest_itr = results;
    DISPLAY_ADAPTER_DESC  *dest_end = results + max_results;
    uint32_t                      n = 0;

    if (results == nullptr || max_results == 0) {
        return D3D12_AdapterTableCount(table);
    }
    while (list_itr < list_end && dest_itr < dest_end) {
        dest_itr->Identifier        = *keys_itr;
        dest_itr->PerformanceLevel  = 2; /* TODO: somehow compute this */
        dest_itr++; list_itr++; keys_itr++; n++;
    } return n;
}

PIL_API(uint32_t)
D3D12_DescribeDisplayOutputs
(
    struct DISPLAY_OUTPUT_TABLE_D3D12 *table, 
    struct DISPLAY_OUTPUT_DESC      *results, 
    uint32_t                     start_index, 
    uint32_t                     max_results, 
    struct WIN32API_DISPATCH         *winapi
)
{
    uint64_t const       *keys_itr = D3D12_OutputTableKeysAt(table, start_index);
    DISPLAY_OUTPUT_D3D12 *list_itr = D3D12_OutputTableStreamAt(table, start_index);
    DISPLAY_OUTPUT_D3D12 *list_end = D3D12_OutputTableStreamEnd(table);
    DISPLAY_OUTPUT_DESC  *dest_itr = results;
    DISPLAY_OUTPUT_DESC  *dest_end = results + max_results;
    uint32_t                     n = 0;
    UINT              dpi_x, dpi_y;
    HMONITOR               monitor;
    MONITORINFO            moninfo;

    if (results == nullptr || max_results == 0) {
        return D3D12_OutputTableCount(table);
    } else {
        moninfo.cbSize = sizeof(MONITORINFO);
    }
    while (list_itr < list_end && dest_itr < dest_end) {
        monitor = list_itr->OutputDesc.Monitor;
        GetMonitorInfo(monitor, &moninfo);
        winapi->GetDpiForMonitor(monitor, MDT_EFFECTIVE_DPI, &dpi_x, &dpi_y);

        dest_itr->Identifier    =*keys_itr;
        dest_itr->VirtualX      =(int32_t ) moninfo.rcMonitor.left;
        dest_itr->VirtualY      =(int32_t ) moninfo.rcMonitor.top;
        dest_itr->ActualWidth   =(uint32_t)(moninfo.rcMonitor.right - moninfo.rcMonitor.left);
        dest_itr->ActualHeight  =(uint32_t)(moninfo.rcMonitor.bottom - moninfo.rcMonitor.top);
        dest_itr->WorkingWidth  =(uint32_t)(moninfo.rcWork.right - moninfo.rcWork.left);
        dest_itr->WorkingHeight =(uint32_t)(moninfo.rcWork.bottom - moninfo.rcWork.top);
        dest_itr->DisplayDpiX   = dpi_x;
        dest_itr->DisplayDpiY   = dpi_y;
        dest_itr++; list_itr++; keys_itr++; n++;
    } return n;
}

PIL_API(int)
D3D12_RefreshDisplayHardware
(
    struct DISPLAY_ADAPTER_TABLE_D3D12 *adapters, 
    struct DISPLAY_OUTPUT_TABLE_D3D12   *outputs, 
    uint32_t                     *o_num_adapters, 
    uint32_t                      *o_num_outputs, 
    IDXGIFactory2                       *factory
)
{
    IDXGIAdapter1              *adapter = NULL;
    HRESULT                      result = S_OK;
    DISPLAY_ADAPTER_D3D12 *adapter_list = D3D12_AdapterTableStreamBegin(adapters);
    uint64_t              *adapter_keys = D3D12_AdapterTableKeysBegin(adapters);
    uint32_t const         max_adapters = D3D12_AdapterTableCapacity(adapters);
    uint32_t                total_count = 0;
    uint32_t                   hw_count = 0;
    UINT                          index;
    uint32_t                      count;
    uint64_t                        key;
    DISPLAY_ADAPTER_D3D12         *item;
    DXGI_ADAPTER_DESC1             desc;

    D3D12_ClearDisplayOutputTable (outputs);
    D3D12_ClearDisplayAdapterTable(adapters);
    if (o_num_adapters) {
       *o_num_adapters = 0;
    }
    if (o_num_outputs) {
       *o_num_outputs = 0;
    }

    for (index = 0, count = 0; /* empty */; ++index, ++count) {
        if (SUCCEEDED((result = factory->EnumAdapters1(index, &adapter)))) {
            if (SUCCEEDED(adapter->GetDesc1(&desc))) {
                key = LuidToUint64(desc.AdapterLuid);
                if ((item = D3D12_ResolveAdapterForKey(adapters, key)) == nullptr) {
                    if (total_count != max_adapters) {
                        /* populate the new entry */
                        item = &adapter_list[total_count];
                        item->DxgiAdapter1 = adapter;
                        item->AdapterDesc1 = desc;
                        item->OutputCount  = 0;
                        EnumerateAdapterOutputs(outputs, &item->OutputCount, adapter);
                        /* make the new entry visible */
                        adapter_keys[total_count++] = key;
                        /* determine whether this is a hardware adapter */
                        if ((desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0) {
                            /* filter our the WARP adapter */
                            if (CompareStringEx(LOCALE_NAME_INVARIANT, NORM_IGNORECASE, desc.Description, -1, L"Microsoft Basic Display Adapter", -1, NULL, NULL, 0) != CSTR_EQUAL) {
                                hw_count++;
                            }
                        }
                    } else { /* increase _D3D12_MAX_ADAPTERS */
                        assert(total_count < max_adapters);
                        break;
                    }
                }
            }
        } else if (result == DXGI_ERROR_NOT_FOUND) { /* no more results */
            break;
        } else { /* an actual error occurred */
            break;
        }
    } adapters->AdapterCount = total_count;

    if (o_num_adapters) *o_num_adapters = D3D12_AdapterTableCount(adapters);
    if (o_num_outputs ) *o_num_outputs  = D3D12_OutputTableCount(outputs);
    if (hw_count > 0  )  return 0;
    else return -1;
}

PIL_API(int)
D3D12_CreateLogicalGpuDevice
(
    struct DISPLAY_DEVICE_TABLE_D3D12 *table, 
    struct GPU_DEVICE_INIT_D3D12       *init, 
    struct GPU_DEVICE_HANDLE       *o_handle
)
{
    ID3D12Device                 *d3d12_device = nullptr;
    ID3D12CommandQueue         *graphics_queue = nullptr;
    ID3D12CommandQueue         *transfer_queue = nullptr;
    ID3D12CommandQueue          *compute_queue = nullptr;
    DISPLAY_ADAPTER_D3D12             *adapter = nullptr;
    DISPLAY_DEVICE_D3D12           *device_itr = D3D12_DeviceTableDeviceStreamBegin(table);
    DISPLAY_DEVICE_D3D12           *device_end = D3D12_DeviceTableDeviceStreamEnd(table);
    uint32_t                       max_devices = D3D12_DeviceTableCapacity(table);
    uint32_t                       num_devices = D3D12_DeviceTableCount(table);
    HANDLE_BITS                           bits = HANDLE_BITS_INVALID;
    D3D12API_DISPATCH                   *d3d12 = init->D3D12Dispatch;
    DISPLAY_ADAPTER_TABLE_D3D12 *adapter_table = init->AdapterTable;
    uint32_t                       usage_flags = init->UsageFlags;
    uint64_t                       adapter_key = init->TargetAdapter;
    HRESULT                             result = S_OK;
    uint32_t                            record = 0;
    D3D12_COMMAND_QUEUE_DESC        queue_desc = {};
    D3D_FEATURE_LEVEL            feature_level = D3D_FEATURE_LEVEL_11_0;
    D3D_FEATURE_LEVEL           feature_list[] = {
        D3D_FEATURE_LEVEL_12_1, 
        D3D_FEATURE_LEVEL_12_0, 
        D3D_FEATURE_LEVEL_11_1, 
        D3D_FEATURE_LEVEL_11_0
    };
    uint32_t const         FEATURE_LEVEL_COUNT = D3D12_CountOf(feature_list);

    /* check for an existing device */
    while (device_itr != device_end) {
        if (device_itr->D3D12Device != nullptr && 
            device_itr->PhysicalDeviceId == adapter_key && 
           (device_itr->UsageFlags & usage_flags) == usage_flags) {
            device_itr->ReferenceCount++;
            bits = D3D12_DeviceTableHandleAt(table, record);
            if (o_handle) {
               *o_handle = GPU_DEVICE_HANDLE{bits};
            }
            return 0;
        } device_itr++; record++;
    }

    /* create a new logical device */
    if (num_devices == max_devices) {
        assert(num_devices < max_devices);
        goto cleanup_and_fail;
    }
    if ((adapter = D3D12_ResolveAdapterForKey(adapter_table, adapter_key)) == nullptr) {
        assert(adapter != nullptr && "Invalid GPU_DEVICE_INIT::TargetAdapter");
        goto cleanup_and_fail;
    }
    /* attempt to create a device with the highest possible feature level */
    for (uint32_t i = 0; i < FEATURE_LEVEL_COUNT; ++i) {
        if (SUCCEEDED((result = d3d12->D3D12CreateDevice(adapter->DxgiAdapter1, feature_list[i], IID_PPV_ARGS(&d3d12_device))))) {
            feature_level = feature_list[i];
            break;
        }
    }
    if (d3d12_device == nullptr) {
        goto cleanup_and_fail;
    }

    /* create command queues for graphics, compute and transfer */
    queue_desc.Type     = D3D12_COMMAND_LIST_TYPE_COMPUTE;
    queue_desc.Flags    = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queue_desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    queue_desc.NodeMask = 0; /* TODO: multi-adapter support? */
    if (FAILED((result  = d3d12_device->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&compute_queue))))) {
        goto cleanup_and_fail;
    }
    queue_desc.Type     = D3D12_COMMAND_LIST_TYPE_COPY;
    queue_desc.Flags    = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queue_desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    queue_desc.NodeMask = 0; /* TODO: multi-adapter support? */
    if (FAILED((result  = d3d12_device->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&transfer_queue))))) {
        goto cleanup_and_fail;
    }
    queue_desc.Type     = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queue_desc.Flags    = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queue_desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    queue_desc.NodeMask = 0; /* TODO: multi-adapter support? */
    if (FAILED((result  = d3d12_device->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&graphics_queue))))) {
        goto cleanup_and_fail;
    }

    /* initialize the system device handle */
    if ((bits = TableCreateId(&record, &table->TableDesc)) == HANDLE_BITS_INVALID) {
        goto cleanup_and_fail;
    }
    device_itr = D3D12_DeviceTableDeviceStreamAt(table, record);
    device_itr->D3D12Device          = d3d12_device;
    device_itr->ComputeCommandQueue  = compute_queue;
    device_itr->GraphicsCommandQueue = graphics_queue;
    device_itr->TransferCommandQueue = transfer_queue;
    device_itr->FeatureLevel         = feature_level;
    device_itr->UsageFlags           = usage_flags;
    device_itr->PhysicalDeviceId     = adapter_key;
    device_itr->ReferenceCount       = 1;
    if (o_handle) {
       *o_handle = GPU_DEVICE_HANDLE{bits};
    }
    return 0;

cleanup_and_fail:
    if (compute_queue) {
        compute_queue->Release();
    }
    if (graphics_queue) {
        graphics_queue->Release();
    }
    if (transfer_queue) {
        transfer_queue->Release();
    }
    if (d3d12_device) {
        d3d12_device->Release();
    }
    if (o_handle) {
       *o_handle = GPU_DEVICE_HANDLE{HANDLE_BITS_INVALID};
    }
    return -1;
}

PIL_API(void)
D3D12_DeleteLogicalGpuDevice
(
    struct DISPLAY_DEVICE_TABLE_D3D12 *table, 
    struct GPU_DEVICE_HANDLE          handle
)
{
    DISPLAY_DEVICE_D3D12       *device = D3D12_ResolveDeviceForHandle(table, handle);
    ID3D12Device         *d3d12_device;
    ID3D12CommandQueue  *compute_queue;
    ID3D12CommandQueue *graphics_queue;
    ID3D12CommandQueue *transfer_queue;
    if (device) {
        if (device->ReferenceCount == 1) {
            /* release retained COM interfaces */
            if ((transfer_queue = device->TransferCommandQueue) != nullptr) {
                device->TransferCommandQueue = nullptr;
                transfer_queue->Release();
            }
            if ((graphics_queue = device->GraphicsCommandQueue) != nullptr) {
                device->GraphicsCommandQueue = nullptr;
                graphics_queue->Release();
            }
            if ((compute_queue = device->ComputeCommandQueue) != nullptr) {
                device->ComputeCommandQueue = nullptr;
                compute_queue->Release();
            }
            if ((d3d12_device = device->D3D12Device) != nullptr) {
                device->D3D12Device = nullptr;
                d3d12_device->Release();
            }
            /* ... */
            device->PhysicalDeviceId = 0;
            device->ReferenceCount = 0;
            /* all cleanup is complete */
            TableDeleteId(&table->TableDesc, handle.Bits);
            /* handle, device are now invalid */
        } else { /* just dec refcount */
            device->ReferenceCount--;
        }
    }
}
