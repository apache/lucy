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

#define C_LUCY_INDEXER
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Index/Indexer.h"
#include "Lucy/Analysis/Analyzer.h"
#include "Lucy/Document/Doc.h"
#include "Lucy/Plan/FieldType.h"
#include "Lucy/Plan/FullTextType.h"
#include "Lucy/Plan/Schema.h"
#include "Lucy/Index/DeletionsReader.h"
#include "Lucy/Index/DeletionsWriter.h"
#include "Lucy/Index/FilePurger.h"
#include "Lucy/Index/IndexManager.h"
#include "Lucy/Index/PolyReader.h"
#include "Lucy/Index/Segment.h"
#include "Lucy/Index/SegReader.h"
#include "Lucy/Index/Snapshot.h"
#include "Lucy/Index/SegWriter.h"
#include "Lucy/Plan/Architecture.h"
#include "Lucy/Search/Matcher.h"
#include "Lucy/Search/Query.h"
#include "Lucy/Store/Folder.h"
#include "Lucy/Store/FSFolder.h"
#include "Lucy/Store/Lock.h"
#include "Lucy/Util/Freezer.h"
#include "Lucy/Util/IndexFileNames.h"
#include "Lucy/Util/Json.h"

int32_t Indexer_CREATE   = 0x00000001;
int32_t Indexer_TRUNCATE = 0x00000002;

// Release the write lock - if it's there.
static void
S_release_write_lock(Indexer *self);

// Release the merge lock - if it's there.
static void
S_release_merge_lock(Indexer *self);

// Verify a Folder or derive an FSFolder from a String path.  Call
// Folder_Initialize() if "create" is true.
static Folder*
S_init_folder(Obj *index, bool create);

// Find the schema file within a snapshot.
static String*
S_find_schema_file(Snapshot *snapshot);

Indexer*
Indexer_new(Schema *schema, Obj *index, IndexManager *manager, int32_t flags) {
    Indexer *self = (Indexer*)Class_Make_Obj(INDEXER);
    return Indexer_init(self, schema, index, manager, flags);
}

