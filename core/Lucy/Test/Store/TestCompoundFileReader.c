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

#define C_LUCY_RAMFOLDER
#define TESTLUCY_USE_SHORT_NAMES
#include "Lucy/Util/ToolSet.h"

#include "Clownfish/TestHarness/TestBatchRunner.h"
#include "Lucy/Test.h"
#include "Lucy/Test/Store/TestCompoundFileReader.h"
#include "Lucy/Store/CompoundFileReader.h"
#include "Lucy/Store/CompoundFileWriter.h"
#include "Lucy/Store/FileHandle.h"
#include "Lucy/Store/InStream.h"
#include "Lucy/Store/OutStream.h"
#include "Lucy/Store/RAMFolder.h"
#include "Lucy/Util/Json.h"

static String *cfmeta_file = NULL;
static String *cfmeta_temp = NULL;
static String *cf_file     = NULL;
static String *foo         = NULL;
static String *bar         = NULL;
static String *baz         = NULL;
static String *seg_1       = NULL;
static String *stuff       = NULL;

TestCompoundFileReader*
TestCFReader_new() {
    return (TestCompoundFileReader*)Class_Make_Obj(TESTCOMPOUNDFILEREADER);
}

static void
S_init_strings(void) {
    cfmeta_file = Str_newf("cfmeta.json");
    cfmeta_temp = Str_newf("cfmeta.json.temp");
    cf_file     = Str_newf("cf.dat");
    foo         = Str_newf("foo");
    bar         = Str_newf("bar");
    baz         = Str_newf("baz");
    seg_1       = Str_newf("seg_1");
    stuff       = Str_newf("stuff");
}

static void
S_destroy_strings(void) {
    DECREF(cfmeta_file);
    DECREF(cfmeta_temp);
    DECREF(cf_file);
    DECREF(foo);
    DECREF(bar);
    DECREF(baz);
    DECREF(seg_1);
    DECREF(stuff);
}

static Folder*
S_folder_with_contents() {
    RAMFolder *folder  = RAMFolder_new(seg_1);
    OutStream *foo_out = RAMFolder_Open_Out(folder, foo);
    OutStream *bar_out = RAMFolder_Open_Out(folder, bar);
    OutStream_Write_Bytes(foo_out, "foo", 3);
    OutStream_Write_Bytes(bar_out, "bar", 3);
    OutStream_Close(foo_out);
    OutStream_Close(bar_out);
    DECREF(foo_out);
    DECREF(bar_out);
    StackString *empty = SSTR_BLANK();
    RAMFolder_Consolidate(folder, (String*)empty);
    return (Folder*)folder;
}

static void
test_open(TestBatchRunner *runner) {
    Folder *real_folder;
    CompoundFileReader *cf_reader;
    Hash *metadata;

    Err_set_error(NULL);
    real_folder = S_folder_with_contents();
    Folder_Delete(real_folder, cfmeta_file);
    cf_reader = CFReader_open(real_folder);
    TEST_TRUE(runner, cf_reader == NULL,
              "Return NULL when cfmeta file missing");
    TEST_TRUE(runner, Err_get_error() != NULL,
              "Set Err_error when cfmeta file missing");
    DECREF(real_folder);

    Err_set_error(NULL);
    real_folder = S_folder_with_contents();
    Folder_Delete(real_folder, cf_file);
    cf_reader = CFReader_open(real_folder);
    TEST_TRUE(runner, cf_reader == NULL,
              "Return NULL when cf.dat file missing");
    TEST_TRUE(runner, Err_get_error() != NULL,
              "Set Err_error when cf.dat file missing");
    DECREF(real_folder);

    Err_set_error(NULL);
    real_folder = S_folder_with_contents();
    metadata = (Hash*)Json_slurp_json(real_folder, cfmeta_file);
    Hash_Store_Utf8(metadata, "format", 6, (Obj*)Str_newf("%i32", -1));
    Folder_Delete(real_folder, cfmeta_file);
    Json_spew_json((Obj*)metadata, real_folder, cfmeta_file);
    cf_reader = CFReader_open(real_folder);
    TEST_TRUE(runner, cf_reader == NULL,
              "Return NULL when format is invalid");
    TEST_TRUE(runner, Err_get_error() != NULL,
              "Set Err_error when format is invalid");

    Err_set_error(NULL);
    Hash_Store_Utf8(metadata, "format", 6, (Obj*)Str_newf("%i32", 1000));
    Folder_Delete(real_folder, cfmeta_file);
    Json_spew_json((Obj*)metadata, real_folder, cfmeta_file);
    cf_reader = CFReader_open(real_folder);
    TEST_TRUE(runner, cf_reader == NULL,
              "Return NULL when format is too recent");
    TEST_TRUE(runner, Err_get_error() != NULL,
              "Set Err_error when format too recent");

    Err_set_error(NULL);
    DECREF(Hash_Delete_Utf8(metadata, "format", 6));
    Folder_Delete(real_folder, cfmeta_file);
    Json_spew_json((Obj*)metadata, real_folder, cfmeta_file);
    cf_reader = CFReader_open(real_folder);
    TEST_TRUE(runner, cf_reader == NULL,
              "Return NULL when format key is missing");
    TEST_TRUE(runner, Err_get_error() != NULL,
              "Set Err_error when format key is missing");

    Hash_Store_Utf8(metadata, "format", 6,
                    (Obj*)Str_newf("%i32", CFWriter_current_file_format));
    DECREF(Hash_Delete_Utf8(metadata, "files", 5));
    Folder_Delete(real_folder, cfmeta_file);
    Json_spew_json((Obj*)metadata, real_folder, cfmeta_file);
    cf_reader = CFReader_open(real_folder);
    TEST_TRUE(runner, cf_reader == NULL,
              "Return NULL when files key is missing");
    TEST_TRUE(runner, Err_get_error() != NULL,
              "Set Err_error when files key is missing");

    DECREF(metadata);
    DECREF(real_folder);
}

