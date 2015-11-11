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

import "testing"
import "strings"
import "reflect"
import "git-wip-us.apache.org/repos/asf/lucy-clownfish.git/runtime/go/clownfish"

func checkQuerySerialize(t *testing.T, query Query) {
	folder := NewRAMFolder("")
	outStream, _ := folder.OpenOut("foo")
	query.serialize(outStream)
	outStream.Close()
	inStream, _ := folder.OpenIn("foo")
	dupe := clownfish.GetClass(query).MakeObj().(Query).deserialize(inStream)
	if !query.Equals(dupe) {
		t.Errorf("Unsuccessful serialization round trip -- expected '%v', got '%v'",
				 query.ToString(), dupe.ToString())
	}
}

func checkQueryDumpLoad(t *testing.T, query Query) {
	dupe := clownfish.GetClass(query).MakeObj().(Query)
	dupe = dupe.Load(query.Dump()).(Query)
	if !query.Equals(dupe) {
		t.Errorf("Unsuccessful Dump/Load round trip -- expected '%v', got '%v'",
				 query.ToString(), dupe.ToString())
	}
}

func checkQueryEquals(t *testing.T, query Query) {
	if !query.Equals(query) {
		t.Error("Equals self")
	}
	if query.Equals("blah") {
		t.Error("Equals against Go string")
	}
}

func checkQueryMakeCompiler(t *testing.T, query Query) {
	index := createTestIndex("foo", "bar", "baz")
	searcher, _ := OpenIndexSearcher(index)
	compiler := query.MakeCompiler(searcher, 1.0, false)
	if got, ok := compiler.(Compiler); !ok {
		t.Error("MakeCompiler failed: got '%v'", got)
	}
}

// Test whether ToString() yields a string which contains "foo".
func checkQueryToStringHasFoo(t *testing.T, query Query) {
	if got := query.ToString(); !strings.Contains(got, "foo") {
		t.Errorf("Unexpected stringification: '%v'", got)
	}
}

func TestTermQueryMisc(t *testing.T) {
	query := NewTermQuery("content", "foo")
	checkQuerySerialize(t, query)
	checkQueryDumpLoad(t, query)
	checkQueryEquals(t, query)
	checkQueryMakeCompiler(t, query)
	checkQueryToStringHasFoo(t, query)
}

func TestTermQueryAccessors(t *testing.T) {
	query := NewTermQuery("content", "foo")
	if got := query.GetField(); got != "content" {
		t.Errorf("Expected 'content', got '%v'", got)
	}
	if got := query.GetTerm().(string); got != "foo" {
		t.Errorf("Expected 'foo', got '%v'", got)
	}
}

func TestTermCompilerMisc(t *testing.T) {
	folder := createTestIndex("foo", "bar", "baz")
	searcher, _ := OpenIndexSearcher(folder)
	query := NewTermQuery("content", "foo")
	compiler := NewTermCompiler(query, searcher, 1.0)
	checkQuerySerialize(t, compiler) 
	checkQueryEquals(t, compiler)
	checkQueryToStringHasFoo(t, compiler)
}

func TestTermCompilerWeighting(t *testing.T) {
	index := createTestIndex("foo", "bar", "baz")
	searcher, _ := OpenIndexSearcher(index)
	query := NewTermQuery("content", "foo")
	compiler := NewTermCompiler(query, searcher, 1.0)
	_ = compiler.SumOfSquaredWeights()
	_ = compiler.GetWeight()
	compiler.ApplyNormFactor(10.0)
}

func TestPhraseQueryMisc(t *testing.T) {
	terms := []interface{}{"foo", "bar"}
	query := NewPhraseQuery("content", terms)
	checkQuerySerialize(t, query)
	checkQueryDumpLoad(t, query)
	checkQueryEquals(t, query)
	checkQueryMakeCompiler(t, query)
	checkQueryToStringHasFoo(t, query)
}

