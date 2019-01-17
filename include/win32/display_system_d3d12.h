/**
 * @summary Define the types relating to a display system implementation for 
 * Microsoft Windows 10 and later platforms utilizing DXGI and Direct3D 12 to 
 * interface with graphics hardware.
 */
#ifndef __PIL_DISPLAY_SYSTEM_D3D12_H__
#define __PIL_DISPLAY_SYSTEM_D3D12_H__

#pragma once

#ifndef PIL_NO_INCLUDES
#   include "win32/dxgiapi_win32.h"
#   include "win32/d3d12api_win32.h"
#   include "win32/win32api_win32.h"
#   include "win32/display_device_d3d12.h"
#   include "win32/display_window_d3d12.h"
#endif

/* @summary Define constants relating to the Direct3D 12 display system implementation.
 * DISPLAY_SYSTEM_D3D12_MAX_ADAPTERS       : The maximum number of display adapters recognized by the implementation.
 * DISPLAY_SYSTEM_D3D12_MAX_OUTPUTS        : The maximum number of display outputs recognized by the implementation.
 * DISPLAY_SYSTEM_D3D12_MAX_WINDOWS        : The maximum number of GPU-driven windows supported by the implementation.
 * DISPLAY_SYSTEM_D3D12_MAX_LOGICAL_DEVICES: The maximum number of logical GPU devices supported by the implementation.
 */
#ifndef DISPLAY_SYSTEM_D3D12_CONSTANTS
#   define DISPLAY_SYSTEM_D3D12_CONSTANTS
#   define DISPLAY_SYSTEM_D3D12_MAX_ADAPTERS                                   32
#   define DISPLAY_SYSTEM_D3D12_MAX_OUTPUTS                                    32
#   define DISPLAY_SYSTEM_D3D12_MAX_WINDOWS                                    64
#   define DISPLAY_SYSTEM_D3D12_MAX_LOGICAL_DEVICES                            64
#endif

/* @summary Helper macroto deduce the number of items in a static array.
 * @param _array A static array declared similar to the following example:
 * D3D12_FEATURE_LEVEL feature_levels[] = {
 *     D3D_FEATURE_LEVEL_12_1, 
 *     D3D_FEATURE_LEVEL_12_0, 
 *     D3D_FEATURE_LEVEL_11_1, 
 *     D3D_FEATURE_LEVEL_11_0
 *  };
 *  Call as uint32_t const FEATURE_LEVEL_COUNT = D3D12_CountOf(feature_levels);
 */
#ifndef D3D12_CountOf
#define D3D12_CountOf(_array)                                                  \
    (sizeof(_array) / sizeof((_array)[0]))
#endif

/* @summary Retrieve the DXGI debug interface.
 * @param _system A pointer to the DISPLAY_SYSTEM_D3D12 to query.
 * @return A pointer to the IDXGIDebug COM interface, or nullptr if debug support is not available.
 */
#ifndef D3D12_GetDxgiDebug
#define D3D12_GetDxgiDebug(_system)                                            \
    (_system)->DxgiDebug
#endif

/* @summary Retrieve the enhanced DXGI debug interface.
 * @param _system A pointer to the DISPLAY_SYSTEM_D3D12 to query.
 * @return A pointer to the IDXGIDebug1 COM interface, or nullptr if debug support is not available.
 */
#ifndef D3D12_GetDxgiDebug1
#define D3D12_GetDxgiDebug1(_system)                                           \
    (_system)->DxgiDebug1
#endif

/* @summary Retrieve the Direct3D 12 debug interface.
 * @param _system A pointer to the DISPLAY_SYSTEM_D3D12 to query.
 * @return A pointer to the ID3D12Debug COM interface, or nullptr if debug support is not available.
 */
#ifndef D3D12_GetD3D12Debug
#define D3D12_GetD3D12Debug(_system)                                           \
    (_system)->D3D12Debug
#endif

/* @summary Retrieve the DXGI factory interface.
 * @param _system A pointer to the DISPLAY_SYSTEM_D3D12 to query.
 * @return A pointer to the IDXGIFactory COM interface.
 */
#ifndef D3D12_GetDxgiFactory
#define D3D12_GetDxgiFactory(_system)                                          \
    (_system)->DxgiFactory
#endif

/* @summary Retrieve the feature flags supported by the Direct3D runtime on the host.
 * @param _system A pointer to the DISPLAY_SYSTEM_D3D12 to query.
 * @return One or more bitwise OR'd values of the D3D12_SYSTEM_FEATURE_FLAGS enumeration.
 */
#ifndef D3D12_GetFeatureFlags
#define D3D12_GetFeatureFlags(_system)                                         \
    (_system)->FeatureFlags
#endif

/* @summary Retrieve a pointer to the DXGI function dispatch table.
 * @param _system A pointer to the DISPLAY_SYSTEM_D3D12 to query.
 * @return A pointer to an instance of DXGIAPI_DISPATCH which can be used to call DXGI functions.
 */
#ifndef D3D12_GetDxgiDispatch
#define D3D12_GetDxgiDispatch(_system)                                         \
    &(_system)->Dispatch_DXGI
#endif

/* @summary Retrieve a pointer to the Direct3D 12 function dispatch table.
 * @param _system A pointer to the DISPLAY_SYSTEM_D3D12 to query.
 * @return A pointer to an instance of D3D12API_DISPATCH which can be used to call Direct3D 12 functions.
 */
