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
import "os"

func TestIndexerAddDoc(t *testing.T) {
	schema := createTestSchema()
	index := NewRAMFolder("")
	indexer, _ := OpenIndexer(&OpenIndexerArgs{
		Create: true,
		Index:  index,
		Schema: schema,
	})
	indexer.AddDoc(&testDoc{Content: "foo"})
	indexer.AddDoc(map[string]interface{}{"content": "foo"})
	doc := NewDoc(0)
	doc.Store("content", "foo")
	indexer.AddDoc(doc)
	indexer.Commit()
	searcher, _ := OpenIndexSearcher(index)
	if got := searcher.DocFreq("content", "foo"); got != 3 {
		t.Errorf("Didn't index all docs -- DocMax: %d", got)
	}
}

func TestIndexerAddIndex(t *testing.T) {
	var err error
	origIndex := "_test_go_indexer_add_index"
	defer os.RemoveAll(origIndex)
	schema := createTestSchema()
	indexer, err := OpenIndexer(&OpenIndexerArgs{
		Create: true,
		Index:  origIndex,
		Schema: schema,
	})
	if err != nil {
		t.Errorf("OpenIndexer: %v", err)
		return
	}
	err = indexer.AddDoc(&testDoc{Content: "foo"})
	if err != nil {
		t.Errorf("AddDoc: %v", err)
	}
	err = indexer.Commit()
	if err != nil {
		t.Errorf("Commit FS index: %v", err)
	}

	indexer, _ = OpenIndexer(&OpenIndexerArgs{
		Create: true,
		Index:  NewRAMFolder(""),
		Schema: schema,
	})
	err = indexer.AddIndex(origIndex)
	if err != nil {
		t.Errorf("AddIndex: %v", err)
	}
	err = indexer.Commit()
	if err != nil {
		t.Errorf("AddIndex: %v", err)
	}
}

func TestIndexerDeletions(t *testing.T) {
	index := createTestIndex("foo", "bar", "baz", "gazi")
	indexer, _ := OpenIndexer(&OpenIndexerArgs{Index: index})
	err := indexer.DeleteByTerm("content", "foo")
	if err != nil {
		t.Errorf("DeleteByTerm: %v", err)
	}
	indexer.DeleteByQuery(NewTermQuery("content", "bar"))
	if err != nil {
		t.Errorf("DeleteByQuery: %v", err)
	}
	indexer.DeleteByDocID(3)
	if err != nil {
		t.Errorf("DeleteByDocID: %v", err)
	}
	err = indexer.Commit()
	if err != nil {
		t.Errorf("Commit: %v", err)
	}
	searcher, _ := OpenIndexSearcher(index)
	if count := searcher.GetReader().DocCount(); count != 1 {
		t.Errorf("Some deletions didn't go through (count=%d)", count)
	}
}

func TestIndexerMisc(t *testing.T) {
	var err error
	index := createTestIndex("foo", "bar", "baz")
	indexer, _ := OpenIndexer(&OpenIndexerArgs{Index: index})
	if _, ok := indexer.GetSchema().(Schema); !ok {
		t.Errorf("GetSchema")
	}
	if _, ok := indexer.GetStockDoc().(Doc); !ok {
		t.Errorf("GetStockDoc")
	}
	if _, ok := indexer.GetSegWriter().(SegWriter); !ok {
		t.Errorf("GetSegWriter")
	}
	indexer.AddDoc(&testDoc{Content: "gazi"})
	indexer.Optimize()
	err = indexer.PrepareCommit()
	if err != nil {
		t.Errorf("PrepareCommit: %v", err)
	}
	err = indexer.Commit()
	if err != nil {
		t.Errorf("Commit: %v", err)
	}
}
