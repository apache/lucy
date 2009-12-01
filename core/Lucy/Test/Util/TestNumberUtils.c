#define C_LUCY_TESTNUMBERUTILS
#include "Lucy/Util/ToolSet.h"
#include <stdlib.h>
#include <time.h>

#include "Lucy/Test.h"
#include "Lucy/Test/Util/TestNumberUtils.h"
#include "Lucy/Util/NumberUtils.h"

static u64_t*
S_random_ints(u64_t *buf, size_t count, u64_t min, u64_t limit) 
{
    u64_t  range = min < limit ? limit - min : 0;
    u64_t *ints = buf ? buf : (u64_t*)CALLOCATE(count, sizeof(u64_t));
    size_t i;
    for (i = 0; i < count; i++) {
        u64_t num =    ((u64_t)rand() << 60)
                     | ((u64_t)rand() << 45)
                     | ((u64_t)rand() << 30)
                     | ((u64_t)rand() << 15) 
                     | ((u64_t)rand() << 0);
        ints[i] = min + num % range;
    }
    return ints;
}

static void
test_u1(TestBatch *batch)
{
    size_t count   = 64;
    u64_t *ints    = S_random_ints(NULL, count, 0, 2);
    size_t amount  = count / 8;
    u8_t  *bits    = (u8_t*)CALLOCATE(amount, sizeof(u8_t));
    size_t i;

    for (i = 0; i < count; i++) {
        if (ints[i]) { NumUtil_u1set(bits, i); }
    }
    for (i = 0; i < count; i++) {
        ASSERT_INT_EQ(batch, NumUtil_u1get(bits, i), ints[i], "u1 set/get");
    }

    for (i = 0; i < count; i++) {
        NumUtil_u1flip(bits, i);
    }
    for (i = 0; i < count; i++) {
        ASSERT_INT_EQ(batch, NumUtil_u1get(bits, i), !ints[i], "u1 flip");
    }

    FREEMEM(bits);
    FREEMEM(ints);
}

static void
test_u2(TestBatch *batch)
{
    size_t count = 32;
    u64_t *ints = S_random_ints(NULL, count, 0, 4);
    u8_t  *bits = (u8_t*)CALLOCATE((count/4), sizeof(u8_t));
    size_t i;

    for (i = 0; i < count; i++) {
        NumUtil_u2set(bits, i, ints[i]);
    }
    for (i = 0; i < count; i++) {
        ASSERT_INT_EQ(batch, NumUtil_u2get(bits, i), ints[i], "u2");
    }

    FREEMEM(bits);
    FREEMEM(ints);
}

static void
test_u4(TestBatch *batch)
{
    size_t count = 128;
    u64_t *ints = S_random_ints(NULL, count, 0, 16);
    u8_t  *bits = (u8_t*)CALLOCATE((count/2), sizeof(u8_t));
    size_t i;

    for (i = 0; i < count; i++) {
        NumUtil_u4set(bits, i, ints[i]);
    }
    for (i = 0; i < count; i++) {
        ASSERT_INT_EQ(batch, NumUtil_u4get(bits, i), ints[i], "u4");
    }

    FREEMEM(bits);
    FREEMEM(ints);
}

static void
test_c32(TestBatch *batch)
{
    u64_t mins[]   = { 0,   0x4000 - 100, (u32_t)I32_MAX - 100, U32_MAX - 10 };
    u64_t limits[] = { 500, 0x4000 + 100, (u32_t)I32_MAX + 100, U32_MAX      };
    u32_t   set_num;
    u32_t   num_sets  = sizeof(mins) / sizeof(u64_t);
    size_t  count     = 64;
    u64_t  *ints      = NULL;
    size_t  amount    = count * C32_MAX_BYTES;
    char   *encoded   = (char*)CALLOCATE(amount, sizeof(char));
    char   *target    = encoded;
    char   *limit     = target + amount;
    size_t i;

    for (set_num = 0; set_num < num_sets; set_num++) {
        char *skip;
        ints = S_random_ints(ints, count, mins[set_num], limits[set_num]);
        target = encoded;
        for (i = 0; i < count; i++) {
            NumUtil_encode_c32(ints[i], &target);
        }
        target = encoded;
        skip   = encoded;
        for (i = 0; i < count; i++) {
            ASSERT_INT_EQ(batch, NumUtil_decode_c32(&target), (long)ints[i], 
                "c32 %lu", (long)ints[i]);
            NumUtil_skip_cint(&skip);
            if (target > limit) { THROW(ERR, "overrun"); }
        }
        ASSERT_TRUE(batch, skip == target, "skip %lu == %lu", 
            (unsigned long)skip, (unsigned long)target);

        target = encoded;
        for (i = 0; i < count; i++) {
            NumUtil_encode_padded_c32(ints[i], &target);
        }
        ASSERT_TRUE(batch, target == limit, 
            "padded c32 uses 5 bytes (%lu == %lu)", (unsigned long)target, 
            (unsigned long)limit);
        target = encoded;
        skip   = encoded;
        for (i = 0; i < count; i++) {
            ASSERT_INT_EQ(batch, NumUtil_decode_c32(&target), (long)ints[i], 
                "padded c32 %lu", (long)ints[i]);
            NumUtil_skip_cint(&skip);
            if (target > limit) { THROW(ERR, "overrun"); }
        }
        ASSERT_TRUE(batch, skip == target, "skip padded %lu == %lu", 
            (unsigned long)skip, (unsigned long)target);
    }

    target = encoded;
    NumUtil_encode_c32(U32_MAX, &target);
    target = encoded;
    ASSERT_INT_EQ(batch, NumUtil_decode_c32(&target), U32_MAX, "c32 U32_MAX");

    FREEMEM(encoded);
    FREEMEM(ints);
}

