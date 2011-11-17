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

#include "Lucy/Store/CompoundFileReader.h"
#include "Lucy/Store/CompoundFileWriter.h"
#include "Lucy/Store/FileHandle.h"
#include "Lucy/Store/InStream.h"
#include "Lucy/Util/IndexFileNames.h"
#include "Lucy/Util/Json.h"
#include "Lucy/Util/StringHelper.h"

CompoundFileReader*
CFReader_open(Folder *folder) {
    CompoundFileReader *self
        = (CompoundFileReader*)VTable_Make_Obj(COMPOUNDFILEREADER);
    return CFReader_do_open(self, folder);
}

CompoundFileReader*
CFReader_do_open(CompoundFileReader *self, Folder *folder) {
    CharBuf *cfmeta_file = (CharBuf*)ZCB_WRAP_STR("cfmeta.json", 11);
    Hash *metadata = (Hash*)Json_slurp_json((Folder*)folder, cfmeta_file);
    Err *error = NULL;

    Folder_init((Folder*)self, Folder_Get_Path(folder));

    // Parse metadata file.
    if (!metadata || !Hash_Is_A(metadata, HASH)) {
        error = Err_new(CB_newf("Can't read '%o' in '%o'", cfmeta_file,
                                Folder_Get_Path(folder)));
    }
    else {
        Obj *format = Hash_Fetch_Str(metadata, "format", 6);
        self->format = format ? (int32_t)Obj_To_I64(format) : 0;
        self->records = (Hash*)INCREF(Hash_Fetch_Str(metadata, "files", 5));
        if (self->format < 1) {
            error = Err_new(CB_newf("Corrupt %o file: Missing or invalid 'format'",
                                    cfmeta_file));
        }
        else if (self->format > CFWriter_current_file_format) {
            error = Err_new(CB_newf("Unsupported compound file format: %i32 "
                                    "(current = %i32", self->format,
                                    CFWriter_current_file_format));
        }
        else if (!self->records) {
            error = Err_new(CB_newf("Corrupt %o file: missing 'files' key",
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
    CharBuf *cf_file = (CharBuf*)ZCB_WRAP_STR("cf.dat", 6);
    self->instream = Folder_Open_In(folder, cf_file);
    if (!self->instream) {
        ERR_ADD_FRAME(Err_get_error());
        DECREF(self);
        return NULL;
    }

    // Assign.
    self->real_folder = (Folder*)INCREF(folder);

    // Strip directory name from filepaths for old format.
    if (self->format == 1) {
        VArray *files = Hash_Keys(self->records);
        ZombieCharBuf *filename = ZCB_BLANK();
        ZombieCharBuf *folder_name
            = IxFileNames_local_part(Folder_Get_Path(folder), ZCB_BLANK());
        size_t folder_name_len = ZCB_Length(folder_name);

        for (uint32_t i = 0, max = VA_Get_Size(files); i < max; i++) {
            CharBuf *orig = (CharBuf*)VA_Fetch(files, i);
            if (CB_Starts_With(orig, (CharBuf*)folder_name)) {
                Obj *record = Hash_Delete(self->records, (Obj*)orig);
                ZCB_Assign(filename, orig);
                ZCB_Nip(filename, folder_name_len + sizeof(DIR_SEP) - 1);
                Hash_Store(self->records, (Obj*)filename, (Obj*)record);
            }
        }

        DECREF(files);
    }

    return self;
}

void
CFReader_destroy(CompoundFileReader *self) {
    DECREF(self->real_folder);
    DECREF(self->instream);
    DECREF(self->records);
    SUPER_DESTROY(self, COMPOUNDFILEREADER);
}

Folder*
CFReader_get_real_folder(CompoundFileReader *self) {
    return self->real_folder;
}

void
CFReader_set_path(CompoundFileReader *self, const CharBuf *path) {
    Folder_Set_Path(self->real_folder, path);
    Folder_set_path((Folder*)self, path);
}

FileHandle*
CFReader_local_open_filehandle(CompoundFileReader *self,
                               const CharBuf *name, uint32_t flags) {
    Hash *entry = (Hash*)Hash_Fetch(self->records, (Obj*)name);
    FileHandle *fh = NULL;

    if (entry) {
        Err_set_error(Err_new(CB_newf("Can't open FileHandle for virtual file %o in '%o'",
                                      name, self->path)));
    }
    else {
        fh = Folder_Local_Open_FileHandle(self->real_folder, name, flags);
        if (!fh) {
            ERR_ADD_FRAME(Err_get_error());
        }
    }

    return fh;
}

bool_t
CFReader_local_delete(CompoundFileReader *self, const CharBuf *name) {
    Hash *record = (Hash*)Hash_Delete(self->records, (Obj*)name);
    DECREF(record);

    if (record == NULL) {
        return Folder_Local_Delete(self->real_folder, name);
    }
    else {
        // Once the number of virtual files falls to 0, remove the compound
        // files.
        if (Hash_Get_Size(self->records) == 0) {
            CharBuf *cf_file = (CharBuf*)ZCB_WRAP_STR("cf.dat", 6);
            if (!Folder_Delete(self->real_folder, cf_file)) {
                return false;
            }
            CharBuf *cfmeta_file = (CharBuf*)ZCB_WRAP_STR("cfmeta.json", 11);
            if (!Folder_Delete(self->real_folder, cfmeta_file)) {
                return false;

            }
        }
        return true;
    }
}

InStream*
CFReader_local_open_in(CompoundFileReader *self, const CharBuf *name) {
    Hash *entry = (Hash*)Hash_Fetch(self->records, (Obj*)name);

    if (!entry) {
        InStream *instream = Folder_Local_Open_In(self->real_folder, name);
        if (!instream) {
            ERR_ADD_FRAME(Err_get_error());
        }
        return instream;
    }
    else {
        Obj *len    = Hash_Fetch_Str(entry, "length", 6);
        Obj *offset = Hash_Fetch_Str(entry, "offset", 6);
        if (!len || !offset) {
            Err_set_error(Err_new(CB_newf("Malformed entry for '%o' in '%o'",
                                          name, Folder_Get_Path(self->real_folder))));
            return NULL;
        }
        else if (CB_Get_Size(self->path)) {
            CharBuf *fullpath = CB_newf("%o/%o", self->path, name);
            InStream *instream = InStream_Reopen(self->instream, fullpath,
                                                 Obj_To_I64(offset), Obj_To_I64(len));
            DECREF(fullpath);
            return instream;
        }
        else {
            return InStream_Reopen(self->instream, name, Obj_To_I64(offset),
                                   Obj_To_I64(len));
        }
    }
}

bool_t
CFReader_local_exists(CompoundFileReader *self, const CharBuf *name) {
    if (Hash_Fetch(self->records, (Obj*)name))        { return true; }
    if (Folder_Local_Exists(self->real_folder, name)) { return true; }
    return false;
}

bool_t
CFReader_local_is_directory(CompoundFileReader *self, const CharBuf *name) {
    if (Hash_Fetch(self->records, (Obj*)name))              { return false; }
    if (Folder_Local_Is_Directory(self->real_folder, name)) { return true; }
    return false;
}

void
CFReader_close(CompoundFileReader *self) {
    InStream_Close(self->instream);
}

bool_t
CFReader_local_mkdir(CompoundFileReader *self, const CharBuf *name) {
    if (Hash_Fetch(self->records, (Obj*)name)) {
        Err_set_error(Err_new(CB_newf("Can't MkDir: '%o' exists", name)));
        return false;
    }
    else {
        bool_t result = Folder_Local_MkDir(self->real_folder, name);
        if (!result) { ERR_ADD_FRAME(Err_get_error()); }
        return result;
    }
}

Folder*
CFReader_local_find_folder(CompoundFileReader *self, const CharBuf *name) {
    if (Hash_Fetch(self->records, (Obj*)name)) { return false; }
    return Folder_Local_Find_Folder(self->real_folder, name);
}

DirHandle*
CFReader_local_open_dir(CompoundFileReader *self) {
    return (DirHandle*)CFReaderDH_new(self);
}

/****************************************************************************/

CFReaderDirHandle*
CFReaderDH_new(CompoundFileReader *cf_reader) {
    CFReaderDirHandle *self
        = (CFReaderDirHandle*)VTable_Make_Obj(CFREADERDIRHANDLE);
    return CFReaderDH_init(self, cf_reader);
}

CFReaderDirHandle*
CFReaderDH_init(CFReaderDirHandle *self, CompoundFileReader *cf_reader) {
    DH_init((DirHandle*)self, CFReader_Get_Path(cf_reader));
    self->cf_reader = (CompoundFileReader*)INCREF(cf_reader);
    self->elems  = Hash_Keys(self->cf_reader->records);
    self->tick   = -1;
    // Accumulate entries from real Folder.
    DirHandle *dh = Folder_Local_Open_Dir(self->cf_reader->real_folder);
    CharBuf *entry = DH_Get_Entry(dh);
    while (DH_Next(dh)) {
        VA_Push(self->elems, (Obj*)CB_Clone(entry));
    }
    DECREF(dh);
    return self;
}

bool_t
CFReaderDH_close(CFReaderDirHandle *self) {
    if (self->elems) {
        VA_Dec_RefCount(self->elems);
        self->elems = NULL;
    }
    if (self->cf_reader) {
        CFReader_Dec_RefCount(self->cf_reader);
        self->cf_reader = NULL;
    }
    return true;
}

bool_t
CFReaderDH_next(CFReaderDirHandle *self) {
    if (self->elems) {
        self->tick++;
        if (self->tick < (int32_t)VA_Get_Size(self->elems)) {
            CharBuf *path = (CharBuf*)CERTIFY(
                                VA_Fetch(self->elems, self->tick), CHARBUF);
            CB_Mimic(self->entry, (Obj*)path);
            return true;
        }
        else {
            self->tick--;
            return false;
        }
    }
    return false;
}

bool_t
CFReaderDH_entry_is_dir(CFReaderDirHandle *self) {
    if (self->elems) {
        CharBuf *name = (CharBuf*)VA_Fetch(self->elems, self->tick);
        if (name) {
            return CFReader_Local_Is_Directory(self->cf_reader, name);
        }
    }
    return false;
}


