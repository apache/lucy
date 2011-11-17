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

#define C_LUCY_ZOMBIECHARBUF
#define C_LUCY_RAMFOLDER
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Test.h"
#include "Lucy/Test/Store/TestCompoundFileReader.h"
#include "Lucy/Store/CompoundFileReader.h"
#include "Lucy/Store/CompoundFileWriter.h"
#include "Lucy/Store/FileHandle.h"
#include "Lucy/Store/InStream.h"
#include "Lucy/Store/OutStream.h"
#include "Lucy/Store/RAMFolder.h"
#include "Lucy/Util/Json.h"

static ZombieCharBuf cfmeta_file = ZCB_LITERAL("cfmeta.json");
static ZombieCharBuf cf_file     = ZCB_LITERAL("cf.dat");
static ZombieCharBuf foo         = ZCB_LITERAL("foo");
static ZombieCharBuf bar         = ZCB_LITERAL("bar");
static ZombieCharBuf baz         = ZCB_LITERAL("baz");
static ZombieCharBuf stuff       = ZCB_LITERAL("stuff");

static Folder*
S_folder_with_contents() {
    ZombieCharBuf seg_1 = ZCB_LITERAL("seg_1");
    RAMFolder *folder  = RAMFolder_new((CharBuf*)&seg_1);
    OutStream *foo_out = RAMFolder_Open_Out(folder, (CharBuf*)&foo);
    OutStream *bar_out = RAMFolder_Open_Out(folder, (CharBuf*)&bar);
    OutStream_Write_Bytes(foo_out, "foo", 3);
    OutStream_Write_Bytes(bar_out, "bar", 3);
    OutStream_Close(foo_out);
    OutStream_Close(bar_out);
    DECREF(foo_out);
    DECREF(bar_out);
    RAMFolder_Consolidate(folder, (CharBuf*)&EMPTY);
    return (Folder*)folder;
}

static void
test_open(TestBatch *batch) {
    Folder *real_folder;
    CompoundFileReader *cf_reader;
    Hash *metadata;

    Err_set_error(NULL);
    real_folder = S_folder_with_contents();
    Folder_Delete(real_folder, (CharBuf*)&cfmeta_file);
    cf_reader = CFReader_open(real_folder);
    TEST_TRUE(batch, cf_reader == NULL,
              "Return NULL when cfmeta file missing");
    TEST_TRUE(batch, Err_get_error() != NULL,
              "Set Err_error when cfmeta file missing");
    DECREF(real_folder);

    Err_set_error(NULL);
    real_folder = S_folder_with_contents();
    Folder_Delete(real_folder, (CharBuf*)&cf_file);
    cf_reader = CFReader_open(real_folder);
    TEST_TRUE(batch, cf_reader == NULL,
              "Return NULL when cf.dat file missing");
    TEST_TRUE(batch, Err_get_error() != NULL,
              "Set Err_error when cf.dat file missing");
    DECREF(real_folder);

    Err_set_error(NULL);
    real_folder = S_folder_with_contents();
    metadata = (Hash*)Json_slurp_json(real_folder, (CharBuf*)&cfmeta_file);
    Hash_Store_Str(metadata, "format", 6, (Obj*)CB_newf("%i32", -1));
    Folder_Delete(real_folder, (CharBuf*)&cfmeta_file);
    Json_spew_json((Obj*)metadata, real_folder, (CharBuf*)&cfmeta_file);
    cf_reader = CFReader_open(real_folder);
    TEST_TRUE(batch, cf_reader == NULL,
              "Return NULL when format is invalid");
    TEST_TRUE(batch, Err_get_error() != NULL,
              "Set Err_error when format is invalid");

    Err_set_error(NULL);
    Hash_Store_Str(metadata, "format", 6, (Obj*)CB_newf("%i32", 1000));
    Folder_Delete(real_folder, (CharBuf*)&cfmeta_file);
    Json_spew_json((Obj*)metadata, real_folder, (CharBuf*)&cfmeta_file);
    cf_reader = CFReader_open(real_folder);
    TEST_TRUE(batch, cf_reader == NULL,
              "Return NULL when format is too recent");
    TEST_TRUE(batch, Err_get_error() != NULL,
              "Set Err_error when format too recent");

    Err_set_error(NULL);
    DECREF(Hash_Delete_Str(metadata, "format", 6));
    Folder_Delete(real_folder, (CharBuf*)&cfmeta_file);
    Json_spew_json((Obj*)metadata, real_folder, (CharBuf*)&cfmeta_file);
    cf_reader = CFReader_open(real_folder);
    TEST_TRUE(batch, cf_reader == NULL,
              "Return NULL when format key is missing");
    TEST_TRUE(batch, Err_get_error() != NULL,
              "Set Err_error when format key is missing");

    Hash_Store_Str(metadata, "format", 6,
                   (Obj*)CB_newf("%i32", CFWriter_current_file_format));
    DECREF(Hash_Delete_Str(metadata, "files", 5));
    Folder_Delete(real_folder, (CharBuf*)&cfmeta_file);
    Json_spew_json((Obj*)metadata, real_folder, (CharBuf*)&cfmeta_file);
    cf_reader = CFReader_open(real_folder);
    TEST_TRUE(batch, cf_reader == NULL,
              "Return NULL when files key is missing");
    TEST_TRUE(batch, Err_get_error() != NULL,
              "Set Err_error when files key is missing");

    DECREF(metadata);
    DECREF(real_folder);
}

