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
#define C_LUCY_CHARBUF
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Test.h"
#include "Lucy/Test/Store/TestFolder.h"
#include "Lucy/Store/DirHandle.h"
#include "Lucy/Store/FileHandle.h"
#include "Lucy/Store/InStream.h"
#include "Lucy/Store/OutStream.h"
#include "Lucy/Store/RAMFolder.h"

static CharBuf foo               = ZCB_LITERAL("foo");
static CharBuf bar               = ZCB_LITERAL("bar");
static CharBuf baz               = ZCB_LITERAL("baz");
static CharBuf boffo             = ZCB_LITERAL("boffo");
static CharBuf banana            = ZCB_LITERAL("banana");
static CharBuf foo_bar           = ZCB_LITERAL("foo/bar");
static CharBuf foo_boffo         = ZCB_LITERAL("foo/boffo");
static CharBuf foo_foo           = ZCB_LITERAL("foo/foo");
static CharBuf foo_bar_baz       = ZCB_LITERAL("foo/bar/baz");
static CharBuf foo_bar_baz_boffo = ZCB_LITERAL("foo/bar/baz/boffo");
static CharBuf nope              = ZCB_LITERAL("nope");

static void
test_Exists(TestBatch *batch) {
    Folder *folder = (Folder*)RAMFolder_new(NULL);
    FileHandle *fh;

    Folder_MkDir(folder, &foo);
    Folder_MkDir(folder, &foo_bar);
    fh = Folder_Open_FileHandle(folder, &boffo, FH_CREATE | FH_WRITE_ONLY);
    DECREF(fh);
    fh = Folder_Open_FileHandle(folder, &foo_boffo,
                                FH_CREATE | FH_WRITE_ONLY);
    DECREF(fh);

    TEST_TRUE(batch, Folder_Exists(folder, &foo), "Dir exists");
    TEST_TRUE(batch, Folder_Exists(folder, &boffo), "File exists");
    TEST_TRUE(batch, Folder_Exists(folder, &foo_bar),
              "Nested dir exists");
    TEST_TRUE(batch, Folder_Exists(folder, &foo_boffo),
              "Nested file exists");

    TEST_FALSE(batch, Folder_Exists(folder, &banana),
               "Non-existent entry");
    TEST_FALSE(batch, Folder_Exists(folder, &foo_foo),
               "Non-existent nested entry");

    DECREF(folder);
}

static void
test_Set_Path_and_Get_Path(TestBatch *batch) {
    Folder *folder = (Folder*)RAMFolder_new(&foo);
    TEST_TRUE(batch, CB_Equals(Folder_Get_Path(folder), (Obj*)&foo),
              "Get_Path");
    Folder_Set_Path(folder, &bar);
    TEST_TRUE(batch, CB_Equals(Folder_Get_Path(folder), (Obj*)&bar),
              "Set_Path");
    DECREF(folder);
}

static void
test_MkDir_and_Is_Directory(TestBatch *batch) {
    Folder *folder = (Folder*)RAMFolder_new(NULL);
    FileHandle *fh;

    TEST_FALSE(batch, Folder_Is_Directory(folder, &foo),
               "Is_Directory() false for non-existent entry");

    TEST_TRUE(batch, Folder_MkDir(folder, &foo),
              "MkDir returns true on success");
    TEST_TRUE(batch, Folder_Is_Directory(folder, &foo),
              "Is_Directory() true for local folder");

    TEST_FALSE(batch, Folder_Is_Directory(folder, &foo_bar_baz),
               "Is_Directory() false for non-existent deeply nested dir");
    Err_set_error(NULL);
    TEST_FALSE(batch, Folder_MkDir(folder, &foo_bar_baz),
               "MkDir for deeply nested dir fails");
    TEST_TRUE(batch, Err_get_error() != NULL,
              "MkDir for deeply nested dir sets Err_error");

    TEST_TRUE(batch, Folder_MkDir(folder, &foo_bar),
              "MkDir for nested dir");
    TEST_TRUE(batch, Folder_Is_Directory(folder, &foo_bar),
              "Is_Directory() true for nested dir");

    Err_set_error(NULL);
    TEST_FALSE(batch, Folder_MkDir(folder, &foo_bar),
               "Overwrite dir with MkDir fails");
    TEST_TRUE(batch, Err_get_error() != NULL,
              "Overwrite dir with MkDir sets Err_error");

    fh = Folder_Open_FileHandle(folder, &foo_boffo,
                                FH_CREATE | FH_WRITE_ONLY);
    DECREF(fh);
    Err_set_error(NULL);
    TEST_FALSE(batch, Folder_MkDir(folder, &foo_boffo),
               "Overwrite file with MkDir fails");
    TEST_TRUE(batch, Err_get_error() != NULL,
              "Overwrite file with MkDir sets Err_error");
    TEST_FALSE(batch, Folder_Is_Directory(folder, &foo_boffo),
               "Is_Directory() false for nested file");

    DECREF(folder);
}

