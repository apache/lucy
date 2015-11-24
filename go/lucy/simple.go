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

#include "Lucy/Simple.h"

*/
import "C"
import "unsafe"
import "reflect"
import "fmt"

import "git-wip-us.apache.org/repos/asf/lucy-clownfish.git/runtime/go/clownfish"

type SimpleIMP struct {
	clownfish.ObjIMP
	err error
}

func OpenSimple(index interface{}, language string) (s Simple, err error) {
	indexC := (*C.cfish_Obj)(clownfish.GoToClownfish(index, unsafe.Pointer(C.CFISH_OBJ), false))
	defer C.cfish_decref(unsafe.Pointer(indexC))
	languageC := (*C.cfish_String)(clownfish.GoToClownfish(language, unsafe.Pointer(C.CFISH_STRING), false))
	defer C.cfish_decref(unsafe.Pointer(languageC))
	err = clownfish.TrapErr(func() {
		cfObj := C.lucy_Simple_new(indexC, languageC)
		s = WRAPSimple(unsafe.Pointer(cfObj))
	})
	return s, err
}

func (s *SimpleIMP) AddDoc(doc interface{}) error {
	self := (*C.lucy_Simple)(clownfish.Unwrap(s, "s"))
	indexer := s.getIndexer()
	var docToIndex Doc
	stockDoc := indexer.getStockDoc()
	stockDocC := (*C.lucy_Doc)(clownfish.Unwrap(stockDoc, "stockDoc"))
	docFields := fetchDocFields(stockDocC)
	for field := range docFields {
		delete(docFields, field)
	}

	switch d := doc.(type) {
	case map[string]interface{}:
		for k, v := range d {
			docFields[k] = v
		}
		docToIndex = stockDoc
	case Doc:
		docToIndex = d
	default:
		docToIndex = stockDoc
		// Get reflection value and type for the supplied struct.
		var docValue reflect.Value
		var success bool
		if reflect.ValueOf(doc).Kind() == reflect.Ptr {
			temp := reflect.ValueOf(doc).Elem()
			if temp.Kind() == reflect.Struct {
				docValue = temp
				success = true
			}
		}
		if !success {
			mess := fmt.Sprintf("Unexpected type for doc: %T", doc)
			return clownfish.NewErr(mess)
		}

		// Copy field values into stockDoc.
		docType := docValue.Type()
		for i := 0; i < docValue.NumField(); i++ {
			field := docType.Field(i).Name
			value := docValue.Field(i).String()
			docFields[field] = value
		}
	}

	return clownfish.TrapErr(func(){
		docC := (*C.lucy_Doc)(clownfish.Unwrap(docToIndex, "docToIndex"))
		C.LUCY_Simple_Add_Doc(self, docC)
	})
}

func (s *SimpleIMP) Search(query string, offset, numWanted int) (totalHits int, err error) {
	err = clownfish.TrapErr(func() {
		self := (*C.lucy_Simple)(clownfish.Unwrap(s, "s"))
		qStringC := (*C.cfish_String)(clownfish.GoToClownfish(query, unsafe.Pointer(C.CFISH_STRING), false))
		defer C.cfish_decref(unsafe.Pointer(qStringC))
		totalHits = int(C.LUCY_Simple_Search(self, qStringC, C.uint32_t(offset), C.uint32_t(numWanted)))
	})
	return totalHits, err
}

func (s *SimpleIMP) Next(hit interface{}) bool {
	var retval bool
	if hits := s.getHits(); hits != nil {
		retval = hits.Next(hit)
		s.err = hits.Error()
	}
	return retval
}

func (s *SimpleIMP) Error() error {
	return s.err
}
