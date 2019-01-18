/**
 * @summary fileio.h: Define the interface to platform file I/O routines.
 */
#ifndef __PIL_FILEIO_H__
#define __PIL_FILEIO_H__

#pragma once

#ifndef PIL_NO_INCLUDES
#   ifndef __PIL_H__
#       include "pil.h"
#   endif
#endif

/* @summary Forward-declare the types exported by this module.
 * The type definitions are included in the platform-specific header.
 */
struct  FILE_HANDLE;
struct  FILE_INFO;
struct  PATH_PARTS;
struct  FILE_ENUMERATOR;
struct  FILE_ENUMERATOR_INIT;
struct  IO_STAT_DATA;
struct  IO_STAT_RESULT;
struct  IO_OPEN_DATA;
struct  IO_OPEN_RESULT;
struct  IO_READ_DATA;
struct  IO_READ_RESULT;
struct  IO_CREATE_DATA;
struct  IO_CREATE_RESULT;
struct  IO_WRITE_DATA;
struct  IO_WRITE_RESULT;
struct  IO_FLUSH_DATA;
struct  IO_FLUSH_RESULT;
struct  IO_CLOSE_DATA;
struct  IO_CLOSE_RESULT;
struct  TASK_ARGS;

/* @summary Define the signature for the callback function invoked when a file stat task has completed.
 * @param args A TASK_ARGS instance specifying data associated with the task and data used to spawn additional tasks.
 * @param result Information returned by the operation.
 */
typedef void (*PFN_FileStat)
(
    struct TASK_ARGS        *args, 
    struct IO_STAT_RESULT *result
);

/* @summary Define the signature for the callback function invoked when a file open task has completed.
 * @param args A TASK_ARGS instance specifying data associated with the task and data used to spawn additional tasks.
 * @param result Information returned by the operation.
 */
typedef void (*PFN_FileOpen)
(
    struct TASK_ARGS        *args, 
    struct IO_OPEN_RESULT *result
);

/* @summary Define the signature for the callback function invoked when a file create task has completed.
 * @param args A TASK_ARGS instance specifying data associated with the task and data used to spawn additional tasks.
 * @param result Information returned by the operation.
 */
typedef void (*PFN_FileCreate)
(
    struct TASK_ARGS          *args, 
    struct IO_CREATE_RESULT *result
);

/* @summary Define the signature for the callback function invoked when a file read task has completed.
 * @param args A TASK_ARGS instance specifying data associated with the task and data used to spawn additional tasks.
 * @param result Information returned by the operation.
 */
typedef void (*PFN_FileRead)
(
    struct TASK_ARGS        *args, 
    struct IO_READ_RESULT *result
);

/* @summary Define the signature for the callback function invoked when a file write task has completed.
 * @param args A TASK_ARGS instance specifying data associated with the task and data used to spawn additional tasks.
 * @param result Information returned by the operation.
 */
typedef void (*PFN_FileWrite)
(
    struct TASK_ARGS         *args, 
    struct IO_WRITE_RESULT *result
);

/* @summary Define the signature for the callback function invoked when a file flush task has completed.
 * @param args A TASK_ARGS instance specifying data associated with the task and data used to spawn additional tasks.
 * @param result Information returned by the operation.
 */
typedef void (*PFN_FileFlush)
(
    struct TASK_ARGS         *args, 
    struct IO_FLUSH_RESULT *result
);

/* @summary Define the signature for the callback function invoked when a file close task has completed.
 * @param args A TASK_ARGS instance specifying data associated with the task and data used to spawn additional tasks.
 * @param result Information returned by the operation.
 */
typedef void (*PFN_FileClose)
(
    struct TASK_ARGS         *args, 
    struct IO_CLOSE_RESULT *result
);

/* @summary Define the signature for the callback function invoked when a file or directory is encountered during filesystem enumeration.
 * @param fsenum The FILE_ENUMERATOR performing the enumeration and invoking the callback.
 * @param absolute_path A nul-terminated string specifying the absolute path of the file or directory.
 * @param relative_path A nul-terminated string specifying the path of the file or directory, relative to the starting point of enumeration.
 * @param entry_name A nul-terminated string specifying the file or directory name, including the extension (if any).
 * @param entry_info A FILE_INFO specifying attributes such as the creation and last write time.
 * @param context The opaque context data specified when the file enumerator was created.
 * @return Non-zero to continue enumeration, or zero to stop enumeration.
 */