static void
test_Enclosing_Folder_and_Find_Folder(TestBatch *batch) {
    Folder *folder = (Folder*)RAMFolder_new(NULL);
    FileHandle *fh;

    Folder_MkDir(folder, &foo);
    Folder_MkDir(folder, &foo_bar);
    Folder_MkDir(folder, &foo_bar_baz);
    fh = Folder_Open_FileHandle(folder, &foo_bar_baz_boffo,
                                FH_CREATE | FH_WRITE_ONLY);

    {
        Folder *encloser = Folder_Enclosing_Folder(folder, (CharBuf*)&nope);
        Folder *found = Folder_Find_Folder(folder, (CharBuf*)&nope);
        TEST_TRUE(batch, encloser == folder,
                  "Enclosing_Folder() - non-existent entry yields parent");
        TEST_TRUE(batch, found == NULL,
                  "Find_Folder() - non-existent entry yields NULL");
    }

    {
        Folder *encloser = Folder_Enclosing_Folder(folder, &foo_bar);
        Folder *found = Folder_Find_Folder(folder, &foo_bar);
        TEST_TRUE(batch,
                  encloser
                  && Folder_Is_A(encloser, FOLDER)
                  && CB_Ends_With(Folder_Get_Path(encloser), &foo),
                  "Enclosing_Folder() - find one directory down");
        TEST_TRUE(batch,
                  found
                  && Folder_Is_A(found, FOLDER)
                  && CB_Ends_With(Folder_Get_Path(found), &bar),
                  "Find_Folder() - 'foo/bar'");
    }

    {
        Folder *encloser = Folder_Enclosing_Folder(folder, &foo_bar_baz);
        Folder *found = Folder_Find_Folder(folder, &foo_bar_baz);
        TEST_TRUE(batch,
                  encloser
                  && Folder_Is_A(encloser, FOLDER)
                  && CB_Ends_With(Folder_Get_Path(encloser), &bar),
                  "Find two directories down");
        TEST_TRUE(batch,
                  found
                  && Folder_Is_A(found, FOLDER)
                  && CB_Ends_With(Folder_Get_Path(found), &baz),
                  "Find_Folder() - 'foo/bar/baz'");
    }

    {
        Folder *encloser
            = Folder_Enclosing_Folder(folder, &foo_bar_baz_boffo);
        Folder *found = Folder_Find_Folder(folder, &foo_bar_baz_boffo);
        TEST_TRUE(batch,
                  encloser
                  && Folder_Is_A(encloser, FOLDER)
                  && CB_Ends_With(Folder_Get_Path(encloser), &baz),
                  "Recurse to find a directory containing a real file");
        TEST_TRUE(batch, found == NULL,
                  "Find_Folder() - file instead of folder yields NULL");
    }

    DECREF(fh);
    DECREF(folder);
}

