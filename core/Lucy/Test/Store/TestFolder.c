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
#include "Lucy/Test/Store/TestFolder.h"
#include "Lucy/Store/DirHandle.h"
#include "Lucy/Store/FileHandle.h"
#include "Lucy/Store/InStream.h"
#include "Lucy/Store/OutStream.h"
#include "Lucy/Store/RAMFolder.h"

static String *foo               = NULL;
static String *bar               = NULL;
static String *baz               = NULL;
static String *boffo             = NULL;
static String *banana            = NULL;
static String *foo_bar           = NULL;
static String *foo_bar_baz       = NULL;
static String *foo_bar_baz_boffo = NULL;
static String *foo_boffo         = NULL;
static String *foo_foo           = NULL;
static String *nope              = NULL;

TestFolder*
TestFolder_new() {
    return (TestFolder*)Class_Make_Obj(TESTFOLDER);
}

static void
S_init_strings(void) {
    foo               = Str_newf("foo");
    bar               = Str_newf("bar");
    baz               = Str_newf("baz");
    boffo             = Str_newf("boffo");
    banana            = Str_newf("banana");
    foo_bar           = Str_newf("foo/bar");
    foo_bar_baz       = Str_newf("foo/bar/baz");
    foo_bar_baz_boffo = Str_newf("foo/bar/baz/boffo");
    foo_boffo         = Str_newf("foo/boffo");
    foo_foo           = Str_newf("foo/foo");
    nope              = Str_newf("nope");
}

static void
S_destroy_strings(void) {
    DECREF(foo);
    DECREF(bar);
    DECREF(baz);
    DECREF(boffo);
    DECREF(banana);
    DECREF(foo_bar);
    DECREF(foo_bar_baz);
    DECREF(foo_bar_baz_boffo);
    DECREF(foo_boffo);
    DECREF(foo_foo);
    DECREF(nope);
}

static void
test_Exists(TestBatchRunner *runner) {
    Folder *folder = (Folder*)RAMFolder_new(NULL);
    FileHandle *fh;

    Folder_MkDir(folder, foo);
    Folder_MkDir(folder, foo_bar);
    fh = Folder_Open_FileHandle(folder, boffo, FH_CREATE | FH_WRITE_ONLY);
    DECREF(fh);
    fh = Folder_Open_FileHandle(folder, foo_boffo,
                                FH_CREATE | FH_WRITE_ONLY);
    DECREF(fh);

    TEST_TRUE(runner, Folder_Exists(folder, foo), "Dir exists");
    TEST_TRUE(runner, Folder_Exists(folder, boffo), "File exists");
    TEST_TRUE(runner, Folder_Exists(folder, foo_bar),
              "Nested dir exists");
    TEST_TRUE(runner, Folder_Exists(folder, foo_boffo),
              "Nested file exists");

    TEST_FALSE(runner, Folder_Exists(folder, banana),
               "Non-existent entry");
    TEST_FALSE(runner, Folder_Exists(folder, foo_foo),
               "Non-existent nested entry");

    DECREF(folder);
}

static void
test_Set_Path_and_Get_Path(TestBatchRunner *runner) {
    Folder *folder = (Folder*)RAMFolder_new(foo);
    TEST_TRUE(runner, Str_Equals(Folder_Get_Path(folder), (Obj*)foo),
              "Get_Path");
    Folder_Set_Path(folder, bar);
    TEST_TRUE(runner, Str_Equals(Folder_Get_Path(folder), (Obj*)bar),
              "Set_Path");
    DECREF(folder);
}