static void
test_c64(TestBatch *batch)
{
    u64_t mins[]   = { 0,   0x4000 - 100, (u64_t)U32_MAX - 100,  U64_MAX - 10 };
    u64_t limits[] = { 500, 0x4000 + 100, (u64_t)U32_MAX + 1000, U64_MAX      };
    u32_t   set_num;
    u32_t   num_sets  = sizeof(mins) / sizeof(u64_t);
    size_t  count     = 64;
    u64_t  *ints      = NULL;
    size_t  amount    = count * C64_MAX_BYTES;
    char   *encoded   = (char*)CALLOCATE(amount, sizeof(char));
    char   *target    = encoded;
    char   *limit     = target + amount;
    size_t i;

    for (set_num = 0; set_num < num_sets; set_num++) {
        char *skip;
        ints = S_random_ints(ints, count, mins[set_num], limits[set_num]);
        target = encoded;
        for (i = 0; i < count; i++) {
            NumUtil_encode_c64(ints[i], &target);
        }
        target = encoded;
        skip   = encoded;
        for (i = 0; i < count; i++) {
            u64_t got = NumUtil_decode_c64(&target);
            ASSERT_TRUE(batch, got == ints[i], 
                "c64 %" U64P " == %" U64P, got, ints[i]);
            if (target > limit) { THROW(ERR, "overrun"); }
            NumUtil_skip_cint(&skip);
        }
        ASSERT_TRUE(batch, skip == target, "skip %lu == %lu", 
            (unsigned long)skip, (unsigned long)target);
    }

    target = encoded;
    NumUtil_encode_c64(U64_MAX, &target);
    target = encoded;
    { 
        u64_t got = NumUtil_decode_c64(&target);
        ASSERT_TRUE(batch, got == U64_MAX, "c64 U64_MAX");
    }

    FREEMEM(encoded);
    FREEMEM(ints);
}

static void
test_bigend_u16(TestBatch *batch)
{
    size_t count     = 32;
    u64_t *ints      = S_random_ints(NULL, count, 0, U16_MAX + 1);
    size_t amount    = (count + 1) * sizeof(u16_t);
    char  *allocated = (char*)CALLOCATE(amount, sizeof(char));
    char  *encoded   = allocated + 1; /* Intentionally misaligned. */
    char  *target    = encoded;
    size_t i;

    for (i = 0; i < count; i++) {
        NumUtil_encode_bigend_u16(ints[i], &target);
        target += sizeof(u16_t);
    }
    target = encoded;
    for (i = 0; i < count; i++) {
        u16_t got = NumUtil_decode_bigend_u16(target);
        ASSERT_INT_EQ(batch, got, ints[i], "bigend u16");
        target += sizeof(u16_t);
    }

    target = encoded;
    NumUtil_encode_bigend_u16(1, &target);
    ASSERT_INT_EQ(batch, encoded[0], 0, "Truly big-endian u16");
    ASSERT_INT_EQ(batch, encoded[1], 1, "Truly big-endian u16");

    FREEMEM(allocated);
    FREEMEM(ints);
}

static void
test_bigend_u32(TestBatch *batch)
{
    size_t count     = 32;
    u64_t *ints      = S_random_ints(NULL, count, 0, U64_C(1) + U32_MAX);
    size_t amount    = (count + 1) * sizeof(u32_t);
    char  *allocated = (char*)CALLOCATE(amount, sizeof(char));
    char  *encoded   = allocated + 1; /* Intentionally misaligned. */
    char  *target    = encoded;
    size_t i;

    for (i = 0; i < count; i++) {
        NumUtil_encode_bigend_u32(ints[i], &target);
        target += sizeof(u32_t);
    }
    target = encoded;
    for (i = 0; i < count; i++) {
        u32_t got = NumUtil_decode_bigend_u32(target);
        ASSERT_INT_EQ(batch, got, ints[i], "bigend u32");
        target += sizeof(u32_t);
    }

    target = encoded;
    NumUtil_encode_bigend_u32(1, &target);
    ASSERT_INT_EQ(batch, encoded[0], 0, "Truly big-endian u32");
    ASSERT_INT_EQ(batch, encoded[3], 1, "Truly big-endian u32");

    FREEMEM(allocated);
    FREEMEM(ints);
}

