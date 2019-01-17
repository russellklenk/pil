/**
 * @summary display_window_d3d12.cc: Implement the D3D12WindowManager class and 
 * any internal helper functions for dealing with window and swapchain-related 
 * functionality.
 */
#include "display.h"
#include "win32/display_system_d3d12.h"

/* @summary Define the name of the window class used by GPU-driven windows.
 */
static WCHAR const *WSI_WndClassName = L"WSI_WndClass_D3D12";

/* @summary Block the calling thread until the GPU has finished processing all commands.
 * @param wdd The WINDOW_DEVICE_DATA_D3D12 associated with the window.
 */
static void
WindowWaitForGpuIdle
(
    struct WINDOW_DEVICE_DATA_D3D12 *wdd
)
{
    ID3D12CommandQueue *queue = wdd->D3D12CommandQueue;
    ID3D12Fence        *fence = wdd->D3D12FenceSync;
    HANDLE              event = wdd->FenceEvent;
    uint32_t      image_index = wdd->ImageIndex;
    uint64_t      fence_value = wdd->FenceValues[image_index];
    DWORD         wait_result = WAIT_OBJECT_0;
    HRESULT            result = S_OK;

    if (FAILED((result = queue->Signal(fence, fence_value)))) {
        return;
    }
    if (FAILED((result = fence->SetEventOnCompletion(fence_value, event)))) {
        return;
    }
    wait_result = WaitForSingleObjectEx(event, INFINITE, FALSE);
    wdd->FenceValues[image_index]++;
}

/* @summary Retrieve the properties of a display output.
 * @param o_moninfo Pointer to a MONITORINFO structure to populate.
 * @param output The DISPLAY_OUTPUT_D3D12 representing the output to query.
 */
static void
QueryMonitorGeometry
(
    MONITORINFO *o_moninfo, 
    HMONITOR       monitor
)
{
    POINT pt = {0, 0}; /* (0, 0) is always on the primary display */
    if (monitor == nullptr) { /* retrieve the primary display */
        monitor  = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
    } o_moninfo->cbSize = sizeof(MONITORINFO);
    GetMonitorInfo(monitor, o_moninfo);
}

/* @summary Handle the WM_CREATE message.
 * This routine retrieves the properties of the display associated with the window, and resizes the window to account for borders and chrome.
 * @param window Data associated with the window.
 * @param hwnd The window handle.
 * @param wparam Additional data associated with the message.
 * @param lparam Additional data associated with the message.
 * @return A message-specific result code.
 */
static LRESULT CALLBACK
WSI_WndProc_WM_CREATE
(
    struct GPU_WINDOW_D3D12 *window, 
    HWND                       hwnd, 
    WPARAM                   wparam, 
    LPARAM                   lparam
)
{
    WINDOW_STATE_DATA_D3D12 *state = window->StateData;
    WIN32API_DISPATCH      *winapi = state->Win32ApiDispatch;
    HMONITOR               monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
    UINT                     dpi_x = USER_DEFAULT_SCREEN_DPI;
    UINT                     dpi_y = USER_DEFAULT_SCREEN_DPI;
    DWORD                    style =(DWORD) GetWindowLongPtr(hwnd, GWL_STYLE);
    DWORD                 ex_style =(DWORD) GetWindowLongPtr(hwnd, GWL_EXSTYLE);
    RECT                        rc;

    UNREFERENCED_PARAMETER(wparam);
    UNREFERENCED_PARAMETER(lparam);
    winapi->GetDpiForMonitor(monitor, MDT_EFFECTIVE_DPI, &dpi_x, &dpi_y);

    rc.left   = rc.top = 0;
    rc.right  = D3D12_LogicalToPhysicalPixels(state->ClientSizeX, dpi_x);
    rc.bottom = D3D12_LogicalToPhysicalPixels(state->ClientSizeY, dpi_y);
    AdjustWindowRectEx(&rc, style, FALSE, ex_style);
    SetWindowPos(hwnd, nullptr, 0, 0, rc.right-rc.left, rc.bottom-rc.top, SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOZORDER);

    state->StatusFlags = GPU_WINDOW_STATUS_FLAG_CREATED;
    state->EventFlags  = GPU_WINDOW_EVENT_FLAG_CREATED;
    state->EventFlags |= GPU_WINDOW_EVENT_FLAG_SIZE_CHANGED;
    state->EventFlags |= GPU_WINDOW_EVENT_FLAG_POSITION_CHANGED;
    state->OutputDpiX  = dpi_x;
    state->OutputDpiY  = dpi_y;
    state->WindowSizeX = D3D12_PhysicalToLogicalPixels((uint32_t)(rc.right-rc.left), dpi_x);
    state->WindowSizeY = D3D12_PhysicalToLogicalPixels((uint32_t)(rc.bottom-rc.top), dpi_y);
    return 0;
}

/* @summary Handle the WM_CLOSE message.
 * This routine hides the window and marks it as destroyed, but does not actually destroy the window.
 * @param window Data associated with the window.
 * @param hwnd The window handle.
 * @param wparam Additional data associated with the message.
 * @param lparam Additional data associated with the message.
 * @return A message-specific result code.
 */
static LRESULT CALLBACK
WSI_WndProc_WM_CLOSE
(
    struct GPU_WINDOW_D3D12 *window, 
    HWND                       hwnd, 
    WPARAM                   wparam, 
    LPARAM                   lparam
)
{
    WINDOW_STATE_DATA_D3D12 *state = window->StateData;
    UNREFERENCED_PARAMETER(wparam);
    UNREFERENCED_PARAMETER(lparam);
    ShowWindow(hwnd, SW_HIDE);
    state->StatusFlags = GPU_WINDOW_STATUS_FLAGS_NONE;
    state->EventFlags  = GPU_WINDOW_EVENT_FLAG_DESTROYED;
    return 0;
}

/* @summary Handle the WM_ACTIVATE message.
 * @param window Data associated with the window.
 * @param hwnd The window handle.
 * @param wparam Additional data associated with the message.
 * @param lparam Additional data associated with the message.
 * @return A message-specific result code.
 */
