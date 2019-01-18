/**
 * @summary memio.cc: Implement the memory buffer I/O functions.
 */
#include "memio.h"

/* @summary Reads a 16-bit value from a buffer and performs a byte swap operation on it before returning it as the final destination type (16-bit unsigned.)
 * @param addr A pointer to the buffer from which the value will be read.
 * @param offset The offset into the buffer at which to read the value.
 * @return The byte-swapped value.
 */
static PIL_INLINE uint16_t
ReadSwap_i16
(
    void       *addr, 
    ptrdiff_t offset
)
{
    uint8_t   *src = ((uint8_t *)addr) + offset;
    return ByteSwap2(*(uint16_t*) src);
}

/* @summary Reads a 32-bit value from a buffer and performs a byte swap operation on it before returning it as the final destination type (32-bit unsigned.)
 * @param addr A pointer to the buffer from which the value will be read.
 * @param offset The offset into the buffer at which to read the value.
 * @return The byte-swapped value.
 */
static PIL_INLINE uint32_t
ReadSwap_i32
(
    void       *addr, 
    ptrdiff_t offset
)
{
    uint8_t   *src = ((uint8_t *)addr) + offset;
    return ByteSwap4(*(uint32_t*) src);
}

/* @summary Reads a 64-bit value from a buffer and performs a byte swap operation on it before returning it as the final destination type (64-bit unsigned.)
 * @param addr A pointer to the buffer from which the value will be read.
 * @param offset The offset into the buffer at which to read the value.
 * @return The byte-swapped value.
 */
static PIL_INLINE uint64_t
ReadSwap_i64
(
    void       *addr, 
    ptrdiff_t offset
)
{
    uint8_t   *src = ((uint8_t *)addr) + offset;
    return ByteSwap8(*(uint64_t*) src);
}

/* @summary Reads a 32-bit value from a buffer and performs a byte swap operation on it before returning it as the final destination type (32-bit floating point.)
 * @param addr A pointer to the buffer from which the value will be read.
 * @param offset The offset into the buffer at which to read the value.
 * @return The byte-swapped value.
 */
static PIL_INLINE float
ReadSwap_f32
(
    void       *addr, 
    ptrdiff_t offset
)
{
    typedef union {
        uint32_t  u32;
        float f32;
    } ui32_or_f32;

    ui32_or_f32   val;
    uint8_t      *src = ((uint8_t *)addr) + offset;
    val.u32 = ByteSwap4(*(uint32_t*) src);
    return val.f32;
}

/* @summary Reads a 64-bit value from a buffer and performs a byte swap operation on it before returning it as the final destination type (64-bit floating point.)
 * @param addr A pointer to the buffer from which the value will be read.
 * @param offset The offset into the buffer at which to read the value.
 * @return The byte-swapped value.
 */
static PIL_INLINE double
ReadSwap_f64
(
    void       *addr, 
    ptrdiff_t offset
)
{
    typedef union {
        uint64_t  u64;
        double    f64;
    } ui64_or_f64;

    ui64_or_f64   val;
    uint8_t      *src = ((uint8_t *)addr) + offset;
    val.u64 = ByteSwap8(*(uint64_t*) src);
    return val.f64;
}

/* @summary Perform a byte swapping operation on a signed 16-bit integer value and write it to a memory location.
 * @param addr A pointer to the buffer where the value will be written.
 * @param value The value to write to the buffer.
 * @param offset The offset into the buffer at which to write the value.
 */
static PIL_INLINE void
SwapWrite_si16
(
    void       *addr,
    int16_t    value,
    ptrdiff_t offset
)
{
    uint8_t * dst = ((uint8_t *) addr) + offset;
  *(uint16_t*)dst = ByteSwap2(*(uint16_t*) &value);
}

/* @summary Perform a byte swapping operation on an unsigned 16-bit integer value and write it to a memory location.
 * @param addr A pointer to the buffer where the value will be written.
 * @param value The value to write to the buffer.
 * @param offset The offset into the buffer at which to write the value.
 */
static PIL_INLINE void
SwapWrite_ui16
(
    void       *addr,
    uint16_t   value,
    ptrdiff_t offset
)
{
    uint8_t * dst = ((uint8_t *) addr) + offset;
  *(uint16_t*)dst = ByteSwap2(value);
}

