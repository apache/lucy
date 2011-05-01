/* Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* Charmonizer/Probe/Integers.h -- info about integer types and sizes.
 *
 * One or the other of these will be defined, depending on whether the
 * processor is big-endian or little-endian.
 *
 * BIG_END
 * LITTLE_END
 *
 * These will always be defined:
 *
 * SIZEOF_CHAR
 * SIZEOF_SHORT
 * SIZEOF_INT
 * SIZEOF_LONG
 * SIZEOF_PTR
 *
 * If long longs are available these symbols will be defined:
 *
 * HAS_LONG_LONG
 * SIZEOF_LONG_LONG
 *
 * Similarly, with the __int64 type (the sizeof is included for completeness):
 *
 * HAS___INT64
 * SIZEOF___INT64
 *
 * If the inttypes.h or stdint.h header files are available, these may be
 * defined:
 *
 * HAS_INTTYPES_H
 * HAS_STDINT_H
 *
 * If stdint.h is is available, it will be pound-included in the configuration
 * header.  If it is not, the following typedefs will be defined if possible:
 *
 * int8_t
 * int16_t
 * int32_t
 * int64_t
 * uint8_t
 * uint16_t
 * uint32_t
 * uint64_t
 *
 * The following typedefs will be created if a suitable integer type exists,
 * as will most often be the case.  However, if for example a char is 64 bits
 * (as on certain Crays), no 8-bit types will be defined, or if no 64-bit
 * integer type is available, no 64-bit types will be defined, etc.
 *
 * bool_t
 * i8_t
 * u8_t
 * i16_t
 * u16_t
 * i32_t
 * u32_t
 * i64_t
 * u64_t
 *
 * Availability of the preceding integer typedefs is indicated by which of
 * these are defined:
 *
 * HAS_I8_T
 * HAS_I16_T
 * HAS_I32_T
 * HAS_I64_T
 *
 * Maximums will be defined for all available integer types (save bool_t), and
 * minimums for all available signed types.
 *
 * I8_MAX
 * U8_MAX
 * I16_MAX
 * U16_MAX
 * I32_MAX
 * U32_MAX
 * I64_MAX
 * U64_MAX
 * I8_MIN
 * I16_MIN
 * I32_MIN
 * I64_MIN
 *
 * If 64-bit integers are available, this macro will promote pointers to i64_t
 * safely.
 *
 * PTR_TO_I64(ptr)
 *
 * If 64-bit integers are available, these macros will expand to the printf
 * conversion specification for signed and unsigned versions (most commonly
 * "lld" and "llu").
 *
 * I64P
 * U64P
 *
 * 32-bit and 64-bit literals can be spec'd via these macros, which append the
 * appropriate postfix:
 *
 * I32_C(n)
 * U32_C(n)
 * I64_C(n)
 * U64_C(n)
 *
 * These symbols will be defined if they are not already:
 *
 * true
 * false
 */

#ifndef H_CHAZ_INTEGERS
#define H_CHAZ_INTEGERS

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

/* Run the Integers module.
 */
void chaz_Integers_run(void);

#ifdef CHAZ_USE_SHORT_NAMES
  #define Integers_run    chaz_Integers_run
#endif

#ifdef __cplusplus
}
#endif

#endif /* H_CHAZ_INTEGERS */



