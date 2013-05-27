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


#define C_TESTLUCY_TESTREVERSETYPE
#define TESTLUCY_USE_SHORT_NAMES
#include "Lucy/Util/ToolSet.h"
#include <stdio.h>
#include <stdlib.h>

#include "Clownfish/TestHarness/TestFormatter.h"
#include "Clownfish/TestHarness/TestUtils.h"
#include "Lucy/Test.h"
#include "Lucy/Test/Search/TestSortSpec.h"
#include "Lucy/Test/TestUtils.h"
#include "Lucy/Search/SortSpec.h"

#include "Lucy/Analysis/StandardTokenizer.h"
#include "Lucy/Document/Doc.h"
#include "Lucy/Document/HitDoc.h"
#include "Lucy/Index/Indexer.h"
#include "Lucy/Plan/FullTextType.h"
#include "Lucy/Plan/NumericType.h"
#include "Lucy/Plan/Schema.h"
#include "Lucy/Plan/StringType.h"
#include "Lucy/Search/Hits.h"
#include "Lucy/Search/IndexSearcher.h"
#include "Lucy/Search/SortRule.h"
#include "Lucy/Store/RAMFolder.h"

static CharBuf *air_cb;
static CharBuf *airplane_cb;
static CharBuf *bike_cb;
static CharBuf *car_cb;
static CharBuf *carrot_cb;
static CharBuf *cat_cb;
static CharBuf *float32_cb;
static CharBuf *float64_cb;
static CharBuf *food_cb;
static CharBuf *home_cb;
static CharBuf *int32_cb;
static CharBuf *int64_cb;
static CharBuf *land_cb;
static CharBuf *name_cb;
static CharBuf *nope_cb;
static CharBuf *num_cb;
static CharBuf *random_cb;
static CharBuf *sloth_cb;
static CharBuf *speed_cb;
static CharBuf *unknown_cb;
static CharBuf *unused_cb;
static CharBuf *vehicle_cb;
static CharBuf *weight_cb;

static CharBuf *random_float32s_cb;
static CharBuf *random_float64s_cb;
static CharBuf *random_int32s_cb;
static CharBuf *random_int64s_cb;

TestSortSpec*
TestSortSpec_new(TestFormatter *formatter) {
    TestSortSpec *self = (TestSortSpec*)VTable_Make_Obj(TESTSORTSPEC);
    return TestSortSpec_init(self, formatter);
}

TestSortSpec*
TestSortSpec_init(TestSortSpec *self, TestFormatter *formatter) {
    return (TestSortSpec*)TestBatch_init((TestBatch*)self, 18, formatter);
}

static void
S_init_strings() {
    air_cb      = CB_newf("air");
    airplane_cb = CB_newf("airplane");
    bike_cb     = CB_newf("bike");
    car_cb      = CB_newf("car");
    carrot_cb   = CB_newf("carrot");
    cat_cb      = CB_newf("cat");
    float32_cb  = CB_newf("float32");
    float64_cb  = CB_newf("float64");
    food_cb     = CB_newf("food");
    home_cb     = CB_newf("home");
    int32_cb    = CB_newf("int32");
    int64_cb    = CB_newf("int64");
    land_cb     = CB_newf("land");
    name_cb     = CB_newf("name");
    nope_cb     = CB_newf("nope");
    num_cb      = CB_newf("num");
    random_cb   = CB_newf("random");
    sloth_cb    = CB_newf("sloth");
    speed_cb    = CB_newf("speed");
    unknown_cb  = CB_newf("unknown");
    unused_cb   = CB_newf("unused");
    vehicle_cb  = CB_newf("vehicle");
    weight_cb   = CB_newf("weight");

    random_float32s_cb = CB_newf("random_float32s");
    random_float64s_cb = CB_newf("random_float64s");
    random_int32s_cb   = CB_newf("random_int32s");
    random_int64s_cb   = CB_newf("random_int64s");
}