/* @summary Perform a byte swapping operation on a signed 32-bit integer value and write it to a memory location.
 * @param addr A pointer to the buffer where the value will be written.
 * @param value The value to write to the buffer.
 * @param offset The offset into the buffer at which to write the value.
 */
static PIL_INLINE void
SwapWrite_si32
(
    void       *addr,
    int32_t    value,
    ptrdiff_t offset
)
{
    uint8_t * dst = ((uint8_t *) addr) + offset;
  *(uint32_t*)dst = ByteSwap4(*(uint32_t*) &value);
}

/* @summary Perform a byte swapping operation on an unsigned 32-bit integer value and write it to a memory location.
 * @param addr A pointer to the buffer where the value will be written.
 * @param value The value to write to the buffer.
 * @param offset The offset into the buffer at which to write the value.
 */
static PIL_INLINE void
SwapWrite_ui32
(
    void       *addr,
    uint32_t   value,
    ptrdiff_t offset
)
{
    uint8_t * dst = ((uint8_t *) addr) + offset;
  *(uint32_t*)dst = ByteSwap4(value);
}

/* @summary Perform a byte swapping operation on a signed 64-bit integer value and write it to a memory location.
 * @param addr A pointer to the buffer where the value will be written.
 * @param value The value to write to the buffer.
 * @param offset The offset into the buffer at which to write the value.
 */
static PIL_INLINE void
SwapWrite_si64
(
    void       *addr,
    int64_t    value,
    ptrdiff_t offset
)
{
    uint8_t * dst = ((uint8_t *) addr) + offset;
  *(uint64_t*)dst = ByteSwap8(*(uint64_t*) &value);
}

/* @summary Perform a byte swapping operation on an unsigned 64-bit integer value and write it to a memory location.
 * @param addr A pointer to the buffer where the value will be written.
 * @param value The value to write to the buffer.
 * @param offset The offset into the buffer at which to write the value.
 */
static PIL_INLINE void
SwapWrite_ui64
(
    void       *addr,
    uint64_t   value,
    ptrdiff_t offset
)
{
    uint8_t * dst = ((uint8_t *) addr) + offset;
  *(uint64_t*)dst = ByteSwap8(value);
}

/* @summary Perform a byte swapping operation on a 32-bit floating point value and write it to a memory location.
 * @param addr A pointer to the buffer where the value will be written.
 * @param value The value to write to the buffer.
 * @param offset The offset into the buffer at which to write the value.
 */
static PIL_INLINE void
SwapWrite_f32
(
    void       *addr,
    float      value,
    ptrdiff_t offset
)
{
    typedef union {
        uint32_t  u32;
        float     f32;
    } ui32_or_f32;

    ui32_or_f32 v;  v.f32 = value;
    uint8_t * dst = ((uint8_t *) addr) + offset;
  *(uint32_t*)dst = ByteSwap4(v.u32);
}

/* @summary Perform a byte swapping operation on a 64-bit floating point value and write it to a memory location.
 * @param addr A pointer to the buffer where the value will be written.
 * @param value The value to write to the buffer.
 * @param offset The offset into the buffer at which to write the value.
 */
static PIL_INLINE void
SwapWrite_f64
(
    void           *addr,
    double  value,
    ptrdiff_t offset
)
{
    typedef union {
        uint64_t  u64;
        double    f64;
    } ui64_or_f64;

    ui64_or_f64 v;  v.f64 = value;
    uint8_t * dst = ((uint8_t *) addr) + offset;
  *(uint64_t*)dst = ByteSwap8(v.u64);
}

PIL_API(int)
EndianessQuery
(
    void
)
{
    union EndianessTest_u
    { char array[4];
      int32_t chars;
    } u;
    char c = 'a';
    u.array[0] = c++;
    u.array[1] = c++;
    u.array[2] = c++;
    u.array[3] = c++;
    return (u.chars == 0x61626364) ? PIL_ENDIANESS_MSB_FIRST : PIL_ENDIANESS_LSB_FIRST;
}

PIL_API(int8_t)
Read_si8
(
    void       *addr, 
    ptrdiff_t offset
)
{
    return *(int8_t *)(((uint8_t *) addr)+offset);
}

