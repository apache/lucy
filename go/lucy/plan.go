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

type Schema struct {
	ref *C.lucy_Schema
}

type FieldType interface {
	clownfish.Obj
	ToFieldTypePtr() uintptr
}

type FullTextType struct {
	ref *C.lucy_FullTextType
}

func NewSchema() *Schema {
	obj := &Schema{
		C.lucy_Schema_new(),
	}
	runtime.SetFinalizer(obj, (*Schema).finalize)
	return obj
}

func (obj *Schema) finalize() {
	C.cfish_dec_refcount(unsafe.Pointer(obj.ref))
	obj.ref = nil
}

func (obj *Schema) SpecField(field string, fieldType FieldType) {
	fieldCF := clownfish.NewString(field)
	C.LUCY_Schema_Spec_Field(obj.ref,
		(*C.cfish_String)(unsafe.Pointer(fieldCF.ToPtr())),
		(*C.lucy_FieldType)(unsafe.Pointer(fieldType.ToFieldTypePtr())))
}

func NewFullTextType(analyzer Analyzer) *FullTextType {
	obj := &FullTextType{
		C.lucy_FullTextType_new(
			(*C.lucy_Analyzer)(unsafe.Pointer(analyzer.ToAnalyzerPtr())),
		),
	}
	runtime.SetFinalizer(obj, (*FullTextType).finalize)
	return obj
}

func (obj *FullTextType) finalize() {
	C.cfish_dec_refcount(unsafe.Pointer(obj.ref))
	obj.ref = nil
}

func (obj *FullTextType) ToPtr() uintptr {
	return uintptr(unsafe.Pointer(obj.ref))
}

func (obj *FullTextType) ToFieldTypePtr() uintptr {
	return obj.ToPtr()
}
