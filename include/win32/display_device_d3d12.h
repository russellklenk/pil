/**
 * @summary display_device_d3d12.h: Define the data structure and functions 
 * related to enumerating attached GPU devices and display outputs, and creating
 * logical GPU devices for use by an application.
 */
#ifndef __PIL_DISPLAY_DEVICE_D3D12_H__
#define __PIL_DISPLAY_DEVICE_D3D12_H__

#pragma once

struct WIN32API_DISPATCH;                                                      /* win32api_win32.h */
struct D3D12API_DISPATCH;                                                      /* d3d12api_win32.h */
struct DISPLAY_SYSTEM_D3D12;                                                   /* display_system_d3d12.h */
struct DISPLAY_OUTPUT_D3D12;
struct DISPLAY_DEVICE_D3D12;
struct DISPLAY_ADAPTER_D3D12;
struct DISPLAY_DEVICE_TABLE_D3D12;
struct DISPLAY_OUTPUT_TABLE_D3D12;
struct DISPLAY_ADAPTER_TABLE_D3D12;
struct GPU_DEVICE_INIT_D3D12;

/* @summary Define constants related to table access.
 * D3D12_DEVICE_STREAM_INDEX: The zero-based index of the data stream of DISPLAY_DEVICE_D3D12 records.
 */
#ifndef DISPLAY_DEVICE_D3D12_CONSTANTS
#   define DISPLAY_DEVICE_D3D12_CONSTANTS
#   define D3D12_DEVICE_STREAM_INDEX                                           0
#endif

/* @summary Retrieve the number of active logical GPU devices.
 * @param _t A pointer to a DISPLAY_DEVICE_TABLE_D3D12 instance.
 * @return The number of active logical GPU devices.
 */
#ifndef D3D12_DeviceTableCount
#define D3D12_DeviceTableCount(_t)                                             \
    Table_GetCount(&(_t)->TableDesc)
#endif

/* @summary Retrieve the maximum number of active logical GPU devices.
 * @param _t A pointer to a DISPLAY_DEVICE_TABLE_D3D12 instance.
 * @return The maximum number of active logical GPU devices.
 */
#ifndef D3D12_DeviceTableCapacity
#define D3D12_DeviceTableCapacity(_t)                                          \
    Table_GetCapacity(&(_t)->TableDesc)
#endif

/* @summary Retrieve a pointer to the first element in the active handle stream for the set of logical GPU devices.
 * @param _t A pointer to a DISPLAY_DEVICE_TABLE_D3D12 instance.
 * @return A pointer (HANDLE_BITS*) to the first active identifier in the table. If equal to the address returned by D3D12_DeviceTableDeviceHandleEnd(_t), the table is empty and the returned pointer must not be dereferenced.
 */
#ifndef D3D12_DeviceTableDeviceHandleBegin
#define D3D12_DeviceTableDeviceHandleBegin(_t)                                 \
    Table_GetHandleBegin(&(_t)->TableDesc)
#endif

/* @summary Retrieve a pointer to one-past the last element in the active handle stream for the set of logical GPU devices.
 * @param _t A pointer to a DISPLAY_DEVICE_TABLE_D3D12 instance.
 * @return A pointer (HANDLE_BITS*) to one-past the last active identifier. Do not dereference the returned pointer.
 */
#ifndef D3D12_DeviceTableDeviceHandleEnd
#define D3D12_DeviceTableDeviceHandleEnd(_t)                                   \
    Table_GetHandleEnd(&(_t)->TableDesc)
#endif

/* @summary Retrieve the _i'th handle of a logical GPU device.
 * @param _t A pointer to a DISPLAY_DEVICE_TABLE_D3D12 instance.
 * @param _i The zero-based index of the active logical GPU device handle to retrieve.
 * @return The _i'th device handle.
 */
#ifndef D3D12_DeviceTableHandleAt
#define D3D12_DeviceTableHandleAt(_t, _i)                                      \
    Table_GetHandle(&(_t)->TableDesc, _i)
#endif