func TestPhraseQueryAccessors(t *testing.T) {
	terms := []interface{}{"foo", "bar"}
	query := NewPhraseQuery("content", terms)
	if field := query.GetField(); field != "content" {
		t.Errorf("Expected 'content', got '%v'", field)
	}
	if got := query.GetTerms(); !reflect.DeepEqual(terms, got) {
		t.Errorf("Expected '%v', got '%v'", terms, got)
	}
}

func TestPhraseCompilerMisc(t *testing.T) {
	folder := createTestIndex("foo", "bar", "baz")
	searcher, _ := OpenIndexSearcher(folder)
	terms := []interface{}{"foo", "bar"}
	query := NewPhraseQuery("content", terms)
	compiler := NewPhraseCompiler(query, searcher, 1.0)
	checkQuerySerialize(t, compiler) 
	checkQueryEquals(t, compiler)
	checkQueryToStringHasFoo(t, compiler)
}

func TestPhraseCompilerWeighting(t *testing.T) {
	index := createTestIndex("foo", "bar", "baz")
	searcher, _ := OpenIndexSearcher(index)
	terms := []interface{}{"foo", "bar"}
	query := NewPhraseQuery("content", terms)
	compiler := NewPhraseCompiler(query, searcher, 1.0)
	_ = compiler.SumOfSquaredWeights()
	_ = compiler.GetWeight()
	compiler.ApplyNormFactor(10.0)
}

func TestANDQueryBasics(t *testing.T) {
	children := []Query{
		NewTermQuery("content", "foo"),
		NewTermQuery("content", "bar"),
	}
	query := NewANDQuery(children)
	checkQuerySerialize(t, query)
	checkQueryDumpLoad(t, query)
	checkQueryEquals(t, query)
	checkQueryMakeCompiler(t, query)
	checkQueryToStringHasFoo(t, query)
}

func TestORQueryBasics(t *testing.T) {
	children := []Query{
		NewTermQuery("content", "foo"),
		NewTermQuery("content", "bar"),
	}
	query := NewORQuery(children)
	checkQuerySerialize(t, query)
	checkQueryDumpLoad(t, query)
	checkQueryEquals(t, query)
	checkQueryMakeCompiler(t, query)
	checkQueryToStringHasFoo(t, query)
}

func TestReqOptQueryBasics(t *testing.T) {
	req := NewTermQuery("content", "foo")
	opt := NewTermQuery("content", "bar")
	query := NewRequiredOptionalQuery(req, opt)
	checkQuerySerialize(t, query)
	checkQueryDumpLoad(t, query)
	checkQueryEquals(t, query)
	checkQueryMakeCompiler(t, query)
	checkQueryToStringHasFoo(t, query)
}

func TestReqOptQueryAccessors(t *testing.T) {
	req := NewTermQuery("content", "foo")
	opt := NewTermQuery("content", "bar")
	query := NewRequiredOptionalQuery(req, opt)
	if query.GetRequiredQuery().TOPTR() != req.TOPTR() {
		t.Errorf("GetRequiredQuery")
	}
	if query.GetOptionalQuery().TOPTR() != opt.TOPTR() {
		t.Errorf("GetOptionalQuery")
	}
}

func TestNOTQueryBasics(t *testing.T) {
	negated := NewTermQuery("content", "foo")
	query := NewNOTQuery(negated)
	checkQuerySerialize(t, query)
	checkQueryDumpLoad(t, query)
	checkQueryEquals(t, query)
	checkQueryMakeCompiler(t, query)
	checkQueryToStringHasFoo(t, query)
}

func TestNOTQueryAccessors(t *testing.T) {
	negated := NewTermQuery("content", "foo")
	query := NewNOTQuery(negated)
	if query.GetNegatedQuery().TOPTR() != negated.TOPTR() {
		t.Errorf("GetNegatedQuery")
	}
}

