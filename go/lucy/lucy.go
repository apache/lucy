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

package lucy

/*

#include <stdlib.h>

#define C_LUCY_DOC
#define C_LUCY_REGEXTOKENIZER
#define C_LUCY_DEFAULTDOCREADER
#define C_LUCY_INVERTER
#define C_LUCY_INVERTERENTRY

#include "lucy_parcel.h"
#include "Lucy/Analysis/RegexTokenizer.h"
#include "Lucy/Document/Doc.h"
#include "Lucy/Index/DocReader.h"
#include "Lucy/Index/Inverter.h"

#include "Clownfish/String.h"
#include "Clownfish/Blob.h"
#include "Clownfish/Num.h"
#include "Clownfish/Hash.h"
#include "Clownfish/HashIterator.h"
#include "Clownfish/Vector.h"
#include "Clownfish/Err.h"
#include "Clownfish/Util/StringHelper.h"
#include "Lucy/Analysis/Analyzer.h"
#include "Lucy/Analysis/Inversion.h"
#include "Lucy/Analysis/Token.h"
#include "Lucy/Document/HitDoc.h"
#include "Lucy/Plan/FieldType.h"
#include "Lucy/Plan/Schema.h"
#include "Lucy/Index/Segment.h"
#include "Lucy/Store/InStream.h"
#include "Lucy/Store/OutStream.h"
#include "Lucy/Util/Freezer.h"

extern lucy_RegexTokenizer*
GOLUCY_RegexTokenizer_init(lucy_RegexTokenizer *self, cfish_String *pattern);
extern lucy_RegexTokenizer*
(*GOLUCY_RegexTokenizer_init_BRIDGE)(lucy_RegexTokenizer *self,
									 cfish_String *pattern);
extern void
GOLUCY_RegexTokenizer_Destroy(lucy_RegexTokenizer *self);
extern void
(*GOLUCY_RegexTokenizer_Destroy_BRIDGE)(lucy_RegexTokenizer *self);
extern void
GOLUCY_RegexTokenizer_Tokenize_Utf8(lucy_RegexTokenizer *self, char *str,
									size_t string_len, lucy_Inversion *inversion);
extern void
(*GOLUCY_RegexTokenizer_Tokenize_Utf8_BRIDGE)(lucy_RegexTokenizer *self, const char *str,
											  size_t string_len, lucy_Inversion *inversion);

extern lucy_Doc*
GOLUCY_Doc_init(lucy_Doc *doc, void *fields, int32_t doc_id);
extern lucy_Doc*
(*GOLUCY_Doc_init_BRIDGE)(lucy_Doc *doc, void *fields, int32_t doc_id);
extern void
GOLUCY_Doc_Set_Fields(lucy_Doc *self, void *fields);
extern void
(*GOLUCY_Doc_Set_Fields_BRIDGE)(lucy_Doc *self, void *fields);
extern uint32_t
GOLUCY_Doc_Get_Size(lucy_Doc *self);
extern uint32_t
(*GOLUCY_Doc_Get_Size_BRIDGE)(lucy_Doc *self);
extern void
GOLUCY_Doc_Store(lucy_Doc *self, cfish_String *field, cfish_Obj *value);
extern void
(*GOLUCY_Doc_Store_BRIDGE)(lucy_Doc *self, cfish_String *field, cfish_Obj *value);
extern void
GOLUCY_Doc_Serialize(lucy_Doc *self, lucy_OutStream *outstream);
extern void
(*GOLUCY_Doc_Serialize_BRIDGE)(lucy_Doc *self, lucy_OutStream *outstream);
extern lucy_Doc*
GOLUCY_Doc_Deserialize(lucy_Doc *self, lucy_InStream *instream);
extern lucy_Doc*
(*GOLUCY_Doc_Deserialize_BRIDGE)(lucy_Doc *self, lucy_InStream *instream);
extern cfish_Obj*
GOLUCY_Doc_Extract(lucy_Doc *self, cfish_String *field);
extern cfish_Obj*
(*GOLUCY_Doc_Extract_BRIDGE)(lucy_Doc *self, cfish_String *field);
extern cfish_Vector*
GOLUCY_Doc_Field_Names(lucy_Doc *self);
extern cfish_Vector*
(*GOLUCY_Doc_Field_Names_BRIDGE)(lucy_Doc *self);
extern bool
GOLUCY_Doc_Equals(lucy_Doc *self, cfish_Obj *other);
extern bool
(*GOLUCY_Doc_Equals_BRIDGE)(lucy_Doc *self, cfish_Obj *other);
extern void
GOLUCY_Doc_Destroy(lucy_Doc *self);
extern void
(*GOLUCY_Doc_Destroy_BRIDGE)(lucy_Doc *self);

extern lucy_HitDoc*
GOLUCY_DefDocReader_Fetch_Doc(lucy_DefaultDocReader *self, int32_t doc_id);
extern lucy_HitDoc*
(*GOLUCY_DefDocReader_Fetch_Doc_BRIDGE)(lucy_DefaultDocReader *self, int32_t doc_id);

extern void
GOLUCY_Inverter_Invert_Doc(lucy_Inverter *self, lucy_Doc *doc);
extern void
(*GOLUCY_Inverter_Invert_Doc_BRIDGE)(lucy_Inverter *self, lucy_Doc *doc);


// C symbols linked into a Go-built package archive are not visible to
// external C code -- but internal code *can* see symbols from outside.
// This allows us to fake up symbol export by assigning values only known
// interally to external symbols during Go package initialization.
static CFISH_INLINE void
GOLUCY_glue_exported_symbols() {
	GOLUCY_RegexTokenizer_init_BRIDGE = GOLUCY_RegexTokenizer_init;
	GOLUCY_RegexTokenizer_Destroy_BRIDGE = GOLUCY_RegexTokenizer_Destroy;
	GOLUCY_RegexTokenizer_Tokenize_Utf8_BRIDGE
		= (LUCY_RegexTokenizer_Tokenize_Utf8_t)GOLUCY_RegexTokenizer_Tokenize_Utf8;
	GOLUCY_Doc_init_BRIDGE = GOLUCY_Doc_init;
	GOLUCY_Doc_Set_Fields_BRIDGE = GOLUCY_Doc_Set_Fields;
	GOLUCY_Doc_Get_Size_BRIDGE = GOLUCY_Doc_Get_Size;
	GOLUCY_Doc_Store_BRIDGE = GOLUCY_Doc_Store;
	GOLUCY_Doc_Serialize_BRIDGE = GOLUCY_Doc_Serialize;
	GOLUCY_Doc_Deserialize_BRIDGE = GOLUCY_Doc_Deserialize;
	GOLUCY_Doc_Extract_BRIDGE = GOLUCY_Doc_Extract;
	GOLUCY_Doc_Field_Names_BRIDGE = GOLUCY_Doc_Field_Names;
	GOLUCY_Doc_Equals_BRIDGE = GOLUCY_Doc_Equals;
	GOLUCY_Doc_Destroy_BRIDGE = GOLUCY_Doc_Destroy;
	GOLUCY_DefDocReader_Fetch_Doc_BRIDGE = GOLUCY_DefDocReader_Fetch_Doc;
	GOLUCY_Inverter_Invert_Doc_BRIDGE = GOLUCY_Inverter_Invert_Doc;
}

static uint32_t
S_count_code_points(const char *string, size_t len) {
    uint32_t num_code_points = 0;
    size_t i = 0;

    while (i < len) {
        i += cfish_StrHelp_UTF8_COUNT[(uint8_t)(string[i])];
        ++num_code_points;
    }

    if (i != len) {
        CFISH_THROW(CFISH_ERR, "Match between code point boundaries in '%s'", string);
    }

    return num_code_points;
}

// Returns the number of code points through the end of the match.
static int
push_token(const char *str, int start, int end, int last_end,
           int cp_count, lucy_Inversion *inversion) {
	const char *match = str + start;
	int match_len = end - start;
	int cp_start = cp_count + S_count_code_points(str + last_end, start - last_end);
	int cp_end   = cp_start + S_count_code_points(match, match_len);
	lucy_Token *token = lucy_Token_new(match, match_len, cp_start, cp_end, 1.0f, 1);
	LUCY_Inversion_Append(inversion, token);
	return cp_end;
}

static void
null_terminate_string(char *string, size_t len) {
	string[len] = '\0';
}

*/
import "C"
import "unsafe"
import "fmt"
import "regexp"
import "git-wip-us.apache.org/repos/asf/lucy-clownfish.git/runtime/go/clownfish"

