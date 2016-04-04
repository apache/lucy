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

#include "Clownfish/HashIterator.h"
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
#include "Lucy/Util/Freezer.h"
#include "Lucy/Util/Json.h"

// Scan the array to see if an object testing as Equal is present.  If not,
// push the elem onto the end of the array.
static void
S_add_unique(Vector *array, Obj *elem);

static void
S_add_text_field(Schema *self, String *field, FieldType *type);
static void
S_add_string_field(Schema *self, String *field, FieldType *type);
static void
S_add_blob_field(Schema *self, String *field, FieldType *type);
static void
S_add_numeric_field(Schema *self, String *field, FieldType *type);

Schema*
Schema_new() {
    Schema *self = (Schema*)Class_Make_Obj(SCHEMA);
    return Schema_init(self);
}

Schema*
Schema_init(Schema *self) {
    SchemaIVARS *const ivars = Schema_IVARS(self);
    // Init.
    ivars->analyzers      = Hash_new(0);
    ivars->types          = Hash_new(0);
    ivars->sims           = Hash_new(0);
    ivars->uniq_analyzers = Vec_new(2);
    Vec_Resize(ivars->uniq_analyzers, 1);

    // Assign.
    ivars->arch = Schema_Architecture(self);
    ivars->sim  = Arch_Make_Similarity(ivars->arch);

    return self;
}

void
Schema_Destroy_IMP(Schema *self) {
    SchemaIVARS *const ivars = Schema_IVARS(self);
    DECREF(ivars->arch);
    DECREF(ivars->analyzers);
    DECREF(ivars->uniq_analyzers);
    DECREF(ivars->types);
    DECREF(ivars->sims);
    DECREF(ivars->sim);
    SUPER_DESTROY(self, SCHEMA);
}

static void
S_add_unique(Vector *array, Obj *elem) {
    if (!elem) { return; }
    for (size_t i = 0, max = Vec_Get_Size(array); i < max; i++) {
        Obj *candidate = Vec_Fetch(array, i);
        if (!candidate) { continue; }
        if (elem == candidate) { return; }
        if (Obj_get_class(elem) == Obj_get_class(candidate)) {
            if (Obj_Equals(elem, candidate)) { return; }
        }
    }
    Vec_Push(array, INCREF(elem));
}

bool
Schema_Equals_IMP(Schema *self, Obj *other) {
    if ((Schema*)other == self)                         { return true; }
    if (!Obj_is_a(other, SCHEMA))                       { return false; }
    SchemaIVARS *const ivars = Schema_IVARS(self);
    SchemaIVARS *const ovars = Schema_IVARS((Schema*)other);
    if (!Arch_Equals(ivars->arch, (Obj*)ovars->arch))   { return false; }
    if (!Sim_Equals(ivars->sim, (Obj*)ovars->sim))      { return false; }
    if (!Hash_Equals(ivars->types, (Obj*)ovars->types)) { return false; }
    return true;
}

Architecture*
Schema_Architecture_IMP(Schema *self) {
    UNUSED_VAR(self);
    return Arch_new();
}

void
Schema_Spec_Field_IMP(Schema *self, String *field, FieldType *type) {
    FieldType *existing  = Schema_Fetch_Type(self, field);

    // If the field already has an association, verify pairing and return.
    if (existing) {
        if (FType_Equals(type, (Obj*)existing)) { return; }
        else { THROW(ERR, "'%o' assigned conflicting FieldType", field); }
    }

    if (FType_is_a(type, FULLTEXTTYPE)) {
        S_add_text_field(self, field, type);
    }
    else if (FType_is_a(type, STRINGTYPE)) {
        S_add_string_field(self, field, type);
    }
    else if (FType_is_a(type, BLOBTYPE)) {
        S_add_blob_field(self, field, type);
    }
    else if (FType_is_a(type, NUMERICTYPE)) {
        S_add_numeric_field(self, field, type);
    }
    else {
        THROW(ERR, "Unrecognized field type: '%o'", type);
    }
}