func TestMatchAllQueryBasics(t *testing.T) {
	query := NewMatchAllQuery()
	checkQuerySerialize(t, query)
	checkQueryDumpLoad(t, query)
	checkQueryEquals(t, query)
	checkQueryMakeCompiler(t, query)
}

func TestNOMatchQueryBasics(t *testing.T) {
	query := NewNoMatchQuery()
	checkQuerySerialize(t, query)
	checkQueryDumpLoad(t, query)
	checkQueryEquals(t, query)
	checkQueryMakeCompiler(t, query)
}

func TestRangeQueryBasics(t *testing.T) {
	query := NewRangeQuery("content", "fab", "foo", true, true)
	checkQuerySerialize(t, query)
	checkQueryDumpLoad(t, query)
	checkQueryEquals(t, query)
	checkQueryMakeCompiler(t, query)
	checkQueryToStringHasFoo(t, query)
}

func TestLeafQueryBasics(t *testing.T) {
	query := NewLeafQuery("content", "foo")
	checkQuerySerialize(t, query)
	checkQueryDumpLoad(t, query)
	checkQueryEquals(t, query)
	checkQueryToStringHasFoo(t, query)
}

func checkMatcher(t *testing.T, matcher Matcher, supportsScore bool) {
	if got := matcher.Next(); got != 42 {
		t.Error("Next: %d", got)
	}
	if got := matcher.GetDocID(); got != 42 {
		t.Error("GetDocID: %d", got)
	}
	if supportsScore {
		if score := matcher.Score(); score != 2 {
			t.Error("Score: %f", score)
		}
	}
	if got := matcher.Advance(50); got != 100 {
		t.Error("Advance: %d", got)
	}
	if got := matcher.Next(); got != 0 {
		t.Error("Next (iteration finished): %d", got)
	}
}

func TestMockMatcherBasics(t *testing.T) {
	matcher := newMockMatcher([]int32{42, 43, 100}, []float32{2,2,2})
	checkMatcher(t, matcher, true)
}

func TestBitVecMatcherBasics(t *testing.T) {
	bv := NewBitVector(0)
	bv.Set(42)
	bv.Set(43)
	bv.Set(100)
	matcher := NewBitVecMatcher(bv)
	checkMatcher(t, matcher, false)
}

func TestANDMatcherBasics(t *testing.T) {
	a := newMockMatcher([]int32{42, 43, 99, 100}, []float32{1,1,1,1})
	b := newMockMatcher([]int32{1, 42, 43, 100}, []float32{1,1,1,1})
	matcher := NewANDMatcher([]Matcher{a, b}, nil)
	checkMatcher(t, matcher, true)
}

func TestORMatcherBasics(t *testing.T) {
	a := newMockMatcher([]int32{42, 43}, nil)
	b := newMockMatcher([]int32{42, 100}, nil)
	matcher := NewORMatcher([]Matcher{a, b})
	checkMatcher(t, matcher, false)
}

func TestORScorerBasics(t *testing.T) {
	a := newMockMatcher([]int32{42, 43}, []float32{1,1})
	b := newMockMatcher([]int32{42, 100}, []float32{1,1})
	matcher := NewORScorer([]Matcher{a, b}, nil)
	checkMatcher(t, matcher, true)
}

func TestReqOptMatcherBasics(t *testing.T) {
	req := newMockMatcher([]int32{42, 43, 100}, []float32{1,1,1})
	opt := newMockMatcher([]int32{1, 42, 99, 100}, []float32{1,1,1,1})
	matcher := NewRequiredOptionalMatcher(nil, req, opt)
	checkMatcher(t, matcher, true)
}

func TestNOTMatcherBasics(t *testing.T) {
	a := newMockMatcher([]int32{1, 42, 43, 99, 100}, nil)
	b := newMockMatcher([]int32{1, 99}, nil)
	notB := NewNOTMatcher(b, 999)
	matcher := NewANDMatcher([]Matcher{a, notB}, nil)
	checkMatcher(t, matcher, false)
}

