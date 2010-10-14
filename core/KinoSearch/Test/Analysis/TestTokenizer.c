#define C_KINO_TESTTOKENIZER
#include "KinoSearch/Util/ToolSet.h"

#include "KinoSearch/Test.h"
#include "KinoSearch/Test/Analysis/TestTokenizer.h"
#include "KinoSearch/Analysis/Tokenizer.h"


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


/* Copyright 2005-2010 Marvin Humphrey
 *
 * This program is free software; you can redistribute it and/or modify
 * under the same terms as Perl itself.
 */

