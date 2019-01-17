/**
 * @summary display_window_d3d12.h: Define the data structures and functions 
 * relating to window and swapchain management.
 */
#ifndef __PIL_DISPLAY_WINDOW_D3D12_H__
#define __PIL_DISPLAY_WINDOW_D3D12_H__

#pragma once

struct WIN32API_DISPATCH;                                                      /* win32api_win32.h */
struct DISPLAY_SYSTEM_D3D12;                                                   /* display_system_d3d12.h */
struct DISPLAY_OUTPUT_D3D12;                                                   /* display_device_d3d12.h */
struct DISPLAY_DEVICE_D3D12;                                                   /* display_device_d3d12.h */
struct DISPLAY_OUTPUT_TABLE_D3D12;                                             /* display_device_d3d12.h */
struct WINDOW_STATE_DATA_D3D12;
struct WINDOW_DEVICE_DATA_D3D12;
struct WINDOW_SWAPCHAIN_DATA_D3D12;
struct GPU_WINDOW_D3D12;
struct GPU_WINDOW_INIT_D3D12;

/* @summary Define constants related to table access and the number of images in a swapchain.
 * D3D12_WINDOW_STATE_STREAM_INDEX       : The zero-based index of the data stream of WINDOW_STATE_DATA_D3D12 records.
 * D3D12_WINDOW_DEVICE_STREAM_INDEX      : The zero-based index of the data stream of WINDOW_DEVICE_DATA_D3D12 records.
 * D3D12_WINDOW_SWAPCHAIN_STREAM_INDEX   : The zero-based index of the data stream of WINDOW_SWAPCHAIN_DATA_D3D12 records.
 * D3D12_WINDOW_SWAPCHAIN_IMAGE_COUNT    : The number of swapchain images -requested- for each GPU-driven window.
 * D3D12_WINDOW_SWAPCHAIN_IMAGE_COUNT_MAX: The maximum number of images that can be associated with a swapchain.
 */
#ifndef DISPLAY_WINDOW_D3D12_CONSTANTS
#   define DISPLAY_WINDOW_D3D12_CONSTANTS
#   define D3D12_WINDOW_STATE_STREAM_INDEX                                     0
#   define D3D12_WINDOW_DEVICE_STREAM_INDEX                                    1
#   define D3D12_WINDOW_SWAPCHAIN_STREAM_INDEX                                 2
#   define D3D12_WINDOW_TABLE_STREAM_COUNT                                     3
#   define D3D12_WINDOW_SWAPCHAIN_IMAGE_COUNT                                  3
#   define D3D12_WINDOW_SWAPCHAIN_IMAGE_COUNT_MAX                              DXGI_MAX_SWAP_CHAIN_BUFFERS
#endif

/* @summary Retrieve the number of active GPU-driven windows.
 * @param _t A pointer to a DISPLAY_WINDOW_TABLE_D3D12 instance.
 * @return The number of active GPU-driven windows.
 */
#ifndef D3D12_WindowTableCount
#define D3D12_WindowTableCount(_t)                                             \
    Table_GetCount(&(_t)->TableDesc)
#endif

/* @summary Retrieve the maximum number of active GPU-driven windows.
 * @param _t A pointer to a DISPLAY_WINDOW_TABLE_D3D12 instance.
 * @return The maximum number of active GPU-driven windows.
 */
#ifndef D3D12_WindowTableCapacity
#define D3D12_WindowTableCapacity(_t)                                          \
    Table_GetCapacity(&(_t)->TableDesc)
#endif

/* @summary Retrieve a pointer to the first element in the active handle stream for the set of GPU-driven windows.
 * @param _t A pointer to a DISPLAY_WINDOW_TABLE_D3D12 instance.
 * @return A pointer (HANDLE_BITS*) to the first active identifier in the table. If equal to the address returned by D3D12_WindowTableWindowHandleEnd(_t), the table is empty and the returned pointer must not be dereferenced.
 */
#ifndef D3D12_WindowTableWindowHandleBegin
#define D3D12_WindowTableWindowHandleBegin(_t)                                 \
    Table_GetHandleBegin(&(_t)->TableDesc)
#endif

