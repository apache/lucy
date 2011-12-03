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

#define C_LUCY_SCHEMA
#include <string.h>
#include <ctype.h>
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Plan/Schema.h"
#include "Lucy/Analysis/Analyzer.h"
#include "Lucy/Index/Similarity.h"
#include "Lucy/Plan/FieldType.h"
#include "Lucy/Plan/BlobType.h"
#include "Lucy/Plan/NumericType.h"
#include "Lucy/Plan/StringType.h"
#include "Lucy/Plan/FullTextType.h"
#include "Lucy/Plan/Architecture.h"
#include "Lucy/Store/Folder.h"
#include "Lucy/Util/Json.h"

// Scan the array to see if an object testing as Equal is present.  If not,
// push the elem onto the end of the array.
static void
S_add_unique(VArray *array, Obj *elem);

static void
S_add_text_field(Schema *self, const CharBuf *field, FieldType *type);
static void
S_add_string_field(Schema *self, const CharBuf *field, FieldType *type);
static void
S_add_blob_field(Schema *self, const CharBuf *field, FieldType *type);
static void
S_add_numeric_field(Schema *self, const CharBuf *field, FieldType *type);

Schema*
Schema_new() {
    Schema *self = (Schema*)VTable_Make_Obj(SCHEMA);
    return Schema_init(self);
}

Schema*
Schema_init(Schema *self) {
    // Init.
    self->analyzers      = Hash_new(0);
    self->types          = Hash_new(0);
    self->sims           = Hash_new(0);
    self->uniq_analyzers = VA_new(2);
    VA_Resize(self->uniq_analyzers, 1);

    // Assign.
    self->arch = Schema_Architecture(self);
    self->sim  = Arch_Make_Similarity(self->arch);

    return self;
}

void
Schema_destroy(Schema *self) {
    DECREF(self->arch);
    DECREF(self->analyzers);
    DECREF(self->uniq_analyzers);
    DECREF(self->types);
    DECREF(self->sims);
    DECREF(self->sim);
    SUPER_DESTROY(self, SCHEMA);
}

static void
S_add_unique(VArray *array, Obj *elem) {
    if (!elem) { return; }
    for (uint32_t i = 0, max = VA_Get_Size(array); i < max; i++) {
        Obj *candidate = VA_Fetch(array, i);
        if (!candidate) { continue; }
        if (elem == candidate) { return; }
        if (Obj_Get_VTable(elem) == Obj_Get_VTable(candidate)) {
            if (Obj_Equals(elem, candidate)) { return; }
        }
    }
    VA_Push(array, INCREF(elem));
}

bool_t
Schema_equals(Schema *self, Obj *other) {
    Schema *twin = (Schema*)other;
    if (twin == self)                                 { return true; }
    if (!Obj_Is_A(other, SCHEMA))                     { return false; }
    if (!Arch_Equals(self->arch, (Obj*)twin->arch))   { return false; }
    if (!Sim_Equals(self->sim, (Obj*)twin->sim))      { return false; }
    if (!Hash_Equals(self->types, (Obj*)twin->types)) { return false; }
    return true;
}

Architecture*
Schema_architecture(Schema *self) {
    UNUSED_VAR(self);
    return Arch_new();
}

void
Schema_spec_field(Schema *self, const CharBuf *field, FieldType *type) {
    FieldType *existing  = Schema_Fetch_Type(self, field);

    // If the field already has an association, verify pairing and return.
    if (existing) {
        if (FType_Equals(type, (Obj*)existing)) { return; }
        else { THROW(ERR, "'%o' assigned conflicting FieldType", field); }
    }

    if (FType_Is_A(type, FULLTEXTTYPE)) {
        S_add_text_field(self, field, type);
    }
    else if (FType_Is_A(type, STRINGTYPE)) {
        S_add_string_field(self, field, type);
    }
    else if (FType_Is_A(type, BLOBTYPE)) {
        S_add_blob_field(self, field, type);
    }
    else if (FType_Is_A(type, NUMERICTYPE)) {
        S_add_numeric_field(self, field, type);
    }
    else {
        THROW(ERR, "Unrecognized field type: '%o'", type);
    }
}

static void
S_add_text_field(Schema *self, const CharBuf *field, FieldType *type) {
    FullTextType *fttype    = (FullTextType*)CERTIFY(type, FULLTEXTTYPE);
    Similarity   *sim       = FullTextType_Make_Similarity(fttype);
    Analyzer     *analyzer  = FullTextType_Get_Analyzer(fttype);

    // Cache helpers.
    Hash_Store(self->sims, (Obj*)field, (Obj*)sim);
    Hash_Store(self->analyzers, (Obj*)field, INCREF(analyzer));
    S_add_unique(self->uniq_analyzers, (Obj*)analyzer);

    // Store FieldType.
    Hash_Store(self->types, (Obj*)field, INCREF(type));
}

