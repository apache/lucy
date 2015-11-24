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

#define C_LUCY_HITS

#include "Lucy/Search/Collector.h"
#include "Lucy/Search/Collector/SortCollector.h"
#include "Lucy/Search/Hits.h"
#include "Lucy/Search/IndexSearcher.h"
#include "Lucy/Search/Query.h"
#include "Lucy/Search/Compiler.h"
#include "Lucy/Search/Searcher.h"
#include "Lucy/Search/ANDQuery.h"
#include "Lucy/Search/ORQuery.h"
#include "Lucy/Search/ANDMatcher.h"
#include "Lucy/Search/MatchDoc.h"
#include "Lucy/Search/ORMatcher.h"
#include "Lucy/Search/SeriesMatcher.h"
#include "Lucy/Search/SortRule.h"
#include "Lucy/Search/SortSpec.h"
#include "Lucy/Search/TopDocs.h"
#include "Lucy/Document/HitDoc.h"
#include "Lucy/Index/IndexReader.h"
#include "LucyX/Search/MockMatcher.h"
#include "Clownfish/Blob.h"
#include "Clownfish/Hash.h"
#include "Clownfish/HashIterator.h"
#include "Clownfish/Vector.h"

static inline void
float32_set(float *floats, size_t i, float value) {
	floats[i] = value;
}

*/
import "C"
import "unsafe"

import "git-wip-us.apache.org/repos/asf/lucy-clownfish.git/runtime/go/clownfish"

type HitsIMP struct {
	clownfish.ObjIMP
	err error
}

