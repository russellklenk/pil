/**
 * @summary Implement The display-related functions of the platform interface 
 * layer using the DXGI interfaces.
 */
#include "memmgr.h"
#include "display.h"

int CreateDxgi
(
    void
)
{
    HRESULT res = S_OK;
    IDXGIFactory2 *dxgi_factory2 = NULL;

    if (SUCCEEDED((res = CreateDXGIFactory2(0, IID_PPV_ARGS(&dxgi_factory2))))) {
        return 1;
    }
    return 0;
}

PIL_API(struct DISPLAY_INTERFACE*)
CreateDisplayInterface
(
    struct MEMORY_ARENA *arena
)
{
    DISPLAY_INTERFACE *dispif = NULL;

    if (arena == NULL) {
    }
}

PIL_API(size_t)
GetDisplayAdapterCount
(
    struct DISPLAY_INTERFACE *dispif
);

PIL_API(size_t)
GetDisplayOutputCount
(
    struct DISPLAY_INTERFACE *dispif
);

PIL_API(void)
ProcessDisplayEvents
(
    struct DISPLAY_INTERFACE *dispif
);

PIL_API(void)
DeleteDisplayInterface
(
    struct DISPLAY_INTERFACE *dispif
);

