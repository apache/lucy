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

#include "charmony.h"

#include "Clownfish/TestHarness/TestBatchRunner.h"
#include "Clownfish/TestHarness/TestUtils.h"
#include "Lucy/Test.h"
#include "Lucy/Test/Search/TestSortSpec.h"
#include "Lucy/Test/TestUtils.h"
#include "Lucy/Search/SortSpec.h"

#include "Clownfish/CharBuf.h"
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

static String *air_str;
static String *airplane_str;
static String *bike_str;
static String *car_str;
static String *carrot_str;
static String *cat_str;
static String *float32_str;
static String *float64_str;
static String *food_str;
static String *home_str;
static String *int32_str;
static String *int64_str;
static String *land_str;
static String *name_str;
static String *nope_str;
static String *num_str;
static String *random_str;
static String *sloth_str;
static String *speed_str;
static String *unknown_str;
static String *unused_str;
static String *vehicle_str;
static String *weight_str;

static String *random_float32s_str;
static String *random_float64s_str;
static String *random_int32s_str;
static String *random_int64s_str;

TestSortSpec*
TestSortSpec_new() {
    return (TestSortSpec*)Class_Make_Obj(TESTSORTSPEC);
}

static void
S_init_strings() {
    air_str      = Str_newf("air");
    airplane_str = Str_newf("airplane");
    bike_str     = Str_newf("bike");
    car_str      = Str_newf("car");
    carrot_str   = Str_newf("carrot");
    cat_str      = Str_newf("cat");
    float32_str  = Str_newf("float32");
    float64_str  = Str_newf("float64");
    food_str     = Str_newf("food");
    home_str     = Str_newf("home");
    int32_str    = Str_newf("int32");
    int64_str    = Str_newf("int64");
    land_str     = Str_newf("land");
    name_str     = Str_newf("name");
    nope_str     = Str_newf("nope");
    num_str      = Str_newf("num");
    random_str   = Str_newf("random");
    sloth_str    = Str_newf("sloth");
    speed_str    = Str_newf("speed");
    unknown_str  = Str_newf("unknown");
    unused_str   = Str_newf("unused");
    vehicle_str  = Str_newf("vehicle");
    weight_str   = Str_newf("weight");

    random_float32s_str = Str_newf("random_float32s");
    random_float64s_str = Str_newf("random_float64s");
    random_int32s_str   = Str_newf("random_int32s");
    random_int64s_str   = Str_newf("random_int64s");
}

static void
S_destroy_strings() {
    DECREF(air_str);
    DECREF(airplane_str);
    DECREF(bike_str);
    DECREF(car_str);
    DECREF(carrot_str);
    DECREF(cat_str);
    DECREF(float32_str);
    DECREF(float64_str);
    DECREF(food_str);
    DECREF(home_str);
    DECREF(int32_str);
    DECREF(int64_str);
    DECREF(land_str);
    DECREF(name_str);
    DECREF(nope_str);
    DECREF(num_str);
    DECREF(random_str);
    DECREF(sloth_str);
    DECREF(speed_str);
    DECREF(unknown_str);
    DECREF(unused_str);
    DECREF(vehicle_str);
    DECREF(weight_str);

    DECREF(random_float32s_str);
    DECREF(random_float64s_str);
    DECREF(random_int32s_str);
    DECREF(random_int64s_str);
}

TestReverseType*
TestReverseType_new() {
    TestReverseType *self = (TestReverseType*)Class_Make_Obj(TESTREVERSETYPE);
    return TestReverseType_init(self);
}

TestReverseType*
TestReverseType_init(TestReverseType *self) {
    return TestReverseType_init2(self, 1.0, false, true, true);
}

TestReverseType*
TestReverseType_init2(TestReverseType *self, float boost, bool indexed,
                      bool stored, bool sortable) {
    Int32Type_init2((Int32Type*)self, boost, indexed, stored, sortable);
    return self;
}

