/**
 * @summary display.h: Define types and functions related to display management.
 * Most of the types are platform-specific, and defined in a platform header.
 */
#ifndef __PIL_DISPLAY_H__
#define __PIL_DISPLAY_H__

#pragma once

#ifndef PIL_NO_INCLUDES
#   ifndef __PIL_H__
#       include "pil.h"
#   endif
#   ifndef __PIL_MEMMGR_H__
#       include "memmgr.h"
#   endif
#   ifndef __PIL_TABLE_H__
#       include "table.h"
#   endif
#endif

/* @summary Forward-declare the types exported by this module.
 */
struct  DISPLAY_SYSTEM;
struct  DISPLAY_SYSTEM_INIT;
struct  DISPLAY_OUTPUT_DESC;
struct  DISPLAY_ADAPTER_DESC;
struct  GPU_WINDOW_HANDLE;
struct  GPU_DEVICE_HANDLE;
struct  GPU_PROGRAM_HANDLE;
struct  GPU_PROGRAM_CACHE;
struct  GPU_DEVICE_INIT;
struct  GPU_WINDOW_INIT;
struct  GPU_WINDOW_STATE;

/* @summary Define an opaque type used to identify a window with an associated swapchain, whose contents is rendered by a GPU device.
 */
typedef struct GPU_WINDOW_HANDLE {
    HANDLE_BITS       Bits;                                                    /* The 32-bit handle value. */
} GPU_WINDOW_HANDLE;

/* @summary Define an opaque type used to identify a logical GPU device.
 */
typedef struct GPU_DEVICE_HANDLE {
    HANDLE_BITS       Bits;                                                    /* The 32-bit handle value. */
} GPU_DEVICE_HANDLE;

/* @summary Define an opaque type used to identify a GPU program object.
 */
typedef struct GPU_PROGRAM_HANDLE {
    HANDLE_BITS       Bits;                                                    /* The 32-bit handle value. */
} GPU_PROGRAM_HANDLE;

/* @summary Define the data describing a display adapter, which represents a physical GPU (or software renderer).
 */
typedef struct DISPLAY_ADAPTER_DESC {
    uint64_t          Identifier;                                              /* The unique identifier of the adapter. This value identifies the adapter until the system is rebooted. */
    uint32_t          PerformanceLevel;                                        /* An integer value representing the performance capability of the adapter, with higher values representing higher powered devices. */
} DISPLAY_ADAPTER_DESC;

/* @summary Define the data describing a display output, such as a monitor, attached to a display adapter.
 */
typedef struct DISPLAY_OUTPUT_DESC {
    uint64_t          Identifier;                                              /* The unique identifer of the display output. This value identifies the output until the system is rebooted. */
    int32_t           VirtualX;                                                /* The location of the upper-left corner of the output, in virtual display space. For the primary display, this value is always zero. */
    int32_t           VirtualY;                                                /* The location of the upper-left corner of the output, in virtual display space. For the primary display, this value is always zero. */
    uint32_t          ActualWidth;                                             /* The width of the current display mode, in physical pixels. */
    uint32_t          ActualHeight;                                            /* The height of the current display mode, in physical pixels. */
    uint32_t          WorkingWidth;                                            /* The available width of the display, in physical pixels, accounting for UI elements such as the taskbar. */
    uint32_t          WorkingHeight;                                           /* The available height of the display, in physical pixels, accounting for UI elements such as the taskbar. */
    uint32_t          DisplayDpiX;                                             /* The horizontal dots-per-inch setting of the display. */
    uint32_t          DisplayDpiY;                                             /* The vertical dots-per-inch setting of the display. */
} DISPLAY_OUTPUT_DESC;

/* @summary Define the data used to configure the display system.
 */
typedef struct DISPLAY_SYSTEM_INIT {
    char const       *ApplicationName;                                         /* A nul-terminated string specifying the name of the application. */
    uint32_t          ApplicationVersionMajor;                                 /* The application's major version number. */
    uint32_t          ApplicationVersionMinor;                                 /* The application's minor version number. */
    char const      **InterfaceList;                                           /* An array of nul-terminated strings specifying the interfaces to use, in order of preference. */
    uint32_t          InterfaceCount;                                          /* The number of nul-terminated strings in the InterfaceList array. */
    uint32_t          CreateFlags;                                             /* One or more bitwise OR'd values of the DISPLAY_SYSTEM_CREATE_FLAGS enumeration. */
    /* max memory usage to init arenas? */
} DISPLAY_SYSTEM_INIT;

/* @summary Define the data used to construct a logical GPU device interface.
 */
