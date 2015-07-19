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
#define C_LUCY_DOC
#define C_LUCY_REGEXTOKENIZER

#include "lucy_parcel.h"
#include "Lucy/Analysis/RegexTokenizer.h"
#include "Lucy/Document/Doc.h"

#include "Clownfish/Hash.h"
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
extern bool
GOLUCY_Doc_Equals(lucy_Doc *self, cfish_Obj *other);
extern bool
(*GOLUCY_Doc_Equals_BRIDGE)(lucy_Doc *self, cfish_Obj *other);
extern void
GOLUCY_Doc_Destroy(lucy_Doc *self);
extern void
(*GOLUCY_Doc_Destroy_BRIDGE)(lucy_Doc *self);



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
	GOLUCY_Doc_Equals_BRIDGE = GOLUCY_Doc_Equals;
	GOLUCY_Doc_Destroy_BRIDGE = GOLUCY_Doc_Destroy;
}

*/
import "C"
import "unsafe"
import _ "git-wip-us.apache.org/repos/asf/lucy-clownfish.git/runtime/go/clownfish"

func init() {
	C.GOLUCY_glue_exported_symbols()
	C.lucy_bootstrap_parcel()
}

//export GOLUCY_RegexTokenizer_init
func GOLUCY_RegexTokenizer_init(rt *C.lucy_RegexTokenizer, pattern *C.cfish_String) *C.lucy_RegexTokenizer {
	return nil
}

//export GOLUCY_RegexTokenizer_Destroy
func GOLUCY_RegexTokenizer_Destroy(rt *C.lucy_RegexTokenizer) {
}

//export GOLUCY_RegexTokenizer_Tokenize_Utf8
func GOLUCY_RegexTokenizer_Tokenize_Utf8(rt *C.lucy_RegexTokenizer, str *C.char,
	stringLen C.size_t, inversion *C.lucy_Inversion) {
}

func NewDoc(docID int32) Doc {
	retvalCF := C.lucy_Doc_new(nil, C.int32_t(docID))
	return WRAPDoc(unsafe.Pointer(retvalCF))
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
