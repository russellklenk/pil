/**
 * @summary memio.h: Define functions for byte swapping and reading and writing 
 * values to and from memory buffers.
 */
#ifndef __PIL_MEMIO_H__
#define __PIL_MEMIO_H__

#pragma once

#ifndef PIL_NO_INCLUDES
#   ifndef __PIL_H__
#       include "pil.h"
#   endif
#endif

/* @summary Swap the bytes in a two-byte value.
 * @param _v The value to byte swap.
 * @return The byte swapped value.
 */
#ifndef ByteSwap2
#define ByteSwap2(_v)                                                          \
    ( (((_v) >> 8) & 0x00FF) |                                                 \
      (((_v) << 8) & 0xFF00) )
#endif

/* @summary Swap the bytes in a four-byte value.
 * @param _v The value to byte swap.
 * @return The byte swapped value.
 */
#ifndef ByteSwap4
#define ByteSwap4(_v)                                                          \
    ( (((_v) >> 24) & 0x000000FF) |                                            \
      (((_v) >>  8) & 0x0000FF00) |                                            \
      (((_v) <<  8) & 0x00FF0000) |                                            \
      (((_v) << 24) & 0xFF000000) )
#endif

/* @summary Swap the bytes in an eight-byte value.
 * @param _v The value to byte swap.
 * @return The byte swapped value.
 */
#ifndef ByteSwap8
#define ByteSwap8(_v)                                                          \
    ( (((_v) >> 56) & 0x00000000000000FFULL) |                                 \
      (((_v) >> 40) & 0x000000000000FF00ULL) |                                 \
      (((_v) >> 24) & 0x0000000000FF0000ULL) |                                 \
      (((_v) >>  8) & 0x00000000FF000000ULL) |                                 \
      (((_v) <<  8) & 0x000000FF00000000ULL) |                                 \
      (((_v) << 24) & 0x0000FF0000000000ULL) |                                 \
      (((_v) << 40) & 0x00FF000000000000ULL) |                                 \
      (((_v) << 56) & 0xFF00000000000000ULL) )
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* @summary Determine the endianess of the host CPU.
 * @return One of PIL_ENDIANESS_LSB_FIRST or PIL_ENDIANESS_MSB_FIRST.
 */
PIL_API(int)
EndianessQuery
(
    void
);

/* @summary Read a signed 8-bit integer value from a memory location.
 * @param addr A pointer to the buffer to read from.
 * @param offset The byte offset in the buffer at which the value is located.
 * @return The signed 8-bit integer value at the specified location.
 */
PIL_API(int8_t)
Read_si8
(
    void       *addr, 
    ptrdiff_t offset
);

/* @summary Read an unsigned 8-bit integer value from a memory location.
 * @param addr A pointer to the buffer to read from.
 * @param offset The byte offset in the buffer at which the value is located.
 * @return The unsigned 8-bit integer value at the specified location.
 */
PIL_API(uint8_t)
Read_ui8
(
    void       *addr, 
    ptrdiff_t offset
);

/* @summary Read a signed 16-bit integer value from a memory location.
 * @param addr A pointer to the buffer to read from.
 * @param offset The byte offset in the buffer at which the value is located.
 * @return The signed 16-bit integer value at the specified location.
 */
PIL_API(int16_t)
Read_si16
(
    void       *addr, 
    ptrdiff_t offset
);

/* @summary Read a signed 16-bit integer value stored in big endian format (MSB first) from a memory location.
 * @param addr A pointer to the buffer to read from.
 * @param offset The byte offset in the buffer at which the value is located.
 * @return The signed 16-bit integer value at the specified location.
 */
PIL_API(int16_t)
Read_si16_msb
(
    void       *addr, 
    ptrdiff_t offset
);

/* @summary Read a signed 16-bit integer value stored in little endian format (LSB first) from a memory location.
 * @param addr A pointer to the buffer to read from.
 * @param offset The byte offset in the buffer at which the value is located.
 * @return The signed 16-bit integer value at the specified location.
 */
PIL_API(int16_t)
Read_si16_lsb
(
    void       *addr, 
    ptrdiff_t offset
);

/* @summary Read an unsigned 16-bit integer value from a memory location.
 * @param addr A pointer to the buffer to read from.
 * @param offset The byte offset in the buffer at which the value is located.
 * @return The unsigned 16-bit integer value at the specified location.
 */
PIL_API(uint16_t)
Read_ui16
(
    void       *addr, 
    ptrdiff_t offset
);

