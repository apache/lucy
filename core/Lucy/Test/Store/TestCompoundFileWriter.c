#define C_LUCY_CHARBUF
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Test.h"
#include "Lucy/Test/Store/TestCompoundFileWriter.h"
#include "Lucy/Store/CompoundFileWriter.h"
#include "Lucy/Store/FileHandle.h"
#include "Lucy/Store/OutStream.h"
#include "Lucy/Store/RAMFolder.h"
#include "Lucy/Util/Json.h"

static CharBuf cfmeta_file = ZCB_LITERAL("cfmeta.json");
static CharBuf cfmeta_temp = ZCB_LITERAL("cfmeta.json.temp");
static CharBuf cf_file     = ZCB_LITERAL("cf.dat");
static CharBuf foo         = ZCB_LITERAL("foo");
static CharBuf bar         = ZCB_LITERAL("bar");
static CharBuf seg_1       = ZCB_LITERAL("seg_1");

static Folder*
S_folder_with_contents()
{
    RAMFolder *folder  = RAMFolder_new(&seg_1);
    OutStream *foo_out = RAMFolder_Open_Out(folder, &foo);
    OutStream *bar_out = RAMFolder_Open_Out(folder, &bar);
    OutStream_Write_Bytes(foo_out, "foo", 3);
    OutStream_Write_Bytes(bar_out, "bar", 3);
    OutStream_Close(foo_out);
    OutStream_Close(bar_out);
    DECREF(foo_out);
    DECREF(bar_out);
    return (Folder*)folder;
}

static void
test_Consolidate(TestBatch *batch)
{
    Folder *folder = S_folder_with_contents();
    FileHandle *fh;
    
    /* Fake up detritus from failed consolidation. */
    fh = Folder_Open_FileHandle(folder, &cf_file, 
        FH_CREATE | FH_WRITE_ONLY | FH_EXCLUSIVE );
    DECREF(fh);
    fh = Folder_Open_FileHandle(folder, &cfmeta_temp, 
        FH_CREATE | FH_WRITE_ONLY | FH_EXCLUSIVE );
    DECREF(fh);

    {
        CompoundFileWriter *cf_writer = CFWriter_new(folder);
        CFWriter_Consolidate(cf_writer);
        PASS(batch, "Consolidate completes despite leftover files");
        DECREF(cf_writer);
    }

    ASSERT_TRUE(batch, Folder_Exists(folder, &cf_file), 
        "cf.dat file written"); 
    ASSERT_TRUE(batch, Folder_Exists(folder, &cfmeta_file), 
        "cfmeta.json file written"); 
    ASSERT_FALSE(batch, Folder_Exists(folder, &foo), 
        "original file zapped");
    ASSERT_FALSE(batch, Folder_Exists(folder, &cfmeta_temp), 
        "detritus from failed consolidation zapped");

    DECREF(folder);
}

static void
test_offsets(TestBatch *batch)
{
    Folder *folder = S_folder_with_contents();
    CompoundFileWriter *cf_writer = CFWriter_new(folder);
    Hash    *cf_metadata;
    Hash    *files;

    CFWriter_Consolidate(cf_writer);

    cf_metadata = (Hash*)CERTIFY(
        Json_slurp_json(folder, &cfmeta_file), HASH);
    files = (Hash*)CERTIFY(
        Hash_Fetch_Str(cf_metadata, "files", 5), HASH);
    {
        CharBuf *file;
        Obj     *filestats;
        bool_t   offsets_ok = true;

        ASSERT_TRUE(batch, Hash_Get_Size(files) > 0, "Multiple files");

        Hash_Iter_Init(files);
        while (Hash_Iter_Next(files, (Obj**)&file, &filestats)) {
            Hash *stats = (Hash*)CERTIFY(filestats, HASH);
            Obj *offset = CERTIFY(Hash_Fetch_Str(stats, "offset", 6), OBJ);
            if (Obj_To_I64(offset) % 8 != 0) {
                offsets_ok = false;
                FAIL(batch, "Offset %o for %o not a multiple of 8: %o",
                    offset, file);
                break;
            }
        }
        if (offsets_ok) {
            PASS(batch, "All offsets are multiples of 8");
        }
    }

    DECREF(cf_metadata);
    DECREF(cf_writer);
    DECREF(folder);
}

void
TestCFWriter_run_tests()
{
    TestBatch *batch = Test_new_batch("TestCompoundFileWriter", 7, NULL);

    PLAN(batch);
    test_Consolidate(batch);
    test_offsets(batch);

    batch->destroy(batch);
}

/* Copyright 2009 The Apache Software Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