static void
test_MkDir_and_Is_Directory(TestBatchRunner *runner) {
    Folder *folder = (Folder*)RAMFolder_new(NULL);
    FileHandle *fh;

    TEST_FALSE(runner, Folder_Is_Directory(folder, foo),
               "Is_Directory() false for non-existent entry");

    TEST_TRUE(runner, Folder_MkDir(folder, foo),
              "MkDir returns true on success");
    TEST_TRUE(runner, Folder_Is_Directory(folder, foo),
              "Is_Directory() true for local folder");

    TEST_FALSE(runner, Folder_Is_Directory(folder, foo_bar_baz),
               "Is_Directory() false for non-existent deeply nested dir");
    Err_set_error(NULL);
    TEST_FALSE(runner, Folder_MkDir(folder, foo_bar_baz),
               "MkDir for deeply nested dir fails");
    TEST_TRUE(runner, Err_get_error() != NULL,
              "MkDir for deeply nested dir sets Err_error");

    TEST_TRUE(runner, Folder_MkDir(folder, foo_bar),
              "MkDir for nested dir");
    TEST_TRUE(runner, Folder_Is_Directory(folder, foo_bar),
              "Is_Directory() true for nested dir");

    Err_set_error(NULL);
    TEST_FALSE(runner, Folder_MkDir(folder, foo_bar),
               "Overwrite dir with MkDir fails");
    TEST_TRUE(runner, Err_get_error() != NULL,
              "Overwrite dir with MkDir sets Err_error");

    fh = Folder_Open_FileHandle(folder, foo_boffo,
                                FH_CREATE | FH_WRITE_ONLY);
    DECREF(fh);
    Err_set_error(NULL);
    TEST_FALSE(runner, Folder_MkDir(folder, foo_boffo),
               "Overwrite file with MkDir fails");
    TEST_TRUE(runner, Err_get_error() != NULL,
              "Overwrite file with MkDir sets Err_error");
    TEST_FALSE(runner, Folder_Is_Directory(folder, foo_boffo),
               "Is_Directory() false for nested file");

    DECREF(folder);
}

static void
test_Enclosing_Folder_and_Find_Folder(TestBatchRunner *runner) {
    Folder *folder = (Folder*)RAMFolder_new(NULL);
    FileHandle *fh;

    Folder_MkDir(folder, foo);
    Folder_MkDir(folder, foo_bar);
    Folder_MkDir(folder, foo_bar_baz);
    fh = Folder_Open_FileHandle(folder, foo_bar_baz_boffo,
                                FH_CREATE | FH_WRITE_ONLY);

    {
        Folder *encloser = Folder_Enclosing_Folder(folder, (String*)nope);
        Folder *found = Folder_Find_Folder(folder, (String*)nope);
        TEST_TRUE(runner, encloser == folder,
                  "Enclosing_Folder() - non-existent entry yields parent");
        TEST_TRUE(runner, found == NULL,
                  "Find_Folder() - non-existent entry yields NULL");
    }

    {
        Folder *encloser = Folder_Enclosing_Folder(folder, foo_bar);
        Folder *found = Folder_Find_Folder(folder, foo_bar);
        TEST_TRUE(runner,
                  encloser
                  && Folder_Is_A(encloser, FOLDER)
                  && Str_Ends_With(Folder_Get_Path(encloser), foo),
                  "Enclosing_Folder() - find one directory down");
        TEST_TRUE(runner,
                  found
                  && Folder_Is_A(found, FOLDER)
                  && Str_Ends_With(Folder_Get_Path(found), bar),
                  "Find_Folder() - 'foo/bar'");
    }

    {
        Folder *encloser = Folder_Enclosing_Folder(folder, foo_bar_baz);
        Folder *found = Folder_Find_Folder(folder, foo_bar_baz);
        TEST_TRUE(runner,
                  encloser
                  && Folder_Is_A(encloser, FOLDER)
                  && Str_Ends_With(Folder_Get_Path(encloser), bar),
                  "Find two directories down");
        TEST_TRUE(runner,
                  found
                  && Folder_Is_A(found, FOLDER)
                  && Str_Ends_With(Folder_Get_Path(found), baz),
                  "Find_Folder() - 'foo/bar/baz'");
    }

    {
        Folder *encloser
            = Folder_Enclosing_Folder(folder, foo_bar_baz_boffo);
        Folder *found = Folder_Find_Folder(folder, foo_bar_baz_boffo);
        TEST_TRUE(runner,
                  encloser
                  && Folder_Is_A(encloser, FOLDER)
                  && Str_Ends_With(Folder_Get_Path(encloser), baz),
                  "Recurse to find a directory containing a real file");
        TEST_TRUE(runner, found == NULL,
                  "Find_Folder() - file instead of folder yields NULL");
    }

    DECREF(fh);
    DECREF(folder);
}

