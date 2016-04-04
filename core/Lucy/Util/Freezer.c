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

#define C_LUCY_FREEZER
#include "Lucy/Util/ToolSet.h"

#include "Clownfish/Blob.h"
#include "Clownfish/Boolean.h"
#include "Clownfish/HashIterator.h"
#include "Clownfish/Num.h"
#include "Lucy/Util/Freezer.h"
#include "Lucy/Store/InStream.h"
#include "Lucy/Store/OutStream.h"
#include "Lucy/Analysis/Analyzer.h"
#include "Lucy/Document/Doc.h"
#include "Lucy/Index/Similarity.h"
#include "Lucy/Index/DocVector.h"
#include "Lucy/Index/TermVector.h"
#include "Lucy/Plan/FieldType.h"
#include "Lucy/Plan/Schema.h"
#include "Lucy/Search/Query.h"
#include "Lucy/Search/SortRule.h"
#include "Lucy/Search/SortSpec.h"
#include "Lucy/Search/MatchDoc.h"
#include "Lucy/Search/TopDocs.h"

void
Freezer_freeze(Obj *obj, OutStream *outstream) {
    Freezer_serialize_string(Obj_get_class_name(obj), outstream);
    Freezer_serialize(obj, outstream);
}

Obj*
Freezer_thaw(InStream *instream) {
    String *class_name = Freezer_read_string(instream);
    Class *klass = Class_singleton(class_name, NULL);
    Obj *blank = Class_Make_Obj(klass);
    DECREF(class_name);
    return Freezer_deserialize(blank, instream);
}

void
Freezer_serialize(Obj *obj, OutStream *outstream) {
    if (Obj_is_a(obj, STRING)) {
        Freezer_serialize_string((String*)obj, outstream);
    }
    else if (Obj_is_a(obj, BLOB)) {
        Freezer_serialize_blob((Blob*)obj, outstream);
    }
    else if (Obj_is_a(obj, VECTOR)) {
        Freezer_serialize_varray((Vector*)obj, outstream);
    }
    else if (Obj_is_a(obj, HASH)) {
        Freezer_serialize_hash((Hash*)obj, outstream);
    }
    else if (Obj_is_a(obj, INTEGER)) {
        int64_t val = Int_Get_Value((Integer*)obj);
        OutStream_Write_C64(outstream, (uint64_t)val);
    }
    else if (Obj_is_a(obj, FLOAT)) {
        double val = Float_Get_Value((Float*)obj);
        OutStream_Write_F64(outstream, val);
    }
    else if (Obj_is_a(obj, BOOLEAN)) {
        bool val = Bool_Get_Value((Boolean*)obj);
        OutStream_Write_U8(outstream, (uint8_t)val);
    }
    else if (Obj_is_a(obj, QUERY)) {
        Query_Serialize((Query*)obj, outstream);
    }
    else if (Obj_is_a(obj, DOC)) {
        Doc_Serialize((Doc*)obj, outstream);
    }
    else if (Obj_is_a(obj, DOCVECTOR)) {
        DocVec_Serialize((DocVector*)obj, outstream);
    }
    else if (Obj_is_a(obj, TERMVECTOR)) {
        TV_Serialize((TermVector*)obj, outstream);
    }
    else if (Obj_is_a(obj, SIMILARITY)) {
        Sim_Serialize((Similarity*)obj, outstream);
    }
    else if (Obj_is_a(obj, MATCHDOC)) {
        MatchDoc_Serialize((MatchDoc*)obj, outstream);
    }
    else if (Obj_is_a(obj, TOPDOCS)) {
        TopDocs_Serialize((TopDocs*)obj, outstream);
    }
    else if (Obj_is_a(obj, SORTSPEC)) {
        SortSpec_Serialize((SortSpec*)obj, outstream);
    }
    else if (Obj_is_a(obj, SORTRULE)) {
        SortRule_Serialize((SortRule*)obj, outstream);
    }
    else {
        THROW(ERR, "Don't know how to serialize a %o",
              Obj_get_class_name(obj));
    }
}