static void
S_add_text_field(Schema *self, String *field, FieldType *type) {
    SchemaIVARS *const ivars = Schema_IVARS(self);
    FullTextType *fttype    = (FullTextType*)CERTIFY(type, FULLTEXTTYPE);
    Similarity   *sim       = FullTextType_Make_Similarity(fttype);
    Analyzer     *analyzer  = FullTextType_Get_Analyzer(fttype);

    // Cache helpers.
    Hash_Store(ivars->sims, field, (Obj*)sim);
    Hash_Store(ivars->analyzers, field, INCREF(analyzer));
    S_add_unique(ivars->uniq_analyzers, (Obj*)analyzer);

    // Store FieldType.
    Hash_Store(ivars->types, field, INCREF(type));
}

static void
S_add_string_field(Schema *self, String *field, FieldType *type) {
    SchemaIVARS *const ivars = Schema_IVARS(self);
    StringType *string_type = (StringType*)CERTIFY(type, STRINGTYPE);
    Similarity *sim         = StringType_Make_Similarity(string_type);

    // Cache helpers.
    Hash_Store(ivars->sims, field, (Obj*)sim);

    // Store FieldType.
    Hash_Store(ivars->types, field, INCREF(type));
}

static void
S_add_blob_field(Schema *self, String *field, FieldType *type) {
    SchemaIVARS *const ivars = Schema_IVARS(self);
    BlobType *blob_type = (BlobType*)CERTIFY(type, BLOBTYPE);
    Hash_Store(ivars->types, field, INCREF(blob_type));
}

static void
S_add_numeric_field(Schema *self, String *field, FieldType *type) {
    SchemaIVARS *const ivars = Schema_IVARS(self);
    NumericType *num_type = (NumericType*)CERTIFY(type, NUMERICTYPE);
    Hash_Store(ivars->types, field, INCREF(num_type));
}

FieldType*
Schema_Fetch_Type_IMP(Schema *self, String *field) {
    SchemaIVARS *const ivars = Schema_IVARS(self);
    return (FieldType*)Hash_Fetch(ivars->types, field);
}

Analyzer*
Schema_Fetch_Analyzer_IMP(Schema *self, String *field) {
    SchemaIVARS *const ivars = Schema_IVARS(self);
    return field
           ? (Analyzer*)Hash_Fetch(ivars->analyzers, field)
           : NULL;
}

Similarity*
Schema_Fetch_Sim_IMP(Schema *self, String *field) {
    SchemaIVARS *const ivars = Schema_IVARS(self);
    Similarity *sim = NULL;
    if (field != NULL) {
        sim = (Similarity*)Hash_Fetch(ivars->sims, field);
    }
    return sim;
}

uint32_t
Schema_Num_Fields_IMP(Schema *self) {
    SchemaIVARS *const ivars = Schema_IVARS(self);
    return Hash_Get_Size(ivars->types);
}

Architecture*
Schema_Get_Architecture_IMP(Schema *self) {
    return Schema_IVARS(self)->arch;
}

Similarity*
Schema_Get_Similarity_IMP(Schema *self) {
    return Schema_IVARS(self)->sim;
}

Vector*
Schema_All_Fields_IMP(Schema *self) {
    return Hash_Keys(Schema_IVARS(self)->types);
}

size_t
S_find_in_array(Vector *array, Obj *obj) {
    for (size_t i = 0, max = Vec_Get_Size(array); i < max; i++) {
        Obj *candidate = Vec_Fetch(array, i);
        if (obj == NULL && candidate == NULL) {
            return i;
        }
        else if (obj != NULL && candidate != NULL) {
            if (Obj_get_class(obj) == Obj_get_class(candidate)) {
                if (Obj_Equals(obj, candidate)) {
                    return i;
                }
            }
        }
    }
    THROW(ERR, "Couldn't find match for %o", obj);
    UNREACHABLE_RETURN(size_t);
}

