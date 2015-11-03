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
#include "Lucy/Object/BitVector.h"
#include "Lucy/Object/I32Array.h"
*/
import "C"
import "fmt"
import "unsafe"

import "git-wip-us.apache.org/repos/asf/lucy-clownfish.git/runtime/go/clownfish"

func (bv *BitVectorIMP) ToArray() []bool {
	cap := bv.getCapacity()
	if cap != uint32(int(cap)) {
		panic(fmt.Sprintf("Capacity of range: %d", cap))
	}
	bools := make([]bool, int(cap))
	for i := uint32(0); i < cap; i++ {
		bools[i] = bv.Get(i)
	}
	return bools
}

func NewI32Array(nums []int32) I32Array {
	size := len(nums)
	if int(C.uint32_t(size)) != size {
		panic(clownfish.NewErr("input too large"))
	}
	obj := C.lucy_I32Arr_new_blank(C.uint32_t(size))
	for i := 0; i < size; i++ {
		C.LUCY_I32Arr_Set(obj, C.uint32_t(i), C.int32_t(nums[i]))
	}
	return WRAPI32Array(unsafe.Pointer(obj))
}

func i32ArrayToSlice(a *C.lucy_I32Array) []int32 {
	size := int(C.LUCY_I32Arr_Get_Size(a))
	nums := make([]int32, size)
	for i := 0; i < size; i++ {
		nums[i] = int32(C.LUCY_I32Arr_Get(a, C.uint32_t(i)))
	}
	return nums
}
