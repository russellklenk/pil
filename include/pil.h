/**
 * @summary pil.h: Define the public types and entry points for the Platform 
 * Interface Layer (PIL). Platform-specific types appear in the platform-
 * specific header included by this file (pil_win32.h, pil_linux.h, etc.)
 */
#ifndef __PIL_H__
#define __PIL_H__

#pragma once

#ifndef PIL_NO_INCLUDES
#   include <assert.h>
#   include <stddef.h>
#   include <stdint.h>
#   if defined(_MSC_VER) && (_MSC_VER > 1800)
#       include <uchar.h>
#   endif
#endif

/* @summary If __STDC_UTF_16__ is defined (in uchar.h) then use existing 
 * char16_t. Otherwise, define it ourselves to be a 16-bit unsigned integer.
 */
#ifndef __STDC_UTF_16__
    typedef unsigned short                   char16_t;
#endif

/* @summary If __STDC_UTF_32__ is defined (in uchar.h) then use existing 
 * char32_t. Otherwise, define it ourselves to be a 32-bit unsigned integer.
 */
#ifndef __STDC_UTF_32__
    typedef unsigned int                     char32_t;
#endif

/* @summary Define the version of the Platform Interface Layer provided by this header.
 */
#ifndef PIL_VERSION_CONSTANTS
#   define PIL_VERSION_CONSTANTS
#   define PIL_VERSION_MAJOR                 1
#   define PIL_VERSION_MINOR                 0
#   define PIL_VERSION_BUGFIX                0
#   define PIL_VERSION_STRINGIZE_(x)         #x
#   define PIL_VERSION_STRINGIZE(x)          PIL_VERSION_STRINGIZE_(x)
#   define PIL_VERSION_STRING                                                  \
        PIL_VERSION_STRINGIZE(PIL_VERSION_MAJOR) "." PIL_VERSION_STRINGIZE(PIL_VERSION_MINOR) "." PIL_VERSION_STRINGIZE(PIL_VERSION_BUGFIX) " (" PIL_TARGET_PLATFORM_NAME "," PIL_TARGET_ARCHITECTURE_NAME "," PIL_TARGET_COMPILER_NAME ")"
#endif

/* @summary Define values used to identify the current target platform.
 */
#ifndef PIL_PLATFORM_CONSTANTS
#   define PIL_PLATFORM_CONSTANTS
#   define PIL_PLATFORM_UNKNOWN              0
#   define PIL_PLATFORM_iOS                  1
#   define PIL_PLATFORM_ANDROID              2
#   define PIL_PLATFORM_WIN32                3
#   define PIL_PLATFORM_WINRT                4
#   define PIL_PLATFORM_MACOS                5
#   define PIL_PLATFORM_LINUX                6
#endif

/* @summary Define values used to identify the current compiler.
 */
#ifndef PIL_COMPILER_CONSTANTS
#   define PIL_COMPILER_CONSTANTS
#   define PIL_COMPILER_UNKNOWN              0
#   define PIL_COMPILER_MSVC                 1
#   define PIL_COMPILER_GNUC                 2
#   define PIL_COMPILER_CLANG                3
#endif

/* @summary Define values used to identify the target processor architecture.
 * Only 64-bit architectures are supported, due to reliance on 64-bit atomic operations.
 */
#ifndef PIL_ARCHITECTURE_CONSTANTS
#   define PIL_ARCHITECTURE_CONSTANTS
#   define PIL_ARCHITECTURE_UNKNOWN          0
#   define PIL_ARCHITECTURE_X64              1
#   define PIL_ARCHITECTURE_ARM64            2
#   define PIL_ARCHITECTURE_PPC              3
#endif

/* @summary Define values used to identify the endianess of the target system.
 */
#ifndef PIL_ENDIANESS_CONSTANTS
#   define PIL_ENDIANESS_CONSTANTS
#   define PIL_ENDIANESS_UNKNOWN             0
#   define PIL_ENDIANESS_LSB_FIRST           1
#   define PIL_ENDIANESS_MSB_FIRST           2
#endif

/* @summary The PIL_TARGET_COMPILER preprocessor value can be used to specify or 
 * query the current compiler.
 */
#ifndef PIL_TARGET_COMPILER
#   define PIL_TARGET_COMPILER               PIL_COMPILER_UNKNOWN
#   define PIL_TARGET_COMPILER_NAME          "Unknown"
#endif

/* @summary The PIL_TARGET_PLATFORM preprocessor value can be used to specify or 
 * query the current target platform.
 */
