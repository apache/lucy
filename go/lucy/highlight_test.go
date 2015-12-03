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

func TestHeatMapBasics(t *testing.T) {
	spans := make([]Span, 2)
	spans[0] = NewSpan(0, 3, 10.0)
	spans[1] = NewSpan(30, 5, 2.0)
	heatMap := NewHeatMap(spans, 133)
	if _, ok := heatMap.(HeatMap); !ok {
		t.Errorf("NewHeatMap")
	}
	boost := heatMap.calcProximityBoost(spans[0], spans[1])
	if boost <= 0.0 {
		t.Errorf("calcProximityBoost: %f", boost)
	}
	if length := len(heatMap.getSpans()); length <= 0 {
		t.Errorf("getSpans: %d", length)
	}
	boosts := heatMap.generateProximityBoosts(spans)
	if length := len(boosts); length <= 0 {
		t.Errorf("generateProximityBoosts: %d", length)
	}
	flattened := heatMap.flattenSpans(spans)
	if length := len(flattened); length <= 0 {
		t.Errorf("flattenSpans: %d", length)
	}
}

func TestHighlighterBasics(t *testing.T) {
	folder := createTestIndex("foo bar baz")
	searcher := NewIndexSearcher(folder)
	hl := NewHighlighter(searcher, "bar", "content", 200)
	found :=  "<strong>bar</strong>"
	if got := hl.Highlight("bar"); got != found {
		t.Errorf("Highlight: '%s'", got)
	}
	doc, _ := searcher.FetchDoc(1)
	if got := hl.CreateExcerpt(doc); !strings.Contains(got, found) {
		t.Errorf("CreateExcerpt: '%s'", got)
	}
	phi := "\u03a6";
	encodedPhi := "&#934;";
	if got := hl.Encode(phi); got != encodedPhi {
		t.Errorf("Encode: '%v'", got)
	}
}

func TestHighlighterAccessors(t *testing.T) {
	folder := createTestIndex("foo bar baz")
	searcher := NewIndexSearcher(folder)
	hl := NewHighlighter(searcher, "bar", "content", 200)
	if field := hl.GetField(); field != "content" {
		t.Errorf("GetField: %v", field)
	}
	if _, ok := hl.GetSearcher().(Searcher); !ok {
		t.Errorf("GetSearcher")
	}
	barQuery := NewTermQuery("content", "bar")
	if got := hl.GetQuery(); !barQuery.Equals(got) {
		t.Errorf("GetQuery: %T %v", got, got)
	}
	if _, ok := hl.GetCompiler().(Compiler); !ok {
		t.Errorf("GetCompiler")
	}
	if got := hl.GetExcerptLength(); got != 200 {
		t.Errorf("GetExcerptLength: %d", got)
	}
	hl.SetPreTag("<blink>")
	if got := hl.GetPreTag(); got != "<blink>" {
		t.Errorf("Set/GetPreTag: %s", got)
	}
	hl.SetPostTag("</blink>")
	if got := hl.GetPostTag(); got != "</blink>" {
		t.Errorf("Set/GetPostTag: %s", got)
	}
}
