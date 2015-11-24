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
#include "Lucy/Index/BackgroundMerger.h"
#include "Lucy/Index/TermVector.h"
#include "Lucy/Index/Segment.h"
#include "Lucy/Index/Snapshot.h"
#include "Lucy/Index/SortCache.h"
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
import "strings"
import "unsafe"
import "git-wip-us.apache.org/repos/asf/lucy-clownfish.git/runtime/go/clownfish"

type IndexerIMP struct {
	clownfish.ObjIMP
	fieldNames map[string]string
}

type OpenIndexerArgs struct {
	Schema   Schema
	Index    interface{}
	Manager  IndexManager
	Create   bool
	Truncate bool
}

func OpenIndexer(args *OpenIndexerArgs) (obj Indexer, err error) {
	schema := (*C.lucy_Schema)(clownfish.UnwrapNullable(args.Schema))
	manager := (*C.lucy_IndexManager)(clownfish.UnwrapNullable(args.Manager))
	index := (*C.cfish_Obj)(clownfish.GoToClownfish(args.Index, unsafe.Pointer(C.CFISH_OBJ), false))
	defer C.cfish_decref(unsafe.Pointer(index))
	var flags int32
	if args.Create {
		flags = flags | int32(C.lucy_Indexer_CREATE)
	}
	if args.Truncate {
		flags = flags | int32(C.lucy_Indexer_TRUNCATE)
	}
	err = clownfish.TrapErr(func() {
		cfObj := C.lucy_Indexer_new(schema, index, manager, C.int32_t(flags))
		obj = WRAPIndexer(unsafe.Pointer(cfObj))
	})
	return obj, err
}

func (obj *IndexerIMP) Close() error {
	// TODO: implement Close in core Lucy rather than bindings.
	return nil // TODO catch errors
}

func (obj *IndexerIMP) addDocObj(doc Doc, boost float32) error {
	self := (*C.lucy_Indexer)(clownfish.Unwrap(obj, "obj"))
	d := (*C.lucy_Doc)(clownfish.Unwrap(doc, "doc"))
	return clownfish.TrapErr(func() {
		C.LUCY_Indexer_Add_Doc(self, d, C.float(boost))
	})
}

func (obj *IndexerIMP) addMapAsDoc(doc map[string]interface{}, boost float32) error {
	self := (*C.lucy_Indexer)(clownfish.Unwrap(obj, "obj"))
	d := C.LUCY_Indexer_Get_Stock_Doc(self)
	docFields := fetchDocFields(d)
	for field := range docFields {
		delete(docFields, field)
	}
	for key, value := range doc {
		field, err := obj.findRealField(key)
		if err != nil {
			return err
		}
		docFields[field] = value
	}
	return clownfish.TrapErr(func() {
		C.LUCY_Indexer_Add_Doc(self, d, C.float(boost))
	})
}

func (obj *IndexerIMP) addStructAsDoc(doc interface{}, boost float32) error {
	self := (*C.lucy_Indexer)(clownfish.Unwrap(obj, "obj"))
	d := C.LUCY_Indexer_Get_Stock_Doc(self)
	docFields := fetchDocFields(d)
	for field := range docFields {
		delete(docFields, field)
	}

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
		mess := fmt.Sprintf("Unexpected type for doc: %t", doc)
		return clownfish.NewErr(mess)
	}

	// Copy field values into stockDoc.
	docType := docValue.Type()
	for i := 0; i < docValue.NumField(); i++ {
		field := docType.Field(i).Name
		value := docValue.Field(i).String()
		realField, err := obj.findRealField(field)
		if err != nil {
			return err
		}
		docFields[realField] = value
	}

	return clownfish.TrapErr(func() {
		C.LUCY_Indexer_Add_Doc(self, d, C.float(boost))
	})
}

func (obj *IndexerIMP) AddDoc(doc interface{}) error {
	// TODO create an additional method AddDocWithBoost which allows the
	// client to supply `boost`.
	boost := float32(1.0)

	if suppliedDoc, ok := doc.(Doc); ok {
		return obj.addDocObj(suppliedDoc, boost)
	} else if m, ok := doc.(map[string]interface{}); ok {
		return obj.addMapAsDoc(m, boost)
	} else {
		return obj.addStructAsDoc(doc, boost)
	}
}

