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
#include "Lucy/Document/Doc.h"
#include "Lucy/Index/Similarity.h"
#include "Lucy/Index/DocVector.h"
#include "Lucy/Index/TermVector.h"
#include "Lucy/Search/Query.h"
#include "Lucy/Search/SortRule.h"
#include "Lucy/Search/SortSpec.h"
#include "Lucy/Search/MatchDoc.h"
#include "Lucy/Search/TopDocs.h"

void
Freezer_freeze(Obj *obj, OutStream *outstream) {
    Freezer_serialize_charbuf(Obj_Get_Class_Name(obj), outstream);
    Freezer_serialize(obj, outstream);
}

Obj*
Freezer_thaw(InStream *instream) {
    CharBuf *class_name = Freezer_read_charbuf(instream);
    VTable *vtable = VTable_singleton(class_name, NULL);
    Obj *blank = VTable_Make_Obj(vtable);
    DECREF(class_name);
    return Freezer_deserialize(blank, instream);
}

void
Freezer_serialize(Obj *obj, OutStream *outstream) {
    VTable *vtable = Obj_Get_VTable(obj);
    if (Obj_Is_A(obj, CHARBUF)) {
        Freezer_serialize_charbuf((CharBuf*)obj, outstream);
    }
    else if (Obj_Is_A(obj, BYTEBUF)) {
        BB_serialize((ByteBuf*)obj, outstream);
    }
    else if (Obj_Is_A(obj, VARRAY)) {
        VA_serialize((VArray*)obj, outstream);
    }
    else if (Obj_Is_A(obj, HASH)) {
        Hash_serialize((Hash*)obj, outstream);
    }
    else if (Obj_Is_A(obj, NUM)) {
        if (Obj_Is_A(obj, INTNUM)) {
            if (Obj_Is_A(obj, BOOLNUM)) {
                Bool_serialize((BoolNum*)obj, outstream);
            }
            else if (Obj_Is_A(obj, INTEGER32)) {
                Int32_serialize((Integer32*)obj, outstream);
            }
            else if (Obj_Is_A(obj, INTEGER64)) {
                Int64_serialize((Integer64*)obj, outstream);
            }
        }
        else if (Obj_Is_A(obj, FLOATNUM)) {
            if (Obj_Is_A(obj, FLOAT32)) {
                Float32_serialize((Float32*)obj, outstream);
            }
            else if (Obj_Is_A(obj, FLOAT64)) {
                Float64_serialize((Float64*)obj, outstream);
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
    VTable *vtable = Obj_Get_VTable(obj);
    if (Obj_Is_A(obj, CHARBUF)) {
        obj = (Obj*)Freezer_deserialize_charbuf((CharBuf*)obj, instream);
    }
    else if (Obj_Is_A(obj, BYTEBUF)) {
        obj = (Obj*)BB_deserialize((ByteBuf*)obj, instream);
    }
    else if (Obj_Is_A(obj, VARRAY)) {
        obj = (Obj*)VA_deserialize((VArray*)obj, instream);
    }
    else if (Obj_Is_A(obj, HASH)) {
        obj = (Obj*)Hash_deserialize((Hash*)obj, instream);
    }
    else if (Obj_Is_A(obj, NUM)) {
        if (Obj_Is_A(obj, INTNUM)) {
            if (Obj_Is_A(obj, BOOLNUM)) {
                obj = (Obj*)Bool_deserialize((BoolNum*)obj, instream);
            }
            else if (Obj_Is_A(obj, INTEGER32)) {
                obj = (Obj*)Int32_deserialize((Integer32*)obj, instream);
            }
            else if (Obj_Is_A(obj, INTEGER64)) {
                obj = (Obj*)Int64_deserialize((Integer64*)obj, instream);
            }
        }
        else if (Obj_Is_A(obj, FLOATNUM)) {
            if (Obj_Is_A(obj, FLOAT32)) {
                obj = (Obj*)Float32_deserialize((Float32*)obj, instream);
            }
            else if (Obj_Is_A(obj, FLOAT64)) {
                obj = (Obj*)Float64_deserialize((Float64*)obj, instream);
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
Freezer_serialize_charbuf(CharBuf *charbuf, OutStream *outstream) {
    size_t size  = CB_Get_Size(charbuf);
    uint8_t *buf = CB_Get_Ptr8(charbuf);
    OutStream_Write_C64(outstream, size);
    OutStream_Write_Bytes(outstream, buf, size);
}

CharBuf*
Freezer_deserialize_charbuf(CharBuf *charbuf, InStream *instream) {
    size_t size = InStream_Read_C32(instream);
    if (size == SIZE_MAX) {
        THROW(ERR, "Can't deserialize SIZE_MAX bytes");
    }
    size_t cap = Memory_oversize(size + 1, sizeof(char));
    char *buf = MALLOCATE(cap);
    InStream_Read_Bytes(instream, buf, size);
    buf[size] = '\0';
    if (!StrHelp_utf8_valid(buf, size)) {
        THROW(ERR, "Attempt to deserialize invalid UTF-8");
    }
    return CB_init_steal_trusted_str(charbuf, buf, size, cap);
}

CharBuf*
Freezer_read_charbuf(InStream *instream) {
    CharBuf *charbuf = (CharBuf*)VTable_Make_Obj(CHARBUF);
    return Freezer_deserialize_charbuf(charbuf, instream);
}