var registry *objRegistry

func init() {
	C.GOLUCY_glue_exported_symbols()
	C.lucy_bootstrap_parcel()
	registry = newObjRegistry(16)
	initWRAP()
}

//export GOLUCY_RegexTokenizer_init
func GOLUCY_RegexTokenizer_init(rt *C.lucy_RegexTokenizer, pattern *C.cfish_String) *C.lucy_RegexTokenizer {
	C.lucy_Analyzer_init(((*C.lucy_Analyzer)(unsafe.Pointer(rt))))

	ivars := C.lucy_RegexTokenizer_IVARS(rt)
	ivars.pattern = C.CFISH_Str_Clone(pattern)

	var patternGo string
	if pattern == nil {
		patternGo = "\\w+(?:['\\x{2019}]\\w+)*"
	} else {
		patternGo = clownfish.CFStringToGo(unsafe.Pointer(pattern))
	}
	rx, err := regexp.Compile(patternGo)
	if err != nil {
		panic(err)
	}
	rxID := registry.store(rx)
	ivars.token_re = unsafe.Pointer(rxID)

	return rt
}

//export GOLUCY_RegexTokenizer_Destroy
func GOLUCY_RegexTokenizer_Destroy(rt *C.lucy_RegexTokenizer) {
	ivars := C.lucy_RegexTokenizer_IVARS(rt)
	rxID := uintptr(ivars.token_re)
	registry.delete(rxID)
	C.cfish_super_destroy(unsafe.Pointer(rt), C.LUCY_REGEXTOKENIZER)
}