static void
test_List(TestBatch *batch) {
    Folder     *folder = (Folder*)RAMFolder_new(NULL);
    FileHandle *fh;
    VArray     *list;
    CharBuf    *elem;

    Folder_MkDir(folder, &foo);
    Folder_MkDir(folder, &foo_bar);
    Folder_MkDir(folder, &foo_bar_baz);
    fh = Folder_Open_FileHandle(folder, &boffo, FH_CREATE | FH_WRITE_ONLY);
    DECREF(fh);
    fh = Folder_Open_FileHandle(folder, &banana, FH_CREATE | FH_WRITE_ONLY);
    DECREF(fh);

    list = Folder_List(folder, NULL);
    VA_Sort(list, NULL, NULL);
    TEST_INT_EQ(batch, VA_Get_Size(list), 3, "List");
    elem = (CharBuf*)DOWNCAST(VA_Fetch(list, 0), CHARBUF);
    TEST_TRUE(batch, elem && CB_Equals(elem, (Obj*)&banana),
              "List first file");
    elem = (CharBuf*)DOWNCAST(VA_Fetch(list, 1), CHARBUF);
    TEST_TRUE(batch, elem && CB_Equals(elem, (Obj*)&boffo),
              "List second file");
    elem = (CharBuf*)DOWNCAST(VA_Fetch(list, 2), CHARBUF);
    TEST_TRUE(batch, elem && CB_Equals(elem, (Obj*)&foo), "List dir");
    DECREF(list);

    list = Folder_List(folder, &foo_bar);
    TEST_INT_EQ(batch, VA_Get_Size(list), 1, "List subdirectory contents");
    elem = (CharBuf*)DOWNCAST(VA_Fetch(list, 0), CHARBUF);
    TEST_TRUE(batch, elem && CB_Equals(elem, (Obj*)&baz),
              "Just the filename");
    DECREF(list);

    DECREF(folder);
}

static void
test_Open_Dir(TestBatch *batch) {
    Folder     *folder = (Folder*)RAMFolder_new(NULL);
    DirHandle  *dh;

    Folder_MkDir(folder, &foo);
    Folder_MkDir(folder, &foo_bar);

    dh = Folder_Open_Dir(folder, &foo);
    TEST_TRUE(batch, dh && DH_Is_A(dh, DIRHANDLE), "Open_Dir");
    DECREF(dh);
    dh = Folder_Open_Dir(folder, &foo_bar);
    TEST_TRUE(batch, dh && DH_Is_A(dh, DIRHANDLE), "Open_Dir nested dir");
    DECREF(dh);

    Err_set_error(NULL);
    dh = Folder_Open_Dir(folder, &bar);
    TEST_TRUE(batch, dh == NULL,
              "Open_Dir on non-existent entry fails");
    TEST_TRUE(batch, Err_get_error() != NULL,
              "Open_Dir on non-existent entry sets Err_error");

    Err_set_error(NULL);
    dh = Folder_Open_Dir(folder, &foo_foo);
    TEST_TRUE(batch, dh == NULL,
              "Open_Dir on non-existent nested entry fails");
    TEST_TRUE(batch, Err_get_error() != NULL,
              "Open_Dir on non-existent nested entry sets Err_error");

    DECREF(folder);
}

static void
test_Open_FileHandle(TestBatch *batch) {
    Folder     *folder = (Folder*)RAMFolder_new(NULL);
    FileHandle *fh;

    Folder_MkDir(folder, &foo);

    fh = Folder_Open_FileHandle(folder, &boffo, FH_CREATE | FH_WRITE_ONLY);
    TEST_TRUE(batch, fh && FH_Is_A(fh, FILEHANDLE), "Open_FileHandle");
    DECREF(fh);

    fh = Folder_Open_FileHandle(folder, &foo_boffo,
                                FH_CREATE | FH_WRITE_ONLY);
    TEST_TRUE(batch, fh && FH_Is_A(fh, FILEHANDLE),
              "Open_FileHandle for nested file");
    DECREF(fh);

    Err_set_error(NULL);
    fh = Folder_Open_FileHandle(folder, &foo, FH_CREATE | FH_WRITE_ONLY);
    TEST_TRUE(batch, fh == NULL,
              "Open_FileHandle on existing dir path fails");
    TEST_TRUE(batch, Err_get_error() != NULL,
              "Open_FileHandle on existing dir name sets Err_error");

    Err_set_error(NULL);
    fh = Folder_Open_FileHandle(folder, &foo_bar_baz_boffo,
                                FH_CREATE | FH_WRITE_ONLY);
    TEST_TRUE(batch, fh == NULL,
              "Open_FileHandle for entry within non-existent dir fails");
    TEST_TRUE(batch, Err_get_error() != NULL,
              "Open_FileHandle for entry within non-existent dir sets Err_error");

    DECREF(folder);
}