func TestSeriesMatcherBasics(t *testing.T) {
	a := newMockMatcher([]int32{42}, nil)
	b := newMockMatcher([]int32{1, 4}, nil)
	c := newMockMatcher([]int32{20}, nil)
	matcher := NewSeriesMatcher([]Matcher{a, b, c}, []int32{0, 42, 80})
	checkMatcher(t, matcher, false)
}

func TestMatchAllMatcherBasics(t *testing.T) {
	matcher := NewMatchAllMatcher(1.5, 42)
	matcher.Next()
	if docID := matcher.Next(); docID != 2 {
		t.Errorf("Unexpected return value for Next: %d", docID)
	}
	if docID := matcher.GetDocID(); docID != 2 {
		t.Errorf("Unexpected return value for GetDocID: %d", docID)
	}
	if docID := matcher.Advance(42); docID != 42 {
		t.Errorf("Advance returned %d", docID)
	}
	if score := matcher.Score(); score != 1.5 {
		t.Errorf("Unexpected score: %f", score)
	}
	if matcher.Next() != 0 {
		t.Error("Matcher should be exhausted")
	}
}

func TestNoMatchMatcherBasics(t *testing.T) {
	matcher := NewNoMatchMatcher()
	if matcher.Next() != 0 {
		t.Error("Next should return false")
	}
	matcher = NewNoMatchMatcher()
	if matcher.Advance(3) != 0 {
		t.Error("Advance should return false")
	}
}

func TestRangeMatcherBasics(t *testing.T) {
	index := createTestIndex("d", "c", "b", "a", "a", "a", "a")
	searcher, _ := OpenIndexSearcher(index)
	segReaders := searcher.GetReader().SegReaders()
	segReader := segReaders[0].(SegReader)
	sortReader := segReader.Obtain("Lucy::Index::SortReader").(SortReader)
	sortCache := sortReader.fetchSortCache("content")
	matcher := NewRangeMatcher(0, 0, sortCache, segReader.DocMax())
	if docID := matcher.Next(); docID != 4 {
		t.Errorf("Next: %d", docID)
	}
	if docID := matcher.GetDocID(); docID != 4 {
		t.Errorf("GetDocID: %d", docID)
	}
	if score := matcher.Score(); score != 0.0 {
		t.Errorf("Score: %f", score)
	}
	if docID := matcher.Advance(7); docID != 7 {
		t.Errorf("Advance: %d", docID)
	}
	if docID := matcher.Next(); docID != 0 {
		t.Errorf("Matcher should be exhausted: %d", docID)
	}
}

func TestTopDocsBasics(t *testing.T) {
	matchDocs := []MatchDoc{
		NewMatchDoc(42, 2.0, nil),
		NewMatchDoc(100, 3.0, nil),
	}
	td := NewTopDocs(matchDocs, 50)
	td.setTotalHits(20)
	if totalHits := td.getTotalHits(); totalHits != 20 {
		t.Errorf("Expected 20 total hits, got %d", totalHits)
	}
	td.SetMatchDocs(matchDocs)
	fetched := td.GetMatchDocs()
	if docID := fetched[0].getDocID(); docID != 42 {
		t.Errorf("Set/Get MatchDocs expected 42, got %d", docID)
	}

	folder := NewRAMFolder("")
	outstream, _ := folder.OpenOut("foo")
	td.serialize(outstream)
	outstream.Close()
	inStream, _ := folder.OpenIn("foo")
	dupe := clownfish.GetClass(td).MakeObj().(TopDocs).deserialize(inStream)
	if dupe.getTotalHits() != td.getTotalHits() {
		t.Errorf("Failed round-trip serializetion of TopDocs")
	}
}

type simpleTestDoc struct {
	Content string
}