static void
test_List(TestBatchRunner *runner) {
    Folder     *folder = (Folder*)RAMFolder_new(NULL);
    FileHandle *fh;
    VArray     *list;
    String     *elem;

    Folder_MkDir(folder, foo);
    Folder_MkDir(folder, foo_bar);
    Folder_MkDir(folder, foo_bar_baz);
    fh = Folder_Open_FileHandle(folder, boffo, FH_CREATE | FH_WRITE_ONLY);
    DECREF(fh);
    fh = Folder_Open_FileHandle(folder, banana, FH_CREATE | FH_WRITE_ONLY);
    DECREF(fh);

    list = Folder_List(folder, NULL);
    VA_Sort(list, NULL, NULL);
    TEST_INT_EQ(runner, VA_Get_Size(list), 3, "List");
    elem = (String*)DOWNCAST(VA_Fetch(list, 0), STRING);
    TEST_TRUE(runner, elem && Str_Equals(elem, (Obj*)banana),
              "List first file");
    elem = (String*)DOWNCAST(VA_Fetch(list, 1), STRING);
    TEST_TRUE(runner, elem && Str_Equals(elem, (Obj*)boffo),
              "List second file");
    elem = (String*)DOWNCAST(VA_Fetch(list, 2), STRING);
    TEST_TRUE(runner, elem && Str_Equals(elem, (Obj*)foo), "List dir");
    DECREF(list);

    list = Folder_List(folder, foo_bar);
    TEST_INT_EQ(runner, VA_Get_Size(list), 1, "List subdirectory contents");
    elem = (String*)DOWNCAST(VA_Fetch(list, 0), STRING);
    TEST_TRUE(runner, elem && Str_Equals(elem, (Obj*)baz),
              "Just the filename");
    DECREF(list);

    DECREF(folder);
}

static void
test_Open_Dir(TestBatchRunner *runner) {
    Folder     *folder = (Folder*)RAMFolder_new(NULL);
    DirHandle  *dh;

    Folder_MkDir(folder, foo);
    Folder_MkDir(folder, foo_bar);

    dh = Folder_Open_Dir(folder, foo);
    TEST_TRUE(runner, dh && DH_Is_A(dh, DIRHANDLE), "Open_Dir");
    DECREF(dh);
    dh = Folder_Open_Dir(folder, foo_bar);
    TEST_TRUE(runner, dh && DH_Is_A(dh, DIRHANDLE), "Open_Dir nested dir");
    DECREF(dh);

    Err_set_error(NULL);
    dh = Folder_Open_Dir(folder, bar);
    TEST_TRUE(runner, dh == NULL,
              "Open_Dir on non-existent entry fails");
    TEST_TRUE(runner, Err_get_error() != NULL,
              "Open_Dir on non-existent entry sets Err_error");

    Err_set_error(NULL);
    dh = Folder_Open_Dir(folder, foo_foo);
    TEST_TRUE(runner, dh == NULL,
              "Open_Dir on non-existent nested entry fails");
    TEST_TRUE(runner, Err_get_error() != NULL,
              "Open_Dir on non-existent nested entry sets Err_error");

    DECREF(folder);
}

static void
test_Open_FileHandle(TestBatchRunner *runner) {
    Folder     *folder = (Folder*)RAMFolder_new(NULL);
    FileHandle *fh;

    Folder_MkDir(folder, foo);

    fh = Folder_Open_FileHandle(folder, boffo, FH_CREATE | FH_WRITE_ONLY);
    TEST_TRUE(runner, fh && FH_Is_A(fh, FILEHANDLE), "Open_FileHandle");
    DECREF(fh);

    fh = Folder_Open_FileHandle(folder, foo_boffo,
                                FH_CREATE | FH_WRITE_ONLY);
    TEST_TRUE(runner, fh && FH_Is_A(fh, FILEHANDLE),
              "Open_FileHandle for nested file");
    DECREF(fh);

    Err_set_error(NULL);
    fh = Folder_Open_FileHandle(folder, foo, FH_CREATE | FH_WRITE_ONLY);
    TEST_TRUE(runner, fh == NULL,
              "Open_FileHandle on existing dir path fails");
    TEST_TRUE(runner, Err_get_error() != NULL,
              "Open_FileHandle on existing dir name sets Err_error");

    Err_set_error(NULL);
    fh = Folder_Open_FileHandle(folder, foo_bar_baz_boffo,
                                FH_CREATE | FH_WRITE_ONLY);
    TEST_TRUE(runner, fh == NULL,
              "Open_FileHandle for entry within non-existent dir fails");
    TEST_TRUE(runner, Err_get_error() != NULL,
              "Open_FileHandle for entry within non-existent dir sets Err_error");

    DECREF(folder);
}