static void
test_Open_Out(TestBatch *batch) {
    Folder    *folder = (Folder*)RAMFolder_new(NULL);
    OutStream *outstream;

    Folder_MkDir(folder, &foo);

    outstream = Folder_Open_Out(folder, &boffo);
    TEST_TRUE(batch, outstream && OutStream_Is_A(outstream, OUTSTREAM),
              "Open_Out");
    DECREF(outstream);

    outstream = Folder_Open_Out(folder, &foo_boffo);
    TEST_TRUE(batch, outstream && OutStream_Is_A(outstream, OUTSTREAM),
              "Open_Out for nested file");
    DECREF(outstream);

    Err_set_error(NULL);
    outstream = Folder_Open_Out(folder, &boffo);
    TEST_TRUE(batch, outstream == NULL,
              "Open_OutStream on existing file fails");
    TEST_TRUE(batch, Err_get_error() != NULL,
              "Open_Out on existing file sets Err_error");

    Err_set_error(NULL);
    outstream = Folder_Open_Out(folder, &foo);
    TEST_TRUE(batch, outstream == NULL,
              "Open_OutStream on existing dir path fails");
    TEST_TRUE(batch, Err_get_error() != NULL,
              "Open_Out on existing dir name sets Err_error");

    Err_set_error(NULL);
    outstream = Folder_Open_Out(folder, &foo_bar_baz_boffo);
    TEST_TRUE(batch, outstream == NULL,
              "Open_Out for entry within non-existent dir fails");
    TEST_TRUE(batch, Err_get_error() != NULL,
              "Open_Out for entry within non-existent dir sets Err_error");

    DECREF(folder);
}

static void
test_Open_In(TestBatch *batch) {
    Folder     *folder = (Folder*)RAMFolder_new(NULL);
    FileHandle *fh;
    InStream   *instream;

    Folder_MkDir(folder, &foo);
    Folder_MkDir(folder, &foo_bar);
    fh = Folder_Open_FileHandle(folder, &boffo, FH_CREATE | FH_WRITE_ONLY);
    DECREF(fh);
    fh = Folder_Open_FileHandle(folder, &foo_boffo, FH_CREATE | FH_WRITE_ONLY);
    DECREF(fh);

    instream = Folder_Open_In(folder, &boffo);
    TEST_TRUE(batch, instream && InStream_Is_A(instream, INSTREAM),
              "Open_In");
    DECREF(instream);

    instream = Folder_Open_In(folder, &foo_boffo);
    TEST_TRUE(batch, instream && InStream_Is_A(instream, INSTREAM),
              "Open_In for nested file");
    DECREF(instream);

    Err_set_error(NULL);
    instream = Folder_Open_In(folder, &foo);
    TEST_TRUE(batch, instream == NULL,
              "Open_InStream on existing dir path fails");
    TEST_TRUE(batch, Err_get_error() != NULL,
              "Open_In on existing dir name sets Err_error");

    Err_set_error(NULL);
    instream = Folder_Open_In(folder, &foo_bar_baz_boffo);
    TEST_TRUE(batch, instream == NULL,
              "Open_In for entry within non-existent dir fails");
    TEST_TRUE(batch, Err_get_error() != NULL,
              "Open_In for entry within non-existent dir sets Err_error");

    DECREF(folder);
}

static void
test_Delete(TestBatch *batch) {
    Folder *folder = (Folder*)RAMFolder_new(NULL);
    FileHandle *fh;
    bool_t result;

    Folder_MkDir(folder, &foo);
    Folder_MkDir(folder, &foo_bar);
    fh = Folder_Open_FileHandle(folder, &boffo, FH_CREATE | FH_WRITE_ONLY);
    DECREF(fh);
    fh = Folder_Open_FileHandle(folder, &foo_boffo,
                                FH_CREATE | FH_WRITE_ONLY);
    DECREF(fh);

    Err_set_error(NULL);
    result = Folder_Delete(folder, &banana);
    TEST_FALSE(batch, result, "Delete on non-existent entry returns false");

    Err_set_error(NULL);
    result = Folder_Delete(folder, &foo);
    TEST_FALSE(batch, result, "Delete on non-empty dir returns false");

    TEST_TRUE(batch, Folder_Delete(folder, &foo_boffo),
              "Delete nested file");
    TEST_FALSE(batch, Folder_Exists(folder, &foo_boffo),
               "File is really gone");
    TEST_TRUE(batch, Folder_Delete(folder, &foo_bar),
              "Delete nested dir");
    TEST_FALSE(batch, Folder_Exists(folder, &foo_bar),
               "Dir is really gone");
    TEST_TRUE(batch, Folder_Delete(folder, &foo), "Delete empty dir");
    TEST_FALSE(batch, Folder_Exists(folder, &foo), "Dir is really gone");

    DECREF(folder);
}