/* @summary Retrieve a pointer to one-past the last element in the active handle stream for the set of GPU-driven windows.
 * @param _t A pointer to a DISPLAY_WINDOW_TABLE_D3D12 instance.
 * @return A pointer (HANDLE_BITS*) to one-past the last active identifier. Do not dereference the returned pointer.
 */
#ifndef D3D12_WindowTableWindowHandleEnd
#define D3D12_WindowTableWindowHandleEnd(_t)                                   \
    Table_GetHandleEnd(&(_t)->TableDesc)
#endif

/* @summary Retrieve the _i'th handle of a GPU-driven window.
 * @param _t A pointer to a DISPLAY_WINDOW_TABLE_D3D12 instance.
 * @param _i The zero-based index of the active window handle to retrieve.
 * @return The _i'th window handle.
 */
#ifndef D3D12_WindowTableHandleAt
#define D3D12_WindowTableHandleAt(_t, _i)                                      \
    Table_GetHandle(&(_t)->TableDesc, _i)
#endif

/* @summary Retrieve a pointer to the first WINDOW_STATE_DATA_D3D12 data record.
 * @param _t A pointer to a DISPLAY_WINDOW_TABLE_D3D12 instance.
 * @return A pointer to the first active WINDOW_STATE_DATA_D3D12 record. If the returned pointer is the same as D3D12_WindowTableStateStreamEnd, the table is empty and the returned pointer must not be dereferenced.
 */
#ifndef D3D12_WindowTableStateStreamBegin
#define D3D12_WindowTableStateStreamBegin(_t)                                  \
    Table_GetStreamBegin(WINDOW_STATE_DATA_D3D12, &(_t)->TableDesc, D3D12_WINDOW_STATE_STREAM_INDEX)
#endif

/* @summary Retrieve a pointer to one-past the last WINDOW_STATE_DATA_D3D12 data record.
 * @param _t A pointer to a DISPLAY_WINDOW_TABLE_D3D12 instance.
 * @return A pointer to one-past the last active WINDOW_STATE_DATA_D3D12 data record. Do not dereference the returned pointer.
 */
#ifndef D3D12_WindowTableStateStreamEnd
#define D3D12_WindowTableStateStreamEnd(_t)                                    \
    Table_GetStreamEnd  (WINDOW_STATE_DATA_D3D12, &(_t)->TableDesc, D3D12_WINDOW_STATE_STREAM_INDEX)
#endif

/* @summary Retrieve a pointer to the _i'th WINDOW_STATE_DATA_D3D12 record.
 * @param _t A pointer to a DISPLAY_WINDOW_TABLE_D3D12 instance.
 * @param _i The zero-based index of the item to retrieve.
 * @return A pointer to the _i'th data record.
 */
#ifndef D3D12_WindowTableStateStreamAt
#define D3D12_WindowTableStateStreamAt(_t, _i)                                 \
    Table_GetStreamElement(WINDOW_STATE_DATA_D3D12, &(_t)->TableDesc, D3D12_WINDOW_STATE_STREAM_INDEX, _i)
#endif

/* @summary Retrieve a pointer to the first WINDOW_DEVICE_DATA_D3D12 data record.
 * @param _t A pointer to a DISPLAY_WINDOW_TABLE_D3D12 instance.
 * @return A pointer to the first active WINDOW_DEVICE_DATA_D3D12 record. If the returned pointer is the same as D3D12_WindowTableDeviceStreamEnd, the table is empty and the returned pointer must not be dereferenced.
 */
#ifndef D3D12_WindowTableDeviceStreamBegin
#define D3D12_WindowTableDeviceStreamBegin(_t)                                 \
    Table_GetStreamBegin(WINDOW_DEVICE_DATA_D3D12, &(_t)->TableDesc, D3D12_WINDOW_DEVICE_STREAM_INDEX)
#endif

/* @summary Retrieve a pointer to one-past the last WINDOW_DEVICE_DATA_D3D12 data record.
 * @param _t A pointer to a DISPLAY_WINDOW_TABLE_D3D12 instance.
 * @return A pointer to one-past the last active WINDOW_DEVICE_DATA_D3D12 data record. Do not dereference the returned pointer.
 */
#ifndef D3D12_WindowTableDeviceStreamEnd
#define D3D12_WindowTableDeviceStreamEnd(_t)                                   \
    Table_GetStreamEnd  (WINDOW_DEVICE_DATA_D3D12, &(_t)->TableDesc, D3D12_WINDOW_DEVICE_STREAM_INDEX)
