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

#define C_LUCY_COMPOUNDFILEREADER
#define C_LUCY_CFREADERDIRHANDLE
#include "Lucy/Util/ToolSet.h"

#include "charmony.h"

#include "Lucy/Store/CompoundFileReader.h"
#include "Lucy/Store/CompoundFileWriter.h"
#include "Lucy/Store/FileHandle.h"
#include "Lucy/Store/InStream.h"
#include "Lucy/Util/IndexFileNames.h"
#include "Lucy/Util/Json.h"
#include "Clownfish/Util/StringHelper.h"

CompoundFileReader*
CFReader_open(Folder *folder) {
    CompoundFileReader *self
        = (CompoundFileReader*)Class_Make_Obj(COMPOUNDFILEREADER);
    return CFReader_do_open(self, folder);
}

CompoundFileReader*
CFReader_do_open(CompoundFileReader *self, Folder *folder) {
    CompoundFileReaderIVARS *const ivars = CFReader_IVARS(self);
    String *cfmeta_file = (String*)SSTR_WRAP_UTF8("cfmeta.json", 11);
    Hash *metadata = (Hash*)Json_slurp_json((Folder*)folder, cfmeta_file);
    Err *error = NULL;

    Folder_init((Folder*)self, Folder_Get_Path(folder));

    // Parse metadata file.
    if (!metadata || !Hash_Is_A(metadata, HASH)) {
        error = Err_new(Str_newf("Can't read '%o' in '%o'", cfmeta_file,
                                 Folder_Get_Path(folder)));
    }
    else {
        Obj *format = Hash_Fetch_Utf8(metadata, "format", 6);
        ivars->format = format ? (int32_t)Obj_To_I64(format) : 0;
        ivars->records = (Hash*)INCREF(Hash_Fetch_Utf8(metadata, "files", 5));
        if (ivars->format < 1) {
            error = Err_new(Str_newf("Corrupt %o file: Missing or invalid 'format'",
                                     cfmeta_file));
        }
        else if (ivars->format > CFWriter_current_file_format) {
            error = Err_new(Str_newf("Unsupported compound file format: %i32 "
                                     "(current = %i32", ivars->format,
                                     CFWriter_current_file_format));
        }
        else if (!ivars->records) {
            error = Err_new(Str_newf("Corrupt %o file: missing 'files' key",
                                     cfmeta_file));
        }
    }
    DECREF(metadata);
    if (error) {
        Err_set_error(error);
        DECREF(self);
        return NULL;
    }

    // Open an instream which we'll clone over and over.
    String *cf_file = (String*)SSTR_WRAP_UTF8("cf.dat", 6);
    ivars->instream = Folder_Open_In(folder, cf_file);
    if (!ivars->instream) {
        ERR_ADD_FRAME(Err_get_error());
        DECREF(self);
        return NULL;
    }

    // Assign.
    ivars->real_folder = (Folder*)INCREF(folder);

    // Strip directory name from filepaths for old format.
    if (ivars->format == 1) {
        VArray *files = Hash_Keys(ivars->records);
        String *folder_name = IxFileNames_local_part(Folder_Get_Path(folder));
        size_t folder_name_len = Str_Length(folder_name);

        for (uint32_t i = 0, max = VA_Get_Size(files); i < max; i++) {
            String *orig = (String*)VA_Fetch(files, i);
            if (Str_Starts_With(orig, folder_name)) {
                Obj *record = Hash_Delete(ivars->records, (Obj*)orig);
                size_t offset = folder_name_len + sizeof(CHY_DIR_SEP) - 1;
                size_t len    = Str_Length(orig) - offset;
                String *filename = Str_SubString(orig, offset, len);
                Hash_Store(ivars->records, (Obj*)filename, (Obj*)record);
                DECREF(filename);
            }
        }

        DECREF(folder_name);
        DECREF(files);
    }

    return self;
}

void
CFReader_Destroy_IMP(CompoundFileReader *self) {
    CompoundFileReaderIVARS *const ivars = CFReader_IVARS(self);
    DECREF(ivars->real_folder);
    DECREF(ivars->instream);
    DECREF(ivars->records);
    SUPER_DESTROY(self, COMPOUNDFILEREADER);
}