typedef int  (*PFN_FileEnum)
(
    struct FILE_ENUMERATOR *fsenum, 
    char_native_t   *absolute_path, 
    char_native_t   *relative_path, 
    char_native_t      *entry_name, 
    struct FILE_INFO   *entry_info, 
    uintptr_t              context
);

/* @summary Define various flags that can be bitwise OR'd to control the behavior of a filesystem enumerator.
 */
typedef enum FILE_ENUMERATOR_FLAGS {
    FILE_ENUMERATOR_FLAGS_NONE             = (0UL <<  0),                      /* Do not recurse or report any files or directories. */
    FILE_ENUMERATOR_FLAG_FILES             = (1UL <<  0),                      /* Report regular file entries during enumeration. */
    FILE_ENUMERATOR_FLAG_DIRECTORIES       = (1UL <<  1),                      /* Report directory entries during enumeration. */
    FILE_ENUMERATOR_FLAG_RECURSIVE         = (1UL <<  2),                      /* Recurse into subdirectories. */
} FILE_ENUMERATOR_FLAGS;

/* @summary Define hint flags that can be used to optimize asynchronous I/O operations.
 */
typedef enum FILE_OPEN_HINT_FLAGS {
    FILE_OPEN_HINT_FLAGS_NONE              = (0UL <<  0),                      /* No I/O hints are specified. Use the default behavior appropriate for the operation. */
    FILE_OPEN_HINT_FLAG_READ               = (1UL <<  0),                      /* Read operations will be issued against the file. */
    FILE_OPEN_HINT_FLAG_WRITE              = (1UL <<  1),                      /* Write operations will be issued against the file. */
    FILE_OPEN_HINT_FLAG_OVERWRITE          = (1UL <<  2),                      /* The existing file contents should be discarded. */
    FILE_OPEN_HINT_FLAG_PREALLOCATE        = (1UL <<  3),                      /* Preallocate the file to the specified size. */
    FILE_OPEN_HINT_FLAG_SEQUENTIAL         = (1UL <<  4),                      /* Optimize for sequential file access when performing cached/buffered I/O. */
    FILE_OPEN_HINT_FLAG_UNCACHED           = (1UL <<  5),                      /* File I/O should bypass the operating system page cache. The source/destination buffer must be aligned to a sector boundary and have a size that's an even multiple of the disk sector size. */
    FILE_OPEN_HINT_FLAG_WRITE_THROUGH      = (1UL <<  6),                      /* Writes should be immediately flushed to disk. */
    FILE_OPEN_HINT_FLAG_TEMPORARY          = (1UL <<  7),                      /* The file is temporary, and will be deleted when the file handle is closed. */
    FILE_OPEN_HINT_FLAG_ASYNCHRONOUS       = (1UL <<  8),                      /* The file should support asynchronous I/O operations. */
} FILE_OPEN_HINT_FLAGS;

/* @summary Define the various flags that can be associated with a parsed path string.
 * These values can be bitwise-ORd in the PATH_PARTS::Flags field.
 */
typedef enum PATH_FLAGS {
    PATH_FLAGS_INVALID                     = (0UL <<  0),                      /* No flags are specified for the path, or the path cannot be parsed. */
    PATH_FLAG_ABSOLUTE                     = (1UL <<  0),                      /* The path string specifies an absolute path. */
    PATH_FLAG_RELATIVE                     = (1UL <<  1),                      /* The path string specifies a relative path. */
    PATH_FLAG_NETWORK                      = (1UL <<  2),                      /* The path string specifies a path in UNC format. */
    PATH_FLAG_DEVICE                       = (1UL <<  3),                      /* The path string specifies a device path. */
    PATH_FLAG_LONG                         = (1UL <<  4),                      /* The path string specifies a long-form path, with a maximum length of 32767 characters, not including the nul. */
    PATH_FLAG_ROOT                         = (1UL <<  5),                      /* The path string has a root component. */
    PATH_FLAG_PATH                         = (1UL <<  6),                      /* The path string has a directory component. */
    PATH_FLAG_FILENAME                     = (1UL <<  7),                      /* The path string has a filename component. */
    PATH_FLAG_EXTENSION                    = (1UL <<  8),                      /* The path string has a file extension component. */
} PATH_FLAGS;