/* @summary Read an unsigned 16-bit integer value stored in big endian format (MSB first) from a memory location.
 * @param addr A pointer to the buffer to read from.
 * @param offset The byte offset in the buffer at which the value is located.
 * @return The unsigned 16-bit integer value at the specified location.
 */
PIL_API(uint16_t)
Read_ui16_msb
(
    void       *addr, 
    ptrdiff_t offset
);

/* @summary Read an unsigned 16-bit integer value stored in little endian format (LSB first) from a memory location.
 * @param addr A pointer to the buffer to read from.
 * @param offset The byte offset in the buffer at which the value is located.
 * @return The unsigned 16-bit integer value at the specified location.
 */
PIL_API(uint16_t)
Read_ui16_lsb
(
    void       *addr, 
    ptrdiff_t offset
);

/* @summary Read a signed 32-bit integer value from a memory location.
 * @param addr A pointer to the buffer to read from.
 * @param offset The byte offset in the buffer at which the value is located.
 * @return The signed 32-bit integer value at the specified location.
 */
PIL_API(int32_t)
Read_si32
(
    void       *addr, 
    ptrdiff_t offset
);

/* @summary Read a signed 32-bit integer value stored in big endian format (MSB first) from a memory location.
 * @param addr A pointer to the buffer to read from.
 * @param offset The byte offset in the buffer at which the value is located.
 * @return The signed 32-bit integer value at the specified location.
 */
PIL_API(int32_t)
Read_si32_msb
(
    void       *addr, 
    ptrdiff_t offset
);

/* @summary Read a signed 32-bit integer value stored in little endian format (LSB first) from a memory location.
 * @param addr A pointer to the buffer to read from.
 * @param offset The byte offset in the buffer at which the value is located.
 * @return The signed 32-bit integer value at the specified location.
 */
PIL_API(int32_t)
Read_si32_lsb
(
    void       *addr, 
    ptrdiff_t offset
);

/* @summary Read an unsigned 32-bit integer value from a memory location.
 * @param addr A pointer to the buffer to read from.
 * @param offset The byte offset in the buffer at which the value is located.
 * @return The unsigned 32-bit integer value at the specified location.
 */
PIL_API(uint32_t)
Read_ui32
(
    void       *addr, 
    ptrdiff_t offset
);

/* @summary Read an unsigned 32-bit integer value stored in big endian format (MSB first) from a memory location.
 * @param addr A pointer to the buffer to read from.
 * @param offset The byte offset in the buffer at which the value is located.
 * @return The unsigned 32-bit integer value at the specified location.
 */
PIL_API(uint32_t)
Read_ui32_msb
(
    void       *addr, 
    ptrdiff_t offset
);

/* @summary Read an unsigned 32-bit integer value stored in little endian format (LSB first) from a memory location.
 * @param addr A pointer to the buffer to read from.
 * @param offset The byte offset in the buffer at which the value is located.
 * @return The unsigned 32-bit integer value at the specified location.
 */
PIL_API(uint32_t)
Read_ui32_lsb
(
    void       *addr, 
    ptrdiff_t offset
);

/* @summary Read a signed 64-bit integer value from a memory location.
 * @param addr A pointer to the buffer to read from.
 * @param offset The byte offset in the buffer at which the value is located.
 * @return The signed 64-bit integer value at the specified location.
 */
PIL_API(int64_t)
Read_si64
(
    void       *addr, 
    ptrdiff_t offset
);

/* @summary Read a signed 64-bit integer value stored in big endian format (MSB first) from a memory location.
 * @param addr A pointer to the buffer to read from.
 * @param offset The byte offset in the buffer at which the value is located.
 * @return The signed 64-bit integer value at the specified location.
 */
PIL_API(int64_t)
Read_si64_msb
(
    void       *addr, 
    ptrdiff_t offset
);

/* @summary Read a signed 64-bit integer value stored in little endian format (LSB first) from a memory location.
 * @param addr A pointer to the buffer to read from.
 * @param offset The byte offset in the buffer at which the value is located.
 * @return The signed 64-bit integer value at the specified location.
 */
PIL_API(int64_t)
Read_si64_lsb
(
    void       *addr, 
    ptrdiff_t offset
);

/* @summary Read an unsigned 64-bit integer value from a memory location.
 * @param addr A pointer to the buffer to read from.
 * @param offset The byte offset in the buffer at which the value is located.
 * @return The unsigned 64-bit integer value at the specified location.
 */
PIL_API(uint64_t)
Read_ui64
(
    void       *addr, 
    ptrdiff_t offset
);