type MatcherIMP struct {
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

// Read data into the supplied doc.
func (s *SearcherIMP) ReadDoc(docID int32, doc interface{}) error {
	self := (*C.lucy_Searcher)(clownfish.Unwrap(s, "s"))
	class := C.cfish_Obj_get_class((*C.cfish_Obj)(unsafe.Pointer(self)))
	if class == C.LUCY_INDEXSEARCHER {
		ixReader := C.LUCY_IxSearcher_Get_Reader((*C.lucy_IndexSearcher)(unsafe.Pointer(self)))
		cfStr := (*C.cfish_String)(clownfish.GoToClownfish("Lucy::Index::DocReader", unsafe.Pointer(C.CFISH_STRING), false))
		defer C.cfish_decref(unsafe.Pointer(cfStr))
		docReader := C.LUCY_IxReader_Fetch(ixReader, cfStr)
		if docReader == nil {
			return clownfish.NewErr("No DocReader available")
		}
		docReaderGo := clownfish.WRAPAny(unsafe.Pointer(C.cfish_incref(unsafe.Pointer(docReader)))).(DocReader)
		return fetchDocFromDocReader(docReaderGo, docID, doc)
	} else {
		return clownfish.NewErr("Support for ReadDoc not implemented")
	}
}

func (s *SearcherIMP) FetchDoc(docID int32) (doc HitDoc, err error) {
	err = clownfish.TrapErr(func() {
		self := (*C.lucy_Searcher)(clownfish.Unwrap(s, "s"))
		docC := C.LUCY_Searcher_Fetch_Doc(self, C.int32_t(docID))
		doc = WRAPHitDoc(unsafe.Pointer(docC))
	})
	return doc, err
}

func (s *SearcherIMP) fetchDocVec(docID int32) (dv DocVector, err error) {
	err = clownfish.TrapErr(func() {
		self := (*C.lucy_Searcher)(clownfish.Unwrap(s, "s"))
		dvC := C.LUCY_Searcher_Fetch_Doc_Vec(self, C.int32_t(docID))
		dv = WRAPDocVector(unsafe.Pointer(dvC))
	})
	return dv, err
}

func (s *SearcherIMP) Close() error {
	return clownfish.TrapErr(func() {
		self := (*C.lucy_Searcher)(clownfish.Unwrap(s, "s"))
		C.LUCY_Searcher_Close(self)
	})
}

func (s *SearcherIMP) Hits(query interface{}, offset uint32, numWanted uint32,
	sortSpec SortSpec) (hits Hits, err error) {
	self := (*C.lucy_Searcher)(clownfish.Unwrap(s, "s"))
	sortSpecC := (*C.lucy_SortSpec)(clownfish.UnwrapNullable(sortSpec))
	queryC := (*C.cfish_Obj)(clownfish.GoToClownfish(query, unsafe.Pointer(C.CFISH_OBJ), false))
	defer C.cfish_decref(unsafe.Pointer(queryC))
	err = clownfish.TrapErr(func() {
		hitsC := C.LUCY_Searcher_Hits(self, queryC,
			C.uint32_t(offset), C.uint32_t(numWanted), sortSpecC)
		hits = WRAPHits(unsafe.Pointer(hitsC))
	})
	return hits, err
}

func (s *SearcherIMP) topDocs(query Query, numWanted uint32,
	sortSpec SortSpec) (topDocs TopDocs, err error) {
	err = clownfish.TrapErr(func() {
		self := (*C.lucy_Searcher)(clownfish.Unwrap(s, "s"))
		sortSpecC := (*C.lucy_SortSpec)(clownfish.UnwrapNullable(sortSpec))
		queryC := (*C.lucy_Query)(clownfish.Unwrap(query, "query"))
		topDocsC := C.LUCY_Searcher_Top_Docs(self, queryC,
			C.uint32_t(numWanted), sortSpecC)
		topDocs = WRAPTopDocs(unsafe.Pointer(topDocsC))
	})
	return topDocs, err
}

type setScorer interface {
	SetScore(float32)
}

func (h *HitsIMP) Next(hit interface{}) bool {
	self := (*C.lucy_Hits)(clownfish.Unwrap(h, "h"))
	ivars := C.lucy_Hits_IVARS(self)
	matchDoc := (*C.lucy_MatchDoc)(unsafe.Pointer(
		C.CFISH_Vec_Fetch(ivars.match_docs, C.size_t(ivars.offset))))
	ivars.offset += 1

	if matchDoc == nil {
		// Bail if there aren't any more *captured* hits.  (There may be
		// more total hits.)
		return false
	} else {
		// Lazily fetch HitDoc, set score.
		searcher := clownfish.WRAPAny(unsafe.Pointer(C.cfish_incref(
			unsafe.Pointer(ivars.searcher)))).(Searcher)
		docID := int32(C.LUCY_MatchDoc_Get_Doc_ID(matchDoc))
		err := searcher.ReadDoc(docID, hit)
		if err != nil {
			h.err = err
			return false
		}
		if ss, ok := hit.(setScorer); ok {
			ss.SetScore(float32(C.LUCY_MatchDoc_Get_Score(matchDoc)))
		}
		return true
	}
}

func (obj *HitsIMP) Error() error {
	return obj.err
}

func NewFieldSortRule(field string, reverse bool) SortRule {
	fieldC := clownfish.GoToClownfish(field, unsafe.Pointer(C.CFISH_STRING), false)
	cfObj := C.lucy_SortRule_new(C.lucy_SortRule_FIELD, (*C.cfish_String)(fieldC), C.bool(reverse))
	return WRAPSortRule(unsafe.Pointer(cfObj))
}

func NewDocIDSortRule(reverse bool) SortRule {
	cfObj := C.lucy_SortRule_new(C.lucy_SortRule_DOC_ID, nil, C.bool(reverse))
	return WRAPSortRule(unsafe.Pointer(cfObj))
}

func NewScoreSortRule(reverse bool) SortRule {
	cfObj := C.lucy_SortRule_new(C.lucy_SortRule_SCORE, nil, C.bool(reverse))
	return WRAPSortRule(unsafe.Pointer(cfObj))
}

func NewSortSpec(rules []SortRule) SortSpec {
	vec := clownfish.NewVector(len(rules))
	for _, rule := range rules {
		vec.Push(rule)
	}
	cfObj := C.lucy_SortSpec_new((*C.cfish_Vector)(clownfish.Unwrap(vec, "rules")))
	return WRAPSortSpec(unsafe.Pointer(cfObj))
}

func (spec *SortSpecIMP) GetRules() []SortRule {
	self := (*C.lucy_SortSpec)(clownfish.Unwrap(spec, "spec"))
	vec := C.LUCY_SortSpec_Get_Rules(self)
	length := int(C.CFISH_Vec_Get_Size(vec))
	slice := make([]SortRule, length)
	for i := 0; i < length; i++ {
		elem := C.cfish_incref(unsafe.Pointer(C.CFISH_Vec_Fetch(vec, C.size_t(i))))
		slice[i] = WRAPSortRule(unsafe.Pointer(elem))
	}
	return slice
}

func NewTopDocs(matchDocs []MatchDoc, totalHits uint32) TopDocs {
	vec := clownfish.NewVector(len(matchDocs))
	for _, matchDoc := range matchDocs {
		vec.Push(matchDoc)
	}
	cfObj := C.lucy_TopDocs_new(((*C.cfish_Vector)(clownfish.Unwrap(vec, "matchDocs"))),
		C.uint32_t(totalHits))
	return WRAPTopDocs(unsafe.Pointer(cfObj))
}

func (td *TopDocsIMP) SetMatchDocs(matchDocs []MatchDoc) {
	self := (*C.lucy_TopDocs)(clownfish.Unwrap(td, "td"))
	vec := clownfish.NewVector(len(matchDocs))
	for _, matchDoc := range matchDocs {
		vec.Push(matchDoc)
	}
	C.LUCY_TopDocs_Set_Match_Docs(self, (*C.cfish_Vector)(clownfish.Unwrap(vec, "matchDocs")))
}

func (td *TopDocsIMP) GetMatchDocs() []MatchDoc {
	self := (*C.lucy_TopDocs)(clownfish.Unwrap(td, "td"))
	vec := C.LUCY_TopDocs_Get_Match_Docs(self)
	length := int(C.CFISH_Vec_Get_Size(vec))
	slice := make([]MatchDoc, length)
	for i := 0; i < length; i++ {
		elem := C.cfish_incref(unsafe.Pointer(C.CFISH_Vec_Fetch(vec, C.size_t(i))))
		slice[i] = WRAPMatchDoc(unsafe.Pointer(elem))
	}
	return slice
}

func (q *QueryIMP) MakeCompiler(searcher Searcher, boost float32,
	subordinate bool) (retval Compiler, err error) {
	err = clownfish.TrapErr(func() {
		self := (*C.lucy_Query)(clownfish.Unwrap(q, "q"))
		searcherCF := (*C.lucy_Searcher)(clownfish.Unwrap(searcher, "searcher"))
		retvalCF := C.LUCY_Query_Make_Compiler(self, searcherCF, C.float(boost), C.bool(subordinate))
		if retvalCF != nil {
			retval = clownfish.WRAPAny(unsafe.Pointer(retvalCF)).(Compiler)
		}
	})
	return retval, err
}

func (c *CompilerIMP) MakeMatcher(reader SegReader, needScore bool) (retval Matcher, err error) {
	err = clownfish.TrapErr(func() {
		self := (*C.lucy_Compiler)(clownfish.Unwrap(c, "c"))
		readerCF := (*C.lucy_SegReader)(clownfish.Unwrap(reader, "reader"))
		retvalCF := C.LUCY_Compiler_Make_Matcher(self, readerCF, C.bool(needScore))
		if retvalCF != nil {
			retval = clownfish.WRAPAny(unsafe.Pointer(retvalCF)).(Matcher)
		}
	})
	if err != nil || retval == nil {
		return nil, err
	}
	return retval, err
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

func (m *MatcherIMP) Next() int32 {
	var retval int32
	m.err = clownfish.TrapErr(func() {
		self := (*C.lucy_Matcher)(clownfish.Unwrap(m, "m"))
		retval = int32(C.LUCY_Matcher_Next(self))
	})
	if m.err != nil {
		return 0
	}
	return retval
}

func (m *MatcherIMP) Error() error {
	return m.err
}

func NewANDMatcher(children []Matcher, sim Similarity) ANDMatcher {
	simC := (*C.lucy_Similarity)(clownfish.UnwrapNullable(sim))
	vec := clownfish.NewVector(len(children))
	for _, child := range children {
		vec.Push(child)
	}
	childrenC := (*C.cfish_Vector)(unsafe.Pointer(vec.TOPTR()))
	cfObj := C.lucy_ANDMatcher_new(childrenC, simC)
	return WRAPANDMatcher(unsafe.Pointer(cfObj))
}

func NewORMatcher(children []Matcher) ORMatcher {
	vec := clownfish.NewVector(len(children))
	for _, child := range children {
		vec.Push(child)
	}
	childrenC := (*C.cfish_Vector)(unsafe.Pointer(vec.TOPTR()))
	cfObj := C.lucy_ORMatcher_new(childrenC)
	return WRAPORMatcher(unsafe.Pointer(cfObj))
}

func NewORScorer(children []Matcher, sim Similarity) ORScorer {
	simC := (*C.lucy_Similarity)(clownfish.UnwrapNullable(sim))
	vec := clownfish.NewVector(len(children))
	for _, child := range children {
		vec.Push(child)
	}
	childrenC := (*C.cfish_Vector)(unsafe.Pointer(vec.TOPTR()))
	cfObj := C.lucy_ORScorer_new(childrenC, simC)
	return WRAPORScorer(unsafe.Pointer(cfObj))
}

func NewSeriesMatcher(matchers []Matcher, offsets []int32) SeriesMatcher {
	vec := clownfish.NewVector(len(matchers))
	for _, child := range matchers {
		vec.Push(child)
	}
	i32arr := NewI32Array(offsets)
	cfObj := C.lucy_SeriesMatcher_new(((*C.cfish_Vector)(clownfish.Unwrap(vec, "matchers"))),
		((*C.lucy_I32Array)(clownfish.Unwrap(i32arr, "offsets"))))
	return WRAPSeriesMatcher(unsafe.Pointer(cfObj))
}

func newMockMatcher(docIDs []int32, scores []float32) MockMatcher {
	docIDsconv := NewI32Array(docIDs)
	docIDsCF := (*C.lucy_I32Array)(unsafe.Pointer(docIDsconv.TOPTR()))
	var blob *C.cfish_Blob = nil
	if scores != nil {
		size := len(scores) * 4
		floats := (*C.float)(C.malloc(C.size_t(size)))
		for i := 0; i < len(scores); i++ {
			C.float32_set(floats, C.size_t(i), C.float(scores[i]))
		}
		blob = C.cfish_Blob_new_steal((*C.char)(unsafe.Pointer(floats)), C.size_t(size))
		defer C.cfish_decref(unsafe.Pointer(blob))
	}
	matcher := C.lucy_MockMatcher_new(docIDsCF, blob)
	return WRAPMockMatcher(unsafe.Pointer(matcher))
}

func (sc *SortCollectorIMP) PopMatchDocs() []MatchDoc {
	self := (*C.lucy_SortCollector)(clownfish.Unwrap(sc, "sc"))
	matchDocsC := C.LUCY_SortColl_Pop_Match_Docs(self)
	defer C.cfish_decref(unsafe.Pointer(matchDocsC))
	length := int(C.CFISH_Vec_Get_Size(matchDocsC))
	slice := make([]MatchDoc, length)
	for i := 0; i < length; i++ {
		elem := C.cfish_incref(unsafe.Pointer(C.CFISH_Vec_Fetch(matchDocsC, C.size_t(i))))
		slice[i] = WRAPMatchDoc(unsafe.Pointer(elem))
	}
	return slice
}