static LRESULT CALLBACK
WSI_WndProc_WM_ACTIVATE
(
    struct GPU_WINDOW_D3D12 *window, 
    HWND                       hwnd, 
    WPARAM                   wparam, 
    LPARAM                   lparam
)
{
    WINDOW_STATE_DATA_D3D12 *state = window->StateData;
    int                     active = LOWORD(wparam);
    int                  minimized = HIWORD(wparam);

    if (active) {
        state->StatusFlags |=  GPU_WINDOW_STATUS_FLAG_ACTIVE | GPU_WINDOW_STATUS_FLAG_VISIBLE;
        state->EventFlags  |=  GPU_WINDOW_EVENT_FLAG_ACTIVATED;
    } else { 
        state->StatusFlags &= ~GPU_WINDOW_STATUS_FLAG_ACTIVE;
        state->EventFlags  |=  GPU_WINDOW_EVENT_FLAG_DEACTIVATED;
    }
    if (minimized) {
        state->StatusFlags &= ~GPU_WINDOW_STATUS_FLAG_VISIBLE;
        state->EventFlags  |=  GPU_WINDOW_EVENT_FLAG_HIDDEN;
    }
    return DefWindowProc(hwnd, WM_ACTIVATE, wparam, lparam);
}

/* @summary Handle the WM_DPICHANGED message.
 * This routine updates the position and size of the window based on the suggestion made by the operating system.
 * @param window Data associated with the window.
 * @param hwnd The window handle.
 * @param wparam Additional data associated with the message.
 * @param lparam Additional data associated with the message.
 * @return A message-specific result code.
 */
static LRESULT CALLBACK
WSI_WndProc_WM_DPICHANGED
(
    struct GPU_WINDOW_D3D12 *window, 
    HWND                       hwnd, 
    WPARAM                   wparam, 
    LPARAM                   lparam
)
{
    WINDOW_STATE_DATA_D3D12 *state = window->StateData;
    HMONITOR               monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
    uint32_t                 flags = GPU_WINDOW_EVENT_FLAGS_NONE;
    UINT                     dpi_x = LOWORD(wparam);
    UINT                     dpi_y = HIWORD(wparam);
    DWORD                    style =(DWORD) GetWindowLong(hwnd, GWL_STYLE);
    DWORD                 ex_style =(DWORD) GetWindowLong(hwnd, GWL_EXSTYLE);
    RECT                *suggested =(RECT*)(lparam);
    RECT                        rc;
    MONITORINFO            moninfo;
    
    if ((style & WS_POPUP) == 0) {
        /* resize the window to account for chrome and borders.
         * position the window at the location suggested by the OS.
         */
        if (suggested->left != state->PositionX || suggested->top != state->PositionY) {
            flags |= GPU_WINDOW_EVENT_FLAG_POSITION_CHANGED;
        }
        rc.left    = suggested->left;
        rc.top     = suggested->top;
        rc.right   = suggested->left + D3D12_LogicalToPhysicalPixels(state->ClientSizeX, dpi_x);
        rc.bottom  = suggested->top  + D3D12_LogicalToPhysicalPixels(state->ClientSizeY, dpi_y);
        AdjustWindowRectEx(&rc, style, FALSE, ex_style);
        SetWindowPos(hwnd, nullptr, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, SWP_NOACTIVATE|SWP_NOZORDER);
    } else { /* fullscreen borderless */
        QueryMonitorGeometry(&moninfo, monitor);
        rc = moninfo.rcMonitor;
    }
    state->OutputDpiX  = dpi_x;
    state->OutputDpiY  = dpi_y;
    state->PositionX   = rc.left;
    state->PositionY   = rc.top;
    state->WindowSizeX = D3D12_PhysicalToLogicalPixels((uint32_t)(rc.right-rc.left), dpi_x);
    state->WindowSizeY = D3D12_PhysicalToLogicalPixels((uint32_t)(rc.bottom-rc.top), dpi_y);
    state->EventFlags |= GPU_WINDOW_EVENT_FLAG_SIZE_CHANGED | flags;
    return 0;
}

/* @summary Handle the WM_MOVE message.
 * This routine updates the position of the window and detects any DPI changes resulting from moving to a different monitor.
 * @param window Data associated with the window.
 * @param hwnd The window handle.
 * @param wparam Additional data associated with the message.
 * @param lparam Additional data associated with the message.
 * @return A message-specific result code.
 */
static LRESULT CALLBACK
WSI_WndProc_WM_MOVE
(
    struct GPU_WINDOW_D3D12 *window, 
    HWND                       hwnd, 
    WPARAM                   wparam, 
    LPARAM                   lparam
)
{
    WINDOW_STATE_DATA_D3D12 *state = window->StateData;
    WIN32API_DISPATCH      *winapi = state->Win32ApiDispatch;
    HMONITOR               monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
    UINT                     dpi_x = USER_DEFAULT_SCREEN_DPI;
    UINT                     dpi_y = USER_DEFAULT_SCREEN_DPI;
    RECT                        rc;

    GetWindowRect(hwnd, &rc);
    winapi->GetDpiForMonitor(monitor, MDT_EFFECTIVE_DPI, &dpi_x, &dpi_y);
    state->EventFlags |= GPU_WINDOW_EVENT_FLAG_POSITION_CHANGED;
    state->PositionX   = rc.left;
    state->PositionY   = rc.top;
    state->WindowSizeX = D3D12_PhysicalToLogicalPixels((uint32_t)(rc.right-rc.left), dpi_x);
    state->WindowSizeY = D3D12_PhysicalToLogicalPixels((uint32_t)(rc.bottom-rc.top), dpi_y);
    state->OutputDpiX  = dpi_x;
    state->OutputDpiY  = dpi_y;
    UNREFERENCED_PARAMETER(wparam);
    UNREFERENCED_PARAMETER(lparam);
    return 0;
}

/* @summary Handle the WM_SIZE message.
 * This routine updates the position and size of the window and detects any DPI changes resulting from moving to a different monitor.
 * @param window Data associated with the window.
 * @param hwnd The window handle.
 * @param wparam Additional data associated with the message.
 * @param lparam Additional data associated with the message.
 * @return A message-specific result code.
 */