Folder*
CFReader_Get_Real_Folder_IMP(CompoundFileReader *self) {
    return CFReader_IVARS(self)->real_folder;
}

void
CFReader_Set_Path_IMP(CompoundFileReader *self, String *path) {
    CompoundFileReaderIVARS *const ivars = CFReader_IVARS(self);
    Folder_Set_Path(ivars->real_folder, path);
    CFReader_Set_Path_t super_set_path
        = (CFReader_Set_Path_t)SUPER_METHOD_PTR(COMPOUNDFILEREADER,
                                                LUCY_CFReader_Set_Path);
    super_set_path(self, path);
}

FileHandle*
CFReader_Local_Open_FileHandle_IMP(CompoundFileReader *self,
                                   String *name, uint32_t flags) {
    CompoundFileReaderIVARS *const ivars = CFReader_IVARS(self);
    Hash *entry = (Hash*)Hash_Fetch(ivars->records, (Obj*)name);
    FileHandle *fh = NULL;

    if (entry) {
        Err_set_error(Err_new(Str_newf("Can't open FileHandle for virtual file %o in '%o'",
                                       name, ivars->path)));
    }
    else {
        fh = Folder_Local_Open_FileHandle(ivars->real_folder, name, flags);
        if (!fh) {
            ERR_ADD_FRAME(Err_get_error());
        }
    }

    return fh;
}

bool
CFReader_Local_Delete_IMP(CompoundFileReader *self, String *name) {
    CompoundFileReaderIVARS *const ivars = CFReader_IVARS(self);
    Hash *record = (Hash*)Hash_Delete(ivars->records, (Obj*)name);
    DECREF(record);

    if (record == NULL) {
        return Folder_Local_Delete(ivars->real_folder, name);
    }
    else {
        // Once the number of virtual files falls to 0, remove the compound
        // files.
        if (Hash_Get_Size(ivars->records) == 0) {
            String *cf_file = (String*)SSTR_WRAP_UTF8("cf.dat", 6);
            if (!Folder_Delete(ivars->real_folder, cf_file)) {
                return false;
            }
            String *cfmeta_file = (String*)SSTR_WRAP_UTF8("cfmeta.json", 11);
            if (!Folder_Delete(ivars->real_folder, cfmeta_file)) {
                return false;

            }
        }
        return true;
    }
}

InStream*
CFReader_Local_Open_In_IMP(CompoundFileReader *self, String *name) {
    CompoundFileReaderIVARS *const ivars = CFReader_IVARS(self);
    Hash *entry = (Hash*)Hash_Fetch(ivars->records, (Obj*)name);

    if (!entry) {
        InStream *instream = Folder_Local_Open_In(ivars->real_folder, name);
        if (!instream) {
            ERR_ADD_FRAME(Err_get_error());
        }
        return instream;
    }
    else {
        Obj *len    = Hash_Fetch_Utf8(entry, "length", 6);
        Obj *offset = Hash_Fetch_Utf8(entry, "offset", 6);
        if (!len || !offset) {
            Err_set_error(Err_new(Str_newf("Malformed entry for '%o' in '%o'",
                                           name, Folder_Get_Path(ivars->real_folder))));
            return NULL;
        }
        else if (Str_Get_Size(ivars->path)) {
            String *fullpath = Str_newf("%o/%o", ivars->path, name);
            InStream *instream = InStream_Reopen(ivars->instream, fullpath,
                                                 Obj_To_I64(offset), Obj_To_I64(len));
            DECREF(fullpath);
            return instream;
        }
        else {
            return InStream_Reopen(ivars->instream, name, Obj_To_I64(offset),
                                   Obj_To_I64(len));
        }
    }
}

bool
CFReader_Local_Exists_IMP(CompoundFileReader *self, String *name) {
    CompoundFileReaderIVARS *const ivars = CFReader_IVARS(self);
    if (Hash_Fetch(ivars->records, (Obj*)name))        { return true; }
    if (Folder_Local_Exists(ivars->real_folder, name)) { return true; }
    return false;
}

bool
CFReader_Local_Is_Directory_IMP(CompoundFileReader *self,
                                String *name) {
    CompoundFileReaderIVARS *const ivars = CFReader_IVARS(self);
    if (Hash_Fetch(ivars->records, (Obj*)name))              { return false; }
    if (Folder_Local_Is_Directory(ivars->real_folder, name)) { return true; }
    return false;
}

