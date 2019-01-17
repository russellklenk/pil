/**
 * @summary dxgiapi_win32.cc: Implement the DXGI runtime loader for Windows.
 */
#include "win32/dxgiapi_win32.h"

static HRESULT WINAPI
CreateDXGIFactory_Stub
(
    REFIID riid, 
    void **pint
)
{
    if (pint) *pint = NULL;
    UNREFERENCED_PARAMETER(riid);
    return E_NOINTERFACE;
}

static HRESULT WINAPI
CreateDXGIFactory1_Stub
(
    REFIID riid, 
    void **pint
)
{
    if (pint) *pint = NULL;
    UNREFERENCED_PARAMETER(riid);
    return E_NOINTERFACE;
}

static HRESULT WINAPI
CreateDXGIFactory2_Stub
(
    UINT  flags, 
    REFIID riid, 
    void **pint
)
{
    if (pint) *pint = NULL;
    UNREFERENCED_PARAMETER(flags);
    UNREFERENCED_PARAMETER(riid);
    return E_NOINTERFACE;
}

static HRESULT WINAPI
DXGIGetDebugInterface_Stub
(
    REFIID riid, 
    void **pint
)
{
    if (pint) *pint = NULL;
    UNREFERENCED_PARAMETER(riid);
    return E_NOINTERFACE;
}

static HRESULT WINAPI
DXGIGetDebugInterface1_Stub
(
    UINT  flags,
    REFIID riid, 
    void **pint
)
{
    if (pint) *pint = NULL;
    UNREFERENCED_PARAMETER(flags);
    UNREFERENCED_PARAMETER(riid);
    return E_NOINTERFACE;
}

static HRESULT WINAPI
DXGIDeclareAdapterRemovalSupport_Stub
(
    void
)
{
    return S_OK;
}

PIL_API(int)
DXGIApiPopulateDispatch
(
    struct DXGIAPI_DISPATCH *dispatch, 
    uint32_t             loader_flags
)
{
    RUNTIME_MODULE       dxgi_dll;
    RUNTIME_MODULE dxgi_debug_dll;
    int                    result;

    assert(dispatch != NULL);
    RuntimeModuleInit(&dxgi_dll);
    RuntimeModuleInit(&dxgi_debug_dll);

    result = RuntimeModuleLoad(&dxgi_dll, "Dxgi.dll");
    if (loader_flags & DXGIAPI_LOADER_FLAG_DEBUG_SUPPORT) {
        RuntimeModuleLoad(&dxgi_debug_dll, "DxgiDebug.dll");
    }
    RuntimeFunctionResolve(dispatch  , &dxgi_dll      , CreateDXGIFactory);
    RuntimeFunctionResolve(dispatch  , &dxgi_dll      , CreateDXGIFactory1);
    RuntimeFunctionResolve(dispatch  , &dxgi_dll      , CreateDXGIFactory2);
    RuntimeFunctionResolve(dispatch  , &dxgi_dll      , DXGIDeclareAdapterRemovalSupport);
    RuntimeFunctionResolve(dispatch  , &dxgi_debug_dll, DXGIGetDebugInterface);
    RuntimeFunctionResolve(dispatch  , &dxgi_debug_dll, DXGIGetDebugInterface1);
    dispatch->ModuleHandle_DxgiDebug =  dxgi_debug_dll;
    dispatch->ModuleHandle_Dxgi      =  dxgi_dll;
    return result;
}

PIL_API(int)
DXGIApiQuerySupport
(
    struct DXGIAPI_DISPATCH *dispatch
)
{
    return RuntimeModuleIsValid(&dispatch->ModuleHandle_Dxgi);
}

PIL_API(void)
DXGIApiInvalidateDispatch
(
    struct DXGIAPI_DISPATCH *dispatch
)
{
    dispatch->CreateDXGIFactory               = CreateDXGIFactory_Stub;
    dispatch->CreateDXGIFactory1              = CreateDXGIFactory1_Stub;
    dispatch->CreateDXGIFactory2              = CreateDXGIFactory2_Stub;
    dispatch->DXGIGetDebugInterface           = DXGIGetDebugInterface_Stub;
    dispatch->DXGIGetDebugInterface1          = DXGIGetDebugInterface1_Stub;
    dispatch->DXGIDeclareAdapterRemovalSupport= DXGIDeclareAdapterRemovalSupport_Stub;
    RuntimeModuleUnload(&dispatch->ModuleHandle_DxgiDebug);
    RuntimeModuleUnload(&dispatch->ModuleHandle_Dxgi);
}

