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

#include <stdlib.h>
#include <time.h>

#define TESTLUCY_USE_SHORT_NAMES
#include "Lucy/Util/ToolSet.h"

#include "charmony.h"

#include "Lucy/Test/Util/TestNumberUtils.h"

#include "Clownfish/TestHarness/TestBatchRunner.h"
#include "Clownfish/TestHarness/TestUtils.h"
#include "Lucy/Util/NumberUtils.h"

TestNumberUtils*
TestNumUtil_new() {
    return (TestNumberUtils*)Class_Make_Obj(TESTNUMBERUTILS);
}

static void
test_u1(TestBatchRunner *runner) {
    size_t    count   = 64;
    uint64_t *ints    = TestUtils_random_u64s(NULL, count, 0, 2);
    size_t    amount  = count / 8;
    uint8_t  *bits    = (uint8_t*)CALLOCATE(amount, sizeof(uint8_t));

    for (size_t i = 0; i < count; i++) {
        if (ints[i]) { NumUtil_u1set(bits, i); }
    }
    for (size_t i = 0; i < count; i++) {
        TEST_INT_EQ(runner, NumUtil_u1get(bits, i), (long)ints[i],
                    "u1 set/get");
    }

    for (size_t i = 0; i < count; i++) {
        NumUtil_u1flip(bits, i);
    }
    for (size_t i = 0; i < count; i++) {
        TEST_INT_EQ(runner, NumUtil_u1get(bits, i), !ints[i], "u1 flip");
    }

    FREEMEM(bits);
    FREEMEM(ints);
}

static void
test_u2(TestBatchRunner *runner) {
    size_t    count = 32;
    uint64_t *ints = TestUtils_random_u64s(NULL, count, 0, 4);
    uint8_t  *bits = (uint8_t*)CALLOCATE((count / 4), sizeof(uint8_t));

    for (size_t i = 0; i < count; i++) {
        NumUtil_u2set(bits, i, (uint8_t)ints[i]);
    }
    for (size_t i = 0; i < count; i++) {
        TEST_INT_EQ(runner, NumUtil_u2get(bits, i), (long)ints[i], "u2");
    }

    FREEMEM(bits);
    FREEMEM(ints);
}

static void
test_u4(TestBatchRunner *runner) {
    size_t    count = 128;
    uint64_t *ints  = TestUtils_random_u64s(NULL, count, 0, 16);
    uint8_t  *bits  = (uint8_t*)CALLOCATE((count / 2), sizeof(uint8_t));

    for (size_t i = 0; i < count; i++) {
        NumUtil_u4set(bits, i, (uint8_t)ints[i]);
    }
    for (size_t i = 0; i < count; i++) {
        TEST_INT_EQ(runner, NumUtil_u4get(bits, i), (long)ints[i], "u4");
    }

    FREEMEM(bits);
    FREEMEM(ints);
}

static void
test_cu32(TestBatchRunner *runner) {
    uint64_t  mins[]   = { 0,   0x4000 - 100, (uint32_t)INT32_MAX - 100, UINT32_MAX - 10 };
    uint64_t  limits[] = { 500, 0x4000 + 100, (uint32_t)INT32_MAX + 100, UINT32_MAX      };
    uint32_t  set_num;
    uint32_t  num_sets  = sizeof(mins) / sizeof(uint64_t);
    size_t    count     = 64;
    uint64_t *ints      = NULL;
    size_t    amount    = count * CU32_MAX_BYTES;
    char     *encoded   = (char*)CALLOCATE(amount, sizeof(char));
    char     *target    = encoded;
    char     *limit     = target + amount;
    const char *decode;

    for (set_num = 0; set_num < num_sets; set_num++) {
        const char *skip;
        ints = TestUtils_random_u64s(ints, count,
                                     mins[set_num], limits[set_num]);
        target = encoded;
        for (size_t i = 0; i < count; i++) {
            ints[i] = (uint32_t)ints[i];
            NumUtil_encode_cu32((uint32_t)ints[i], &target);
        }
        decode = encoded;
        skip   = encoded;
        for (size_t i = 0; i < count; i++) {
            TEST_INT_EQ(runner, NumUtil_decode_cu32(&decode), ints[i],
                        "cu32 %lu", (long)ints[i]);
            NumUtil_skip_cint(&skip);
            if (decode > limit) { THROW(ERR, "overrun"); }
        }
        TEST_TRUE(runner, skip == decode, "skip %lu == %lu",
                  (unsigned long)skip, (unsigned long)decode);

        target = encoded;
        for (size_t i = 0; i < count; i++) {
            NumUtil_encode_padded_cu32((uint32_t)ints[i], &target);
        }
        TEST_TRUE(runner, target == limit,
                  "padded cu32 uses 5 bytes (%lu == %lu)", (unsigned long)target,
                  (unsigned long)limit);
        decode = encoded;
        skip   = encoded;
        for (size_t i = 0; i < count; i++) {
            TEST_INT_EQ(runner, NumUtil_decode_cu32(&decode), ints[i],
                        "padded cu32 %lu", (long)ints[i]);
            NumUtil_skip_cint(&skip);
            if (decode > limit) { THROW(ERR, "overrun"); }
        }
        TEST_TRUE(runner, skip == decode, "skip padded %lu == %lu",
                  (unsigned long)skip, (unsigned long)decode);
    }

    target = encoded;
    NumUtil_encode_cu32(UINT32_MAX, &target);
    decode = encoded;
    TEST_INT_EQ(runner, NumUtil_decode_cu32(&decode), UINT32_MAX, "cu32 UINT32_MAX");

    FREEMEM(encoded);
    FREEMEM(ints);
}