static LRESULT CALLBACK
WSI_WndProc_WM_SIZE
(
    struct GPU_WINDOW_D3D12 *window, 
    HWND                       hwnd, 
    WPARAM                   wparam, 
    LPARAM                   lparam
)
{
    WINDOW_STATE_DATA_D3D12    *state = window->StateData;
    WINDOW_DEVICE_DATA_D3D12  *device = window->DeviceData;
    WINDOW_SWAPCHAIN_DATA_D3D12 *swap = window->SwapchainData;
    WIN32API_DISPATCH         *winapi = state->Win32ApiDispatch;
    HMONITOR                  monitor = MonitorFromWindow(hwnd , MONITOR_DEFAULTTONEAREST);
    uint32_t             phy_client_w =(uint32_t)LOWORD(lparam);
    uint32_t             phy_client_h =(uint32_t)HIWORD(lparam);
    UINT                        dpi_x = USER_DEFAULT_SCREEN_DPI;
    UINT                        dpi_y = USER_DEFAULT_SCREEN_DPI;
    HRESULT                    result = S_OK;
    uint32_t                    flags = GPU_WINDOW_EVENT_FLAG_SIZE_CHANGED;
    uint32_t                   status = state->StatusFlags;
    UINT                   swap_flags = 0;
    IDXGISwapChain3   *dxgi_swapchain = nullptr;
    ID3D12Resource           **images;
    uint64_t            *fence_values;
    uint64_t              fence_value;
    uint32_t             log_client_w;
    uint32_t             log_client_h;
    BOOL                is_fullscreen;
    BOOL                   is_visible;
    BOOL                     did_size;
    RECT                           rc;
    uint32_t                     i, n;

    /* we'll need to access the swapchain and per-frame resources */
    winapi->GetDpiForMonitor(monitor, MDT_EFFECTIVE_DPI , &dpi_x, &dpi_y);
    log_client_w = D3D12_PhysicalToLogicalPixels(phy_client_w, dpi_x);
    log_client_h = D3D12_PhysicalToLogicalPixels(phy_client_h, dpi_y);

    /* determine the visibility status and whether the size changed */
    if (wparam == SIZE_MINIMIZED || wparam == SIZE_MAXHIDE) {
        flags     |= GPU_WINDOW_EVENT_FLAG_HIDDEN;
        status    &=~GPU_WINDOW_STATUS_FLAG_VISIBLE;
        is_visible = FALSE;
    } else if (wparam == SIZE_RESTORED) {
        flags     |= GPU_WINDOW_EVENT_FLAG_SHOWN;
        status    |= GPU_WINDOW_STATUS_FLAG_VISIBLE;
        is_visible = TRUE;
    } else {
        status    |= GPU_WINDOW_STATUS_FLAG_VISIBLE;
        is_visible = TRUE;
    }
    if (log_client_w == state->ClientSizeX && log_client_h == state->ClientSizeY) {
        did_size = FALSE;
    } else {
        did_size = TRUE;
    }
    if (is_visible == FALSE || did_size == FALSE) {
        state->StatusFlags = status;
        return 0;
    }

    /* the window is visible, and the size did change.
     * flush all pending GPU commands, because the swapchain needs to resize.
     */
    WindowWaitForGpuIdle(device);
    fence_values   = device->FenceValues;
    fence_value    = device->FenceValues[device->ImageIndex];
    images         = swap->ImageResources;
    dxgi_swapchain = swap->DxgiSwapChain;

    /* release resources referencing the swapchain */
    for (i = 0, n = device->ImageCount; i < n; ++i) {
        if (images[i]) {
            images[i]->Release();
            images[i] = nullptr;
        } fence_values[i] = fence_value;
    }

    if (swap->StatusFlags & SWAPCHAIN_STATUS_FLAG_D3D12_TEARING) {
        swap_flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
    }
    if (swap->StatusFlags & SWAPCHAIN_STATUS_FLAG_D3D12_FULLSCREEN) {
        is_fullscreen = TRUE;
    } else {
        is_fullscreen = FALSE;
    }
    if (FAILED((result = dxgi_swapchain->ResizeBuffers(swap->ImageCount, phy_client_w, phy_client_h, swap->ImageFormat, swap_flags)))) {
        return 0;
    }
    device->ImageIndex = dxgi_swapchain->GetCurrentBackBufferIndex();
    /* TODO: recreate RTVs & descriptors for swapchain images (LoadSizeDependentResources) */

    if (is_fullscreen) {
        status |= GPU_WINDOW_STATUS_FLAG_FULLSCREEN;
    } else {
        status &=~GPU_WINDOW_STATUS_FLAG_FULLSCREEN;
    }

    GetWindowRect(hwnd, &rc);
    state->StatusFlags = status;
    state->EventFlags |= flags;
    state->PositionX   = rc.left;
    state->PositionY   = rc.top;
    state->WindowSizeX = D3D12_PhysicalToLogicalPixels((uint32_t)(rc.right-rc.left), dpi_x);
    state->WindowSizeY = D3D12_PhysicalToLogicalPixels((uint32_t)(rc.bottom-rc.top), dpi_y);
    state->ClientSizeX = log_client_w;
    state->ClientSizeY = log_client_h;
    state->OutputDpiX  = dpi_x;
    state->OutputDpiY  = dpi_y;
    return 0;
}

/* @summary Handle the WM_SHOWWINDOW message.
 * This routine updates the visibility status of the window.
 * @param window Data associated with the window.
 * @param hwnd The window handle.
 * @param wparam Additional data associated with the message.
 * @param lparam Additional data associated with the message.
 * @return A message-specific result code.
 */
static LRESULT CALLBACK
WSI_WndProc_WM_SHOWWINDOW
(
    struct GPU_WINDOW_D3D12 *window, 
    HWND                       hwnd, 
    WPARAM                   wparam, 
    LPARAM                   lparam
)
{
    WINDOW_STATE_DATA_D3D12 *state = window->StateData;
    if (wparam) { /* window is being shown */
        WIN32API_DISPATCH *winapi = state->Win32ApiDispatch;
        HMONITOR          monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
        UINT                dpi_x = USER_DEFAULT_SCREEN_DPI;
        UINT                dpi_y = USER_DEFAULT_SCREEN_DPI;
        RECT                   rc;

        GetWindowRect(hwnd, &rc);
        winapi->GetDpiForMonitor(monitor, MDT_EFFECTIVE_DPI, &dpi_x, &dpi_y);
        state->StatusFlags |= GPU_WINDOW_STATUS_FLAG_VISIBLE;
        state->EventFlags  |= GPU_WINDOW_EVENT_FLAG_SHOWN;
        state->PositionX    = rc.left;
        state->PositionY    = rc.top;
        state->WindowSizeX  = D3D12_PhysicalToLogicalPixels((uint32_t)(rc.right-rc.left), dpi_x);
        state->WindowSizeY  = D3D12_PhysicalToLogicalPixels((uint32_t)(rc.bottom-rc.top), dpi_y);
        GetClientRect(hwnd , &rc);
        state->ClientSizeX  = D3D12_PhysicalToLogicalPixels((uint32_t)(rc.right-rc.left), dpi_x);
        state->ClientSizeY  = D3D12_PhysicalToLogicalPixels((uint32_t)(rc.bottom-rc.top), dpi_y);
        state->OutputDpiX   = dpi_x;
        state->OutputDpiY   = dpi_y;
    } else { /* window is being hidden */
        state->StatusFlags &=~GPU_WINDOW_STATUS_FLAG_VISIBLE;
        state->StatusFlags &=~GPU_WINDOW_STATUS_FLAG_ACTIVE;
        state->EventFlags  |=(GPU_WINDOW_EVENT_FLAG_HIDDEN | GPU_WINDOW_EVENT_FLAG_DEACTIVATED);
    }
    return DefWindowProc(hwnd, WM_SHOWWINDOW, wparam, lparam);
}

