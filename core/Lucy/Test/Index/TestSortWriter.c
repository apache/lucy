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

#define TESTLUCY_USE_SHORT_NAMES
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Test/Index/TestSortWriter.h"

#include "Clownfish/TestHarness/TestBatchRunner.h"
#include "Lucy/Analysis/StandardTokenizer.h"
#include "Lucy/Document/Doc.h"
#include "Lucy/Document/HitDoc.h"
#include "Lucy/Index/DocReader.h"
#include "Lucy/Index/Indexer.h"
#include "Lucy/Index/IndexManager.h"
#include "Lucy/Index/PolyReader.h"
#include "Lucy/Index/Segment.h"
#include "Lucy/Index/SegReader.h"
#include "Lucy/Index/SortCache.h"
#include "Lucy/Index/SortReader.h"
#include "Lucy/Index/SortWriter.h"
#include "Lucy/Plan/FullTextType.h"
#include "Lucy/Plan/Schema.h"
#include "Lucy/Plan/StringType.h"
#include "Lucy/Store/RAMFolder.h"

static String *name_str;
static String *speed_str;
static String *weight_str;
static String *home_str;
static String *cat_str;
static String *wheels_str;
static String *unused_str;
static String *nope_str;

TestSortWriter*
TestSortWriter_new() {
    return (TestSortWriter*)Class_Make_Obj(TESTSORTWRITER);
}

static void
S_init_strings() {
    name_str   = Str_newf("name");
    speed_str  = Str_newf("speed");
    weight_str = Str_newf("weight");
    home_str   = Str_newf("home");
    cat_str    = Str_newf("cat");
    wheels_str = Str_newf("wheels");
    unused_str = Str_newf("unused");
    nope_str   = Str_newf("nope");
}

static void
S_destroy_strings() {
    DECREF(name_str);
    DECREF(speed_str);
    DECREF(weight_str);
    DECREF(home_str);
    DECREF(cat_str);
    DECREF(wheels_str);
    DECREF(unused_str);
    DECREF(nope_str);
}

static Schema*
S_create_schema() {
    Schema *schema = Schema_new();

    StandardTokenizer *tokenizer = StandardTokenizer_new();
    FullTextType *full_text_type = FullTextType_new((Analyzer*)tokenizer);
    FullTextType_Set_Sortable(full_text_type, true);

    StringType *string_type = StringType_new();
    StringType_Set_Sortable(string_type, true);

    StringType *unsortable = StringType_new();

    Schema_Spec_Field(schema, name_str,   (FieldType*)full_text_type);
    Schema_Spec_Field(schema, speed_str,  (FieldType*)string_type);
    Schema_Spec_Field(schema, weight_str, (FieldType*)string_type);
    Schema_Spec_Field(schema, home_str,   (FieldType*)string_type);
    Schema_Spec_Field(schema, cat_str,    (FieldType*)string_type);
    Schema_Spec_Field(schema, wheels_str, (FieldType*)string_type);
    Schema_Spec_Field(schema, unused_str, (FieldType*)string_type);
    Schema_Spec_Field(schema, nope_str,   (FieldType*)unsortable);

    DECREF(unsortable);
    DECREF(string_type);
    DECREF(full_text_type);
    DECREF(tokenizer);

    return schema;
}

static void
S_store_field(Doc *doc, String *field, const char *value) {
    if (value) {
        StackString *string = SSTR_WRAP_UTF8(value, strlen(value));
        Doc_Store(doc, field, (Obj*)string);
    }
}

static void
S_add_doc(Indexer *indexer, const char *name, const char *speed,
              const char *weight, const char *home, const char *wheels,
              const char *nope) {
    Doc *doc   = Doc_new(NULL, 0);

    S_store_field(doc, name_str,   name);
    S_store_field(doc, speed_str,  speed);
    S_store_field(doc, weight_str, weight);
    S_store_field(doc, home_str,   home);
    S_store_field(doc, cat_str,    "vehicle");
    S_store_field(doc, wheels_str, wheels);
    S_store_field(doc, nope_str,   nope);

    Indexer_Add_Doc(indexer, doc, 1.0f);

    DECREF(doc);
}

