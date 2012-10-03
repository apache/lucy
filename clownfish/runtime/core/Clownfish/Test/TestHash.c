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

#define LUCY_USE_SHORT_NAMES
#define CHY_USE_SHORT_NAMES

#include "Clownfish/Test.h"
#include "Clownfish/Test/TestUtils.h"
#include "Clownfish/Test/TestHash.h"
#include "Clownfish/Hash.h"
#include "Clownfish/CharBuf.h"
#include "Clownfish/VArray.h"
#include "Clownfish/Num.h"

static void
test_Equals(TestBatch *batch) {
    Hash *hash  = Hash_new(0);
    Hash *other = Hash_new(0);
    ZombieCharBuf *stuff = ZCB_WRAP_STR("stuff", 5);

    TEST_TRUE(batch, Hash_Equals(hash, (Obj*)other),
              "Empty hashes are equal");

    Hash_Store_Str(hash, "foo", 3, (Obj*)CFISH_TRUE);
    TEST_FALSE(batch, Hash_Equals(hash, (Obj*)other),
               "Add one pair and Equals returns false");

    Hash_Store_Str(other, "foo", 3, (Obj*)CFISH_TRUE);
    TEST_TRUE(batch, Hash_Equals(hash, (Obj*)other),
              "Add a matching pair and Equals returns true");

    Hash_Store_Str(other, "foo", 3, INCREF(stuff));
    TEST_FALSE(batch, Hash_Equals(hash, (Obj*)other),
               "Non-matching value spoils Equals");

    DECREF(hash);
    DECREF(other);
}

static void
test_Store_and_Fetch(TestBatch *batch) {
    Hash          *hash         = Hash_new(100);
    Hash          *dupe         = Hash_new(100);
    const uint32_t starting_cap = Hash_Get_Capacity(hash);
    VArray        *expected     = VA_new(100);
    VArray        *got          = VA_new(100);
    ZombieCharBuf *twenty       = ZCB_WRAP_STR("20", 2);
    ZombieCharBuf *forty        = ZCB_WRAP_STR("40", 2);
    ZombieCharBuf *foo          = ZCB_WRAP_STR("foo", 3);

    for (int32_t i = 0; i < 100; i++) {
        CharBuf *cb = CB_newf("%i32", i);
        Hash_Store(hash, (Obj*)cb, (Obj*)cb);
        Hash_Store(dupe, (Obj*)cb, INCREF(cb));
        VA_Push(expected, INCREF(cb));
    }
    TEST_TRUE(batch, Hash_Equals(hash, (Obj*)dupe), "Equals");

    TEST_INT_EQ(batch, Hash_Get_Capacity(hash), starting_cap,
                "Initial capacity sufficient (no rebuilds)");

    for (int32_t i = 0; i < 100; i++) {
        Obj *key  = VA_Fetch(expected, i);
        Obj *elem = Hash_Fetch(hash, key);
        VA_Push(got, (Obj*)INCREF(elem));
    }

    TEST_TRUE(batch, VA_Equals(got, (Obj*)expected),
              "basic Store and Fetch");
    TEST_INT_EQ(batch, Hash_Get_Size(hash), 100,
                "size incremented properly by Hash_Store");

    TEST_TRUE(batch, Hash_Fetch(hash, (Obj*)foo) == NULL,
              "Fetch against non-existent key returns NULL");

    Hash_Store(hash, (Obj*)forty, INCREF(foo));
    TEST_TRUE(batch, ZCB_Equals(foo, Hash_Fetch(hash, (Obj*)forty)),
              "Hash_Store replaces existing value");
    TEST_FALSE(batch, Hash_Equals(hash, (Obj*)dupe),
               "replacement value spoils equals");
    TEST_INT_EQ(batch, Hash_Get_Size(hash), 100,
                "size unaffected after value replaced");

    TEST_TRUE(batch, Hash_Delete(hash, (Obj*)forty) == (Obj*)foo,
              "Delete returns value");
    DECREF(foo);
    TEST_INT_EQ(batch, Hash_Get_Size(hash), 99,
                "size decremented by successful Delete");
    TEST_TRUE(batch, Hash_Delete(hash, (Obj*)forty) == NULL,
              "Delete returns NULL when key not found");
    TEST_INT_EQ(batch, Hash_Get_Size(hash), 99,
                "size not decremented by unsuccessful Delete");
    DECREF(Hash_Delete(dupe, (Obj*)forty));
    TEST_TRUE(batch, VA_Equals(got, (Obj*)expected), "Equals after Delete");

    Hash_Clear(hash);
    TEST_TRUE(batch, Hash_Fetch(hash, (Obj*)twenty) == NULL, "Clear");
    TEST_TRUE(batch, Hash_Get_Size(hash) == 0, "size is 0 after Clear");

    DECREF(hash);
    DECREF(dupe);
    DECREF(got);
    DECREF(expected);
}

