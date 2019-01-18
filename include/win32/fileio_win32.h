/**
 * @summary fileio_win32.h: 
 */
#ifndef __PIL_FILEIO_WIN32_H__
#define __PIL_FILEIO_WIN32_H___

#pragma once

#ifndef PIL_NO_INCLUDES
#   include <Windows.h>
#   include <strsafe.h>
#endif

/* @summary Define the data associated with an open file.
 */
typedef struct FILE_HANDLE {
    HANDLE                         Handle;                 /* The Win32 HANDLE associated with the file. */
} FILE_HANDLE;

/* @summary Define the data retrieved about a file when it is opened or stat'd.
 */
typedef struct FILE_INFO {
    int64_t                        FileSize;               /* The size of the file, in bytes. */
    int64_t                        CreationTime;           /* The file creation time, as a Unix timestamp. */
    int64_t                        AccessTime;             /* The file last access time, as a Unix timestamp. */
    int64_t                        WriteTime;              /* The file last write time, as a Unix timestamp.*/
    uint32_t                       Alignment;              /* The required alignment for performing asynchronous I/O operations, in bytes. */
    uint32_t                       Attributes;             /* File attributes as would be returned by stat(2). */
} FILE_INFO;

/* @summary Define the data associated with a native path string parsed in-place.
 */
typedef struct PATH_PARTS {
    char_native_t                 *Root;                   /* Pointer to the first character of the root, share or drive portion of the path. */
    char_native_t                 *RootEnd;                /* Pointer to the last character of the root, share or drive portion of the path. */
    char_native_t                 *Path;                   /* Pointer to the first character of the directory portion of the path. */
    char_native_t                 *PathEnd;                /* Pointer to the last character of the directory portion of the path. */
    char_native_t                 *Filename;               /* Pointer to the first character of the filename portion of the path. */
    char_native_t                 *FilenameEnd;            /* Pointer to the last character of the filename portion of the path. */
    char_native_t                 *Extension;              /* Pointer to the first character of the extension portion of the path. */
    char_native_t                 *ExtensionEnd;           /* Pointer to the last character of the extension portion of the path. */
    uint32_t                       PathFlags;              /* One or more bitwise OR'd values of the PATH_FLAGS enumeration specifying path attributes and which components are present. */ 
} PATH_PARTS;

/* @summary Define the data associated with a filesystem enumerator object, used to report information about files and directories.
 */
typedef struct FILE_ENUMERATOR {
    HANDLE                         DirectoryHandle;        /* The Win32 handle for the root directory. */
    PFN_FileEnum                   FileCallback;           /* The callback to invoke for files encountered during enumeration. */
    PFN_FileEnum                   DirectoryCallback;      /* The callback to invoke for directories encountered during enumeration. */
    uintptr_t                      OpaqueData;             /* A pointer-sized value reserved for application use. This value is passed through unchanged to the file and directory callbacks. */
    char_native_t                 *AbsolutePath;           /* A nul-terminated string specifying the absolute path of the file or directory. */
    char_native_t                 *RelativePath;           /* A pointer to the first character of the path in the AbsolutePath buffer for the portion of the path used the initial search location. */
    char_native_t                 *SearchPath;             /* A pointer to the buffer used to specifythe search filter. */
    char_native_t                 *SearchEnd;              /* A pointer to the '*' character at the end of the search filter. */
    uint32_t                       SearchFlags;            /* One or more FILE_ENUMERATOR_FLAGS specifying the desired search behavior. */
    uint32_t                       BasePathLength;         /* The length of the base path string, in characters. */
} FILE_ENUMERATOR;

/* @summary Define the data used to configure a filesystem enumerator object.
 */