static void
S_add_string_field(Schema *self, const CharBuf *field, FieldType *type) {
    StringType *string_type = (StringType*)CERTIFY(type, STRINGTYPE);
    Similarity *sim         = StringType_Make_Similarity(string_type);

    // Cache helpers.
    Hash_Store(self->sims, (Obj*)field, (Obj*)sim);

    // Store FieldType.
    Hash_Store(self->types, (Obj*)field, INCREF(type));
}

static void
S_add_blob_field(Schema *self, const CharBuf *field, FieldType *type) {
    BlobType *blob_type = (BlobType*)CERTIFY(type, BLOBTYPE);
    Hash_Store(self->types, (Obj*)field, INCREF(blob_type));
}

static void
S_add_numeric_field(Schema *self, const CharBuf *field, FieldType *type) {
    NumericType *num_type = (NumericType*)CERTIFY(type, NUMERICTYPE);
    Hash_Store(self->types, (Obj*)field, INCREF(num_type));
}

FieldType*
Schema_fetch_type(Schema *self, const CharBuf *field) {
    return (FieldType*)Hash_Fetch(self->types, (Obj*)field);
}

Analyzer*
Schema_fetch_analyzer(Schema *self, const CharBuf *field) {
    return field
           ? (Analyzer*)Hash_Fetch(self->analyzers, (Obj*)field)
           : NULL;
}

Similarity*
Schema_fetch_sim(Schema *self, const CharBuf *field) {
    Similarity *sim = NULL;
    if (field != NULL) {
        sim = (Similarity*)Hash_Fetch(self->sims, (Obj*)field);
    }
    return sim;
}

uint32_t
Schema_num_fields(Schema *self) {
    return Hash_Get_Size(self->types);
}

Architecture*
Schema_get_architecture(Schema *self) {
    return self->arch;
}

Similarity*
Schema_get_similarity(Schema *self) {
    return self->sim;
}

VArray*
Schema_all_fields(Schema *self) {
    return Hash_Keys(self->types);
}

uint32_t
S_find_in_array(VArray *array, Obj *obj) {
    for (uint32_t i = 0, max = VA_Get_Size(array); i < max; i++) {
        Obj *candidate = VA_Fetch(array, i);
        if (obj == NULL && candidate == NULL) {
            return i;
        }
        else if (obj != NULL && candidate != NULL) {
            if (Obj_Get_VTable(obj) == Obj_Get_VTable(candidate)) {
                if (Obj_Equals(obj, candidate)) {
                    return i;
                }
            }
        }
    }
    THROW(ERR, "Couldn't find match for %o", obj);
    UNREACHABLE_RETURN(uint32_t);
}

Hash*
Schema_dump(Schema *self) {
    Hash *dump = Hash_new(0);
    Hash *type_dumps = Hash_new(Hash_Get_Size(self->types));
    CharBuf *field;
    FieldType *type;

    // Record class name, store dumps of unique Analyzers.
    Hash_Store_Str(dump, "_class", 6,
                   (Obj*)CB_Clone(Schema_Get_Class_Name(self)));
    Hash_Store_Str(dump, "analyzers", 9, (Obj*)VA_Dump(self->uniq_analyzers));

    // Dump FieldTypes.
    Hash_Store_Str(dump, "fields", 6, (Obj*)type_dumps);
    Hash_Iterate(self->types);
    while (Hash_Next(self->types, (Obj**)&field, (Obj**)&type)) {
        VTable *type_vtable = FType_Get_VTable(type);

        // Dump known types to simplified format.
        if (type_vtable == FULLTEXTTYPE) {
            FullTextType *fttype = (FullTextType*)type;
            Hash *type_dump = FullTextType_Dump_For_Schema(fttype);
            Analyzer *analyzer = FullTextType_Get_Analyzer(fttype);
            uint32_t tick
                = S_find_in_array(self->uniq_analyzers, (Obj*)analyzer);

            // Store the tick which references a unique analyzer.
            Hash_Store_Str(type_dump, "analyzer", 8,
                           (Obj*)CB_newf("%u32", tick));

            Hash_Store(type_dumps, (Obj*)field, (Obj*)type_dump);
        }
        else if (type_vtable == STRINGTYPE || type_vtable == BLOBTYPE) {
            Hash *type_dump = FType_Dump_For_Schema(type);
            Hash_Store(type_dumps, (Obj*)field, (Obj*)type_dump);
        }
        // Unknown FieldType type, so punt.
        else {
            Hash_Store(type_dumps, (Obj*)field, FType_Dump(type));
        }
    }

    return dump;
}