static void
test_Local_MkDir_and_Find_Folder(TestBatch *batch) {
    Folder *real_folder = S_folder_with_contents();
    CompoundFileReader *cf_reader = CFReader_open(real_folder);

    TEST_FALSE(batch,
               CFReader_Local_Is_Directory(cf_reader, (CharBuf*)&stuff),
               "Local_Is_Directory returns false for non-existent entry");

    TEST_TRUE(batch, CFReader_MkDir(cf_reader, (CharBuf*)&stuff),
              "MkDir returns true");
    TEST_TRUE(batch,
              Folder_Find_Folder(real_folder, (CharBuf*)&stuff) != NULL,
              "Local_MkDir pass-through");
    TEST_TRUE(batch,
              Folder_Find_Folder(real_folder, (CharBuf*)&stuff)
              == CFReader_Find_Folder(cf_reader, (CharBuf*)&stuff),
              "Local_Find_Folder pass-through");
    TEST_TRUE(batch,
              CFReader_Local_Is_Directory(cf_reader, (CharBuf*)&stuff),
              "Local_Is_Directory pass through");

    Err_set_error(NULL);
    TEST_FALSE(batch, CFReader_MkDir(cf_reader, (CharBuf*)&stuff),
               "MkDir returns false when dir already exists");
    TEST_TRUE(batch, Err_get_error() != NULL,
              "MkDir sets Err_error when dir already exists");

    Err_set_error(NULL);
    TEST_FALSE(batch, CFReader_MkDir(cf_reader, (CharBuf*)&foo),
               "MkDir returns false when virtual file exists");
    TEST_TRUE(batch, Err_get_error() != NULL,
              "MkDir sets Err_error when virtual file exists");

    TEST_TRUE(batch,
              CFReader_Find_Folder(cf_reader, (CharBuf*)&foo) == NULL,
              "Virtual file not reported as directory");
    TEST_FALSE(batch, CFReader_Local_Is_Directory(cf_reader, (CharBuf*)&foo),
               "Local_Is_Directory returns false for virtual file");

    DECREF(real_folder);
    DECREF(cf_reader);
}

static void
test_Local_Delete_and_Exists(TestBatch *batch) {
    Folder *real_folder = S_folder_with_contents();
    CompoundFileReader *cf_reader = CFReader_open(real_folder);

    CFReader_MkDir(cf_reader, (CharBuf*)&stuff);
    TEST_TRUE(batch, CFReader_Local_Exists(cf_reader, (CharBuf*)&stuff),
              "pass through for Local_Exists");
    TEST_TRUE(batch, CFReader_Local_Exists(cf_reader, (CharBuf*)&foo),
              "Local_Exists returns true for virtual file");

    TEST_TRUE(batch,
              CFReader_Local_Exists(cf_reader, (CharBuf*)&cfmeta_file),
              "cfmeta file exists");

    TEST_TRUE(batch, CFReader_Local_Delete(cf_reader, (CharBuf*)&stuff),
              "Local_Delete returns true when zapping real entity");
    TEST_FALSE(batch, CFReader_Local_Exists(cf_reader, (CharBuf*)&stuff),
               "Local_Exists returns false after real entity zapped");

    TEST_TRUE(batch, CFReader_Local_Delete(cf_reader, (CharBuf*)&foo),
              "Local_Delete returns true when zapping virtual file");
    TEST_FALSE(batch, CFReader_Local_Exists(cf_reader, (CharBuf*)&foo),
               "Local_Exists returns false after virtual file zapped");

    TEST_TRUE(batch, CFReader_Local_Delete(cf_reader, (CharBuf*)&bar),
              "Local_Delete returns true when zapping last virtual file");
    TEST_FALSE(batch,
               CFReader_Local_Exists(cf_reader, (CharBuf*)&cfmeta_file),
               "cfmeta file deleted when last virtual file deleted");
    TEST_FALSE(batch,
               CFReader_Local_Exists(cf_reader, (CharBuf*)&cf_file),
               "compound data file deleted when last virtual file deleted");

    DECREF(cf_reader);
    DECREF(real_folder);
}