typedef struct FILE_ENUMERATOR_INIT {
    char_native_t                 *StartPath;              /* A nul-terminated string specifying the path of the directory at which enumeration will begin. */
    PFN_FileEnum                   FileCallback;           /* The callback to invoke for files encountered during enumeration. */
    PFN_FileEnum                   DirectoryCallback;      /* The callback to invoke for directories encountered during enumeration. */
    uint32_t                       SearchFlags;            /* One or more of FILE_ENUMERATOR_FLAGS specifying the desired search behavior. */
    uint32_t                       Reserved;               /* Reserved for future use. Set to zero. */
    uintptr_t                      OpaqueData;             /* A pointer-sized value reserved for application use. This value is passed through unchanged to the file and directory callbacks. */
} FILE_ENUMERATOR_INIT;

/* @summary Define the data used to request information about a file or directory.
 * assert(sizeof(IO_STAT_DATA) <= MAX_TASK_DATA_BYTES)
 */
typedef struct IO_STAT_DATA {
    char_native_t                 *Path;                   /* A nul-terminated string specifying the path to query. The string must remain valid for the duration of task execution. */
    PFN_FileStat                   Callback;               /* The callback function to invoke when the operation has completed. */
    uintptr_t                      OpaqueData;             /* A pointer-sized value reserved for application use. This value is passed through unchanged in the result structure. */
    uint32_t                       OpaqueId;               /* A 32-bit value reserved for application use. This value is passed through unchanged in the result structure. */
} IO_STAT_DATA;

/* @summary Define the data returned when a filesystem stat request completes.
 */
typedef struct IO_STAT_RESULT {
    char_native_t                 *Path;                   /* The nul-terminated string specifying the path that was queried. */
    FILE_INFO                      Info;                   /* The information returned by the stat operation. */
    int32_t                        Success;                /* This value is non-zero if the operation was successful, or zero if the stat operation failed. */
    uint32_t                       ResultCode;             /* The OS result code returned by the stat operation. */
    uintptr_t                      OpaqueData;             /* A pointer-sized value reserved for application use. This value is passed through unchanged in the result structure. */
    uint32_t                       OpaqueId;               /* A 32-bit value reserved for application use. This value is passed through unchanged in the result structure. */
} IO_STAT_RESULT;

/* @summary Define the data used to request that a file be opened.
 * assert(sizeof(IO_OPEN_DATA) <= MAX_TASK_DATA_BYTES)
 */
typedef struct IO_OPEN_DATA {
    char_native_t                 *Path;                   /* A nul-terminated string specifying the path to open. The string must remain valid for the duration of task execution. */
    PFN_FileOpen                   Callback;               /* The callback function to invoke when the operation has completed. */
    uintptr_t                      OpaqueData;             /* A pointer-sized value reserved for application use. This value is passed through unchanged in the result structure. */
    uint32_t                       OpenHints;              /* One or more FILE_OPEN_HINT_FLAGS providing hints about how the file will be used. */
    uint32_t                       OpaqueId;               /* A 32-bit value reserved for application use. This value is passed through unchanged in the result structure. */
} IO_OPEN_DATA;

/* @summary Define the data returned when a file open request completes.
 */
typedef struct IO_OPEN_RESULT {
    char_native_t                 *Path;                   /* The nul-terminated string specifying the path of the file that was opened. */
    FILE_HANDLE                    File;                   /* The file handle used to access the file. */
    FILE_INFO                      Info;                   /* The information returned by the stat operation. */
    int32_t                        Success;                /* This value is non-zero if the operation was successful, or zero if the operation failed. */
    uint32_t                       ResultCode;             /* The OS result code returned by the operation. */
    uintptr_t                      OpaqueData;             /* A pointer-sized value reserved for application use. This value is passed through unchanged in the result structure. */
    uint32_t                       OpaqueId;               /* A 32-bit value reserved for application use. This value is passed through unchanged in the result structure. */
} IO_OPEN_RESULT;

/* @summary Define the data used to request an asynchronous read from a file into a host memory buffer.
 * assert(sizeof(IO_READ_DATA) <= MAX_TASK_DATA_BYTES)
 */