static void
test_Keys_Values_Iter(TestBatch *batch) {
    Hash     *hash     = Hash_new(0); // trigger multiple rebuilds.
    VArray   *expected = VA_new(100);
    VArray   *keys;
    VArray   *values;

    for (uint32_t i = 0; i < 500; i++) {
        CharBuf *cb = CB_newf("%u32", i);
        Hash_Store(hash, (Obj*)cb, (Obj*)cb);
        VA_Push(expected, INCREF(cb));
    }

    VA_Sort(expected, NULL, NULL);

    keys   = Hash_Keys(hash);
    values = Hash_Values(hash);
    VA_Sort(keys, NULL, NULL);
    VA_Sort(values, NULL, NULL);
    TEST_TRUE(batch, VA_Equals(keys, (Obj*)expected), "Keys");
    TEST_TRUE(batch, VA_Equals(values, (Obj*)expected), "Values");
    VA_Clear(keys);
    VA_Clear(values);

    {
        Obj *key;
        Obj *value;
        Hash_Iterate(hash);
        while (Hash_Next(hash, &key, &value)) {
            VA_Push(keys, INCREF(key));
            VA_Push(values, INCREF(value));
        }
    }

    VA_Sort(keys, NULL, NULL);
    VA_Sort(values, NULL, NULL);
    TEST_TRUE(batch, VA_Equals(keys, (Obj*)expected), "Keys from Iter");
    TEST_TRUE(batch, VA_Equals(values, (Obj*)expected), "Values from Iter");

    {
        ZombieCharBuf *forty = ZCB_WRAP_STR("40", 2);
        ZombieCharBuf *nope  = ZCB_WRAP_STR("nope", 4);
        Obj *key = Hash_Find_Key(hash, (Obj*)forty, ZCB_Hash_Sum(forty));
        TEST_TRUE(batch, Obj_Equals(key, (Obj*)forty), "Find_Key");
        key = Hash_Find_Key(hash, (Obj*)nope, ZCB_Hash_Sum(nope)),
        TEST_TRUE(batch, key == NULL,
                  "Find_Key returns NULL for non-existent key");
    }

    DECREF(hash);
    DECREF(expected);
    DECREF(keys);
    DECREF(values);
}

static void
test_Dump_and_Load(TestBatch *batch) {
    Hash *hash = Hash_new(0);
    Obj  *dump;
    Hash *loaded;

    Hash_Store_Str(hash, "foo", 3,
                   (Obj*)CB_new_from_trusted_utf8("foo", 3));
    dump = (Obj*)Hash_Dump(hash);
    loaded = (Hash*)Obj_Load(dump, dump);
    TEST_TRUE(batch, Hash_Equals(hash, (Obj*)loaded),
              "Dump => Load round trip");
    DECREF(dump);
    DECREF(loaded);

    /* TODO: Fix Hash_Load().

    Hash_Store_Str(hash, "_class", 6,
        (Obj*)CB_new_from_trusted_utf8("not_a_class", 11));
    dump = (Obj*)Hash_Dump(hash);
    loaded = (Hash*)Obj_Load(dump, dump);

    TEST_TRUE(batch, Hash_Equals(hash, (Obj*)loaded),
              "Load still works with _class if it's not a real class");
    DECREF(dump);
    DECREF(loaded);

    */

    DECREF(hash);
}

static void
test_stress(TestBatch *batch) {
    Hash     *hash     = Hash_new(0); // trigger multiple rebuilds.
    VArray   *expected = VA_new(1000);
    VArray   *keys;
    VArray   *values;

    for (uint32_t i = 0; i < 1000; i++) {
        CharBuf *cb = TestUtils_random_string(rand() % 1200);
        while (Hash_Fetch(hash, (Obj*)cb)) {
            DECREF(cb);
            cb = TestUtils_random_string(rand() % 1200);
        }
        Hash_Store(hash, (Obj*)cb, (Obj*)cb);
        VA_Push(expected, INCREF(cb));
    }

    VA_Sort(expected, NULL, NULL);

    // Overwrite for good measure.
    for (uint32_t i = 0; i < 1000; i++) {
        CharBuf *cb = (CharBuf*)VA_Fetch(expected, i);
        Hash_Store(hash, (Obj*)cb, INCREF(cb));
    }

    keys   = Hash_Keys(hash);
    values = Hash_Values(hash);
    VA_Sort(keys, NULL, NULL);
    VA_Sort(values, NULL, NULL);
    TEST_TRUE(batch, VA_Equals(keys, (Obj*)expected), "stress Keys");
    TEST_TRUE(batch, VA_Equals(values, (Obj*)expected), "stress Values");

    DECREF(keys);
    DECREF(values);
    DECREF(expected);
    DECREF(hash);
}

void
TestHash_run_tests() {
    TestBatch *batch = TestBatch_new(28);

    srand((unsigned int)time((time_t*)NULL));

    TestBatch_Plan(batch);
    test_Equals(batch);
    test_Store_and_Fetch(batch);
    test_Keys_Values_Iter(batch);
    test_Dump_and_Load(batch);
    test_stress(batch);

    DECREF(batch);
}