/* @summary Retrieve a pointer to the first DISPLAY_DEVICE_D3D12 data record.
 * @param _t A pointer to a DISPLAY_DEVICE_TABLE_D3D12 instance.
 * @return A pointer to the first active DISPLAY_DEVICE_D3D12 record. If the returned pointer is the same as D3D12_DeviceTableDeviceStreamEnd, the table is empty and the returned pointer must not be dereferenced.
 */
#ifndef D3D12_DeviceTableDeviceStreamBegin
#define D3D12_DeviceTableDeviceStreamBegin(_t)                                 \
    Table_GetStreamBegin(DISPLAY_DEVICE_D3D12, &(_t)->TableDesc, D3D12_DEVICE_STREAM_INDEX)
#endif

/* @summary Retrieve a pointer to one-past the last DISPLAY_DEVICE_D3D12 data record.
 * @param _t A pointer to a DISPLAY_DEVICE_TABLE_D3D12 instance.
 * @return A pointer to one-past the last active DISPLAY_DEVICE_D3D12 data record. Do not dereference the returned pointer.
 */
#ifndef D3D12_DeviceTableDeviceStreamEnd
#define D3D12_DeviceTableDeviceStreamEnd(_t)                                   \
    Table_GetStreamEnd  (DISPLAY_DEVICE_D3D12, &(_t)->TableDesc, D3D12_DEVICE_STREAM_INDEX)
#endif

/* @summary Retrieve a pointer to the _i'th DISPLAY_DEVICE_D3D12 record.
 * @param _t A pointer to a DISPLAY_DEVICE_TABLE_D3D12 instance.
 * @param _i The zero-based index of the item to retrieve.
 * @return A pointer to the _i'th data record.
 */
#ifndef D3D12_DeviceTableDeviceStreamAt
#define D3D12_DeviceTableDeviceStreamAt(_t, _i)                                \
    Table_GetStreamElement(DISPLAY_DEVICE_D3D12, &(_t)->TableDesc, D3D12_DEVICE_STREAM_INDEX, _i)
#endif

/* @summary Retrieve the number of active display output records.
 * @param _t A pointer to a DISPLAY_OUTPUT_TABLE_D3D12 instance.
 * @return The number of attached display outputs as of the latest refresh.
 */
#ifndef D3D12_OutputTableCount
#define D3D12_OutputTableCount(_t)                                             \
    (_t)->OutputCount
#endif

/* @summary Retrieve the maximum number of active display outputs.
 * @param _t A pointer to a DISPLAY_OUTPUT_TABLE_D3D12 instance.
 * @return The maximum number of attached display outputs.
 */
#ifndef D3D12_OutputTableCapacity
#define D3D12_OutputTableCapacity(_t)                                          \
    (_t)->OutputCapacity
#endif

/* @summary Retrieve a pointer to the first display output identifier.
 * @param _t A pointer to a DISPLAY_OUTPUT_TABLE_D3D12 instance.
 * @return A pointer to the first display output identifier. If the returned pointer is the same as D3D12_OutputTableKeysEnd, the table is empty and the returned pointer must not be dereferenced.
 */
#ifndef D3D12_OutputTableKeysBegin
#define D3D12_OutputTableKeysBegin(_t)                                         \
    (_t)->OutputKeys
#endif

/* @summary Retrieve a pointer to one-past the last active display output identifier.
 * @param _t A pointer to a DISPLAY_OUTPUT_TABLE_D3D12 instance.
 * @return A pointer to one-past the last display output identifier. Do not dereference the returned pointer.
 */
#ifndef D3D12_OutputTableKeysEnd
#define D3D12_OutputTableKeysEnd(_t)                                           \
    ((_t)->OutputKeys + (_t)->OutputCount)
#endif

/* @summary Retrieve a pointer to the _i'th identifier for a display output.
 * @param _t A pointer to a DISPLAY_OUTPUT_TABLE_D3D12 instance.
 * @param _i The zero-based index of the item to retrieve.
 * @return A pointer to the _i'th data record identifier.
 */
