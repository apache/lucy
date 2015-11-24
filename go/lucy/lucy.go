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
#define C_LUCY_POLYDOCREADER

#include "lucy_parcel.h"
#include "testlucy_parcel.h"
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
#include "Lucy/Index/DocReader.h"
#include "Lucy/Index/PolyReader.h"
#include "Lucy/Index/Segment.h"
#include "Lucy/Store/InStream.h"
#include "Lucy/Store/OutStream.h"
#include "Lucy/Object/I32Array.h"
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
import "strings"
import "regexp"
import "reflect"
import "git-wip-us.apache.org/repos/asf/lucy-clownfish.git/runtime/go/clownfish"

var registry *objRegistry

func init() {
	C.GOLUCY_glue_exported_symbols()
	C.lucy_bootstrap_parcel()
	C.testlucy_bootstrap_parcel()
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
	if fields == nil {
		fieldsGo := make(map[string]interface{})
		fieldsID := registry.store(fieldsGo)
		ivars.fields = unsafe.Pointer(fieldsID)
	} else {
		ivars.fields = fields
	}
	ivars.doc_id = docID
	return d
}

//export GOLUCY_Doc_Set_Fields
func GOLUCY_Doc_Set_Fields(d *C.lucy_Doc, fields unsafe.Pointer) {
	panic(clownfish.NewErr("Set_Fields unsupported from C-space in Go"))
}

//export GOLUCY_Doc_Get_Size
func GOLUCY_Doc_Get_Size(d *C.lucy_Doc) C.uint32_t {
	fields := fetchDocFields(d)
	return C.uint32_t(len(fields))
}

//export GOLUCY_Doc_Store
func GOLUCY_Doc_Store(d *C.lucy_Doc, field *C.cfish_String, value *C.cfish_Obj) {
	fields := fetchDocFields(d)
	fieldGo := clownfish.CFStringToGo(unsafe.Pointer(field))
	valGo := clownfish.ToGo(unsafe.Pointer(value))
	fields[fieldGo] = valGo
}

//export GOLUCY_Doc_Serialize
func GOLUCY_Doc_Serialize(d *C.lucy_Doc, outstream *C.lucy_OutStream) {
	ivars := C.lucy_Doc_IVARS(d)
	fields := fetchDocFields(d)
	hash := clownfish.GoToClownfish(fields, unsafe.Pointer(C.CFISH_HASH), false)
	defer C.cfish_decref(hash)
	C.lucy_Freezer_serialize_hash((*C.cfish_Hash)(hash), outstream)
	C.LUCY_OutStream_Write_C32(outstream, C.uint32_t(ivars.doc_id))
}

//export GOLUCY_Doc_Deserialize
func GOLUCY_Doc_Deserialize(d *C.lucy_Doc, instream *C.lucy_InStream) *C.lucy_Doc {
	ivars := C.lucy_Doc_IVARS(d)
	hash := unsafe.Pointer(C.lucy_Freezer_read_hash(instream))
	defer C.cfish_decref(hash)
	fields := clownfish.ToGo(hash)
	fieldsID := registry.store(fields)
	ivars.fields = unsafe.Pointer(fieldsID)
	ivars.doc_id = C.int32_t(C.LUCY_InStream_Read_C32(instream))
	return d
}

//export GOLUCY_Doc_Extract
func GOLUCY_Doc_Extract(d *C.lucy_Doc, field *C.cfish_String) *C.cfish_Obj {
	fields := fetchDocFields(d)
	fieldGo := clownfish.CFStringToGo(unsafe.Pointer(field))
	return (*C.cfish_Obj)(clownfish.GoToClownfish(fields[fieldGo],
		unsafe.Pointer(C.CFISH_OBJ), true))
}

//export GOLUCY_Doc_Field_Names
func GOLUCY_Doc_Field_Names(d *C.lucy_Doc) *C.cfish_Vector {
	fields := fetchDocFields(d)
	vec := clownfish.NewVector(len(fields))
	for key, _ := range fields {
		vec.Push(key)
	}
	return (*C.cfish_Vector)(C.cfish_incref(clownfish.Unwrap(vec, "vec")))
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
	fields := fetchDocFields(d)
	otherFields := fetchDocFields(twin)
	result := reflect.DeepEqual(fields, otherFields)
	return C.bool(result)
}

