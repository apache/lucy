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

#define C_LUCY_TESTTOKENIZER
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Test.h"
#include "Lucy/Test/Analysis/TestTokenizer.h"
#include "Lucy/Analysis/Tokenizer.h"


static void
test_Dump_Load_and_Equals(TestBatch *batch)
{
    ZombieCharBuf *word_char_pattern  = ZCB_WRAP_STR("\\w+", 3);  
    ZombieCharBuf *whitespace_pattern = ZCB_WRAP_STR("\\S+", 3);
    Tokenizer *word_char_tokenizer =
        Tokenizer_new((CharBuf*)word_char_pattern);
    Tokenizer *whitespace_tokenizer =
        Tokenizer_new((CharBuf*)whitespace_pattern);
    Obj *word_char_dump  = Tokenizer_Dump(word_char_tokenizer);
    Obj *whitespace_dump = Tokenizer_Dump(whitespace_tokenizer);
    Tokenizer *word_char_clone 
        = Tokenizer_Load(whitespace_tokenizer, word_char_dump);
    Tokenizer *whitespace_clone 
        = Tokenizer_Load(whitespace_tokenizer, whitespace_dump);

    TEST_FALSE(batch, Tokenizer_Equals(word_char_tokenizer,
        (Obj*)whitespace_tokenizer), "Equals() false with different pattern");
    TEST_TRUE(batch, Tokenizer_Equals(word_char_tokenizer,
        (Obj*)word_char_clone), "Dump => Load round trip");
    TEST_TRUE(batch, Tokenizer_Equals(whitespace_tokenizer,
        (Obj*)whitespace_clone), "Dump => Load round trip");

    DECREF(word_char_tokenizer);
    DECREF(word_char_dump);
    DECREF(word_char_clone);
    DECREF(whitespace_tokenizer);
    DECREF(whitespace_dump);
    DECREF(whitespace_clone);
}

void
TestTokenizer_run_tests()
{
    TestBatch *batch = TestBatch_new(3);

    TestBatch_Plan(batch);

    test_Dump_Load_and_Equals(batch);

    DECREF(batch);
}