static void
test_bigend_u64(TestBatch *batch)
{
    size_t count     = 32;
    u64_t *ints      = S_random_ints(NULL, count, 0, U64_MAX);
    size_t amount    = (count + 1) * sizeof(u64_t);
    char  *allocated = (char*)CALLOCATE(amount, sizeof(char));
    char  *encoded   = allocated + 1; /* Intentionally misaligned. */
    char  *target    = encoded;
    size_t i;

    for (i = 0; i < count; i++) {
        NumUtil_encode_bigend_u64(ints[i], &target);
        target += sizeof(u64_t);
    }
    target = encoded;
    for (i = 0; i < count; i++) {
        u64_t got = NumUtil_decode_bigend_u64(target);
        ASSERT_TRUE(batch, got == ints[i], "bigend u64");
        target += sizeof(u64_t);
    }

    target = encoded;
    NumUtil_encode_bigend_u64(1, &target);
    ASSERT_INT_EQ(batch, encoded[0], 0, "Truly big-endian");
    ASSERT_INT_EQ(batch, encoded[7], 1, "Truly big-endian");

    FREEMEM(allocated);
    FREEMEM(ints);
}

static void
test_bigend_f32(TestBatch *batch)
{
    float source[]  = { -1.3f, 0.0f, 100.2f };
    size_t count     = 3;
    size_t amount    = (count + 1) * sizeof(float);
    u8_t  *allocated = (u8_t*)CALLOCATE(amount, sizeof(u8_t));
    u8_t  *encoded   = allocated + 1; /* Intentionally misaligned. */
    u8_t  *target    = encoded;
    size_t i;

    for (i = 0; i < count; i++) {
        NumUtil_encode_bigend_f32(source[i], &target);
        target += sizeof(float);
    }
    target = encoded;
    for (i = 0; i < count; i++) {
        float got = NumUtil_decode_bigend_f32(target);
        ASSERT_TRUE(batch, got == source[i], "bigend f32");

        target += sizeof(float);
    }

    target = encoded;
    NumUtil_encode_bigend_f32(-2.0f, &target);
    ASSERT_INT_EQ(batch, (encoded[0] & 0x80), 0x80, 
        "Truly big-endian (IEEE 754 sign bit set for negative number)");
    ASSERT_INT_EQ(batch, encoded[0], 0xC0, 
        "IEEE 754 representation of -2.0f, byte 0");
    for (i = 1; i < sizeof(float); i++) {
        ASSERT_INT_EQ(batch, encoded[i], 0, 
            "IEEE 754 representation of -2.0f, byte %d", (int)i);
    }

    FREEMEM(allocated);
}

static void
test_bigend_f64(TestBatch *batch)
{
    double source[]  = { -1.3, 0.0, 100.2 };
    size_t count     = 3;
    size_t amount    = (count + 1) * sizeof(double);
    u8_t  *allocated = (u8_t*)CALLOCATE(amount, sizeof(u8_t));
    u8_t  *encoded   = allocated + 1; /* Intentionally misaligned. */
    u8_t  *target    = encoded;
    size_t i;

    for (i = 0; i < count; i++) {
        NumUtil_encode_bigend_f64(source[i], &target);
        target += sizeof(double);
    }
    target = encoded;
    for (i = 0; i < count; i++) {
        double got = NumUtil_decode_bigend_f64(target);
        ASSERT_TRUE(batch, got == source[i], "bigend f64");

        target += sizeof(double);
    }

    target = encoded;
    NumUtil_encode_bigend_f64(-2.0, &target);
    ASSERT_INT_EQ(batch, (encoded[0] & 0x80), 0x80, 
        "Truly big-endian (IEEE 754 sign bit set for negative number)");
    ASSERT_INT_EQ(batch, encoded[0], 0xC0, 
        "IEEE 754 representation of -2.0, byte 0");
    for (i = 1; i < sizeof(double); i++) {
        ASSERT_INT_EQ(batch, encoded[i], 0, 
            "IEEE 754 representation of -2.0, byte %d", (int)i);
    }

    FREEMEM(allocated);
}

void
TestNumUtil_run_tests()
{
    TestBatch *batch = Test_new_batch("TestNumberUtils", 1196, NULL);

    PLAN(batch);
    srand((unsigned int)time((time_t*)NULL));

    test_u1(batch);
    test_u2(batch);
    test_u4(batch);
    test_c32(batch);
    test_c64(batch);
    test_bigend_u16(batch);
    test_bigend_u32(batch);
    test_bigend_u64(batch);
    test_bigend_f32(batch);
    test_bigend_f64(batch);

    batch->destroy(batch);
}


/* Copyright 2009 The Apache Software Foundation
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

