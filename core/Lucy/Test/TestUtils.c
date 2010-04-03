#define C_LUCY_TESTUTILS
#include "Lucy/Util/ToolSet.h"
#include <stdarg.h>
#include <string.h>

#include "Lucy/Test/TestUtils.h"
#include "Lucy/Store/InStream.h"
#include "Lucy/Store/OutStream.h"
#include "Lucy/Store/RAMFile.h"
#include "Lucy/Util/Freezer.h"

uint64_t
TestUtils_random_u64()
{
    uint64_t num = ((uint64_t)rand() << 60)
                 | ((uint64_t)rand() << 45)
                 | ((uint64_t)rand() << 30)
                 | ((uint64_t)rand() << 15) 
                 | ((uint64_t)rand() << 0);
    return num;
}

int64_t*
TestUtils_random_i64s(int64_t *buf, size_t count, int64_t min, int64_t limit) 
{
    uint64_t  range = min < limit ? limit - min : 0;
    int64_t *ints = buf ? buf : (int64_t*)CALLOCATE(count, sizeof(int64_t));
    size_t i;
    for (i = 0; i < count; i++) {
        ints[i] = min + TestUtils_random_u64() % range;
    }
    return ints;
}

uint64_t*
TestUtils_random_u64s(uint64_t *buf, size_t count, uint64_t min, uint64_t limit) 
{
    uint64_t  range = min < limit ? limit - min : 0;
    uint64_t *ints = buf ? buf : (uint64_t*)CALLOCATE(count, sizeof(uint64_t));
    size_t i;
    for (i = 0; i < count; i++) {
        ints[i] = min + TestUtils_random_u64() % range;
    }
    return ints;
}

double*
TestUtils_random_f64s(double *buf, size_t count) 
{
    double *f64s = buf ? buf : (double*)CALLOCATE(count, sizeof(double));
    size_t i;
    for (i = 0; i < count; i++) {
        uint64_t num = TestUtils_random_u64();
        f64s[i] = (double)num / U64_MAX;
    }
    return f64s;
}

VArray*
TestUtils_doc_set()
{
    VArray *docs = VA_new(10);

    VA_Push(docs, (Obj*)TestUtils_get_cb("x"));
    VA_Push(docs, (Obj*)TestUtils_get_cb("y"));
    VA_Push(docs, (Obj*)TestUtils_get_cb("z"));
    VA_Push(docs, (Obj*)TestUtils_get_cb("x a"));
    VA_Push(docs, (Obj*)TestUtils_get_cb("x a b"));
    VA_Push(docs, (Obj*)TestUtils_get_cb("x a b c"));
    VA_Push(docs, (Obj*)TestUtils_get_cb("x foo a b c d"));
    
    return docs;
}

CharBuf*
TestUtils_get_cb(const char *ptr)
{
    return CB_new_from_utf8(ptr, strlen(ptr));
}

Obj*
TestUtils_freeze_thaw(Obj *object)
{
    if (object) {
        RAMFile *ram_file = RAMFile_new(NULL, false);
        OutStream *outstream = OutStream_open((Obj*)ram_file);
        FREEZE(object, outstream);
        OutStream_Close(outstream);
        DECREF(outstream);
        {
            InStream *instream = InStream_open((Obj*)ram_file);
            Obj *retval = THAW(instream);
            DECREF(instream);
            DECREF(ram_file);
            return retval;
        }
    }
    else {
        return NULL;
    }
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

