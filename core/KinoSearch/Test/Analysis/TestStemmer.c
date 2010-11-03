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

#define C_LUCY_TESTSTEMMER
#include "KinoSearch/Util/ToolSet.h"

#include "KinoSearch/Test.h"
#include "KinoSearch/Test/Analysis/TestStemmer.h"
#include "KinoSearch/Analysis/Stemmer.h"

static void
test_Dump_Load_and_Equals(TestBatch *batch)
{
    CharBuf *EN          = (CharBuf*)ZCB_WRAP_STR("en", 2); 
    CharBuf *ES          = (CharBuf*)ZCB_WRAP_STR("es", 2); 
    Stemmer *stemmer     = Stemmer_new(EN);
    Stemmer *other       = Stemmer_new(ES);
    Obj     *dump        = (Obj*)Stemmer_Dump(stemmer);
    Obj     *other_dump  = (Obj*)Stemmer_Dump(other);
    Stemmer *clone       = (Stemmer*)Stemmer_Load(other, dump);
    Stemmer *other_clone = (Stemmer*)Stemmer_Load(other, other_dump);

    TEST_FALSE(batch, Stemmer_Equals(stemmer,
        (Obj*)other), "Equals() false with different language");
    TEST_TRUE(batch, Stemmer_Equals(stemmer,
        (Obj*)clone), "Dump => Load round trip");
    TEST_TRUE(batch, Stemmer_Equals(other,
        (Obj*)other_clone), "Dump => Load round trip");

    DECREF(stemmer);
    DECREF(dump);
    DECREF(clone);
    DECREF(other);
    DECREF(other_dump);
    DECREF(other_clone);
}

void
TestStemmer_run_tests()
{
    TestBatch *batch = TestBatch_new(3);

    TestBatch_Plan(batch);

    test_Dump_Load_and_Equals(batch);

    DECREF(batch);
}