typedef struct IO_READ_DATA {
    FILE_HANDLE                    File;                   /* The file to read from. */
    PFN_FileRead                   Callback;               /* The callback function to invoke when the operation has completed. */
    void                          *Destination;            /* The destination buffer. This buffer must remain valid for the duration of task execution. */
    int64_t                        ReadOffset;             /* The byte offset within the source file at which to begin reading. */
    uintptr_t                      OpaqueData;             /* A pointer-sized value reserved for application use. This value is passed through unchanged in the result structure. */
    uint32_t                       OpaqueId;               /* A 32-bit value reserved for application use. This value is passed through unchanged in the result structure. */
    uint32_t                       ReadAmount;             /* The number of bytes to read from the file. */
} IO_READ_DATA;

/* @summary Define the data returned when a file read request completes.
 */
typedef struct IO_READ_RESULT {
    FILE_HANDLE                    File;                   /* The file to read from. */
    void                          *Destination;            /* The destination buffer. This buffer must remain valid for the duration of task execution. */
    int64_t                        ReadOffset;             /* The byte offset within the source file at which to begin reading. */
    uintptr_t                      OpaqueData;             /* A pointer-sized value reserved for application use. This value is passed through unchanged in the result structure. */
    uint32_t                       OpaqueId;               /* A 32-bit value reserved for application use. This value is passed through unchanged in the result structure. */
    uint32_t                       ReadAmount;             /* The number of bytes the application requested to read from the file. */
    uint32_t                       TransferAmount;         /* The actual number of bytes read from the file into the destination buffer. */
    int32_t                        Success;                /* This value is non-zero if the operation was successful, or zero if the operation failed. */
    uint32_t                       ResultCode;             /* The OS result code returned by the operation. */
} IO_READ_RESULT;

/* @summary Define the data used to request an asynchronous write from a host memory buffer to a file.
 * assert(sizeof(IO_WRITE_DATA) <= MAX_TASK_DATA_BYTES)
 */
typedef struct IO_WRITE_DATA {
    FILE_HANDLE                    File;                   /* The file to write to. */
    PFN_FileWrite                  Callback;               /* The callback function to invoke when the operation has completed. */
    void                          *Source;                 /* The source buffer. This buffer must remain valid for the duration of task execution. */
    int64_t                        WriteOffset;            /* The byte offset within the target file at which to begin writing. */
    uintptr_t                      OpaqueData;             /* A pointer-sized value reserved for application use. This value is passed through unchanged in the result structure. */
    uint32_t                       OpaqueId;               /* A 32-bit value reserved for application use. This value is passed through unchanged in the result structure. */
    uint32_t                       WriteAmount;            /* The number of bytes to write to the file. */
} IO_WRITE_DATA;

/* @summary Define the data returned when a file write request completes.
 */
typedef struct IO_WRITE_RESULT {
    FILE_HANDLE                    File;                   /* The file to write to. */
    void                          *Source;                 /* The source buffer. This buffer must remain valid for the duration of task execution. */
    int64_t                        WriteOffset;            /* The byte offset within the target file at which to begin writing. */
    uintptr_t                      OpaqueData;             /* A pointer-sized value reserved for application use. This value is passed through unchanged in the result structure. */
    uint32_t                       OpaqueId;               /* A 32-bit value reserved for application use. This value is passed through unchanged in the result structure. */
    uint32_t                       WriteAmount;            /* The number of bytes the application requested to write to the file. */
    uint32_t                       TransferAmount;         /* The actual number of bytes written to the file from the source buffer. */
    int32_t                        Success;                /* This value is non-zero if the operation was successful, or zero if the operation failed. */
    uint32_t                       ResultCode;             /* The OS result code returned by the operation. */
} IO_WRITE_RESULT;

/* @summary Define the data used to request that a file be created with a pre-allocated size.
 * The file must then be written sequentially by the application for maximum performance.
 * assert(sizeof(IO_CREATE_DATA) <= MAX_TASK_DATA_BYTES)
 */