#endif

/* @summary Retrieve a pointer to the _i'th WINDOW_DEVICE_DATA_D3D12 record.
 * @param _t A pointer to a DISPLAY_WINDOW_TABLE_D3D12 instance.
 * @param _i The zero-based index of the item to retrieve.
 * @return A pointer to the _i'th data record.
 */
#ifndef D3D12_WindowTableDeviceStreamAt
#define D3D12_WindowTableDeviceStreamAt(_t, _i)                                \
    Table_GetStreamElement(WINDOW_DEVICE_DATA_D3D12, &(_t)->TableDesc, D3D12_WINDOW_DEVICE_STREAM_INDEX, _i)
#endif

/* @summary Retrieve a pointer to the first WINDOW_SWAPCHAIN_DATA_D3D12 data record.
 * @param _t A pointer to a DISPLAY_WINDOW_TABLE_D3D12 instance.
 * @return A pointer to the first active WINDOW_SWAPCHAIN_DATA_D3D12 record. If the returned pointer is the same as D3D12_WindowTableSwapchainStreamEnd, the table is empty and the returned pointer must not be dereferenced.
 */
#ifndef D3D12_WindowTableSwapchainStreamBegin
#define D3D12_WindowTableSwapchainStreamBegin(_t)                              \
    Table_GetStreamBegin(WINDOW_SWAPCHAIN_DATA_D3D12, &(_t)->TableDesc, D3D12_WINDOW_SWAPCHAIN_STREAM_INDEX)
#endif

/* @summary Retrieve a pointer to one-past the last WINDOW_SWAPCHAIN_DATA_D3D12 data record.
 * @param _t A pointer to a DISPLAY_WINDOW_TABLE_D3D12 instance.
 * @return A pointer to one-past the last active WINDOW_SWAPCHAIN_DATA_D3D12 data record. Do not dereference the returned pointer.
 */
#ifndef D3D12_WindowTableSwapchainStreamEnd
#define D3D12_WindowTableSwapchainStreamEnd(_t)                                \
    Table_GetStreamEnd  (WINDOW_SWAPCHAIN_DATA_D3D12, &(_t)->TableDesc, D3D12_WINDOW_SWAPCHAIN_STREAM_INDEX)
#endif

/* @summary Retrieve a pointer to the _i'th WINDOW_SWAPCHAIN_DATA_D3D12 record.
 * @param _t A pointer to a DISPLAY_WINDOW_TABLE_D3D12 instance.
 * @param _i The zero-based index of the item to retrieve.
 * @return A pointer to the _i'th data record.
 */
#ifndef D3D12_WindowTableSwapchainStreamAt
#define D3D12_WindowTableSwapchainStreamAt(_t, _i)                             \
    Table_GetStreamElement(WINDOW_SWAPCHAIN_DATA_D3D12, &(_t)->TableDesc, D3D12_WINDOW_SWAPCHAIN_STREAM_INDEX, _i)
#endif

/* @summary Convert from physical to logical pixels.
 * @param _dim The value to convert, specified in physical pixels.
 * @param _dpi The DPI value of the display.
 * @return The corresponding dimention specified in logical pixels.
 */
#ifndef D3D12_PhysicalToLogicalPixels
#define D3D12_PhysicalToLogicalPixels(_dim, _dpi)                              \
    (((_dim) * USER_DEFAULT_SCREEN_DPI) / (_dpi))
#endif

/* @summary Convert from logical to physical pixels.
 * @param _dim The value to convert, specified in logical pixels.
 * @param _dpi The DPI value of the display.
 * @return The corresponding dimension specified in physical pixels.
 */
#ifndef D3D12_LogicalToPhysicalPixels
#define D3D12_LogicalToPhysicalPixels(_dim, _dpi)                              \
    (((_dim) * (_dpi)) / USER_DEFAULT_SCREEN_DPI)
#endif

/* @summary Define the data used to create a GPU-driven window.
 * The public entry points convert from a GPU_WINDOW_INIT to a GPU_WINDOW_INIT_D3D12 structure.
 */
