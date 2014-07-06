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
#define TESTLUCY_USE_SHORT_NAMES
#include "Lucy/Util/ToolSet.h"

#include "Clownfish/TestHarness/TestBatchRunner.h"
#include "Lucy/Test.h"
#include "Lucy/Test/Util/TestJson.h"
#include "Lucy/Util/Json.h"
#include "Lucy/Store/FileHandle.h"
#include "Lucy/Store/RAMFolder.h"

TestJson*
TestJson_new() {
    return (TestJson*)Class_Make_Obj(TESTJSON);
}

// Create a test data structure including at least one each of Hash, VArray,
// and String.
static Obj*
S_make_dump() {
    Hash *dump = Hash_new(0);
    Hash_Store_Utf8(dump, "foo", 3, (Obj*)Str_newf("foo"));
    Hash_Store_Utf8(dump, "stuff", 5, (Obj*)VA_new(0));
    return (Obj*)dump;
}

static void
test_tolerance(TestBatchRunner *runner) {
    String *foo = Str_newf("foo");
    String *not_json = Json_to_json((Obj*)foo);
    TEST_TRUE(runner, not_json == NULL,
              "to_json returns NULL when fed invalid data type");
    TEST_TRUE(runner, Err_get_error() != NULL,
              "to_json sets Err_error when fed invalid data type");
    DECREF(foo);
}