//export GOLUCY_RegexTokenizer_Tokenize_Utf8
func GOLUCY_RegexTokenizer_Tokenize_Utf8(rt *C.lucy_RegexTokenizer, str *C.char,
	stringLen C.size_t, inversion *C.lucy_Inversion) {

	ivars := C.lucy_RegexTokenizer_IVARS(rt)
	rxID := uintptr(ivars.token_re)
	rx, ok := registry.fetch(rxID).(*regexp.Regexp)
	if !ok {
		mess := fmt.Sprintf("Failed to Fetch *RegExp with id %d and pattern %s",
			rxID, clownfish.CFStringToGo(unsafe.Pointer(ivars.pattern)))
		panic(clownfish.NewErr(mess))
	}

	buf := C.GoBytes(unsafe.Pointer(str), C.int(stringLen))
	found := rx.FindAllIndex(buf, int(stringLen))
	lastEnd := 0
	cpCount := 0
	for _, startEnd := range found {
		cpCount = int(C.push_token(str, C.int(startEnd[0]), C.int(startEnd[1]),
			C.int(lastEnd), C.int(cpCount), inversion))
		lastEnd = startEnd[1]
	}
}

//export GOLUCY_Doc_init
func GOLUCY_Doc_init(d *C.lucy_Doc, fields unsafe.Pointer, docID C.int32_t) *C.lucy_Doc {
	ivars := C.lucy_Doc_IVARS(d)
	if fields != nil {
		ivars.fields = unsafe.Pointer(C.cfish_inc_refcount(fields))
	} else {
		ivars.fields = unsafe.Pointer(C.cfish_Hash_new(0))
	}
	ivars.doc_id = docID
	return d
}

//export GOLUCY_Doc_Set_Fields
func GOLUCY_Doc_Set_Fields(d *C.lucy_Doc, fields unsafe.Pointer) {
	ivars := C.lucy_Doc_IVARS(d)
	temp := ivars.fields
	ivars.fields = unsafe.Pointer(C.cfish_inc_refcount(fields))
	C.cfish_decref(temp)
}