Indexer*
Indexer_init(Indexer *self, Schema *schema, Obj *index,
             IndexManager *manager, int32_t flags) {
    IndexerIVARS *const ivars = Indexer_IVARS(self);
    bool      create   = (flags & Indexer_CREATE)   ? true : false;
    bool      truncate = (flags & Indexer_TRUNCATE) ? true : false;
    Folder   *folder   = S_init_folder(index, create);
    Snapshot *latest_snapshot = Snapshot_new();

    // Init.
    ivars->stock_doc     = Doc_new(NULL, 0);
    ivars->truncate      = false;
    ivars->optimize      = false;
    ivars->prepared      = false;
    ivars->needs_commit  = false;
    ivars->snapfile      = NULL;
    ivars->merge_lock    = NULL;

    // Assign.
    ivars->folder       = folder;
    ivars->manager      = manager
                         ? (IndexManager*)INCREF(manager)
                         : IxManager_new(NULL, NULL);
    IxManager_Set_Folder(ivars->manager, folder);

    // Get a write lock for this folder.
    Lock *write_lock = IxManager_Make_Write_Lock(ivars->manager);
    Lock_Clear_Stale(write_lock);
    if (Lock_Obtain(write_lock)) {
        // Only assign if successful, otherwise DESTROY unlocks -- bad!
        ivars->write_lock = write_lock;
    }
    else {
        DECREF(write_lock);
        DECREF(self);
        RETHROW(INCREF(Err_get_error()));
    }

    // Find the latest snapshot or create a new one.
    String *latest_snapfile = IxFileNames_latest_snapshot(folder);
    if (latest_snapfile) {
        Snapshot_Read_File(latest_snapshot, folder, latest_snapfile);
    }

    // Look for an existing Schema if one wasn't supplied.
    if (schema) {
        ivars->schema = (Schema*)INCREF(schema);
    }
    else {
        if (!latest_snapfile) {
            S_release_write_lock(self);
            THROW(ERR, "No Schema supplied, and can't find one in the index");
        }
        else {
            String *schema_file = S_find_schema_file(latest_snapshot);
            Obj *dump = Json_slurp_json(folder, schema_file);
            if (dump) { // read file successfully
                ivars->schema = (Schema*)CERTIFY(Freezer_load(dump), SCHEMA);
                schema = ivars->schema;
                DECREF(dump);
                schema_file = NULL;
            }
            else {
                THROW(ERR, "Failed to parse %o", schema_file);
            }
        }
    }

    // If we're clobbering, start with an empty Snapshot and an empty
    // PolyReader.  Otherwise, start with the most recent Snapshot and an
    // up-to-date PolyReader.
    if (truncate) {
        ivars->snapshot = Snapshot_new();
        ivars->polyreader = PolyReader_new(schema, folder, NULL, NULL, NULL);
        ivars->truncate = true;
    }
    else {
        // TODO: clone most recent snapshot rather than read it twice.
        ivars->snapshot = (Snapshot*)INCREF(latest_snapshot);
        ivars->polyreader = latest_snapfile
                           ? PolyReader_open((Obj*)folder, NULL, NULL)
                           : PolyReader_new(schema, folder, NULL, NULL, NULL);

        if (latest_snapfile) {
            // Make sure than any existing fields which may have been
            // dynamically added during past indexing sessions get added.
            Schema *old_schema = PolyReader_Get_Schema(ivars->polyreader);
            Schema_Eat(schema, old_schema);
        }
    }

    // Zap detritus from previous sessions.
    // Note: we have to feed FilePurger with the most recent snapshot file
    // now, but with the Indexer's snapshot later.
    FilePurger *file_purger
        = FilePurger_new(folder, latest_snapshot, ivars->manager);
    FilePurger_Purge(file_purger);
    DECREF(file_purger);

    // Create a new segment.
    int64_t new_seg_num
        = IxManager_Highest_Seg_Num(ivars->manager, latest_snapshot) + 1;
    Lock *merge_lock = IxManager_Make_Merge_Lock(ivars->manager);
    if (Lock_Is_Locked(merge_lock)) {
        // If there's a background merge process going on, stay out of its
        // way.
        Hash *merge_data = IxManager_Read_Merge_Data(ivars->manager);
        Obj *cutoff_obj = merge_data
                          ? Hash_Fetch_Utf8(merge_data, "cutoff", 6)
                          : NULL;
        if (!cutoff_obj) {
            DECREF(merge_lock);
            DECREF(merge_data);
            THROW(ERR, "Background merge detected, but can't read merge data");
        }
        else {
            int64_t cutoff = Obj_To_I64(cutoff_obj);
            if (cutoff >= new_seg_num) {
                new_seg_num = cutoff + 1;
            }
        }
        DECREF(merge_data);
    }
    ivars->segment = Seg_new(new_seg_num);

    // Add all known fields to Segment.
    VArray *fields = Schema_All_Fields(schema);
    for (uint32_t i = 0, max = VA_Get_Size(fields); i < max; i++) {
        Seg_Add_Field(ivars->segment, (String*)VA_Fetch(fields, i));
    }
    DECREF(fields);

    DECREF(merge_lock);

    // Create new SegWriter and FilePurger.
    ivars->file_purger
        = FilePurger_new(folder, ivars->snapshot, ivars->manager);
    ivars->seg_writer = SegWriter_new(ivars->schema, ivars->snapshot,
                                     ivars->segment, ivars->polyreader);
    SegWriter_Prep_Seg_Dir(ivars->seg_writer);

    // Grab a local ref to the DeletionsWriter.
    ivars->del_writer = (DeletionsWriter*)INCREF(
                           SegWriter_Get_Del_Writer(ivars->seg_writer));

    DECREF(latest_snapfile);
    DECREF(latest_snapshot);

    return self;
}

void
Indexer_Destroy_IMP(Indexer *self) {
    IndexerIVARS *const ivars = Indexer_IVARS(self);
    S_release_merge_lock(self);
    S_release_write_lock(self);
    DECREF(ivars->schema);
    DECREF(ivars->folder);
    DECREF(ivars->segment);
    DECREF(ivars->manager);
    DECREF(ivars->stock_doc);
    DECREF(ivars->polyreader);
    DECREF(ivars->del_writer);
    DECREF(ivars->snapshot);
    DECREF(ivars->seg_writer);
    DECREF(ivars->file_purger);
    DECREF(ivars->write_lock);
    DECREF(ivars->snapfile);
    SUPER_DESTROY(self, INDEXER);
}

static Folder*
S_init_folder(Obj *index, bool create) {
    Folder *folder = NULL;

    // Validate or acquire a Folder.
    if (Obj_Is_A(index, FOLDER)) {
        folder = (Folder*)INCREF(index);
    }
    else if (Obj_Is_A(index, STRING)) {
        folder = (Folder*)FSFolder_new((String*)index);
    }
    else {
        THROW(ERR, "Invalid type for 'index': %o", Obj_Get_Class_Name(index));
    }

    // Validate or create the index directory.
    if (create) {
        Folder_Initialize(folder);
    }
    else {
        if (!Folder_Check(folder)) {
            THROW(ERR, "Folder '%o' failed check", Folder_Get_Path(folder));
        }
    }

    return folder;
}