#ifndef D3D12_OutputTableKeysAt
#define D3D12_OutputTableKeysAt(_t, _i)                                        \
    ((_t)->OutputKeys + (_i))
#endif

/* @summary Retrieve a pointer to the first active DISPLAY_OUTPUT_D3D12 record.
 * @param _t A pointer to a DISPLAY_OUTPUT_TABLE_D3D12 instance.
 * @return A pointer to the first active DISPLAY_OUTPUT_D3D12 record. If the returned pointer is the same as D3D12_OutputTableStreamEnd, the table is empty and the returned pointer must not be dereferenced.
 */
#ifndef D3D12_OutputTableStreamBegin
#define D3D12_OutputTableStreamBegin(_t)                                       \
    (_t)->OutputList
#endif

/* @summary Retrieve a pointer to one-past the last active DISPLAY_OUTPUT_D3D12 record.
 * @param _t A pointer to a DISPLAY_OUTPUT_TABLE_D3D12 instance.
 * @return A pointer to one-past the last active display output record. Do not dereference the returned pointer.
 */
#ifndef D3D12_OutputTableStreamEnd
#define D3D12_OutputTableStreamEnd(_t)                                         \
    ((_t)->OutputList + (_t)->OutputCount)
#endif

/* @summary Retrieve a pointer to the _i'th DISPLAY_OUTPUT_D3D12 record.
 * @param _t A pointer to a DISPLAY_OUTPUT_TABLE_D3D12 instance.
 * @param _i The zero-based index of the item to retrieve.
 * @return A pointer to the _i'th data record.
 */
#ifndef D3D12_OutputTableStreamAt
#define D3D12_OutputTableStreamAt(_t, _i)                                      \
    ((_t)->OutputList + (_i))
#endif

/* @summary Retrieve the number of active display adapter records.
 * @param _t A pointer to a DISPLAY_ADAPTER_TABLE_D3D12 instance.
 * @return The number of attached display adapters as of the latest refresh.
 */
#ifndef D3D12_AdapterTableCount
#define D3D12_AdapterTableCount(_t)                                            \
    (_t)->AdapterCount
#endif

/* @summary Retrieve the maximum number of active display adapters.
 * @param _t A pointer to a DISPLAY_ADAPTER_TABLE_D3D12 instance.
 * @return The maximum number of attached display adapters.
 */
#ifndef D3D12_AdapterTableCapacity
#define D3D12_AdapterTableCapacity(_t)                                         \
    (_t)->AdapterCapacity
#endif

/* @summary Retrieve a pointer to the first display adapter identifier.
 * @param _t A pointer to a DISPLAY_ADAPTER_TABLE_D3D12 instance.
 * @return A pointer to the first display adapter identifier. If the returned pointer is the same as D3D12_AdapterTableKeysEnd, the table is empty and the returned pointer must not be dereferenced.
 */
#ifndef D3D12_AdapterTableKeysBegin
#define D3D12_AdapterTableKeysBegin(_t)                                        \
    (_t)->AdapterKeys
#endif

/* @summary Retrieve a pointer to one-past the last active display adapter identifier.
 * @param _t A pointer to a DISPLAY_ADAPTER_TABLE_D3D12 instance.
 * @return A pointer to one-past the last display adapter identifier. Do not dereference the returned pointer.
 */
#ifndef D3D12_AdapterTableKeysEnd
#define D3D12_AdapterTableKeysEnd(_t)                                          \
    ((_t)->AdapterKeys + (_t)->AdapterCount)
#endif

/* @summary Retrieve a pointer to the _i'th identifier for a display adapter.
 * @param _t A pointer to a DISPLAY_ADAPTER_TABLE_D3D12 instance.
 * @param _i The zero-based index of the item to retrieve.
 * @return A pointer to the _i'th data record identifier.
 */
#ifndef D3D12_AdapterTableKeysAt
#define D3D12_AdapterTableKeysAt(_t, _i)                                       \
    ((_t)->AdapterKeys + (_i))
#endif

