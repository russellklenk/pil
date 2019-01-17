/**
 * @summary Define the Win32-specific types associated with the dynamic library 
 * loader interface for the Platform Interface Layer.
 */
#ifndef __PIL_DYNLIB_WIN32_H__
#define __PIL_DYNLIB_WIN32_H__

#pragma once

#ifndef PIL_NO_INCLUDES
#   include <Windows.h>
#endif

/* @summary Define the data associated with an executable or dynamic library loaded into the process address space at runtime.
 */
typedef struct RUNTIME_MODULE {
    HMODULE    Handle;                                                         /* The Win32 module handle returned by LoadLibrary. */
} RUNTIME_MODULE;

#endif /* __PIL_DYNLIB_WIN32_H__ */

