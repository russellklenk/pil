/**
 * @summary Define the Win32-specific types associated with the display 
 * interface portion of the Platform Interface Layer.
 */
#ifndef __PIL_DISPLAY_WIN32_H__
#define __PIL_DISPLAY_WIN32_H__

#pragma once

#ifndef PIL_NO_INCLUDES
#   include <Windows.h>
#   include <Dxgi.h>
#   include <Dxgi1_3.h>
#endif

/* @summary Define the internal information associated with a display output.
 */
typedef struct DISPLAY_OUTPUT {
    IDXGIOutput3        *OutputIf;
    uint32_t             ModeCount;
    uint32_t             CurrentMode;
    DXGI_MODE_DESC1     *DisplayModes;
    UINT                 DisplayDpiX;
    UINT                 DisplayDpiY;
    DXGI_OUTPUT_DESC     OutputInfo;
} DISPLAY_OUTPUT;

/* @summary Define the internal information associated with a display adapter (GPU).
 */
typedef struct DISPLAY_ADAPTER {
    IDXGIAdapter        *AdapterIf;                                            /* */
} DISPLAY_ADAPTER;

/* @summary Define the information maintained by a display interface associated with a PIL context.
 * The display interface receives events when GPU devices and display outputs are attached and removed.
 */
typedef struct DISPLAY_INTERFACE {
    HWND                 NotifyWindow;                                         /* The handle of the notification window. */
    IDXGIFactory2       *FactoryIf;                                            /* The DXGI object factory interface. */
    uint32_t             OutputCount;                                          /* The number of valid outputs in the OutputList array. */
    uint32_t             AdapterCount;                                         /* The number of valid adapters in the AdapterList array. */
    DISPLAY_OUTPUT       OutputList   [PIL_MAX_DISPLAY_OUTPUTS ];              /* */
    DISPLAY_ADAPTER      AdapterList  [PIL_MAX_DISPLAY_ADAPTERS];              /* */
} DISPLAY_INTERFACE;

#endif /* __PIL_DISPLAY_WIN32_H__ */

