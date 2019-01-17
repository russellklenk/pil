/**
 * @summary Implement the runtime loading portion of the Platform Interface 
 * Layer for the Windows platform.
 */
#include "dynlib.h"

PIL_API(void)
RuntimeModuleInit
(
    struct RUNTIME_MODULE *module
)
{
    module->Handle = NULL;
}

PIL_API(int)
RuntimeModuleLoad
(
    struct RUNTIME_MODULE *module, 
    char const              *path
)
{
    module->Handle = LoadLibraryA(path);
    return (module->Handle != NULL) ? 0 : -1;
}

PIL_API(void)
RuntimeModuleUnload
(
    struct RUNTIME_MODULE *module
)
{
    if (module->Handle != NULL) {
        FreeLibrary(module->Handle);
        module->Handle = NULL;
    }
}

PIL_API(int)
RuntimeModuleIsValid
(
    struct RUNTIME_MODULE *module
)
{
    return (module->Handle != NULL) ? 1 : 0;
}

PIL_API(PFN_Unknown)
RuntimeModuleResolve
(
    struct RUNTIME_MODULE *module, 
    char const            *symbol
)
{
    if (module->Handle != NULL) {
        return (PFN_Unknown) GetProcAddress(module->Handle, symbol);
    } else {
        return (PFN_Unknown) NULL;
    }
}