Obj*
Freezer_deserialize(Obj *obj, InStream *instream) {
    if (Obj_is_a(obj, STRING)) {
        obj = (Obj*)Freezer_deserialize_string((String*)obj, instream);
    }
    else if (Obj_is_a(obj, BLOB)) {
        obj = (Obj*)Freezer_deserialize_blob((Blob*)obj, instream);
    }
    else if (Obj_is_a(obj, VECTOR)) {
        obj = (Obj*)Freezer_deserialize_varray((Vector*)obj, instream);
    }
    else if (Obj_is_a(obj, HASH)) {
        obj = (Obj*)Freezer_deserialize_hash((Hash*)obj, instream);
    }
    else if (Obj_is_a(obj, INTEGER)) {
        int64_t value = (int64_t)InStream_Read_C64(instream);
        obj = (Obj*)Int_init((Integer*)obj, value);
    }
    else if (Obj_is_a(obj, FLOAT)) {
        double value = InStream_Read_F64(instream);
        obj = (Obj*)Float_init((Float*)obj, value);
    }
    else if (Obj_is_a(obj, BOOLEAN)) {
        bool value = !!InStream_Read_U8(instream);
        Obj *result = value ? INCREF(CFISH_TRUE) : INCREF(CFISH_FALSE);
        // FIXME: This DECREF is essentially a no-op causing a
        // memory leak.
        DECREF(obj);
        obj = result;
    }
    else if (Obj_is_a(obj, QUERY)) {
        obj = (Obj*)Query_Deserialize((Query*)obj, instream);
    }
    else if (Obj_is_a(obj, DOC)) {
        obj = (Obj*)Doc_Deserialize((Doc*)obj, instream);
    }
    else if (Obj_is_a(obj, DOCVECTOR)) {
        obj = (Obj*)DocVec_Deserialize((DocVector*)obj, instream);
    }
    else if (Obj_is_a(obj, TERMVECTOR)) {
        obj = (Obj*)TV_Deserialize((TermVector*)obj, instream);
    }
    else if (Obj_is_a(obj, SIMILARITY)) {
        obj = (Obj*)Sim_Deserialize((Similarity*)obj, instream);
    }
    else if (Obj_is_a(obj, MATCHDOC)) {
        obj = (Obj*)MatchDoc_Deserialize((MatchDoc*)obj, instream);
    }
    else if (Obj_is_a(obj, TOPDOCS)) {
        obj = (Obj*)TopDocs_Deserialize((TopDocs*)obj, instream);
    }
    else if (Obj_is_a(obj, SORTSPEC)) {
        obj = (Obj*)SortSpec_Deserialize((SortSpec*)obj, instream);
    }
    else if (Obj_is_a(obj, SORTRULE)) {
        obj = (Obj*)SortRule_Deserialize((SortRule*)obj, instream);
    }
    else {
        THROW(ERR, "Don't know how to deserialize a %o",
              Obj_get_class_name(obj));
    }

    return obj;
}

void
Freezer_serialize_string(String *string, OutStream *outstream) {
    size_t      size = Str_Get_Size(string);
    const char *buf  = Str_Get_Ptr8(string);
    OutStream_Write_C64(outstream, size);
    OutStream_Write_Bytes(outstream, buf, size);
}

String*
Freezer_deserialize_string(String *string, InStream *instream) {
    size_t size = InStream_Read_C32(instream);
    if (size == SIZE_MAX) {
        THROW(ERR, "Can't deserialize SIZE_MAX bytes");
    }
    char *buf = (char*)MALLOCATE(size + 1);
    InStream_Read_Bytes(instream, buf, size);
    buf[size] = '\0';
    if (!StrHelp_utf8_valid(buf, size)) {
        THROW(ERR, "Attempt to deserialize invalid UTF-8");
    }
    return Str_init_steal_trusted_utf8(string, buf, size);
}

String*
Freezer_read_string(InStream *instream) {
    String *string = (String*)Class_Make_Obj(STRING);
    return Freezer_deserialize_string(string, instream);
}

void
Freezer_serialize_blob(Blob *blob, OutStream *outstream) {
    size_t size = Blob_Get_Size(blob);
    OutStream_Write_C32(outstream, size);
    OutStream_Write_Bytes(outstream, Blob_Get_Buf(blob), size);
}

Blob*
Freezer_deserialize_blob(Blob *blob, InStream *instream) {
    size_t size = InStream_Read_C32(instream);
    char   *buf = (char*)MALLOCATE(size);
    InStream_Read_Bytes(instream, buf, size);
    return Blob_init_steal(blob, buf, size);
}