void
Indexer_Add_Doc_IMP(Indexer *self, Doc *doc, float boost) {
    IndexerIVARS *const ivars = Indexer_IVARS(self);
    SegWriter_Add_Doc(ivars->seg_writer, doc, boost);
}

void
Indexer_Delete_By_Term_IMP(Indexer *self, String *field, Obj *term) {
    IndexerIVARS *const ivars = Indexer_IVARS(self);
    Schema    *schema = ivars->schema;
    FieldType *type   = Schema_Fetch_Type(schema, field);

    // Raise exception if the field isn't indexed.
    if (!type || !FType_Indexed(type)) {
        THROW(ERR, "%o is not an indexed field", field);
    }

    // Analyze term if appropriate, then zap.
    if (FType_Is_A(type, FULLTEXTTYPE)) {
        CERTIFY(term, STRING);
        Analyzer *analyzer = Schema_Fetch_Analyzer(schema, field);
        VArray *terms = Analyzer_Split(analyzer, (String*)term);
        Obj *analyzed_term = VA_Fetch(terms, 0);
        if (analyzed_term) {
            DelWriter_Delete_By_Term(ivars->del_writer, field,
                                     analyzed_term);
        }
        DECREF(terms);
    }
    else {
        DelWriter_Delete_By_Term(ivars->del_writer, field, term);
    }
}

void
Indexer_Delete_By_Query_IMP(Indexer *self, Query *query) {
    IndexerIVARS *const ivars = Indexer_IVARS(self);
    DelWriter_Delete_By_Query(ivars->del_writer, query);
}

void
Indexer_Delete_By_Doc_ID_IMP(Indexer *self, int32_t doc_id) {
    IndexerIVARS *const ivars = Indexer_IVARS(self);
    DelWriter_Delete_By_Doc_ID(ivars->del_writer, doc_id);
}

void
Indexer_Add_Index_IMP(Indexer *self, Obj *index) {
    IndexerIVARS *const ivars = Indexer_IVARS(self);
    Folder *other_folder = NULL;
    IndexReader *reader  = NULL;

    if (Obj_Is_A(index, FOLDER)) {
        other_folder = (Folder*)INCREF(index);
    }
    else if (Obj_Is_A(index, STRING)) {
        other_folder = (Folder*)FSFolder_new((String*)index);
    }
    else {
        THROW(ERR, "Invalid type for 'index': %o", Obj_Get_Class_Name(index));
    }

    reader = IxReader_open((Obj*)other_folder, NULL, NULL);
    if (reader == NULL) {
        THROW(ERR, "Index doesn't seem to contain any data");
    }
    else {
        Schema *schema       = ivars->schema;
        Schema *other_schema = IxReader_Get_Schema(reader);
        VArray *other_fields = Schema_All_Fields(other_schema);
        VArray *seg_readers  = IxReader_Seg_Readers(reader);

        // Validate schema compatibility and add fields.
        Schema_Eat(schema, other_schema);

        // Add fields to Segment.
        for (uint32_t i = 0, max = VA_Get_Size(other_fields); i < max; i++) {
            String *other_field = (String*)VA_Fetch(other_fields, i);
            Seg_Add_Field(ivars->segment, other_field);
        }
        DECREF(other_fields);

        // Add all segments.
        for (uint32_t i = 0, max = VA_Get_Size(seg_readers); i < max; i++) {
            SegReader *seg_reader = (SegReader*)VA_Fetch(seg_readers, i);
            DeletionsReader *del_reader
                = (DeletionsReader*)SegReader_Fetch(
                      seg_reader, Class_Get_Name(DELETIONSREADER));
            Matcher *deletions = del_reader
                                 ? DelReader_Iterator(del_reader)
                                 : NULL;
            I32Array *doc_map = DelWriter_Generate_Doc_Map(
                                    ivars->del_writer, deletions,
                                    SegReader_Doc_Max(seg_reader),
                                    (int32_t)Seg_Get_Count(ivars->segment));
            SegWriter_Add_Segment(ivars->seg_writer, seg_reader, doc_map);
            DECREF(deletions);
            DECREF(doc_map);
        }
        DECREF(seg_readers);
    }

    DECREF(reader);
    DECREF(other_folder);
}

