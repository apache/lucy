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
    Freezer_serialize_string(Obj_Get_Class_Name(obj), outstream);
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
    if (Obj_Is_A(obj, STRING)) {
        Freezer_serialize_string((String*)obj, outstream);
    }
    else if (Obj_Is_A(obj, BYTEBUF)) {
        Freezer_serialize_bytebuf((ByteBuf*)obj, outstream);
    }
    else if (Obj_Is_A(obj, VARRAY)) {
        Freezer_serialize_varray((VArray*)obj, outstream);
    }
    else if (Obj_Is_A(obj, HASH)) {
        Freezer_serialize_hash((Hash*)obj, outstream);
    }
    else if (Obj_Is_A(obj, NUM)) {
        if (Obj_Is_A(obj, INTNUM)) {
            if (Obj_Is_A(obj, BOOLNUM)) {
                bool val = Bool_Get_Value((BoolNum*)obj);
                OutStream_Write_U8(outstream, (uint8_t)val);
            }
            else if (Obj_Is_A(obj, INTEGER32)) {
                int32_t val = Int32_Get_Value((Integer32*)obj);
                OutStream_Write_C32(outstream, (uint32_t)val);
            }
            else if (Obj_Is_A(obj, INTEGER64)) {
                int64_t val = Int64_Get_Value((Integer64*)obj);
                OutStream_Write_C64(outstream, (uint64_t)val);
            }
        }
        else if (Obj_Is_A(obj, FLOATNUM)) {
            if (Obj_Is_A(obj, FLOAT32)) {
                float val = Float32_Get_Value((Float32*)obj);
                OutStream_Write_F32(outstream, val);
            }
            else if (Obj_Is_A(obj, FLOAT64)) {
                double val = Float64_Get_Value((Float64*)obj);
                OutStream_Write_F64(outstream, val);
            }
        }
    }
    else if (Obj_Is_A(obj, QUERY)) {
        Query_Serialize((Query*)obj, outstream);
    }
    else if (Obj_Is_A(obj, DOC)) {
        Doc_Serialize((Doc*)obj, outstream);
    }
    else if (Obj_Is_A(obj, DOCVECTOR)) {
        DocVec_Serialize((DocVector*)obj, outstream);
    }
    else if (Obj_Is_A(obj, TERMVECTOR)) {
        TV_Serialize((TermVector*)obj, outstream);
    }
    else if (Obj_Is_A(obj, SIMILARITY)) {
        Sim_Serialize((Similarity*)obj, outstream);
    }
    else if (Obj_Is_A(obj, MATCHDOC)) {
        MatchDoc_Serialize((MatchDoc*)obj, outstream);
    }
    else if (Obj_Is_A(obj, TOPDOCS)) {
        TopDocs_Serialize((TopDocs*)obj, outstream);
    }
    else if (Obj_Is_A(obj, SORTSPEC)) {
        SortSpec_Serialize((SortSpec*)obj, outstream);
    }
    else if (Obj_Is_A(obj, SORTRULE)) {
        SortRule_Serialize((SortRule*)obj, outstream);
    }
    else {
        THROW(ERR, "Don't know how to serialize a %o",
              Obj_Get_Class_Name(obj));
    }
}

