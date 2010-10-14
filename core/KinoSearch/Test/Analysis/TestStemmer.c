#define C_KINO_TESTSTEMMER
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


/* Copyright 2005-2010 Marvin Humphrey
 *
 * This program is free software; you can redistribute it and/or modify
 * under the same terms as Perl itself.
 */