static void
S_destroy_strings() {
    DECREF(air_cb);
    DECREF(airplane_cb);
    DECREF(bike_cb);
    DECREF(car_cb);
    DECREF(carrot_cb);
    DECREF(cat_cb);
    DECREF(float32_cb);
    DECREF(float64_cb);
    DECREF(food_cb);
    DECREF(home_cb);
    DECREF(int32_cb);
    DECREF(int64_cb);
    DECREF(land_cb);
    DECREF(name_cb);
    DECREF(nope_cb);
    DECREF(num_cb);
    DECREF(random_cb);
    DECREF(sloth_cb);
    DECREF(speed_cb);
    DECREF(unknown_cb);
    DECREF(unused_cb);
    DECREF(vehicle_cb);
    DECREF(weight_cb);

    DECREF(random_float32s_cb);
    DECREF(random_float64s_cb);
    DECREF(random_int32s_cb);
    DECREF(random_int64s_cb);
}

TestReverseType*
TestReverseType_new() {
    TestReverseType *self = (TestReverseType*)VTable_Make_Obj(TESTREVERSETYPE);
    return TestReverseType_init(self);
}

TestReverseType*
TestReverseType_init(TestReverseType *self) {
    return TestReverseType_init2(self, 1.0, false, true, true);
}

TestReverseType*
TestReverseType_init2(TestReverseType *self, float boost, bool indexed,
                      bool stored, bool sortable) {
    FType_init((FieldType*)self);
    self->boost      = boost;
    self->indexed    = indexed;
    self->stored     = stored;
    self->sortable   = sortable;
    return self;
}

int32_t
TestReverseType_compare_values(TestReverseType *self, Obj *a, Obj *b) {
    UNUSED_VAR(self);
    return Obj_Compare_To(b, a);
}

static Schema*
S_create_schema() {
    Schema *schema = Schema_new();

    StandardTokenizer *tokenizer = StandardTokenizer_new();
    FullTextType *unsortable = FullTextType_new((Analyzer*)tokenizer);
    DECREF(tokenizer);

    StringType *string_type = StringType_new();
    StringType_Set_Sortable(string_type, true);

    Int32Type *int32_type = Int32Type_new();
    Int32Type_Set_Indexed(int32_type, false);
    Int32Type_Set_Sortable(int32_type, true);

    Int64Type *int64_type = Int64Type_new();
    Int64Type_Set_Indexed(int64_type, false);
    Int64Type_Set_Sortable(int64_type, true);

    Float32Type *float32_type = Float32Type_new();
    Float32Type_Set_Indexed(float32_type, false);
    Float32Type_Set_Sortable(float32_type, true);

    Float64Type *float64_type = Float64Type_new();
    Float64Type_Set_Indexed(float64_type, false);
    Float64Type_Set_Sortable(float64_type, true);

    TestReverseType *reverse_type = TestReverseType_new();

    Schema_Spec_Field(schema, name_cb,    (FieldType*)string_type);
    Schema_Spec_Field(schema, speed_cb,   (FieldType*)int32_type);
    Schema_Spec_Field(schema, sloth_cb,   (FieldType*)reverse_type);
    Schema_Spec_Field(schema, weight_cb,  (FieldType*)int32_type);
    Schema_Spec_Field(schema, int32_cb,   (FieldType*)int32_type);
    Schema_Spec_Field(schema, int64_cb,   (FieldType*)int64_type);
    Schema_Spec_Field(schema, float32_cb, (FieldType*)float32_type);
    Schema_Spec_Field(schema, float64_cb, (FieldType*)float64_type);
    Schema_Spec_Field(schema, home_cb,    (FieldType*)string_type);
    Schema_Spec_Field(schema, cat_cb,     (FieldType*)string_type);
    Schema_Spec_Field(schema, unused_cb,  (FieldType*)string_type);
    Schema_Spec_Field(schema, nope_cb,    (FieldType*)unsortable);

    DECREF(reverse_type);
    DECREF(float64_type);
    DECREF(float32_type);
    DECREF(int64_type);
    DECREF(int32_type);
    DECREF(string_type);
    DECREF(unsortable);

    return schema;
}

