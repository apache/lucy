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

#include <string.h>
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Test.h"
#include "Lucy/Test/Util/TestJson.h"
#include "Lucy/Util/Json.h"
#include "Lucy/Store/FileHandle.h"
#include "Lucy/Store/RAMFolder.h"

// Create a test data structure including at least one each of Hash, VArray,
// and CharBuf.
static Obj*
S_make_dump() {
    Hash *dump = Hash_new(0);
    Hash_Store_Str(dump, "foo", 3, (Obj*)CB_newf("foo"));
    Hash_Store_Str(dump, "stuff", 5, (Obj*)VA_new(0));
    return (Obj*)dump;
}

static void
test_tolerance(TestBatch *batch) {
    CharBuf *foo = CB_newf("foo");
    CharBuf *not_json = Json_to_json((Obj*)foo);
    TEST_TRUE(batch, not_json == NULL,
              "to_json returns NULL when fed invalid data type");
    TEST_TRUE(batch, Err_get_error() != NULL,
              "to_json sets Err_error when fed invalid data type");
    DECREF(foo);
}

// Test escapes for control characters ASCII 0-31.
static char* control_escapes[] = {
    "\\u0000",
    "\\u0001",
    "\\u0002",
    "\\u0003",
    "\\u0004",
    "\\u0005",
    "\\u0006",
    "\\u0007",
    "\\b",
    "\\t",
    "\\n",
    "\\u000b",
    "\\f",
    "\\r",
    "\\u000e",
    "\\u000f",
    "\\u0010",
    "\\u0011",
    "\\u0012",
    "\\u0013",
    "\\u0014",
    "\\u0015",
    "\\u0016",
    "\\u0017",
    "\\u0018",
    "\\u0019",
    "\\u001a",
    "\\u001b",
    "\\u001c",
    "\\u001d",
    "\\u001e",
    "\\u001f",
    NULL
};

// Test quote and backslash escape in isolation, then in context.
static char* quote_escapes_source[] = {
    "\"",
    "\\",
    "abc\"",
    "abc\\",
    "\"xyz",
    "\\xyz",
    "\\\"",
    "\"\\",
    NULL
};
static char* quote_escapes_json[] = {
    "\\\"",
    "\\\\",
    "abc\\\"",
    "abc\\\\",
    "\\\"xyz",
    "\\\\xyz",
    "\\\\\\\"",
    "\\\"\\\\",
    NULL
};

static void
test_escapes(TestBatch *batch) {
    CharBuf *string      = CB_new(10);
    CharBuf *json_wanted = CB_new(10);

    for (int i = 0; control_escapes[i] != NULL; i++) {
        CB_Truncate(string, 0);
        CB_Cat_Char(string, i);
        char    *escaped = control_escapes[i];
        CharBuf *json    = Json_to_json((Obj*)string);
        CharBuf *decoded = (CharBuf*)Json_from_json(json);

        CB_setf(json_wanted, "\"%s\"", escaped);
        CB_Trim(json);
        TEST_TRUE(batch, json != NULL && CB_Equals(json_wanted, (Obj*)json),
                  "encode control escape: %s", escaped);

        TEST_TRUE(batch, decoded != NULL && CB_Equals(string, (Obj*)decoded),
                  "decode control escape: %s", escaped);

        DECREF(json);
        DECREF(decoded);
    }

    for (int i = 0; quote_escapes_source[i] != NULL; i++) {
        char *source  = quote_escapes_source[i];
        char *escaped = quote_escapes_json[i];
        CB_setf(string, source, strlen(source));
        CharBuf *json    = Json_to_json((Obj*)string);
        CharBuf *decoded = (CharBuf*)Json_from_json(json);

        CB_setf(json_wanted, "\"%s\"", escaped);
        CB_Trim(json);
        TEST_TRUE(batch, json != NULL && CB_Equals(json_wanted, (Obj*)json),
                  "encode quote/backslash escapes: %s", source);

        TEST_TRUE(batch, decoded != NULL && CB_Equals(string, (Obj*)decoded),
                  "decode quote/backslash escapes: %s", source);

        DECREF(json);
        DECREF(decoded);
    }

    DECREF(json_wanted);
    DECREF(string);
}

