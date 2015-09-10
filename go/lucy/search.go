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
#include "Lucy/Search/Collector.h"
#include "Lucy/Search/Hits.h"
#include "Lucy/Search/IndexSearcher.h"
#include "Lucy/Search/Query.h"
#include "Lucy/Search/Searcher.h"
#include "Lucy/Search/ANDQuery.h"
#include "Lucy/Search/ORQuery.h"
#include "Lucy/Document/HitDoc.h"
#include "Clownfish/Hash.h"
#include "Clownfish/HashIterator.h"
*/
import "C"
import "fmt"
import "reflect"
import "strings"
import "unsafe"

import "git-wip-us.apache.org/repos/asf/lucy-clownfish.git/runtime/go/clownfish"

type HitsIMP struct {
	clownfish.ObjIMP
	err error
}

func OpenIndexSearcher(index interface{}) (obj IndexSearcher, err error) {
	indexC := (*C.cfish_Obj)(clownfish.GoToClownfish(index, unsafe.Pointer(C.CFISH_OBJ), false))
	defer C.cfish_decref(unsafe.Pointer(indexC))
	err = clownfish.TrapErr(func() {
		cfObj := C.lucy_IxSearcher_new(indexC)
		obj = WRAPIndexSearcher(unsafe.Pointer(cfObj))
	})
	return obj, err
}

func (obj *IndexSearcherIMP) Close() error {
	return doClose(obj)
}

func (obj *IndexSearcherIMP) Hits(query interface{}, offset uint32, numWanted uint32,
	sortSpec SortSpec) (hits Hits, err error) {
	return doHits(obj, query, offset, numWanted, sortSpec)
}

func doClose(obj Searcher) error {
	self := ((*C.lucy_Searcher)(unsafe.Pointer(obj.TOPTR())))
	return clownfish.TrapErr(func() {
		C.LUCY_Searcher_Close(self)
	})
}

func doHits(obj Searcher, query interface{}, offset uint32, numWanted uint32,
	sortSpec SortSpec) (hits Hits, err error) {
	self := ((*C.lucy_Searcher)(unsafe.Pointer(obj.TOPTR())))
	var sortSpecC *C.lucy_SortSpec
	if sortSpec != nil {
		sortSpecC = (*C.lucy_SortSpec)(unsafe.Pointer(sortSpec.TOPTR()))
	}
	switch query.(type) {
	case string:
		queryStringC := clownfish.NewString(query.(string))
		err = clownfish.TrapErr(func() {
			hitsC := C.LUCY_Searcher_Hits(self,
				(*C.cfish_Obj)(unsafe.Pointer(queryStringC.TOPTR())),
				C.uint32_t(offset), C.uint32_t(numWanted), sortSpecC)
			hits = WRAPHits(unsafe.Pointer(hitsC))
		})
	default:
		panic("TODO: support Query objects")
	}
	return hits, err
}

func (obj *SearcherIMP) Close() error {
	return doClose(obj)
}

func (obj *SearcherIMP) Hits(query interface{}, offset uint32, numWanted uint32,
	sortSpec SortSpec) (hits Hits, err error) {
	return doHits(obj, query, offset, numWanted, sortSpec)
}

func (obj *PolySearcherIMP) Close() error {
	return doClose(obj)
}

func (obj *PolySearcherIMP) Hits(query interface{}, offset uint32, numWanted uint32,
	sortSpec SortSpec) (hits Hits, err error) {
	return doHits(obj, query, offset, numWanted, sortSpec)
}

func (obj *HitsIMP) Next(hit interface{}) bool {
	self := ((*C.lucy_Hits)(unsafe.Pointer(obj.TOPTR())))
	// TODO: accept a HitDoc object and populate score.

	// Get reflection value and type for the supplied struct.
	var hitValue reflect.Value
	if reflect.ValueOf(hit).Kind() == reflect.Ptr {
		temp := reflect.ValueOf(hit).Elem()
		if temp.Kind() == reflect.Struct {
			if temp.CanSet() {
				hitValue = temp
			}
		}
	}
	if hitValue == (reflect.Value{}) {
		mess := fmt.Sprintf("Arg not writeable struct pointer: %v",
			reflect.TypeOf(hit))
		obj.err = clownfish.NewErr(mess)
		return false
	}

	var docC *C.lucy_HitDoc
	errCallingNext := clownfish.TrapErr(func() {
		docC = C.LUCY_Hits_Next(self)
	})
	if errCallingNext != nil {
		obj.err = errCallingNext
		return false
	}
	if docC == nil {
		return false
	}
	defer C.cfish_dec_refcount(unsafe.Pointer(docC))

	fields := (*C.cfish_Hash)(unsafe.Pointer(C.LUCY_HitDoc_Get_Fields(docC)))
	iterator := C.cfish_HashIter_new(fields)
	defer C.cfish_dec_refcount(unsafe.Pointer(iterator))
	for C.CFISH_HashIter_Next(iterator) {
		keyC := C.CFISH_HashIter_Get_Key(iterator)
		valC := C.CFISH_HashIter_Get_Value(iterator)
		key := clownfish.CFStringToGo(unsafe.Pointer(keyC))
		val := clownfish.CFStringToGo(unsafe.Pointer(valC))
		match := func(name string) bool {
			return strings.EqualFold(key, name)
		}
		structField := hitValue.FieldByNameFunc(match)
		if structField != (reflect.Value{}) {
			structField.SetString(val)
		}
	}
	return true
}

func (obj *HitsIMP) Error() error {
	return obj.err
}

func NewANDQuery(children []Query) ANDQuery {
	vec := clownfish.NewVector(len(children))
	for _, child := range children {
		vec.Push(child)
	}
	childrenC := (*C.cfish_Vector)(unsafe.Pointer(vec.TOPTR()))
	cfObj := C.lucy_ANDQuery_new(childrenC)
	return WRAPANDQuery(unsafe.Pointer(cfObj))
}

func NewORQuery(children []Query) ORQuery {
	vec := clownfish.NewVector(len(children))
	for _, child := range children {
		vec.Push(child)
	}
	childrenC := (*C.cfish_Vector)(unsafe.Pointer(vec.TOPTR()))
	cfObj := C.lucy_ORQuery_new(childrenC)
	return WRAPORQuery(unsafe.Pointer(cfObj))
}