#ifndef PIL_TARGET_PLATFORM
#   define PIL_TARGET_PLATFORM               PIL_PLATFORM_UNKNOWN
#   define PIL_TARGET_PLATFORM_NAME          "Unknown"
#endif

/* @summary The PIL_TARGET_ARCHITECTURE preprocessor value can be used to specify
 * or query the current target processor architecture.
 */
#ifndef PIL_TARGET_ARCHITECTURE
#   define PIL_TARGET_ARCHITECTURE           PIL_ARCHITECTURE_UNKNOWN
#   define PIL_TARGET_ARCHITECTURE_NAME      "Unknown"
#endif

/* @summary The PIL_SYSTEM_ENDIANESS preprocessor value can be used to specify or 
 * query the endianess of the host system. Default to little endian since most 
 * processor architectures these days are configurable. GCC defines the __BYTE_ORDER__
 * preprocessor value that can be used to test at compile time.
 */
#ifndef PIL_SYSTEM_ENDIANESS
#   if defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#       define PIL_SYSTEM_ENDIANESS          PIL_ENDIANESS_MSB_FIRST
#   else
#       define PIL_SYSTEM_ENDIANESS          PIL_ENDIANESS_LSB_FIRST
#   endif
#endif

/* @summary Perform compiler detection based on preprocessor directives.
 */
#if PIL_TARGET_COMPILER == PIL_COMPILER_UNKNOWN
#   if   defined(_MSC_VER)
#       undef  PIL_TARGET_COMPILER
#       undef  PIL_TARGET_COMPILER_NAME
#       define PIL_TARGET_COMPILER           PIL_COMPILER_MSVC
#       define PIL_TARGET_COMPILER_NAME      "MSVC"
#   elif defined(__clang__)
#       undef  PIL_TARGET_COMPILER
#       undef  PIL_TARGET_COMPILER_NAME
#       define PIL_TARGET_COMPILER           PIL_COMPILER_CLANG
#       define PIL_TARGET_COMPILER_NAME      "Clang"
#   elif defined(__GNUC__)
#       undef  PIL_TARGET_COMPILER
#       undef  PIL_TARGET_COMPILER_NAME
#       define PIL_TARGET_COMPILER           PIL_COMPILER_GNUC
#       define PIL_TARGET_COMPILER_NAME      "GNU"
#   else
#       error  pil.h: Failed to detect target compiler. Update compiler detection.
#   endif
#endif

/* @summary Perform processor architecture detection based on preprocessor directives.
 */
#if PIL_TARGET_ARCHITECTURE == PIL_ARCHITECTURE_UNKNOWN
#   if   defined(__aarch64__) || defined(_M_ARM64)
#       undef  PIL_TARGET_ARCHITECTURE
#       undef  PIL_TARGET_ARCHITECTURE_NAME
#       define PIL_TARGET_ARCHITECTURE       PIL_ARCHITECTURE_ARM64
#       define PIL_TARGET_ARCHITECTURE_NAME  "ARM64"
#   elif defined(__arm__) || defined(_M_ARM)
#       error  pil.h: Only 64-bit ARM platforms are supported.
#   elif defined(__amd64__) || defined(__x86_64__) || defined(_M_X64) || defined(_M_AMD64)
#       undef  PIL_TARGET_ARCHITECTURE
#       undef  PIL_TARGET_ARCHITECTURE_NAME
#       define PIL_TARGET_ARCHITECTURE       PIL_ARCHITECTURE_X64
#       define PIL_TARGET_ARCHITECTURE_NAME  "x86_64"
#   elif defined(__i386__) || defined(__i486__) || defined(__i586__) || defined(__i686__) || defined(_M_IX86) || defined(_X86_)
#       error  pil.h: Only 64-bit Intel platforms are supported.
#   elif defined(__ppc__) || defined(__powerpc__) || defined(__PPC__)
#       undef  PIL_TARGET_ARCHITECTURE
#       undef  PIL_TARGET_ARCHITECTURE_NAME
#       define PIL_TARGET_ARCHITECTURE       PIL_ARCHITECTURE_PPC
#       define PIL_TARGET_ARCHITECTURE_NAME  "PowerPC"
#   else
#       error  pil.h: Failed to detect target architecture. Update architecture detection.
#   endif
#endif

/* @summary Perform platform detection based on preprocessor directives.
 */