Hash*
Schema_Dump_IMP(Schema *self) {
    SchemaIVARS *const ivars = Schema_IVARS(self);
    Hash *dump = Hash_new(0);
    Hash *type_dumps = Hash_new(Hash_Get_Size(ivars->types));

    // Record class name, store dumps of unique Analyzers.
    Hash_Store_Utf8(dump, "_class", 6,
                    (Obj*)Str_Clone(Schema_get_class_name(self)));
    Hash_Store_Utf8(dump, "analyzers", 9,
                    Freezer_dump((Obj*)ivars->uniq_analyzers));

    // Dump FieldTypes.
    Hash_Store_Utf8(dump, "fields", 6, (Obj*)type_dumps);
    HashIterator *iter = HashIter_new(ivars->types);
    while (HashIter_Next(iter)) {
        String    *field      = HashIter_Get_Key(iter);
        FieldType *type       = (FieldType*)HashIter_Get_Value(iter);
        Class     *type_class = FType_get_class(type);

        // Dump known types to simplified format.
        if (type_class == FULLTEXTTYPE) {
            FullTextType *fttype = (FullTextType*)type;
            Hash *type_dump = FullTextType_Dump_For_Schema(fttype);
            Analyzer *analyzer = FullTextType_Get_Analyzer(fttype);
            size_t tick
                = S_find_in_array(ivars->uniq_analyzers, (Obj*)analyzer);

            // Store the tick which references a unique analyzer.
            Hash_Store_Utf8(type_dump, "analyzer", 8,
                            (Obj*)Str_newf("%u64", (uint64_t)tick));

            Hash_Store(type_dumps, field, (Obj*)type_dump);
        }
        else if (type_class == STRINGTYPE || type_class == BLOBTYPE) {
            Hash *type_dump = FType_Dump_For_Schema(type);
            Hash_Store(type_dumps, field, (Obj*)type_dump);
        }
        // Unknown FieldType type, so punt.
        else {
            Hash_Store(type_dumps, field, FType_Dump(type));
        }
    }
    DECREF(iter);

    return dump;
}

static FieldType*
S_load_type(Class *klass, Obj *type_dump) {
    FieldType *dummy = (FieldType*)Class_Make_Obj(klass);
    FieldType *loaded = (FieldType*)FType_Load(dummy, type_dump);
    DECREF(dummy);
    return loaded;
}

