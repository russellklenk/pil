/**
 * @summary win32api_win32.cc: Implement the Windows API runtime loader for the
 * Windows platform.
 */
#include "win32/win32api_win32.h"

static HRESULT WINAPI
SetProcessDpiAwareness_Stub
(
    PROCESS_DPI_AWARENESS level
)
{
    UNREFERENCED_PARAMETER(level);
    if (SetProcessDPIAware()) {
        return S_OK;
    } else {
        return E_ACCESSDENIED;
    }
}

static HRESULT WINAPI
GetDpiForMonitor_Stub
(
    HMONITOR      monitor, 
    MONITOR_DPI_TYPE type, 
    UINT           *dpi_x, 
    UINT           *dpi_y
)
{
    if (type == MDT_EFFECTIVE_DPI) {
        HDC screen_dc = GetDC(NULL);
        DWORD   h_dpi = GetDeviceCaps(screen_dc, LOGPIXELSX);
        DWORD   v_dpi = GetDeviceCaps(screen_dc, LOGPIXELSY);
        ReleaseDC(NULL, screen_dc);
        *dpi_x = h_dpi;
        *dpi_y = v_dpi;
        UNREFERENCED_PARAMETER(monitor);
        return S_OK;
    } else {
        *dpi_x = USER_DEFAULT_SCREEN_DPI;
        *dpi_y = USER_DEFAULT_SCREEN_DPI;
        return E_INVALIDARG;
    }
}

PIL_API(int)
Win32ApiPopulateDispatch
(
    struct WIN32API_DISPATCH *dispatch, 
    uint32_t              loader_flags
)
{
    RUNTIME_MODULE shcore_dll;
    int                result;

    assert(dispatch != NULL);
    RuntimeModuleInit(&shcore_dll);
    UNREFERENCED_PARAMETER(loader_flags);

    result = RuntimeModuleLoad(&shcore_dll, "Shcore.dll");
    RuntimeFunctionResolve(dispatch, &shcore_dll, SetProcessDpiAwareness);
    RuntimeFunctionResolve(dispatch, &shcore_dll, GetDpiForMonitor);
    dispatch->ModuleHandle_Shcore  =  shcore_dll;
    return result;
}

PIL_API(int)
Win32ApiQuerySupport
(
    struct WIN32API_DISPATCH *dispatch
)
{
    UNREFERENCED_PARAMETER(dispatch);
    return 1;
}

PIL_API(void)
Win32ApiInvalidateDispatch
(
    struct WIN32API_DISPATCH *dispatch
)
{
    dispatch->SetProcessDpiAwareness = SetProcessDpiAwareness_Stub;
    dispatch->GetDpiForMonitor       = GetDpiForMonitor_Stub;
    RuntimeModuleUnload(&dispatch->ModuleHandle_Shcore);
}
