/**
 * @summary d3d12api_win32.h: Define an interface used for dynamically loading 
 * D3D12.dll into the process address space and resolving available entry 
 * points. This is needed so the application can run on earlier versions of 
 * Windows that may not provide Direct3D 12 support, and gracefully fall back 
 * to some other supported graphics interface.
 */
#ifndef __PIL_D3D12API_WIN32_H__
#define __PIL_D3D12API_WIN32_H__

#pragma once

#ifndef PIL_NO_INCLUDES
#   ifndef __PIL_H__
#       include "pil.h"
#   endif
#   ifndef __PIL_DYNLIB_H__
#       include "dynlib.h"
#   endif
#   include <Windows.h>
#   include <d3d12.h>
#endif

/**
 * Function pointer types.
 */
typedef HRESULT (WINAPI *PFN_D3D12CreateDevice                            )(IUnknown*, D3D_FEATURE_LEVEL, REFIID, void**);
typedef HRESULT (WINAPI *PFN_D3D12CreateRootSignatureDeserializer         )(LPCVOID, SIZE_T, REFIID, void**);
typedef HRESULT (WINAPI *PFN_D3D12CreateVersionedRootSignatureDeserializer)(LPCVOID, SIZE_T, REFIID, void**);
typedef HRESULT (WINAPI *PFN_D3D12EnableExperimentalFeatures              )(UINT, IID const*, void*, UINT*);
typedef HRESULT (WINAPI *PFN_D3D12GetDebugInterface                       )(REFIID, void**);
typedef HRESULT (WINAPI *PFN_D3D12SerializeRootSignature                  )(D3D12_ROOT_SIGNATURE_DESC const*, D3D_ROOT_SIGNATURE_VERSION, ID3DBlob**, ID3DBlob**);
typedef HRESULT (WINAPI *PFN_D3D12SerializeVersionedRootSignature         )(D3D12_VERSIONED_ROOT_SIGNATURE_DESC const*, ID3DBlob**, ID3DBlob**);

/* @summary Define the data associated with the dispatch table used to call functions from D3D12.dll.
 */
typedef struct D3D12API_DISPATCH {
    PFN_D3D12CreateDevice                             D3D12CreateDevice;
    PFN_D3D12CreateRootSignatureDeserializer          D3D12CreateRootSignatureDeserializer;
    PFN_D3D12CreateVersionedRootSignatureDeserializer D3D12CreateVersionedRootSignatureDeserializer;
    PFN_D3D12EnableExperimentalFeatures               D3D12EnableExperimentalFeatures;
    PFN_D3D12GetDebugInterface                        D3D12GetDebugInterface;
    PFN_D3D12SerializeRootSignature                   D3D12SerializeRootSignature;
    PFN_D3D12SerializeVersionedRootSignature          D3D12SerializeVersionedRootSignature;
    RUNTIME_MODULE                                    ModuleHandle_D3D12;
} D3D12API_DISPATCH;

/* @summary Define a series of flags that can be bitwise OR'd together to control loader behavior.
 */
typedef enum D3D12API_LOADER_FLAGS {
    D3D12API_LOADER_FLAGS_NONE                        = (0UL << 0),            /* No special behavior is requested. */
} D3D12API_LOADER_FLAGS;

#ifdef __cplusplus
extern "C" {
#endif

/* @summary Load D3D12.dll into the process address space and resolve entry points.
 * Any missing entry points are set to stub functions, so none of the function pointers will be NULL.
 * @param dispatch The dispatch table to populate.
 * @param loader_flags One or more values of the D3D12API_LOADER_FLAGS enumeration.
 * @return Zero if the dispatch table is successfully populated.
 */
PIL_API(int)
D3D12ApiPopulateDispatch
(
    struct D3D12API_DISPATCH *dispatch, 
    uint32_t              loader_flags
);

/* @summary Determine whether Direct3D 12 is available on the host system.
 * @param dispatch The dispatch table to query, which must have been previously populated by D3D12ApiPopulateDispatch.
 * @return Non-zero if Direct3D 12 is available on the host, or zero if Direct3D 12 is not available.
 */
PIL_API(int)
D3D12ApiQuerySupport
(
    struct D3D12API_DISPATCH *dispatch
);

/* @summary Free resources associated with a dispatch table.
 * This function invalidates the entry points associated with the dispatch table.
 * @param dispatch The dispatch table to invalidate.
 */
PIL_API(void)
D3D12ApiInvalidateDispatch
(
    struct D3D12API_DISPATCH *dispatch
);

#ifdef __cplusplus
}; /* extern "C" */
#endif

#endif /* __PIL_D3D12API_WIN32_H__ */

