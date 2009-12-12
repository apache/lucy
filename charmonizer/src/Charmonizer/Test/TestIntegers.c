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
TestIntegers_prepare()
{
    return Test_new_batch("Integers", 37, TestIntegers_run);
}

void
TestIntegers_run(TestBatch *batch)
{
    {
        long one= 1;
        long big_endian = !(*((char *)(&one)));
#ifdef BIG_END
        ASSERT_TRUE(batch, big_endian, "BIG_END");
#else
 #if defined(LITTLE_END)
        ASSERT_TRUE(batch, !big_endian, "LITTLE_END");
 #else
        FAIL(batch, "Either BIG_END or LITTLE_END should be defined");
 #endif
#endif
    }
    
    ASSERT_INT_EQ(batch, SIZEOF_CHAR,  sizeof(char),  "SIZEOF_CHAR");
    ASSERT_INT_EQ(batch, SIZEOF_SHORT, sizeof(short), "SIZEOF_SHORT");
    ASSERT_INT_EQ(batch, SIZEOF_INT,   sizeof(int),   "SIZEOF_INT");
    ASSERT_INT_EQ(batch, SIZEOF_LONG,  sizeof(long),  "SIZEOF_LONG");
    ASSERT_INT_EQ(batch, SIZEOF_PTR,   sizeof(void*), "SIZEOF_PTR");

#ifdef HAS_LONG_LONG
    ASSERT_INT_EQ(batch, SIZEOF_LONG_LONG, sizeof(long long), 
        "HAS_LONG_LONG and SIZEOF_LONG_LONG");
#endif
    
#ifdef HAS_INTTYPES_H
    ASSERT_INT_EQ(batch, sizeof(int8_t), 1, "HAS_INTTYPES_H");
#else
    SKIP(batch, "No inttypes.h");
#endif

    {
        bool_t the_truth = true;
        ASSERT_TRUE(batch, the_truth, "bool_t true");
        ASSERT_FALSE(batch, false, "false is false");
    }
#ifdef HAS_I8_T
    {
        i8_t foo = -100;
        u8_t bar = 200;
        ASSERT_INT_EQ(batch, foo, -100, "i8_t is signed");
        ASSERT_INT_EQ(batch, bar, 200, "u8_t is unsigned");
        ASSERT_INT_EQ(batch, sizeof(i8_t), 1, "i8_t is 1 byte");
        ASSERT_INT_EQ(batch, sizeof(u8_t), 1, "u8_t is 1 byte");
        ASSERT_INT_EQ(batch, I8_MAX,  127, "I8_MAX");
        ASSERT_INT_EQ(batch, I8_MIN, -128, "I8_MIN");
        ASSERT_INT_EQ(batch, U8_MAX,  255, "U8_MAX");
    }
#endif
#ifdef HAS_I16_T
    {
        i16_t foo = -100;
        u16_t bar = 30000;
        ASSERT_INT_EQ(batch, foo, -100, "i16_t is signed");
        ASSERT_INT_EQ(batch, bar, 30000, "u16_t is unsigned");
        ASSERT_INT_EQ(batch, sizeof(i16_t), 2, "i16_t is 2 bytes");
        ASSERT_INT_EQ(batch, sizeof(u16_t), 2, "u16_t is 2 bytes");
        ASSERT_INT_EQ(batch, I16_MAX,  32767, "I16_MAX");
        ASSERT_INT_EQ(batch, I16_MIN, -32768, "I16_MIN");
        ASSERT_INT_EQ(batch, U16_MAX,  65535, "U16_MAX");
    }
#endif
#ifdef HAS_I32_T
    {
        i32_t foo = -100;
        u32_t bar = 4000000000UL;
        ASSERT_TRUE(batch, (foo == -100), "i32_t is signed");
        ASSERT_TRUE(batch, (bar == 4000000000UL), "u32_t is unsigned");
        ASSERT_TRUE(batch, (sizeof(i32_t) == 4), "i32_t is 4 bytes");
        ASSERT_TRUE(batch, (sizeof(u32_t) == 4), "u32_t is 4 bytes");
        ASSERT_TRUE(batch, (I32_MAX == I32_C(2147483647)), "I32_MAX");
        /* the (-2147483647 - 1) avoids a compiler warning */
        ASSERT_TRUE(batch, (I32_MIN == I32_C(-2147483647 - 1)), "I32_MIN");
        ASSERT_TRUE(batch, (U32_MAX == U32_C(4294967295)), "U32_MAX");
    }
#endif
#ifdef HAS_I64_T
    {
        char buf[100];
        i64_t foo = -100;
        u64_t bar = U64_C(18000000000000000000);
        ASSERT_TRUE(batch, (foo == -100), "i64_t is signed");
        ASSERT_TRUE(batch, (bar == U64_C(18000000000000000000)), 
            "u64_t is unsigned");
        ASSERT_TRUE(batch, (sizeof(i64_t) == 8), "i64_t is 8 bytes");
        ASSERT_TRUE(batch, (sizeof(u64_t) == 8), "u64_t is 8 bytes");
        sprintf(buf, "%"I64P, foo);
        ASSERT_STR_EQ(batch, buf, "-100", "I64P");
        sprintf(buf, "%"U64P, bar);
        ASSERT_STR_EQ(batch, buf, "18000000000000000000", "U64P");
    }
#endif
}

/**
 * Copyright 2006 The Apache Software Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

