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

import "git-wip-us.apache.org/repos/asf/lucy-clownfish.git/runtime/go/clownfish"
import "testing"

type testDoc struct {
	Content string
}

// Build a RAM index, using the supplied array of strings as source material.
// The index will have a single field: "content".
func createTestIndex(values ...string) Folder {
	folder := NewRAMFolder("")
	schema := createTestSchema()
	indexerArgs := &OpenIndexerArgs{
		Schema:   schema,
		Index:    folder,
		Create:   true,
	}
	indexer, err := OpenIndexer(indexerArgs)
	if err != nil {
		panic(err)
	}
	defer indexer.Close()

	for _, val := range values {
		err := indexer.AddDoc(&testDoc{val})
		if err != nil {
			panic(err)
		}
	}
	err = indexer.Commit()
	if err != nil {
		panic(err)
	}

	return folder
}

func createTestSchema() Schema {
	schema := NewSchema()
	analyzer := NewStandardTokenizer()
	fieldType := NewFullTextType(analyzer)
	fieldType.SetHighlightable(true)
	fieldType.SetSortable(true)
	schema.SpecField("content", fieldType)
	return schema
}

func TestOpenIndexer(t *testing.T) {
	_, err := OpenIndexer(&OpenIndexerArgs{Index: "notalucyindex"})
	if _, ok := err.(clownfish.Err); !ok {
		t.Error("Didn't catch exception opening indexer")
	}
}