Schema*
Schema_load(Schema *self, Obj *dump) {
    Hash *source = (Hash*)CERTIFY(dump, HASH);
    CharBuf *class_name
        = (CharBuf*)CERTIFY(Hash_Fetch_Str(source, "_class", 6), CHARBUF);
    VTable *vtable = VTable_singleton(class_name, NULL);
    Schema *loaded = (Schema*)VTable_Make_Obj(vtable);
    Hash *type_dumps
        = (Hash*)CERTIFY(Hash_Fetch_Str(source, "fields", 6), HASH);
    VArray *analyzer_dumps
        = (VArray*)CERTIFY(Hash_Fetch_Str(source, "analyzers", 9), VARRAY);
    VArray *analyzers
        = (VArray*)VA_Load(analyzer_dumps, (Obj*)analyzer_dumps);
    CharBuf *field;
    Hash    *type_dump;
    UNUSED_VAR(self);

    // Start with a blank Schema.
    Schema_init(loaded);
    VA_Grow(loaded->uniq_analyzers, VA_Get_Size(analyzers));

    Hash_Iterate(type_dumps);
    while (Hash_Next(type_dumps, (Obj**)&field, (Obj**)&type_dump)) {
        CharBuf *type_str;
        CERTIFY(type_dump, HASH);
        type_str = (CharBuf*)Hash_Fetch_Str(type_dump, "type", 4);
        if (type_str) {
            if (CB_Equals_Str(type_str, "fulltext", 8)) {
                // Replace the "analyzer" tick with the real thing.
                Obj *tick
                    = CERTIFY(Hash_Fetch_Str(type_dump, "analyzer", 8), OBJ);
                Analyzer *analyzer
                    = (Analyzer*)VA_Fetch(analyzers,
                                          (uint32_t)Obj_To_I64(tick));
                if (!analyzer) {
                    THROW(ERR, "Can't find analyzer for '%o'", field);
                }
                Hash_Store_Str(type_dump, "analyzer", 8, INCREF(analyzer));
                FullTextType *type
                    = (FullTextType*)VTable_Load_Obj(FULLTEXTTYPE,
                                                     (Obj*)type_dump);
                Schema_Spec_Field(loaded, field, (FieldType*)type);
                DECREF(type);
            }
            else if (CB_Equals_Str(type_str, "string", 6)) {
                StringType *type
                    = (StringType*)VTable_Load_Obj(STRINGTYPE,
                                                   (Obj*)type_dump);
                Schema_Spec_Field(loaded, field, (FieldType*)type);
                DECREF(type);
            }
            else if (CB_Equals_Str(type_str, "blob", 4)) {
                BlobType *type
                    = (BlobType*)VTable_Load_Obj(BLOBTYPE, (Obj*)type_dump);
                Schema_Spec_Field(loaded, field, (FieldType*)type);
                DECREF(type);
            }
            else if (CB_Equals_Str(type_str, "i32_t", 5)) {
                Int32Type *type
                    = (Int32Type*)VTable_Load_Obj(INT32TYPE, (Obj*)type_dump);
                Schema_Spec_Field(loaded, field, (FieldType*)type);
                DECREF(type);
            }
            else if (CB_Equals_Str(type_str, "i64_t", 5)) {
                Int64Type *type
                    = (Int64Type*)VTable_Load_Obj(INT64TYPE, (Obj*)type_dump);
                Schema_Spec_Field(loaded, field, (FieldType*)type);
                DECREF(type);
            }
            else if (CB_Equals_Str(type_str, "f32_t", 5)) {
                Float32Type *type
                    = (Float32Type*)VTable_Load_Obj(FLOAT32TYPE,
                                                    (Obj*)type_dump);
                Schema_Spec_Field(loaded, field, (FieldType*)type);
                DECREF(type);
            }
            else if (CB_Equals_Str(type_str, "f64_t", 5)) {
                Float64Type *type
                    = (Float64Type*)VTable_Load_Obj(FLOAT64TYPE,
                                                    (Obj*)type_dump);
                Schema_Spec_Field(loaded, field, (FieldType*)type);
                DECREF(type);
            }
            else {
                THROW(ERR, "Unknown type '%o' for field '%o'", type_str, field);
            }
        }
        else {
            FieldType *type = (FieldType*)CERTIFY(
                                  Hash_Load(type_dump, (Obj*)type_dump),
                                  FIELDTYPE);
            Schema_Spec_Field(loaded, field, type);
            DECREF(type);
        }
    }

    DECREF(analyzers);

    return loaded;
}

void
Schema_eat(Schema *self, Schema *other) {
    if (!Schema_Is_A(self, Schema_Get_VTable(other))) {
        THROW(ERR, "%o not a descendent of %o",
              Schema_Get_Class_Name(self), Schema_Get_Class_Name(other));
    }

    CharBuf *field;
    FieldType *type;
    Hash_Iterate(other->types);
    while (Hash_Next(other->types, (Obj**)&field, (Obj**)&type)) {
        Schema_Spec_Field(self, field, type);
    }
}

void
Schema_write(Schema *self, Folder *folder, const CharBuf *filename) {
    Hash *dump = Schema_Dump(self);
    ZombieCharBuf *schema_temp = ZCB_WRAP_STR("schema.temp", 11);
    bool_t success;
    Folder_Delete(folder, (CharBuf*)schema_temp); // Just in case.
    Json_spew_json((Obj*)dump, folder, (CharBuf*)schema_temp);
    success = Folder_Rename(folder, (CharBuf*)schema_temp, filename);
    DECREF(dump);
    if (!success) { RETHROW(INCREF(Err_get_error())); }
}


