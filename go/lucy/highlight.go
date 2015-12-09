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

#include "Lucy/Highlight/HeatMap.h"
#include "Clownfish/Vector.h"
*/
import "C"
import "unsafe"

import "git-wip-us.apache.org/repos/asf/lucy-clownfish.git/runtime/go/clownfish"

func vecToSpanSlice(v *C.cfish_Vector) []Span {
	if v == nil {
		return nil
	}
	length := int(C.CFISH_Vec_Get_Size(v))
	slice := make([]Span, length)
	for i := 0; i < length; i++ {
		elem := C.CFISH_Vec_Fetch(v, C.size_t(i))
		slice[i] = clownfish.ToGo(unsafe.Pointer(elem)).(Span)
	}
	return slice
}

func spanSliceToVec(slice []Span) *C.cfish_Vector {
	if slice == nil {
		return nil
	}
	length := len(slice)
	vec := C.cfish_Vec_new(C.size_t(length))
	for i := 0; i < length; i++ {
		elem := clownfish.Unwrap(slice[i], "slice[i]")
		C.CFISH_Vec_Push(vec, C.cfish_incref(elem))
	}
	return vec
}

func NewHeatMap(spans []Span, window uint32) HeatMap {
	spansCF := spanSliceToVec(spans)
	defer C.cfish_decref(unsafe.Pointer(spansCF))
	retvalCF := C.lucy_HeatMap_new(spansCF, C.uint32_t(window))
	return clownfish.WRAPAny(unsafe.Pointer(retvalCF)).(HeatMap)
}

func (h *HeatMapIMP) flattenSpans(spans []Span) []Span {
	self := (*C.lucy_HeatMap)(clownfish.Unwrap(h, "h"))
	spansCF := spanSliceToVec(spans)
	defer C.cfish_decref(unsafe.Pointer(spansCF))
	retvalCF := C.LUCY_HeatMap_Flatten_Spans(self, spansCF)
	defer C.cfish_decref(unsafe.Pointer(retvalCF))
	return vecToSpanSlice(retvalCF)
}

func (h *HeatMapIMP) generateProximityBoosts(spans []Span) []Span {
	self := (*C.lucy_HeatMap)(clownfish.Unwrap(h, "h"))
	spansCF := spanSliceToVec(spans)
	defer C.cfish_decref(unsafe.Pointer(spansCF))
	retvalCF := C.LUCY_HeatMap_Generate_Proximity_Boosts(self, spansCF)
	defer C.cfish_decref(unsafe.Pointer(retvalCF))
	return vecToSpanSlice(retvalCF)
}

func (h *HeatMapIMP) getSpans() []Span {
	self := (*C.lucy_HeatMap)(clownfish.Unwrap(h, "h"))
	retvalCF := C.LUCY_HeatMap_Get_Spans(self)
	return vecToSpanSlice(retvalCF)
}
