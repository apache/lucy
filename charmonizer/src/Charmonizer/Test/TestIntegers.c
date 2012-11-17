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

#define CHAZ_USE_SHORT_NAMES

#include "Charmonizer/Test.h"
#include "charmony.h"
#include <stdio.h>
#include <string.h>

static void
S_run_tests(TestBatch *batch) {
    {
        long one = 1;
        long big_endian = !(*((char *)(&one)));
#ifdef BIG_END
        OK(big_endian, "BIG_END");
#else
 #if defined(LITTLE_END)
        OK(!big_endian, "LITTLE_END");
 #else
        FAIL("Either BIG_END or LITTLE_END should be defined");
 #endif
#endif
    }

    LONG_EQ(SIZEOF_CHAR,  sizeof(char),  "SIZEOF_CHAR");
    LONG_EQ(SIZEOF_SHORT, sizeof(short), "SIZEOF_SHORT");
    LONG_EQ(SIZEOF_INT,   sizeof(int),   "SIZEOF_INT");
    LONG_EQ(SIZEOF_LONG,  sizeof(long),  "SIZEOF_LONG");
    LONG_EQ(SIZEOF_PTR,   sizeof(void*), "SIZEOF_PTR");

#ifdef HAS_LONG_LONG
    LONG_EQ(SIZEOF_LONG_LONG, sizeof(long long),
            "HAS_LONG_LONG and SIZEOF_LONG_LONG");
#else
    SKIP("No 'long long' type");
#endif

    {
        bool_t the_truth = true;
        OK(the_truth, "bool_t true");
        OK(!false, "false is false");
    }
#ifdef HAS_INT8_T
    {
        int8_t foo = -100;
        uint8_t bar = 200;
        LONG_EQ(foo, -100, "int8_t is signed");
        LONG_EQ(bar, 200, "uint8_t is unsigned");
        LONG_EQ(sizeof(int8_t), 1, "i8_t is 1 byte");
        LONG_EQ(sizeof(uint8_t), 1, "u8_t is 1 byte");
        LONG_EQ(INT8_MAX,  127, "INT8_MAX");
        LONG_EQ(INT8_MIN, -128, "INT8_MIN");
        LONG_EQ(UINT8_MAX,  255, "UINT8_MAX");
    }
#endif
#ifdef HAS_INT16_T
    {
        int16_t foo = -100;
        uint16_t bar = 30000;
        LONG_EQ(foo, -100, "int16_t is signed");
        LONG_EQ(bar, 30000, "uint16_t is unsigned");
        LONG_EQ(sizeof(int16_t), 2, "int16_t is 2 bytes");
        LONG_EQ(sizeof(uint16_t), 2, "uint16_t is 2 bytes");
        LONG_EQ(INT16_MAX,  32767, "INT16_MAX");
        LONG_EQ(INT16_MIN, -32768, "INT16_MIN");
        LONG_EQ(UINT16_MAX,  65535, "UINT16_MAX");
    }
#endif
#ifdef HAS_INT32_T
    {
        int32_t foo = -100;
        uint32_t bar = 4000000000UL;
        OK((foo == -100), "int32_t is signed");
        OK((bar == 4000000000UL), "uint32_t is unsigned");
        OK((sizeof(int32_t) == 4), "int32_t is 4 bytes");
        OK((sizeof(uint32_t) == 4), "uint32_t is 4 bytes");
        OK((INT32_MAX == INT32_C(2147483647)), "INT32_MAX");
        /* The (-2147483647 - 1) avoids a compiler warning. */
        OK((INT32_MIN == -INT32_C(2147483647) - 1), "INT32_MIN");
        OK((UINT32_MAX == UINT32_C(4294967295)), "UINT32_MAX");
    }
#endif
#ifdef HAS_INT64_T
    {
        char buf[100];
        int64_t foo = -100;
        uint64_t bar = UINT64_C(18000000000000000000);
        OK((foo == -100), "int64_t is signed");
        OK((bar == UINT64_C(18000000000000000000)), "uint64_t is unsigned");
        OK((sizeof(int64_t) == 8), "int64_t is 8 bytes");
        OK((sizeof(uint64_t) == 8), "uint64_t is 8 bytes");
        OK((INT64_MAX == INT64_C(0x7FFFFFFFFFFFFFFF)), "INT64_MAX");
        OK((INT64_MIN == -INT64_C(0x7FFFFFFFFFFFFFFF) - 1), "INT64_MIN");
        OK((UINT64_MAX == UINT64_C(0xFFFFFFFFFFFFFFFF)), "UINT64_MAX");
        sprintf(buf, "%"I64P, foo);
        STR_EQ(buf, "-100", "I64P");
        sprintf(buf, "%"U64P, bar);
        STR_EQ(buf, "18000000000000000000", "U64P");
    }
#endif
}

int main(int argc, char **argv) {
    TestBatch *batch = Test_start(39);
    S_run_tests(batch);
    return !Test_finish();
}

