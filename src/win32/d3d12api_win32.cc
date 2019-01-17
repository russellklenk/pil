/**
 * @summary d3d12api_win32.cc: Implement the runtime loader for D3D12.dll for 
 * the Windows platform.
 */
#include "d3d12api_win32.h"

static HRESULT WINAPI
D3D12CreateDevice_Stub
(
    IUnknown                    *pAdapter,
    D3D_FEATURE_LEVEL minimumFeatureLevel, 
    REFIID                           riid, 
    void                       **ppDevice
)
{
    if (ppDevice) *ppDevice = NULL;
    UNREFERENCED_PARAMETER(pAdapter);
    UNREFERENCED_PARAMETER(minimumFeatureLevel);
    UNREFERENCED_PARAMETER(riid);
    return E_NOTIMPL;
}

static HRESULT WINAPI
D3D12CreateRootSignatureDeserializer_Stub
(
    LPCVOID                           pSrcData, 
    SIZE_T                  srcDataSizeInBytes, 
    REFIID pRootSignatureDeserializerInterface, 
    void         **ppRootSignatureDeserializer
)
{
    if (ppRootSignatureDeserializer) *ppRootSignatureDeserializer = NULL;
    UNREFERENCED_PARAMETER(pSrcData);
    UNREFERENCED_PARAMETER(srcDataSizeInBytes);
    UNREFERENCED_PARAMETER(pRootSignatureDeserializerInterface);
    return E_NOTIMPL;
}

static HRESULT WINAPI
D3D12CreateVersionedRootSignatureDeserializer_Stub
(
    LPCVOID                           pSrcData, 
    SIZE_T                  srcDataSizeInBytes, 
    REFIID pRootSignatureDeserializerInterface, 
    void         **ppRootSignatureDeserializer
)
{
    if (ppRootSignatureDeserializer) *ppRootSignatureDeserializer = NULL;
    UNREFERENCED_PARAMETER(pSrcData);
    UNREFERENCED_PARAMETER(srcDataSizeInBytes);
    UNREFERENCED_PARAMETER(pRootSignatureDeserializerInterface);
    return E_NOTIMPL;
}

static HRESULT WINAPI
D3D12EnableExperimentalFeatures_Stub
(
    UINT                numFeatures, 
    IID const                *pIIDs, 
    void     *pConfigurationStructs, 
    UINT *pConfigurationStructSizes
)
{
    UNREFERENCED_PARAMETER(numFeatures);
    UNREFERENCED_PARAMETER(pIIDs);
    UNREFERENCED_PARAMETER(pConfigurationStructs);
    UNREFERENCED_PARAMETER(pConfigurationStructSizes);
    return E_NOTIMPL;
}

static HRESULT WINAPI
D3D12GetDebugInterface_Stub
(
    REFIID riid, 
    void **pint
)
{
    if (pint) *pint = NULL;
    UNREFERENCED_PARAMETER(riid);
    return E_NOTIMPL;
}

static HRESULT WINAPI
D3D12SerializeRootSignature_Stub
(
    D3D12_ROOT_SIGNATURE_DESC const *pRootSignature, 
    D3D_ROOT_SIGNATURE_VERSION              version, 
    ID3DBlob                               **ppBlob, 
    ID3DBlob                          **ppErrorBlob
)
{
    if (ppBlob     )      *ppBlob = NULL;
    if (ppErrorBlob) *ppErrorBlob = NULL;
    UNREFERENCED_PARAMETER(pRootSignature);
    UNREFERENCED_PARAMETER(version);
    return E_NOTIMPL;
}

static HRESULT WINAPI
D3D12SerializeVersionedRootSignature_Stub
(
    D3D12_VERSIONED_ROOT_SIGNATURE_DESC const *pRootSignature, 
    ID3DBlob                                         **ppBlob, 
    ID3DBlob                                    **ppErrorBlob
)
{
    if (ppBlob     )      *ppBlob = NULL;
    if (ppErrorBlob) *ppErrorBlob = NULL;
    UNREFERENCED_PARAMETER(pRootSignature);
    return E_NOTIMPL;
}

PIL_API(int)
D3D12ApiPopulateDispatch
(
    struct D3D12API_DISPATCH *dispatch, 
    uint32_t              loader_flags
)
{
    RUNTIME_MODULE d3d12_dll;
    int               result;

    assert(dispatch != NULL);
    RuntimeModuleInit(&d3d12_dll);
    UNREFERENCED_PARAMETER(loader_flags);

    result = RuntimeModuleLoad(&d3d12_dll, "D3D12.dll");
    RuntimeFunctionResolve(dispatch, &d3d12_dll, D3D12CreateDevice);
    RuntimeFunctionResolve(dispatch, &d3d12_dll, D3D12CreateRootSignatureDeserializer);
    RuntimeFunctionResolve(dispatch, &d3d12_dll, D3D12CreateVersionedRootSignatureDeserializer);
    RuntimeFunctionResolve(dispatch, &d3d12_dll, D3D12EnableExperimentalFeatures);
    RuntimeFunctionResolve(dispatch, &d3d12_dll, D3D12GetDebugInterface);
    RuntimeFunctionResolve(dispatch, &d3d12_dll, D3D12SerializeRootSignature);
    RuntimeFunctionResolve(dispatch, &d3d12_dll, D3D12SerializeVersionedRootSignature);
    dispatch->ModuleHandle_D3D12   =  d3d12_dll;
    return result;
}

PIL_API(int)
D3D12ApiQuerySupport
(
    struct D3D12API_DISPATCH *dispatch
)
{
    return RuntimeModuleIsValid(&dispatch->ModuleHandle_D3D12);
}

PIL_API(void)
D3D12ApiInvalidateDispatch
(
    struct D3D12API_DISPATCH *dispatch
)
{
    dispatch->D3D12CreateDevice                             = D3D12CreateDevice_Stub;
    dispatch->D3D12CreateRootSignatureDeserializer          = D3D12CreateRootSignatureDeserializer_Stub;
    dispatch->D3D12CreateVersionedRootSignatureDeserializer = D3D12CreateVersionedRootSignatureDeserializer_Stub;
    dispatch->D3D12EnableExperimentalFeatures               = D3D12EnableExperimentalFeatures_Stub;
    dispatch->D3D12GetDebugInterface                        = D3D12GetDebugInterface_Stub;
    dispatch->D3D12SerializeRootSignature                   = D3D12SerializeRootSignature_Stub;
    dispatch->D3D12SerializeVersionedRootSignature          = D3D12SerializeVersionedRootSignature_Stub;
    RuntimeModuleUnload(&dispatch->ModuleHandle_D3D12);
}
