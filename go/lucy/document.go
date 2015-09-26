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
#include "Lucy/Document/Doc.h"
#include "Lucy/Document/HitDoc.h"
*/
import "C"
import "unsafe"

func NewDoc(docID int32) Doc {
	retvalCF := C.lucy_Doc_new(nil, C.int32_t(docID))
	return WRAPDoc(unsafe.Pointer(retvalCF))
}

func NewHitDoc(docID int32, score float32) HitDoc {
	retvalCF := C.lucy_HitDoc_new(nil, C.int32_t(docID), C.float(score))
	return WRAPHitDoc(unsafe.Pointer(retvalCF))
}