//export GOLUCY_Doc_Destroy
func GOLUCY_Doc_Destroy(d *C.lucy_Doc) {
	ivars := C.lucy_Doc_IVARS(d)
	fieldsID := uintptr(ivars.fields)
	registry.delete(fieldsID)
	C.cfish_super_destroy(unsafe.Pointer(d), C.LUCY_DOC)
}

func fetchEntry(ivars *C.lucy_InverterIVARS, fieldGo string) *C.lucy_InverterEntry {
	field := (*C.cfish_String)(clownfish.GoToClownfish(fieldGo,
		unsafe.Pointer(C.CFISH_STRING), false))
	defer C.cfish_decref(unsafe.Pointer(field))
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

func fetchDocFromDocReader(dr DocReader, docID int32, doc interface{}) error {
	switch v := dr.(type) {
	case *DefaultDocReaderIMP:
		return v.readDoc(docID, doc)
	case *PolyDocReaderIMP:
		return v.readDoc(docID, doc)
	default:
		panic(clownfish.NewErr(fmt.Sprintf("Unexpected type: %T", v)))
	}
}

func (pdr *PolyDocReaderIMP) readDoc(docID int32, doc interface{}) error {
	self := (*C.lucy_PolyDocReader)(clownfish.Unwrap(pdr, "pdr"))
	ivars := C.lucy_PolyDocReader_IVARS(self)
	segTick := C.lucy_PolyReader_sub_tick(ivars.offsets, C.int32_t(docID))
	offset := C.LUCY_I32Arr_Get(ivars.offsets, segTick)
	defDocReader := (*C.lucy_DefaultDocReader)(C.CFISH_Vec_Fetch(ivars.readers, C.size_t(segTick)))
	if (defDocReader == nil) {
		return clownfish.NewErr(fmt.Sprintf("Invalid docID: %d", docID))
	}
	if !C.cfish_Obj_is_a((*C.cfish_Obj)(unsafe.Pointer(defDocReader)), C.LUCY_DEFAULTDOCREADER) {
		panic(clownfish.NewErr("Unexpected type")) // sanity check
	}
	adjustedDocID := docID - int32(offset)
	err := doReadDocData(defDocReader, adjustedDocID, doc)
	if docDoc, ok := doc.(Doc); ok {
		docDoc.SetDocID(docID)
	}
	return err
}

func (ddr *DefaultDocReaderIMP) readDoc(docID int32, doc interface{}) error {
	self := (*C.lucy_DefaultDocReader)(clownfish.Unwrap(ddr, "ddr"))
	return doReadDocData(self, docID, doc)
}

func setMapField(store interface{}, field string, val interface{}) error {
	m := store.(map[string]interface{})
	m[field] = val
	return nil
}

func setStructField(store interface{}, field string, val interface{}) error {
	structStore := store.(reflect.Value)
	stringVal := val.(string) // TODO type switch
	match := func(name string) bool {
		return strings.EqualFold(field, name)
	}
	structField := structStore.FieldByNameFunc(match)
	if structField != (reflect.Value{}) { // TODO require match?
		structField.SetString(stringVal)
	}
	return nil
}

func doReadDocData(ddrC *C.lucy_DefaultDocReader, docID int32, doc interface{}) error {

	// Adapt for different types of "doc".
	var setField func(interface{}, string, interface{}) error
	var fields interface{}
	switch v := doc.(type) {
	case Doc:
		docC := (*C.lucy_Doc)(clownfish.Unwrap(v, "doc"))
		fieldsMap := fetchDocFields(docC)
		for field, _ := range fieldsMap {
			delete(fieldsMap, field)
		}
		fields = fieldsMap
		setField = setMapField
	case map[string]interface{}:
		for field, _ := range v {
			delete(v, field)
		}
		fields = v
		setField = setMapField
	default:
		// Get reflection value and type for the supplied struct.
		var hitValue reflect.Value
		if reflect.ValueOf(doc).Kind() == reflect.Ptr {
			temp := reflect.ValueOf(doc).Elem()
			if temp.Kind() == reflect.Struct {
				if temp.CanSet() {
					hitValue = temp
				}
			}
		}
		if hitValue == (reflect.Value{}) {
			mess := fmt.Sprintf("Arg not writeable struct pointer: %v",
				reflect.TypeOf(doc))
			return clownfish.NewErr(mess)
		}
		fields = hitValue
		setField = setStructField
	}

	ivars := C.lucy_DefDocReader_IVARS(ddrC)
	schema := ivars.schema
	datInstream := ivars.dat_in
	ixInstream := ivars.ix_in
	fieldNameCap := C.size_t(31)
	var fieldName *C.char = ((*C.char)(C.malloc(fieldNameCap + 1)))
	defer C.free(unsafe.Pointer(fieldName))

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
		fieldNameGo := C.GoStringN(fieldName, C.int(fieldNameLen))
		fieldType := C.LUCY_Schema_Fetch_Type(schema, fieldNameStr)
		C.cfish_dec_refcount(unsafe.Pointer(fieldNameStr))

		// Read the field value.
		switch C.LUCY_FType_Primitive_ID(fieldType) & C.lucy_FType_PRIMITIVE_ID_MASK {
		case C.lucy_FType_TEXT:
			valueLen := C.size_t(C.LUCY_InStream_Read_C32(datInstream))
			buf := ((*C.char)(C.malloc(valueLen + 1)))
			C.LUCY_InStream_Read_Bytes(datInstream, buf, valueLen)
			val := C.GoStringN(buf, C.int(valueLen))
			err := setField(fields, fieldNameGo, val)
			if err != nil {
				return err
			}
		case C.lucy_FType_BLOB:
			valueLen := C.size_t(C.LUCY_InStream_Read_C32(datInstream))
			buf := ((*C.char)(C.malloc(valueLen)))
			C.LUCY_InStream_Read_Bytes(datInstream, buf, valueLen)
			val := C.GoBytes(unsafe.Pointer(buf), C.int(valueLen))
			err := setField(fields, fieldNameGo, val)
			if err != nil {
				return err
			}
		case C.lucy_FType_FLOAT32:
			err := setField(fields, fieldNameGo, float32(C.LUCY_InStream_Read_F32(datInstream)))
			if err != nil {
				return err
			}
		case C.lucy_FType_FLOAT64:
			err := setField(fields, fieldNameGo, float64(C.LUCY_InStream_Read_F64(datInstream)))
			if err != nil {
				return err
			}
		case C.lucy_FType_INT32:
			err := setField(fields, fieldNameGo, int32(C.LUCY_InStream_Read_C32(datInstream)))
			if err != nil {
				return err
			}
		case C.lucy_FType_INT64:
			err := setField(fields, fieldNameGo, int64(C.LUCY_InStream_Read_C64(datInstream)))
			if err != nil {
				return err
			}
		default:
			return clownfish.NewErr(
				"Internal Lucy error: bad type id for field " + fieldNameGo)
		}
	}
	return nil
}