func (obj *IndexerIMP) findRealField(name string) (string, error) {
	self := ((*C.lucy_Indexer)(unsafe.Pointer(obj.TOPTR())))
	if obj.fieldNames == nil {
		obj.fieldNames = make(map[string]string)
	}
	if field, ok := obj.fieldNames[name]; ok {
		return field, nil
	} else {
		schema := C.LUCY_Indexer_Get_Schema(self)
		fieldList := C.LUCY_Schema_All_Fields(schema)
		defer C.cfish_dec_refcount(unsafe.Pointer(fieldList))
		for i := 0; i < int(C.CFISH_Vec_Get_Size(fieldList)); i++ {
			cfString := unsafe.Pointer(C.CFISH_Vec_Fetch(fieldList, C.size_t(i)))
			field := clownfish.CFStringToGo(cfString)
			if strings.EqualFold(name, field) {
				obj.fieldNames[name] = field
				return field, nil
			}
		}
	}
	return "", clownfish.NewErr(fmt.Sprintf("Unknown field: '%v'", name))
}

func (obj *IndexerIMP) AddIndex(index interface{}) error {
	return clownfish.TrapErr(func() {
		self := (*C.lucy_Indexer)(clownfish.Unwrap(obj, "obj"))
		indexC := (*C.cfish_Obj)(clownfish.GoToClownfish(index, unsafe.Pointer(C.CFISH_OBJ), false))
		defer C.cfish_decref(unsafe.Pointer(indexC))
		C.LUCY_Indexer_Add_Index(self, indexC)
	})
}

func (obj *IndexerIMP) DeleteByTerm(field string, term interface{}) error {
	return clownfish.TrapErr(func() {
		self := (*C.lucy_Indexer)(clownfish.Unwrap(obj, "obj"))
		fieldC := (*C.cfish_String)(clownfish.GoToClownfish(field, unsafe.Pointer(C.CFISH_STRING), false))
		defer C.cfish_decref(unsafe.Pointer(fieldC))
		termC := (*C.cfish_Obj)(clownfish.GoToClownfish(term, unsafe.Pointer(C.CFISH_OBJ), false))
		defer C.cfish_decref(unsafe.Pointer(termC))
		C.LUCY_Indexer_Delete_By_Term(self, fieldC, termC)
	})
}

func (obj *IndexerIMP) DeleteByQuery(query Query) error {
	return clownfish.TrapErr(func() {
		self := (*C.lucy_Indexer)(clownfish.Unwrap(obj, "obj"))
		queryC := (*C.lucy_Query)(clownfish.Unwrap(query, "query"))
		C.LUCY_Indexer_Delete_By_Query(self, queryC)
	})
}

func (obj *IndexerIMP) DeleteByDocID(docID int32) error {
	return clownfish.TrapErr(func() {
		self := (*C.lucy_Indexer)(clownfish.Unwrap(obj, "obj"))
		C.LUCY_Indexer_Delete_By_Doc_ID(self, C.int32_t(docID))
	})
}

func (obj *IndexerIMP) PrepareCommit() error {
	self := ((*C.lucy_Indexer)(unsafe.Pointer(obj.TOPTR())))
	return clownfish.TrapErr(func() {
		C.LUCY_Indexer_Prepare_Commit(self)
	})
}

func (obj *IndexerIMP) Commit() error {
	self := ((*C.lucy_Indexer)(unsafe.Pointer(obj.TOPTR())))
	return clownfish.TrapErr(func() {
		C.LUCY_Indexer_Commit(self)
	})
}

func OpenBackgroundMerger(index interface{}, manager IndexManager) (bgm BackgroundMerger, err error) {
	err = clownfish.TrapErr(func() {
		indexC := (*C.cfish_Obj)(clownfish.GoToClownfish(index, unsafe.Pointer(C.CFISH_OBJ), false))
		defer C.cfish_decref(unsafe.Pointer(indexC))
		managerC := (*C.lucy_IndexManager)(clownfish.UnwrapNullable(manager))
		cfObj := C.lucy_BGMerger_new(indexC, managerC)
		bgm = WRAPBackgroundMerger(unsafe.Pointer(cfObj))
	})
	return bgm, err
}

func (bgm *BackgroundMergerIMP) PrepareCommit() error {
	return clownfish.TrapErr(func() {
		self := (*C.lucy_BackgroundMerger)(clownfish.Unwrap(bgm, "bgm"))
		C.LUCY_BGMerger_Prepare_Commit(self)
	})
}