static void
test_Local_MkDir_and_Find_Folder(TestBatchRunner *runner) {
    Folder *real_folder = S_folder_with_contents();
    CompoundFileReader *cf_reader = CFReader_open(real_folder);

    TEST_FALSE(runner,
               CFReader_Local_Is_Directory(cf_reader, stuff),
               "Local_Is_Directory returns false for non-existent entry");

    TEST_TRUE(runner, CFReader_MkDir(cf_reader, stuff),
              "MkDir returns true");
    TEST_TRUE(runner,
              Folder_Find_Folder(real_folder, stuff) != NULL,
              "Local_MkDir pass-through");
    TEST_TRUE(runner,
              Folder_Find_Folder(real_folder, stuff)
              == CFReader_Find_Folder(cf_reader, stuff),
              "Local_Find_Folder pass-through");
    TEST_TRUE(runner,
              CFReader_Local_Is_Directory(cf_reader, stuff),
              "Local_Is_Directory pass through");

    Err_set_error(NULL);
    TEST_FALSE(runner, CFReader_MkDir(cf_reader, stuff),
               "MkDir returns false when dir already exists");
    TEST_TRUE(runner, Err_get_error() != NULL,
              "MkDir sets Err_error when dir already exists");

    Err_set_error(NULL);
    TEST_FALSE(runner, CFReader_MkDir(cf_reader, foo),
               "MkDir returns false when virtual file exists");
    TEST_TRUE(runner, Err_get_error() != NULL,
              "MkDir sets Err_error when virtual file exists");

    TEST_TRUE(runner,
              CFReader_Find_Folder(cf_reader, foo) == NULL,
              "Virtual file not reported as directory");
    TEST_FALSE(runner, CFReader_Local_Is_Directory(cf_reader, foo),
               "Local_Is_Directory returns false for virtual file");

    DECREF(real_folder);
    DECREF(cf_reader);
}

static void
test_Local_Delete_and_Exists(TestBatchRunner *runner) {
    Folder *real_folder = S_folder_with_contents();
    CompoundFileReader *cf_reader = CFReader_open(real_folder);

    CFReader_MkDir(cf_reader, stuff);
    TEST_TRUE(runner, CFReader_Local_Exists(cf_reader, stuff),
              "pass through for Local_Exists");
    TEST_TRUE(runner, CFReader_Local_Exists(cf_reader, foo),
              "Local_Exists returns true for virtual file");

    TEST_TRUE(runner,
              CFReader_Local_Exists(cf_reader, cfmeta_file),
              "cfmeta file exists");

    TEST_TRUE(runner, CFReader_Local_Delete(cf_reader, stuff),
              "Local_Delete returns true when zapping real entity");
    TEST_FALSE(runner, CFReader_Local_Exists(cf_reader, stuff),
               "Local_Exists returns false after real entity zapped");

    TEST_TRUE(runner, CFReader_Local_Delete(cf_reader, foo),
              "Local_Delete returns true when zapping virtual file");
    TEST_FALSE(runner, CFReader_Local_Exists(cf_reader, foo),
               "Local_Exists returns false after virtual file zapped");

    TEST_TRUE(runner, CFReader_Local_Delete(cf_reader, bar),
              "Local_Delete returns true when zapping last virtual file");
    TEST_FALSE(runner,
               CFReader_Local_Exists(cf_reader, cfmeta_file),
               "cfmeta file deleted when last virtual file deleted");
    TEST_FALSE(runner,
               CFReader_Local_Exists(cf_reader, cf_file),
               "compound data file deleted when last virtual file deleted");

    DECREF(cf_reader);
    DECREF(real_folder);
}

