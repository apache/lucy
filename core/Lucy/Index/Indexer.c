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

// Verify a Folder or derive an FSFolder from a CharBuf path.  Call
// Folder_Initialize() if "create" is true.
static Folder*
S_init_folder(Obj *index, bool_t create);

// Find the schema file within a snapshot.
static CharBuf*
S_find_schema_file(Snapshot *snapshot);

Indexer*
Indexer_new(Schema *schema, Obj *index, IndexManager *manager, int32_t flags) {
    Indexer *self = (Indexer*)VTable_Make_Obj(INDEXER);
    return Indexer_init(self, schema, index, manager, flags);
}

Indexer*
Indexer_init(Indexer *self, Schema *schema, Obj *index,
             IndexManager *manager, int32_t flags) {
    bool_t    create   = (flags & Indexer_CREATE)   ? true : false;
    bool_t    truncate = (flags & Indexer_TRUNCATE) ? true : false;
    Folder   *folder   = S_init_folder(index, create);
    Snapshot *latest_snapshot = Snapshot_new();

    // Init.
    self->stock_doc     = Doc_new(NULL, 0);
    self->truncate      = false;
    self->optimize      = false;
    self->prepared      = false;
    self->needs_commit  = false;
    self->snapfile      = NULL;
    self->merge_lock    = NULL;

    // Assign.
    self->folder       = folder;
    self->manager      = manager
                         ? (IndexManager*)INCREF(manager)
                         : IxManager_new(NULL, NULL);
    IxManager_Set_Folder(self->manager, folder);

    // Get a write lock for this folder.
    Lock *write_lock = IxManager_Make_Write_Lock(self->manager);
    Lock_Clear_Stale(write_lock);
    if (Lock_Obtain(write_lock)) {
        // Only assign if successful, otherwise DESTROY unlocks -- bad!
        self->write_lock = write_lock;
    }
    else {
        DECREF(write_lock);
        DECREF(self);
        RETHROW(INCREF(Err_get_error()));
    }

    // Find the latest snapshot or create a new one.
    CharBuf *latest_snapfile = IxFileNames_latest_snapshot(folder);
    if (latest_snapfile) {
        Snapshot_Read_File(latest_snapshot, folder, latest_snapfile);
    }

    // Look for an existing Schema if one wasn't supplied.
    if (schema) {
        self->schema = (Schema*)INCREF(schema);
    }
    else {
        if (!latest_snapfile) {
            THROW(ERR, "No Schema supplied, and can't find one in the index");
        }
        else {
            CharBuf *schema_file = S_find_schema_file(latest_snapshot);
            Hash *dump = (Hash*)Json_slurp_json(folder, schema_file);
            if (dump) { // read file successfully
                self->schema = (Schema*)CERTIFY(
                                   VTable_Load_Obj(SCHEMA, (Obj*)dump),
                                   SCHEMA);
                schema = self->schema;
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
        self->snapshot = Snapshot_new();
        self->polyreader = PolyReader_new(schema, folder, NULL, NULL, NULL);
        self->truncate = true;
    }
    else {
        // TODO: clone most recent snapshot rather than read it twice.
        self->snapshot = (Snapshot*)INCREF(latest_snapshot);
        self->polyreader = latest_snapfile
                           ? PolyReader_open((Obj*)folder, NULL, NULL)
                           : PolyReader_new(schema, folder, NULL, NULL, NULL);

        if (latest_snapfile) {
            // Make sure than any existing fields which may have been
            // dynamically added during past indexing sessions get added.
            Schema *old_schema = PolyReader_Get_Schema(self->polyreader);
            Schema_Eat(schema, old_schema);
        }
    }

    // Zap detritus from previous sessions.
    // Note: we have to feed FilePurger with the most recent snapshot file
    // now, but with the Indexer's snapshot later.
    FilePurger *file_purger
        = FilePurger_new(folder, latest_snapshot, self->manager);
    FilePurger_Purge(file_purger);
    DECREF(file_purger);

    // Create a new segment.
    int64_t new_seg_num
        = IxManager_Highest_Seg_Num(self->manager, latest_snapshot) + 1;
    Lock *merge_lock = IxManager_Make_Merge_Lock(self->manager);
    if (Lock_Is_Locked(merge_lock)) {
        // If there's a background merge process going on, stay out of its
        // way.
        Hash *merge_data = IxManager_Read_Merge_Data(self->manager);
        Obj *cutoff_obj = merge_data
                          ? Hash_Fetch_Str(merge_data, "cutoff", 6)
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
    self->segment = Seg_new(new_seg_num);

    // Add all known fields to Segment.
    VArray *fields = Schema_All_Fields(schema);
    for (uint32_t i = 0, max = VA_Get_Size(fields); i < max; i++) {
        Seg_Add_Field(self->segment, (CharBuf*)VA_Fetch(fields, i));
    }
    DECREF(fields);

    DECREF(merge_lock);

    // Create new SegWriter and FilePurger.
    self->file_purger
        = FilePurger_new(folder, self->snapshot, self->manager);
    self->seg_writer = SegWriter_new(self->schema, self->snapshot,
                                     self->segment, self->polyreader);
    SegWriter_Prep_Seg_Dir(self->seg_writer);

    // Grab a local ref to the DeletionsWriter.
    self->del_writer = (DeletionsWriter*)INCREF(
                           SegWriter_Get_Del_Writer(self->seg_writer));

    DECREF(latest_snapfile);
    DECREF(latest_snapshot);

    return self;
}

void
Indexer_destroy(Indexer *self) {
    S_release_merge_lock(self);
    S_release_write_lock(self);
    DECREF(self->schema);
    DECREF(self->folder);
    DECREF(self->segment);
    DECREF(self->manager);
    DECREF(self->stock_doc);
    DECREF(self->polyreader);
    DECREF(self->del_writer);
    DECREF(self->snapshot);
    DECREF(self->seg_writer);
    DECREF(self->file_purger);
    DECREF(self->write_lock);
    DECREF(self->snapfile);
    SUPER_DESTROY(self, INDEXER);
}

static Folder*
S_init_folder(Obj *index, bool_t create) {
    Folder *folder = NULL;

    // Validate or acquire a Folder.
    if (Obj_Is_A(index, FOLDER)) {
        folder = (Folder*)INCREF(index);
    }
    else if (Obj_Is_A(index, CHARBUF)) {
        folder = (Folder*)FSFolder_new((CharBuf*)index);
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
Indexer_add_doc(Indexer *self, Doc *doc, float boost) {
    SegWriter_Add_Doc(self->seg_writer, doc, boost);
}

void
Indexer_delete_by_term(Indexer *self, CharBuf *field, Obj *term) {
    Schema    *schema = self->schema;
    FieldType *type   = Schema_Fetch_Type(schema, field);

    // Raise exception if the field isn't indexed.
    if (!type || !FType_Indexed(type)) {
        THROW(ERR, "%o is not an indexed field", field);
    }

    // Analyze term if appropriate, then zap.
    if (FType_Is_A(type, FULLTEXTTYPE)) {
        CERTIFY(term, CHARBUF);
        Analyzer *analyzer = Schema_Fetch_Analyzer(schema, field);
        VArray *terms = Analyzer_Split(analyzer, (CharBuf*)term);
        Obj *analyzed_term = VA_Fetch(terms, 0);
        if (analyzed_term) {
            DelWriter_Delete_By_Term(self->del_writer, field,
                                     analyzed_term);
        }
        DECREF(terms);
    }
    else {
        DelWriter_Delete_By_Term(self->del_writer, field, term);
    }
}

void
Indexer_delete_by_query(Indexer *self, Query *query) {
    DelWriter_Delete_By_Query(self->del_writer, query);
}

void
Indexer_add_index(Indexer *self, Obj *index) {
    Folder *other_folder = NULL;
    IndexReader *reader  = NULL;

    if (Obj_Is_A(index, FOLDER)) {
        other_folder = (Folder*)INCREF(index);
    }
    else if (Obj_Is_A(index, CHARBUF)) {
        other_folder = (Folder*)FSFolder_new((CharBuf*)index);
    }
    else {
        THROW(ERR, "Invalid type for 'index': %o", Obj_Get_Class_Name(index));
    }

    reader = IxReader_open((Obj*)other_folder, NULL, NULL);
    if (reader == NULL) {
        THROW(ERR, "Index doesn't seem to contain any data");
    }
    else {
        Schema *schema       = self->schema;
        Schema *other_schema = IxReader_Get_Schema(reader);
        VArray *other_fields = Schema_All_Fields(other_schema);
        VArray *seg_readers  = IxReader_Seg_Readers(reader);

        // Validate schema compatibility and add fields.
        Schema_Eat(schema, other_schema);

        // Add fields to Segment.
        for (uint32_t i = 0, max = VA_Get_Size(other_fields); i < max; i++) {
            CharBuf *other_field = (CharBuf*)VA_Fetch(other_fields, i);
            Seg_Add_Field(self->segment, other_field);
        }
        DECREF(other_fields);

        // Add all segments.
        for (uint32_t i = 0, max = VA_Get_Size(seg_readers); i < max; i++) {
            SegReader *seg_reader = (SegReader*)VA_Fetch(seg_readers, i);
            DeletionsReader *del_reader
                = (DeletionsReader*)SegReader_Fetch(
                      seg_reader, VTable_Get_Name(DELETIONSREADER));
            Matcher *deletions = del_reader
                                 ? DelReader_Iterator(del_reader)
                                 : NULL;
            I32Array *doc_map = DelWriter_Generate_Doc_Map(
                                    self->del_writer, deletions,
                                    SegReader_Doc_Max(seg_reader),
                                    (int32_t)Seg_Get_Count(self->segment));
            SegWriter_Add_Segment(self->seg_writer, seg_reader, doc_map);
            DECREF(deletions);
            DECREF(doc_map);
        }
        DECREF(seg_readers);
    }

    DECREF(reader);
    DECREF(other_folder);
}

void
Indexer_optimize(Indexer *self) {
    self->optimize = true;
}

static CharBuf*
S_find_schema_file(Snapshot *snapshot) {
    VArray *files = Snapshot_List(snapshot);
    CharBuf *retval = NULL;
    for (uint32_t i = 0, max = VA_Get_Size(files); i < max; i++) {
        CharBuf *file = (CharBuf*)VA_Fetch(files, i);
        if (CB_Starts_With_Str(file, "schema_", 7)
            && CB_Ends_With_Str(file, ".json", 5)
           ) {
            retval = file;
            break;
        }
    }
    DECREF(files);
    return retval;
}

static bool_t
S_maybe_merge(Indexer *self, VArray *seg_readers) {
    bool_t    merge_happened  = false;
    uint32_t  num_seg_readers = VA_Get_Size(seg_readers);
    Lock     *merge_lock      = IxManager_Make_Merge_Lock(self->manager);
    bool_t    got_merge_lock  = Lock_Obtain(merge_lock);
    int64_t   cutoff;

    if (got_merge_lock) {
        self->merge_lock = merge_lock;
        cutoff = 0;
    }
    else {
        // If something else holds the merge lock, don't interfere.
        Hash *merge_data = IxManager_Read_Merge_Data(self->manager);
        if (merge_data) {
            Obj *cutoff_obj = Hash_Fetch_Str(merge_data, "cutoff", 6);
            if (cutoff_obj) {
                cutoff = Obj_To_I64(cutoff_obj);
            }
            else {
                cutoff = I64_MAX;
            }
            DECREF(merge_data);
        }
        else {
            cutoff = I64_MAX;
        }
        DECREF(merge_lock);
    }

    // Get a list of segments to recycle.  Validate and confirm that there are
    // no dupes in the list.
    VArray *to_merge = IxManager_Recycle(self->manager, self->polyreader,
                                         self->del_writer, cutoff, self->optimize);

    Hash *seen = Hash_new(VA_Get_Size(to_merge));
    for (uint32_t i = 0, max = VA_Get_Size(to_merge); i < max; i++) {
        SegReader *seg_reader
            = (SegReader*)CERTIFY(VA_Fetch(to_merge, i), SEGREADER);
        CharBuf *seg_name = SegReader_Get_Seg_Name(seg_reader);
        if (Hash_Fetch(seen, (Obj*)seg_name)) {
            THROW(ERR, "Recycle() tried to merge segment '%o' twice",
                  seg_name);
        }
        Hash_Store(seen, (Obj*)seg_name, INCREF(&EMPTY));
    }
    DECREF(seen);

    // Consolidate segments if either sparse or optimizing forced.
    for (uint32_t i = 0, max = VA_Get_Size(to_merge); i < max; i++) {
        SegReader *seg_reader = (SegReader*)VA_Fetch(to_merge, i);
        int64_t seg_num = SegReader_Get_Seg_Num(seg_reader);
        Matcher *deletions
            = DelWriter_Seg_Deletions(self->del_writer, seg_reader);
        I32Array *doc_map = DelWriter_Generate_Doc_Map(
                                self->del_writer, deletions,
                                SegReader_Doc_Max(seg_reader),
                                (int32_t)Seg_Get_Count(self->segment));
        if (seg_num <= cutoff) {
            THROW(ERR, "Segment %o violates cutoff (%i64 <= %i64)",
                  SegReader_Get_Seg_Name(seg_reader), seg_num, cutoff);
        }
        SegWriter_Merge_Segment(self->seg_writer, seg_reader, doc_map);
        merge_happened = true;
        DECREF(deletions);
        DECREF(doc_map);
    }

    // Write out new deletions.
    if (DelWriter_Updated(self->del_writer)) {
        // Only write out if they haven't all been applied.
        if (VA_Get_Size(to_merge) != num_seg_readers) {
            DelWriter_Finish(self->del_writer);
        }
    }

    DECREF(to_merge);
    return merge_happened;
}

void
Indexer_prepare_commit(Indexer *self) {
    VArray   *seg_readers     = PolyReader_Get_Seg_Readers(self->polyreader);
    uint32_t  num_seg_readers = VA_Get_Size(seg_readers);
    bool_t    merge_happened  = false;

    if (!self->write_lock || self->prepared) {
        THROW(ERR, "Can't call Prepare_Commit() more than once");
    }

    // Merge existing index data.
    if (num_seg_readers) {
        merge_happened = S_maybe_merge(self, seg_readers);
    }

    // Add a new segment and write a new snapshot file if...
    if (Seg_Get_Count(self->segment)             // Docs/segs added.
        || merge_happened                        // Some segs merged.
        || !Snapshot_Num_Entries(self->snapshot) // Initializing index.
        || DelWriter_Updated(self->del_writer)
       ) {
        Folder   *folder   = self->folder;
        Schema   *schema   = self->schema;
        Snapshot *snapshot = self->snapshot;

        // Derive snapshot and schema file names.
        DECREF(self->snapfile);
        self->snapfile = IxManager_Make_Snapshot_Filename(self->manager);
        CB_Cat_Trusted_Str(self->snapfile, ".temp", 5);
        uint64_t schema_gen = IxFileNames_extract_gen(self->snapfile);
        char base36[StrHelp_MAX_BASE36_BYTES];
        StrHelp_to_base36(schema_gen, &base36);
        CharBuf *new_schema_name = CB_newf("schema_%s.json", base36);

        // Finish the segment, write schema file.
        SegWriter_Finish(self->seg_writer);
        Schema_Write(schema, folder, new_schema_name);
        CharBuf *old_schema_name = S_find_schema_file(snapshot);
        if (old_schema_name) {
            Snapshot_Delete_Entry(snapshot, old_schema_name);
        }
        Snapshot_Add_Entry(snapshot, new_schema_name);
        DECREF(new_schema_name);

        // Write temporary snapshot file.
        Folder_Delete(folder, self->snapfile);
        Snapshot_Write_File(snapshot, folder, self->snapfile);

        self->needs_commit = true;
    }

    // Close reader, so that we can delete its files if appropriate.
    PolyReader_Close(self->polyreader);

    self->prepared = true;
}

void
Indexer_commit(Indexer *self) {
    // Safety check.
    if (!self->write_lock) {
        THROW(ERR, "Can't call commit() more than once");
    }

    if (!self->prepared) {
        Indexer_Prepare_Commit(self);
    }

    if (self->needs_commit) {
        bool_t success;

        // Rename temp snapshot file.
        CharBuf *temp_snapfile = CB_Clone(self->snapfile);
        CB_Chop(self->snapfile, sizeof(".temp") - 1);
        Snapshot_Set_Path(self->snapshot, self->snapfile);
        success = Folder_Rename(self->folder, temp_snapfile, self->snapfile);
        DECREF(temp_snapfile);
        if (!success) { RETHROW(INCREF(Err_get_error())); }

        // Purge obsolete files.
        FilePurger_Purge(self->file_purger);
    }

    // Release locks, invalidating the Indexer.
    S_release_merge_lock(self);
    S_release_write_lock(self);
}

SegWriter*
Indexer_get_seg_writer(Indexer *self) {
    return self->seg_writer;
}

Doc*
Indexer_get_stock_doc(Indexer *self) {
    return self->stock_doc;
}

static void
S_release_write_lock(Indexer *self) {
    if (self->write_lock) {
        Lock_Release(self->write_lock);
        DECREF(self->write_lock);
        self->write_lock = NULL;
    }
}

static void
S_release_merge_lock(Indexer *self) {
    if (self->merge_lock) {
        Lock_Release(self->merge_lock);
        DECREF(self->merge_lock);
        self->merge_lock = NULL;
    }
}