static void
test_numbers(TestBatch *batch) {
    Integer64 *i64  = Int64_new(33);
    CharBuf   *json = Json_to_json((Obj*)i64);
    CB_Trim(json);
    TEST_TRUE(batch, json && CB_Equals_Str(json, "33", 2), "Integer");
    DECREF(json);

    Float64 *f64 = Float64_new(33.33);
    json = Json_to_json((Obj*)f64);
    if (json) {
        double value = CB_To_F64(json);
        double diff = 33.33 - value;
        if (diff < 0.0) { diff = 0.0 - diff; }
        TEST_TRUE(batch, diff < 0.0001, "Float");
        DECREF(json);
    }
    else {
        FAIL(batch, "Float conversion to  json  failed.");
    }

    DECREF(i64);
    DECREF(f64);
}

static void
test_to_and_from(TestBatch *batch) {
    Obj *dump = S_make_dump();
    CharBuf *json = Json_to_json(dump);
    Obj *got = Json_from_json(json);
    TEST_TRUE(batch, got != NULL && Obj_Equals(dump, got),
              "Round trip through to_json and from_json");
    DECREF(dump);
    DECREF(json);
    DECREF(got);
}

static void
test_spew_and_slurp(TestBatch *batch) {
    Obj *dump = S_make_dump();
    Folder *folder = (Folder*)RAMFolder_new(NULL);

    CharBuf *foo = (CharBuf*)ZCB_WRAP_STR("foo", 3);
    bool_t result = Json_spew_json(dump, folder, foo);
    TEST_TRUE(batch, result, "spew_json returns true on success");
    TEST_TRUE(batch, Folder_Exists(folder, foo),
              "spew_json wrote file");

    Obj *got = Json_slurp_json(folder, foo);
    TEST_TRUE(batch, got && Obj_Equals(dump, got),
              "Round trip through spew_json and slurp_json");
    DECREF(got);

    Err_set_error(NULL);
    result = Json_spew_json(dump, folder, foo);
    TEST_FALSE(batch, result, "Can't spew_json when file exists");
    TEST_TRUE(batch, Err_get_error() != NULL,
              "Failed spew_json sets Err_error");

    Err_set_error(NULL);
    CharBuf *bar = (CharBuf*)ZCB_WRAP_STR("bar", 3);
    got = Json_slurp_json(folder, bar);
    TEST_TRUE(batch, got == NULL,
              "slurp_json returns NULL when file doesn't exist");
    TEST_TRUE(batch, Err_get_error() != NULL,
              "Failed slurp_json sets Err_error");

    CharBuf *boffo = (CharBuf*)ZCB_WRAP_STR("boffo", 5);

    FileHandle *fh
        = Folder_Open_FileHandle(folder, boffo, FH_CREATE | FH_WRITE_ONLY);
    FH_Write(fh, "garbage", 7);
    DECREF(fh);

    Err_set_error(NULL);
    got = Json_slurp_json(folder, boffo);
    TEST_TRUE(batch, got == NULL,
              "slurp_json returns NULL when file doesn't contain valid JSON");
    TEST_TRUE(batch, Err_get_error() != NULL,
              "Failed slurp_json sets Err_error");
    DECREF(got);

    DECREF(dump);
    DECREF(folder);
}

static void
S_verify_bad_syntax(TestBatch *batch, const char *bad, const char *mess) {
    ZombieCharBuf *has_errors = ZCB_WRAP_STR(bad, strlen(bad));
    Err_set_error(NULL);
    Obj *not_json = Json_from_json((CharBuf*)has_errors);
    TEST_TRUE(batch, not_json == NULL, "from_json returns NULL: %s", mess);
    TEST_TRUE(batch, Err_get_error() != NULL, "from_json sets Err_error: %s",
              mess);
}

static void
test_syntax_errors(TestBatch *batch) {
    S_verify_bad_syntax(batch, "[", "unclosed left bracket");
    S_verify_bad_syntax(batch, "]", "unopened right bracket");
    S_verify_bad_syntax(batch, "{", "unclosed left curly");
    S_verify_bad_syntax(batch, "}", "unopened right curly");
    S_verify_bad_syntax(batch, "{}[]", "two top-level objects");
    S_verify_bad_syntax(batch, "[1 \"foo\"]", "missing comma in array");
    S_verify_bad_syntax(batch, "[1, \"foo\",]", "extra comma in array");
    S_verify_bad_syntax(batch, "{\"1\":1 \"2\":2}", "missing comma in hash");
    S_verify_bad_syntax(batch, "{\"1\":1,\"2\":2,}", "extra comma in hash");
    S_verify_bad_syntax(batch, "\"1", "unterminated string");
    // Tolerated by strtod().
    // S_verify_bad_syntax(batch, "1. ", "float missing fraction");
    // S_verify_bad_syntax(batch, "-.3 ", "Number missing integral part");
    S_verify_bad_syntax(batch, "-. ", "Number missing any digits");
    S_verify_bad_syntax(batch, "+1.0 ", "float with prepended plus");
    S_verify_bad_syntax(batch, "\"\\g\"", "invalid char escape");
    S_verify_bad_syntax(batch, "\"\\uAAAZ\"", "invalid \\u escape");
}