/* @summary Read an unsigned 64-bit integer value stored in big endian format (MSB first) from a memory location.
 * @param addr A pointer to the buffer to read from.
 * @param offset The byte offset in the buffer at which the value is located.
 * @return The unsigned 64-bit integer value at the specified location.
 */
PIL_API(uint64_t)
Read_ui64_msb
(
    void       *addr, 
    ptrdiff_t offset
);

/* @summary Read an unsigned 64-bit integer value stored in little endian format (LSB first) from a memory location.
 * @param addr A pointer to the buffer to read from.
 * @param offset The byte offset in the buffer at which the value is located.
 * @return The unsigned 64-bit integer value at the specified location.
 */
PIL_API(uint64_t)
Read_ui64_lsb
(
    void       *addr, 
    ptrdiff_t offset
);

/* @summary Read a 32-bit floating-point value from a memory location.
 * @param addr A pointer to the buffer to read from.
 * @param offset The byte offset in the buffer at which the value is located.
 * @return The 32-bit floating-point value at the specified location.
 */
PIL_API(float)
Read_f32
(
    void       *addr, 
    ptrdiff_t offset
);

/* @summary Read a 32-bit floating-point value stored in big endian format (MSB first) from a memory location.
 * @param addr A pointer to the buffer to read from.
 * @param offset The byte offset in the buffer at which the value is located.
 * @return The 32-bit floating-point value at the specified location.
 */
PIL_API(float)
Read_f32_msb
(
    void       *addr, 
    ptrdiff_t offset
);

/* @summary Read a 32-bit floating-point value stored in little endian format (LSB first) from a memory location.
 * @param addr A pointer to the buffer to read from.
 * @param offset The byte offset in the buffer at which the value is located.
 * @return The 32-bit floating-point value at the specified location.
 */
PIL_API(float)
Read_f32_lsb
(
    void       *addr, 
    ptrdiff_t offset
);

/* @summary Read a 64-bit floating-point value from a memory location.
 * @param addr A pointer to the buffer to read from.
 * @param offset The byte offset in the buffer at which the value is located.
 * @return The 64-bit floating-point value at the specified location.
 */
PIL_API(double)
Read_f64
(
    void       *addr, 
    ptrdiff_t offset
);

/* @summary Read a 64-bit floating-point value stored in big endian format (MSB first) from a memory location.
 * @param addr A pointer to the buffer to read from.
 * @param offset The byte offset in the buffer at which the value is located.
 * @return The 64-bit floating-point value at the specified location.
 */
PIL_API(double)
Read_f64_msb
(
    void       *addr, 
    ptrdiff_t offset
);

/* @summary Read a 64-bit floating-point value stored in little endian format (LSB first) from a memory location.
 * @param addr A pointer to the buffer to read from.
 * @param offset The byte offset in the buffer at which the value is located.
 * @return The 64-bit floating-point value at the specified location.
 */
PIL_API(double)
Read_f64_lsb
(
    void       *addr, 
    ptrdiff_t offset
);

/* @summary Write a signed 8-bit integer value to a memory location.
 * @param addr A pointer to the buffer to write to.
 * @param value The value to write.
 * @param offset The byte offset in the buffer at which the value will be written.
 * @return The number of bytes written.
 */
PIL_API(size_t)
Write_si8
(
    void       *addr, 
    int8_t     value,
    ptrdiff_t offset
);

/* @summary Write an unsigned 8-bit integer value to a memory location.
 * @param addr A pointer to the buffer to write to.
 * @param value The value to write.
 * @param offset The byte offset in the buffer at which the value will be written.
 * @return The number of bytes written.
 */
PIL_API(size_t)
Write_ui8
(
    void       *addr, 
    uint8_t    value,
    ptrdiff_t offset
);

/* @summary Write a signed 16-bit integer value to a memory location.
 * @param addr A pointer to the buffer to write to.
 * @param value The value to write.
 * @param offset The byte offset in the buffer at which the value will be written.
 * @return The number of bytes written.
 */
PIL_API(size_t)
Write_si16
(
    void       *addr, 
    int16_t    value,
    ptrdiff_t offset
);

/* @summary Write a signed 16-bit integer value to a memory location. The value is written in big-endian (MSB first) format.
 * @param addr A pointer to the buffer to write to.
 * @param value The value to write.
 * @param offset The byte offset in the buffer at which the value will be written.
 * @return The number of bytes written.
 */
PIL_API(size_t)
Write_si16_msb
(
    void       *addr, 
    int16_t    value,
    ptrdiff_t offset
);