func (bgm *BackgroundMergerIMP) Commit() error {
	return clownfish.TrapErr(func() {
		self := (*C.lucy_BackgroundMerger)(clownfish.Unwrap(bgm, "bgm"))
		C.LUCY_BGMerger_Commit(self)
	})
}

func (im *IndexManagerIMP) WriteMergeData(cutoff int64) error {
	return clownfish.TrapErr(func() {
		self := (*C.lucy_IndexManager)(clownfish.Unwrap(im, "im"))
		C.LUCY_IxManager_Write_Merge_Data(self, C.int64_t(cutoff))
	})
}

func (im *IndexManagerIMP) ReadMergeData() (retval map[string]interface{}, err error) {
	err = clownfish.TrapErr(func() {
		self := (*C.lucy_IndexManager)(clownfish.Unwrap(im, "im"))
		retvalC := C.LUCY_IxManager_Read_Merge_Data(self)
		if retvalC != nil {
			defer C.cfish_decref(unsafe.Pointer(retvalC))
			retval = clownfish.ToGo(unsafe.Pointer(retvalC)).(map[string]interface{})
		}
	})
	return retval, err
}

func (im *IndexManagerIMP) RemoveMergeData() error {
	return clownfish.TrapErr(func() {
		self := (*C.lucy_IndexManager)(clownfish.Unwrap(im, "im"))
		C.LUCY_IxManager_Remove_Merge_Data(self)
	})
}

func (im *IndexManagerIMP) MakeSnapshotFilename() (retval string, err error) {
	err = clownfish.TrapErr(func() {
		self := (*C.lucy_IndexManager)(clownfish.Unwrap(im, "im"))
		retvalC := C.LUCY_IxManager_Make_Snapshot_Filename(self)
		if retvalC != nil {
			defer C.cfish_decref(unsafe.Pointer(retvalC))
			retval = clownfish.ToGo(unsafe.Pointer(retvalC)).(string)
		}
	})
	return retval, err
}

func (im *IndexManagerIMP) Recycle(reader PolyReader, delWriter DeletionsWriter,
	cutoff int64, optimize bool) (retval []SegReader, err error) {
	err = clownfish.TrapErr(func() {
		self := (*C.lucy_IndexManager)(clownfish.Unwrap(im, "im"))
		readerC := (*C.lucy_PolyReader)(clownfish.Unwrap(reader, "reader"))
		delWriterC := (*C.lucy_DeletionsWriter)(clownfish.Unwrap(delWriter, "delWriter"))
		vec := C.LUCY_IxManager_Recycle(self, readerC, delWriterC,
			C.int64_t(cutoff), C.bool(optimize))
		if vec != nil {
			defer C.cfish_decref(unsafe.Pointer(vec))
			size := int(C.CFISH_Vec_Get_Size(vec))
			retval = make([]SegReader, size)
			for i := 0; i < size; i++ {
				elem := C.CFISH_Vec_Fetch(vec, C.size_t(i))
				retval[i] = WRAPSegReader(unsafe.Pointer(C.cfish_incref(unsafe.Pointer(elem))))
			}
		}
	})
	return retval, err
}

func NewTermVector(field, text string, positions, startOffsets, endOffsets []int32) TermVector {
	fieldC := (*C.cfish_String)(clownfish.GoToClownfish(field, unsafe.Pointer(C.CFISH_STRING), false))
	textC := (*C.cfish_String)(clownfish.GoToClownfish(text, unsafe.Pointer(C.CFISH_STRING), false))
	defer C.cfish_decref(unsafe.Pointer(fieldC))
	defer C.cfish_decref(unsafe.Pointer(textC))
	posits := NewI32Array(positions)
	starts := NewI32Array(startOffsets)
	ends := NewI32Array(endOffsets)
	positsC := (*C.lucy_I32Array)(clownfish.Unwrap(posits, "posits"))
	startsC := (*C.lucy_I32Array)(clownfish.Unwrap(starts, "starts"))
	endsC := (*C.lucy_I32Array)(clownfish.Unwrap(ends, "ends"))
	retvalC := C.lucy_TV_new(fieldC, textC, positsC, startsC, endsC)
	return WRAPTermVector(unsafe.Pointer(retvalC))
}

func (tv *TermVectorIMP) GetPositions() []int32 {
	self := (*C.lucy_TermVector)(clownfish.Unwrap(tv, "tv"))
	return i32ArrayToSlice(C.LUCY_TV_Get_Positions(self))
}