static void
test_cu64(TestBatchRunner *runner) {
    uint64_t  mins[]    = { 0,   0x4000 - 100, (uint64_t)UINT32_MAX - 100,  UINT64_MAX - 10 };
    uint64_t  limits[]  = { 500, 0x4000 + 100, (uint64_t)UINT32_MAX + 1000, UINT64_MAX      };
    uint32_t  set_num;
    uint32_t  num_sets  = sizeof(mins) / sizeof(uint64_t);
    size_t    count     = 64;
    uint64_t *ints      = NULL;
    size_t    amount    = count * CU64_MAX_BYTES;
    char     *encoded   = (char*)CALLOCATE(amount, sizeof(char));
    char     *target    = encoded;
    char     *limit     = target + amount;
    const char *decode;

    for (set_num = 0; set_num < num_sets; set_num++) {
        const char *skip;
        ints = TestUtils_random_u64s(ints, count,
                                     mins[set_num], limits[set_num]);
        target = encoded;
        for (size_t i = 0; i < count; i++) {
            NumUtil_encode_cu64(ints[i], &target);
        }
        decode = encoded;
        skip   = encoded;
        for (size_t i = 0; i < count; i++) {
            uint64_t got = NumUtil_decode_cu64(&decode);
            TEST_TRUE(runner, got == ints[i],
                      "cu64 %" PRIu64 " == %" PRIu64, got, ints[i]);
            if (decode > limit) { THROW(ERR, "overrun"); }
            NumUtil_skip_cint(&skip);
        }
        TEST_TRUE(runner, skip == decode, "skip %lu == %lu",
                  (unsigned long)skip, (unsigned long)decode);
    }

    target = encoded;
    NumUtil_encode_cu64(UINT64_MAX, &target);

    decode = encoded;
    uint64_t got = NumUtil_decode_cu64(&decode);
    TEST_TRUE(runner, got == UINT64_MAX, "cu64 UINT64_MAX");

    FREEMEM(encoded);
    FREEMEM(ints);
}

static void
test_bigend_u16(TestBatchRunner *runner) {
    size_t    count     = 32;
    uint64_t *ints      = TestUtils_random_u64s(NULL, count, 0, UINT16_MAX + 1);
    size_t    amount    = (count + 1) * sizeof(uint16_t);
    char     *allocated = (char*)CALLOCATE(amount, sizeof(char));
    char     *encoded   = allocated + 1; // Intentionally misaligned.
    char     *target    = encoded;

    for (size_t i = 0; i < count; i++) {
        NumUtil_encode_bigend_u16((uint16_t)ints[i], &target);
        target += sizeof(uint16_t);
    }
    target = encoded;
    for (size_t i = 0; i < count; i++) {
        uint16_t got = NumUtil_decode_bigend_u16(target);
        TEST_INT_EQ(runner, got, (long)ints[i], "bigend u16");
        target += sizeof(uint16_t);
    }

    target = encoded;
    NumUtil_encode_bigend_u16(1, &target);
    TEST_INT_EQ(runner, encoded[0], 0, "Truly big-endian u16");
    TEST_INT_EQ(runner, encoded[1], 1, "Truly big-endian u16");

    FREEMEM(allocated);
    FREEMEM(ints);
}

static void
test_bigend_u32(TestBatchRunner *runner) {
    size_t    count     = 32;
    uint64_t *ints      = TestUtils_random_u64s(NULL, count, 0, UINT64_C(1) + UINT32_MAX);
    size_t    amount    = (count + 1) * sizeof(uint32_t);
    char     *allocated = (char*)CALLOCATE(amount, sizeof(char));
    char     *encoded   = allocated + 1; // Intentionally misaligned.
    char     *target    = encoded;

    for (size_t i = 0; i < count; i++) {
        ints[i] = (uint32_t)ints[i];
        NumUtil_encode_bigend_u32((uint32_t)ints[i], &target);
        target += sizeof(uint32_t);
    }
    target = encoded;
    for (size_t i = 0; i < count; i++) {
        uint32_t got = NumUtil_decode_bigend_u32(target);
        TEST_INT_EQ(runner, got, ints[i], "bigend u32");
        target += sizeof(uint32_t);
    }

    target = encoded;
    NumUtil_encode_bigend_u32(1, &target);
    TEST_INT_EQ(runner, encoded[0], 0, "Truly big-endian u32");
    TEST_INT_EQ(runner, encoded[3], 1, "Truly big-endian u32");

    FREEMEM(allocated);
    FREEMEM(ints);
}