//export GOLUCY_Doc_Get_Size
func GOLUCY_Doc_Get_Size(d *C.lucy_Doc) C.uint32_t {
	ivars := C.lucy_Doc_IVARS(d)
	hash := ((*C.cfish_Hash)(ivars.fields))
	return C.uint32_t(C.CFISH_Hash_Get_Size(hash))
}

//export GOLUCY_Doc_Store
func GOLUCY_Doc_Store(d *C.lucy_Doc, field *C.cfish_String, value *C.cfish_Obj) {
	ivars := C.lucy_Doc_IVARS(d)
	hash := (*C.cfish_Hash)(ivars.fields)
	C.CFISH_Hash_Store(hash, field, C.cfish_inc_refcount(unsafe.Pointer(value)))
}

//export GOLUCY_Doc_Serialize
func GOLUCY_Doc_Serialize(d *C.lucy_Doc, outstream *C.lucy_OutStream) {
	ivars := C.lucy_Doc_IVARS(d)
	hash := (*C.cfish_Hash)(ivars.fields)
	C.lucy_Freezer_serialize_hash(hash, outstream)
	C.LUCY_OutStream_Write_C32(outstream, C.uint32_t(ivars.doc_id))
}

//export GOLUCY_Doc_Deserialize
func GOLUCY_Doc_Deserialize(d *C.lucy_Doc, instream *C.lucy_InStream) *C.lucy_Doc {
	ivars := C.lucy_Doc_IVARS(d)
	ivars.fields = unsafe.Pointer(C.lucy_Freezer_read_hash(instream))
	ivars.doc_id = C.int32_t(C.LUCY_InStream_Read_C32(instream))
	return d
}

//export GOLUCY_Doc_Extract
func GOLUCY_Doc_Extract(d *C.lucy_Doc, field *C.cfish_String) *C.cfish_Obj {
	ivars := C.lucy_Doc_IVARS(d)
	hash := (*C.cfish_Hash)(ivars.fields)
	val := C.CFISH_Hash_Fetch(hash, field)
	return C.cfish_inc_refcount(unsafe.Pointer(val))
}

//export GOLUCY_Doc_Field_Names
func GOLUCY_Doc_Field_Names(d *C.lucy_Doc) *C.cfish_Vector {
	ivars := C.lucy_Doc_IVARS(d)
	hash := (*C.cfish_Hash)(ivars.fields)
	return C.CFISH_Hash_Keys(hash)
}

//export GOLUCY_Doc_Equals
func GOLUCY_Doc_Equals(d *C.lucy_Doc, other *C.cfish_Obj) C.bool {
	twin := (*C.lucy_Doc)(unsafe.Pointer(other))
	if twin == d {
		return true
	}
	if !C.cfish_Obj_is_a(other, C.LUCY_DOC) {
		return false
	}
	ivars := C.lucy_Doc_IVARS(d)
	ovars := C.lucy_Doc_IVARS(twin)
	hash := (*C.cfish_Hash)(ivars.fields)
	otherHash := (*C.cfish_Obj)(ovars.fields)
	return C.CFISH_Hash_Equals(hash, otherHash)
}

//export GOLUCY_Doc_Destroy
func GOLUCY_Doc_Destroy(d *C.lucy_Doc) {
	ivars := C.lucy_Doc_IVARS(d)
	C.cfish_decref(unsafe.Pointer(ivars.fields))
	C.cfish_super_destroy(unsafe.Pointer(d), C.LUCY_DOC)
}

