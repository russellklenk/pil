/**
 * @summary dxgiapi_win32.h: Define an interface used for dynamically loading 
 * Dxgi.dll into the process address space and resolving available entry points.
 * This is needed because different versions of Windows provide different 
 * versions of DXGI, and applications generally want to avoid statically linking
 * to a specific version.
 */
#ifndef __PIL_DXGIAPI_WIN32_H__
#define __PIL_DXGIAPI_WIN32_H__

#pragma once

#ifndef PIL_NO_INCLUDES
#   ifndef __PIL_H__
#       include "pil.h"
#   endif
#   ifndef __PIL_DYNLIB_H__
#       include "dynlib.h"
#   endif
#   include <Windows.h>
#   include <dxgi.h>
#   include <dxgi1_6.h>
#   include <dxgidebug.h>
#endif

/**
 * Function pointer types.
 */
typedef HRESULT (WINAPI *PFN_CreateDXGIFactory               )(REFIID, void**);
typedef HRESULT (WINAPI *PFN_CreateDXGIFactory1              )(REFIID, void**);
typedef HRESULT (WINAPI *PFN_CreateDXGIFactory2              )(UINT  , REFIID, void**);
typedef HRESULT (WINAPI *PFN_DXGIGetDebugInterface           )(REFIID, void**);
typedef HRESULT (WINAPI *PFN_DXGIGetDebugInterface1          )(UINT  , REFIID, void**);
typedef HRESULT (WINAPI *PFN_DXGIDeclareAdapterRemovalSupport)(void);

/* @summary Define the data associated with the dispatch table used to call functions from Dxgi.dll and DxgiDebug.dll.
 */
typedef struct DXGIAPI_DISPATCH {
    PFN_CreateDXGIFactory                CreateDXGIFactory;                    /* Available with Windows Vista or later. */
    PFN_CreateDXGIFactory1               CreateDXGIFactory1;                   /* Available with Windows 7 or later. */
    PFN_CreateDXGIFactory2               CreateDXGIFactory2;                   /* Available with Windows 8.1 or later. */
    PFN_DXGIGetDebugInterface            DXGIGetDebugInterface;                /* Available with Windows 8 or later. */
    PFN_DXGIGetDebugInterface1           DXGIGetDebugInterface1;               /* Available with Windows 8.1 or later. */
    PFN_DXGIDeclareAdapterRemovalSupport DXGIDeclareAdapterRemovalSupport;     /* Available with DXGI 1.6 or later (Windows 10 Spring 2018). */
    RUNTIME_MODULE                       ModuleHandle_DxgiDebug;               /* The handle of DxgiDebug.dll for the process. */
    RUNTIME_MODULE                       ModuleHandle_Dxgi;                    /* The handle of Dxgi.dll for the process. */
} DXGIAPI_DISPATCH;

/* @summary Define a series of flags that can be bitwise OR'd together to control loader behavior.
 */
typedef enum DXGIAPI_LOADER_FLAGS {
    DXGIAPI_LOADER_FLAGS_NONE            = (0UL << 0),                         /* No special behavior is requested. */
    DXGIAPI_LOADER_FLAG_DEBUG_SUPPORT    = (1UL << 0),                         /* Attempt to load DxgiDebug.dll. Requires the Windows SDK to be installed. */
} DXGIAPI_LOADER_FLAGS;

#ifdef __cplusplus
extern "C" {
#endif

/* @summary Load Dxgi.dll (and DxgiDebug.dll if requested) into the process address space and resolve entry points.
 * Any missing entry points are set to stub functions, so none of the function pointers will be NULL.
 * @param dispatch The dispatch table to populate.
 * @param loader_flags One or more values of the DXGIAPI_LOADER_FLAGS enumeration.
 * @return Zero if the dispatch table is successfully populated.
 */
PIL_API(int)
DXGIApiPopulateDispatch
(
    struct DXGIAPI_DISPATCH *dispatch, 
    uint32_t             loader_flags 
);

/* @summary Determine whether the DXGI API is supported on the host.
 * @param dispatch The dispatch table to query.
 * @return Non-zero if the DXGI API is supported on the host, or zero otherwise.
 */
PIL_API(int)
DXGIApiQuerySupport
(
    struct DXGIAPI_DISPATCH *dispatch
);

/* @summary Free resources associated with a dispatch table.
 * This function invalidates the entry points associated with the dispatch table.
 * @param dispatch The dispatch table to invalidate.
 */
PIL_API(void)
DXGIApiInvalidateDispatch
(
    struct DXGIAPI_DISPATCH *dispatch
);

#ifdef __cplusplus
}; /* extern "C" */
#endif

#endif /* __PIL_DXGIAPI_WIN32_H__ */