static void
test_bigend_u64(TestBatchRunner *runner) {
    size_t    count     = 32;
    uint64_t *ints      = TestUtils_random_u64s(NULL, count, 0, UINT64_MAX);
    size_t    amount    = (count + 1) * sizeof(uint64_t);
    char     *allocated = (char*)CALLOCATE(amount, sizeof(char));
    char     *encoded   = allocated + 1; // Intentionally misaligned.
    char     *target    = encoded;

    for (size_t i = 0; i < count; i++) {
        NumUtil_encode_bigend_u64(ints[i], &target);
        target += sizeof(uint64_t);
    }
    target = encoded;
    for (size_t i = 0; i < count; i++) {
        uint64_t got = NumUtil_decode_bigend_u64(target);
        TEST_TRUE(runner, got == ints[i], "bigend u64");
        target += sizeof(uint64_t);
    }

    target = encoded;
    NumUtil_encode_bigend_u64(1, &target);
    TEST_INT_EQ(runner, encoded[0], 0, "Truly big-endian");
    TEST_INT_EQ(runner, encoded[7], 1, "Truly big-endian");

    FREEMEM(allocated);
    FREEMEM(ints);
}

static void
test_bigend_f32(TestBatchRunner *runner) {
    float    source[]  = { -1.3f, 0.0f, 100.2f };
    size_t   count     = 3;
    size_t   amount    = (count + 1) * sizeof(float);
    uint8_t *allocated = (uint8_t*)CALLOCATE(amount, sizeof(uint8_t));
    uint8_t *encoded   = allocated + 1; // Intentionally misaligned.
    uint8_t *target    = encoded;

    for (size_t i = 0; i < count; i++) {
        NumUtil_encode_bigend_f32(source[i], &target);
        target += sizeof(float);
    }
    target = encoded;
    for (size_t i = 0; i < count; i++) {
        float got = NumUtil_decode_bigend_f32(target);
        TEST_TRUE(runner, got == source[i], "bigend f32");
        target += sizeof(float);
    }

    target = encoded;
    NumUtil_encode_bigend_f32(-2.0f, &target);
    TEST_INT_EQ(runner, (encoded[0] & 0x80), 0x80,
                "Truly big-endian (IEEE 754 sign bit set for negative number)");
    TEST_INT_EQ(runner, encoded[0], 0xC0,
                "IEEE 754 representation of -2.0f, byte 0");
    for (size_t i = 1; i < sizeof(float); i++) {
        TEST_INT_EQ(runner, encoded[i], 0,
                    "IEEE 754 representation of -2.0f, byte %d", (int)i);
    }

    FREEMEM(allocated);
}

static void
test_bigend_f64(TestBatchRunner *runner) {
    double   source[]  = { -1.3, 0.0, 100.2 };
    size_t   count     = 3;
    size_t   amount    = (count + 1) * sizeof(double);
    uint8_t *allocated = (uint8_t*)CALLOCATE(amount, sizeof(uint8_t));
    uint8_t *encoded   = allocated + 1; // Intentionally misaligned.
    uint8_t *target    = encoded;

    for (size_t i = 0; i < count; i++) {
        NumUtil_encode_bigend_f64(source[i], &target);
        target += sizeof(double);
    }
    target = encoded;
    for (size_t i = 0; i < count; i++) {
        double got = NumUtil_decode_bigend_f64(target);
        TEST_TRUE(runner, got == source[i], "bigend f64");
        target += sizeof(double);
    }

    target = encoded;
    NumUtil_encode_bigend_f64(-2.0, &target);
    TEST_INT_EQ(runner, (encoded[0] & 0x80), 0x80,
                "Truly big-endian (IEEE 754 sign bit set for negative number)");
    TEST_INT_EQ(runner, encoded[0], 0xC0,
                "IEEE 754 representation of -2.0, byte 0");
    for (size_t i = 1; i < sizeof(double); i++) {
        TEST_INT_EQ(runner, encoded[i], 0,
                    "IEEE 754 representation of -2.0, byte %d", (int)i);
    }

    FREEMEM(allocated);
}

void
TestNumUtil_Run_IMP(TestNumberUtils *self, TestBatchRunner *runner) {
    TestBatchRunner_Plan(runner, (TestBatch*)self, 1196);
    srand((unsigned int)time((time_t*)NULL));
    test_u1(runner);
    test_u2(runner);
    test_u4(runner);
    test_cu32(runner);
    test_cu64(runner);
    test_bigend_u16(runner);
    test_bigend_u32(runner);
    test_bigend_u64(runner);
    test_bigend_f32(runner);
    test_bigend_f64(runner);
}