Obj*
Freezer_deserialize(Obj *obj, InStream *instream) {
    if (Obj_Is_A(obj, STRING)) {
        obj = (Obj*)Freezer_deserialize_string((String*)obj, instream);
    }
    else if (Obj_Is_A(obj, BYTEBUF)) {
        obj = (Obj*)Freezer_deserialize_bytebuf((ByteBuf*)obj, instream);
    }
    else if (Obj_Is_A(obj, VARRAY)) {
        obj = (Obj*)Freezer_deserialize_varray((VArray*)obj, instream);
    }
    else if (Obj_Is_A(obj, HASH)) {
        obj = (Obj*)Freezer_deserialize_hash((Hash*)obj, instream);
    }
    else if (Obj_Is_A(obj, NUM)) {
        if (Obj_Is_A(obj, INTNUM)) {
            if (Obj_Is_A(obj, BOOLNUM)) {
                bool value = !!InStream_Read_U8(instream);
                BoolNum *self = (BoolNum*)obj;
                if (self && self != CFISH_TRUE && self != CFISH_FALSE) {
                    Bool_Dec_RefCount_t super_decref
                        = SUPER_METHOD_PTR(BOOLNUM, CFISH_Bool_Dec_RefCount);
                    super_decref(self);
                }
                obj = value ? (Obj*)CFISH_TRUE : (Obj*)CFISH_FALSE;
            }
            else if (Obj_Is_A(obj, INTEGER32)) {
                int32_t value = (int32_t)InStream_Read_C32(instream);
                obj = (Obj*)Int32_init((Integer32*)obj, value);
            }
            else if (Obj_Is_A(obj, INTEGER64)) {
                int64_t value = (int64_t)InStream_Read_C64(instream);
                obj = (Obj*)Int64_init((Integer64*)obj, value);
            }
        }
        else if (Obj_Is_A(obj, FLOATNUM)) {
            if (Obj_Is_A(obj, FLOAT32)) {
                float value = InStream_Read_F32(instream);
                obj = (Obj*)Float32_init((Float32*)obj, value);
            }
            else if (Obj_Is_A(obj, FLOAT64)) {
                double value = InStream_Read_F64(instream);
                obj = (Obj*)Float64_init((Float64*)obj, value);
            }
        }
    }
    else if (Obj_Is_A(obj, QUERY)) {
        obj = (Obj*)Query_Deserialize((Query*)obj, instream);
    }
    else if (Obj_Is_A(obj, DOC)) {
        obj = (Obj*)Doc_Deserialize((Doc*)obj, instream);
    }
    else if (Obj_Is_A(obj, DOCVECTOR)) {
        obj = (Obj*)DocVec_Deserialize((DocVector*)obj, instream);
    }
    else if (Obj_Is_A(obj, TERMVECTOR)) {
        obj = (Obj*)TV_Deserialize((TermVector*)obj, instream);
    }
    else if (Obj_Is_A(obj, SIMILARITY)) {
        obj = (Obj*)Sim_Deserialize((Similarity*)obj, instream);
    }
    else if (Obj_Is_A(obj, MATCHDOC)) {
        obj = (Obj*)MatchDoc_Deserialize((MatchDoc*)obj, instream);
    }
    else if (Obj_Is_A(obj, TOPDOCS)) {
        obj = (Obj*)TopDocs_Deserialize((TopDocs*)obj, instream);
    }
    else if (Obj_Is_A(obj, SORTSPEC)) {
        obj = (Obj*)SortSpec_Deserialize((SortSpec*)obj, instream);
    }
    else if (Obj_Is_A(obj, SORTRULE)) {
        obj = (Obj*)SortRule_Deserialize((SortRule*)obj, instream);
    }
    else {
        THROW(ERR, "Don't know how to deserialize a %o",
              Obj_Get_Class_Name(obj));
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
Freezer_serialize_bytebuf(ByteBuf *bytebuf, OutStream *outstream) {
    size_t size = BB_Get_Size(bytebuf);
    OutStream_Write_C32(outstream, size);
    OutStream_Write_Bytes(outstream, BB_Get_Buf(bytebuf), size);
}

ByteBuf*
Freezer_deserialize_bytebuf(ByteBuf *bytebuf, InStream *instream) {
    size_t size = InStream_Read_C32(instream);
    char   *buf = (char*)MALLOCATE(size);
    InStream_Read_Bytes(instream, buf, size);
    return BB_init_steal_bytes(bytebuf, buf, size, size);
}

ByteBuf*
Freezer_read_bytebuf(InStream *instream) {
    ByteBuf *bytebuf = (ByteBuf*)Class_Make_Obj(BYTEBUF);
    return Freezer_deserialize_bytebuf(bytebuf, instream);
}

void
Freezer_serialize_varray(VArray *array, OutStream *outstream) {
    uint32_t last_valid_tick = 0;
    size_t size = VA_Get_Size(array);
    OutStream_Write_C32(outstream, size);
    for (uint32_t i = 0; i < size; i++) {
        Obj *elem = VA_Fetch(array, i);
        if (elem) {
            OutStream_Write_C32(outstream, i - last_valid_tick);
            FREEZE(elem, outstream);
            last_valid_tick = i;
        }
    }
    // Terminate.
    OutStream_Write_C32(outstream, size - last_valid_tick);
}

VArray*
Freezer_deserialize_varray(VArray *array, InStream *instream) {
    uint32_t size = InStream_Read_C32(instream);
    VA_init(array, size);
    for (uint32_t tick = InStream_Read_C32(instream);
         tick < size;
         tick += InStream_Read_C32(instream)
        ) {
        Obj *obj = THAW(instream);
        VA_Store(array, tick, obj);
    }
    VA_Resize(array, size);
    return array;
}

VArray*
Freezer_read_varray(InStream *instream) {
    VArray *array = (VArray*)Class_Make_Obj(VARRAY);
    return Freezer_deserialize_varray(array, instream);
}

void
Freezer_serialize_hash(Hash *hash, OutStream *outstream) {
    Obj *key;
    Obj *val;
    uint32_t string_count = 0;
    uint32_t hash_size = Hash_Get_Size(hash);
    OutStream_Write_C32(outstream, hash_size);

    // Write String keys first.  String keys are the common case; grouping
    // them together is a form of run-length-encoding and saves space, since
    // we omit the per-key class name.
    Hash_Iterate(hash);
    while (Hash_Next(hash, &key, &val)) {
        if (Obj_Is_A(key, STRING)) { string_count++; }
    }
    OutStream_Write_C32(outstream, string_count);
    Hash_Iterate(hash);
    while (Hash_Next(hash, &key, &val)) {
        if (Obj_Is_A(key, STRING)) {
            Freezer_serialize_string((String*)key, outstream);
            FREEZE(val, outstream);
        }
    }

    // Punt on the classes of the remaining keys.
    Hash_Iterate(hash);
    while (Hash_Next(hash, &key, &val)) {
        if (!Obj_Is_A(key, STRING)) {
            FREEZE(key, outstream);
            FREEZE(val, outstream);
        }
    }
}

Hash*
Freezer_deserialize_hash(Hash *hash, InStream *instream) {
    uint32_t size        = InStream_Read_C32(instream);
    uint32_t num_strings = InStream_Read_C32(instream);
    uint32_t num_other   = size - num_strings;

    Hash_init(hash, size);

    // Read key-value pairs with String keys.
    while (num_strings--) {
        uint32_t len = InStream_Read_C32(instream);
        char *key_buf = (char*)MALLOCATE(len + 1);
        InStream_Read_Bytes(instream, key_buf, len);
        key_buf[len] = '\0';
        String *key = Str_new_steal_utf8(key_buf, len);
        Hash_Store(hash, (Obj*)key, THAW(instream));
        DECREF(key);
    }

    // Read remaining key/value pairs.
    while (num_other--) {
        Obj *k = THAW(instream);
        Hash_Store(hash, k, THAW(instream));
        DECREF(k);
    }

    return hash;
}

Hash*
Freezer_read_hash(InStream *instream) {
    Hash *hash = (Hash*)Class_Make_Obj(HASH);
    return Freezer_deserialize_hash(hash, instream);
}

static Obj*
S_dump_array(VArray *array) {
    VArray *dump = VA_new(VA_Get_Size(array));
    for (uint32_t i = 0, max = VA_Get_Size(array); i < max; i++) {
        Obj *elem = VA_Fetch(array, i);
        if (elem) {
            VA_Store(dump, i, Freezer_dump(elem));
        }
    }
    return (Obj*)dump;
}

Obj*
S_dump_hash(Hash *hash) {
    Hash *dump = Hash_new(Hash_Get_Size(hash));
    Obj *key;
    Obj *value;

    Hash_Iterate(hash);
    while (Hash_Next(hash, &key, &value)) {
        // Since JSON only supports text hash keys, dump() can only support
        // text hash keys.
        CERTIFY(key, STRING);
        Hash_Store(dump, key, Freezer_dump(value));
    }

    return (Obj*)dump;
}

Obj*
Freezer_dump(Obj *obj) {
    if (Obj_Is_A(obj, STRING)) {
        return (Obj*)Obj_To_String(obj);
    }
    else if (Obj_Is_A(obj, VARRAY)) {
        return S_dump_array((VArray*)obj);
    }
    else if (Obj_Is_A(obj, HASH)) {
        return S_dump_hash((Hash*)obj);
    }
    else if (Obj_Is_A(obj, ANALYZER)) {
        return Analyzer_Dump((Analyzer*)obj);
    }
    else if (Obj_Is_A(obj, DOC)) {
        return (Obj*)Doc_Dump((Doc*)obj);
    }
    else if (Obj_Is_A(obj, SIMILARITY)) {
        return Sim_Dump((Similarity*)obj);
    }
    else if (Obj_Is_A(obj, FIELDTYPE)) {
        return FType_Dump((FieldType*)obj);
    }
    else if (Obj_Is_A(obj, SCHEMA)) {
        return (Obj*)Schema_Dump((Schema*)obj);
    }
    else if (Obj_Is_A(obj, QUERY)) {
        return Query_Dump((Query*)obj);
    }
    else {
        return (Obj*)Obj_To_String(obj);
    }
}

static Obj*
S_load_via_load_method(Class *klass, Obj *dump) {
    Obj *dummy = Class_Make_Obj(klass);
    Obj *loaded = NULL;
    if (Obj_Is_A(dummy, ANALYZER)) {
        loaded = Analyzer_Load((Analyzer*)dummy, dump);
    }
    else if (Obj_Is_A(dummy, DOC)) {
        loaded = (Obj*)Doc_Load((Doc*)dummy, dump);
    }
    else if (Obj_Is_A(dummy, SIMILARITY)) {
        loaded = (Obj*)Sim_Load((Similarity*)dummy, dump);
    }
    else if (Obj_Is_A(dummy, FIELDTYPE)) {
        loaded = FType_Load((FieldType*)dummy, dump);
    }
    else if (Obj_Is_A(dummy, SCHEMA)) {
        loaded = (Obj*)Schema_Load((Schema*)dummy, dump);
    }
    else if (Obj_Is_A(dummy, QUERY)) {
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
    if (class_name && Str_Is_A(class_name, STRING)) {
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
    Obj *key;
    Obj *value;
    Hash_Iterate(dump);
    while (Hash_Next(dump, &key, &value)) {
        Hash_Store(loaded, key, Freezer_load(value));
    }

    return (Obj*)loaded;

}


Obj*
S_load_from_array(VArray *dump) {
    VArray *loaded = VA_new(VA_Get_Size(dump));

    for (uint32_t i = 0, max = VA_Get_Size(dump); i < max; i++) {
        Obj *elem_dump = VA_Fetch(dump, i);
        if (elem_dump) {
            VA_Store(loaded, i, Freezer_load(elem_dump));
        }
    }

    return (Obj*)loaded;
}

Obj*
Freezer_load(Obj *obj) {
    if (Obj_Is_A(obj, HASH)) {
        return S_load_from_hash((Hash*)obj);
    }
    else if (Obj_Is_A(obj, VARRAY)) {
        return S_load_from_array((VArray*)obj);
    }
    else {
        return Obj_Clone(obj);
    }
}

