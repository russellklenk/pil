/**
 * @summary display_system_d3d12.cc: Implements the Direct3D 12 and DXGI display
 * system interface.
 */
#define  INITGUID
#include "display.h"
#include "win32/display_system_d3d12.h"
#include <initguid.h>

PIL_API(uint32_t)
PhysicalToLogicalPixels
(
    uint32_t     dimension, 
    uint32_t dots_per_inch
)
{
    return D3D12_PhysicalToLogicalPixels(dimension, dots_per_inch);
}

PIL_API(uint32_t)
LogicalToPhysicalPixels
(
    uint32_t     dimension, 
    uint32_t dots_per_inch
)
{
    return D3D12_LogicalToPhysicalPixels(dimension, dots_per_inch);
}

PIL_API(struct DISPLAY_SYSTEM*)
DisplaySystemCreate
(
    struct DISPLAY_SYSTEM_INIT *init
)
{
    DISPLAY_SYSTEM_D3D12    *system = nullptr;
    ID3D12Debug        *d3d12_debug = nullptr;
    IDXGIDebug          *dxgi_debug = nullptr;
    IDXGIDebug1        *dxgi_debug1 = nullptr;
    IDXGIInfoQueue *dxgi_debug_info = nullptr;
    IDXGIFactory2          *factory = nullptr;
    IDXGIFactory5         *factory5 = nullptr;
    IDXGIFactory7         *factory7 = nullptr;
    HRESULT                  result = S_OK;
    uint32_t           num_adapters = 0;
    uint32_t            num_outputs = 0;
    uint32_t      dxgi_loader_flags = 0;
    DWORD        dxgi_factory_flags = 0;
    BOOL                    tearing = FALSE;

    /* allocate and zero-initialize the memory block */
    if ((system = (DISPLAY_SYSTEM_D3D12*) malloc(sizeof(DISPLAY_SYSTEM_D3D12))) == NULL) {
        goto cleanup_and_fail;
    } ZeroMemory(system, sizeof(DISPLAY_SYSTEM_D3D12));

    /* set flags for debugging support if requested */
    if (init->CreateFlags & DISPLAY_SYSTEM_CREATE_FLAG_DEBUG) {
        dxgi_loader_flags = DXGIAPI_LOADER_FLAG_DEBUG_SUPPORT;
        dxgi_factory_flags= DXGI_CREATE_FACTORY_DEBUG;
    }

    /* resolve runtime APIs - if Direct3D 12 is not available, fail */
    DXGIApiPopulateDispatch (&system->Dispatch_DXGI , dxgi_loader_flags);
    Win32ApiPopulateDispatch(&system->Dispatch_Win32, WIN32API_LOADER_FLAGS_NONE);
    D3D12ApiPopulateDispatch(&system->Dispatch_D3D12, D3D12API_LOADER_FLAGS_NONE);
    if (D3D12ApiQuerySupport(&system->Dispatch_D3D12) == 0) {
        goto cleanup_and_fail;
    }

    /* enable debugging support if requested - if this fails, it's not fatal */
    if (init->CreateFlags & DISPLAY_SYSTEM_CREATE_FLAG_DEBUG) {
        result = system->Dispatch_DXGI.DXGIGetDebugInterface  (IID_PPV_ARGS(&dxgi_debug));
        result = system->Dispatch_DXGI.DXGIGetDebugInterface  (IID_PPV_ARGS(&dxgi_debug_info));
        result = system->Dispatch_DXGI.DXGIGetDebugInterface1 (0, IID_PPV_ARGS(&dxgi_debug1));
        result = system->Dispatch_D3D12.D3D12GetDebugInterface(IID_PPV_ARGS(&d3d12_debug));
        if (dxgi_debug_info) {
            dxgi_debug_info->AddApplicationMessage(DXGI_INFO_QUEUE_MESSAGE_SEVERITY_INFO, "DXGI debug enabled");
            dxgi_debug_info->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, TRUE);
            dxgi_debug_info->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, TRUE);
            dxgi_debug_info->SetMuteDebugOutput(DXGI_DEBUG_ALL, FALSE);
            dxgi_debug_info->Release();
            dxgi_debug_info = nullptr;
        }
        if (dxgi_debug1) {
            dxgi_debug1->EnableLeakTrackingForThread();
        }
        if (d3d12_debug) {
            d3d12_debug->EnableDebugLayer();
        }
        system->DxgiDebug  =  dxgi_debug;
        system->DxgiDebug1 =  dxgi_debug1;
        system->D3D12Debug = d3d12_debug;
    }

    /* create the tables for the device manager */
    if (D3D12_CreateDisplayOutputTable(&system->OutputTable, DISPLAY_SYSTEM_D3D12_MAX_OUTPUTS) != 0) {
        goto cleanup_and_fail;
    }
    if (D3D12_CreateDisplayAdapterTable(&system->AdapterTable, DISPLAY_SYSTEM_D3D12_MAX_ADAPTERS) != 0) {
        goto cleanup_and_fail;
    }
    if (D3D12_CreateDisplayDeviceTable(&system->DeviceTable, DISPLAY_SYSTEM_D3D12_MAX_LOGICAL_DEVICES) != 0) {
        goto cleanup_and_fail;
    }
    if (D3D12_CreateDisplayWindowTable(&system->WindowTable, DISPLAY_SYSTEM_D3D12_MAX_WINDOWS) != 0) {
        goto cleanup_and_fail;
    }

    /* the display system supports adapter attach & remove */
    system->Dispatch_DXGI.DXGIDeclareAdapterRemovalSupport();

    /* create the factory used to enumerate displays and outputs */
    if (FAILED((result = system->Dispatch_DXGI.CreateDXGIFactory2(dxgi_factory_flags, IID_PPV_ARGS(&factory))))) {
        goto cleanup_and_fail;
    } system->DxgiFactory = factory;

    if (D3D12_RefreshDisplayHardware(&system->AdapterTable, &system->OutputTable, &num_adapters, &num_outputs, factory) != 0) {
        goto cleanup_and_fail;
    }

    /* check whether various features are available */
    if (SUCCEEDED(factory->QueryInterface(IID_PPV_ARGS(&factory5)))) {
        if (SUCCEEDED(factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &tearing, sizeof(BOOL)))) {
            if (tearing) {
                system->FeatureFlags |= DISPLAY_SYSTEM_FEATURE_FLAG_D3D12_TEARING;
            }
        } factory5->Release();
    }
    if (SUCCEEDED(factory->QueryInterface(IID_PPV_ARGS(&factory7)))) {
        /* TODO: register for adapter change notification */
        factory7->Release();
    }

    return (struct DISPLAY_SYSTEM*) system;

