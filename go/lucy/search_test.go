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

func TestMockMatcherBasics(t *testing.T) {
	matcher := newMockMatcher([]int32{42, 43, 100}, []float32{1.5, 1.5, 1.5})
	if got := matcher.Next(); got != 42 {
		t.Error("Next: %d", got)
	}
	if got := matcher.GetDocID(); got != 42 {
		t.Error("GetDocID: %d", got)
	}
	if score := matcher.Score(); score != 1.5 {
		t.Error("Score: %f", score)
	}
	if got := matcher.Advance(50); got != 100 {
		t.Error("Advance: %d", got)
	}
	if got := matcher.Next(); got != 0 {
		t.Error("Next (iteration finished): %d", got)
	}
}

func TestBitVecMatcherBasics(t *testing.T) {
	bv := NewBitVector(0)
	bv.Set(42)
	bv.Set(43)
	bv.Set(100)
	matcher := NewBitVecMatcher(bv)
	if got := matcher.Next(); got != 42 {
		t.Error("Next: %d", got)
	}
	if got := matcher.GetDocID(); got != 42 {
		t.Error("GetDocID: %d", got)
	}
	if got := matcher.Advance(50); got != 100 {
		t.Error("Advance: %d", got)
	}
	if got := matcher.Next(); got != 0 {
		t.Error("Next (iteration finished): %d", got)
	}
}