Schema*
Schema_Load_IMP(Schema *self, Obj *dump) {
    Hash *source = (Hash*)CERTIFY(dump, HASH);
    String *class_name
        = (String*)CERTIFY(Hash_Fetch_Utf8(source, "_class", 6), STRING);
    Class *klass = Class_singleton(class_name, NULL);
    Schema *loaded = (Schema*)Class_Make_Obj(klass);
    Hash *type_dumps
        = (Hash*)CERTIFY(Hash_Fetch_Utf8(source, "fields", 6), HASH);
    Vector *analyzer_dumps
        = (Vector*)CERTIFY(Hash_Fetch_Utf8(source, "analyzers", 9), VECTOR);
    Vector *analyzers
        = (Vector*)Freezer_load((Obj*)analyzer_dumps);
    UNUSED_VAR(self);

    // Start with a blank Schema.
    Schema_init(loaded);
    SchemaIVARS *const loaded_ivars = Schema_IVARS(loaded);
    Vec_Grow(loaded_ivars->uniq_analyzers, Vec_Get_Size(analyzers));

    HashIterator *iter = HashIter_new(type_dumps);
    while (HashIter_Next(iter)) {
        String *field     = HashIter_Get_Key(iter);
        Hash   *type_dump = (Hash*)CERTIFY(HashIter_Get_Value(iter), HASH);
        String *type_str  = (String*)Hash_Fetch_Utf8(type_dump, "type", 4);
        if (type_str) {
            if (Str_Equals_Utf8(type_str, "fulltext", 8)) {
                // Replace the "analyzer" tick with the real thing.
                Obj *tick
                    = CERTIFY(Hash_Fetch_Utf8(type_dump, "analyzer", 8), OBJ);
                Analyzer *analyzer
                    = (Analyzer*)Vec_Fetch(analyzers,
                                          (uint32_t)Json_obj_to_i64(tick));
                if (!analyzer) {
                    THROW(ERR, "Can't find analyzer for '%o'", field);
                }
                Hash_Store_Utf8(type_dump, "analyzer", 8, INCREF(analyzer));
                FullTextType *type
                    = (FullTextType*)S_load_type(FULLTEXTTYPE,
                                                 (Obj*)type_dump);
                Schema_Spec_Field(loaded, field, (FieldType*)type);
                DECREF(type);
            }
            else if (Str_Equals_Utf8(type_str, "string", 6)) {
                StringType *type
                    = (StringType*)S_load_type(STRINGTYPE, (Obj*)type_dump);
                Schema_Spec_Field(loaded, field, (FieldType*)type);
                DECREF(type);
            }
            else if (Str_Equals_Utf8(type_str, "blob", 4)) {
                BlobType *type
                    = (BlobType*)S_load_type(BLOBTYPE, (Obj*)type_dump);
                Schema_Spec_Field(loaded, field, (FieldType*)type);
                DECREF(type);
            }
            else if (Str_Equals_Utf8(type_str, "i32_t", 5)) {
                Int32Type *type
                    = (Int32Type*)S_load_type(INT32TYPE, (Obj*)type_dump);
                Schema_Spec_Field(loaded, field, (FieldType*)type);
                DECREF(type);
            }
            else if (Str_Equals_Utf8(type_str, "i64_t", 5)) {
                Int64Type *type
                    = (Int64Type*)S_load_type(INT64TYPE, (Obj*)type_dump);
                Schema_Spec_Field(loaded, field, (FieldType*)type);
                DECREF(type);
            }
            else if (Str_Equals_Utf8(type_str, "f32_t", 5)) {
                Float32Type *type
                    = (Float32Type*)S_load_type(FLOAT32TYPE, (Obj*)type_dump);
                Schema_Spec_Field(loaded, field, (FieldType*)type);
                DECREF(type);
            }
            else if (Str_Equals_Utf8(type_str, "f64_t", 5)) {
                Float64Type *type
                    = (Float64Type*)S_load_type(FLOAT64TYPE, (Obj*)type_dump);
                Schema_Spec_Field(loaded, field, (FieldType*)type);
                DECREF(type);
            }
            else {
                THROW(ERR, "Unknown type '%o' for field '%o'", type_str, field);
            }
        }
        else {
            FieldType *type
                = (FieldType*)CERTIFY(Freezer_load((Obj*)type_dump),
                                      FIELDTYPE);
            Schema_Spec_Field(loaded, field, type);
            DECREF(type);
        }
    }
    DECREF(iter);

    DECREF(analyzers);

    return loaded;
}

void
Schema_Eat_IMP(Schema *self, Schema *other) {
    if (!Schema_is_a(self, Schema_get_class(other))) {
        THROW(ERR, "%o not a descendent of %o",
              Schema_get_class_name(self), Schema_get_class_name(other));
    }

    SchemaIVARS *const ovars = Schema_IVARS(other);
    HashIterator *iter = HashIter_new(ovars->types);
    while (HashIter_Next(iter)) {
        String    *field = HashIter_Get_Key(iter);
        FieldType *type  = (FieldType*)HashIter_Get_Value(iter);
        Schema_Spec_Field(self, field, type);
    }
    DECREF(iter);
}

void
Schema_Write_IMP(Schema *self, Folder *folder, String *filename) {
    Hash *dump = Schema_Dump(self);
    String *schema_temp = SSTR_WRAP_C("schema.temp");
    bool success;
    Folder_Delete(folder, schema_temp); // Just in case.
    Json_spew_json((Obj*)dump, folder, schema_temp);
    success = Folder_Rename(folder, schema_temp, filename);
    DECREF(dump);
    if (!success) { RETHROW(INCREF(Err_get_error())); }
}