/* @summary Retrieve a pointer to the first active DISPLAY_ADAPTER_D3D12 record.
 * @param _t A pointer to a DISPLAY_ADAPTER_TABLE_D3D12 instance.
 * @return A pointer to the first active DISPLAY_ADAPTER_D3D12 record. If the returned pointer is the same as D3D12_AdapterTableStreamEnd, the table is empty and the returned pointer must not be dereferenced.
 */
#ifndef D3D12_AdapterTableStreamBegin
#define D3D12_AdapterTableStreamBegin(_t)                                      \
    (_t)->AdapterList
#endif

/* @summary Retrieve a pointer to one-past the last active DISPLAY_ADAPTER_D3D12 record.
 * @param _t A pointer to a DISPLAY_ADAPTER_TABLE_D3D12 instance.
 * @return A pointer to one-past the last active display adapter record. Do not dereference the returned pointer.
 */
#ifndef D3D12_AdapterTableStreamEnd
#define D3D12_AdapterTableStreamEnd(_t)                                        \
    ((_t)->AdapterList + (_t)->AdapterCount)
#endif

/* @summary Retrieve a pointer to the _i'th DISPLAY_ADAPTER_D3D12 record.
 * @param _t A pointer to a DISPLAY_ADAPTER_TABLE_D3D12 instance.
 * @param _i The zero-based index of the item to retrieve.
 * @return A pointer to the _i'th data record.
 */
#ifndef D3D12_AdapterTableStreamAt
#define D3D12_AdapterTableStreamAt(_t, _i)                                     \
    ((_t)->AdapterList + (_i))
#endif

/* @summary Define the data used to create a logical GPU device.
 */
typedef struct GPU_DEVICE_INIT_D3D12 {
    struct DISPLAY_ADAPTER_TABLE_D3D12 *AdapterTable;                          /* The display system managing the logical device. */
    struct D3D12API_DISPATCH           *D3D12Dispatch;                         /* The dispatch table used to call Direct3D 12 entry points. */
    uint64_t                            TargetAdapter;                         /* The identifier of the display adapter to use for executing GPU commands. */
    uint32_t                            UsageFlags;                            /* One or more bitwise OR'd values of the GPU_DEVICE_USAGE_FLAGS enumeration specifying the intended usage for the device. */
} GPU_DEVICE_INIT_D3D12;

/* @summary Define the data stored with an output (monitor, etc.) within the display device table.
 * The set of known outputs is updated when the attached display hardware is refreshed.
 */
typedef struct DISPLAY_OUTPUT_D3D12 {
    IDXGIOutput                        *DxgiOutput;                            /* The base DXGI output interface. */
    DXGI_OUTPUT_DESC                    OutputDesc;                            /* Basic information about the output, including the monitor handle. */
} DISPLAY_OUTPUT_D3D12;

/* @summary Define the data stored with a display adapter (physical device) within the display device table.
 * The set of known adapters is updated when the attached display hardware is refreshed.
 */
typedef struct DISPLAY_ADAPTER_D3D12 {
    IDXGIAdapter1                      *DxgiAdapter1;                          /* The base DXGI adapter interface. */
    uint32_t                            OutputCount;                           /* The number of outputs attached to the adapter. Having no outputs doesn't mean the device is disabled. */
    DXGI_ADAPTER_DESC1                  AdapterDesc1;                          /* Basic information about the display adapter. */
} DISPLAY_ADAPTER_D3D12;

/* @summary Define the data associated with a logical GPU device within the display device table.
 * A GPU_DEVICE_HANDLE resolves into an instance of the DISPLAY_DEVICE_D3D12 type.
 */