typedef struct GPU_DEVICE_INIT {
    uint64_t          AdapterIdentifier;                                       /* The identifier of the display adapter to use for GPU operations. */
    uint32_t          UsageFlags;                                              /* One or more bitwise OR'd values of the GPU_DEVICE_USAGE_FLAGS enumeration specifying the intended usage for the device. */
} GPU_DEVICE_INIT;

/* @summary Define the data used to create a GPU-driven window.
 */
typedef struct GPU_WINDOW_INIT {
    char const       *WindowTitle;                                             /* A nul-terminated string specifying the window title. */
    uint64_t          OutputIdentifier;                                        /* The identifier of the display output on which the window should be created. */
    uint32_t          ClientWidth;                                             /* The desired width of the window client area, in physical pixels. Specify zero to use the entire display area. */
    uint32_t          ClientHeight;                                            /* The desired height of the window client area, in physical pixels. Specify zero to use the entire display area. */
    GPU_DEVICE_HANDLE GpuDevice;                                               /* The handle of the GPU device which will drive window rendering and presentation. */
} GPU_WINDOW_INIT;

/* @summary Define the data reported about the state of a GPU-driven window.
 */
typedef struct GPU_WINDOW_STATE {
    uint32_t          StatusFlags;                                             /* One or more bitwise OR'd values from the GPU_WINDOW_STATUS_FLAGS enumeration indicating the window status. */
    uint32_t          EventFlags;                                              /* One or more bitwise OR'd values from the GPU_WINDOW_EVENT_FLAGS enumeration indicating the events that occurred during the most recent update. */
    uint32_t          OutputDpiX;                                              /* The horizontal dots-per-inch setting of the display output containing the majority of the window. This value can be used to convert logical size values to physical size. */
    uint32_t          OutputDpiY;                                              /* The vertical dots-per-inch setting of the display output containing the majority of the window, This value can be used to convert logical size values to physical size. */
    int32_t           WindowPositionX;                                         /* The x-coordinate of the upper-left corner of the outer edge of the window, in virtual display space. */
    int32_t           WindowPositionY;                                         /* The y-coordinate of the upper-left corner of the outer edge of the window, in virtual display space. */
    uint32_t          WindowSizeX;                                             /* The horizontal size of the window, in logical pixels. */
    uint32_t          WindowSizeY;                                             /* The vertical size of the window, in logical pixels. */
    uint32_t          ClientSizeX;                                             /* The horizontal size of the window client area, in logical pixels. */
    uint32_t          ClientSizeY;                                             /* The vertical size of the window client area, in logical pixels. */
} GPU_WINDOW_STATE;

/* @summary Define a series of flags that can be bitwise OR'd together to describe special behavior requested by the application when initializing the display system.
 */
typedef enum DISPLAY_SYSTEM_CREATE_FLAGS {
    DISPLAY_SYSTEM_CREATE_FLAGS_NONE       = (0UL << 0),                       /* No special behavior is requested. */
    DISPLAY_SYSTEM_CREATE_FLAG_DEBUG       = (1UL << 0),                       /* Enable debugging support at a potential performance cost. */
} DISPLAY_SYSTEM_CREATE_FLAGS;

/* @summary Define a series of flags that can be bitwise OR'd together to describe how an application will use a GPU device.
 */
typedef enum GPU_DEVICE_USAGE_FLAGS {
    GPU_DEVICE_USAGE_FLAGS_NONE            = (0UL << 0),                       /* The GPU device will not be used at all. */
    GPU_DEVICE_USAGE_FLAG_GRAPHICS         = (1UL << 0),                       /* The GPU device will be used for graphics rendering operations. */
    GPU_DEVICE_USAGE_FLAG_COMPUTE          = (1UL << 1),                       /* The GPU device will be used for compute operations. */
} GPU_DEVICE_USAGE_FLAGS;

/* @summary Define a series of flags that can be bitwise OR'd together to describe the events received by a window during an update.
 */
typedef enum GPU_WINDOW_EVENT_FLAGS {
    GPU_WINDOW_EVENT_FLAGS_NONE            = (0UL <<  0),                      /* No events were received by the window. */
    GPU_WINDOW_EVENT_FLAG_CREATED          = (1UL <<  0),                      /* The window was just created. */
    GPU_WINDOW_EVENT_FLAG_DESTROYED        = (1UL <<  1),                      /* The window was just destroyed. */
    GPU_WINDOW_EVENT_FLAG_SHOWN            = (1UL <<  2),                      /* The window was just made visible. */
    GPU_WINDOW_EVENT_FLAG_HIDDEN           = (1UL <<  3),                      /* The window was just hidden. */
    GPU_WINDOW_EVENT_FLAG_ACTIVATED        = (1UL <<  4),                      /* The window just received input focus. */
    GPU_WINDOW_EVENT_FLAG_DEACTIVATED      = (1UL <<  5),                      /* The window just lost input focus. */
    GPU_WINDOW_EVENT_FLAG_SIZE_CHANGED     = (1UL <<  6),                      /* The window size was just changed. */
    GPU_WINDOW_EVENT_FLAG_POSITION_CHANGED = (1UL <<  7),                      /* The window position was just changed. */
} GPU_WINDOW_EVENT_FLAGS;