static void
test_Local_Open_Dir(TestBatch *batch) {

    Folder *real_folder = S_folder_with_contents();
    CompoundFileReader *cf_reader = CFReader_open(real_folder);
    DirHandle *dh;
    CharBuf *entry;
    bool_t saw_foo       = false;
    bool_t saw_stuff     = false;
    bool_t stuff_was_dir = false;

    CFReader_MkDir(cf_reader, (CharBuf*)&stuff);

    dh = CFReader_Local_Open_Dir(cf_reader);
    entry = DH_Get_Entry(dh);
    while (DH_Next(dh)) {
        if (CB_Equals(entry, (Obj*)&foo)) {
            saw_foo = true;
        }
        else if (CB_Equals(entry, (Obj*)&stuff)) {
            saw_stuff = true;
            stuff_was_dir = DH_Entry_Is_Dir(dh);
        }
    }

    TEST_TRUE(batch, saw_foo, "DirHandle iterated over virtual file");
    TEST_TRUE(batch, saw_stuff, "DirHandle iterated over real directory");
    TEST_TRUE(batch, stuff_was_dir,
              "DirHandle knew that real entry was dir");

    DECREF(dh);
    DECREF(cf_reader);
    DECREF(real_folder);
}

static void
test_Local_Open_FileHandle(TestBatch *batch) {
    Folder *real_folder = S_folder_with_contents();
    CompoundFileReader *cf_reader = CFReader_open(real_folder);
    FileHandle *fh;

    OutStream *outstream = CFReader_Open_Out(cf_reader, (CharBuf*)&baz);
    OutStream_Write_Bytes(outstream, "baz", 3);
    OutStream_Close(outstream);
    DECREF(outstream);

    fh = CFReader_Local_Open_FileHandle(cf_reader, (CharBuf*)&baz,
                                        FH_READ_ONLY);
    TEST_TRUE(batch, fh != NULL,
              "Local_Open_FileHandle pass-through for real file");
    DECREF(fh);

    Err_set_error(NULL);
    fh = CFReader_Local_Open_FileHandle(cf_reader, (CharBuf*)&stuff,
                                        FH_READ_ONLY);
    TEST_TRUE(batch, fh == NULL,
              "Local_Open_FileHandle for non-existent file returns NULL");
    TEST_TRUE(batch, Err_get_error() != NULL,
              "Local_Open_FileHandle for non-existent file sets Err_error");

    Err_set_error(NULL);
    fh = CFReader_Local_Open_FileHandle(cf_reader, (CharBuf*)&foo,
                                        FH_READ_ONLY);
    TEST_TRUE(batch, fh == NULL,
              "Local_Open_FileHandle for virtual file returns NULL");
    TEST_TRUE(batch, Err_get_error() != NULL,
              "Local_Open_FileHandle for virtual file sets Err_error");

    DECREF(cf_reader);
    DECREF(real_folder);
}

static void
test_Local_Open_In(TestBatch *batch) {
    Folder *real_folder = S_folder_with_contents();
    CompoundFileReader *cf_reader = CFReader_open(real_folder);
    InStream *instream;

    instream = CFReader_Local_Open_In(cf_reader, (CharBuf*)&foo);
    TEST_TRUE(batch, instream != NULL,
              "Local_Open_In for virtual file");
    TEST_TRUE(batch,
              CB_Starts_With(InStream_Get_Filename(instream), CFReader_Get_Path(cf_reader)),
              "InStream's path includes directory");
    DECREF(instream);

    OutStream *outstream = CFReader_Open_Out(cf_reader, (CharBuf*)&baz);
    OutStream_Write_Bytes(outstream, "baz", 3);
    OutStream_Close(outstream);
    DECREF(outstream);
    instream = CFReader_Local_Open_In(cf_reader, (CharBuf*)&baz);
    TEST_TRUE(batch, instream != NULL,
              "Local_Open_In pass-through for real file");
    DECREF(instream);

    Err_set_error(NULL);
    instream = CFReader_Local_Open_In(cf_reader, (CharBuf*)&stuff);
    TEST_TRUE(batch, instream == NULL,
              "Local_Open_In for non-existent file returns NULL");
    TEST_TRUE(batch, Err_get_error() != NULL,
              "Local_Open_In for non-existent file sets Err_error");

    DECREF(cf_reader);
    DECREF(real_folder);
}

static void
test_Close(TestBatch *batch) {
    Folder *real_folder = S_folder_with_contents();
    CompoundFileReader *cf_reader = CFReader_open(real_folder);

    CFReader_Close(cf_reader);
    PASS(batch, "Close completes without incident");

    CFReader_Close(cf_reader);
    PASS(batch, "Calling Close() multiple times is ok");

    DECREF(cf_reader);
    DECREF(real_folder);
}

void
TestCFReader_run_tests() {
    TestBatch *batch = TestBatch_new(48);

    TestBatch_Plan(batch);
    test_open(batch);
    test_Local_MkDir_and_Find_Folder(batch);
    test_Local_Delete_and_Exists(batch);
    test_Local_Open_Dir(batch);
    test_Local_Open_FileHandle(batch);
    test_Local_Open_In(batch);
    test_Close(batch);

    DECREF(batch);
}