PIL_API(uint8_t)
Read_ui8
(
    void       *addr, 
    ptrdiff_t offset
)
{
    return *(uint8_t *)(((uint8_t *) addr)+offset);
}

PIL_API(int16_t)
Read_si16
(
    void       *addr, 
    ptrdiff_t offset
)
{
    return *(int16_t*)(((uint8_t *) addr)+offset);
}

PIL_API(int16_t)
Read_si16_msb
(
    void       *addr, 
    ptrdiff_t offset
)
{
#if PIL_SYSTEM_ENDIANESS == PIL_ENDIANESS_MSB_FIRST
    return *(int16_t*)(((uint8_t *) addr)+offset);
#else
    return  (int16_t )ReadSwap_i16(addr, offset);
#endif
}

PIL_API(int16_t)
Read_si16_lsb
(
    void       *addr, 
    ptrdiff_t offset
)
{
#if PIL_SYSTEM_ENDIANESS == PIL_ENDIANESS_LSB_FIRST
    return *(int16_t*)(((uint8_t *) addr)+offset);
#else
    return  (int16_t )ReadSwap_i16(addr, offset);
#endif
}

PIL_API(uint16_t)
Read_ui16
(
    void       *addr, 
    ptrdiff_t offset
)
{
    return *(uint16_t*)(((uint8_t *) addr)+offset);
}

PIL_API(uint16_t)
Read_ui16_msb
(
    void       *addr, 
    ptrdiff_t offset
)
{
#if PIL_SYSTEM_ENDIANESS == PIL_ENDIANESS_MSB_FIRST
    return *(uint16_t*)(((uint8_t *) addr)+offset);
#else
    return  (uint16_t )ReadSwap_i16(addr, offset);
#endif
}

PIL_API(uint16_t)
Read_ui16_lsb
(
    void       *addr, 
    ptrdiff_t offset
)
{
#if PIL_SYSTEM_ENDIANESS == PIL_ENDIANESS_LSB_FIRST
    return *(uint16_t*)(((uint8_t *) addr)+offset);
#else
    return  (uint16_t )ReadSwap_i16(addr, offset);
#endif
}

PIL_API(int32_t)
Read_si32
(
    void       *addr, 
    ptrdiff_t offset
)
{
    return *(int32_t*)(((uint8_t *) addr)+offset);
}

PIL_API(int32_t)
Read_si32_msb
(
    void       *addr, 
    ptrdiff_t offset
)
{
#if PIL_SYSTEM_ENDIANESS == PIL_ENDIANESS_MSB_FIRST
    return *(int32_t*)(((uint8_t *) addr)+offset);
#else
    return  (int32_t )ReadSwap_i32(addr, offset);
#endif
}

PIL_API(int32_t)
Read_si32_lsb
(
    void       *addr, 
    ptrdiff_t offset
)
{
#if PIL_SYSTEM_ENDIANESS == PIL_ENDIANESS_LSB_FIRST
    return *(int32_t*)(((uint8_t *) addr)+offset);
#else
    return  (int32_t )ReadSwap_i32(addr, offset);
#endif
}

PIL_API(uint32_t)
Read_ui32
(
    void       *addr, 
    ptrdiff_t offset
)
{
    return *(uint32_t*)(((uint8_t *) addr)+offset);
}

PIL_API(uint32_t)
Read_ui32_msb
(
    void       *addr, 
    ptrdiff_t offset
)
{
#if PIL_SYSTEM_ENDIANESS == PIL_ENDIANESS_MSB_FIRST
    return *(uint32_t*)(((uint8_t *) addr)+offset);
#else
    return  (uint32_t )ReadSwap_i32(addr, offset);
#endif
}

PIL_API(uint32_t)
Read_ui32_lsb
(
    void       *addr, 
    ptrdiff_t offset
)
{
#if PIL_SYSTEM_ENDIANESS == PIL_ENDIANESS_LSB_FIRST
    return *(uint32_t*)(((uint8_t *) addr)+offset);
#else
    return  (uint32_t )ReadSwap_i32(addr, offset);
#endif
}

PIL_API(int64_t)
Read_si64
(
    void       *addr, 
    ptrdiff_t offset
)
{
    return *(int64_t*)(((uint8_t *) addr)+offset);
}

