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
import "reflect"
import "git-wip-us.apache.org/repos/asf/lucy-clownfish.git/runtime/go/clownfish"

func TestDocMisc(t *testing.T) {
	doc := NewDoc(1)
	if got := doc.GetDocID(); got != 1 {
		t.Errorf("GetDocID: %d", got)
	}
	doc.SetDocID(42)
	if got := doc.GetDocID(); got != 42 {
		t.Errorf("Set/GetDocID: %d", got)
	}
	fields := map[string]interface{}{"content": "foo"}
	doc.SetFields(fields)
	if got, ok := doc.Extract("content").(string); !ok || got != "foo" {
		t.Errorf("Extract: %v", got)
	}
	doc.Store("content", "bar")
	retrievedFields := doc.GetFields()
	if got, ok := retrievedFields["content"].(string); !ok || got != "bar" {
		t.Errorf("Store/GetFields: %v", got)
	}
	if got := doc.GetSize(); got != 1 {
		t.Errorf("GetSize: %d", got)
	}
	if got := doc.FieldNames(); !reflect.DeepEqual(got, []string{"content"}) {
		t.Errorf("FieldNames: %v", got)
	}
	checkDocSerialize(t, doc)
	checkdocDumpLoad(t, doc)
}

func TestHitDocMisc(t *testing.T) {
	hitDoc := NewHitDoc(42, 1.5)
	if got := hitDoc.GetScore(); got != 1.5 {
		t.Errorf("GetScore: %f", got)
	}
	hitDoc.SetScore(2.0)
	if got := hitDoc.GetScore(); got != 2.0 {
		t.Errorf("Set/GetScore: %f", got)
	}
	checkDocSerialize(t, hitDoc)
	checkdocDumpLoad(t, hitDoc)
}

func checkDocSerialize(t *testing.T, doc Doc) {
	folder := NewRAMFolder("")
	outStream, _ := folder.OpenOut("foo")
	doc.serialize(outStream)
	outStream.Close()
	inStream, _ := folder.OpenIn("foo")
	dupe := clownfish.GetClass(doc).MakeObj().(Doc).deserialize(inStream)
	if !doc.Equals(dupe) {
		t.Errorf("Unsuccessful serialization round trip -- expected '%v', got '%v'",
				 doc.ToString(), dupe.ToString())
	}
}

func checkdocDumpLoad(t *testing.T, doc Doc) {
	t.Skip("Dump/Load are TODO")
	return
	dupe := clownfish.GetClass(doc).MakeObj().(Doc)
	dupe = dupe.load(doc.dump()).(Doc)
	if !doc.Equals(dupe) {
		t.Errorf("Unsuccessful dump/load round trip -- expected '%v', got '%v'",
				 doc.ToString(), dupe.ToString())
	}
}