static void
S_test_sort_cache(TestBatchRunner *runner, RAMFolder *folder,
                  SegReader *seg_reader, const char *gen, bool is_used,
                  String *field) {
    Segment *segment   = SegReader_Get_Segment(seg_reader);
    int32_t  field_num = Seg_Field_Num(segment, field);
    String  *filename  = Str_newf("seg_%s/sort-%i32.ord", gen, field_num);
    if (is_used) {
        TEST_TRUE(runner, RAMFolder_Exists(folder, filename),
                  "sort files written for %s", Str_Get_Ptr8(field));
    }
    else {
        TEST_TRUE(runner, !RAMFolder_Exists(folder, filename),
                  "no sort files written for %s", Str_Get_Ptr8(field));
    }
    DECREF(filename);

    if (!is_used) { return; }

    SortReader *sort_reader
        = (SortReader*)SegReader_Obtain(seg_reader,
                                        Class_Get_Name(SORTREADER));
    DocReader *doc_reader
        = (DocReader*)SegReader_Obtain(seg_reader, Class_Get_Name(DOCREADER));
    SortCache *sort_cache
        = SortReader_Fetch_Sort_Cache(sort_reader, field);

    int32_t doc_max = SegReader_Doc_Max(seg_reader);
    for (int32_t doc_id = 1; doc_id <= doc_max; ++doc_id) {
        int32_t  ord         = SortCache_Ordinal(sort_cache, doc_id);
        Obj     *cache_value = SortCache_Value(sort_cache, ord);
        HitDoc  *doc         = DocReader_Fetch_Doc(doc_reader, doc_id);
        Obj     *doc_value   = HitDoc_Extract(doc, field);

        bool is_equal;
        if (cache_value == NULL || doc_value == NULL) {
            is_equal = (cache_value == doc_value);
        }
        else {
            is_equal = Obj_Equals(cache_value, doc_value);
        }
        TEST_TRUE(runner, is_equal, "correct cached value field %s doc %d",
                  Str_Get_Ptr8(field), doc_id);

        DECREF(doc_value);
        DECREF(doc);
        DECREF(cache_value);
    }
}