func fetchEntry(ivars *C.lucy_InverterIVARS, field *C.cfish_String) *C.lucy_InverterEntry {
	schema := ivars.schema
	fieldNum := C.LUCY_Seg_Field_Num(ivars.segment, field)
	if fieldNum == 0 {
		// This field seems not to be in the segment yet.  Try to find it in
		// the Schema.
		if C.LUCY_Schema_Fetch_Type(schema, field) != nil {
			// The field is in the Schema.  Get a field num from the Segment.
			fieldNum = C.LUCY_Seg_Add_Field(ivars.segment, field)
		} else {
			// We've truly failed to find the field.  The user must
			// not have spec'd it.
			fieldGo := clownfish.CFStringToGo(unsafe.Pointer(field))
			err := clownfish.NewErr("Unknown field name: '" + fieldGo + "'")
			panic(err)
		}
	}
	entry := C.CFISH_Vec_Fetch(ivars.entry_pool, C.size_t(fieldNum))
	if entry == nil {
		newEntry := C.lucy_InvEntry_new(schema, field, fieldNum)
		C.CFISH_Vec_Store(ivars.entry_pool, C.size_t(fieldNum),
			(*C.cfish_Obj)(unsafe.Pointer(entry)))
		return newEntry
	}
	return (*C.lucy_InverterEntry)(unsafe.Pointer(entry))
}

//export GOLUCY_DefDocReader_Fetch_Doc
func GOLUCY_DefDocReader_Fetch_Doc(ddr *C.lucy_DefaultDocReader,
	docID C.int32_t) *C.lucy_HitDoc {
	ivars := C.lucy_DefDocReader_IVARS(ddr)
	schema := ivars.schema
	datInstream := ivars.dat_in
	ixInstream := ivars.ix_in
	fields := C.cfish_Hash_new(1)
	fieldNameCap := C.size_t(31)
	var fieldName *C.char = ((*C.char)(C.malloc(fieldNameCap + 1)))

	// Get data file pointer from index, read number of fields.
	C.LUCY_InStream_Seek(ixInstream, C.int64_t(docID*8))
	start := C.LUCY_InStream_Read_U64(ixInstream)
	C.LUCY_InStream_Seek(datInstream, C.int64_t(start))
	numFields := uint32(C.LUCY_InStream_Read_C32(datInstream))

	// Decode stored data and build up the doc field by field.
	for i := uint32(0); i < numFields; i++ {
		// Read field name.
		fieldNameLen := C.size_t(C.LUCY_InStream_Read_C32(datInstream))
		if fieldNameLen > fieldNameCap {
			fieldNameCap = fieldNameLen
			fieldName = ((*C.char)(C.realloc(unsafe.Pointer(fieldName), fieldNameCap+1)))
		}
		C.LUCY_InStream_Read_Bytes(datInstream, fieldName, fieldNameLen)

		// Find the Field's FieldType.
		// TODO: Creating and destroying a new string each time is
		// inefficient.  The solution should be to add a privte
		// Schema_Fetch_Type_Utf8 method which takes char* and size_t.
		fieldNameStr := C.cfish_Str_new_from_utf8(fieldName, fieldNameLen)
		fieldType := C.LUCY_Schema_Fetch_Type(schema, fieldNameStr)
		C.cfish_dec_refcount(unsafe.Pointer(fieldNameStr))

		// Read the field value.
		var value *C.cfish_Obj
		switch C.LUCY_FType_Primitive_ID(fieldType) & C.lucy_FType_PRIMITIVE_ID_MASK {
		case C.lucy_FType_TEXT:
			valueLen := C.size_t(C.LUCY_InStream_Read_C32(datInstream))
			buf := ((*C.char)(C.malloc(valueLen + 1)))
			C.LUCY_InStream_Read_Bytes(datInstream, buf, valueLen)
			C.null_terminate_string(buf, valueLen)
			value = ((*C.cfish_Obj)(C.cfish_Str_new_steal_utf8(buf, valueLen)))
		case C.lucy_FType_BLOB:
			valueLen := C.size_t(C.LUCY_InStream_Read_C32(datInstream))
			buf := ((*C.char)(C.malloc(valueLen)))
			C.LUCY_InStream_Read_Bytes(datInstream, buf, valueLen)
			value = ((*C.cfish_Obj)(C.cfish_Blob_new_steal(buf, valueLen)))
		case C.lucy_FType_FLOAT32:
			value = ((*C.cfish_Obj)(C.cfish_Float_new(C.double(C.LUCY_InStream_Read_F32(datInstream)))))
		case C.lucy_FType_FLOAT64:
			value = ((*C.cfish_Obj)(C.cfish_Float_new(C.LUCY_InStream_Read_F64(datInstream))))
		case C.lucy_FType_INT32:
			value = ((*C.cfish_Obj)(C.cfish_Int_new(C.int64_t(C.LUCY_InStream_Read_C32(datInstream)))))
		case C.lucy_FType_INT64:
			value = ((*C.cfish_Obj)(C.cfish_Int_new(C.int64_t(C.LUCY_InStream_Read_C64(datInstream)))))
		default:
			value = nil
			panic(clownfish.NewErr("Internal Lucy error: bad type id for field " +
				C.GoStringN(fieldName, C.int(fieldNameLen))))
		}

		// Store the value.
		C.CFISH_Hash_Store_Utf8(fields, fieldName, fieldNameLen, value)
	}
	C.free(unsafe.Pointer(fieldName))

	retval := C.lucy_HitDoc_new(unsafe.Pointer(fields), docID, 0.0)
	C.cfish_dec_refcount(unsafe.Pointer(fields))
	return retval
}