static void
test_Delete_Tree(TestBatch *batch) {
    Folder *folder = (Folder*)RAMFolder_new(NULL);
    FileHandle *fh;
    bool_t result;

    // Create tree to be deleted.
    Folder_MkDir(folder, &foo);
    Folder_MkDir(folder, &foo_bar);
    Folder_MkDir(folder, &foo_bar_baz);
    fh = Folder_Open_FileHandle(folder, &foo_bar_baz_boffo,
                                FH_CREATE | FH_WRITE_ONLY);
    DECREF(fh);

    // Create bystanders.
    Folder_MkDir(folder, &bar);
    fh = Folder_Open_FileHandle(folder, &baz, FH_CREATE | FH_WRITE_ONLY);
    DECREF(fh);

    result = Folder_Delete_Tree(folder, &foo);
    TEST_TRUE(batch, result, "Delete_Tree() succeeded");
    TEST_FALSE(batch, Folder_Exists(folder, &foo), "Tree really gone");

    TEST_TRUE(batch, Folder_Exists(folder, &bar),
              "local dir with same name as nested dir left intact");
    TEST_TRUE(batch, Folder_Exists(folder, &baz),
              "local file with same name as nested dir left intact");

    // Kill off the bystanders.
    result = Folder_Delete_Tree(folder, &bar);
    TEST_TRUE(batch, result, "Delete_Tree() on empty dir");
    result = Folder_Delete_Tree(folder, &baz);
    TEST_TRUE(batch, result, "Delete_Tree() on file");

    // Create new tree to be deleted.
    Folder_MkDir(folder, &foo);
    Folder_MkDir(folder, &foo_bar);
    Folder_MkDir(folder, &foo_bar_baz);
    fh = Folder_Open_FileHandle(folder, &foo_bar_baz_boffo,
                                FH_CREATE | FH_WRITE_ONLY);
    DECREF(fh);

    // Remove tree in subdir.
    result = Folder_Delete_Tree(folder, &foo_bar);
    TEST_TRUE(batch, result, "Delete_Tree() of subdir succeeded");
    TEST_FALSE(batch, Folder_Exists(folder, &foo_bar),
               "subdir really gone");
    TEST_TRUE(batch, Folder_Exists(folder, &foo),
              "enclosing dir left intact");

    DECREF(folder);
}

static void
test_Slurp_File(TestBatch *batch) {
    Folder *folder = (Folder*)RAMFolder_new(NULL);
    FileHandle *fh = Folder_Open_FileHandle(folder, &foo,
                                            FH_CREATE | FH_WRITE_ONLY);
    ByteBuf *contents;

    FH_Write(fh, "stuff", 5);
    FH_Close(fh);
    DECREF(fh);
    contents = Folder_Slurp_File(folder, &foo);
    TEST_TRUE(batch, BB_Equals_Bytes(contents, "stuff", 5), "Slurp_File");

    DECREF(contents);
    DECREF(folder);
}

void
TestFolder_run_tests() {
    TestBatch *batch = TestBatch_new(79);

    TestBatch_Plan(batch);
    test_Exists(batch);
    test_Set_Path_and_Get_Path(batch);
    test_MkDir_and_Is_Directory(batch);
    test_Enclosing_Folder_and_Find_Folder(batch);
    test_List(batch);
    test_Open_Dir(batch);
    test_Open_FileHandle(batch);
    test_Open_Out(batch);
    test_Open_In(batch);
    test_Delete(batch);
    test_Delete_Tree(batch);
    test_Slurp_File(batch);

    DECREF(batch);
}