/* @summary Handle the WM_SYSCOMMAND message.
 * This routine processes the Alt+Enter key combination to toggle between windowed and fullscreen.
 * @param window Data associated with the window.
 * @param hwnd The window handle.
 * @param wparam Additional data associated with the message.
 * @param lparam Additional data associated with the message.
 * @return A message-specific result code.
 */
static LRESULT CALLBACK
WSI_WndProc_WM_SYSCOMMAND
(
    struct GPU_WINDOW_D3D12 *window, 
    HWND                       hwnd, 
    WPARAM                   wparam, 
    LPARAM                   lparam
)
{
    if ((wparam & 0xFFF0) == SC_KEYMENU) {
        if (lparam == VK_RETURN) { /* Alt+Enter */
            WINDOW_STATE_DATA_D3D12    *state = window->StateData;
            WINDOW_SWAPCHAIN_DATA_D3D12 *swap = window->SwapchainData;
            WIN32API_DISPATCH         *winapi = state->Win32ApiDispatch;
            HMONITOR                  monitor = MonitorFromWindow(hwnd , MONITOR_DEFAULTTONEAREST);
            DISPLAY_OUTPUT_D3D12      *output = D3D12_ResolveOutputForHMONITOR(state->OutputTable, monitor);
            IDXGISwapChain3   *dxgi_swapchain = swap->DxgiSwapChain;
            IDXGIOutput          *dxgi_output = nullptr;
            HRESULT                    result = S_OK;
            UINT                        dpi_x = USER_DEFAULT_SCREEN_DPI;
            UINT                        dpi_y = USER_DEFAULT_SCREEN_DPI;
            MONITORINFO               moninfo;

            winapi->GetDpiForMonitor(monitor, MDT_EFFECTIVE_DPI , &dpi_x, &dpi_y);
            if (swap->StatusFlags & SWAPCHAIN_STATUS_FLAG_D3D12_FULLSCREEN) {
                /* toggle back to windowed mode */
                if (swap->StatusFlags & SWAPCHAIN_STATUS_FLAG_D3D12_TEARING) {
                    RECT     rc = state->RestoreRect;
                    SetWindowLong(hwnd, GWL_STYLE  , state->RestoreStyle);
                    SetWindowLong(hwnd, GWL_EXSTYLE, state->RestoreStyleEx);
                    SetWindowPos (hwnd, HWND_TOP   , rc.left, rc.top, (rc.right-rc.left), (rc.bottom-rc.top), SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOOWNERZORDER);
                } else {
                    if (output != nullptr) {
                        dxgi_output = output->DxgiOutput;
                    }
                    if (FAILED((result = dxgi_swapchain->SetFullscreenState(FALSE, dxgi_output)))) {
                        return 0;
                    }
                }
                swap ->StatusFlags &=~SWAPCHAIN_STATUS_FLAG_D3D12_FULLSCREEN;
                state->StatusFlags &=~GPU_WINDOW_STATUS_FLAG_FULLSCREEN;
                ShowWindow(hwnd, SW_NORMAL);
            } else {
                /* toggle to fullscreen mode.
                 * if the system supports tearing, then we have to do this ourselves.
                 * otherwise, DXGI will handle it for us. make sure to match the 
                 * HMONITOR to the IDXGIOutput so that the window goes fullscreen on the same monitor.
                 */
                QueryMonitorGeometry(&moninfo, monitor);
                GetWindowRect(hwnd,  &state->RestoreRect);
                state->RestoreStyle   = (DWORD)GetWindowLong(hwnd, GWL_STYLE);
                state->RestoreStyleEx = (DWORD)GetWindowLong(hwnd, GWL_EXSTYLE);
                if (swap->StatusFlags & SWAPCHAIN_STATUS_FLAG_D3D12_TEARING) {
                    RECT     rc = moninfo.rcMonitor;
                    SetWindowLong(hwnd, GWL_STYLE, WS_POPUP);
                    SetWindowPos (hwnd, HWND_TOP , rc.left, rc.top, (rc.right-rc.left), (rc.bottom-rc.top), SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOOWNERZORDER);
                } else {
                    if (FAILED((result = dxgi_swapchain->SetFullscreenState(TRUE, dxgi_output)))) {
                        return 0;
                    }
                }
                swap ->StatusFlags |= SWAPCHAIN_STATUS_FLAG_D3D12_FULLSCREEN;
                state->StatusFlags |= GPU_WINDOW_STATUS_FLAG_FULLSCREEN;
                ShowWindow(hwnd, SW_MAXIMIZE);
            }
            return 0; /* handled Alt+Enter */
        }
    }
    return DefWindowProc(hwnd, WM_SYSCOMMAND, wparam, lparam);
}

/* @summary Implement the window message callback for all GPU-driven windows.
 * @param hwnd The window handle.
 * @param msg The identifier of the message to process.
 * @param wparam Additional data associated with the message.
 * @param lparam Additional data associated with the message.
 * @return A message-specific result code.
 */