static void
S_round_trip_integer(TestBatch *batch, int64_t value) {
    Integer64 *num = Int64_new(value);
    VArray *array = VA_new(1);
    VA_Store(array, 0, (Obj*)num);
    CharBuf *json = Json_to_json((Obj*)array);
    Obj *dump = Json_from_json(json);
    TEST_TRUE(batch, VA_Equals(array, dump), "Round trip integer %ld",
              (long)value);
    DECREF(dump);
    DECREF(json);
    DECREF(array);
}

static void
test_integers(TestBatch *batch) {
    S_round_trip_integer(batch, 0);
    S_round_trip_integer(batch, -1);
    S_round_trip_integer(batch, -1000000);
    S_round_trip_integer(batch, 1000000);
}

static void
S_round_trip_float(TestBatch *batch, double value, double max_diff) {
    Float64 *num = Float64_new(value);
    VArray *array = VA_new(1);
    VA_Store(array, 0, (Obj*)num);
    CharBuf *json = Json_to_json((Obj*)array);
    Obj *dump = CERTIFY(Json_from_json(json), VARRAY);
    Float64 *got = (Float64*)CERTIFY(VA_Fetch((VArray*)dump, 0), FLOAT64);
    double diff = Float64_Get_Value(num) - Float64_Get_Value(got);
    if (diff < 0) { diff = 0 - diff; }
    TEST_TRUE(batch, diff <= max_diff, "Round trip float %f", value);
    DECREF(dump);
    DECREF(json);
    DECREF(array);
}

static void
test_floats(TestBatch *batch) {
    S_round_trip_float(batch, 0.0, 0.0);
    S_round_trip_float(batch, 0.1, 0.00001);
    S_round_trip_float(batch, -0.1, 0.00001);
    S_round_trip_float(batch, 1000000.5, 1.0);
    S_round_trip_float(batch, -1000000.5, 1.0);
}

static void
test_max_depth(TestBatch *batch) {
    Hash *circular = Hash_new(0);
    Hash_Store_Str(circular, "circular", 8, INCREF(circular));
    Err_set_error(NULL);
    CharBuf *not_json = Json_to_json((Obj*)circular);
    TEST_TRUE(batch, not_json == NULL,
              "to_json returns NULL when fed recursing data");
    TEST_TRUE(batch, Err_get_error() != NULL,
              "to_json sets Err_error when fed recursing data");
    DECREF(Hash_Delete_Str(circular, "circular", 8));
    DECREF(circular);
}

static void
test_illegal_keys(TestBatch *batch) {
    Hash *hash = Hash_new(0);
    Float64 *key = Float64_new(1.1);
    Hash_Store(hash, (Obj*)key, (Obj*)CB_newf("blah"));
    Err_set_error(NULL);
    CharBuf *not_json = Json_to_json((Obj*)hash);
    TEST_TRUE(batch, not_json == NULL,
              "to_json returns NULL when fed an illegal key");
    TEST_TRUE(batch, Err_get_error() != NULL,
              "to_json sets Err_error when fed an illegal key");
    DECREF(key);
    DECREF(hash);
}

void
TestJson_run_tests() {
    int num_tests = 107;
#ifndef LUCY_VALGRIND
    num_tests += 28; // FIXME: syntax errors leak memory.
#endif
    TestBatch *batch = TestBatch_new(num_tests);
    TestBatch_Plan(batch);

    // Test tolerance, then liberalize for testing.
    test_tolerance(batch);
    Json_set_tolerant(true);

    test_to_and_from(batch);
    test_escapes(batch);
    test_numbers(batch);
    test_spew_and_slurp(batch);
    test_integers(batch);
    test_floats(batch);
    test_max_depth(batch);
    test_illegal_keys(batch);

#ifndef LUCY_VALGRIND
    test_syntax_errors(batch);
#endif

    DECREF(batch);
}