#ifdef __cplusplus
extern "C" {
#endif

/* @summary Parse a path string, in place, into its constituient parts.
 * @param parts The PATH_PARTS to populate with the start and end of each portion of the path string.
 * @param path_buf The buffer to parse, containing a native path string.
 * @param path_end A pointer to the last character of the input buffer to parse, or NULL to scan for a nul terminator.
 * @return Zero if the path was successfully parsed, or -1 if an error occurred.
 */
PIL_API(int)
PathParse
(
    struct PATH_PARTS *parts, 
    char_native_t  *path_buf, 
    char_native_t  *path_end
);

/* @summary Given an open file, retrieve the absolute native path of the file.
 * @param buffer The destination buffer to receive the absolute native path of the file.
 * @param buffer_size The maximum number of bytes that can be written to the destination buffer.
 * @param buffer_end If non-NULL, on return this location points to the trailing nul in the destination buffer.
 * @param buffer_need If non-NULL, on return this location is updated with the number of bytes required to store the path string.
 * @param file The open file to query.
 * @return Zero if the path was retrieved successfully, or -1 if an error occurred.
 */
PIL_API(int)
PathForFile
(
    char_native_t      *buffer, 
    size_t         buffer_size,
    char_native_t **buffer_end, 
    size_t        *buffer_need, 
    struct FILE_HANDLE   *file
);

/* @summary Append one path fragment to another.
 * @param buffer The buffer containing the existing nul-terminated path fragment, to which the new fragment will be appended.
 * @param buffer_size The maximum number of bytes that can be written to the destination buffer.
 * @param buffer_end If non-NULL, on return this location points to the trailing nul in the destination buffer.
 * @param buffer_need If non-NULL, on return this location is updated with the number of bytes required to store the path string with the appended fragment.
 * @param append The nul-terminated path fragment to append to the existing path fragment in buffer.
 * @return Zero if the path fragment was successfully appended, or -1 if an error occurred.
 */
PIL_API(int)
PathAppend
(
    char_native_t       *buffer, 
    size_t          buffer_size,
    char_native_t  **buffer_end, 
    size_t         *buffer_need, 
    char_native_t const *append
);

/* @summary Change the file extension of a path.
 * @param buffer The buffer containing the existing nul-terminated path including filename whose extension will be changed.
 * @param buffer_size The maximum number of bytes that can be written to the destination buffer.
 * @param buffer_end If non-NULL, on return this location points to the trailing nul in the destination buffer.
 * @param buffer_need If non-NULL, on return this location is updated with the number of bytes required to store the path string with the updated extension.
 * @param new_extension The nul-terminated file extension to apply to the existing filename in buffer.
 * @return Zero if the file extension was successfully changed, or -1 if an error occurred.
 */
PIL_API(int)
PathChangeExtension
(
    char_native_t              *buffer, 
    size_t                 buffer_size,
    char_native_t         **buffer_end, 
    size_t                *buffer_need, 
    char_native_t const *new_extension
);

/* @summary Append a file extension to a path.
 * @param buffer The buffer containing the existing nul-terminated path including filename to which the file extension will be appended.
 * @param buffer_size The maximum number of bytes that can be written to the destination buffer.
 * @param buffer_end If non-NULL, on return this location points to the trailing nul in the destination buffer.
 * @param buffer_need If non-NULL, on return this location is updated with the number of bytes required to store the path string with the appended extension.
 * @param extension The nul-terminated file extension to append to the existing filename in buffer.
 * @return Zero if the file extension was successfully appended, or -1 if an error occurred.
 */
PIL_API(int)
PathAppendExtension
(
    char_native_t          *buffer, 
    size_t             buffer_size,
    char_native_t     **buffer_end, 
    size_t            *buffer_need, 
    char_native_t const *extension
);

/* @summary Ensures that each directory in a given path string exists. If the directory does not exist, it is created.
 * @param path A nul-terminated native path string specifying the directory tree.
 * @return Zero if the directory tree exists or was created, or -1 if an error occurred.
 */
PIL_API(int)
DirectoryCreate
(
    char_native_t *path
);

/* @summary Initialize a filesystem enumerator using the given configuration.
 * The root directory is locked and cannot be deleted until the enumerator is deleted.
 * @param fsenum The FILE_ENUMERATOR to initialize.
 * @param init Data used to configure the filesystem enumerator.
 * @return Zero if the filesystem enumerator is successfully initialized, or -1 if an error occurred.
 */
PIL_API(int)
FileEnumeratorCreate
(
    struct FILE_ENUMERATOR    *fsenum, 
    struct FILE_ENUMERATOR_INIT *init
);

/* @summary Free resources and unlock the root directory associated with a filesystem enumerator.
 * @param fsenum The filesystem enumerator to delete.
 */
PIL_API(void)
FileEnumeratorDelete
(
    struct FILE_ENUMERATOR *fsenum
);

/* @summary Execute a filesystem enumeration operation. User callbacks are invoked for each file and directory encountered.
 * @param fsenum The filesystem enumerator to execute.
 * @return Zero if enumeration completed successfully, or -1 if an error occurred.
 */
PIL_API(int)
FileEnumeratorExecute
(
    struct FILE_ENUMERATOR *fsenum
);

/* @summary Determine whether a given file handle is valid.
 * @param file The file handle to query.
 * @return Non-zero if the specified file handle is valid, or zero if the file handle is invalid.
 */
PIL_API(int)
FileHandleIsValid
(
    struct FILE_HANDLE *file
);

/* @summary Determine whether a filesystem entry is a file.
 * @param ent_info Information about a filesystem entry.
 * @return Non-zero of the specified information is associated with a file, or zero if the information is associated with some other type of entity.
 */
PIL_API(int)
IsFile
(
    struct FILE_INFO *ent_info
);

/* @summary Determine whether a filesystem entry is a directory.
 * @param ent_info Information about a filesystem entry.
 * @return Non-zero of the specified information is associated with a directory, or zero if the information is associated with some other type of entity.
 */
PIL_API(int)
IsDirectory
(
    struct FILE_INFO *ent_info
);

/* @summary Given a path to a filesystem entry, retrieve basic information about the file or directory.
 * @param result On return, the result structure is populated with information about the file system entity.
 * @param info Information about the file or directory to query. The Callback field is ignored.
 * @return Zero if the file system data was successfully retrieved, or non-zero if the operation failed.
 */
PIL_API(int)
FileStatPath
(
    struct IO_STAT_RESULT *result, 
    struct IO_STAT_DATA     *info
);

/* @summary Given a valid file handle, retrieve up-to-date basic information such as the file size.
 * @param ent_info On return, this structure is updated with information about the file.
 * @param file The file handle to query.
 * @return Zero if the file system data was successfully retrieved, or non-zero if the operation failed.
 */
PIL_API(int)
FileStatHandle
(
    struct FILE_INFO *ent_info,
    struct FILE_HANDLE   *file
);

/* @summary Open a file. The FILE_OPEN_HINT_PREALLOCATE flag is ignored - to pre-allocate a file, use FileCreate.
 * @param result On return, this structure is updated with information about the result of the operation.
 * @param data Information about the file to open. The Callback field is ignored.
 * @return Zero if the file was successfully opened, or non-zero if the operation failed.
 */
PIL_API(int)
FileOpen
(
    struct IO_OPEN_RESULT *result, 
    struct IO_OPEN_DATA     *data
);

/* @summary Create a file, optionally pre-allocating storage space for efficient sequential writing.
 * If the file exists, its current contents are destroyed. The file is opened with write access.
 * @param result On return, this structure is updated with information about the result of the operation.
 * @param data Information about the file to create. The Callback field is ignored.
 * @return Zero if the file was successfully created, or non-zero if the operation failed.
 */
PIL_API(int)
FileCreate
(
    struct IO_CREATE_RESULT *result, 
    struct IO_CREATE_DATA     *data
);

/* @summary Synchronously read data from a file into a host memory buffer.
 * @param result On return, this structure is updated with information about the result of the operation.
 * @param data Information about the operation to perform. The Callback field is ignored.
 * @return Zero if the read operation completed successfully, or non-zero if the operation failed.
 */
PIL_API(int)
FileRead
(
    struct IO_READ_RESULT *result, 
    struct IO_READ_DATA     *data
);

/* @summary Synchronously write data from a host memory buffer to a file.
 * @param result On return, this structure is updated with information about the result of the operation.
 * @param data Information about the operation to perform. The Callback field is ignored.
 * @return Zero if the write operation completed successfully, or non-zero if the operation failed.
 */
PIL_API(int)
FileWrite
(
    struct IO_WRITE_RESULT *result, 
    struct IO_WRITE_DATA     *data
);

/* @summary Flush any buffered writes for a given file to disk.
 * @param result On return, this structure is updated with information about the result of the operation.
 * @param data Information about the operation to perform. The Callback field is ignored.
 * @return Zero if the flush operation completed successfully, or non-zero if the operation failed.
 */
PIL_API(int)
FileFlush
(
    struct IO_FLUSH_RESULT *result, 
    struct IO_FLUSH_DATA     *data
);

/* @summary Close a file opened with FileOpen or FileCreate.
 * @param result On return, this structure is updated with information about the result of the operation.
 * @param data Information about the operation to perform. The Callback field is ignored.
 * @return Zero if the close operation completed successfully, or non-zero if the operation failed.
 */
PIL_API(int)
FileClose
(
    struct IO_CLOSE_RESULT *result, 
    struct IO_CLOSE_DATA     *data
);

#if 0
/* @summary Define the task entry point for a FileStatPath operation.
 * The filesystem operation executes asynchronously on the task worker pool. The worker thread is blocked for the duration of the operation.
 * The task data should point to an instance of IO_STAT_DATA. Results are returned through the callback on the thread executing the task.
 * @param args A TASK_ARGS instance specifying data associated with the task and data used to spawn additional tasks.
 */
PIL_API(void)
FileStatPath_TaskMain
(
    struct TASK_ARGS *args
);

/* @summary Define the task entry point for a FileOpen operation.
 * The file is opened for asynchronous access. The synchronous I/O functions cannot be used.
 * The filesystem operation executes asynchronously on the task worker pool. The worker thread is blocked for the duration of the operation.
 * The task data should point to an instance of IO_OPEN_DATA. Results are returned through the callback on the thread executing the task.
 * The returned file handle supports asynchronous read and write operations executed on the task pool.
 * @param args A TASK_ARGS instance specifying data associated with the task and data used to spawn additional tasks.
 */
PIL_API(void)
FileOpen_TaskMain
(
    struct TASK_ARGS *args
);

/* @summary Define the task entry point for a FileCreate operation.
 * The file is opened for asynchronous access. The synchronous I/O functions cannot be used.
 * The filesystem operation executes asynchronously on the task worker pool. The worker thread is blocked for the duration of the operation.
 * The task data should point to an instance of IO_CREATE_DATA. Results are returned through the callback on the thread executing the task.
 * @param args A TASK_ARGS instance specifying data associated with the task and data used to spawn additional tasks.
 */
PIL_API(void)
FileCreate_TaskMain
(
    struct TASK_ARGS *args
);

/* @summary Define the task entry point for a FileRead operation.
 * The filesystem operation executes asynchronously on the task worker pool. 
 * The I/O operation may execute synchronously, in which case the worker thread is blocked for the duration of the operation.
 * If the I/O operation can be executed truly asynchronously, the worker thread submits the operation and returns. The task does not complete until some later point in time.
 * The task data should point to an instance of IO_READ_DATA. Results are returned through the callback on the thread completing the task.
 * @param args A TASK_ARGS instance specifying data associated with the task and data used to spawn additional tasks.
 */
PIL_API(void)
FileRead_TaskMain
(
    struct TASK_ARGS *args
);

/* @summary Define the task entry point for a FileWrite operation.
 * The filesystem operation executes asynchronously on the task worker pool. 
 * The I/O operation may execute synchronously, in which case the worker thread is blocked for the duration of the operation.
 * If the I/O operation can be executed truly asynchronously, the worker thread submits the operation and returns. The task does not complete until some later point in time.
 * The task data should point to an instance of IO_WRITE_DATA. Results are returned through the callback on the thread completing the task.
 * @param args A TASK_ARGS instance specifying data associated with the task and data used to spawn additional tasks.
 */
PIL_API(void)
FileWrite_TaskMain
(
    struct TASK_ARGS *args
);

/* @summary Define the task entry point for a FileFlush operation.
 * The filesystem operation executes asynchronously on the task worker pool. The worker thread is blocked for the duration of the operation.
 * The task data should point to an instance of IO_FLUSH_DATA. Results are returned through the callback on the thread executing the task.
 * @param args A TASK_ARGS instance specifying data associated with the task and data used to spawn additional tasks.
 */
PIL_API(void)
FileFlush_TaskMain
(
    struct TASK_ARGS *args
);

/* @summary Define the task entry point for a FileClose operation.
 * The filesystem operation executes asynchronously on the task worker pool. The worker thread is blocked for the duration of the operation.
 * The task data should point to an instance of IO_CLOSE_DATA. Results are returned through the callback on the thread executing the task.
 * @param args A TASK_ARGS instance specifying data associated with the task and data used to spawn additional tasks.
 */
PIL_API(void)
FileClose_TaskMain
(
    struct TASK_ARGS *args
);
#endif /* DISABLED */

#ifdef __cplusplus
}; /* extern "C" */
#endif

#endif /* __PIL_FILEIO_H__ */

#if   PIL_TARGET_PLATFORM == PIL_PLATFORM_WIN32
#   include "win32/fileio_win32.h"
#elif PIL_TARGET_PLATFORM == PIL_PLATFORM_LINUX
#   include "linux/fileio_linux.h"
#else
#   error No Platform Interface Layer file I/O module for your platform!
#endif