#if PIL_TARGET_PLATFORM == PIL_PLATFORM_UNKNOWN
#   if   defined(ANDROID)
#       undef  PIL_TARGET_PLATFORM
#       undef  PIL_TARGET_PLATFORM_NAME
#       define PIL_TARGET_PLATFORM           PIL_PLATFORM_ANDROID
#       define PIL_TARGET_PLATFORM_NAME      "Android"
#   elif defined(__APPLE__)
#       include <TargetConditionals.h>
#       if   defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
#           undef  PIL_TARGET_PLATFORM
#           undef  PIL_TARGET_PLATFORM_NAME
#           define PIL_TARGET_PLATFORM       PIL_PLATFORM_iOS
#           define PIL_TARGET_PLATFORM_NAME  "iOS"
#       else
#           undef  PIL_TARGET_PLATFORM
#           undef  PIL_TARGET_PLATFORM_NAME
#           define PIL_TARGET_PLATFORM       PIL_PLATFORM_MACOS
#           define PIL_TARGET_PLATFORM_NAME  "MacOS"
#       endif
#   elif defined(_WIN32) || defined(_WIN64) || defined(__cplusplus_winrt)
#       if   defined(__cplusplus_winrt)
#           undef  PIL_TARGET_PLATFORM
#           undef  PIL_TARGET_PLATFORM_NAME
#           define PIL_TARGET_PLATFORM       PIL_PLATFORM_WINRT
#           define PIL_TARGET_PLATFORM_NAME  "WinRT/UWP"
#       else
#           undef  PIL_TARGET_PLATFORM
#           undef  PIL_TARGET_PLATFORM_NAME
#           define PIL_TARGET_PLATFORM       PIL_PLATFORM_WIN32
#           define PIL_TARGET_PLATFORM_NAME  "Win32"
#       endif
#   elif defined(__linux__) || defined(__gnu_linux__)
#       undef  PIL_TARGET_PLATFORM
#       undef  PIL_TARGET_PLATFORM_NAME
#       define PIL_TARGET_PLATFORM           PIL_PLATFORM_LINUX
#       define PIL_TARGET_PLATFORM_NAME      "Linux"
#   else
#       error  pil.h: Failed to detect target platform. Update platform detection.
#   endif
#endif

/* @summary Abstract away some commonly-used compiler directives.
 */
#if   PIL_TARGET_COMPILER == PIL_COMPILER_MSVC
#   define PIL_NEVER_INLINE                  __declspec(noinline)
#   define PIL_FORCE_INLINE                  __forceinline
#   define PIL_STRUCT_ALIGN(_x)              __declspec(align(_x))
#   define PIL_ALIGN_OF(_x)                  __alignof(_x)
#   define PIL_RESTRICT                      __restrict
#   define PIL_SHARED_EXPORT                 __declspec(dllexport)
#   define PIL_SHARED_IMPORT                 __declspec(dllimport)
#   define PIL_OFFSET_OF(_type, _field)      offsetof(_type, _field)
#   define PIL_UNUSED_ARG(_x)                (void)(_x)
#   define PIL_UNUSED_LOCAL(_x)              (void)(_x)
#   ifdef __cplusplus
#       define PIL_INLINE                    inline
#   else
#       define PIL_INLINE                  
#   endif
#elif PIL_TARGET_COMPILER == PIL_COMPILER_GNUC || PIL_TARGET_COMPILER == PIL_COMPILER_CLANG
#   define PIL_NEVER_INLINE                  __attribute__((noinline))
#   define PIL_FORCE_INLINE                  __attribute__((always_inline))
#   define PIL_STRUCT_ALIGN(_x)              __attribute__((aligned(_x)))
#   define PIL_ALIGN_OF(_x)                  __alignof__(_x)
#   define PIL_RESTRICT                      __restrict
#   define PIL_SHARED_EXPORT                 
#   define PIL_SHARED_IMPORT                 
#   define PIL_UNUSED_ARG(_x)                (void)(sizeof(_x))
#   define PIL_UNUSED_LOCAL(_x)              (void)(sizeof(_x))
#   define PIL_OFFSET_OF(_type, _field)      offsetof(_type, _field)
#   ifdef __cplusplus
#       define PIL_INLINE                    inline
#   else
#       define PIL_INLINE                  
#   endif
#endif

/* @summary #define PIL_STATIC to make all function declarations and definitions
 * static. This is useful if the library implementation needs to be included 
 * several times within a project.
 */
#ifdef  PIL_STATIC
#   define PIL_API(_rt)                      static _rt
#else
#   define PIL_API(_rt)                      extern _rt
#endif

/* @summary Assign a value to an output argument.
 * @param _dst A pointer to the destination location.
 * @param _val The value to assign to the destination location.
 */
#ifndef PIL_Assign
#define PIL_Assign(_dst, _val)                                                 \
    if ((_dst)) *(_dst) = (_val) 
#endif