static LRESULT CALLBACK
WSI_WndProc
(
    HWND     hwnd, 
    UINT      msg, 
    WPARAM wparam, 
    LPARAM lparam
)
{
    LRESULT                 result = 0;
    WINDOW_STATE_DATA_D3D12 *state = nullptr;
    GPU_WINDOW_D3D12        window;

    /* WM_NCCREATE performs special handling to store the state associated with the window.
     * The handler for WM_NCCREATE executes before the call to CreateWindowEx returns.
     */
    if (msg == WM_NCCREATE) {
        CREATESTRUCT            *cs =(CREATESTRUCT           *)  lparam;
        WINDOW_STATE_DATA_D3D12 *wd =(WINDOW_STATE_DATA_D3D12*)  cs->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA,   (LONG_PTR ) wd); wd->WindowHandle = hwnd;
        return DefWindowProc(hwnd, msg, wparam, lparam);
    }
    /* WndProc may receive several messaes before WM_NCCREATE.
     * Send these off to the default handler.
     */
    if ((state = (WINDOW_STATE_DATA_D3D12*) GetWindowLongPtr(hwnd, GWLP_USERDATA)) == nullptr) {
        return DefWindowProc(hwnd, msg, wparam, lparam);
    }
    if (D3D12_ResolveWindowForWindowState(state, &window) != 0) {
        return DefWindowProc(hwnd, msg, wparam, lparam);
    }
    switch (msg) {
        case WM_MOVE:
            { /* update position and detect monitor/DPI changes */
              result = WSI_WndProc_WM_MOVE(&window, hwnd, wparam, lparam);
            } break;

        case WM_SIZE:
            { /* update size and detect monitor/DPI changes */
              result = WSI_WndProc_WM_SIZE(&window, hwnd, wparam, lparam);
            } break;

        case WM_ACTIVATE:
            { /* update active status so app can save power when inactive */
              result = WSI_WndProc_WM_ACTIVATE(&window, hwnd, wparam, lparam);
            } break;

        case WM_SHOWWINDOW:
            { /* update visibility status so app can save power when hidden */
              result = WSI_WndProc_WM_SHOWWINDOW(&window, hwnd, wparam, lparam);
            } break;

        case WM_DPICHANGED:
            { /* respond to DPI setting changes */
              result = WSI_WndProc_WM_DPICHANGED(&window, hwnd, wparam, lparam);
            } break;

        case WM_SYSCOMMAND:
            { /* handle Alt+Enter to toggle fullscreen and windowed */
              result = WSI_WndProc_WM_SYSCOMMAND(&window, hwnd, wparam, lparam);
            } break;

        case WM_CREATE:
            { /* update and resize window client area if necessary */
              result = WSI_WndProc_WM_CREATE(&window, hwnd, wparam, lparam);
            } break;

        case WM_CLOSE:
            { /* close the window and mark it as being destroyed */
              result = WSI_WndProc_WM_CLOSE(&window, hwnd, wparam, lparam);
            } break;

        default:
            { /* pass the message off to the default handler */
              result = DefWindowProc(hwnd, msg, wparam, lparam);
            } break;
    }
    return result;
}

PIL_API(int)
D3D12_CreateDisplayWindowTable
(
    struct DISPLAY_WINDOW_TABLE_D3D12 *table, 
    uint32_t                        capacity
)
{
#   define N D3D12_WINDOW_TABLE_STREAM_COUNT
    uint32_t const          stream_count = N;
    uint32_t                      result = 0;
    TABLE_INIT                table_init = {};
    TABLE_DATA_STREAM_DESC table_data[N] = {
        { &table->StateData    , sizeof(WINDOW_STATE_DATA_D3D12    ) }, 
        { &table->DeviceData   , sizeof(WINDOW_DEVICE_DATA_D3D12   ) }, 
        { &table->SwapchainData, sizeof(WINDOW_SWAPCHAIN_DATA_D3D12) }
    };
#   undef  N
    table_init.Index         =&table->TableIndex;
    table_init.Streams       = table_data;
    table_init.StreamCount   = stream_count;
    table_init.TableCapacity = capacity;
    table_init.InitialCommit = capacity;
    if ((result = TableCreate(&table_init)) != 0) {
        return result;
    }
    table->TableStreams[D3D12_WINDOW_STATE_STREAM_INDEX    ] =&table->StateData;
    table->TableStreams[D3D12_WINDOW_DEVICE_STREAM_INDEX   ] =&table->DeviceData;
    table->TableStreams[D3D12_WINDOW_SWAPCHAIN_STREAM_INDEX] =&table->SwapchainData;
    table->TableDesc.Index                                   =&table->TableIndex;
    table->TableDesc.Streams                                 = table->TableStreams;
    table->TableDesc.StreamCount                             = stream_count;
    return 0;
}

PIL_API(void)
D3D12_DeleteDisplayWindowTable
(
    struct DISPLAY_WINDOW_TABLE_D3D12 *table
)
{
    assert(D3D12_WindowTableCount(table) == 0);
    TableDelete(&table->TableDesc);
}

PIL_API(int)
D3D12_ResolveWindowForHandle
(
    struct DISPLAY_WINDOW_TABLE_D3D12 *table, 
    struct GPU_WINDOW_HANDLE          handle, 
    struct GPU_WINDOW_D3D12          *o_data
)
{
    uint32_t record;
    if (TableResolve(&record, &table->TableDesc, handle.Bits)) {
        o_data->StateData     = D3D12_WindowTableStateStreamAt    (table, record);
        o_data->DeviceData    = D3D12_WindowTableDeviceStreamAt   (table, record);
        o_data->SwapchainData = D3D12_WindowTableSwapchainStreamAt(table, record);
        o_data->HandleBits    = handle.Bits;
        o_data->RecordIndex   = record;
        return  0;
    } else {
        ZeroMemory(o_data, sizeof(GPU_WINDOW_D3D12));
        return -1;
    }
}

PIL_API(int)
D3D12_ResolveWindowForWindowState
(
    struct WINDOW_STATE_DATA_D3D12 *state, 
    struct GPU_WINDOW_D3D12       *o_data
)
{
    DISPLAY_WINDOW_TABLE_D3D12 *table = state->WindowTable;
    TABLE_DATA                *stream = table->TableStreams[D3D12_WINDOW_STATE_STREAM_INDEX];
    uint32_t                   record = TableData_GetElementIndex         (stream, state);
    o_data->StateData                 = D3D12_WindowTableStateStreamAt    (table, record);
    o_data->DeviceData                = D3D12_WindowTableDeviceStreamAt   (table, record);
    o_data->SwapchainData             = D3D12_WindowTableSwapchainStreamAt(table, record);
    o_data->HandleBits                = D3D12_WindowTableHandleAt         (table, record);
    o_data->RecordIndex               = record;
    return 0;
}