func TestHitsBasics(t *testing.T) {
	index := createTestIndex("a", "b")
	searcher, _ := OpenIndexSearcher(index)
	topDocs, _ := searcher.topDocs(NewTermQuery("content", "a"), 10, nil)
	hits := NewHits(searcher, topDocs, 0)
	if got := hits.TotalHits(); got != topDocs.getTotalHits() {
		t.Errorf("TotalHits is off: %d", got)
	}
	var doc simpleTestDoc
	if !hits.Next(&doc) {
		t.Error("Hits.Next")
	}
	if doc.Content != "a" {
		t.Errorf("Bad doc content after Next: %s", doc.Content)
	}
	if hits.Next(&doc) {
		t.Error("Hits iterator should be exhausted");
	}
	if err := hits.Error(); err != nil {
		t.Error("Hits.Error() not nil: %v", err)
	}
}

func TestHitsNext(t *testing.T) {
	index := createTestIndex("a x", "a y", "a z", "b")
	searcher, _ := OpenIndexSearcher(index)
	hits, _ := searcher.Hits("a", 0, 10, nil)
	docDoc := NewHitDoc(0, -1.0)
	docStruct := &simpleTestDoc{}
	docMap := make(map[string]interface{})
	if !hits.Next(docDoc) || !hits.Next(docStruct) || !hits.Next(docMap) {
		t.Errorf("Hits.Next returned false: %v", hits.Error())
	}
	if hits.Next(&simpleTestDoc{}) {
		t.Error("Hits iterator should be exhausted");
	}
	if docDoc.Extract("content").(string) != "a x" {
		t.Error("Next with Doc object yielded bad data")
	}
	if docStruct.Content != "a y" {
		t.Error("Next with struct yielded bad data")
	}
	if docMap["content"].(string) != "a z" {
		t.Error("Next with map yielded bad data")
	}
}

func TestSortSpecBasics(t *testing.T) {
	folder := NewRAMFolder("")
	schema := NewSchema()
	fieldType := NewFullTextType(NewStandardTokenizer())
	fieldType.SetSortable(true)
	schema.SpecField("content", fieldType)
	args := &OpenIndexerArgs{Index: folder, Schema: schema, Create: true}
	indexer, err := OpenIndexer(args)
	if err != nil {
		panic(err)
	}
	for _, fieldVal := range []string{"a b", "a a"} {
		indexer.AddDoc(&simpleTestDoc{fieldVal})
	}
	indexer.Commit()

	rules := []SortRule{
		NewFieldSortRule("content", false),
	}
	sortSpec := NewSortSpec(rules)
	searcher, _ := OpenIndexSearcher(folder)
	hits, _ := searcher.Hits("a", 0, 1, sortSpec)
	var doc simpleTestDoc
	hits.Next(&doc)
	if doc.Content != "a a" {
		t.Error("Sort by field value")
	}

	outstream, _ := folder.OpenOut("foo")
	sortSpec.serialize(outstream)
	outstream.Close()
	inStream, _ := folder.OpenIn("foo")
	dupe := clownfish.GetClass(sortSpec).MakeObj().(SortSpec).deserialize(inStream)
	if len(dupe.GetRules()) != len(rules) {
		t.Errorf("Failed round-trip serializetion of SortSpec")
	}
}

func TestHitQueueBasics(t *testing.T) {
	hitQ := NewHitQueue(nil, nil, 1)
	fortyTwo := NewMatchDoc(42, 1.0, nil)
	fortyThree := NewMatchDoc(43, 1.0, nil)
	if !hitQ.lessThan(fortyThree, fortyTwo) {
		t.Error("lessThan")
	}
	if !hitQ.insert(fortyTwo) {
		t.Error("insert")
	}
	if hitQ.getSize() != 1 {
		t.Error("getSize")
	}
	if bumped := hitQ.jostle(fortyThree); bumped.(MatchDoc).getDocID() != 43 {
		t.Error("jostle")
	}
	if peeked := hitQ.peek(); peeked.(MatchDoc).getDocID() != 42 {
		t.Error("peek")
	}
	if popped := hitQ.pop(); popped.(MatchDoc).getDocID() != 42 {
		t.Error("pop")
	}
	hitQ.insert(fortyTwo)
	if got := hitQ.popAll(); got[0].(MatchDoc).getDocID() != 42 {
		t.Error("popAll")
	}
}