// Test escapes for control characters ASCII 0-31.
static const char* control_escapes[] = {
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
static const char* quote_escapes_source[] = {
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
static const char* quote_escapes_json[] = {
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
test_escapes(TestBatchRunner *runner) {
    for (int i = 0; control_escapes[i] != NULL; i++) {
        String     *string  = Str_new_from_char(i);
        const char *escaped = control_escapes[i];
        String     *json    = Json_to_json((Obj*)string);
        String     *trimmed = Str_Trim(json);
        String     *decoded = (String*)Json_from_json(json);

        String *json_wanted = Str_newf("\"%s\"", escaped);
        TEST_TRUE(runner, Str_Equals(json_wanted, (Obj*)trimmed),
                  "encode control escape: %s", escaped);

        TEST_TRUE(runner, decoded != NULL && Str_Equals(string, (Obj*)decoded),
                  "decode control escape: %s", escaped);

        DECREF(string);
        DECREF(json);
        DECREF(trimmed);
        DECREF(decoded);
        DECREF(json_wanted);
    }

    for (int i = 0; quote_escapes_source[i] != NULL; i++) {
        const char *source  = quote_escapes_source[i];
        const char *escaped = quote_escapes_json[i];
        String *string  = Str_new_from_utf8(source, strlen(source));
        String *json    = Json_to_json((Obj*)string);
        String *trimmed = Str_Trim(json);
        String *decoded = (String*)Json_from_json(json);

        String *json_wanted = Str_newf("\"%s\"", escaped);
        TEST_TRUE(runner, Str_Equals(json_wanted, (Obj*)trimmed),
                  "encode quote/backslash escapes: %s", source);

        TEST_TRUE(runner, decoded != NULL && Str_Equals(string, (Obj*)decoded),
                  "decode quote/backslash escapes: %s", source);

        DECREF(string);
        DECREF(json);
        DECREF(trimmed);
        DECREF(decoded);
        DECREF(json_wanted);
    }
}

static void
test_numbers(TestBatchRunner *runner) {
    Integer64 *i64     = Int64_new(33);
    String    *json    = Json_to_json((Obj*)i64);
    String    *trimmed = Str_Trim(json);
    TEST_TRUE(runner, Str_Equals_Utf8(trimmed, "33", 2), "Integer");
    DECREF(json);
    DECREF(trimmed);

    Float64 *f64 = Float64_new(33.33);
    json = Json_to_json((Obj*)f64);
    if (json) {
        double value = Str_To_F64(json);
        double diff = 33.33 - value;
        if (diff < 0.0) { diff = 0.0 - diff; }
        TEST_TRUE(runner, diff < 0.0001, "Float");
        DECREF(json);
    }
    else {
        FAIL(runner, "Float conversion to  json  failed.");
    }

    DECREF(i64);
    DECREF(f64);
}

static void
test_to_and_from(TestBatchRunner *runner) {
    Obj *dump = S_make_dump();
    String *json = Json_to_json(dump);
    Obj *got = Json_from_json(json);
    TEST_TRUE(runner, got != NULL && Obj_Equals(dump, got),
              "Round trip through to_json and from_json");
    DECREF(dump);
    DECREF(json);
    DECREF(got);
}

static void
test_spew_and_slurp(TestBatchRunner *runner) {
    Obj *dump = S_make_dump();
    Folder *folder = (Folder*)RAMFolder_new(NULL);

    String *foo = (String*)SSTR_WRAP_UTF8("foo", 3);
    bool result = Json_spew_json(dump, folder, foo);
    TEST_TRUE(runner, result, "spew_json returns true on success");
    TEST_TRUE(runner, Folder_Exists(folder, foo),
              "spew_json wrote file");

    Obj *got = Json_slurp_json(folder, foo);
    TEST_TRUE(runner, got && Obj_Equals(dump, got),
              "Round trip through spew_json and slurp_json");
    DECREF(got);

    Err_set_error(NULL);
    result = Json_spew_json(dump, folder, foo);
    TEST_FALSE(runner, result, "Can't spew_json when file exists");
    TEST_TRUE(runner, Err_get_error() != NULL,
              "Failed spew_json sets Err_error");

    Err_set_error(NULL);
    String *bar = (String*)SSTR_WRAP_UTF8("bar", 3);
    got = Json_slurp_json(folder, bar);
    TEST_TRUE(runner, got == NULL,
              "slurp_json returns NULL when file doesn't exist");
    TEST_TRUE(runner, Err_get_error() != NULL,
              "Failed slurp_json sets Err_error");

    String *boffo = (String*)SSTR_WRAP_UTF8("boffo", 5);

    FileHandle *fh
        = Folder_Open_FileHandle(folder, boffo, FH_CREATE | FH_WRITE_ONLY);
    FH_Write(fh, "garbage", 7);
    DECREF(fh);

    Err_set_error(NULL);
    got = Json_slurp_json(folder, boffo);
    TEST_TRUE(runner, got == NULL,
              "slurp_json returns NULL when file doesn't contain valid JSON");
    TEST_TRUE(runner, Err_get_error() != NULL,
              "Failed slurp_json sets Err_error");
    DECREF(got);

    DECREF(dump);
    DECREF(folder);
}

static void
S_verify_bad_syntax(TestBatchRunner *runner, const char *bad, const char *mess) {
    StackString *has_errors = SSTR_WRAP_UTF8(bad, strlen(bad));
    Err_set_error(NULL);
    Obj *not_json = Json_from_json((String*)has_errors);
    TEST_TRUE(runner, not_json == NULL, "from_json returns NULL: %s", mess);
    TEST_TRUE(runner, Err_get_error() != NULL, "from_json sets Err_error: %s",
              mess);
}

static void
test_syntax_errors(TestBatchRunner *runner) {
    S_verify_bad_syntax(runner, "[", "unclosed left bracket");
    S_verify_bad_syntax(runner, "]", "unopened right bracket");
    S_verify_bad_syntax(runner, "{", "unclosed left curly");
    S_verify_bad_syntax(runner, "}", "unopened right curly");
    S_verify_bad_syntax(runner, "{}[]", "two top-level objects");
    S_verify_bad_syntax(runner, "[1 \"foo\"]", "missing comma in array");
    S_verify_bad_syntax(runner, "[1, \"foo\",]", "extra comma in array");
    S_verify_bad_syntax(runner, "{\"1\":1 \"2\":2}", "missing comma in hash");
    S_verify_bad_syntax(runner, "{\"1\":1,\"2\":2,}", "extra comma in hash");
    S_verify_bad_syntax(runner, "\"1", "unterminated string");
    // Tolerated by strtod().
    // S_verify_bad_syntax(runner, "1. ", "float missing fraction");
    // S_verify_bad_syntax(runner, "-.3 ", "Number missing integral part");
    S_verify_bad_syntax(runner, "-. ", "Number missing any digits");
    S_verify_bad_syntax(runner, "+1.0 ", "float with prepended plus");
    S_verify_bad_syntax(runner, "\"\\g\"", "invalid char escape");
    S_verify_bad_syntax(runner, "\"\\uAAAZ\"", "invalid \\u escape");
}

static void
S_round_trip_integer(TestBatchRunner *runner, int64_t value) {
    Integer64 *num = Int64_new(value);
    VArray *array = VA_new(1);
    VA_Store(array, 0, (Obj*)num);
    String *json = Json_to_json((Obj*)array);
    Obj *dump = Json_from_json(json);
    TEST_TRUE(runner, VA_Equals(array, dump), "Round trip integer %ld",
              (long)value);
    DECREF(dump);
    DECREF(json);
    DECREF(array);
}

static void
test_integers(TestBatchRunner *runner) {
    S_round_trip_integer(runner, 0);
    S_round_trip_integer(runner, -1);
    S_round_trip_integer(runner, -1000000);
    S_round_trip_integer(runner, 1000000);
}

static void
S_round_trip_float(TestBatchRunner *runner, double value, double max_diff) {
    Float64 *num = Float64_new(value);
    VArray *array = VA_new(1);
    VA_Store(array, 0, (Obj*)num);
    String *json = Json_to_json((Obj*)array);
    Obj *dump = CERTIFY(Json_from_json(json), VARRAY);
    Float64 *got = (Float64*)CERTIFY(VA_Fetch((VArray*)dump, 0), FLOAT64);
    double diff = Float64_Get_Value(num) - Float64_Get_Value(got);
    if (diff < 0) { diff = 0 - diff; }
    TEST_TRUE(runner, diff <= max_diff, "Round trip float %f", value);
    DECREF(dump);
    DECREF(json);
    DECREF(array);
}

static void
test_floats(TestBatchRunner *runner) {
    S_round_trip_float(runner, 0.0, 0.0);
    S_round_trip_float(runner, 0.1, 0.00001);
    S_round_trip_float(runner, -0.1, 0.00001);
    S_round_trip_float(runner, 1000000.5, 1.0);
    S_round_trip_float(runner, -1000000.5, 1.0);
}

static void
test_max_depth(TestBatchRunner *runner) {
    Hash *circular = Hash_new(0);
    Hash_Store_Utf8(circular, "circular", 8, INCREF(circular));
    Err_set_error(NULL);
    String *not_json = Json_to_json((Obj*)circular);
    TEST_TRUE(runner, not_json == NULL,
              "to_json returns NULL when fed recursing data");
    TEST_TRUE(runner, Err_get_error() != NULL,
              "to_json sets Err_error when fed recursing data");
    DECREF(Hash_Delete_Utf8(circular, "circular", 8));
    DECREF(circular);
}

static void
test_illegal_keys(TestBatchRunner *runner) {
    Hash *hash = Hash_new(0);
    Float64 *key = Float64_new(1.1);
    Hash_Store(hash, (Obj*)key, (Obj*)Str_newf("blah"));
    Err_set_error(NULL);
    String *not_json = Json_to_json((Obj*)hash);
    TEST_TRUE(runner, not_json == NULL,
              "to_json returns NULL when fed an illegal key");
    TEST_TRUE(runner, Err_get_error() != NULL,
              "to_json sets Err_error when fed an illegal key");
    DECREF(key);
    DECREF(hash);
}

void
TestJson_Run_IMP(TestJson *self, TestBatchRunner *runner) {
    uint32_t num_tests = 107;
#ifndef LUCY_VALGRIND
    num_tests += 28; // FIXME: syntax errors leak memory.
#endif
    TestBatchRunner_Plan(runner, (TestBatch*)self, num_tests);

    // Test tolerance, then liberalize for testing.
    test_tolerance(runner);
    Json_set_tolerant(true);

    test_to_and_from(runner);
    test_escapes(runner);
    test_numbers(runner);
    test_spew_and_slurp(runner);
    test_integers(runner);
    test_floats(runner);
    test_max_depth(runner);
    test_illegal_keys(runner);

#ifndef LUCY_VALGRIND
    test_syntax_errors(runner);
#endif
}