PIL_API(int)
D3D12_CreateWindowAndSwapchain
(
    struct DISPLAY_WINDOW_TABLE_D3D12 *table, 
    struct GPU_WINDOW_INIT_D3D12       *init, 
    struct GPU_WINDOW_HANDLE       *o_handle
)
{
    WINDOW_SWAPCHAIN_DATA_D3D12         *scd = nullptr;
    WINDOW_DEVICE_DATA_D3D12            *wdd = nullptr;
    WINDOW_STATE_DATA_D3D12             *wsd = nullptr;
    DISPLAY_OUTPUT_TABLE_D3D12 *output_table = init->OutputTable;
    WIN32API_DISPATCH                *winapi = init->Win32ApiDispatch;
    IDXGIFactory2              *dxgi_factory = init->DxgiFactory;
    IDXGIOutput                 *dxgi_output = init->DxgiOutput;
    IDXGISwapChain1          *dxgi_swapchain = nullptr;
    IDXGISwapChain3         *dxgi_swapchain3 = nullptr;
    ID3D12Device               *d3d12_device = init->D3D12Device;
    ID3D12CommandQueue          *d3d12_queue = init->D3D12CommandQueue;
    ID3D12Fence                 *d3d12_fence = nullptr;
    ID3D12Resource         **d3d12_resources = nullptr;
    uint64_t                   *fence_values = nullptr;
    HANDLE                       fence_event = nullptr;
    HWND                                hwnd = nullptr;
    int32_t                        virtual_x = 0;
    int32_t                        virtual_y = 0;
    uint32_t                        dim_x_px = init->ClientWidth;
    uint32_t                        dim_y_px = init->ClientHeight;
    HRESULT                           result = S_OK;
    DWORD                         error_code = ERROR_SUCCESS;
    DWORD                           style_ex = 0;
    DWORD                              style = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
    HANDLE_BITS                         bits = HANDLE_BITS_INVALID;
    HANDLE_BITS                        moved = HANDLE_BITS_INVALID;
    DXGI_SWAP_CHAIN_DESC1     swapchain_desc = {};
    uint32_t                          record;
    uint32_t                    buffer_index;
    uint32_t                            i, n;
    RECT                                  rc;
    MONITORINFO                      moninfo;
    WNDCLASSEX                      wndclass;
    DXGI_OUTPUT_DESC             output_desc;

    if (!GetClassInfoEx(GetModuleHandle(nullptr), WSI_WndClassName, &wndclass)) {
        wndclass.cbSize        = sizeof(WNDCLASSEX);
        wndclass.cbClsExtra    = 0;
        wndclass.cbWndExtra    = sizeof(WINDOW_STATE_DATA_D3D12*);
        wndclass.hInstance     = GetModuleHandle(nullptr);
        wndclass.lpszClassName = WSI_WndClassName;
        wndclass.lpszMenuName  = nullptr;
        wndclass.lpfnWndProc   = WSI_WndProc;
        wndclass.hIcon         = LoadIcon  (0, IDI_APPLICATION);
        wndclass.hIconSm       = LoadIcon  (0, IDI_APPLICATION);
        wndclass.hCursor       = LoadCursor(0, IDC_ARROW);
        wndclass.style         = CS_HREDRAW | CS_VREDRAW;
        wndclass.hbrBackground =(HBRUSH) GetStockObject(WHITE_BRUSH);
        if (!RegisterClassEx(&wndclass)) {
            error_code = GetLastError();
            if (o_handle) {
               *o_handle = GPU_WINDOW_HANDLE{HANDLE_BITS_INVALID};
            } return -1;
        }
    }

    /* create the window object reference */
    if ((bits = TableCreateId(&record, &table->TableDesc)) != HANDLE_BITS_INVALID) {
        wsd = D3D12_WindowTableStateStreamAt     (table, record);
        wdd = D3D12_WindowTableDeviceStreamAt    (table, record);
        scd = D3D12_WindowTableSwapchainStreamAt (table, record);
        ZeroMemory(wsd, sizeof(WINDOW_STATE_DATA_D3D12));
        ZeroMemory(wdd, sizeof(WINDOW_DEVICE_DATA_D3D12));
        ZeroMemory(scd, sizeof(WINDOW_SWAPCHAIN_DATA_D3D12));
    } else {
        goto cleanup_and_fail;
    }

    /* the window can be created either on the primary display (no TargetOutput) 
     * or on a specified display. the window is always created centered and with 
     * chrome. if fullscreen is desired, fullscreen mode is set after the window 
     * is created. note that the swapchain may have a different resolution from 
     * the window, in which case the compositor handles the upscaling.
     */
    if (dxgi_output != nullptr) {
        ZeroMemory(&output_desc, sizeof(DXGI_OUTPUT_DESC));
        dxgi_output->GetDesc(&output_desc);
        QueryMonitorGeometry(&moninfo, output_desc.Monitor);
    } else { /* use primary display */
        QueryMonitorGeometry(&moninfo, nullptr);
    } rc = moninfo.rcWork;

    /* center the window on the target display */
    virtual_x = rc.left + (rc.right - rc.left - dim_x_px) / 2;
    virtual_y = rc.top  + (rc.bottom - rc.top - dim_y_px) / 2;
    if ((int32_t)(virtual_x + dim_x_px) > (int32_t)(rc.right - rc.left)) {
        dim_x_px = (rc.right - rc.left) - virtual_x;
    }
    if ((int32_t)(virtual_y + dim_y_px) > (int32_t)(rc.bottom - rc.top)) {
        dim_y_px = (rc.bottom - rc.top) - virtual_y;
    }

    /* setup for window creation */
    wsd->WindowTable          = table;
    wsd->OutputTable          = output_table;
    wsd->Win32ApiDispatch     = winapi;
    wsd->StatusFlags          = GPU_WINDOW_STATUS_FLAGS_NONE;
    wsd->EventFlags           = GPU_WINDOW_EVENT_FLAGS_NONE;
    wsd->OutputDpiX           = USER_DEFAULT_SCREEN_DPI;
    wsd->OutputDpiY           = USER_DEFAULT_SCREEN_DPI;
    wsd->PositionX            = virtual_x;
    wsd->PositionY            = virtual_y;
    wsd->WindowSizeX          = dim_x_px;
    wsd->WindowSizeY          = dim_y_px;
    wsd->ClientSizeX          = init->ClientWidth;
    wsd->ClientSizeY          = init->ClientHeight;
    wsd->RestoreStyle         = style;
    wsd->RestoreStyleEx       = style_ex;
    if ((hwnd = CreateWindowExW(style_ex, WSI_WndClassName, init->WindowTitle, style, virtual_x, virtual_y, dim_x_px, dim_y_px, nullptr, nullptr, GetModuleHandle(nullptr), wsd)) == nullptr) {
        goto cleanup_and_fail;
    } ShowWindow(hwnd, SW_SHOW);

    /* create the swapchain associated with the window */
    ZeroMemory(&swapchain_desc, sizeof(DXGI_SWAP_CHAIN_DESC1));
    swapchain_desc.BufferCount       = D3D12_WINDOW_SWAPCHAIN_IMAGE_COUNT;
    swapchain_desc.Width             = init->SwapchainWidth;
    swapchain_desc.Height            = init->SwapchainHeight;
    swapchain_desc.Format            = DXGI_FORMAT_R8G8B8A8_UNORM; /* HDR displays? */
    swapchain_desc.BufferUsage       = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapchain_desc.SwapEffect        = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapchain_desc.SampleDesc.Count  = 1;
    if (init->FeatureFlags & DISPLAY_SYSTEM_FEATURE_FLAG_D3D12_TEARING) {
        swapchain_desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
    }
    if (FAILED((result = dxgi_factory->CreateSwapChainForHwnd(d3d12_queue, hwnd, &swapchain_desc, nullptr, nullptr, &dxgi_swapchain)))) {
        goto cleanup_and_fail;
    }
    if (FAILED((result = dxgi_swapchain->QueryInterface(IID_PPV_ARGS(&dxgi_swapchain3))))) {
        goto cleanup_and_fail;
    } else {
        dxgi_swapchain->Release();
        dxgi_swapchain = nullptr;
    }
    if (init->FeatureFlags & DISPLAY_SYSTEM_FEATURE_FLAG_D3D12_TEARING) {
        dxgi_factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);
        scd->StatusFlags = SWAPCHAIN_STATUS_FLAG_D3D12_TEARING;
    } else { /* DXGI will handle fullscreen switching for us */
        scd->StatusFlags = SWAPCHAIN_STATUS_FLAGS_D3D12_NONE;
    }
    /* fill out the swapchain data based on what we actually got */
    dxgi_swapchain3->GetDesc1(&swapchain_desc);
    if ((fence_values = (uint64_t*) malloc(swapchain_desc.BufferCount * sizeof(uint64_t))) == nullptr) {
        goto cleanup_and_fail;
    }
    if ((d3d12_resources  = (ID3D12Resource**) malloc(swapchain_desc.BufferCount * sizeof(ID3D12Resource*))) == nullptr) {
        goto cleanup_and_fail;
    }
    memset(fence_values   , 0, swapchain_desc.BufferCount * sizeof(uint64_t));
    memset(d3d12_resources, 0, swapchain_desc.BufferCount * sizeof(ID3D12Resource*));
    scd->DxgiSwapChain       = dxgi_swapchain3;
    scd->ImageResources      = d3d12_resources;
    scd->ImageFormat         = swapchain_desc.Format;
    scd->ImageWidth          = swapchain_desc.Width;
    scd->ImageHeight         = swapchain_desc.Height;
    scd->ImageCount          = swapchain_desc.BufferCount;
    for (i = 0, n = swapchain_desc.BufferCount; i < n; ++i) {
        if (FAILED((result = dxgi_swapchain3->GetBuffer (i, IID_PPV_ARGS(&d3d12_resources[i]))))) {
            goto cleanup_and_fail;
        }
    }

    /* create per-image synchronization resources */
    buffer_index = dxgi_swapchain3->GetCurrentBackBufferIndex();
    if (FAILED((result = d3d12_device->CreateFence(fence_values[buffer_index], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&d3d12_fence))))) {
        goto cleanup_and_fail;
    }
    if ((fence_event = CreateEvent(nullptr, FALSE, FALSE, nullptr)) == nullptr) {
        goto cleanup_and_fail;
    }
    wdd->D3D12Device       = d3d12_device;
    wdd->D3D12FenceSync    = d3d12_fence;
    wdd->D3D12CommandQueue = init->D3D12CommandQueue;
    wdd->FenceEvent        = fence_event;
    wdd->FenceValues       = fence_values;
    wdd->ImageIndex        = buffer_index;
    wdd->ImageCount        = swapchain_desc.BufferCount;
    wdd->FenceValues[buffer_index]++;
    // D3D12RenderTargets[]
    // descriptor heaps for render target views (D3D12Fullscreen::LoadPipeline)
    // command allocators for each frams (D3D12Fullscreen::LoadPipeline)
   *o_handle = GPU_WINDOW_HANDLE{bits};
    return 0;