cleanup_and_fail:
    if (system) {
        D3D12_DeleteDisplayWindowTable (&system->WindowTable );
        D3D12_DeleteDisplayDeviceTable (&system->DeviceTable );
        D3D12_DeleteDisplayOutputTable (&system->OutputTable );
        D3D12_DeleteDisplayAdapterTable(&system->AdapterTable);
        if (factory) {
            factory->Release();
        }
        if (d3d12_debug) {
            d3d12_debug->Release();
        }
        if (dxgi_debug1) {
            dxgi_debug1->Release();
        }
        if (dxgi_debug) {
            dxgi_debug->Release();
        }
        Win32ApiInvalidateDispatch(&system->Dispatch_Win32);
        D3D12ApiInvalidateDispatch(&system->Dispatch_D3D12);
        DXGIApiInvalidateDispatch (&system->Dispatch_DXGI);
        free(system);
    }
    return NULL;
}

PIL_API(void)
DisplaySystemDelete
(
    struct DISPLAY_SYSTEM *system
)
{
    if (system) {
        DISPLAY_SYSTEM_D3D12 *system_ = (DISPLAY_SYSTEM_D3D12*) system;
        /* ... */
        D3D12_DeleteDisplayWindowTable (&system_->WindowTable );
        D3D12_DeleteDisplayDeviceTable (&system_->DeviceTable );
        D3D12_DeleteDisplayOutputTable (&system_->OutputTable );
        D3D12_DeleteDisplayAdapterTable(&system_->AdapterTable);
        /* release retained COM interfaces */
        if (system_->DxgiFactory) {
            system_->DxgiFactory->Release();
            system_->DxgiFactory = nullptr;
        }
        if (system_->D3D12Debug) {
            system_->D3D12Debug->Release();
            system_->D3D12Debug = nullptr;
        }
        if (system_->DxgiDebug1) {
            system_->DxgiDebug1->Release();
            system_->DxgiDebug1 = nullptr;
        }
        if (system_->DxgiDebug) {
            system_->DxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
            system_->DxgiDebug->Release();
            system_->DxgiDebug = nullptr;
        }
        /* unload runtime modules */
        Win32ApiInvalidateDispatch(&system_->Dispatch_Win32);
        D3D12ApiInvalidateDispatch(&system_->Dispatch_D3D12);
        DXGIApiInvalidateDispatch (&system_->Dispatch_DXGI);
        free(system);
    }
}

