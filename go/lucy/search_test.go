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
	outStream := folder.OpenOut("foo")
	query.Serialize(outStream)
	outStream.Close()
	inStream := folder.OpenIn("foo")
	dupe := clownfish.GetClass(query).MakeObj().(Query).Deserialize(inStream)
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
	sortCache := sortReader.FetchSortCache("content")
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
	td.SetTotalHits(20)
	if totalHits := td.GetTotalHits(); totalHits != 20 {
		t.Errorf("Expected 20 total hits, got %d", totalHits)
	}
	td.SetMatchDocs(matchDocs)
	fetched := td.GetMatchDocs()
	if docID := fetched[0].GetDocID(); docID != 42 {
		t.Errorf("Set/Get MatchDocs expected 42, got %d", docID)
	}

	folder := NewRAMFolder("")
	outstream := folder.OpenOut("foo")
	td.Serialize(outstream)
	outstream.Close()
	inStream := folder.OpenIn("foo")
	dupe := clownfish.GetClass(td).MakeObj().(TopDocs).Deserialize(inStream)
	if dupe.GetTotalHits() != td.GetTotalHits() {
		t.Errorf("Failed round-trip serializetion of TopDocs")
	}
}

type simpleTestDoc struct {
	Content string
}

func TestHitsBasics(t *testing.T) {
	index := createTestIndex("a", "b")
	searcher, _ := OpenIndexSearcher(index)
	topDocs := searcher.TopDocs(NewTermQuery("content", "a"), 10, nil)
	hits := NewHits(searcher, topDocs, 0)
	if got := hits.TotalHits(); got != topDocs.GetTotalHits() {
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

	outstream := folder.OpenOut("foo")
	sortSpec.Serialize(outstream)
	outstream.Close()
	inStream := folder.OpenIn("foo")
	dupe := clownfish.GetClass(sortSpec).MakeObj().(SortSpec).Deserialize(inStream)
	if len(dupe.GetRules()) != len(rules) {
		t.Errorf("Failed round-trip serializetion of SortSpec")
	}
}

func TestHitQueueBasics(t *testing.T) {
	hitQ := NewHitQueue(nil, nil, 1)
	fortyTwo := NewMatchDoc(42, 1.0, nil)
	fortyThree := NewMatchDoc(43, 1.0, nil)
	if !hitQ.LessThan(fortyThree, fortyTwo) {
		t.Error("LessThan")
	}
	if !hitQ.Insert(fortyTwo) {
		t.Error("Insert")
	}
	if hitQ.GetSize() != 1 {
		t.Error("GetSize")
	}
	if bumped := hitQ.Jostle(fortyThree); bumped.(MatchDoc).GetDocID() != 43 {
		t.Error("Jostle")
	}
	if peeked := hitQ.Peek(); peeked.(MatchDoc).GetDocID() != 42 {
		t.Error("Peek")
	}
	if popped := hitQ.Pop(); popped.(MatchDoc).GetDocID() != 42 {
		t.Error("Pop")
	}
	hitQ.Insert(fortyTwo)
	if got := hitQ.PopAll(); got[0].(MatchDoc).GetDocID() != 42 {
		t.Error("PopAll")
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
