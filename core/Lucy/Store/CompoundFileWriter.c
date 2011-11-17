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

#define C_LUCY_COMPOUNDFILEWRITER
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Store/CompoundFileWriter.h"
#include "Lucy/Store/Folder.h"
#include "Lucy/Store/InStream.h"
#include "Lucy/Store/OutStream.h"
#include "Lucy/Util/IndexFileNames.h"
#include "Lucy/Util/Json.h"

int32_t CFWriter_current_file_format = 2;

// Helper which does the heavy lifting for CFWriter_consolidate.
static void
S_do_consolidate(CompoundFileWriter *self);

// Clean up files which may be left over from previous merge attempts.
static void
S_clean_up_old_temp_files(CompoundFileWriter *self);

CompoundFileWriter*
CFWriter_new(Folder *folder) {
    CompoundFileWriter *self
        = (CompoundFileWriter*)VTable_Make_Obj(COMPOUNDFILEWRITER);
    return CFWriter_init(self, folder);
}

CompoundFileWriter*
CFWriter_init(CompoundFileWriter *self, Folder *folder) {
    self->folder = (Folder*)INCREF(folder);
    return self;
}

void
CFWriter_destroy(CompoundFileWriter *self) {
    DECREF(self->folder);
    SUPER_DESTROY(self, COMPOUNDFILEWRITER);
}

void
CFWriter_consolidate(CompoundFileWriter *self) {
    CharBuf *cfmeta_file = (CharBuf*)ZCB_WRAP_STR("cfmeta.json", 11);
    if (Folder_Exists(self->folder, cfmeta_file)) {
        THROW(ERR, "Merge already performed for %o",
              Folder_Get_Path(self->folder));
    }
    else {
        S_clean_up_old_temp_files(self);
        S_do_consolidate(self);
    }
}

static void
S_clean_up_old_temp_files(CompoundFileWriter *self) {
    Folder  *folder      = self->folder;
    CharBuf *cfmeta_temp = (CharBuf*)ZCB_WRAP_STR("cfmeta.json.temp", 16);
    CharBuf *cf_file     = (CharBuf*)ZCB_WRAP_STR("cf.dat", 6);

    if (Folder_Exists(folder, cf_file)) {
        if (!Folder_Delete(folder, cf_file)) {
            THROW(ERR, "Can't delete '%o'", cf_file);
        }
    }
    if (Folder_Exists(folder, cfmeta_temp)) {
        if (!Folder_Delete(folder, cfmeta_temp)) {
            THROW(ERR, "Can't delete '%o'", cfmeta_temp);
        }
    }
}

static void
S_do_consolidate(CompoundFileWriter *self) {
    Folder    *folder       = self->folder;
    Hash      *metadata     = Hash_new(0);
    Hash      *sub_files    = Hash_new(0);
    VArray    *files        = Folder_List(folder, NULL);
    VArray    *merged       = VA_new(VA_Get_Size(files));
    CharBuf   *cf_file      = (CharBuf*)ZCB_WRAP_STR("cf.dat", 6);
    OutStream *outstream    = Folder_Open_Out(folder, (CharBuf*)cf_file);
    uint32_t   i, max;
    bool_t     rename_success;

    if (!outstream) { RETHROW(INCREF(Err_get_error())); }

    // Start metadata.
    Hash_Store_Str(metadata, "files", 5, INCREF(sub_files));
    Hash_Store_Str(metadata, "format", 6,
                   (Obj*)CB_newf("%i32", CFWriter_current_file_format));

    CharBuf *infilepath = CB_new(30);
    size_t base_len = 0;
    VA_Sort(files, NULL, NULL);
    for (i = 0, max = VA_Get_Size(files); i < max; i++) {
        CharBuf *infilename = (CharBuf*)VA_Fetch(files, i);

        if (!CB_Ends_With_Str(infilename, ".json", 5)) {
            InStream *instream   = Folder_Open_In(folder, infilename);
            Hash     *file_data  = Hash_new(2);
            int64_t   offset, len;

            if (!instream) { RETHROW(INCREF(Err_get_error())); }

            // Absorb the file.
            offset = OutStream_Tell(outstream);
            OutStream_Absorb(outstream, instream);
            len = OutStream_Tell(outstream) - offset;

            // Record offset and length.
            Hash_Store_Str(file_data, "offset", 6,
                           (Obj*)CB_newf("%i64", offset));
            Hash_Store_Str(file_data, "length", 6,
                           (Obj*)CB_newf("%i64", len));
            CB_Set_Size(infilepath, base_len);
            CB_Cat(infilepath, infilename);
            Hash_Store(sub_files, (Obj*)infilepath, (Obj*)file_data);
            VA_Push(merged, INCREF(infilename));

            // Add filler NULL bytes so that every sub-file begins on a file
            // position multiple of 8.
            OutStream_Align(outstream, 8);

            InStream_Close(instream);
            DECREF(instream);
        }
    }
    DECREF(infilepath);

    // Write metadata to cfmeta file.
    CharBuf *cfmeta_temp = (CharBuf*)ZCB_WRAP_STR("cfmeta.json.temp", 16);
    CharBuf *cfmeta_file = (CharBuf*)ZCB_WRAP_STR("cfmeta.json", 11);
    Json_spew_json((Obj*)metadata, (Folder*)self->folder, cfmeta_temp);
    rename_success = Folder_Rename(self->folder, cfmeta_temp, cfmeta_file);
    if (!rename_success) { RETHROW(INCREF(Err_get_error())); }

    // Clean up.
    OutStream_Close(outstream);
    DECREF(outstream);
    DECREF(files);
    DECREF(metadata);
    /*
    CharBuf *merged_file;
    Obj     *ignore;
    Hash_Iterate(sub_files);
    while (Hash_Next(sub_files, (Obj**)&merged_file, &ignore)) {
        if (!Folder_Delete(folder, merged_file)) {
            CharBuf *mess = MAKE_MESS("Can't delete '%o'", merged_file);
            DECREF(sub_files);
            Err_throw_mess(ERR, mess);
        }
    }
    */
    DECREF(sub_files);
    for (uint32_t i = 0, max = VA_Get_Size(merged); i < max; i++) {
        CharBuf *merged_file = (CharBuf*)VA_Fetch(merged, i);
        if (!Folder_Delete(folder, merged_file)) {
            CharBuf *mess = MAKE_MESS("Can't delete '%o'", merged_file);
            DECREF(merged);
            Err_throw_mess(ERR, mess);
        }
    }
    DECREF(merged);
}