static void
test_Open_Out(TestBatchRunner *runner) {
    Folder    *folder = (Folder*)RAMFolder_new(NULL);
    OutStream *outstream;

    Folder_MkDir(folder, foo);

    outstream = Folder_Open_Out(folder, boffo);
    TEST_TRUE(runner, outstream && OutStream_Is_A(outstream, OUTSTREAM),
              "Open_Out");
    DECREF(outstream);

    outstream = Folder_Open_Out(folder, foo_boffo);
    TEST_TRUE(runner, outstream && OutStream_Is_A(outstream, OUTSTREAM),
              "Open_Out for nested file");
    DECREF(outstream);

    Err_set_error(NULL);
    outstream = Folder_Open_Out(folder, boffo);
    TEST_TRUE(runner, outstream == NULL,
              "Open_OutStream on existing file fails");
    TEST_TRUE(runner, Err_get_error() != NULL,
              "Open_Out on existing file sets Err_error");

    Err_set_error(NULL);
    outstream = Folder_Open_Out(folder, foo);
    TEST_TRUE(runner, outstream == NULL,
              "Open_OutStream on existing dir path fails");
    TEST_TRUE(runner, Err_get_error() != NULL,
              "Open_Out on existing dir name sets Err_error");

    Err_set_error(NULL);
    outstream = Folder_Open_Out(folder, foo_bar_baz_boffo);
    TEST_TRUE(runner, outstream == NULL,
              "Open_Out for entry within non-existent dir fails");
    TEST_TRUE(runner, Err_get_error() != NULL,
              "Open_Out for entry within non-existent dir sets Err_error");

    DECREF(folder);
}

static void
test_Open_In(TestBatchRunner *runner) {
    Folder     *folder = (Folder*)RAMFolder_new(NULL);
    FileHandle *fh;
    InStream   *instream;

    Folder_MkDir(folder, foo);
    Folder_MkDir(folder, foo_bar);
    fh = Folder_Open_FileHandle(folder, boffo, FH_CREATE | FH_WRITE_ONLY);
    DECREF(fh);
    fh = Folder_Open_FileHandle(folder, foo_boffo, FH_CREATE | FH_WRITE_ONLY);
    DECREF(fh);

    instream = Folder_Open_In(folder, boffo);
    TEST_TRUE(runner, instream && InStream_Is_A(instream, INSTREAM),
              "Open_In");
    DECREF(instream);

    instream = Folder_Open_In(folder, foo_boffo);
    TEST_TRUE(runner, instream && InStream_Is_A(instream, INSTREAM),
              "Open_In for nested file");
    DECREF(instream);

    Err_set_error(NULL);
    instream = Folder_Open_In(folder, foo);
    TEST_TRUE(runner, instream == NULL,
              "Open_InStream on existing dir path fails");
    TEST_TRUE(runner, Err_get_error() != NULL,
              "Open_In on existing dir name sets Err_error");

    Err_set_error(NULL);
    instream = Folder_Open_In(folder, foo_bar_baz_boffo);
    TEST_TRUE(runner, instream == NULL,
              "Open_In for entry within non-existent dir fails");
    TEST_TRUE(runner, Err_get_error() != NULL,
              "Open_In for entry within non-existent dir sets Err_error");

    DECREF(folder);
}

static void
test_Delete(TestBatchRunner *runner) {
    Folder *folder = (Folder*)RAMFolder_new(NULL);
    FileHandle *fh;
    bool result;

    Folder_MkDir(folder, foo);
    Folder_MkDir(folder, foo_bar);
    fh = Folder_Open_FileHandle(folder, boffo, FH_CREATE | FH_WRITE_ONLY);
    DECREF(fh);
    fh = Folder_Open_FileHandle(folder, foo_boffo,
                                FH_CREATE | FH_WRITE_ONLY);
    DECREF(fh);

    Err_set_error(NULL);
    result = Folder_Delete(folder, banana);
    TEST_FALSE(runner, result, "Delete on non-existent entry returns false");

    Err_set_error(NULL);
    result = Folder_Delete(folder, foo);
    TEST_FALSE(runner, result, "Delete on non-empty dir returns false");

    TEST_TRUE(runner, Folder_Delete(folder, foo_boffo),
              "Delete nested file");
    TEST_FALSE(runner, Folder_Exists(folder, foo_boffo),
               "File is really gone");
    TEST_TRUE(runner, Folder_Delete(folder, foo_bar),
              "Delete nested dir");
    TEST_FALSE(runner, Folder_Exists(folder, foo_bar),
               "Dir is really gone");
    TEST_TRUE(runner, Folder_Delete(folder, foo), "Delete empty dir");
    TEST_FALSE(runner, Folder_Exists(folder, foo), "Dir is really gone");

    DECREF(folder);
}