typedef struct IO_CREATE_DATA {
    char_native_t                 *Path;                   /* A nul-terminated string specifying the path to open. The string must remain valid for the duration of task execution. */
    PFN_FileCreate                 Callback;               /* The callback function to invoke when the operation has completed. */
    int64_t                        DesiredSize;            /* The number of bytes to pre-allocate for the file data. */
    uintptr_t                      OpaqueData;             /* A pointer-sized value reserved for application use. This value is passed through unchanged in the result structure. */
    uint32_t                       OpenHints;              /* One or more FILE_OPEN_HINT_FLAGS providing hints about how the file will be used. */
    uint32_t                       OpaqueId;               /* A 32-bit value reserved for application use. This value is passed through unchanged in the result structure. */
} IO_CREATE_DATA;

/* @summary Define the data returned when a file create request completes.
 */
typedef struct IO_CREATE_RESULT {
    char_native_t                 *Path;                   /* The nul-terminated string specifying the path of the file that was opened. */
    FILE_HANDLE                    File;                   /* The file handle used to access the file. */
    FILE_INFO                      Info;                   /* The information returned by the stat operation. */
    int32_t                        Success;                /* This value is non-zero if the operation was successful, or zero if the operation failed. */
    uint32_t                       ResultCode;             /* The OS result code returned by the operation. */
    uintptr_t                      OpaqueData;             /* A pointer-sized value reserved for application use. This value is passed through unchanged in the result structure. */
    uint32_t                       OpaqueId;               /* A 32-bit value reserved for application use. This value is passed through unchanged in the result structure. */
} IO_CREATE_RESULT;

/* @summary Define the data used to request that any buffered writes to a file be flushed to disk.
 * assert(sizeof(IO_FLUSH_DATA) <= MAX_TASK_DATA_BYTES)
 */
typedef struct IO_FLUSH_DATA {
    FILE_HANDLE                    File;                   /* The file to flush. */
    PFN_FileFlush                  Callback;               /* The callback function to invoke when the operation has completed. */
    uintptr_t                      OpaqueData;             /* A pointer-sized value reserved for application use. This value is passed through unchanged in the result structure. */
    uint32_t                       OpaqueId;               /* A 32-bit value reserved for application use. This value is passed through unchanged in the result structure. */
} IO_FLUSH_DATA;

/* @summary Define the data returned when a file flush request completes.
 */
typedef struct IO_FLUSH_RESULT {
    FILE_HANDLE                    File;                   /* The file to flush. */
    FILE_INFO                      Info;                   /* The information returned by the stat operation. */
    int32_t                        Success;                /* This value is non-zero if the operation was successful, or zero if the operation failed. */
    uint32_t                       ResultCode;             /* The OS result code returned by the operation. */
    uintptr_t                      OpaqueData;             /* A pointer-sized value reserved for application use. This value is passed through unchanged in the result structure. */
    uint32_t                       OpaqueId;               /* A 32-bit value reserved for application use. This value is passed through unchanged in the result structure. */
} IO_FLUSH_RESULT;

/* @summary Define the data used to request that a file handle be closed.
 * assert(sizeof(IO_CLOSE_DATA) <= MAX_TASK_DATA_BYTES)
 */
typedef struct IO_CLOSE_DATA {
    FILE_HANDLE                    File;                   /* The file to close. */
    PFN_FileClose                  Callback;               /* The callback function to invoke when the operation has completed. */
    uintptr_t                      OpaqueData;             /* A pointer-sized value reserved for application use. This value is passed through unchanged in the result structure. */
    uint32_t                       OpaqueId;               /* A 32-bit value reserved for application use. This value is passed through unchanged in the result structure. */
} IO_CLOSE_DATA;

/* @summary Define the data returned when a file close request completes.
 */
typedef struct IO_CLOSE_RESULT {
    FILE_HANDLE                    File;                   /* The file to close. */
    int32_t                        Success;                /* This value is non-zero if the operation was successful, or zero if the operation failed. */
    uint32_t                       ResultCode;             /* The OS result code returned by the operation. */
    uintptr_t                      OpaqueData;             /* A pointer-sized value reserved for application use. This value is passed through unchanged in the result structure. */
    uint32_t                       OpaqueId;               /* A 32-bit value reserved for application use. This value is passed through unchanged in the result structure. */
} IO_CLOSE_RESULT;

#endif /* __PIL_FILEIO_WIN32_H__ */