func TestSpanBasics(t *testing.T) {
	a := NewSpan(42, 1, 0.0)
	b := NewSpan(42, 2, 0.0)
	if !a.Equals(a) {
		t.Error("Equals self")
	}
	if a.Equals(b) {
		t.Error("Equals should return false for non-equal spans")
	}
	if got := a.CompareTo(b); got >= 0 {
		t.Errorf("CompareTo returned %d", got)
	}
	a.SetOffset(21)
	if got := a.GetOffset(); got != 21 {
		t.Errorf("Set/Get offset: %d", got)
	}
	a.SetLength(10)
	if got := a.GetLength(); got != 10 {
		t.Errorf("Set/Get length: %d", got)
	}
	a.SetWeight(1.5)
	if got := a.GetWeight(); got != 1.5 {
		t.Errorf("Set/Get weight: %f", got)
	}
}

func TestBitCollectorBasics(t *testing.T) {
	index := createTestIndex("a", "b", "c", "a")
	searcher, _ := OpenIndexSearcher(index)
	bitVec := NewBitVector(5)
	collector := NewBitCollector(bitVec)
	searcher.Collect(NewTermQuery("content", "a"), collector)
	expected := []bool{false, true, false, false, true, false, false, false}
	if got := bitVec.ToArray(); !reflect.DeepEqual(got,expected) {
		t.Errorf("Unexpected result set: %v", got)
	}
}

func TestOffsetCollectorBasics(t *testing.T) {
	index := createTestIndex("a", "b", "c")
	searcher, _ := OpenIndexSearcher(index)
	bitVec := NewBitVector(64)
	bitColl := NewBitCollector(bitVec)
	offsetColl := NewOffsetCollector(bitColl, 40)
	searcher.Collect(NewTermQuery("content", "b"), offsetColl)
	if got := bitVec.NextHit(0); got != 42 {
		t.Errorf("Unexpected docID: %d", got)
	}
}

func TestSortCollectorBasics(t *testing.T) {
	index := createTestIndex("a", "b", "c", "a")
	searcher, _ := OpenIndexSearcher(index)
	collector := NewSortCollector(nil, nil, 1)
	searcher.Collect(NewTermQuery("content", "a"), collector)
	if totalHits := collector.getTotalHits(); totalHits != 2 {
		t.Errorf("Unexpected TotalHits: %d", totalHits)
	}
	matchDocs := collector.PopMatchDocs()
	if docID := matchDocs[0].getDocID(); docID != 1 {
		t.Errorf("Weird MatchDoc: %d", docID)
	}
}

func TestIndexSearcherMisc(t *testing.T) {
	index := createTestIndex("a", "b", "c", "a a")
	searcher, _ := OpenIndexSearcher(index)
	if got := searcher.DocFreq("content", "a"); got != 2 {
		t.Errorf("DocFreq expected 2, got %d", got)
	}
	if got := searcher.DocMax(); got != 4 {
		t.Errorf("DocMax expected 4, got %d", got)
	}
	if _, ok := searcher.GetReader().(PolyReader); !ok {
		t.Error("GetReader")
	}
	doc, err := searcher.FetchDoc(4)
	if _, ok := doc.(Doc); !ok || err != nil {
		t.Error("FetchDoc: %v", err)
	}
	docVec, err := searcher.fetchDocVec(4)
	if _, ok := docVec.(DocVector); !ok || err != nil {
		t.Error("FetchDocVec: %v", err)
	}
}