PIL_API(uint32_t)
DescribeDisplayAdapters
(
    struct DISPLAY_ADAPTER_DESC *o_desc_array, 
    uint32_t                      start_index, 
    uint32_t                      max_results, 
    struct DISPLAY_SYSTEM             *system
)
{
    DISPLAY_SYSTEM_D3D12  *system_ =(DISPLAY_SYSTEM_D3D12*) system;
    return D3D12_DescribeDisplayAdapters(&system_->AdapterTable, o_desc_array, start_index, max_results);
}

PIL_API(uint32_t)
DescribeDisplayOutputs
(
    struct DISPLAY_OUTPUT_DESC *o_desc_array, 
    uint32_t                     start_index, 
    uint32_t                     max_results, 
    struct DISPLAY_SYSTEM            *system
)
{
    DISPLAY_SYSTEM_D3D12  *system_ =(DISPLAY_SYSTEM_D3D12 *) system;
    WIN32API_DISPATCH    *win32api = D3D12_GetWin32Dispatch (system_);
    return D3D12_DescribeDisplayOutputs(&system_->OutputTable, o_desc_array, start_index, max_results, win32api);
}

PIL_API(int)
GpuDeviceCreate
(
    struct GPU_DEVICE_HANDLE *o_handle, 
    struct GPU_DEVICE_INIT       *init, 
    struct DISPLAY_SYSTEM      *system
)
{
    DISPLAY_SYSTEM_D3D12 *system_ =(DISPLAY_SYSTEM_D3D12*) system;
    GPU_DEVICE_INIT_D3D12  init12 = {};
    GPU_DEVICE_HANDLE      handle = {};
    init12.D3D12Dispatch = D3D12_GetD3D12Dispatch(system_);
    init12.AdapterTable  = D3D12_GetDisplayAdapterTable(system_);
    init12.TargetAdapter = init->AdapterIdentifier;
    init12.UsageFlags    = init->UsageFlags;
    return D3D12_CreateLogicalGpuDevice(&system_->DeviceTable, &init12, o_handle);
}

PIL_API(void)
GpuDeviceDelete
(
    struct GPU_DEVICE_HANDLE handle, 
    struct DISPLAY_SYSTEM   *system
)
{
    DISPLAY_SYSTEM_D3D12  *system_ =(DISPLAY_SYSTEM_D3D12*) system;
    D3D12_DeleteLogicalGpuDevice(&system_->DeviceTable, handle);
}