typedef struct GPU_WINDOW_INIT_D3D12 {
    struct DISPLAY_OUTPUT_TABLE_D3D12 *OutputTable;                            /* The display output table used to map an HMONITOR to an IDXGIOutput. */
    struct WIN32API_DISPATCH          *Win32ApiDispatch;                       /* The dispatch table used to access dynamically-loaded Windows API functions. */
    IDXGIFactory2                     *DxgiFactory;                            /* The object used to create DXGI interfaces. */
    IDXGIOutput                       *DxgiOutput;                             /* The target output (monitor) for the window, or NULL to target the primary display. */
    ID3D12Device                      *D3D12Device;                            /* The Direct3D 12 device used to drive window rendering and presentation. */
    ID3D12CommandQueue                *D3D12CommandQueue;                      /* The Direct3D 12 command queue used for executing commands that will update swapchain images. */
    WCHAR const                       *WindowTitle;                            /* A nul-terminated string specifying the window title. */
    uint32_t                           ClientWidth;                            /* The desired horizontal size of the window client area, in logical pixels. */
    uint32_t                           ClientHeight;                           /* The desired vertical size of the window client area, in logical pixels. */
    uint32_t                           SwapchainWidth;                         /* The desired horizontal size of the swapchain, in physical pixels. */
    uint32_t                           SwapchainHeight;                        /* The desired vertical size of the swapchain, in physical pixels. */
    uint32_t                           FeatureFlags;                           /* One or more bitwise OR'd values of the D3D12_SYSTEM_FEATURE_FLAGS enumeration. */
} GPU_WINDOW_INIT_D3D12;

/* @summary Define the data associated with the window itself.
 * This data is updated by the window message callback during the window update loop.
 */
typedef struct WINDOW_STATE_DATA_D3D12 {
    struct DISPLAY_WINDOW_TABLE_D3D12 *WindowTable;                            /* The window table that owns this window. */
    struct DISPLAY_OUTPUT_TABLE_D3D12 *OutputTable;                            /* The display output table used to map an HMONITOR to an IDXGIOutput. */
    struct WIN32API_DISPATCH          *Win32ApiDispatch;                       /* The Windows API runtime dispatch table. */
    HWND                               WindowHandle;                           /* The Win32 window handle. */
    uint32_t                           StatusFlags;                            /* One or more bitwise OR'd values of the GPU_WINDOW_STATUS_FLAGS enumeration. */
    uint32_t                           EventFlags;                             /* One or more bitwise OR'd values of the GPU_WINDOW_EVENT_FLAGS enumeration. */
    uint32_t                           OutputDpiX;                             /* The horizontal dots-per-inch setting of the output containing the window. */
    uint32_t                           OutputDpiY;                             /* The vertical dots-per-inch setting of the output containing the window. */
    int32_t                            PositionX;                              /* The x-coordinate of the upper-left corner of the window, in virtual display space. */
    int32_t                            PositionY;                              /* The y-coordinate of the upper-left corner of the window, in virtual display space. */
    uint32_t                           WindowSizeX;                            /* The horizontal size of the window, including borders and chrome, in logical pixels. */
    uint32_t                           WindowSizeY;                            /* The vertical size of the window, including borders and chrome, in logical pixels. */
    uint32_t                           ClientSizeX;                            /* The horizontal size of the window client area, in logical pixels. */
    uint32_t                           ClientSizeY;                            /* The vertical size of the window client area, in logical pixels. */
    RECT                               RestoreRect;                            /* The RECT describing the window position and size prior to entering fullscreen mode, in physical pixels. */
    DWORD                              RestoreStyle;                           /* The window style used for the window prior to entering fullscreen mode. */
    DWORD                              RestoreStyleEx;                         /* The extended window style used for the window prior to entering fullscreen mode. */
} WINDOW_STATE_DATA_D3D12;

/* @summary Define the Direct3D device data associated with a GPU-driven window.
 */
