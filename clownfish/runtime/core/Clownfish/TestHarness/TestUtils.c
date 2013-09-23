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
#include <string.h>

#define CHY_USE_SHORT_NAMES
#define CFISH_USE_SHORT_NAMES

#include "charmony.h"

#include "Clownfish/TestHarness/TestUtils.h"

#include "Clownfish/CharBuf.h"
#include "Clownfish/String.h"
#include "Clownfish/Util/Memory.h"

uint64_t
TestUtils_random_u64() {
    uint64_t num = ((uint64_t)(rand()   & 0x7FFF) << 60)
                   | ((uint64_t)(rand() & 0x7FFF) << 45)
                   | ((uint64_t)(rand() & 0x7FFF) << 30)
                   | ((uint64_t)(rand() & 0x7FFF) << 15)
                   | ((uint64_t)(rand() & 0x7FFF) << 0);
    return num;
}

int64_t*
TestUtils_random_i64s(int64_t *buf, size_t count, int64_t min,
                      int64_t limit) {
    uint64_t  range = min < limit ? limit - min : 0;
    int64_t *ints = buf ? buf : (int64_t*)CALLOCATE(count, sizeof(int64_t));
    for (size_t i = 0; i < count; i++) {
        ints[i] = min + TestUtils_random_u64() % range;
    }
    return ints;
}

uint64_t*
TestUtils_random_u64s(uint64_t *buf, size_t count, uint64_t min,
                      uint64_t limit) {
    uint64_t  range = min < limit ? limit - min : 0;
    uint64_t *ints = buf ? buf : (uint64_t*)CALLOCATE(count, sizeof(uint64_t));
    for (size_t i = 0; i < count; i++) {
        ints[i] = min + TestUtils_random_u64() % range;
    }
    return ints;
}

double*
TestUtils_random_f64s(double *buf, size_t count) {
    double *f64s = buf ? buf : (double*)CALLOCATE(count, sizeof(double));
    for (size_t i = 0; i < count; i++) {
        uint64_t num = TestUtils_random_u64();
        f64s[i] = U64_TO_DOUBLE(num) / UINT64_MAX;
    }
    return f64s;
}

static int32_t
S_random_code_point(void) {
    int32_t code_point = 0;
    while (1) {
        uint8_t chance = (rand() % 9) + 1;
        switch (chance) {
            case 1: case 2: case 3:
                code_point = rand() % 0x80;
                break;
            case 4: case 5: case 6:
                code_point = (rand() % (0x0800  - 0x0080)) + 0x0080;
                break;
            case 7: case 8:
                code_point = (rand() % (0x10000 - 0x0800)) + 0x0800;
                break;
            case 9: {
                    uint64_t num = TestUtils_random_u64();
                    code_point = (num % (0x10FFFF - 0x10000)) + 0x10000;
                }
        }
        if (code_point > 0x10FFFF) {
            continue; // Too high.
        }
        if (code_point > 0xD7FF && code_point < 0xE000) {
            continue; // UTF-16 surrogate.
        }
        break;
    }
    return code_point;
}

String*
TestUtils_random_string(size_t length) {
    CharBuf *buf = CB_new(length);
    while (length--) {
        CB_Cat_Char(buf, S_random_code_point());
    }
    String *string = CB_Yield_String(buf);
    DECREF(buf);
    return string;
}

String*
TestUtils_get_str(const char *ptr) {
    return Str_new_from_utf8(ptr, strlen(ptr));
}