PIL_API(int64_t)
Read_si64_msb
(
    void       *addr, 
    ptrdiff_t offset
)
{
#if PIL_SYSTEM_ENDIANESS == PIL_ENDIANESS_MSB_FIRST
    return *(int64_t*)(((uint8_t *) addr)+offset);
#else
    return  (int64_t )ReadSwap_i64(addr, offset);
#endif
}

PIL_API(int64_t)
Read_si64_lsb
(
    void       *addr, 
    ptrdiff_t offset
)
{
#if PIL_SYSTEM_ENDIANESS == PIL_ENDIANESS_LSB_FIRST
    return *(int64_t*)(((uint8_t *) addr)+offset);
#else
    return  (int64_t )ReadSwap_i64(addr, offset);
#endif
}

PIL_API(uint64_t)
Read_ui64
(
    void       *addr, 
    ptrdiff_t offset
)
{
    return *(uint64_t*)(((uint8_t *) addr)+offset);
}

PIL_API(uint64_t)
Read_ui64_msb
(
    void       *addr, 
    ptrdiff_t offset
)
{
#if PIL_SYSTEM_ENDIANESS == PIL_ENDIANESS_MSB_FIRST
    return *(uint64_t*)(((uint8_t *) addr)+offset);
#else
    return  (uint64_t )ReadSwap_i64(addr, offset);
#endif
}

PIL_API(uint64_t)
Read_ui64_lsb
(
    void       *addr, 
    ptrdiff_t offset
)
{
#if PIL_SYSTEM_ENDIANESS == PIL_ENDIANESS_LSB_FIRST
    return *(uint64_t*)(((uint8_t *) addr)+offset);
#else
    return  (uint64_t )ReadSwap_i64(addr, offset);
#endif
}

PIL_API(float)
Read_f32
(
    void       *addr, 
    ptrdiff_t offset
)
{
    return *(float*)(((uint8_t *) addr)+offset);
}

PIL_API(float)
Read_f32_msb
(
    void       *addr, 
    ptrdiff_t offset
)
{
#if PIL_SYSTEM_ENDIANESS == PIL_ENDIANESS_MSB_FIRST
    return *(float*)(((uint8_t *) addr)+offset);
#else
    return  (float )ReadSwap_f32(addr, offset);
#endif
}

PIL_API(float)
Read_f32_lsb
(
    void       *addr, 
    ptrdiff_t offset
)
{
#if PIL_SYSTEM_ENDIANESS == PIL_ENDIANESS_LSB_FIRST
    return *(float*)(((uint8_t *) addr)+offset);
#else
    return  (float )ReadSwap_f32(addr, offset);
#endif
}

PIL_API(double)
Read_f64
(
    void       *addr, 
    ptrdiff_t offset
)
{
    return *(double*)(((uint8_t *) addr)+offset);
}

PIL_API(double)
Read_f64_msb
(
    void       *addr, 
    ptrdiff_t offset
)
{
#if PIL_SYSTEM_ENDIANESS == PIL_ENDIANESS_MSB_FIRST
    return *(double*)(((uint8_t *) addr)+offset);
#else
    return  (double )ReadSwap_f64(addr, offset);
#endif
}

PIL_API(double)
Read_f64_lsb
(
    void       *addr, 
    ptrdiff_t offset
)
{
#if PIL_SYSTEM_ENDIANESS == PIL_ENDIANESS_LSB_FIRST
    return *(double*)(((uint8_t *) addr)+offset);
#else
    return  (double )ReadSwap_f64(addr, offset);
#endif
}

PIL_API(size_t)
Write_si8
(
    void       *addr, 
    int8_t     value,
    ptrdiff_t offset
)
{
    *(int8_t *)(((uint8_t*) addr)+offset) = value;
    return sizeof(int8_t);
}

PIL_API(size_t)
Write_ui8
(
    void       *addr, 
    uint8_t    value,
    ptrdiff_t offset
)
{
    *(uint8_t *)(((uint8_t*) addr)+offset) = value;
    return sizeof(uint8_t);
}

PIL_API(size_t)
Write_si16
(
    void       *addr, 
    int16_t    value,
    ptrdiff_t offset
)
{
    *(int16_t*)(((uint8_t*) addr)+offset) = value;
    return sizeof(int16_t);
}