static void
S_refresh_indexer(Indexer **indexer, Schema *schema, RAMFolder *folder) {
    if (*indexer) {
        Indexer_Commit(*indexer);
        DECREF(*indexer);
    }
    *indexer = Indexer_new(schema, (Obj*)folder, NULL, 0);
}

static void
S_add_vehicle(Indexer *indexer, CharBuf *name, int32_t speed, int32_t sloth,
              int32_t weight, CharBuf *home, CharBuf *cat) {
    Doc       *doc   = Doc_new(NULL, 0);

    Doc_Store(doc, name_cb, (Obj*)name);
    Doc_Store(doc, home_cb, (Obj*)home);
    Doc_Store(doc, cat_cb,  (Obj*)cat);

    CharBuf *string;
    string = CB_newf("%i32", speed);
    Doc_Store(doc, speed_cb, (Obj*)string);
    DECREF(string);
    string = CB_newf("%i32", sloth);
    Doc_Store(doc, sloth_cb, (Obj*)string);
    DECREF(string);
    string = CB_newf("%i32", weight);
    Doc_Store(doc, weight_cb, (Obj*)string);
    DECREF(string);

    Indexer_Add_Doc(indexer, doc, 1.0f);

    DECREF(doc);
}

static void
S_add_doc(Indexer *indexer, Obj *name_obj, CharBuf *cat, CharBuf *field_name) {
    Doc *doc = Doc_new(NULL, 0);
    CharBuf *name = Obj_To_String(name_obj);
    Doc_Store(doc, name_cb, (Obj*)name);
    Doc_Store(doc, cat_cb,  (Obj*)cat);
    if (field_name) {
        Doc_Store(doc, field_name, (Obj*)name);
    }
    Indexer_Add_Doc(indexer, doc, 1.0f);
    DECREF(name);
    DECREF(doc);
}

typedef Obj* (*random_generator_t)();

static Obj*
S_random_string() {
    size_t length = 1 + rand() % 10;
    CharBuf *string = CB_new(length);
    while (length--) {
        uint32_t code_point = 'a' + rand() % ('z' - 'a' + 1);
        CB_Cat_Char(string, code_point);
    }
    return (Obj*)string;
}

static Obj*
S_random_int32() {
    uint64_t num = TestUtils_random_u64();
    return (Obj*)Int32_new(num & 0x7FFFFFFF);
}

static Obj*
S_random_int64() {
    uint64_t num = TestUtils_random_u64();
    return (Obj*)Int64_new(num & INT64_C(0x7FFFFFFFFFFFFFFF));
}

static Obj*
S_random_float32() {
    uint64_t num = TestUtils_random_u64();
    return (Obj*)Float32_new(U64_TO_DOUBLE(num) * (10.0 / UINT64_MAX));
}

static Obj*
S_random_float64() {
    uint64_t num = TestUtils_random_u64();
    return (Obj*)Float64_new(U64_TO_DOUBLE(num) * (10.0 / UINT64_MAX));
}

static VArray*
S_add_random_objects(Indexer **indexer, Schema *schema, RAMFolder *folder,
                     random_generator_t rng, CharBuf *field_name,
                     CharBuf *cat) {
    VArray *objects = VA_new(100);

    for (int i = 0; i < 100; ++i) {
        Obj *object = rng();
        S_add_doc(*indexer, object, cat, field_name);
        VA_Push(objects, object);
        if (i % 10 == 0) {
            S_refresh_indexer(indexer, schema, folder);
        }
    }

    VA_Sort(objects, NULL, NULL);

    for (int i = 0; i < 100; ++i) {
        Obj *obj = VA_Fetch(objects, i);
        CharBuf *string = Obj_To_String(obj);
        VA_Store(objects, i, (Obj*)string);
    }

    return objects;
}