/* @summary Calculate the number of items in a fixed-length array.
 * @param _array The fixed-length array.
 * @return A pal_uint32_t specifying the number of items in the array.
 */
#ifndef PIL_CountOf
#define PIL_CountOf(_array)                                                    \
    ((uint32_t)(sizeof((_array)) / sizeof((_array)[0])))
#endif

/* @summary Retrieve the byte offset of a field from the start of a structure.
 * @param _type The typename, which must be a struct or a class type.
 * @param _field The fieldname within the struct or class type whose offset will be retrieved.
 */
#ifndef PIL_OffsetOf
#define PIL_OffsetOf(_type, _field)                                            \
    PIL_OFFSET_OF(_type, _field)
#endif

/* @summary Retrieve the size of a particular type, in bytes. 
 * @param _type A typename, such as int, specifying the type whose size is to be retrieved.
 */
#ifndef PIL_SizeOf
#define PIL_SizeOf(_type)                                                      \
    sizeof(_type)
#endif

/* @summary Retrieve the alignment of a particular type, in bytes.
 * @param _type A typename, such as int, specifying the type whose alignment is to be retrieved.
 */
#ifndef PIL_AlignOf
#define PIL_AlignOf(_type)                                                     \
    PIL_ALIGN_OF(_type)
#endif

/* @summary Align a non-zero size up to the nearest even multiple of a given power-of-two.
 * @param _quantity The size value to align up.
 * @param _alignment The desired power-of-two alignment.
 */
#ifndef PIL_AlignUp
#define PIL_AlignUp(_quantity, _alignment)                                     \
    (((_quantity) + ((_alignment)-1)) & ~((_alignment)-1))
#endif

/* @summary For a given address, return the address aligned for a particular type.
 * @param _address The unaligned address.
 * @param _type A typename, such as int, specifying the type that is to be accessed.
 */
#ifndef PIL_AlignFor
#define PIL_AlignFor(_address, _type)                                          \
    ((void*)(((uint8_t*)(_address)) + ((((PIL_ALIGN_OF(_type))-1)) & ~((PIL_ALIGN_OF(_type))-1))))
#endif

/* @summary For a given type, calculate the maximum number of bytes that will need to be allocated for a single instance.
 * @param _type A typename, such as int, specifying the type whose allocation size is being queried.
 */
#ifndef PIL_AllocationSizeType
#define PIL_AllocationSizeType(_type)                                          \
    ((sizeof(_type)) + (PIL_ALIGN_OF(_type)-1))
#endif

/* @summary For a given type, calculate the maximum number of bytes that will need to be allocated for an array of a given capacity.
 * @param _type A typename, such as int, specifying the type whose allocation size is being queried.
 * @param _count The number of elements in the array.
 */
#ifndef PIL_AllocationSizeArray
#define PIL_AllocationSizeArray(_type, _count)                                 \
    ((sizeof(_type) * (_count)) + (PIL_ALIGN_OF(_type)-1))
#endif

/* @summary Given a struct size and required alignment, calculate the maximum number of bytes that will need to be allocated for an array of a given capacity.
 * @param _objsize The object size, in bytes.
 * @param _objalign The required alignment of the object, in bytes.
 * @param _count The number of elements in the array.
 */
#ifndef PIL_AllocationSizeArrayRaw
#define PIL_AllocationSizeArrayRaw(_objsize, _objalign, _count)                \
    (((_objsize) * (_count)) + ((_objalign)-1))
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* @summary Retrieve the version of the platform interface layer.
 * @param o_major On return, the major component of the version number is stored here.
 * @param o_minor On return, the minor component of the version number is stored here.
 * @param o_bugfix On return, the bugfix component of the version number is stored here.
 */
PIL_API(void)
PIL_GetVersion
(
    int32_t *o_major, 
    int32_t *o_minor, 
    int32_t *o_bugfix
);

/* @summary Retrieve a pointer to a nul-terminated ASCII string specifying the version of the platform interface layer.
 * Do not attempt to modify or free the returned string.
 */
PIL_API(char const*)
PIL_GetVersionString
(
    void
);

#ifdef __cplusplus
}; /* extern "C" */
#endif

#endif /* __PIL_H__ */

/* @summary Include the appropriate platform-specific header to pick up definitions 
 * for platform-specific types forward-declared in this header.
 */
#if   PIL_TARGET_PLATFORM == PIL_PLATFORM_WIN32
#   include "pil_win32.h"
#elif PIL_TARGET_PLATFORM == PIL_PLATFORM_LINUX
#   include "pil_linux.h"
#else
#   error No Platform Interface Layer implementation for your platform!
#endif