typedef struct DISPLAY_DEVICE_D3D12 {
    ID3D12Device                       *D3D12Device;                           /* The D3D12 device interface. */
    ID3D12CommandQueue                 *ComputeCommandQueue;                   /* The command queue used for submitting asynchronous compute commands. */
    ID3D12CommandQueue                 *GraphicsCommandQueue;                  /* The command queue used for submitting graphics commands. */
    ID3D12CommandQueue                 *TransferCommandQueue;                  /* The command queue used for submitting asynchronous data transfer commands between the host and the device. */
    D3D_FEATURE_LEVEL                   FeatureLevel;                          /* The D3D_FEATURE_LEVEL of the device. */
    uint32_t                            UsageFlags;                            /* One or more bitwise OR'd GPU_DEVICE_USAGE_FLAGS describing how the device will be used. */
    uint64_t                            PhysicalDeviceId;                      /* The identifier of the physical device used to execute commands sent to the logical device. */
    uint32_t                            ReferenceCount;                        /* The number of outstanding references to the logical device. */
} DISPLAY_DEVICE_D3D12;

/* @summary Define the data associated with the set of display adapters (physical GPU devices) attached to the system.
 * Adapters are assigned a "stable" identifier that will remain constant until the system is rebooted, so that an adapter is assigned the same identifier if it is removed and then re-attached.
 * The AdapterKeys and AdapterList arrays are aligned, such that AdapterKeys[i] is the identifier for the adapter described by AdapterList[i].
 */
typedef struct DISPLAY_ADAPTER_TABLE_D3D12 {
    uint32_t                            AdapterCount;                          /* The number of valid adapters in the table. */
    uint32_t                            AdapterCapacity;                       /* The maximum number of adapters that can be stored in the table. */
    uint64_t                           *AdapterKeys;                           /* An array of identifiers for each physical GPU device that remain the same until the system is rebooted. */
    struct DISPLAY_ADAPTER_D3D12       *AdapterList;                           /* An array of structures describing each physical GPU device attached to the system. */
} DISPLAY_ADAPTER_TABLE_D3D12;

/* @summary Define the data associated with the set of display ouputs (monitors) attached to the system.
 * The OutputKeys and OutputList arrays are aligned, such that OutputKeys[i] is the identifier for the output described by OutputList[i].
 */
typedef struct DISPLAY_OUTPUT_TABLE_D3D12 {
    uint32_t                            OutputCount;                           /* The number of valid outputs in the table. */
    uint32_t                            OutputCapacity;                        /* The maximum number of outputs that can be stored in the table. */
    uint64_t                           *OutputKeys;                            /* An array of identifiers for each display output. */
    struct DISPLAY_OUTPUT_D3D12        *OutputList;                            /* An array of structures describing each display output attached to the system. */
} DISPLAY_OUTPUT_TABLE_D3D12;

/* @summary Define the data associated with the table of logical GPU devices.
 * A logical device is identified externally using a GPU_DEVICE_HANDLE.
 */
typedef struct DISPLAY_DEVICE_TABLE_D3D12 {
    TABLE_DESC                          TableDesc;                             /* The table descriptor, used for calling the data table functions. */
    TABLE_INDEX                         TableIndex;                            /* The table index used to map GPU_DEVICE_HANDLE to a dense array index. */
    TABLE_DATA                          DeviceData;                            /* The table data storing the stream of DISPLAY_DEVICE_D3D12 instances. */
    TABLE_DATA                         *TableStreams[1];                       /* An array of pointers to the table data streams. */
} DISPLAY_DEVICE_TABLE_D3D12;

/* @summary Allocate and initialize storage for a table describing the physical GPU devices attached to the host system.
 * @param table The table structure to initialize.
 * @param capacity The maximum number of display adapters the application can work with.
 * @return Zero if the table is successfully initialized, or non-zero if an error occurred.
 */
PIL_API(int)
D3D12_CreateDisplayAdapterTable
(
    struct DISPLAY_ADAPTER_TABLE_D3D12 *table, 
    uint32_t                         capacity
);

/* @summary Free resources associated with a display adapter table.
 * @param table The table to delete.
 */
PIL_API(void)
D3D12_DeleteDisplayAdapterTable
(
    struct DISPLAY_ADAPTER_TABLE_D3D12 *table
);

/* @summary Release any operating system interfaces held by a display adapter table.
 * The underlying table storage is not freed. This function invalidates any pointers to DISPLAY_ADAPTER_D3D12 instances.
 * @param table The table to clear.
 */
