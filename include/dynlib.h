/**
 * @summary dynlib.h: Define types, functions and macros for dynamically loading
 * code modules into the process address space and resolving entry points.
 */
#ifndef __PIL_DYNLIB_H__
#define __PIL_DYNLIB_H__

#pragma once

#ifndef PIL_NO_INCLUDES
#   ifndef __PIL_H__
#       include "pil.h"
#   endif
#endif

/* @summary Helper macro for populating a dispatch table with functions loaded at runtime.
 * If the function is not found, the entry point is updated to point to a stub implementation provided by the caller.
 * This macro relies on specific naming conventions:
 * - The signature must be PFN_BlahBlahBlah where BlahBlahBlah corresponds to the _func argument.
 * - The dispatch table field must be BlahBlahBlah where BlahBlahBlah corresponds to the _func argument.
 * - The stub function must be named BlahBlahBlah_Stub where BlahBlahBlah corresponds to the _func argument.
 * @param _disp A pointer to the dispatch table to populate.
 * @param _module A pointer to the RUNTIME_MODULE representing the module loaded into the process address space.
 * @param _func The name of the function to dynamically load.
 */
#ifndef RuntimeFunctionResolve
#define RuntimeFunctionResolve(_disp, _module, _func)                          \
    for (;;) {                                                                 \
        (_disp)->_func=(PFN_##_func) RuntimeModuleResolve((_module), #_func);  \
        if ((_disp)->_func == NULL) {                                          \
            (_disp)->_func  = _func##_Stub;                                    \
        } break;                                                               \
    }
#endif

/* @summary Set a runtime-resolved function entry point to point at the stub implementation provided by the application.
 * @param _disp A pointer to the dispatch table to populate.
 * @param _func The name of the function to dynamically load.
 */
#ifndef RuntimeFunctionSetStub
#define RuntimeFunctionSetStub(_disp, _func)                                   \
    (_disp)->_func=(PFN_##_func) _func##_Stub
#endif

/* @summary Determine whether a runtime-resolved function was resolved to its stub implementation, meaning that it is not implemented on the host.
 * @param _disp A pointer to the dispatch table.
 * @param _func The name of the function to check.
 * @return Non-zero if the dispatch table entry for _func points to the stub implementation.
 */
#ifndef RuntimeFunctionIsMissing
#define RuntimeFunctionIsMissing(_disp, _func)                                 \
    (_disp)->_func == _func##_Stub
#endif

/* @summary Define a general signature for a dynamically loaded function. 
 * Code will have to case the function pointer to the specific type.
 */
typedef int (*PFN_Unknown)(void);

/* @summary Forward-declare the types exported by this module.
 * The type definitions are included in the platform-specific header.
 */
struct  RUNTIME_MODULE;

#ifdef __cplusplus
extern "C" {
#endif

/* @summary Initialize a RUNTIME_MODULE instance such that RuntimeModuleIsValid will return zero.
 * @param module The RUNTIME_MODULE to initialize.
 */
PIL_API(void)
RuntimeModuleInit
(
    struct RUNTIME_MODULE *module
);

/* @summary Attempt to load a named module into the process address space.
 * @param module On return, the handle of the loaded module is written to this location.
 * @param path The path and filename of the module to load. 
 * @return Zero if the module is successfully loaded, or non-zero if the module could not be loaded.
 */
PIL_API(int)
RuntimeModuleLoad
(
    struct RUNTIME_MODULE *module, 
    char const              *path
);

/* @summary Decrement the reference count on a loaded module. If the module reference count reaches zero, the module is unloaded from the process address space.
 * @param module The handle of the module to unload, returned by a prior call to RuntimeModuleLoad.
 */
PIL_API(void)
RuntimeModuleUnload
(
    struct RUNTIME_MODULE *module
);

/* @summary Determine whether a RUNTIME_MODULE represents a valid module handle.
 * @param module The module handle to inspect.
 * @return Non-zero if the module handle is valid, or zero if the module handle is invalid.
 */
PIL_API(int)
RuntimeModuleIsValid
(
    struct RUNTIME_MODULE *module
);

/* @summary Resolve a function within a module loaded into the process address space.
 * @param module THe handle of the module that defines the symbol.
 * @param symbol A nul-terminated ANSI string specifying the mangled name of the exported symbol.
 * @return The address of the symbol within the process address space, or NULL if no symbol with the specified name was found.
 */
PIL_API(PFN_Unknown)
RuntimeModuleResolve
(
    struct RUNTIME_MODULE *module, 
    char const        *symbol
);

#ifdef __cplusplus
}; /* extern "C" */
#endif

#endif /* __PIL_DYNLIB_H__ */

#if   PIL_TARGET_PLATFORM == PIL_PLATFORM_WIN32
#   include "win32/dynlib_win32.h"
#elif PIL_TARGET_PLATFORM == PIL_PLATFORM_LINUX
#   include "linux/dynlib_linux.h"
#else
#   error No Platform Interface Layer dynamic library module for your platform!
#endif