/* @summary Define a series of flags that can be bitwise OR'd together to describe the current state of a window.
 */
typedef enum GPU_WINDOW_STATUS_FLAGS {
    GPU_WINDOW_STATUS_FLAGS_NONE           = (0UL <<  0),                      /* The window has been destroyed. */
    GPU_WINDOW_STATUS_FLAG_CREATED         = (1UL <<  0),                      /* The window has been created. */
    GPU_WINDOW_STATUS_FLAG_ACTIVE          = (1UL <<  1),                      /* The window is active and has input focus. */
    GPU_WINDOW_STATUS_FLAG_VISIBLE         = (1UL <<  2),                      /* The window is currently visible on some display output. */
    GPU_WINDOW_STATUS_FLAG_FULLSCREEN      = (1UL <<  3),                      /* The window is currently in fullscreen mode. */
} GPU_WINDOW_STATUS_FLAGS;

#ifdef __cplusplus
extern "C" {
#endif

/* @summary Convert a value specified in physical pixels to a value specified in logical pixels.
 * @param dimension The value to convert, specified in physical pixels.
 * @param dots_per_inch The DPI setting of the display output.
 * @return The corresponding value in logical pixels.
 */
PIL_API(uint32_t)
PhysicalToLogicalPixels
(
    uint32_t     dimension, 
    uint32_t dots_per_inch
);

/* @summary Convert a value specified in logical pixels to a value specified in physical pixels.
 * @param dimension The value to convert, specified in logical pixels.
 * @param dots_per_inch The DPI setting of the display output.
 * @return The corresponding value in physical pixels.
 */
PIL_API(uint32_t)
LogicalToPhysicalPixels
(
    uint32_t     dimension, 
    uint32_t dots_per_inch
);

/* @summary Allocate and initialize a DISPLAY_SYSTEM interface for the application.
 * This enumerates all display adapters and outputs attached to the system.
 * The DISPLAY_SYSTEM is owned by the PIL_CONTEXT, and is only destroyed when the context is deleted.
 * @param init Data used to configure the creation of the display system.
 * @return A pointer to the DISPLAY_SYSTEM object, or NULL.
 */
PIL_API(struct DISPLAY_SYSTEM*)
DisplaySystemCreate
(
    struct DISPLAY_SYSTEM_INIT *init
);

/* @summary Free resources associated with a DISPLAY_SYSTEM interface.
 * @param system The DISPLAY_SYSTEM to delete.
 */
PIL_API(void)
DisplaySystemDelete
(
    struct DISPLAY_SYSTEM *system
);

/* @summary Retrieve descriptions of the display adapters (physical GPUs) attached to the host.
 * Access to the DISPLAY_SYSTEM is externally synchronized and must be managed by the application.
 * 
 * To retrieve the number of attached display adapters, use:
 * uint32_t num_adapters = DescribeDisplayAdapters(NULL, 0, 0, system);
 *
 * Alternatively, you can retrieve one adapter at a time:
 * uint32_t            index = 0;
 * DISPLAY_ADAPTER_DESC desc;
 * while (DescribeDisplayAdapters(&desc, index, 1, system)) {
 *     // do something with desc
 *     index++;
 * }
 *
 * @param o_desc_array The array of DISPLAY_ADAPTER_DESC structures to populate.
 * @param start_index The zero-based index of the first item to return.
 * @param max_results The maximum number of DISPLAY_ADAPTER_DESC structures that can be written to o_desc_array.
 * @param system The DISPLAY_SYSTEM interface to query.
 * @return The number of items written to o_desc_array, or if o_desc_array is NULL and max_results is 0, the number of display adapters attached to the host.
 */
PIL_API(uint32_t)
DescribeDisplayAdapters
(
    struct DISPLAY_ADAPTER_DESC *o_desc_array, 
    uint32_t                      start_index, 
    uint32_t                      max_results, 
    struct DISPLAY_SYSTEM             *system
);

/* @summary Retrieve descriptions of the display outputs (monitors) attached to the host.
 * Access to the DISPLAY_SYSTEM is externally synchronized and must be managed by the application.
 * 
 * To retrieve the number of attached display outputs, use:
 * uint32_t num_outputs = DescribeDisplayOutputs(NULL, 0, 0, system);
 *
 * Alternatively, you can retrieve one output at a time:
 * uint32_t           index = 0;
 * DISPLAY_OUTPUT_DESC desc;
 * while (DescribeDisplayOutputs(&desc, index, 1, system)) {
 *     // do something with desc
 *     index++;
 * }
 *
 * @param o_desc_array The array of DISPLAY_OUTPUT_DESC structures to populate.
 * @param start_index The zero-based index of the first item to return.
 * @param max_results The maximum number of DISPLAY_OUTPUT_DESC structures that can be written to o_desc_array.
 * @param system The DISPLAY_SYSTEM interface to query.
 * @return The number of items written to o_desc_array, or if o_desc_array is NULL and max_results is 0, the number of display outputs attached to the host.
 */
PIL_API(uint32_t)
DescribeDisplayOutputs
(
    struct DISPLAY_OUTPUT_DESC *o_desc_array, 
    uint32_t                     start_index, 
    uint32_t                     max_results, 
    struct DISPLAY_SYSTEM            *system
);

/* @summary Create a logical GPU device to use for rendering or compute operations.
 * Access to the DISPLAY_SYSTEM is externally synchronized and must be managed by the application.
 * @param o_handle On return, this location is updated with the GPU device handle.
 * @param init Data used to configure the creation of the GPU device.
 * @param system The DISPLAY_SYSTEM interface that will manage the logical GPU device.
 * @return Zero if the GPU device is created successfully, or non-zero if an error occurred.
 */
PIL_API(int)
GpuDeviceCreate
(
    struct GPU_DEVICE_HANDLE *o_handle, 
    struct GPU_DEVICE_INIT       *init, 
    struct DISPLAY_SYSTEM      *system 
);

/* @summary Destroy a logical GPU device.
 * Access to the DISPLAY_SYSTEM is externally synchronized and must be managed by the application.
 * @param handle The handle of the GPU device to delete.
 * @param system The DISPLAY_SYSTEM interface managing the logical GPU device.
 */
PIL_API(void)
GpuDeviceDelete
(
    struct GPU_DEVICE_HANDLE handle, 
    struct DISPLAY_SYSTEM   *system
);

/* @summary Create a GPU-driven window.
 * Access to the DISPLAY_SYSTEM is externally synchronized and must be managed by the application.
 * @param o_handle On return, this location is updated with the GPU window handle.
 * @param init Data used to configure the window.
 * @param system The DISPLAY_SYSTEM interface that will manage the window.
 * @return Zero if the window is created successfully, or non-zero if an error occurred.
 */
PIL_API(int)
GpuWindowCreate
(
    struct GPU_WINDOW_HANDLE *o_handle, 
    struct GPU_WINDOW_INIT       *init, 
    struct DISPLAY_SYSTEM      *system
);

/* @summary Destroy a GPU-driven window.
 * Access to the display system is externally synchronized and must be managed by the application.
 * @param handle The handle of the GPU window to delete.
 * @param system The DISPLAY_SYSTEM interface managing the GPU window.
 */
PIL_API(void)
GpuWindowDelete
(
    struct GPU_WINDOW_HANDLE handle, 
    struct DISPLAY_SYSTEM   *system
);

/* @summary Receive and dispatch events for all GPU windows managed by the display system interface.
 * Access to the display system is externally synchronized and must be managed by the application.
 * @param system The DISPLAY_SYSTEM interface managing the GPU windows to update.
 */
PIL_API(void)
ProcessGpuWindowEvents
(
    struct DISPLAY_SYSTEM *system
);

/* @summary Retrieve the most up-to-date description of the current state of a window.
 * Access to the display system is externally synchronized and must be managed by the application.
 * @param o_state Pointer to the GPU_WINDOW_STATE structure to populate.
 * @param handle The handle of the window to query.
 * @param system The DISPLAY_SYSTEM interface managing the window.
 * @return Zero if the state is written to o_state, or non-zero if an error occurred.
 */
PIL_API(int)
QueryGpuWindowState
(
    struct GPU_WINDOW_STATE *o_state, 
    struct GPU_WINDOW_HANDLE  handle, 
    struct DISPLAY_SYSTEM    *system
);

#ifdef __cplusplus
}; /* extern "C" */
#endif

#endif /* __PIL_DISPLAY_H__ */