void
Indexer_Optimize_IMP(Indexer *self) {
    Indexer_IVARS(self)->optimize = true;
}

static String*
S_find_schema_file(Snapshot *snapshot) {
    VArray *files = Snapshot_List(snapshot);
    String *retval = NULL;
    for (uint32_t i = 0, max = VA_Get_Size(files); i < max; i++) {
        String *file = (String*)VA_Fetch(files, i);
        if (Str_Starts_With_Utf8(file, "schema_", 7)
            && Str_Ends_With_Utf8(file, ".json", 5)
           ) {
            retval = file;
            break;
        }
    }
    DECREF(files);
    return retval;
}

static bool
S_maybe_merge(Indexer *self, VArray *seg_readers) {
    IndexerIVARS *const ivars = Indexer_IVARS(self);
    bool      merge_happened  = false;
    uint32_t  num_seg_readers = VA_Get_Size(seg_readers);
    Lock     *merge_lock      = IxManager_Make_Merge_Lock(ivars->manager);
    bool      got_merge_lock  = Lock_Obtain(merge_lock);
    int64_t   cutoff;

    if (got_merge_lock) {
        ivars->merge_lock = merge_lock;
        cutoff = 0;
    }
    else {
        // If something else holds the merge lock, don't interfere.
        Hash *merge_data = IxManager_Read_Merge_Data(ivars->manager);
        if (merge_data) {
            Obj *cutoff_obj = Hash_Fetch_Utf8(merge_data, "cutoff", 6);
            if (cutoff_obj) {
                cutoff = Obj_To_I64(cutoff_obj);
            }
            else {
                cutoff = INT64_MAX;
            }
            DECREF(merge_data);
        }
        else {
            cutoff = INT64_MAX;
        }
        DECREF(merge_lock);
    }

    // Get a list of segments to recycle.  Validate and confirm that there are
    // no dupes in the list.
    VArray *to_merge = IxManager_Recycle(ivars->manager, ivars->polyreader,
                                         ivars->del_writer, cutoff, ivars->optimize);

    Hash *seen = Hash_new(VA_Get_Size(to_merge));
    for (uint32_t i = 0, max = VA_Get_Size(to_merge); i < max; i++) {
        SegReader *seg_reader
            = (SegReader*)CERTIFY(VA_Fetch(to_merge, i), SEGREADER);
        String *seg_name = SegReader_Get_Seg_Name(seg_reader);
        if (Hash_Fetch(seen, (Obj*)seg_name)) {
            DECREF(seen);
            DECREF(to_merge);
            THROW(ERR, "Recycle() tried to merge segment '%o' twice",
                  seg_name);
        }
        Hash_Store(seen, (Obj*)seg_name, (Obj*)CFISH_TRUE);
    }
    DECREF(seen);

    // Consolidate segments if either sparse or optimizing forced.
    for (uint32_t i = 0, max = VA_Get_Size(to_merge); i < max; i++) {
        SegReader *seg_reader = (SegReader*)VA_Fetch(to_merge, i);
        int64_t seg_num = SegReader_Get_Seg_Num(seg_reader);
        Matcher *deletions
            = DelWriter_Seg_Deletions(ivars->del_writer, seg_reader);
        I32Array *doc_map = DelWriter_Generate_Doc_Map(
                                ivars->del_writer, deletions,
                                SegReader_Doc_Max(seg_reader),
                                (int32_t)Seg_Get_Count(ivars->segment));
        if (seg_num <= cutoff) {
            THROW(ERR, "Segment %o violates cutoff (%i64 <= %i64)",
                  SegReader_Get_Seg_Name(seg_reader), seg_num, cutoff);
        }
        SegWriter_Merge_Segment(ivars->seg_writer, seg_reader, doc_map);
        merge_happened = true;
        DECREF(deletions);
        DECREF(doc_map);
    }

    // Write out new deletions.
    if (DelWriter_Updated(ivars->del_writer)) {
        // Only write out if they haven't all been applied.
        if (VA_Get_Size(to_merge) != num_seg_readers) {
            DelWriter_Finish(ivars->del_writer);
        }
    }

    DECREF(to_merge);
    return merge_happened;
}