typedef struct WINDOW_DEVICE_DATA_D3D12 {
    ID3D12Device                      *D3D12Device;                            /* The Direct3D 12 device used for rendering and presentation. */
    ID3D12Fence                       *D3D12FenceSync;                         /* The fence object used for CPU-GPU synchronization. */
    ID3D12CommandQueue                *D3D12CommandQueue;                      /* The command queue used submit commands that update swapchain images. */
    uint64_t                          *FenceValues;                            /* The current fence values for each swapchain image. */
    HANDLE                             FenceEvent;                             /* A Win32 event object the CPU can use to wait for GPU work to complete. */
    uint32_t                           ImageIndex;                             /* The zero-based index of the image being rendered by the application. */
    uint32_t                           ImageCount;                             /* The number of valid entries in the per-image resource arrays. */
} WINDOW_DEVICE_DATA_D3D12;

/* @summary Define the data associated with the swapchain bound to a window.
 */
typedef struct WINDOW_SWAPCHAIN_DATA_D3D12 {
    IDXGISwapChain3                   *DxgiSwapChain;                          /* The swapchain interface associated with the window. */
    ID3D12Resource                   **ImageResources;                         /* The render target interfaces for each swapchain image. */
    DXGI_FORMAT                        ImageFormat;                            /* The format of the swapchain images. */
    uint32_t                           ImageWidth;                             /* The width of the swapchain, in physical pixels. */
    uint32_t                           ImageHeight;                            /* The height of the swapchain, in physical pixels. */
    uint32_t                           ImageCount;                             /* The number of swapchain images. Always <= _D3D12_MAX_FRAMES. */
    uint32_t                           StatusFlags;                            /* One or more bitwise OR'd values of the D3D12_SWAPCHAIN_STATUS_FLAGS enumeration. */
} WINDOW_SWAPCHAIN_DATA_D3D12;

/* @summary Wrap up the data associated with a window in a convenient package.
 * Instances of this structure should not be cached for long periods of time.
 */
typedef struct GPU_WINDOW_D3D12 {
    struct WINDOW_STATE_DATA_D3D12     *StateData;                             /* The basic window state data, updated by handling messages in WndProc. */
    struct WINDOW_DEVICE_DATA_D3D12    *DeviceData;                            /* Direct3D command queues and synchronization objects associated with the window . */
    struct WINDOW_SWAPCHAIN_DATA_D3D12 *SwapchainData;                         /* Data associated with the DXGI swapchain bound to the window. */
    HANDLE_BITS                         HandleBits;                            /* The HANDLE_BITS used to construct the GPU_WINDOW_HANDLE. */
    uint32_t                            RecordIndex;                           /* The dense record index used to resolve the handle into pointers. */
} GPU_WINDOW_D3D12;

/* @summary Define the data associated with the GPU-driven Direct3D 12 window manager.
 * An instance of this type is created and owned by the DISPLAY_SYSTEM_D3D12 instance.
 */
typedef struct DISPLAY_WINDOW_TABLE_D3D12 {
    TABLE_DESC                          TableDesc;                             /* The table descriptor, used for calling the data table functions. */
    TABLE_INDEX                         TableIndex;                            /* The table index used to map GPU_DEVICE_HANDLE to a dense array index. */
    TABLE_DATA                          StateData;                             /* The table data storing the stream of WINDOW_STATE_DATA_D3D12 instances. */
    TABLE_DATA                          DeviceData;                            /* The table data storing the stream of WINDOW_DEVICE_DATA_D3D12 instances. */
    TABLE_DATA                          SwapchainData;                         /* The table data storing the stream of WINDOW_SWAPCHAIN_DATA_D3D12 instances. */
    TABLE_DATA                         *TableStreams[3];                       /* An array of pointers to the table data streams. */
} DISPLAY_WINDOW_TABLE_D3D12;

/* @summary Define a series of flags that can be bitwise OR'd to describe the status of a swapchain.
 */
typedef enum SWAPCHAIN_STATUS_FLAGS_D3D12 {
    SWAPCHAIN_STATUS_FLAGS_D3D12_NONE              = (0UL <<  0),              /* The swapchain is in windowed mode. */
    SWAPCHAIN_STATUS_FLAG_D3D12_TEARING            = (1UL <<  0),              /* The swapchain supports tearing. */
    SWAPCHAIN_STATUS_FLAG_D3D12_FULLSCREEN         = (1UL <<  1),              /* The swapchain is in fullscreen mode. */
} SWAPCHAIN_STATUS_FLAGS_D3D12;