PIL_API(size_t)
Write_si16_msb
(
    void       *addr, 
    int16_t    value,
    ptrdiff_t offset
)
{
#if PIL_SYSTEM_ENDIANESS == PIL_ENDIANESS_MSB_FIRST
    *(int16_t*)(((uint8_t*) addr)+offset) = value;
#else
    SwapWrite_si16(addr, value, offset);
#endif
    return sizeof(int16_t);
}

PIL_API(size_t)
Write_si16_lsb
(
    void       *addr, 
    int16_t    value,
    ptrdiff_t offset
)
{
#if PIL_SYSTEM_ENDIANESS == PIL_ENDIANESS_LSB_FIRST
    *(int16_t*)(((uint8_t*) addr)+offset) = value;
#else
    SwapWrite_si16(addr, value, offset);
#endif
    return sizeof(int16_t);
}

PIL_API(size_t)
Write_ui16
(
    void       *addr, 
    uint16_t   value,
    ptrdiff_t offset
)
{
    *(uint16_t*)(((uint8_t*) addr)+offset) = value;
    return sizeof(uint16_t);
}

PIL_API(size_t)
Write_ui16_msb
(
    void       *addr, 
    uint16_t   value,
    ptrdiff_t offset
)
{
#if PIL_SYSTEM_ENDIANESS == PIL_ENDIANESS_MSB_FIRST
    *(uint16_t*)(((uint8_t*) addr)+offset) = value;
#else
    SwapWrite_ui16(addr, value, offset);
#endif
    return sizeof(uint16_t);
}

PIL_API(size_t)
Write_ui16_lsb
(
    void       *addr, 
    uint16_t   value,
    ptrdiff_t offset
)
{
#if PIL_SYSTEM_ENDIANESS == PIL_ENDIANESS_LSB_FIRST
    *(uint16_t*)(((uint8_t*) addr)+offset) = value;
#else
    SwapWrite_ui16(addr, value, offset);
#endif
    return sizeof(uint16_t);
}

PIL_API(size_t)
Write_si32
(
    void       *addr, 
    int32_t    value,
    ptrdiff_t offset
)
{
    *(int32_t*)(((uint8_t*) addr)+offset) = value;
    return sizeof(int32_t);
}

PIL_API(size_t)
Write_si32_msb
(
    void       *addr, 
    int32_t    value,
    ptrdiff_t offset
)
{
#if PIL_SYSTEM_ENDIANESS == PIL_ENDIANESS_MSB_FIRST
    *(int32_t*)(((uint8_t*) addr)+offset) = value;
#else
    SwapWrite_si32(addr, value, offset);
#endif
    return sizeof(int32_t);
}

PIL_API(size_t)
Write_si32_lsb
(
    void       *addr, 
    int32_t    value,
    ptrdiff_t offset
)
{
#if PIL_SYSTEM_ENDIANESS == PIL_ENDIANESS_LSB_FIRST
    *(int32_t*)(((uint8_t*) addr)+offset) = value;
#else
    SwapWrite_si32(addr, value, offset);
#endif
    return sizeof(int32_t);
}

PIL_API(size_t)
Write_ui32
(
    void       *addr, 
    uint32_t   value,
    ptrdiff_t offset
)
{
    *(uint32_t*)(((uint8_t*) addr)+offset) = value;
    return sizeof(uint32_t);
}

PIL_API(size_t)
Write_ui32_msb
(
    void       *addr, 
    uint32_t   value,
    ptrdiff_t offset
)
{
#if PIL_SYSTEM_ENDIANESS == PIL_ENDIANESS_MSB_FIRST
    *(uint32_t*)(((uint8_t*) addr)+offset) = value;
#else
    SwapWrite_ui32(addr, value, offset);
#endif
    return sizeof(uint32_t);
}

PIL_API(size_t)
Write_ui32_lsb
(
    void       *addr, 
    uint32_t   value,
    ptrdiff_t offset
)
{
#if PIL_SYSTEM_ENDIANESS == PIL_ENDIANESS_LSB_FIRST
    *(uint32_t*)(((uint8_t*) addr)+offset) = value;
#else
    SwapWrite_ui32(addr, value, offset);
#endif
    return sizeof(uint32_t);
}