void
Indexer_Prepare_Commit_IMP(Indexer *self) {
    IndexerIVARS *const ivars = Indexer_IVARS(self);
    VArray   *seg_readers     = PolyReader_Get_Seg_Readers(ivars->polyreader);
    uint32_t  num_seg_readers = VA_Get_Size(seg_readers);
    bool      merge_happened  = false;

    if (!ivars->write_lock || ivars->prepared) {
        THROW(ERR, "Can't call Prepare_Commit() more than once");
    }

    // Merge existing index data.
    if (num_seg_readers) {
        merge_happened = S_maybe_merge(self, seg_readers);
    }

    // Add a new segment and write a new snapshot file if...
    if (Seg_Get_Count(ivars->segment)             // Docs/segs added.
        || merge_happened                        // Some segs merged.
        || !Snapshot_Num_Entries(ivars->snapshot) // Initializing index.
        || DelWriter_Updated(ivars->del_writer)
       ) {
        Folder   *folder   = ivars->folder;
        Schema   *schema   = ivars->schema;
        Snapshot *snapshot = ivars->snapshot;

        // Derive snapshot and schema file names.
        DECREF(ivars->snapfile);
        String *snapfile = IxManager_Make_Snapshot_Filename(ivars->manager);
        ivars->snapfile = Str_Cat_Trusted_Utf8(snapfile, ".temp", 5);
        DECREF(snapfile);
        uint64_t schema_gen = IxFileNames_extract_gen(ivars->snapfile);
        char base36[StrHelp_MAX_BASE36_BYTES];
        StrHelp_to_base36(schema_gen, &base36);
        String *new_schema_name = Str_newf("schema_%s.json", base36);

        // Finish the segment, write schema file.
        SegWriter_Finish(ivars->seg_writer);
        Schema_Write(schema, folder, new_schema_name);
        String *old_schema_name = S_find_schema_file(snapshot);
        if (old_schema_name) {
            Snapshot_Delete_Entry(snapshot, old_schema_name);
        }
        Snapshot_Add_Entry(snapshot, new_schema_name);
        DECREF(new_schema_name);

        // Write temporary snapshot file.
        Folder_Delete(folder, ivars->snapfile);
        Snapshot_Write_File(snapshot, folder, ivars->snapfile);

        ivars->needs_commit = true;
    }

    // Close reader, so that we can delete its files if appropriate.
    PolyReader_Close(ivars->polyreader);

    ivars->prepared = true;
}

void
Indexer_Commit_IMP(Indexer *self) {
    IndexerIVARS *const ivars = Indexer_IVARS(self);

    // Safety check.
    if (!ivars->write_lock) {
        THROW(ERR, "Can't call commit() more than once");
    }

    if (!ivars->prepared) {
        Indexer_Prepare_Commit(self);
    }

    if (ivars->needs_commit) {
        bool success;

        // Rename temp snapshot file.
        String *temp_snapfile = ivars->snapfile;
        size_t ext_len      = sizeof(".temp") - 1;
        size_t snapfile_len = Str_Length(temp_snapfile);
        if (snapfile_len <= ext_len) {
            THROW(ERR, "Invalid snapfile name: %o", temp_snapfile);
        }
        ivars->snapfile = Str_SubString(temp_snapfile, 0,
                                       snapfile_len - ext_len);
        Snapshot_Set_Path(ivars->snapshot, ivars->snapfile);
        success = Folder_Rename(ivars->folder, temp_snapfile, ivars->snapfile);
        DECREF(temp_snapfile);
        if (!success) { RETHROW(INCREF(Err_get_error())); }

        // Purge obsolete files.
        FilePurger_Purge(ivars->file_purger);
    }

    // Release locks, invalidating the Indexer.
    S_release_merge_lock(self);
    S_release_write_lock(self);
}

Schema*
Indexer_Get_Schema_IMP(Indexer *self) {
    return Indexer_IVARS(self)->schema;
}

SegWriter*
Indexer_Get_Seg_Writer_IMP(Indexer *self) {
    return Indexer_IVARS(self)->seg_writer;
}

Doc*
Indexer_Get_Stock_Doc_IMP(Indexer *self) {
    return Indexer_IVARS(self)->stock_doc;
}

static void
S_release_write_lock(Indexer *self) {
    IndexerIVARS *const ivars = Indexer_IVARS(self);
    if (ivars->write_lock) {
        Lock_Release(ivars->write_lock);
        DECREF(ivars->write_lock);
        ivars->write_lock = NULL;
    }
}

static void
S_release_merge_lock(Indexer *self) {
    IndexerIVARS *const ivars = Indexer_IVARS(self);
    if (ivars->merge_lock) {
        Lock_Release(ivars->merge_lock);
        DECREF(ivars->merge_lock);
        ivars->merge_lock = NULL;
    }
}


