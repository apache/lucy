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
#include "Lucy/Document/HitDoc.h"
#include "Clownfish/Hash.h"
#include "Clownfish/HashIterator.h"
*/
import "C"
import "fmt"
import "reflect"
import "runtime"
import "strings"
import "unsafe"

import "git-wip-us.apache.org/repos/asf/lucy-clownfish.git/runtime/go/clownfish"

type Query interface {
	clownfish.Obj
}

type implQuery struct {
	ref *C.lucy_Query
}

type Searcher interface {
	clownfish.Obj
	Hits(query interface{}, offset uint32, numWanted uint32, sortSpec SortSpec) (Hits, error)
	Close() error
}

type Hits interface {
	clownfish.Obj
	Next(hit interface{}) bool
	Error() error
}

type implHits struct {
	ref *C.lucy_Hits
	err error
}

type SortSpec interface {
	clownfish.Obj
}

type implSortSpec struct {
	ref *C.lucy_SortSpec
}

type IndexSearcher interface {
	Searcher
}

type implIndexSearcher struct {
	ref *C.lucy_IndexSearcher
}

func OpenIndexSearcher(index interface{}) (obj IndexSearcher, err error) {
	var indexC *C.cfish_Obj
	switch index.(type) {
	case string:
		ixLoc := clownfish.NewString(index.(string))
		indexC = (*C.cfish_Obj)(unsafe.Pointer(ixLoc.TOPTR()))
	default:
		panic("TODO: support Folder")
	}
	err = clownfish.TrapErr(func() {
		cfObj := C.lucy_IxSearcher_new(indexC)
		obj = WRAPIndexSearcher(unsafe.Pointer(cfObj))
	})
	return obj, err
}

func WRAPIndexSearcher(ptr unsafe.Pointer) IndexSearcher {
	obj := &implIndexSearcher{(*C.lucy_IndexSearcher)(ptr)}
	runtime.SetFinalizer(obj, (*implIndexSearcher).finalize)
	return obj
}

func (obj *implIndexSearcher) finalize() {
	C.cfish_dec_refcount(unsafe.Pointer(obj.ref))
	obj.ref = nil
}

func (obj *implIndexSearcher) Close() error {
	return clownfish.TrapErr(func() {
		C.LUCY_IxSearcher_Close(obj.ref)
	})
}

func (obj *implIndexSearcher) TOPTR() uintptr {
	return uintptr(unsafe.Pointer(obj.ref))
}

func (obj *implIndexSearcher) Hits(query interface{}, offset uint32, numWanted uint32,
	sortSpec SortSpec) (hits Hits, err error) {
	var sortSpecC *C.lucy_SortSpec
	if sortSpec != nil {
		sortSpecC = (*C.lucy_SortSpec)(unsafe.Pointer(sortSpec.TOPTR()))
	}
	switch query.(type) {
	case string:
		queryStringC := clownfish.NewString(query.(string))
		err = clownfish.TrapErr(func() {
			hitsC := C.LUCY_IxSearcher_Hits(obj.ref,
				(*C.cfish_Obj)(unsafe.Pointer(queryStringC.TOPTR())),
				C.uint32_t(offset), C.uint32_t(numWanted), sortSpecC)
			hits = WRAPHits(unsafe.Pointer(hitsC))
		})
	default:
		panic("TODO: support Query objects")
	}
	return hits, err
}

func WRAPHits(ptr unsafe.Pointer) Hits {
	obj := &implHits{(*C.lucy_Hits)(ptr), nil}
	runtime.SetFinalizer(obj, (*implHits).finalize)
	return obj
}

func (obj *implHits) TOPTR() uintptr {
	return uintptr(unsafe.Pointer(obj.ref))
}

func (obj *implHits) Next(hit interface{}) bool {
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
		docC = C.LUCY_Hits_Next(obj.ref)
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

func (obj *implHits) finalize() {
	C.cfish_dec_refcount(unsafe.Pointer(obj.ref))
	obj.ref = nil
}

func (obj *implHits) Error() error {
	return obj.err
}