void
CFReader_Close_IMP(CompoundFileReader *self) {
    CompoundFileReaderIVARS *const ivars = CFReader_IVARS(self);
    InStream_Close(ivars->instream);
}

bool
CFReader_Local_MkDir_IMP(CompoundFileReader *self, String *name) {
    CompoundFileReaderIVARS *const ivars = CFReader_IVARS(self);
    if (Hash_Fetch(ivars->records, (Obj*)name)) {
        Err_set_error(Err_new(Str_newf("Can't MkDir: '%o' exists", name)));
        return false;
    }
    else {
        bool result = Folder_Local_MkDir(ivars->real_folder, name);
        if (!result) { ERR_ADD_FRAME(Err_get_error()); }
        return result;
    }
}

Folder*
CFReader_Local_Find_Folder_IMP(CompoundFileReader *self,
                               String *name) {
    CompoundFileReaderIVARS *const ivars = CFReader_IVARS(self);
    if (Hash_Fetch(ivars->records, (Obj*)name)) { return false; }
    return Folder_Local_Find_Folder(ivars->real_folder, name);
}

DirHandle*
CFReader_Local_Open_Dir_IMP(CompoundFileReader *self) {
    return (DirHandle*)CFReaderDH_new(self);
}

/****************************************************************************/

CFReaderDirHandle*
CFReaderDH_new(CompoundFileReader *cf_reader) {
    CFReaderDirHandle *self
        = (CFReaderDirHandle*)Class_Make_Obj(CFREADERDIRHANDLE);
    return CFReaderDH_init(self, cf_reader);
}

CFReaderDirHandle*
CFReaderDH_init(CFReaderDirHandle *self, CompoundFileReader *cf_reader) {
    DH_init((DirHandle*)self, CFReader_Get_Path(cf_reader));
    CFReaderDirHandleIVARS *const ivars = CFReaderDH_IVARS(self);
    ivars->cf_reader = (CompoundFileReader*)INCREF(cf_reader);

    Hash *cf_records = CFReader_IVARS(ivars->cf_reader)->records;
    ivars->elems  = Hash_Keys(cf_records);
    ivars->tick   = -1;
    // Accumulate entries from real Folder.
    Folder *real_folder = CFReader_Get_Real_Folder(ivars->cf_reader);
    DirHandle *dh = Folder_Local_Open_Dir(real_folder);
    while (DH_Next(dh)) {
        String *entry = DH_Get_Entry(dh);
        VA_Push(ivars->elems, (Obj*)Str_Clone(entry));
        DECREF(entry);
    }
    DECREF(dh);
    return self;
}

bool
CFReaderDH_Close_IMP(CFReaderDirHandle *self) {
    CFReaderDirHandleIVARS *const ivars = CFReaderDH_IVARS(self);
    if (ivars->elems) {
        DECREF(ivars->elems);
        ivars->elems = NULL;
    }
    if (ivars->cf_reader) {
        DECREF(ivars->cf_reader);
        ivars->cf_reader = NULL;
    }
    return true;
}

bool
CFReaderDH_Next_IMP(CFReaderDirHandle *self) {
    CFReaderDirHandleIVARS *const ivars = CFReaderDH_IVARS(self);
    if (ivars->elems) {
        ivars->tick++;
        if (ivars->tick < (int32_t)VA_Get_Size(ivars->elems)) {
            String *path = (String*)CERTIFY(
                                VA_Fetch(ivars->elems, ivars->tick), STRING);
            DECREF(ivars->entry);
            ivars->entry = (String*)INCREF(path);
            return true;
        }
        else {
            ivars->tick--;
            return false;
        }
    }
    return false;
}

bool
CFReaderDH_Entry_Is_Dir_IMP(CFReaderDirHandle *self) {
    CFReaderDirHandleIVARS *const ivars = CFReaderDH_IVARS(self);
    if (ivars->elems) {
        String *name = (String*)VA_Fetch(ivars->elems, ivars->tick);
        if (name) {
            return CFReader_Local_Is_Directory(ivars->cf_reader, name);
        }
    }
    return false;
}


