/**
 * @summary win32api_win32.h: Define an interface used for dynamically loading 
 * various Windows DLLs related to desktop application development into 
 * the process address space and resolving entry points. This is needed to 
 * allow an application to load on an older version of Windows, where some 
 * needed entry points might not be available.
 */
#ifndef __PIL_WIN32API_WIN32_H__
#define __PIL_WIN32API_WIN32_H__

#pragma once

#ifndef PIL_NO_INCLUDES
#   ifndef __PIL_H__
#       include "pil.h"
#   endif
#   ifndef __PIL_DYNLIB_H__
#       include "dynlib.h"
#   endif
#   include <Windows.h>
#   include <shellapi.h>
#   include <ShellScalingApi.h>
#endif

/**
 * Function pointer types.
 */
typedef HRESULT (WINAPI *PFN_SetProcessDpiAwareness)(PROCESS_DPI_AWARENESS);
typedef HRESULT (WINAPI *PFN_GetDpiForMonitor      )(HMONITOR, MONITOR_DPI_TYPE, UINT*, UINT*);

/* @summary Define the data associated with the dispatch table used to call runtime-resolved Windows APIs.
 */
typedef struct WIN32API_DISPATCH {
    PFN_SetProcessDpiAwareness SetProcessDpiAwareness;                         /* The SetProcessDpiAwareness entry point. */
    PFN_GetDpiForMonitor       GetDpiForMonitor;                               /* The GetDpiForMonitor entry point. */
    RUNTIME_MODULE             ModuleHandle_Shcore;                            /* The module handle for Shcore.dll. */
} WIN32API_DISPATCH;

/* @summary Define a series of flags that can be bitwise OR'd together to control loader behavior.
 */
typedef enum WIN32API_LOADER_FLAGS {
    WIN32API_LOADER_FLAGS_NONE     = (0UL << 0),                               /* No special behavior is required. */
} WIN32API_LOADER_FLAGS;

#ifdef __cplusplus
extern "C" {
#endif

/* @summary Load DLLs into the process address space and resolve entry points.
 * Any missing entry points are set to stub functions, so none of the function pointers will be NULL.
 * @param dispatch The dispatch table to populate.
 * @param loader_flags One or more values of the WIN32API_LOADER_FLAGS enumeration.
 * @return Zero if the dispatch table is successfully populated.
 */
PIL_API(int)
Win32ApiPopulateDispatch
(
    struct WIN32API_DISPATCH *dispatch, 
    uint32_t              loader_flags
);

/* @summary Determine whether the Windows API is supported on the host.
 * @param dispatch The dispatch table to query.
 * @return Non-zero if the Windows API is supported on the host, or zero otherwise.
 */
PIL_API(int)
Win32ApiQuerySupport
(
    struct WIN32API_DISPATCH *dispatch
);

/* @summary Free resources associated with a dispatch table.
 * This function invalidates the entry points associated with the dispatch table.
 * @param dispatch The dispatch table to invalidate.
 */
PIL_API(void)
Win32ApiInvalidateDispatch
(
    struct WIN32API_DISPATCH *dispatch
);

#ifdef __cplusplus
}; /* extern "C" */
#endif

#endif /* __PIL_WIN32API_WIN32_H__ */