static void
test_sort_writer(TestBatchRunner *runner) {
    Schema    *schema  = S_create_schema();
    RAMFolder *folder  = RAMFolder_new(NULL);

    {
        // Add vehicles.
        Indexer *indexer = Indexer_new(schema, (Obj*)folder, NULL, 0);

        S_add_doc(indexer, "airplane", "0200", "8000", "air", "3", "nyet");
        S_add_doc(indexer, "bike", "0015", "0025", "land", "2", NULL);
        S_add_doc(indexer, "car", "0070", "3000", "land",  "4", NULL);

        Indexer_Commit(indexer);
        DECREF(indexer);
    }

    {
        PolyReader *poly_reader = PolyReader_open((Obj*)folder, NULL, NULL);
        VArray     *seg_readers = PolyReader_Get_Seg_Readers(poly_reader);
        SegReader  *seg_reader  = (SegReader*)VA_Fetch(seg_readers, 0);

        S_test_sort_cache(runner, folder, seg_reader, "1", true,  name_str);
        S_test_sort_cache(runner, folder, seg_reader, "1", true,  speed_str);
        S_test_sort_cache(runner, folder, seg_reader, "1", true,  weight_str);
        S_test_sort_cache(runner, folder, seg_reader, "1", true,  home_str);
        S_test_sort_cache(runner, folder, seg_reader, "1", true,  cat_str);
        S_test_sort_cache(runner, folder, seg_reader, "1", true,  wheels_str);
        S_test_sort_cache(runner, folder, seg_reader, "1", false, unused_str);
        S_test_sort_cache(runner, folder, seg_reader, "1", false, nope_str);

        DECREF(poly_reader);
    }

    {
        // Add a second segment.
        NonMergingIndexManager *manager = NMIxManager_new();
        Indexer *indexer
            = Indexer_new(schema, (Obj*)folder, (IndexManager*)manager, 0);
        // no "wheels" field -- test NULL/undef
        S_add_doc(indexer, "dirigible", "0040", "0000", "air", NULL, NULL);
        Indexer_Commit(indexer);
        DECREF(indexer);
        DECREF(manager);
    }

    {
        // Consolidate everything, to test merging.
        Indexer *indexer = Indexer_new(schema, (Obj*)folder, NULL, 0);
        StackString *bike_str = SSTR_WRAP_UTF8("bike", 4);
        Indexer_Delete_By_Term(indexer, name_str, (Obj*)bike_str);
        // no "wheels" field -- test NULL/undef
        S_add_doc(indexer, "elephant", "0020", "6000", "land", NULL, NULL);
        Indexer_Optimize(indexer);
        Indexer_Commit(indexer);
        DECREF(indexer);
    }

    {
        VArray *filenames = RAMFolder_List_R(folder, NULL);
        int num_old_seg_files = 0;
        for (uint32_t i = 0, size = VA_Get_Size(filenames); i < size; ++i) {
            String *filename = (String*)VA_Fetch(filenames, i);
            if (Str_Find_Utf8(filename, "seg_1", 5) >= 0
                || Str_Find_Utf8(filename, "seg_2", 5) >= 0
               ) {
                ++num_old_seg_files;
            }
        }
        TEST_INT_EQ(runner, num_old_seg_files, 0,
                    "all files from earlier segments zapped");
        DECREF(filenames);
    }

    {
        PolyReader *poly_reader = PolyReader_open((Obj*)folder, NULL, NULL);
        VArray     *seg_readers = PolyReader_Get_Seg_Readers(poly_reader);
        SegReader  *seg_reader  = (SegReader*)VA_Fetch(seg_readers, 0);

        S_test_sort_cache(runner, folder, seg_reader, "3", true, name_str);
        S_test_sort_cache(runner, folder, seg_reader, "3", true, speed_str);
        S_test_sort_cache(runner, folder, seg_reader, "3", true, weight_str);
        S_test_sort_cache(runner, folder, seg_reader, "3", true, home_str);
        S_test_sort_cache(runner, folder, seg_reader, "3", true, cat_str);
        S_test_sort_cache(runner, folder, seg_reader, "3", true, wheels_str);

        DECREF(poly_reader);
    }

    DECREF(folder);
    DECREF(schema);
}

void
TestSortWriter_Run_IMP(TestSortWriter *self, TestBatchRunner *runner) {
    TestBatchRunner_Plan(runner, (TestBatch*)self, 57);

    // Force frequent flushes.
    SortWriter_set_default_mem_thresh(100);

    S_init_strings();
    test_sort_writer(runner);
    S_destroy_strings();
}

NonMergingIndexManager*
NMIxManager_new() {
    NonMergingIndexManager *self
        = (NonMergingIndexManager*)Class_Make_Obj(NONMERGINGINDEXMANAGER);
    return NMIxManager_init(self);
}

NonMergingIndexManager*
NMIxManager_init(NonMergingIndexManager *self) {
    IxManager_init((IndexManager*)self, NULL, NULL);
    return self;
}

VArray*
NMIxManager_Recycle_IMP(NonMergingIndexManager *self, PolyReader *reader,
                        lucy_DeletionsWriter *del_writer, int64_t cutoff,
                        bool optimize) {
    UNUSED_VAR(self);
    UNUSED_VAR(reader);
    UNUSED_VAR(del_writer);
    UNUSED_VAR(cutoff);
    UNUSED_VAR(optimize);
    return VA_new(0);
}


