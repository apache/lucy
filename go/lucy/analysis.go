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

#include "Lucy/Analysis/Analyzer.h"
#include "Lucy/Analysis/Token.h"
#include "Clownfish/Vector.h"
*/
import "C"
import "unsafe"

import "git-wip-us.apache.org/repos/asf/lucy-clownfish.git/runtime/go/clownfish"

func NewToken(text string) Token {
	chars := C.CString(text)
	defer C.free(unsafe.Pointer(chars))
	size := C.size_t(len(text))
	objC := C.lucy_Token_new(chars, size, 0, 0, 1.0, 1)
	return WRAPToken(unsafe.Pointer(objC))
}

func (t *TokenIMP) SetText(text string) {
	self := (*C.lucy_Token)(clownfish.Unwrap(t, "t"))
	chars := C.CString(text)
	defer C.free(unsafe.Pointer(chars))
	size := C.size_t(len(text))
	C.LUCY_Token_Set_Text(self, chars, size)
}

func (t *TokenIMP) GetText() string {
	self := (*C.lucy_Token)(clownfish.Unwrap(t, "t"))
	chars := C.LUCY_Token_Get_Text(self)
	size := C.LUCY_Token_Get_Len(self)
	return C.GoStringN(chars, C.int(size))
}

func (a *AnalyzerIMP) Split(text string) []string {
	self := (*C.lucy_Analyzer)(clownfish.Unwrap(a, "a"))
	input := (*C.cfish_String)(clownfish.GoToClownfish(text, unsafe.Pointer(C.CFISH_STRING), false))
	defer C.cfish_decref(unsafe.Pointer(input))
	retvalCF := C.LUCY_Analyzer_Split(self, input)
	size := C.CFISH_Vec_Get_Size(retvalCF)
	retvalGO := make([]string, int(size))
	for i := 0; i < int(size); i++ {
		elem := C.CFISH_Vec_Fetch(retvalCF, C.size_t(i))
		retvalGO[i] = clownfish.CFStringToGo(unsafe.Pointer(elem))
	}
	return retvalGO
}