static void
test_Local_Open_Dir(TestBatchRunner *runner) {

    Folder *real_folder = S_folder_with_contents();
    CompoundFileReader *cf_reader = CFReader_open(real_folder);
    bool saw_foo       = false;
    bool saw_stuff     = false;
    bool stuff_was_dir = false;

    CFReader_MkDir(cf_reader, stuff);

    DirHandle *dh = CFReader_Local_Open_Dir(cf_reader);
    while (DH_Next(dh)) {
        String *entry = DH_Get_Entry(dh);
        if (Str_Equals(entry, (Obj*)foo)) {
            saw_foo = true;
        }
        else if (Str_Equals(entry, (Obj*)stuff)) {
            saw_stuff = true;
            stuff_was_dir = DH_Entry_Is_Dir(dh);
        }
        DECREF(entry);
    }

    TEST_TRUE(runner, saw_foo, "DirHandle iterated over virtual file");
    TEST_TRUE(runner, saw_stuff, "DirHandle iterated over real directory");
    TEST_TRUE(runner, stuff_was_dir,
              "DirHandle knew that real entry was dir");

    DECREF(dh);
    DECREF(cf_reader);
    DECREF(real_folder);
}

static void
test_Local_Open_FileHandle(TestBatchRunner *runner) {
    Folder *real_folder = S_folder_with_contents();
    CompoundFileReader *cf_reader = CFReader_open(real_folder);
    FileHandle *fh;

    OutStream *outstream = CFReader_Open_Out(cf_reader, baz);
    OutStream_Write_Bytes(outstream, "baz", 3);
    OutStream_Close(outstream);
    DECREF(outstream);

    fh = CFReader_Local_Open_FileHandle(cf_reader, baz,
                                        FH_READ_ONLY);
    TEST_TRUE(runner, fh != NULL,
              "Local_Open_FileHandle pass-through for real file");
    DECREF(fh);

    Err_set_error(NULL);
    fh = CFReader_Local_Open_FileHandle(cf_reader, stuff,
                                        FH_READ_ONLY);
    TEST_TRUE(runner, fh == NULL,
              "Local_Open_FileHandle for non-existent file returns NULL");
    TEST_TRUE(runner, Err_get_error() != NULL,
              "Local_Open_FileHandle for non-existent file sets Err_error");

    Err_set_error(NULL);
    fh = CFReader_Local_Open_FileHandle(cf_reader, foo,
                                        FH_READ_ONLY);
    TEST_TRUE(runner, fh == NULL,
              "Local_Open_FileHandle for virtual file returns NULL");
    TEST_TRUE(runner, Err_get_error() != NULL,
              "Local_Open_FileHandle for virtual file sets Err_error");

    DECREF(cf_reader);
    DECREF(real_folder);
}

static void
test_Local_Open_In(TestBatchRunner *runner) {
    Folder *real_folder = S_folder_with_contents();
    CompoundFileReader *cf_reader = CFReader_open(real_folder);
    InStream *instream;

    instream = CFReader_Local_Open_In(cf_reader, foo);
    TEST_TRUE(runner, instream != NULL,
              "Local_Open_In for virtual file");
    TEST_TRUE(runner,
              Str_Starts_With(InStream_Get_Filename(instream), CFReader_Get_Path(cf_reader)),
              "InStream's path includes directory");
    DECREF(instream);

    OutStream *outstream = CFReader_Open_Out(cf_reader, baz);
    OutStream_Write_Bytes(outstream, "baz", 3);
    OutStream_Close(outstream);
    DECREF(outstream);
    instream = CFReader_Local_Open_In(cf_reader, baz);
    TEST_TRUE(runner, instream != NULL,
              "Local_Open_In pass-through for real file");
    DECREF(instream);

    Err_set_error(NULL);
    instream = CFReader_Local_Open_In(cf_reader, stuff);
    TEST_TRUE(runner, instream == NULL,
              "Local_Open_In for non-existent file returns NULL");
    TEST_TRUE(runner, Err_get_error() != NULL,
              "Local_Open_In for non-existent file sets Err_error");

    DECREF(cf_reader);
    DECREF(real_folder);
}

static void
test_Close(TestBatchRunner *runner) {
    Folder *real_folder = S_folder_with_contents();
    CompoundFileReader *cf_reader = CFReader_open(real_folder);

    CFReader_Close(cf_reader);
    PASS(runner, "Close completes without incident");

    CFReader_Close(cf_reader);
    PASS(runner, "Calling Close() multiple times is ok");

    DECREF(cf_reader);
    DECREF(real_folder);
}

void
TestCFReader_Run_IMP(TestCompoundFileReader *self, TestBatchRunner *runner) {
    TestBatchRunner_Plan(runner, (TestBatch*)self, 48);
    S_init_strings();
    test_open(runner);
    test_Local_MkDir_and_Find_Folder(runner);
    test_Local_Delete_and_Exists(runner);
    test_Local_Open_Dir(runner);
    test_Local_Open_FileHandle(runner);
    test_Local_Open_In(runner);
    test_Close(runner);
    S_destroy_strings();
}