static void
test_Delete_Tree(TestBatchRunner *runner) {
    Folder *folder = (Folder*)RAMFolder_new(NULL);
    FileHandle *fh;
    bool result;

    // Create tree to be deleted.
    Folder_MkDir(folder, foo);
    Folder_MkDir(folder, foo_bar);
    Folder_MkDir(folder, foo_bar_baz);
    fh = Folder_Open_FileHandle(folder, foo_bar_baz_boffo,
                                FH_CREATE | FH_WRITE_ONLY);
    DECREF(fh);

    // Create bystanders.
    Folder_MkDir(folder, bar);
    fh = Folder_Open_FileHandle(folder, baz, FH_CREATE | FH_WRITE_ONLY);
    DECREF(fh);

    result = Folder_Delete_Tree(folder, foo);
    TEST_TRUE(runner, result, "Delete_Tree() succeeded");
    TEST_FALSE(runner, Folder_Exists(folder, foo), "Tree really gone");

    TEST_TRUE(runner, Folder_Exists(folder, bar),
              "local dir with same name as nested dir left intact");
    TEST_TRUE(runner, Folder_Exists(folder, baz),
              "local file with same name as nested dir left intact");

    // Kill off the bystanders.
    result = Folder_Delete_Tree(folder, bar);
    TEST_TRUE(runner, result, "Delete_Tree() on empty dir");
    result = Folder_Delete_Tree(folder, baz);
    TEST_TRUE(runner, result, "Delete_Tree() on file");

    // Create new tree to be deleted.
    Folder_MkDir(folder, foo);
    Folder_MkDir(folder, foo_bar);
    Folder_MkDir(folder, foo_bar_baz);
    fh = Folder_Open_FileHandle(folder, foo_bar_baz_boffo,
                                FH_CREATE | FH_WRITE_ONLY);
    DECREF(fh);

    // Remove tree in subdir.
    result = Folder_Delete_Tree(folder, foo_bar);
    TEST_TRUE(runner, result, "Delete_Tree() of subdir succeeded");
    TEST_FALSE(runner, Folder_Exists(folder, foo_bar),
               "subdir really gone");
    TEST_TRUE(runner, Folder_Exists(folder, foo),
              "enclosing dir left intact");

    DECREF(folder);
}

static void
test_Slurp_File(TestBatchRunner *runner) {
    Folder *folder = (Folder*)RAMFolder_new(NULL);
    FileHandle *fh = Folder_Open_FileHandle(folder, foo,
                                            FH_CREATE | FH_WRITE_ONLY);
    ByteBuf *contents;

    FH_Write(fh, "stuff", 5);
    FH_Close(fh);
    DECREF(fh);
    contents = Folder_Slurp_File(folder, foo);
    TEST_TRUE(runner, BB_Equals_Bytes(contents, "stuff", 5), "Slurp_File");

    DECREF(contents);
    DECREF(folder);
}

void
TestFolder_Run_IMP(TestFolder *self, TestBatchRunner *runner) {
    TestBatchRunner_Plan(runner, (TestBatch*)self, 79);
    S_init_strings();
    test_Exists(runner);
    test_Set_Path_and_Get_Path(runner);
    test_MkDir_and_Is_Directory(runner);
    test_Enclosing_Folder_and_Find_Folder(runner);
    test_List(runner);
    test_Open_Dir(runner);
    test_Open_FileHandle(runner);
    test_Open_Out(runner);
    test_Open_In(runner);
    test_Delete(runner);
    test_Delete_Tree(runner);
    test_Slurp_File(runner);
    S_destroy_strings();
}