PIL_API(void)
D3D12_ClearDisplayAdapterTable
(
    struct DISPLAY_ADAPTER_TABLE_D3D12 *table
);

/* @summary Allocate and initialize storage for a table describing the display outputs attached to the host system.
 * @param table The table structure to initialize.
 * @param capacity The maximum number of display outputs the application can work with.
 * @return Zero if the table is successfully initialized, or non-zero if an error occurred.
 */
PIL_API(int)
D3D12_CreateDisplayOutputTable
(
    struct DISPLAY_OUTPUT_TABLE_D3D12 *table, 
    uint32_t                        capacity
);

/* @summary Free resources associated with a display output table.
 * @param table The table to delete.
 */
PIL_API(void)
D3D12_DeleteDisplayOutputTable
(
    struct DISPLAY_OUTPUT_TABLE_D3D12 *table
);

/* @summary Release any operating system interfaces held by a display output table.
 * The underlying table storage is not freed. This function invalidates any pointers to DISPLAY_OUTPUT_D3D12 instances.
 * @param table The table to clear.
 */
PIL_API(void)
D3D12_ClearDisplayOutputTable
(
    struct DISPLAY_OUTPUT_TABLE_D3D12 *table
);

/* @summary Allocate and initialize storage for a table describing the logical GPU device interfaces created by the application.
 * @param table The table structure to initialize.
 * @param capacity The maximum number of logical GPU device interfaces required by the application.
 * @return Zero if the table is successfully initialized, or non-zero if an error occurred.
 */
PIL_API(int)
D3D12_CreateDisplayDeviceTable
(
    struct DISPLAY_DEVICE_TABLE_D3D12 *table, 
    uint32_t                        capacity
);

/* @summary Free resources associated with a logical GPU device table.
 * @param table The table to delete.
 */
PIL_API(void)
D3D12_DeleteDisplayDeviceTable
(
    struct DISPLAY_DEVICE_TABLE_D3D12 *table
);

/* @summary Given an adapter identifier, return a structure describing the adapter.
 * @param table The adapter table to query.
 * @param adapter_key The identifier for the adapter to retrieve.
 * @return A DISPLAY_ADAPTER_D3D12 pointer. Do not cache the returned pointer. It remains valid until the next call to D3D12_RefreshDisplayHardware.
 */
PIL_API(struct DISPLAY_ADAPTER_D3D12*)
D3D12_ResolveAdapterForKey
(
    struct DISPLAY_ADAPTER_TABLE_D3D12 *table, 
    uint64_t                      adapter_key
);

/* @summary Given an display output identifier, return a structure describing the output.
 * @param table The display output table to query.
 * @param output_key The identifier for the display output to retrieve.
 * @return A DISPLAY_OUTPUT_D3D12 pointer. Do not cache the returned pointer. It remains valid until the next call to D3D12_RefreshDisplayHardware.
 */
PIL_API(struct DISPLAY_OUTPUT_D3D12*)
D3D12_ResolveOutputForKey
(
    struct DISPLAY_OUTPUT_TABLE_D3D12 *table, 
    uint64_t                      output_key
);

/* @summary Given a Win32 monitor handle, return a structure describing the display output.
 * @param table The display output table to query.
 * @param monitor The Win32 monitor handle associated with the display output.
 * @return A DISPLAY_OUTPUT_D3D12 pointer. Do not cache the returned pointer. It remains valid until the next call to D3D12_RefreshDisplayHardware.
 */
PIL_API(struct DISPLAY_OUTPUT_D3D12*)
D3D12_ResolveOutputForHMONITOR
(
    struct DISPLAY_OUTPUT_TABLE_D3D12 *table, 
    HMONITOR                         monitor
);

/* @summary Given a GPU_DEVICE_HANDLE, return a structure describing the logical device.
 * @param table The logical device table to query.
 * @param handle The GPU_DEVICE_HANDLE identifying the logical GPU device.
 * @return A DISPLAY_DEVICE_D3D12 pointer. Do not cache the returned pointer. It remains valid until any logical GPU device is deleted.
 */