int32_t
TestReverseType_Compare_Values_IMP(TestReverseType *self, Obj *a, Obj *b) {
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

    Schema_Spec_Field(schema, name_str,    (FieldType*)string_type);
    Schema_Spec_Field(schema, speed_str,   (FieldType*)int32_type);
    Schema_Spec_Field(schema, sloth_str,   (FieldType*)reverse_type);
    Schema_Spec_Field(schema, weight_str,  (FieldType*)int32_type);
    Schema_Spec_Field(schema, int32_str,   (FieldType*)int32_type);
    Schema_Spec_Field(schema, int64_str,   (FieldType*)int64_type);
    Schema_Spec_Field(schema, float32_str, (FieldType*)float32_type);
    Schema_Spec_Field(schema, float64_str, (FieldType*)float64_type);
    Schema_Spec_Field(schema, home_str,    (FieldType*)string_type);
    Schema_Spec_Field(schema, cat_str,     (FieldType*)string_type);
    Schema_Spec_Field(schema, unused_str,  (FieldType*)string_type);
    Schema_Spec_Field(schema, nope_str,    (FieldType*)unsortable);

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
S_add_vehicle(Indexer *indexer, String *name, int32_t speed, int32_t sloth,
              int32_t weight, String *home, String *cat) {
    Doc       *doc   = Doc_new(NULL, 0);

    Doc_Store(doc, name_str, (Obj*)name);
    Doc_Store(doc, home_str, (Obj*)home);
    Doc_Store(doc, cat_str,  (Obj*)cat);

    String *string;
    string = Str_newf("%i32", speed);
    Doc_Store(doc, speed_str, (Obj*)string);
    DECREF(string);
    string = Str_newf("%i32", sloth);
    Doc_Store(doc, sloth_str, (Obj*)string);
    DECREF(string);
    string = Str_newf("%i32", weight);
    Doc_Store(doc, weight_str, (Obj*)string);
    DECREF(string);

    Indexer_Add_Doc(indexer, doc, 1.0f);

    DECREF(doc);
}

static void
S_add_doc(Indexer *indexer, Obj *name_obj, String *cat, String *field_name) {
    Doc *doc = Doc_new(NULL, 0);
    String *name = Obj_To_String(name_obj);
    Doc_Store(doc, name_str, (Obj*)name);
    Doc_Store(doc, cat_str,  (Obj*)cat);
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
    CharBuf *buf = CB_new(length);
    while (length--) {
        int32_t code_point = 'a' + rand() % ('z' - 'a' + 1);
        CB_Cat_Char(buf, code_point);
    }
    String *string = CB_Yield_String(buf);
    DECREF(buf);
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
    double d = CHY_U64_TO_DOUBLE(num) * (10.0 / UINT64_MAX);
    return (Obj*)Float32_new((float)d);
}

static Obj*
S_random_float64() {
    uint64_t num = TestUtils_random_u64();
    return (Obj*)Float64_new(CHY_U64_TO_DOUBLE(num) * (10.0 / UINT64_MAX));
}

static VArray*
S_add_random_objects(Indexer **indexer, Schema *schema, RAMFolder *folder,
                     random_generator_t rng, String *field_name,
                     String *cat) {
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
        String *string = Obj_To_String(obj);
        VA_Store(objects, i, (Obj*)string);
    }

    return objects;
}

static VArray*
S_test_sorted_search(IndexSearcher *searcher, String *query,
                     uint32_t num_wanted, ...) {
    VArray  *rules = VA_new(2);
    String *field;
    va_list  args;

    va_start(args, num_wanted);
    while (NULL != (field = va_arg(args, String*))) {
        int       reverse = va_arg(args, int);
        SortRule *rule    = SortRule_new(SortRule_FIELD, field, !!reverse);
        VA_Push(rules, (Obj*)rule);
    }
    va_end(args);
    SortRule *rule = SortRule_new(SortRule_DOC_ID, NULL, 0);
    VA_Push(rules, (Obj*)rule);
    SortSpec *spec = SortSpec_new(rules);

    Hits *hits = IxSearcher_Hits(searcher, (Obj*)query, 0, num_wanted, spec);

    VArray *results = VA_new(10);
    HitDoc *hit_doc;
    while (NULL != (hit_doc = Hits_Next(hits))) {
        String *name = (String*)HitDoc_Extract(hit_doc, name_str);
        VA_Push(results, (Obj*)Str_Clone((String*)name));
        DECREF(name);
        DECREF(hit_doc);
    }

    DECREF(hits);
    DECREF(spec);
    DECREF(rules);

    return results;
}

typedef struct SortContext {
    IndexSearcher *searcher;
    String        *sort_field;
} SortContext;

static void
S_attempt_sorted_search(void *context) {
    SortContext *sort_ctx = (SortContext*)context;
    VArray *results = S_test_sorted_search(sort_ctx->searcher, vehicle_str, 100,
                                           sort_ctx->sort_field, false, NULL);
    DECREF(results);
}

