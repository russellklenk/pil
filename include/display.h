/**
 * @summary display.h: Define types and functions related to display management.
 * Most of the types are platform-specific, and defined in a platform header.
 */
#ifndef __PIL_DISPLAY_H__
#define __PIL_DISPLAY_H__

#pragma once

#ifndef PIL_NO_INCLUDES
#   ifndef __PIL_H__
#       include "pil.h"
#   endif
#   ifndef __PIL_MEMMGR_H__
#       include "memmgr.h"
#   endif
#endif

/* @summary Define the maximum number of display adapters that can be recognized by the platform interface layer.
 */
#ifndef PIL_MAX_DISPLAY_ADAPTERS
#   define PIL_MAX_DISPLAY_ADAPTERS    8
#endif

/* @summary Define the maximum number of display outputs that can be rendered to by the platform interface layer.
 */
#ifndef PIL_MAX_DISPLAY_OUTPUTS
#   define PIL_MAX_DISPLAY_OUTPUTS     8
#endif

/* @summary Forward-declare the types exported by this module.
 */
struct  DISPLAY_INTERFACE;

/* @summary Define types used to hold values uniquely identifying display outputs and adapters.
 * Indices cannot be used since storage indices may change when devices are attached or removed.
 */
typedef uint64_t DISPLAY_OUTPUT_ID;
typedef uint64_t DISPLAY_ADAPTER_ID;

#ifdef __cplusplus
extern "C" {
#endif

/* @summary Allocate and initialize a DISPLAY_INTERFACE for the application.
 * This enumerates all display adapters and display outputs attached to the system.
 * The DISPLAY_INTERFACE is owned by the PIL_CONTEXT, and is only destroyed when the context is deleted.
 * @param arena The memory arena from which the DISPLAY_INTERFACE will be allocated.
 * @return A pointer to the display interface, or NULL.
 */
PIL_API(struct DISPLAY_INTERFACE*)
CreateDisplayInterface
(
    struct MEMORY_ARENA *arena
);

/* @summary Retrieve the number of display adapters (GPUs) attached to the host system.
 * Display adapter attach and remove events used to update this count are processed during ProcessDisplayEvents.
 * @param dispif The DISPLAY_INTERFACE to query.
 * @return The number of display adapters attached to the system.
 */
PIL_API(size_t)
GetDisplayAdapterCount
(
    struct DISPLAY_INTERFACE *dispif
);

/* @summary Retrieve the number of display outputs (monitors) attached to the host system.
 * Display output attach and remove events used to update this count are processed during ProcessDisplayEvents.
 * @param dispif The DISPLAY_INTERFACE to query.
 * @return The number of display outputs attached to the system.
 */
PIL_API(size_t)
GetDisplayOutputCount
(
    struct DISPLAY_INTERFACE *dispif
);

/* @summary Process any pending events from the display system.
 * No other calls should be made into the display system until this function returns.
 * @param dispif The DISPLAY_INTERFACE to update.
 */
PIL_API(void)
ProcessDisplayEvents
(
    struct DISPLAY_INTERFACE *dispif
);

/* @summary Free resources associated with the display context.
 * @param dispif The DISPLAY_INTERFACE to delete.
 */
PIL_API(void)
DeleteDisplayInterface
(
    struct DISPLAY_INTERFACE *dispif
);

#ifdef __cplusplus
}; /* extern "C" */
#endif

#endif /* __PIL_DISPLAY_H__ */

#if   PIL_TARGET_PLATFORM == PIL_PLATFORM_WIN32
#   include "display_win32.h"
#elif PIL_TARGET_PLATFORM == PIL_PLATFORM_LINUX
#   include "display_linux.h"
#else
#   error No Platform Interface Layer display module for your platform!
#endif

