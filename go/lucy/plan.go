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
#include "Lucy/Plan/Schema.h"
#include "Lucy/Plan/FullTextType.h"
*/
import "C"
import "runtime"
import "unsafe"

import "git-wip-us.apache.org/repos/asf/lucy-clownfish.git/runtime/go/clownfish"

type Schema interface {
	clownfish.Obj
	SpecField(field string, fieldType FieldType)
}

type implSchema struct {
	ref *C.lucy_Schema
}

type FieldType interface {
	clownfish.Obj
	ToFieldTypePtr() uintptr
}

type implFieldType struct {
	ref *C.lucy_FieldType
}

type FullTextType interface {
	FieldType
}

type implFullTextType struct {
	ref *C.lucy_FullTextType
}

func NewSchema() Schema {
	cfObj := C.lucy_Schema_new()
	return WRAPSchema(unsafe.Pointer(cfObj))
}

func WRAPSchema(ptr unsafe.Pointer) Schema {
	obj := &implSchema{(*C.lucy_Schema)(ptr)}
	runtime.SetFinalizer(obj, (*implSchema).finalize)
	return obj
}

func (obj *implSchema) finalize() {
	C.cfish_dec_refcount(unsafe.Pointer(obj.ref))
	obj.ref = nil
}

func (obj *implSchema) SpecField(field string, fieldType FieldType) {
	fieldCF := clownfish.NewString(field)
	C.LUCY_Schema_Spec_Field(obj.ref,
		(*C.cfish_String)(unsafe.Pointer(fieldCF.ToPtr())),
		(*C.lucy_FieldType)(unsafe.Pointer(fieldType.ToPtr())))
}

func (obj *implSchema) ToPtr() uintptr {
	return uintptr(unsafe.Pointer(obj.ref))
}

func NewFullTextType(analyzer Analyzer) FullTextType {
	cfObj := C.lucy_FullTextType_new(
		(*C.lucy_Analyzer)(unsafe.Pointer(analyzer.ToPtr())))
	return WRAPFullTextType(unsafe.Pointer(cfObj))
}

func WRAPFullTextType(ptr unsafe.Pointer) FullTextType {
	obj := &implFullTextType{(*C.lucy_FullTextType)(ptr)}
	runtime.SetFinalizer(obj, (*implFullTextType).finalize)
	return obj
}

func (obj *implFullTextType) finalize() {
	C.cfish_dec_refcount(unsafe.Pointer(obj.ref))
	obj.ref = nil
}

func (obj *implFullTextType) ToPtr() uintptr {
	return uintptr(unsafe.Pointer(obj.ref))
}

func (obj *implFullTextType) ToFieldTypePtr() uintptr {
	return obj.ToPtr()
}