func (tv *TermVectorIMP) GetStartOffsets() []int32 {
	self := (*C.lucy_TermVector)(clownfish.Unwrap(tv, "tv"))
	return i32ArrayToSlice(C.LUCY_TV_Get_Start_Offsets(self))
}

func (tv *TermVectorIMP) GetEndOffsets() []int32 {
	self := (*C.lucy_TermVector)(clownfish.Unwrap(tv, "tv"))
	return i32ArrayToSlice(C.LUCY_TV_Get_End_Offsets(self))
}

func (s *SnapshotIMP) List() []string {
	self := (*C.lucy_Snapshot)(clownfish.Unwrap(s, "s"))
	retvalC := C.LUCY_Snapshot_List(self)
	defer C.cfish_decref(unsafe.Pointer(retvalC))
	return vecToStringSlice(retvalC)
}

func (s *SnapshotIMP) ReadFile(folder Folder, path string) (Snapshot, error) {
	err := clownfish.TrapErr(func() {
		self := (*C.lucy_Snapshot)(clownfish.Unwrap(s, "s"))
		folderC := (*C.lucy_Folder)(clownfish.Unwrap(folder, "folder"))
		pathC := (*C.cfish_String)(clownfish.GoToClownfish(path, unsafe.Pointer(C.CFISH_STRING), false))
		defer C.cfish_decref(unsafe.Pointer(pathC))
		C.LUCY_Snapshot_Read_File(self, folderC, pathC)
	})
	return s, err
}

func (s *SnapshotIMP) WriteFile(folder Folder, path string) error {
	return clownfish.TrapErr(func() {
		self := (*C.lucy_Snapshot)(clownfish.Unwrap(s, "s"))
		folderC := (*C.lucy_Folder)(clownfish.Unwrap(folder, "folder"))
		pathC := (*C.cfish_String)(clownfish.GoToClownfish(path, unsafe.Pointer(C.CFISH_STRING), false))
		defer C.cfish_decref(unsafe.Pointer(pathC))
		C.LUCY_Snapshot_Write_File(self, folderC, pathC)
	})
}

func (s *SegmentIMP) WriteFile(folder Folder) error {
	return clownfish.TrapErr(func() {
		self := (*C.lucy_Segment)(clownfish.Unwrap(s, "s"))
		folderC := (*C.lucy_Folder)(clownfish.Unwrap(folder, "folder"))
		C.LUCY_Seg_Write_File(self, folderC)
	})
}

func (s *SegmentIMP) ReadFile(folder Folder) error {
	return clownfish.TrapErr(func() {
		self := (*C.lucy_Segment)(clownfish.Unwrap(s, "s"))
		folderC := (*C.lucy_Folder)(clownfish.Unwrap(folder, "folder"))
		C.LUCY_Seg_Read_File(self, folderC)
	})
}

func (s *SortCacheIMP) Value(ord int32) (retval interface{}, err error) {
	err = clownfish.TrapErr(func() {
		self := (*C.lucy_SortCache)(clownfish.Unwrap(s, "s"))
		retvalCF := C.LUCY_SortCache_Value(self, C.int32_t(ord))
		defer C.cfish_decref(unsafe.Pointer(retvalCF))
		retval = clownfish.ToGo(unsafe.Pointer(retvalCF))
	})
	return retval, err
}

func (s *SortCacheIMP) Ordinal(docId int32) (retval int32, err error) {
	err = clownfish.TrapErr(func() {
		self := (*C.lucy_SortCache)(clownfish.Unwrap(s, "s"))
		retvalCF := C.LUCY_SortCache_Ordinal(self, C.int32_t(docId))
		retval = int32(retvalCF)
	})
	return retval, err
}

func (s *SortCacheIMP) Find(term interface{}) (retval int32, err error) {
	err = clownfish.TrapErr(func() {
		self := (*C.lucy_SortCache)(clownfish.Unwrap(s, "s"))
		termCF := (*C.cfish_Obj)(clownfish.GoToClownfish(term, unsafe.Pointer(C.CFISH_OBJ), true))
		defer C.cfish_decref(unsafe.Pointer(termCF))
		retvalCF := C.LUCY_SortCache_Find(self, termCF)
		retval = int32(retvalCF)
	})
	return retval, err
}
