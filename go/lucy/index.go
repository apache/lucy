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
#include "Lucy/Index/Indexer.h"
#include "Lucy/Index/IndexManager.h"
#include "Lucy/Document/Doc.h"
#include "Lucy/Plan/Schema.h"
#include "Clownfish/Hash.h"
#include "Clownfish/String.h"
#include "Clownfish/Vector.h"
#include "Clownfish/Err.h"
*/
import "C"
import "fmt"
import "reflect"
import "runtime"
import "strings"
import "unsafe"
import "git-wip-us.apache.org/repos/asf/lucy-clownfish.git/runtime/go/clownfish"

type Indexer interface {
	clownfish.Obj
	Close() error
	AddDoc(doc interface{}) error
	Commit() error
}

type implIndexer struct {
	ref        *C.lucy_Indexer
	fieldNames map[string]clownfish.String
}

type IndexManager interface {
	clownfish.Obj
}

type implIndexManager struct {
	ref *C.lucy_IndexManager
}

type OpenIndexerArgs struct {
	Schema   Schema
	Index    interface{}
	Manager  IndexManager
	Create   bool
	Truncate bool
}

func OpenIndexer(args *OpenIndexerArgs) (obj Indexer, err error) {
	var schemaC *C.lucy_Schema = nil
	if args.Schema != nil {
		schemaC = (*C.lucy_Schema)(unsafe.Pointer(args.Schema.TOPTR()))
	}
	switch args.Index.(type) {
	case string:
	default:
		panic("TODO: support Folder")
	}
	ixLoc := clownfish.NewString(args.Index.(string))
	var managerC *C.lucy_IndexManager = nil
	if args.Manager != nil {
		managerC = (*C.lucy_IndexManager)(unsafe.Pointer(args.Manager.TOPTR()))
	}
	var flags int32
	if args.Create {
		flags = flags | int32(C.lucy_Indexer_CREATE)
	}
	if args.Truncate {
		flags = flags | int32(C.lucy_Indexer_TRUNCATE)
	}
	err = clownfish.TrapErr(func() {
		cfObj := C.lucy_Indexer_new(schemaC,
			(*C.cfish_Obj)(unsafe.Pointer(ixLoc.TOPTR())),
			managerC, C.int32_t(flags))
		obj = WRAPIndexer(unsafe.Pointer(cfObj))
	})
	return obj, err
}

func WRAPIndexer(ptr unsafe.Pointer) Indexer {
	obj := &implIndexer{(*C.lucy_Indexer)(ptr), nil}
	runtime.SetFinalizer(obj, (*implIndexer).finalize)
	return obj
}

func (obj *implIndexer) finalize() {
	obj.Close()
}

func (obj *implIndexer) Close() error {
	// TODO: implement Close in core Lucy rather than bindings.
	if obj.ref != nil {
		C.cfish_dec_refcount(unsafe.Pointer(obj.ref))
		obj.ref = nil
	}
	return nil // TODO catch errors
}

func (obj *implIndexer) AddDoc(doc interface{}) error {
	stockDoc := C.LUCY_Indexer_Get_Stock_Doc(obj.ref)
	docFields := (*C.cfish_Hash)(C.LUCY_Doc_Get_Fields(stockDoc))
	C.CFISH_Hash_Clear(docFields)

	// TODO: Support map as doc in addition to struct as doc.

	// Get reflection value and type for the supplied struct.
	var docValue reflect.Value
	if reflect.ValueOf(doc).Kind() == reflect.Ptr {
		temp := reflect.ValueOf(doc).Elem()
		if temp.Kind() == reflect.Struct {
			docValue = temp
		}
	}
	if docValue == (reflect.Value{}) {
		mess := fmt.Sprintf("Doc not struct pointer: %v",
			reflect.TypeOf(doc))
		return clownfish.NewErr(mess)
	}
	docType := docValue.Type()

	for i := 0; i < docValue.NumField(); i++ {
		field := docType.Field(i).Name
		value := docValue.Field(i).String()
		fieldC := obj.findFieldC(field)
		valueC := clownfish.NewString(value)
		C.CFISH_Hash_Store(docFields,
			(*C.cfish_String)(unsafe.Pointer(fieldC)),
			C.cfish_inc_refcount(unsafe.Pointer(valueC.TOPTR())))
	}

	// TODO create an additional method AddDocWithBoost which allows the
	// client to supply `boost`.
	boost := 1.0
	err := clownfish.TrapErr(func() {
		C.LUCY_Indexer_Add_Doc(obj.ref, stockDoc, C.float(boost))
	})

	return err
}

func (obj *implIndexer) findFieldC(name string) *C.cfish_String {
	if obj.fieldNames == nil {
		obj.fieldNames = make(map[string]clownfish.String)
	}
	f, ok := obj.fieldNames[name]
	if !ok {
		schema := C.LUCY_Indexer_Get_Schema(obj.ref)
		fieldList := C.LUCY_Schema_All_Fields(schema)
		defer C.cfish_dec_refcount(unsafe.Pointer(fieldList))
		for i := 0; i < int(C.CFISH_Vec_Get_Size(fieldList)); i++ {
			cfString := unsafe.Pointer(C.CFISH_Vec_Fetch(fieldList, C.uint32_t(i)))
			field := clownfish.CFStringToGo(cfString)
			if strings.EqualFold(name, field) {
				C.cfish_inc_refcount(cfString)
				f = clownfish.WRAPString(cfString)
				obj.fieldNames[name] = f
			}
		}
	}
	return (*C.cfish_String)(unsafe.Pointer(f.TOPTR()))
}

func (obj *implIndexer) Commit() error {
	return clownfish.TrapErr(func() {
		C.LUCY_Indexer_Commit(obj.ref)
	})
}

func (obj *implIndexer) TOPTR() uintptr {
	return uintptr(unsafe.Pointer(obj.ref))
}

func (obj *implIndexManager) TOPTR() uintptr {
	return uintptr(unsafe.Pointer(obj.ref))
}