func TestIndexSearcherOpenClose(t *testing.T) {
	if _, err := OpenIndexSearcher(NewRAMFolder("")); err == nil {
		t.Error("Open non-existent index")
	}
	if _, err := OpenIndexSearcher(42); err == nil {
		t.Error("Garbage 'index' argument")
	}
	index := createTestIndex("a", "b", "c")
	searcher, _ := OpenIndexSearcher(index)
	searcher.Close()
}

func TestIndexSearcherHits(t *testing.T) {
	index := createTestIndex("a", "b", "c", "a a")
	searcher, _ := OpenIndexSearcher(index)
	if got, _ := searcher.Hits("a", 0, 1, nil); got.TotalHits() != 2 {
		t.Errorf("Hits() with query string: %d", got.TotalHits())
	}
	termQuery := NewTermQuery("content", "a")
	if got, _ := searcher.Hits(termQuery, 0, 1, nil); got.TotalHits() != 2 {
		t.Errorf("Hits() with TermQuery object: %d", got.TotalHits())
	}

	if _, err := searcher.Hits(42, 0, 1, nil); err == nil {
		t.Error("Garbage 'query' argument")
	}
}

func TestIndexSearcherTopDocs(t *testing.T) {
	index := createTestIndex("a", "b")
	searcher, _ := OpenIndexSearcher(index)
	topDocs, err := searcher.topDocs(NewTermQuery("content", "b"), 10, nil)
	if err != nil {
		t.Errorf("topDocs: %v", err)
	}
	matchDocs := topDocs.GetMatchDocs()
	if docID := matchDocs[0].getDocID(); docID != 2 {
		t.Errorf("TopDocs expected 2, got %d", docID)
	}
}

func TestIndexSearcherReadDoc(t *testing.T) {
	index := createTestIndex("a", "b")
	searcher, _ := OpenIndexSearcher(index)
	docDoc := NewHitDoc(0, -1.0)
	docStruct := &simpleTestDoc{}
	docMap := make(map[string]interface{})
	var err error
	err = searcher.ReadDoc(2, docDoc)
	if err != nil {
		t.Errorf("ReadDoc failed with HitDoc: %v", err)
	}
	err = searcher.ReadDoc(2, docStruct)
	if err != nil {
		t.Errorf("ReadDoc failed with struct: %v", err)
	}
	err = searcher.ReadDoc(2, docMap)
	if err != nil {
		t.Errorf("ReadDoc failed with map: %v", err)
	}
	if docDoc.Extract("content").(string) != "b" {
		t.Error("Next with Doc object yielded bad data")
	}
	if docStruct.Content != "b" {
		t.Error("Next with struct yielded bad data")
	}
	if docMap["content"].(string) != "b" {
		t.Error("Next with map yielded bad data")
	}
}

func TestMatchDocBasics(t *testing.T) {
	matchDoc := NewMatchDoc(0, 1.0, nil)
	matchDoc.setDocID(42)
	if got := matchDoc.getDocID(); got != 42 {
		t.Errorf("set/getDocID: %d", got)
	}
	matchDoc.setScore(1.5)
	if got := matchDoc.getScore(); got != 1.5 {
		t.Errorf("set/getScore: %f", got)
	}
	values := []interface{}{"foo", int64(42)}
	matchDoc.setValues(values)
	if got := matchDoc.getValues(); !reflect.DeepEqual(got, values) {
		t.Error("get/setValues")
	}
}

func TestMatchDocSerialization(t *testing.T) {
	values := []interface{}{"foo", int64(42)}
	matchDoc := NewMatchDoc(100, 1.5, values)
	folder := NewRAMFolder("")
	outstream, _ := folder.OpenOut("foo")
	matchDoc.serialize(outstream)
	outstream.Close()
	inStream, _ := folder.OpenIn("foo")
	dupe := clownfish.GetClass(matchDoc).MakeObj().(MatchDoc).deserialize(inStream)
	if got := dupe.getValues(); !reflect.DeepEqual(got, values) {
		t.Errorf("Failed round-trip serializetion of MatchDoc")
	}
}