PIL_API(struct DISPLAY_DEVICE_D3D12*)
D3D12_ResolveDeviceForHandle
(
    struct DISPLAY_DEVICE_TABLE_D3D12 *table, 
    struct GPU_DEVICE_HANDLE          handle
);

/* @summary Retrieve descriptions of display adapters attached to the system.
 * @param table The display adapter table to query. The table must be populated by a call to D3D12_RefreshDisplayHardware.
 * @param results The array where result data will be written.
 * @param start_index The zero-based index of the first item to retrieve.
 * @param max_results The maximum number of results that can be written to the results array.
 * @return The number of items written to the results array, or if results is NULL and max_results is 0, the number of display adapters attached to the system.
 */
PIL_API(uint32_t)
D3D12_DescribeDisplayAdapters
(
    struct DISPLAY_ADAPTER_TABLE_D3D12 *table, 
    struct DISPLAY_ADAPTER_DESC      *results, 
    uint32_t                      start_index, 
    uint32_t                      max_results
);

/* @summary Retrieve descriptions of display outputs attached to the system.
 * @param table The display output table to query. The table must be populated by a call to D3D12_RefreshDisplayHardware.
 * @param results The array where result data will be written.
 * @param start_index The zero-based index of the first item to retrieve.
 * @param max_results The maximum number of results that can be written to the results array.
 * @param win32 Pointer to a WIN32API_DISPATCH containing Windows API entry points resolved at runtime. This is needed to obtain monitor information.
 * @return The number of items written to the results array, or if results is NULL and max_results is 0, the number of display outputs attached to the system.
 */
PIL_API(uint32_t)
D3D12_DescribeDisplayOutputs
(
    struct DISPLAY_OUTPUT_TABLE_D3D12 *table, 
    struct DISPLAY_OUTPUT_DESC      *results, 
    uint32_t                     start_index, 
    uint32_t                     max_results, 
    struct WIN32API_DISPATCH          *win32
);

/* @summary Enumerate (or re-enumerate) display hardware attached to the host.
 * This operation clears the supplied adapter and output tables and invalidates any cached pointers and interfaces.
 * @param adapters The table of display adapters to populate.
 * @param outputs The table of display outputs to populate.
 * @param o_num_adapters On return, this location stores the number of display adapters attached to the system.
 * @param o_num_outputs On return, this location stores the number of display outputs attached to the system.
 * @param factory The DXGI factory used to interface with the display system.
 * @return Zero if hardware enumeration completed successfully, or non-zero if an error occurred.
 */
PIL_API(int)
D3D12_RefreshDisplayHardware
(
    struct DISPLAY_ADAPTER_TABLE_D3D12 *adapters, 
    struct DISPLAY_OUTPUT_TABLE_D3D12   *outputs, 
    uint32_t                     *o_num_adapters, 
    uint32_t                      *o_num_outputs, 
    IDXGIFactory2                       *factory
);

/* @summary Create a logical GPU device interface.
 * This operation may either create a new device interface or reference an existing device interface.
 * @param table The table managing the GPU device storage.
 * @param init Data used to control the creation of the GPU device interface.
 * @param o_handle Pointer to the GPU_DEVICE_HANDLE to set to the identifier of the logical GPU device.
 * @return Zero if the device is successfully created, or non-zero if an error occurred.
 */
PIL_API(int)
D3D12_CreateLogicalGpuDevice
(
    struct DISPLAY_DEVICE_TABLE_D3D12 *table, 
    struct GPU_DEVICE_INIT_D3D12       *init, 
    struct GPU_DEVICE_HANDLE       *o_handle
);

/* @summary Delete a logical GPU device interface.
 * @param table The table managing the GPU device interface.
 * @param handle The handle of the GPU device interface to delete.
 */
PIL_API(void)
D3D12_DeleteLogicalGpuDevice
(
    struct DISPLAY_DEVICE_TABLE_D3D12 *table, 
    struct GPU_DEVICE_HANDLE          handle
);

#endif /* __PIL_DISPLAY_DEVICE_D3D12_H__ */