//export GOLUCY_DefDocReader_Fetch_Doc
func GOLUCY_DefDocReader_Fetch_Doc(ddr *C.lucy_DefaultDocReader,
	docID C.int32_t) *C.lucy_HitDoc {
	fields := make(map[string]interface{})
	err := doReadDocData(ddr, int32(docID), fields)
	if err != nil {
		panic(err)
	}
	fieldsID := registry.store(fields)
	retval := C.lucy_HitDoc_new(unsafe.Pointer(fieldsID), docID, 0.0)
	return retval
}

//export GOLUCY_Inverter_Invert_Doc
func GOLUCY_Inverter_Invert_Doc(inverter *C.lucy_Inverter, doc *C.lucy_Doc) {
	ivars := C.lucy_Inverter_IVARS(inverter)
	fields := fetchDocFields(doc)

	// Prepare for the new doc.
	C.LUCY_Inverter_Set_Doc(inverter, doc)

	// Extract and invert the doc's fields.
	for field, val := range(fields) {
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
			panic(clownfish.NewErr("Internal Lucy error: bad type id for field " + field))
		}
		temp := inventryIvars.value
		valCF := clownfish.GoToClownfish(val, unsafe.Pointer(expectedType), false)
		inventryIvars.value = C.cfish_inc_refcount(valCF)
		C.cfish_decref(unsafe.Pointer(temp))

		C.LUCY_Inverter_Add_Field(inverter, inventry)
	}
}

// Turn a Vector of Clownfish Strings into a slice of Go string.  NULL
// elements in the Vector are not allowed.
func vecToStringSlice(v *C.cfish_Vector) []string {
	if v == nil {
		return nil
	}
	length := int(C.CFISH_Vec_Get_Size(v))
	slice := make([]string, length)
	for i := 0; i < length; i++ {
		slice[i] = clownfish.CFStringToGo(unsafe.Pointer(C.CFISH_Vec_Fetch(v, C.size_t(i))))
	}
	return slice
}