//export GOLUCY_Inverter_Invert_Doc
func GOLUCY_Inverter_Invert_Doc(inverter *C.lucy_Inverter, doc *C.lucy_Doc) {
	ivars := C.lucy_Inverter_IVARS(inverter)
	fields := (*C.cfish_Hash)(C.LUCY_Doc_Get_Fields(doc))

	// Prepare for the new doc.
	C.LUCY_Inverter_Set_Doc(inverter, doc)

	// Extract and invert the doc's fields.
	iter := C.cfish_HashIter_new(fields)
	for C.CFISH_HashIter_Next(iter) {
		field := C.CFISH_HashIter_Get_Key(iter)
		obj := C.CFISH_HashIter_Get_Value(iter)
		if obj == nil {
			mess := "Invalid nil value for field" + clownfish.CFStringToGo(unsafe.Pointer(field))
			panic(clownfish.NewErr(mess))
		}

		inventry := fetchEntry(ivars, field)
		inventryIvars := C.lucy_InvEntry_IVARS(inventry)
		fieldType := inventryIvars._type

		// Get the field value.
		var expectedType *C.cfish_Class
		switch C.LUCY_FType_Primitive_ID(fieldType) & C.lucy_FType_PRIMITIVE_ID_MASK {
		case C.lucy_FType_TEXT:
			expectedType = C.CFISH_STRING
		case C.lucy_FType_BLOB:
			expectedType = C.CFISH_BLOB
		case C.lucy_FType_INT32:
			expectedType = C.CFISH_INTEGER
		case C.lucy_FType_INT64:
			expectedType = C.CFISH_INTEGER
		case C.lucy_FType_FLOAT32:
			expectedType = C.CFISH_FLOAT
		case C.lucy_FType_FLOAT64:
			expectedType = C.CFISH_FLOAT
		default:
			panic(clownfish.NewErr("Internal Lucy error: bad type id for field " +
				clownfish.CFStringToGo(unsafe.Pointer(field))))
		}
		if !C.cfish_Obj_is_a(obj, expectedType) {
			className := C.cfish_Obj_get_class_name((*C.cfish_Obj)(unsafe.Pointer(fieldType)))
			mess := fmt.Sprintf("Invalid type for field '%s': '%s'",
				clownfish.CFStringToGo(unsafe.Pointer(field)),
				clownfish.CFStringToGo(unsafe.Pointer(className)))
			panic(clownfish.NewErr(mess))
		}
		if inventryIvars.value != obj {
			C.cfish_decref(unsafe.Pointer(inventryIvars.value))
			inventryIvars.value = C.cfish_inc_refcount(unsafe.Pointer(obj))
		}

		C.LUCY_Inverter_Add_Field(inverter, inventry)
	}
	C.cfish_dec_refcount(unsafe.Pointer(iter))
}
