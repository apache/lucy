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
 * header.  If it is not, the following typedefs and macros will be defined if
 * possible:
 *
 * int8_t
 * int16_t
 * int32_t
 * int64_t
 * uint8_t
 * uint16_t
 * uint32_t
 * uint64_t
 * INT8_MAX
 * INT16_MAX
 * INT32_MAX
 * INT64_MAX
 * INT8_MIN
 * INT16_MIN
 * INT32_MIN
 * INT64_MIN
 * UINT8_MAX
 * UINT16_MAX
 * UINT32_MAX
 * UINT64_MAX
 * SIZE_MAX
 * INT32_C
 * INT64_C
 * UINT32_C
 * UINT64_C
 *
 * If inttypes.h is is available, it will be pound-included in the
 * configuration header.  If it is not, the following macros will be defined if
 * possible:
 *
 * PRId64
 * PRIu64
 *
 * Availability of integer types is indicated by which of these are defined:
 *
 * HAS_INT8_T
 * HAS_INT16_T
 * HAS_INT32_T
 * HAS_INT64_T
 *
 * If 64-bit integers are available, this macro will promote pointers to i64_t
 * safely.
 *
 * PTR_TO_I64(ptr)
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

#ifdef __cplusplus
}
#endif

#endif /* H_CHAZ_INTEGERS */