Blob*
Freezer_read_blob(InStream *instream) {
    Blob *blob = (Blob*)Class_Make_Obj(BLOB);
    return Freezer_deserialize_blob(blob, instream);
}

void
Freezer_serialize_varray(Vector *array, OutStream *outstream) {
    uint32_t last_valid_tick = 0;
    uint32_t size = (uint32_t)Vec_Get_Size(array);
    OutStream_Write_C32(outstream, size);
    for (uint32_t i = 0; i < size; i++) {
        Obj *elem = Vec_Fetch(array, i);
        if (elem) {
            OutStream_Write_C32(outstream, i - last_valid_tick);
            FREEZE(elem, outstream);
            last_valid_tick = i;
        }
    }
    // Terminate.
    OutStream_Write_C32(outstream, size - last_valid_tick);
}

Vector*
Freezer_deserialize_varray(Vector *array, InStream *instream) {
    uint32_t size = InStream_Read_C32(instream);
    Vec_init(array, size);
    for (uint32_t tick = InStream_Read_C32(instream);
         tick < size;
         tick += InStream_Read_C32(instream)
        ) {
        Obj *obj = THAW(instream);
        Vec_Store(array, tick, obj);
    }
    Vec_Resize(array, size);
    return array;
}

Vector*
Freezer_read_varray(InStream *instream) {
    Vector *array = (Vector*)Class_Make_Obj(VECTOR);
    return Freezer_deserialize_varray(array, instream);
}

void
Freezer_serialize_hash(Hash *hash, OutStream *outstream) {
    uint32_t hash_size = Hash_Get_Size(hash);
    OutStream_Write_C32(outstream, hash_size);

    HashIterator *iter = HashIter_new(hash);
    while (HashIter_Next(iter)) {
        String *key = HashIter_Get_Key(iter);
        Obj    *val = HashIter_Get_Value(iter);
        Freezer_serialize_string(key, outstream);
        FREEZE(val, outstream);
    }
    DECREF(iter);
}

Hash*
Freezer_deserialize_hash(Hash *hash, InStream *instream) {
    uint32_t size = InStream_Read_C32(instream);

    Hash_init(hash, size);

    while (size--) {
        uint32_t len = InStream_Read_C32(instream);
        char *key_buf = (char*)MALLOCATE(len + 1);
        InStream_Read_Bytes(instream, key_buf, len);
        key_buf[len] = '\0';
        String *key = Str_new_steal_utf8(key_buf, len);
        Hash_Store(hash, key, THAW(instream));
        DECREF(key);
    }

    return hash;
}

Hash*
Freezer_read_hash(InStream *instream) {
    Hash *hash = (Hash*)Class_Make_Obj(HASH);
    return Freezer_deserialize_hash(hash, instream);
}

static Obj*
S_dump_array(Vector *array) {
    Vector *dump = Vec_new(Vec_Get_Size(array));
    for (size_t i = 0, max = Vec_Get_Size(array); i < max; i++) {
        Obj *elem = Vec_Fetch(array, i);
        if (elem) {
            Vec_Store(dump, i, Freezer_dump(elem));
        }
    }
    return (Obj*)dump;
}

Obj*
S_dump_hash(Hash *hash) {
    Hash *dump = Hash_new(Hash_Get_Size(hash));

    HashIterator *iter = HashIter_new(hash);
    while (HashIter_Next(iter)) {
        String *key   = HashIter_Get_Key(iter);
        Obj    *value = HashIter_Get_Value(iter);
        Hash_Store(dump, key, Freezer_dump(value));
    }
    DECREF(iter);

    return (Obj*)dump;
}

