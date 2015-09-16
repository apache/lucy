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

#include "Lucy/Document/Doc.h"
#include "Lucy/Document/HitDoc.h"
*/
import "C"
import "unsafe"
import "fmt"

import "git-wip-us.apache.org/repos/asf/lucy-clownfish.git/runtime/go/clownfish"


func NewDoc(docID int32) Doc {
	retvalCF := C.lucy_Doc_new(nil, C.int32_t(docID))
	return WRAPDoc(unsafe.Pointer(retvalCF))
}

func NewHitDoc(docID int32, score float32) HitDoc {
	retvalCF := C.lucy_HitDoc_new(nil, C.int32_t(docID), C.float(score))
	return WRAPHitDoc(unsafe.Pointer(retvalCF))
}

func fetchDocFields(d *C.lucy_Doc) *C.cfish_Hash {
	ivars := C.lucy_Doc_IVARS(d)
	fieldsID := uintptr(ivars.fields)
	fieldsGo, ok := registry.fetch(fieldsID).(clownfish.Hash)
	if !ok {
		panic(clownfish.NewErr(fmt.Sprintf("Failed to fetch doc %d from registry ", fieldsID)))
	}
	return (*C.cfish_Hash)(clownfish.Unwrap(fieldsGo, "fieldsGo"))
}
