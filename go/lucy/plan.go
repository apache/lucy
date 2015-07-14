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
import "unsafe"

import "git-wip-us.apache.org/repos/asf/lucy-clownfish.git/runtime/go/clownfish"

func (obj *SchemaIMP) SpecField(field string, fieldType FieldType) {
	self := ((*C.lucy_Schema)(unsafe.Pointer(obj.TOPTR())))
	fieldCF := clownfish.NewString(field)
	C.LUCY_Schema_Spec_Field(self,
		(*C.cfish_String)(unsafe.Pointer(fieldCF.TOPTR())),
		(*C.lucy_FieldType)(unsafe.Pointer(fieldType.TOPTR())))
}