#ifndef D3D12_GetD3D12Dispatch
#define D3D12_GetD3D12Dispatch(_system)                                        \
    &(_system)->Dispatch_D3D12
#endif

/* @summary Retrieve a pointer to the Windows API function dispatch table.
 * @param _system A pointer to the DISPLAY_SYSTEM_D3D12 to query.
 * @return A pointer to an instance of WIN32API_DISPATCH which can be used to call Windows API functions.
 */
#ifndef D3D12_GetWin32Dispatch
#define D3D12_GetWin32Dispatch(_system)                                        \
    &(_system)->Dispatch_Win32
#endif

/* @summary Retrieve a pointer to the table of logical GPU devices created by the application.
 * @param _system A pointer to the DISPLAY_SYSTEM_D3D12 to query.
 * @return A pointer to an instance of DISPLAY_DEVICE_TABLE_D3D12 which maintains state for logical GPU devices.
 */
#ifndef D3D12_GetDeviceTable
#define D3D12_GetDeviceTable(_system)                                          \
    &(_system)->DeviceTable
#endif

/* @summary Retrieve a pointer to the table of GPU-driven windows created by the application.
 * @param _system A pointer to the DISPLAY_SYSTEM_D3D12 to query.
 * @return A pointer to an instance of DISPLAY_WINDOW_TABLE_D3D12 which maintains state for GPU-driven windows.
 */
#ifndef D3D12_GetWindowTable
#define D3D12_GetWindowTable(_system)                                          \
    &(_system)->WindowTable
#endif

/* @summary Retrieve a pointer to the table of attached display outputs (monitors).
 * @param _system A pointer to the DISPLAY_SYSTEM_D3D12 to query.
 * @return A pointer to an instance of DISPLAY_OUTPUT_TABLE_D3D12 which maintains state for attached displays.
 */
#ifndef D3D12_GetDisplayOutputTable
#define D3D12_GetDisplayOutputTable(_system)                                   \
    &(_system)->OutputTable
#endif

/* @summary Retrieve a pointer to the table of attached display adapters (physical GPU devices).
 * @param _system A pointer to the DISPLAY_SYSTEM_D3D12 to query.
 * @return A pointer to an instance of DISPLAY_ADAPTER_TABLE_D3D12 which maintains state for attached physical GPU devices.
 */
#ifndef D3D12_GetDisplayAdapterTable
#define D3D12_GetDisplayAdapterTable(_system)                                  \
    &(_system)->AdapterTable
#endif

/* @summary Define the data maintained by the Direct3D 12 display system implementation.
 */
typedef struct DISPLAY_SYSTEM_D3D12 {
    IDXGIDebug                 *DxgiDebug;                                     /* The interface used to control DXGI debug features. Will be nullptr on systems where debug features are not supported. */
    IDXGIDebug1                *DxgiDebug1;                                    /* The interface used to control extended DXGI debug features. Will be nullptr on systems where debug features are not supported. */
    ID3D12Debug                *D3D12Debug;                                    /* The interface used to control Direct3D debug features. Will be nullptr on systems where debug features are not supported. */
    IDXGIFactory2              *DxgiFactory;                                   /* The interface used to create DXGI resources such as swapchains, and to enumerate attached display adapters and outputs. */
    uint32_t                    FeatureFlags;                                  /* One or more bitwise OR'd values of the DISPLAY_SYSTEM_FEATURE_FLAGS_D3D12 enumeration describing the capabilities of the Direct3D runtime on the host. */
    uint32_t                    Reserved0;                                     /* Reserved for future use. Set to zero. */
    DXGIAPI_DISPATCH            Dispatch_DXGI;                                 /* A dispatch table used to call DXGI entry points loaded at runtime. */
    D3D12API_DISPATCH           Dispatch_D3D12;                                /* A dispatch table used to call Direct3D 12 entry points loaded at runtime. */
    WIN32API_DISPATCH           Dispatch_Win32;                                /* A dispatch table used to call Windows API entry points loaded at runtime. */
    DISPLAY_DEVICE_TABLE_D3D12  DeviceTable;                                   /* The table of logical GPU devices (GPU_DEVICE_HANDLE). See display_device_d3d12.h|.cc. */
    DISPLAY_WINDOW_TABLE_D3D12  WindowTable;                                   /* The table of GPU-driven windows (GPU_WINDOW_HANDLE). See display_window_d3d12.h|.cc. */
    DISPLAY_OUTPUT_TABLE_D3D12  OutputTable;                                   /* The table of information about the display outputs attached to the host. See display_device_d3d12.h|.cc. */
    DISPLAY_ADAPTER_TABLE_D3D12 AdapterTable;                                  /* The table of information about the display adapters (physical GPUs) attached to the host. See display_device_d3d12.h|.cc. */
} DISPLAY_SYSTEM_D3D12;

/* @summary A set of flags that can be bitwise OR'd together to describe the features of the Direct3D runtime on the host.
 */
typedef enum DISPLAY_SYSTEM_FEATURE_FLAGS_D3D12 {
    DISPLAY_SYSTEM_FEATURE_FLAGS_D3D12_NONE     = (0UL << 0),                  /* No optional features are supported. */
    DISPLAY_SYSTEM_FEATURE_FLAG_D3D12_TEARING   = (1UL << 0),                  /* The system supports unthrottled presentation. */
} DISPLAY_SYSTEM_FEATURE_FLAGS_D3D12;

#endif /* __PIL_DISPLAY_SYSTEM_D3D12_H__ */