static void
test_sort_spec(TestBatchRunner *runner) {
    RAMFolder *folder  = RAMFolder_new(NULL);
    Schema    *schema  = S_create_schema();
    Indexer   *indexer = NULL;
    VArray    *wanted  = VA_new(10);
    VArray    *results;
    VArray    *results2;

    // First, add vehicles.
    S_refresh_indexer(&indexer, schema, folder);
    S_add_vehicle(indexer, airplane_str, 200, 200, 8000, air_str,  vehicle_str);
    S_add_vehicle(indexer, bike_str,      15,  15,   25, land_str, vehicle_str);
    S_add_vehicle(indexer, car_str,       70,  70, 3000, land_str, vehicle_str);

    // Add random objects.
    VArray *random_strings =
        S_add_random_objects(&indexer, schema, folder, S_random_string,
                             NULL, random_str);
    VArray *random_int32s =
        S_add_random_objects(&indexer, schema, folder, S_random_int32,
                             int32_str, random_int32s_str);
    VArray *random_int64s =
        S_add_random_objects(&indexer, schema, folder, S_random_int64,
                             int64_str, random_int64s_str);
    VArray *random_float32s =
        S_add_random_objects(&indexer, schema, folder, S_random_float32,
                             float32_str, random_float32s_str);
    VArray *random_float64s =
        S_add_random_objects(&indexer, schema, folder, S_random_float64,
                             float64_str, random_float64s_str);

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
        StackString *name = SSTR_WRAP_UTF8(name_buf, 2);
        S_add_doc(indexer, (Obj*)name, num_str, NULL);
        if (i % 10 == 0) {
            S_refresh_indexer(&indexer, schema, folder);
        }
    }
    FREEMEM(nums);

    Indexer_Commit(indexer);
    DECREF(indexer);

    // Start tests

    IndexSearcher *searcher = IxSearcher_new((Obj*)folder);

    results = S_test_sorted_search(searcher, vehicle_str, 100,
                                   name_str, false, NULL);
    VA_Clear(wanted);
    VA_Push(wanted, INCREF(airplane_str));
    VA_Push(wanted, INCREF(bike_str));
    VA_Push(wanted, INCREF(car_str));
    TEST_TRUE(runner, VA_Equals(results, (Obj*)wanted), "sort by one criteria");
    DECREF(results);

#ifdef LUCY_VALGRIND
    SKIP(runner, "known leaks");
    SKIP(runner, "known leaks");
#else
    Err *error;
    SortContext sort_ctx;
    sort_ctx.searcher = searcher;

    sort_ctx.sort_field = nope_str;
    error = Err_trap(S_attempt_sorted_search, &sort_ctx);
    TEST_TRUE(runner, error != NULL
              && Err_Is_A(error, ERR)
              && Str_Find_Utf8(Err_Get_Mess(error), "sortable", 8) != -1,
              "sorting on a non-sortable field throws an error");
    DECREF(error);

    sort_ctx.sort_field = unknown_str;
    error = Err_trap(S_attempt_sorted_search, &sort_ctx);
    TEST_TRUE(runner, error != NULL
              && Err_Is_A(error, ERR)
              && Str_Find_Utf8(Err_Get_Mess(error), "sortable", 8) != -1,
              "sorting on an unknown field throws an error");
    DECREF(error);