/* @summary Allocate and initialize storage for a table describing the GPU-driven windows created by the application.
 * @param table The table structure to initialize.
 * @param capacity The maximum number of windows required by the application at any one time.
 * @return Zero if the table is successfully initialized, or non-zero if an error occurred.
 */
PIL_API(int)
D3D12_CreateDisplayWindowTable
(
    struct DISPLAY_WINDOW_TABLE_D3D12 *table, 
    uint32_t                        capacity
);

/* @summary Free resources associated with a GPU window table.
 * @param table The table to delete.
 */
PIL_API(void)
D3D12_DeleteDisplayWindowTable
(
    struct DISPLAY_WINDOW_TABLE_D3D12 *table
);

/* @summary Given a GPU_WINDOW_HANDLE, return a structure pointing to the window manager data records associated with the window.
 * The returned data should not be cached. The data record pointers remain valid until any window is deleted.
 * @param table The window table to query.
 * @param handle The GPU_WINDOW_HANDLE identifying the window to the application.
 * @param o_data Pointer to the GPU_WINDOW_D3D12 structure to populate.
 * @return Zero if the window is successfully resolved, or non-zero otherwise.
 */
PIL_API(int)
D3D12_ResolveWindowForHandle
(
    struct DISPLAY_WINDOW_TABLE_D3D12 *table, 
    struct GPU_WINDOW_HANDLE          handle, 
    struct GPU_WINDOW_D3D12          *o_data
);

/* @summary Given a WINDOW_STATE_DATA_D3D12, return a structure pointing to the remaining window manager data records associated with the window.
 * The returned data should not be cached. The data record pointers remain valid until any window is deleted.
 * @param state The existing window state reference.
 * @param o_data Pointer to the GPU_WINDOW_D3D12 structure to populate.
 * @return Zero if the window is successfully resolved, or non-zero otherwise.
 */
PIL_API(int)
D3D12_ResolveWindowForWindowState
(
    struct WINDOW_STATE_DATA_D3D12 *state, 
    struct GPU_WINDOW_D3D12       *o_data
);

/* @summary Create a GPU-driven window and its associated swapchain, used for presenting content.
 * The window can be created on a target display, however the window is always created in "windowed" mode.
 * @param table The window table that will manage the window.
 * @param init Data used to configure window creation and behavior.
 * @param o_handle Pointer to the GPU_WINDOW_HANDLE to set to the identifier of the new window.
 * @return Zero if the window is successfully created, or non-zero if an error occurred.
 */
PIL_API(int)
D3D12_CreateWindowAndSwapchain
(
    struct DISPLAY_WINDOW_TABLE_D3D12 *table, 
    struct GPU_WINDOW_INIT_D3D12       *init, 
    struct GPU_WINDOW_HANDLE       *o_handle
);

/* @summary Close and destroy a GPU-driven window and its associated swapchain resources.
 * This operation invalidates the supplied window handle, and may block the calling thread while pending messages are processed.
 * @param table The window table that manages the window.
 * @param handle The identifier of the window to delete.
 */
PIL_API(void)
D3D12_DeleteWindowAndSwapchain
(
    struct DISPLAY_WINDOW_TABLE_D3D12 *table, 
    struct GPU_WINDOW_HANDLE          handle
);

/* @summary Process queued operating system events for all windows.
 * This operation may result in swapchains being resized and image resources being invalidated.
 * Retrieve the state of each specific window, when necessary, using the D3D12_QueryWindowState function.
 * @param table The window table defining the set of windows for which events will be processed.
 */
PIL_API(void)
D3D12_ProcessWindowEvents
(
    struct DISPLAY_WINDOW_TABLE_D3D12 *table
);

/* @summary Retrieve the state of a given window as of the most recent call to D3D12_ProcessWindowEvents.
 * @param table The window table to query.
 * @param handle The identifier of the window to query.
 * @param o_state Pointer to the GPU_WINDOW_STATE to populate with state information.
 * @return Zero if window state information was written to o_state, or non-zero if an error occurred.
 */
PIL_API(int)
D3D12_QueryWindowState
(
    struct DISPLAY_WINDOW_TABLE_D3D12 *table,
    struct GPU_WINDOW_HANDLE          handle, 
    struct GPU_WINDOW_STATE         *o_state
);

#endif /* __PIL_DISPLAY_WINDOW_D3D12_H__ */