/* @summary Write a signed 16-bit integer value to a memory location. The value is written in little-endian (LSB first) format.
 * @param addr A pointer to the buffer to write to.
 * @param value The value to write.
 * @param offset The byte offset in the buffer at which the value will be written.
 * @return The number of bytes written.
 */
PIL_API(size_t)
Write_si16_lsb
(
    void       *addr, 
    int16_t    value,
    ptrdiff_t offset
);

/* @summary Write an unsigned 16-bit integer value to a memory location.
 * @param addr A pointer to the buffer to write to.
 * @param value The value to write.
 * @param offset The byte offset in the buffer at which the value will be written.
 * @return The number of bytes written.
 */
PIL_API(size_t)
Write_ui16
(
    void       *addr, 
    uint16_t   value,
    ptrdiff_t offset
);

/* @summary Write an unsigned 16-bit integer value to a memory location. The value is written in big-endian (MSB first) format.
 * @param addr A pointer to the buffer to write to.
 * @param value The value to write.
 * @param offset The byte offset in the buffer at which the value will be written.
 * @return The number of bytes written.
 */
PIL_API(size_t)
Write_ui16_msb
(
    void       *addr, 
    uint16_t   value,
    ptrdiff_t offset
);

/* @summary Write an unsigned 16-bit integer value to a memory location. The value is written in little-endian (LSB first) format.
 * @param addr A pointer to the buffer to write to.
 * @param value The value to write.
 * @param offset The byte offset in the buffer at which the value will be written.
 * @return The number of bytes written.
 */
PIL_API(size_t)
Write_ui16_lsb
(
    void       *addr, 
    uint16_t   value,
    ptrdiff_t offset
);

/* @summary Write a signed 32-bit integer value to a memory location.
 * @param addr A pointer to the buffer to write to.
 * @param value The value to write.
 * @param offset The byte offset in the buffer at which the value will be written.
 * @return The number of bytes written.
 */
PIL_API(size_t)
Write_si32
(
    void       *addr, 
    int32_t    value,
    ptrdiff_t offset
);

/* @summary Write a signed 32-bit integer value to a memory location. The value is written in big-endian (MSB first) format.
 * @param addr A pointer to the buffer to write to.
 * @param value The value to write.
 * @param offset The byte offset in the buffer at which the value will be written.
 * @return The number of bytes written.
 */
PIL_API(size_t)
Write_si32_msb
(
    void       *addr, 
    int32_t    value,
    ptrdiff_t offset
);

/* @summary Write a signed 32-bit integer value to a memory location. The value is written in little-endian (LSB first) format.
 * @param addr A pointer to the buffer to write to.
 * @param value The value to write.
 * @param offset The byte offset in the buffer at which the value will be written.
 * @return The number of bytes written.
 */
PIL_API(size_t)
Write_si32_lsb
(
    void       *addr, 
    int32_t    value,
    ptrdiff_t offset
);

/* @summary Write an unsigned 32-bit integer value to a memory location.
 * @param addr A pointer to the buffer to write to.
 * @param value The value to write.
 * @param offset The byte offset in the buffer at which the value will be written.
 * @return The number of bytes written.
 */
PIL_API(size_t)
Write_ui32
(
    void       *addr, 
    uint32_t   value,
    ptrdiff_t offset
);

/* @summary Write an unsigned 32-bit integer value to a memory location. The value is written in big-endian (MSB first) format.
 * @param addr A pointer to the buffer to write to.
 * @param value The value to write.
 * @param offset The byte offset in the buffer at which the value will be written.
 * @return The number of bytes written.
 */
PIL_API(size_t)
Write_ui32_msb
(
    void       *addr, 
    uint32_t   value,
    ptrdiff_t offset
);

/* @summary Write an unsigned 32-bit integer value to a memory location. The value is written in little-endian (LSB first) format.
 * @param addr A pointer to the buffer to write to.
 * @param value The value to write.
 * @param offset The byte offset in the buffer at which the value will be written.
 * @return The number of bytes written.
 */
PIL_API(size_t)
Write_ui32_lsb
(
    void       *addr, 
    uint32_t   value,
    ptrdiff_t offset
);

/* @summary Write a signed 64-bit integer value to a memory location.
 * @param addr A pointer to the buffer to write to.
 * @param value The value to write.
 * @param offset The byte offset in the buffer at which the value will be written.
 * @return The number of bytes written.
 */
PIL_API(size_t)
Write_si64
(
    void       *addr, 
    int64_t    value,
    ptrdiff_t offset
);

