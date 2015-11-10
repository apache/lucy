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

func TestSimpleBasics(t *testing.T) {
	var err error
	simple, err := OpenSimple(NewRAMFolder(""), "en")
	if simple == nil || err != nil {
		t.Errorf("OpenSimple: %v", err)
	}

	docStruct := &testDoc{Content: "foo"}
	docMap := map[string]interface{}{"Content": "foo"}
	docDoc := NewDoc(0)
	docDoc.Store("Content", "foo")

	err = simple.AddDoc(docStruct)
	if err != nil {
		t.Errorf("AddDoc with struct: %v", err)
	}
	err = simple.AddDoc(docMap)
	if err != nil {
		t.Errorf("AddDoc with map: %v", err)
	}
	err = simple.AddDoc(docDoc)
	if err != nil {
		t.Errorf("AddDoc with Doc: %v", err)
	}

	count, err := simple.Search("foo", 0, 10)
	if count != 3 || err != nil {
		t.Errorf("Search: %d, %v", count, err)
	}
	docStruct.Content = ""
	if !simple.Next(docStruct) || docStruct.Content != "foo" {
		t.Errorf("Next with struct: %v", simple.Error())
	}
	delete(docMap, "Content")
	if !simple.Next(docMap) || docMap["Content"].(string) != "foo" {
		t.Errorf("Next with map: %v", simple.Error())
	}
	docDoc.Store("Content", "")
	if !simple.Next(docDoc) {
		t.Errorf("Next with Doc: %v", simple.Error())
	}
}