PIL_API(int)
GpuWindowCreate
(
    struct GPU_WINDOW_HANDLE *o_handle, 
    struct GPU_WINDOW_INIT       *init, 
    struct DISPLAY_SYSTEM      *system
)
{
    DISPLAY_SYSTEM_D3D12            *system_ =(DISPLAY_SYSTEM_D3D12*) system;
    GPU_WINDOW_INIT_D3D12             init12 = {};
    GPU_WINDOW_HANDLE                 handle = {};
    DISPLAY_DEVICE_TABLE_D3D12 *device_table = D3D12_GetDeviceTable(system_);
    DISPLAY_OUTPUT_TABLE_D3D12 *output_table = D3D12_GetDisplayOutputTable(system_);
    DISPLAY_OUTPUT_D3D12             *output = D3D12_ResolveOutputForKey(output_table, init->OutputIdentifier);
    DISPLAY_DEVICE_D3D12             *device = D3D12_ResolveDeviceForHandle(device_table, init->GpuDevice);
    IDXGIOutput                 *dxgi_output = nullptr;

    if (init->OutputIdentifier != 0 && output == nullptr) {
        assert(output != nullptr && "Failed to resolve OutputIdentifier");
        if (o_handle) {
           *o_handle = GPU_WINDOW_HANDLE{HANDLE_BITS_INVALID};
        } return -1;
    }
    if (device == nullptr) {
        assert(device != nullptr && "Failed to resolve GpuDevice handle");
        if (o_handle) {
           *o_handle = GPU_WINDOW_HANDLE{HANDLE_BITS_INVALID};
        } return -1;
    }
    if (output != nullptr) {
        dxgi_output = output->DxgiOutput;
    }

    init12.OutputTable       = D3D12_GetDisplayOutputTable(system_);
    init12.Win32ApiDispatch  = D3D12_GetWin32Dispatch(system_);
    init12.DxgiFactory       = D3D12_GetDxgiFactory(system_);
    init12.DxgiOutput        = dxgi_output;
    init12.D3D12Device       = device->D3D12Device;
    init12.D3D12CommandQueue = device->GraphicsCommandQueue;
    init12.WindowTitle       = L"DX12 Window";// init->WindowTitle; // TODO: convert to WCHAR
    init12.ClientWidth       = init->ClientWidth;
    init12.ClientHeight      = init->ClientHeight;
    init12.SwapchainWidth    = init->ClientWidth;
    init12.SwapchainHeight   = init->ClientHeight;
    init12.FeatureFlags      = D3D12_GetFeatureFlags(system_);
    return D3D12_CreateWindowAndSwapchain(&system_->WindowTable, &init12, o_handle);
}

PIL_API(void)
DestroyGpuWindow
(
    struct GPU_WINDOW_HANDLE handle, 
    struct DISPLAY_SYSTEM   *system
)
{
    DISPLAY_SYSTEM_D3D12     *system_ =(DISPLAY_SYSTEM_D3D12*) system;
    DISPLAY_WINDOW_TABLE_D3D12 *table = D3D12_GetWindowTable(system_);
    return D3D12_DeleteWindowAndSwapchain(table, handle);
}

PIL_API(void)
ProcessGpuWindowEvents
(
    struct DISPLAY_SYSTEM *system
)
{
    DISPLAY_SYSTEM_D3D12     *system_ =(DISPLAY_SYSTEM_D3D12*) system;
    DISPLAY_WINDOW_TABLE_D3D12 *table = D3D12_GetWindowTable(system_);
    return D3D12_ProcessWindowEvents(table);
}

PIL_API(int)
QueryGpuWindowStatus
(
    struct GPU_WINDOW_STATE *o_state, 
    struct GPU_WINDOW_HANDLE  handle, 
    struct DISPLAY_SYSTEM    *system
)
{
    DISPLAY_SYSTEM_D3D12     *system_ =(DISPLAY_SYSTEM_D3D12*) system;
    DISPLAY_WINDOW_TABLE_D3D12 *table = D3D12_GetWindowTable(system_);
    return D3D12_QueryWindowState(table, handle, o_state);
}