PIL_API(size_t)
Write_si64
(
    void       *addr, 
    int64_t    value,
    ptrdiff_t offset
)
{
    *(int64_t*)(((uint8_t*) addr)+offset) = value;
    return sizeof(int64_t);
}

PIL_API(size_t)
Write_si64_msb
(
    void       *addr, 
    int64_t    value,
    ptrdiff_t offset
)
{
#if PIL_SYSTEM_ENDIANESS == PIL_ENDIANESS_MSB_FIRST
    *(int64_t*)(((uint8_t*) addr)+offset) = value;
#else
    SwapWrite_si64(addr, value, offset);
#endif
    return sizeof(int64_t);
}

PIL_API(size_t)
Write_si64_lsb
(
    void       *addr, 
    int64_t    value,
    ptrdiff_t offset
)
{
#if PIL_SYSTEM_ENDIANESS == PIL_ENDIANESS_LSB_FIRST
    *(int64_t*)(((uint8_t*) addr)+offset) = value;
#else
    SwapWrite_si64(addr, value, offset);
#endif
    return sizeof(int64_t);
}

PIL_API(size_t)
Write_ui64
(
    void       *addr, 
    uint64_t   value,
    ptrdiff_t offset
)
{
    *(uint64_t*)(((uint8_t*) addr)+offset) = value;
    return sizeof(uint64_t);
}

PIL_API(size_t)
Write_ui64_msb
(
    void       *addr, 
    uint64_t   value,
    ptrdiff_t offset
)
{
#if PIL_SYSTEM_ENDIANESS == PIL_ENDIANESS_MSB_FIRST
    *(uint64_t*)(((uint8_t*) addr)+offset) = value;
#else
    SwapWrite_ui64(addr, value, offset);
#endif
    return sizeof(uint64_t);
}

PIL_API(size_t)
Write_ui64_lsb
(
    void       *addr, 
    uint64_t   value,
    ptrdiff_t offset
)
{
#if PIL_SYSTEM_ENDIANESS == PIL_ENDIANESS_LSB_FIRST
    *(uint64_t*)(((uint8_t*) addr)+offset) = value;
#else
    SwapWrite_ui64(addr, value, offset);
#endif
    return sizeof(uint64_t);
}

PIL_API(size_t)
Write_f32
(
    void       *addr, 
    float      value,
    ptrdiff_t offset
)
{
    *(float*)(((uint8_t*) addr)+offset) = value;
    return sizeof(float);
}

PIL_API(size_t)
Write_f32_msb
(
    void       *addr, 
    float      value,
    ptrdiff_t offset
)
{
#if PIL_SYSTEM_ENDIANESS == PIL_ENDIANESS_MSB_FIRST
    *(float*)(((uint8_t*) addr)+offset) = value;
#else
    SwapWrite_f32(addr, value, offset);
#endif
    return sizeof(float);
}

PIL_API(size_t)
Write_f32_lsb
(
    void       *addr, 
    float      value,
    ptrdiff_t offset
)
{
#if PIL_SYSTEM_ENDIANESS == PIL_ENDIANESS_LSB_FIRST
    *(float*)(((uint8_t*) addr)+offset) = value;
#else
    SwapWrite_f32(addr, value, offset);
#endif
    return sizeof(float);
}

PIL_API(size_t)
Write_f64
(
    void       *addr, 
    double     value,
    ptrdiff_t offset
)
{
    *(double*)(((uint8_t*) addr)+offset) = value;
    return sizeof(double);
}

PIL_API(size_t)
Write_f64_msb
(
    void       *addr, 
    double     value,
    ptrdiff_t offset
)
{
#if PIL_SYSTEM_ENDIANESS == PIL_ENDIANESS_MSB_FIRST
    *(double*)(((uint8_t*) addr)+offset) = value;
#else
    SwapWrite_f64(addr, value, offset);
#endif
    return sizeof(double);
}

PIL_API(size_t)
Write_f64_lsb
(
    void       *addr, 
    double     value,
    ptrdiff_t offset
)
{
#if PIL_SYSTEM_ENDIANESS == PIL_ENDIANESS_LSB_FIRST
    *(double*)(((uint8_t*) addr)+offset) = value;
#else
    SwapWrite_f64(addr, value, offset);
#endif
    return sizeof(double);
}

