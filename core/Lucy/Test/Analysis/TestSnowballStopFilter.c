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

#define C_LUCY_TESTSNOWBALLSTOPFILTER
#include "Lucy/Util/ToolSet.h"
#include <stdarg.h>

#include "Lucy/Test.h"
#include "Lucy/Test/Analysis/TestSnowballStopFilter.h"
#include "Lucy/Analysis/SnowballStopFilter.h"

static SnowballStopFilter*
S_make_stopfilter(void *unused, ...) {
    va_list args;
    SnowballStopFilter *self = (SnowballStopFilter*)VTable_Make_Obj(SNOWBALLSTOPFILTER);
    Hash *stoplist = Hash_new(0);
    char *stopword;

    va_start(args, unused);
    while (NULL != (stopword = va_arg(args, char*))) {
        Hash_Store_Str(stoplist, stopword, strlen(stopword), INCREF(&EMPTY));
    }
    va_end(args);

    self = SnowStop_init(self, NULL, stoplist);
    DECREF(stoplist);
    return self;
}

static void
test_Dump_Load_and_Equals(TestBatch *batch) {
    SnowballStopFilter *stopfilter =
        S_make_stopfilter(NULL, "foo", "bar", "baz", NULL);
    SnowballStopFilter *other =
        S_make_stopfilter(NULL, "foo", "bar", NULL);
    Obj *dump       = SnowStop_Dump(stopfilter);
    Obj *other_dump = SnowStop_Dump(other);
    SnowballStopFilter *clone       = (SnowballStopFilter*)SnowStop_Load(other, dump);
    SnowballStopFilter *other_clone = (SnowballStopFilter*)SnowStop_Load(other, other_dump);

    TEST_FALSE(batch,
               SnowStop_Equals(stopfilter, (Obj*)other),
               "Equals() false with different stoplist");
    TEST_TRUE(batch,
              SnowStop_Equals(stopfilter, (Obj*)clone),
              "Dump => Load round trip");
    TEST_TRUE(batch,
              SnowStop_Equals(other, (Obj*)other_clone),
              "Dump => Load round trip");

    DECREF(stopfilter);
    DECREF(dump);
    DECREF(clone);
    DECREF(other);
    DECREF(other_dump);
    DECREF(other_clone);
}

void
TestSnowStop_run_tests() {
    TestBatch *batch = TestBatch_new(3);

    TestBatch_Plan(batch);

    test_Dump_Load_and_Equals(batch);

    DECREF(batch);
}