static VArray*
S_test_sorted_search(IndexSearcher *searcher, CharBuf *query,
                     uint32_t num_wanted, ...) {
    VArray  *rules = VA_new(2);
    CharBuf *field;
    va_list  args;

    va_start(args, num_wanted);
    while (NULL != (field = va_arg(args, CharBuf*))) {
        bool        reverse = va_arg(args, int);
        SortRule *rule    = SortRule_new(SortRule_FIELD, field, reverse);
        VA_Push(rules, (Obj*)rule);
    }
    va_end(args);
    SortRule *rule = SortRule_new(SortRule_DOC_ID, NULL, 0);
    VA_Push(rules, (Obj*)rule);
    SortSpec *spec = SortSpec_new(rules);

    Hits *hits = IxSearcher_Hits(searcher, (Obj*)query, 0, num_wanted, spec);

    VArray *results = VA_new(10);
    ViewCharBuf *name = (ViewCharBuf*)ZCB_BLANK();
    HitDoc *hit_doc;
    while (NULL != (hit_doc = Hits_Next(hits))) {
        HitDoc_Extract(hit_doc, name_cb, name);
        VA_Push(results, (Obj*)CB_Clone((CharBuf*)name));
        DECREF(hit_doc);
    }

    DECREF(hits);
    DECREF(spec);
    DECREF(rules);

    return results;
}

typedef struct SortContext {
    IndexSearcher *searcher;
    CharBuf       *sort_field;
} SortContext;

static void
S_attempt_sorted_search(void *context) {
    SortContext *sort_ctx = (SortContext*)context;
    VArray *results = S_test_sorted_search(sort_ctx->searcher, vehicle_cb, 100,
                                           sort_ctx->sort_field, false, NULL);
    DECREF(results);
}