cleanup_and_fail:
    if (d3d12_resources) {
        for (i = 0, n = swapchain_desc.BufferCount; i < n; ++i) {
            if (d3d12_resources[i] != nullptr) {
                d3d12_resources[i]->Release();
                d3d12_resources[i] = nullptr;
            }
        } free(d3d12_resources);
    }
    if (d3d12_fence    ) d3d12_fence->Release();
    if (fence_event    ) CloseHandle(fence_event);
    if (fence_values   ) free(fence_values);
    if (dxgi_swapchain3) dxgi_swapchain3->Release();
    if (dxgi_swapchain ) dxgi_swapchain ->Release();
    if (hwnd           ) DestroyWindow(hwnd);
    if (bits) {
        if ((moved = TableDeleteId(&table->TableDesc, bits)) != HANDLE_BITS_INVALID) {
            /* at this point wsd, scd and wdd point to the data for the moved item */
            SetWindowLongPtr(wsd->WindowHandle, GWLP_USERDATA, (LONG_PTR) wsd);
        }
    } *o_handle = GPU_WINDOW_HANDLE{HANDLE_BITS_INVALID};
    return -1;
}

PIL_API(void)
D3D12_DeleteWindowAndSwapchain
(
    struct DISPLAY_WINDOW_TABLE_D3D12 *table, 
    struct GPU_WINDOW_HANDLE          handle
)
{
    GPU_WINDOW_D3D12 w;
    if (D3D12_ResolveWindowForHandle(table, handle, &w) == 0) {
        WINDOW_STATE_DATA_D3D12     *wsd  = w.StateData;
        WINDOW_DEVICE_DATA_D3D12    *wdd  = w.DeviceData;
        WINDOW_SWAPCHAIN_DATA_D3D12 *scd  = w.SwapchainData;
        IDXGISwapChain3  *dxgi_swapchain  = scd->DxgiSwapChain;
        ID3D12Resource **d3d12_resources  = scd->ImageResources;
        ID3D12Fence         *d3d12_fence  = wdd->D3D12FenceSync;
        HANDLE               fence_event  = wdd->FenceEvent;
        uint64_t           *fence_values  = wdd->FenceValues;
        HANDLE_BITS                moved  = HANDLE_BITS_INVALID;
        uint32_t                    i, n;

        WindowWaitForGpuIdle(wdd);

        /* always switch back to windowed if tearing is not supported */
        if((scd->StatusFlags & SWAPCHAIN_STATUS_FLAG_D3D12_TEARING) == 0) {
            dxgi_swapchain->SetFullscreenState(FALSE, nullptr);
        }

        /* free "per-frame" resources */
        wdd->D3D12FenceSync = nullptr;
        wdd->FenceValues = nullptr;
        wdd->FenceEvent = nullptr;
        CloseHandle(fence_event);
        d3d12_fence->Release();
        free(fence_values);

        /* free swapchain resources */
        for (i = 0, n = scd->ImageCount; i < n; ++i) {
            if (d3d12_resources[i] != nullptr) {
                d3d12_resources[i]->Release();
                d3d12_resources[i]  = nullptr;
            }
        } free(d3d12_resources);
        scd->DxgiSwapChain = nullptr;
        dxgi_swapchain->Release();

        /* destroy the underlying window */
        if (DestroyWindow(wsd->WindowHandle) != FALSE) {
            HWND   hwnd = wsd->WindowHandle;
            BOOL    ret;
            MSG     msg;
            while ((ret = GetMessage(&msg, hwnd, 0, 0)) != 0) {
                if (ret == -1 || msg.message == WM_QUIT) {
                    break;
                }
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        /* remove the item from the table. 
         * this may cause another item to be moved into the vacant item's slot, 
         * in which case we need to update the window data to point to the correct location.
         * if an item was moved by TableDeleteId, window and swapchain now point to the MOVED item 
         * and not the DELETED item.
         */
        if ((moved = TableDeleteId(&table->TableDesc, handle.Bits)) != HANDLE_BITS_INVALID) {
            /* at this point wsd, scd and wdd point to the data for the moved item */
            SetWindowLongPtr(wsd->WindowHandle, GWLP_USERDATA, (LONG_PTR) wsd);
        }
    }
}

PIL_API(void)
D3D12_ProcessWindowEvents
(
    struct DISPLAY_WINDOW_TABLE_D3D12 *table
)
{
    WINDOW_STATE_DATA_D3D12 *window_itr = D3D12_WindowTableStateStreamBegin(table);
    WINDOW_STATE_DATA_D3D12 *window_end = D3D12_WindowTableStateStreamEnd(table);
    HWND                           hwnd;
    BOOL                            ret;
    MSG                             msg;

    while (window_itr != window_end) {
        hwnd = window_itr->WindowHandle;
        while ((ret = PeekMessage(&msg, hwnd, 0, 0, PM_REMOVE)) != 0) {
            if (ret < 0 || msg.message == WM_QUIT) {
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage (&msg);
        } window_itr++;
    }
}

PIL_API(int)
D3D12_QueryWindowState
(
    struct DISPLAY_WINDOW_TABLE_D3D12 *table,
    struct GPU_WINDOW_HANDLE          handle, 
    struct GPU_WINDOW_STATE         *o_state
)
{
    GPU_WINDOW_D3D12 w;
    if (D3D12_ResolveWindowForHandle(table, handle, &w) == 0) {
        WINDOW_STATE_DATA_D3D12 *wsd = w.StateData;
        o_state->StatusFlags         = wsd->StatusFlags;
        o_state->EventFlags          = wsd->EventFlags;
        o_state->OutputDpiX          = wsd->OutputDpiX;
        o_state->OutputDpiY          = wsd->OutputDpiY;
        o_state->WindowPositionX     = wsd->PositionX;
        o_state->WindowPositionY     = wsd->PositionY;
        o_state->WindowSizeX         = wsd->WindowSizeX;
        o_state->WindowSizeY         = wsd->WindowSizeY;
        o_state->ClientSizeX         = wsd->ClientSizeX;
        o_state->ClientSizeY         = wsd->ClientSizeY;
        return  0;
    } else {
        ZeroMemory(o_state, sizeof(GPU_WINDOW_STATE));
        return -1;
    }
}

#if 0
bool
D3D12WindowManager::AcquireFrame
(
    struct FRAME_RESOURCES_D3D12 *o_frame, 
    GPU_WINDOW_HANDLE              handle
)
{
    DISPLAY_OBJECTS_D3D12 record;
    if (ResolveWindow(&record, handle) == false) {
        return false;
    }
    o_frame->D3D12Device        = record.ResourceData->D3D12Device;
    o_frame->D3D12ComputeQueue  = record.ResourceData->D3D12ComputeQueue;
    o_frame->D3D12GraphicsQueue = record.ResourceData->D3D12GraphicsQueue;
    o_frame->D3D12TransferQueue = record.ResourceData->D3D12TransferQueue;
    o_frame->ImageIndex         = record.ResourceData->FrameIndex;
    o_frame->WindowHandle       = handle;
    o_frame->DxgiSwapChain      = record.SwapchainData->DxgiSwapChain;
    o_frame->StatusFlags        = record.SwapchainData->StatusFlags;
    return true;
}

bool
D3D12WindowManager::PresentFrame
(
    struct FRAME_RESOURCES_D3D12 *frame
)
{
    IDXGISwapChain3 *dxgi_swapchain = frame->DxgiSwapChain;
    uint32_t       swapchain_status = frame->StatusFlags;
    HRESULT                  result = S_OK;
    UINT              present_flags = 0;
    DISPLAY_RESOURCES_D3D12    *res;
    ID3D12CommandQueue       *queue;
    ID3D12Fence              *fence;
    HANDLE                    event;
    uint64_t                *values;
    uint64_t             curr_value;
    uint32_t             curr_index;
    uint32_t             next_index;
    DISPLAY_OBJECTS_D3D12    record;

    if (ResolveWindow(&record, frame->WindowHandle) == false) {
        return false;
    }

    /* queue the presentation */
    if (swapchain_status & D3D12_SWAPCHAIN_STATUS_FLAG_TEARING) { /* tearing support */
        if((swapchain_status & D3D12_SWAPCHAIN_STATUS_FLAG_FULLSCREEN) == 0) { /* windowed */
            present_flags = DXGI_PRESENT_ALLOW_TEARING;
        }
    }
    if (FAILED((result = dxgi_swapchain->Present(0, present_flags)))) {
        return false;
    }

    /* have the GPU signal the fence when all queued commands finish, 
     * including the frame presentation to the display output.
     */
    res        = record.ResourceData;
    queue      = res->D3D12GraphicsQueue;
    fence      = res->D3D12FenceSync;
    event      = res->FenceEvent;
    values     = res->FenceValues;
    curr_index = res->FrameIndex;
    curr_value = res->FenceValues[curr_index];
    if (FAILED((result = queue->Signal(fence, curr_value)))) {
        return false;
    }
    
    /* retrieve the index of the next swapchain image */
    next_index = dxgi_swapchain->GetCurrentBackBufferIndex();
    
    /* if necessary, wait to render the next frame */
    if (fence->GetCompletedValue() < values[next_index]) {
        if (FAILED((result = fence->SetEventOnCompletion(values[next_index], event)))) {
            return false;
        } WaitForSingleObjectEx(event, INFINITE, FALSE);
    }

    /* set the fence value for the next frame */
    values[next_index] = curr_value + 1;
    return true;
}
#endif

