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
#include "Charmonizer/Test/AllTests.h"
#include "charmony.h"
#include <stdio.h>
#include <string.h>
#ifdef HAS_INTTYPES_H
    #include <inttypes.h>
#endif

TestBatch*
TestIntegers_prepare() {
    return Test_new_batch("Integers", 37, TestIntegers_run);
}

void
TestIntegers_run(TestBatch *batch) {
    {
        long one = 1;
        long big_endian = !(*((char *)(&one)));
#ifdef BIG_END
        TEST_TRUE(batch, big_endian, "BIG_END");
#else
 #if defined(LITTLE_END)
        TEST_TRUE(batch, !big_endian, "LITTLE_END");
 #else
        FAIL(batch, "Either BIG_END or LITTLE_END should be defined");
 #endif
#endif
    }

    TEST_INT_EQ(batch, SIZEOF_CHAR,  sizeof(char),  "SIZEOF_CHAR");
    TEST_INT_EQ(batch, SIZEOF_SHORT, sizeof(short), "SIZEOF_SHORT");
    TEST_INT_EQ(batch, SIZEOF_INT,   sizeof(int),   "SIZEOF_INT");
    TEST_INT_EQ(batch, SIZEOF_LONG,  sizeof(long),  "SIZEOF_LONG");
    TEST_INT_EQ(batch, SIZEOF_PTR,   sizeof(void*), "SIZEOF_PTR");

#ifdef HAS_LONG_LONG
    TEST_INT_EQ(batch, SIZEOF_LONG_LONG, sizeof(long long),
                "HAS_LONG_LONG and SIZEOF_LONG_LONG");
#endif

#ifdef HAS_INTTYPES_H
    TEST_INT_EQ(batch, sizeof(int8_t), 1, "HAS_INTTYPES_H");
#else
    SKIP(batch, "No inttypes.h");
#endif

    {
        bool_t the_truth = true;
        TEST_TRUE(batch, the_truth, "bool_t true");
        TEST_FALSE(batch, false, "false is false");
    }
#ifdef HAS_I8_T
    {
        int8_t foo = -100;
        uint8_t bar = 200;
        TEST_INT_EQ(batch, foo, -100, "int8_t is signed");
        TEST_INT_EQ(batch, bar, 200, "uint8_t is unsigned");
        TEST_INT_EQ(batch, sizeof(int8_t), 1, "i8_t is 1 byte");
        TEST_INT_EQ(batch, sizeof(uint8_t), 1, "u8_t is 1 byte");
        TEST_INT_EQ(batch, I8_MAX,  127, "I8_MAX");
        TEST_INT_EQ(batch, I8_MIN, -128, "I8_MIN");
        TEST_INT_EQ(batch, U8_MAX,  255, "U8_MAX");
    }
#endif
#ifdef HAS_I16_T
    {
        int16_t foo = -100;
        uint16_t bar = 30000;
        TEST_INT_EQ(batch, foo, -100, "int16_t is signed");
        TEST_INT_EQ(batch, bar, 30000, "uint16_t is unsigned");
        TEST_INT_EQ(batch, sizeof(int16_t), 2, "int16_t is 2 bytes");
        TEST_INT_EQ(batch, sizeof(uint16_t), 2, "uint16_t is 2 bytes");
        TEST_INT_EQ(batch, I16_MAX,  32767, "I16_MAX");
        TEST_INT_EQ(batch, I16_MIN, -32768, "I16_MIN");
        TEST_INT_EQ(batch, U16_MAX,  65535, "U16_MAX");
    }
#endif
#ifdef HAS_I32_T
    {
        int32_t foo = -100;
        uint32_t bar = 4000000000UL;
        TEST_TRUE(batch, (foo == -100), "int32_t is signed");
        TEST_TRUE(batch, (bar == 4000000000UL), "uint32_t is unsigned");
        TEST_TRUE(batch, (sizeof(int32_t) == 4), "int32_t is 4 bytes");
        TEST_TRUE(batch, (sizeof(uint32_t) == 4), "uint32_t is 4 bytes");
        TEST_TRUE(batch, (I32_MAX == I32_C(2147483647)), "I32_MAX");
        /* The (-2147483647 - 1) avoids a compiler warning. */
        TEST_TRUE(batch, (I32_MIN == I32_C(-2147483647 - 1)), "I32_MIN");
        TEST_TRUE(batch, (U32_MAX == U32_C(4294967295)), "U32_MAX");
    }
#endif
#ifdef HAS_I64_T
    {
        char buf[100];
        int64_t foo = -100;
        uint64_t bar = U64_C(18000000000000000000);
        TEST_TRUE(batch, (foo == -100), "int64_t is signed");
        TEST_TRUE(batch, (bar == U64_C(18000000000000000000)),
                  "uint64_t is unsigned");
        TEST_TRUE(batch, (sizeof(int64_t) == 8), "int64_t is 8 bytes");
        TEST_TRUE(batch, (sizeof(uint64_t) == 8), "uint64_t is 8 bytes");
        sprintf(buf, "%"I64P, foo);
        TEST_STR_EQ(batch, buf, "-100", "I64P");
        sprintf(buf, "%"U64P, bar);
        TEST_STR_EQ(batch, buf, "18000000000000000000", "U64P");
    }
#endif
}