#endif

    results = S_test_sorted_search(searcher, vehicle_str, 100,
                                   weight_str, false, NULL);
    VA_Clear(wanted);
    VA_Push(wanted, INCREF(bike_str));
    VA_Push(wanted, INCREF(car_str));
    VA_Push(wanted, INCREF(airplane_str));
    TEST_TRUE(runner, VA_Equals(results, (Obj*)wanted), "sort by one criteria");
    DECREF(results);

    results = S_test_sorted_search(searcher, vehicle_str, 100,
                                   name_str, true, NULL);
    VA_Clear(wanted);
    VA_Push(wanted, INCREF(car_str));
    VA_Push(wanted, INCREF(bike_str));
    VA_Push(wanted, INCREF(airplane_str));
    TEST_TRUE(runner, VA_Equals(results, (Obj*)wanted), "reverse sort");
    DECREF(results);

    results = S_test_sorted_search(searcher, vehicle_str, 100,
                                   home_str, false, name_str, false, NULL);
    VA_Clear(wanted);
    VA_Push(wanted, INCREF(airplane_str));
    VA_Push(wanted, INCREF(bike_str));
    VA_Push(wanted, INCREF(car_str));
    TEST_TRUE(runner, VA_Equals(results, (Obj*)wanted), "multiple criteria");
    DECREF(results);

    results = S_test_sorted_search(searcher, vehicle_str, 100,
                                   home_str, false, name_str, true, NULL);
    VA_Clear(wanted);
    VA_Push(wanted, INCREF(airplane_str));
    VA_Push(wanted, INCREF(car_str));
    VA_Push(wanted, INCREF(bike_str));
    TEST_TRUE(runner, VA_Equals(results, (Obj*)wanted),
              "multiple criteria with reverse");
    DECREF(results);

    results = S_test_sorted_search(searcher, vehicle_str, 100,
                                   speed_str, true, NULL);
    results2 = S_test_sorted_search(searcher, vehicle_str, 100,
                                    sloth_str, false, NULL);
    TEST_TRUE(runner, VA_Equals(results, (Obj*)results2),
              "FieldType_Compare_Values");
    DECREF(results2);
    DECREF(results);

    results = S_test_sorted_search(searcher, random_str, 100,
                                   name_str, false, NULL);
    TEST_TRUE(runner, VA_Equals(results, (Obj*)random_strings),
              "random strings");
    DECREF(results);

    results = S_test_sorted_search(searcher, random_int32s_str, 100,
                                   int32_str, false, NULL);
    TEST_TRUE(runner, VA_Equals(results, (Obj*)random_int32s),
              "int32");
    DECREF(results);

    results = S_test_sorted_search(searcher, random_int64s_str, 100,
                                   int64_str, false, NULL);
    TEST_TRUE(runner, VA_Equals(results, (Obj*)random_int64s),
              "int64");
    DECREF(results);

    results = S_test_sorted_search(searcher, random_float32s_str, 100,
                                   float32_str, false, NULL);
    TEST_TRUE(runner, VA_Equals(results, (Obj*)random_float32s),
              "float32");
    DECREF(results);

    results = S_test_sorted_search(searcher, random_float64s_str, 100,
                                   float64_str, false, NULL);
    TEST_TRUE(runner, VA_Equals(results, (Obj*)random_float64s),
              "float64");
    DECREF(results);

    String *bbbcca_str = Str_newf("bike bike bike car car airplane");
    results = S_test_sorted_search(searcher, bbbcca_str, 100,
                                   unused_str, false, NULL);
    VA_Clear(wanted);
    VA_Push(wanted, INCREF(airplane_str));
    VA_Push(wanted, INCREF(bike_str));
    VA_Push(wanted, INCREF(car_str));
    TEST_TRUE(runner, VA_Equals(results, (Obj*)wanted),
              "sorting on field with no values sorts by doc id");
    DECREF(results);
    DECREF(bbbcca_str);

    String *nn_str        = Str_newf("99");
    String *nn_or_car_str = Str_newf("99 OR car");
    results = S_test_sorted_search(searcher, nn_or_car_str, 10,
                                   speed_str, false, NULL);
    VA_Clear(wanted);
    VA_Push(wanted, INCREF(car_str));
    VA_Push(wanted, INCREF(nn_str));
    TEST_TRUE(runner, VA_Equals(results, (Obj*)wanted),
              "doc with NULL value sorts last");
    DECREF(results);
    DECREF(nn_str);
    DECREF(nn_or_car_str);

    results = S_test_sorted_search(searcher, num_str, 10,
                                   name_str, false, NULL);
    results2 = S_test_sorted_search(searcher, num_str, 30,
                                    name_str, false, NULL);
    VA_Resize(results2, 10);
    TEST_TRUE(runner, VA_Equals(results, (Obj*)results2),
              "same order regardless of queue size");
    DECREF(results2);
    DECREF(results);

    results = S_test_sorted_search(searcher, num_str, 10,
                                   name_str, true, NULL);
    results2 = S_test_sorted_search(searcher, num_str, 30,
                                    name_str, true, NULL);
    VA_Resize(results2, 10);
    TEST_TRUE(runner, VA_Equals(results, (Obj*)results2),
              "same order regardless of queue size (reverse sort)");
    DECREF(results2);
    DECREF(results);

    DECREF(searcher);

    // Add another seg to index.
    indexer = Indexer_new(schema, (Obj*)folder, NULL, 0);
    S_add_vehicle(indexer, carrot_str, 0, 0, 1, land_str, food_str);
    Indexer_Commit(indexer);
    DECREF(indexer);

    searcher = IxSearcher_new((Obj*)folder);
    results = S_test_sorted_search(searcher, vehicle_str, 100,
                                   name_str, false, NULL);
    VA_Clear(wanted);
    VA_Push(wanted, INCREF(airplane_str));
    VA_Push(wanted, INCREF(bike_str));
    VA_Push(wanted, INCREF(car_str));
    TEST_TRUE(runner, VA_Equals(results, (Obj*)wanted), "Multi-segment sort");
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
TestSortSpec_Run_IMP(TestSortSpec *self, TestBatchRunner *runner) {
    TestBatchRunner_Plan(runner, (TestBatch*)self, 18);
    S_init_strings();
    test_sort_spec(runner);
    S_destroy_strings();
}