static void
test_sort_spec(TestBatch *batch) {
    RAMFolder *folder  = RAMFolder_new(NULL);
    Schema    *schema  = S_create_schema();
    Indexer   *indexer = NULL;
    VArray    *wanted  = VA_new(10);
    VArray    *results;
    VArray    *results2;

    // First, add vehicles.
    S_refresh_indexer(&indexer, schema, folder);
    S_add_vehicle(indexer, airplane_cb, 200, 200, 8000, air_cb,  vehicle_cb);
    S_add_vehicle(indexer, bike_cb,      15,  15,   25, land_cb, vehicle_cb);
    S_add_vehicle(indexer, car_cb,       70,  70, 3000, land_cb, vehicle_cb);

    // Add random objects.
    VArray *random_strings =
        S_add_random_objects(&indexer, schema, folder, S_random_string,
                             NULL, random_cb);
    VArray *random_int32s =
        S_add_random_objects(&indexer, schema, folder, S_random_int32,
                             int32_cb, random_int32s_cb);
    VArray *random_int64s =
        S_add_random_objects(&indexer, schema, folder, S_random_int64,
                             int64_cb, random_int64s_cb);
    VArray *random_float32s =
        S_add_random_objects(&indexer, schema, folder, S_random_float32,
                             float32_cb, random_float32s_cb);
    VArray *random_float64s =
        S_add_random_objects(&indexer, schema, folder, S_random_float64,
                             float64_cb, random_float64s_cb);

    // Add numbers to verify consistent ordering.
    int32_t *nums = (int32_t*)MALLOCATE(100 * sizeof(int32_t));
    for (int i = 0; i < 100; ++i) {
        nums[i] = i;
    }
    // Shuffle
    for (int i = 99; i > 0; --i) {
        int r = rand() % (i + 1);
        if (r != i) {
            // Swap
            int32_t tmp = nums[i];
            nums[i] = nums[r];
            nums[r] = tmp;
        }
    }
    for (int i = 0; i < 100; ++i) {
        char name_buf[3];
        sprintf(name_buf, "%02d", nums[i]);
        ZombieCharBuf *name = ZCB_WRAP_STR(name_buf, 2);
        S_add_doc(indexer, (Obj*)name, num_cb, NULL);
        if (i % 10 == 0) {
            S_refresh_indexer(&indexer, schema, folder);
        }
    }
    FREEMEM(nums);

    Indexer_Commit(indexer);
    DECREF(indexer);

    // Start tests

    IndexSearcher *searcher = IxSearcher_new((Obj*)folder);

    results = S_test_sorted_search(searcher, vehicle_cb, 100,
                                   name_cb, false, NULL);
    VA_Clear(wanted);
    VA_Push(wanted, INCREF(airplane_cb));
    VA_Push(wanted, INCREF(bike_cb));
    VA_Push(wanted, INCREF(car_cb));
    TEST_TRUE(batch, VA_Equals(results, (Obj*)wanted), "sort by one criteria");
    DECREF(results);

#ifdef LUCY_VALGRIND
    SKIP(batch, "known leaks");
    SKIP(batch, "known leaks");
#else
    Err *error;
    SortContext sort_ctx;
    sort_ctx.searcher = searcher;

    sort_ctx.sort_field = nope_cb;
    error = Err_trap(S_attempt_sorted_search, &sort_ctx);
    TEST_TRUE(batch, error != NULL
              && Err_Is_A(error, ERR)
              && CB_Find_Str(Err_Get_Mess(error), "sortable", 8) != -1,
              "sorting on a non-sortable field throws an error");
    DECREF(error);

    sort_ctx.sort_field = unknown_cb;
    error = Err_trap(S_attempt_sorted_search, &sort_ctx);
    TEST_TRUE(batch, error != NULL
              && Err_Is_A(error, ERR)
              && CB_Find_Str(Err_Get_Mess(error), "sortable", 8) != -1,
              "sorting on an unknown field throws an error");
    DECREF(error);
#endif

    results = S_test_sorted_search(searcher, vehicle_cb, 100,
                                   weight_cb, false, NULL);
    VA_Clear(wanted);
    VA_Push(wanted, INCREF(bike_cb));
    VA_Push(wanted, INCREF(car_cb));
    VA_Push(wanted, INCREF(airplane_cb));
    TEST_TRUE(batch, VA_Equals(results, (Obj*)wanted), "sort by one criteria");
    DECREF(results);

    results = S_test_sorted_search(searcher, vehicle_cb, 100,
                                   name_cb, true, NULL);
    VA_Clear(wanted);
    VA_Push(wanted, INCREF(car_cb));
    VA_Push(wanted, INCREF(bike_cb));
    VA_Push(wanted, INCREF(airplane_cb));
    TEST_TRUE(batch, VA_Equals(results, (Obj*)wanted), "reverse sort");
    DECREF(results);

    results = S_test_sorted_search(searcher, vehicle_cb, 100,
                                   home_cb, false, name_cb, false, NULL);
    VA_Clear(wanted);
    VA_Push(wanted, INCREF(airplane_cb));
    VA_Push(wanted, INCREF(bike_cb));
    VA_Push(wanted, INCREF(car_cb));
    TEST_TRUE(batch, VA_Equals(results, (Obj*)wanted), "multiple criteria");
    DECREF(results);

    results = S_test_sorted_search(searcher, vehicle_cb, 100,
                                   home_cb, false, name_cb, true, NULL);
    VA_Clear(wanted);
    VA_Push(wanted, INCREF(airplane_cb));
    VA_Push(wanted, INCREF(car_cb));
    VA_Push(wanted, INCREF(bike_cb));
    TEST_TRUE(batch, VA_Equals(results, (Obj*)wanted),
              "multiple criteria with reverse");
    DECREF(results);

    results = S_test_sorted_search(searcher, vehicle_cb, 100,
                                   speed_cb, true, NULL);
    results2 = S_test_sorted_search(searcher, vehicle_cb, 100,
                                    sloth_cb, false, NULL);
    TEST_TRUE(batch, VA_Equals(results, (Obj*)results2),
              "FieldType_Compare_Values");
    DECREF(results2);
    DECREF(results);

    results = S_test_sorted_search(searcher, random_cb, 100,
                                   name_cb, false, NULL);
    TEST_TRUE(batch, VA_Equals(results, (Obj*)random_strings),
              "random strings");
    DECREF(results);

    results = S_test_sorted_search(searcher, random_int32s_cb, 100,
                                   int32_cb, false, NULL);
    TEST_TRUE(batch, VA_Equals(results, (Obj*)random_int32s),
              "int32");
    DECREF(results);

    results = S_test_sorted_search(searcher, random_int64s_cb, 100,
                                   int64_cb, false, NULL);
    TEST_TRUE(batch, VA_Equals(results, (Obj*)random_int64s),
              "int64");
    DECREF(results);

    results = S_test_sorted_search(searcher, random_float32s_cb, 100,
                                   float32_cb, false, NULL);
    TEST_TRUE(batch, VA_Equals(results, (Obj*)random_float32s),
              "float32");
    DECREF(results);

    results = S_test_sorted_search(searcher, random_float64s_cb, 100,
                                   float64_cb, false, NULL);
    TEST_TRUE(batch, VA_Equals(results, (Obj*)random_float64s),
              "float64");
    DECREF(results);

    CharBuf *bbbcca_cb = CB_newf("bike bike bike car car airplane");
    results = S_test_sorted_search(searcher, bbbcca_cb, 100,
                                   unused_cb, false, NULL);
    VA_Clear(wanted);
    VA_Push(wanted, INCREF(airplane_cb));
    VA_Push(wanted, INCREF(bike_cb));
    VA_Push(wanted, INCREF(car_cb));
    TEST_TRUE(batch, VA_Equals(results, (Obj*)wanted),
              "sorting on field with no values sorts by doc id");
    DECREF(results);
    DECREF(bbbcca_cb);

    CharBuf *nn_cb        = CB_newf("99");
    CharBuf *nn_or_car_cb = CB_newf("99 OR car");
    results = S_test_sorted_search(searcher, nn_or_car_cb, 10,
                                   speed_cb, false, NULL);
    VA_Clear(wanted);
    VA_Push(wanted, INCREF(car_cb));
    VA_Push(wanted, INCREF(nn_cb));
    TEST_TRUE(batch, VA_Equals(results, (Obj*)wanted),
              "doc with NULL value sorts last");
    DECREF(results);
    DECREF(nn_cb);
    DECREF(nn_or_car_cb);

    results = S_test_sorted_search(searcher, num_cb, 10,
                                   name_cb, false, NULL);
    results2 = S_test_sorted_search(searcher, num_cb, 30,
                                    name_cb, false, NULL);
    VA_Resize(results2, 10);
    TEST_TRUE(batch, VA_Equals(results, (Obj*)results2),
              "same order regardless of queue size");
    DECREF(results2);
    DECREF(results);

    results = S_test_sorted_search(searcher, num_cb, 10,
                                   name_cb, true, NULL);
    results2 = S_test_sorted_search(searcher, num_cb, 30,
                                    name_cb, true, NULL);
    VA_Resize(results2, 10);
    TEST_TRUE(batch, VA_Equals(results, (Obj*)results2),
              "same order regardless of queue size (reverse sort)");
    DECREF(results2);
    DECREF(results);

    DECREF(searcher);

    // Add another seg to index.
    indexer = Indexer_new(schema, (Obj*)folder, NULL, 0);
    S_add_vehicle(indexer, carrot_cb, 0, 0, 1, land_cb, food_cb);
    Indexer_Commit(indexer);
    DECREF(indexer);

    searcher = IxSearcher_new((Obj*)folder);
    results = S_test_sorted_search(searcher, vehicle_cb, 100,
                                   name_cb, false, NULL);
    VA_Clear(wanted);
    VA_Push(wanted, INCREF(airplane_cb));
    VA_Push(wanted, INCREF(bike_cb));
    VA_Push(wanted, INCREF(car_cb));
    TEST_TRUE(batch, VA_Equals(results, (Obj*)wanted), "Multi-segment sort");
    DECREF(results);
    DECREF(searcher);

    DECREF(random_strings);
    DECREF(random_int32s);
    DECREF(random_int64s);
    DECREF(random_float32s);
    DECREF(random_float64s);

    DECREF(wanted);
    DECREF(schema);
    DECREF(folder);
}

void
TestSortSpec_run_tests(TestSortSpec *self) {
    TestBatch *batch = (TestBatch*)self;
    S_init_strings();
    test_sort_spec(batch);
    S_destroy_strings();
}