/* @summary Write a signed 64-bit integer value to a memory location. The value is written in big-endian (MSB first) format.
 * @param addr A pointer to the buffer to write to.
 * @param value The value to write.
 * @param offset The byte offset in the buffer at which the value will be written.
 * @return The number of bytes written.
 */
PIL_API(size_t)
Write_si64_msb
(
    void       *addr, 
    int64_t    value,
    ptrdiff_t offset
);

/* @summary Write a signed 64-bit integer value to a memory location. The value is written in little-endian (LSB first) format.
 * @param addr A pointer to the buffer to write to.
 * @param value The value to write.
 * @param offset The byte offset in the buffer at which the value will be written.
 * @return The number of bytes written.
 */
PIL_API(size_t)
Write_si64_lsb
(
    void       *addr, 
    int64_t    value,
    ptrdiff_t offset
);

/* @summary Write an unsigned 64-bit integer value to a memory location.
 * @param addr A pointer to the buffer to write to.
 * @param value The value to write.
 * @param offset The byte offset in the buffer at which the value will be written.
 * @return The number of bytes written.
 */
PIL_API(size_t)
Write_ui64
(
    void       *addr, 
    uint64_t   value,
    ptrdiff_t offset
);

/* @summary Write an unsigned 64-bit integer value to a memory location. The value is written in big-endian (MSB first) format.
 * @param addr A pointer to the buffer to write to.
 * @param value The value to write.
 * @param offset The byte offset in the buffer at which the value will be written.
 * @return The number of bytes written.
 */
PIL_API(size_t)
Write_ui64_msb
(
    void       *addr, 
    uint64_t   value,
    ptrdiff_t offset
);

/* @summary Write an unsigned 64-bit integer value to a memory location. The value is written in little-endian (LSB first) format.
 * @param addr A pointer to the buffer to write to.
 * @param value The value to write.
 * @param offset The byte offset in the buffer at which the value will be written.
 * @return The number of bytes written.
 */
PIL_API(size_t)
Write_ui64_lsb
(
    void       *addr, 
    uint64_t   value,
    ptrdiff_t offset
);

/* @summary Write a 32-bit floating-point value to a memory location.
 * @param addr A pointer to the buffer to write to.
 * @param value The value to write.
 * @param offset The byte offset in the buffer at which the value will be written.
 * @return The number of bytes written.
 */
PIL_API(size_t)
Write_f32
(
    void       *addr, 
    float      value,
    ptrdiff_t offset
);

/* @summary Write a 32-bit floating-point value to a memory location. The value is written in big-endian (MSB first) format.
 * @param addr A pointer to the buffer to write to.
 * @param value The value to write.
 * @param offset The byte offset in the buffer at which the value will be written.
 * @return The number of bytes written.
 */
PIL_API(size_t)
Write_f32_msb
(
    void       *addr, 
    float      value,
    ptrdiff_t offset
);

/* @summary Write a 32-bit floating-point value to a memory location. The value is written in little-endian (LSB first) format.
 * @param addr A pointer to the buffer to write to.
 * @param value The value to write.
 * @param offset The byte offset in the buffer at which the value will be written.
 * @return The number of bytes written.
 */
PIL_API(size_t)
Write_f32_lsb
(
    void       *addr, 
    float      value,
    ptrdiff_t offset
);

/* @summary Write a 64-bit floating-point value to a memory location.
 * @param addr A pointer to the buffer to write to.
 * @param value The value to write.
 * @param offset The byte offset in the buffer at which the value will be written.
 * @return The number of bytes written.
 */
PIL_API(size_t)
Write_f64
(
    void       *addr, 
    double     value,
    ptrdiff_t offset
);

/* @summary Write a 64-bit floating-point value to a memory location. The value is written in big-endian (MSB first) format.
 * @param addr A pointer to the buffer to write to.
 * @param value The value to write.
 * @param offset The byte offset in the buffer at which the value will be written.
 * @return The number of bytes written.
 */
PIL_API(size_t)
Write_f64_msb
(
    void       *addr, 
    double     value,
    ptrdiff_t offset
);

/* @summary Write a 64-bit floating-point value to a memory location. The value is written in little-endian (LSB first) format.
 * @param addr A pointer to the buffer to write to.
 * @param value The value to write.
 * @param offset The byte offset in the buffer at which the value will be written.
 * @return The number of bytes written.
 */
PIL_API(size_t)
Write_f64_lsb
(
    void       *addr, 
    double     value,
    ptrdiff_t offset
);

#ifdef __cplusplus
}; /* extern "C" */
#endif

#endif /* __PIL_MEMIO_H__ */