Obj*
Freezer_dump(Obj *obj) {
    if (Obj_is_a(obj, STRING)) {
        return (Obj*)Obj_To_String(obj);
    }
    else if (Obj_is_a(obj, VECTOR)) {
        return S_dump_array((Vector*)obj);
    }
    else if (Obj_is_a(obj, HASH)) {
        return S_dump_hash((Hash*)obj);
    }
    else if (Obj_is_a(obj, ANALYZER)) {
        return Analyzer_Dump((Analyzer*)obj);
    }
    else if (Obj_is_a(obj, DOC)) {
        return (Obj*)Doc_Dump((Doc*)obj);
    }
    else if (Obj_is_a(obj, SIMILARITY)) {
        return Sim_Dump((Similarity*)obj);
    }
    else if (Obj_is_a(obj, FIELDTYPE)) {
        return FType_Dump((FieldType*)obj);
    }
    else if (Obj_is_a(obj, SCHEMA)) {
        return (Obj*)Schema_Dump((Schema*)obj);
    }
    else if (Obj_is_a(obj, QUERY)) {
        return Query_Dump((Query*)obj);
    }
    else if (Obj_is_a(obj, FLOAT)
             || Obj_is_a(obj, INTEGER)
             || Obj_is_a(obj, BOOLEAN)) {
        return Obj_Clone(obj);
    }
    else {
        return (Obj*)Obj_To_String(obj);
    }
}

static Obj*
S_load_via_load_method(Class *klass, Obj *dump) {
    Obj *dummy = Class_Make_Obj(klass);
    Obj *loaded = NULL;
    if (Obj_is_a(dummy, ANALYZER)) {
        loaded = Analyzer_Load((Analyzer*)dummy, dump);
    }
    else if (Obj_is_a(dummy, DOC)) {
        loaded = (Obj*)Doc_Load((Doc*)dummy, dump);
    }
    else if (Obj_is_a(dummy, SIMILARITY)) {
        loaded = (Obj*)Sim_Load((Similarity*)dummy, dump);
    }
    else if (Obj_is_a(dummy, FIELDTYPE)) {
        loaded = FType_Load((FieldType*)dummy, dump);
    }
    else if (Obj_is_a(dummy, SCHEMA)) {
        loaded = (Obj*)Schema_Load((Schema*)dummy, dump);
    }
    else if (Obj_is_a(dummy, QUERY)) {
        loaded = Query_Load((Query*)dummy, dump);
    }
    else {
        DECREF(dummy);
        THROW(ERR, "Don't know how to load '%o'", Class_Get_Name(klass));
    }

    DECREF(dummy);
    return loaded;
}

static Obj*
S_load_from_hash(Hash *dump) {
    String *class_name = (String*)Hash_Fetch_Utf8(dump, "_class", 6);

    // Assume that the presence of the "_class" key paired with a valid class
    // name indicates the output of a dump() rather than an ordinary Hash.
    if (class_name && Str_is_a(class_name, STRING)) {
        Class *klass = Class_fetch_class(class_name);

        if (!klass) {
            String *parent_class_name = Class_find_parent_class(class_name);
            if (parent_class_name) {
                Class *parent = Class_singleton(parent_class_name, NULL);
                klass = Class_singleton(class_name, parent);
                DECREF(parent_class_name);
            }
            else {
                // TODO: Fix load() so that it works with ordinary hash keys
                // named "_class".
                THROW(ERR, "Can't find class '%o'", class_name);
            }
        }

        // Dispatch to an alternate Load() method.
        if (klass) {
            return S_load_via_load_method(klass, (Obj*)dump);
        }

    }

    // It's an ordinary Hash.
    Hash *loaded = Hash_new(Hash_Get_Size(dump));

    HashIterator *iter = HashIter_new(dump);
    while (HashIter_Next(iter)) {
        String *key   = HashIter_Get_Key(iter);
        Obj    *value = HashIter_Get_Value(iter);
        Hash_Store(loaded, key, Freezer_load(value));
    }
    DECREF(iter);

    return (Obj*)loaded;

}


Obj*
S_load_from_array(Vector *dump) {
    Vector *loaded = Vec_new(Vec_Get_Size(dump));

    for (size_t i = 0, max = Vec_Get_Size(dump); i < max; i++) {
        Obj *elem_dump = Vec_Fetch(dump, i);
        if (elem_dump) {
            Vec_Store(loaded, i, Freezer_load(elem_dump));
        }
    }

    return (Obj*)loaded;
}

Obj*
Freezer_load(Obj *obj) {
    if (Obj_is_a(obj, HASH)) {
        return S_load_from_hash((Hash*)obj);
    }
    else if (Obj_is_a(obj, VECTOR)) {
        return S_load_from_array((Vector*)obj);
    }
    else {
        return Obj_Clone(obj);
    }
}

