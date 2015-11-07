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

func TestSchemaSpecField(t *testing.T) {
	schema := NewSchema()
	fieldType := NewStringType()
	schema.SpecField("foo", fieldType)
	if got, ok := schema.FetchType("foo").(FieldType); !ok {
		t.Errorf("SpecField failed, type is %T", got)
	}
}

func TestSchemaFetchType(t *testing.T) {
	schema := createTestSchema()
	fieldType := schema.FetchType("content")
	if _, ok := fieldType.(FullTextType); !ok {
		t.Errorf("Unexpected result for FetchType: %T", fieldType)
	}
	if got := schema.FetchType(""); got != nil {
		t.Errorf("Expected nil from FetchType, got %T %v", got, got)
	}
}

func TestSchemaFetchAnalyzer(t *testing.T) {
	schema := createTestSchema()
	analyzer := schema.fetchAnalyzer("content")
	if _, ok := analyzer.(StandardTokenizer); !ok {
		t.Errorf("Unexpected result for fetchAnalyzer: %T", analyzer)
	}
	if got := schema.fetchAnalyzer(""); got != nil {
		t.Errorf("Expected nil from fetchAnalyzer, got %T %v", got, got)
	}
}

func TestSchemaFetchSim(t *testing.T) {
	schema := createTestSchema()
	sim := schema.FetchSim("content")
	if _, ok := sim.(Similarity); !ok {
		t.Errorf("Unexpected result for FetchSim: %T", sim)
	}
	if got := schema.FetchSim(""); got != nil {
		t.Errorf("Expected nil from FetchSim, got %T %v", got, got)
	}
}

func TestSchemaNumFields(t *testing.T) {
	num := createTestSchema().NumFields()
	if num != 1 {
		t.Errorf("Unexpected NumFields result: %d", num)
	}
}

func TestSchemaAllFields(t *testing.T) {
	schema := createTestSchema()
	got := schema.AllFields()
	expected := []string{"content"}
	if !reflect.DeepEqual(got, expected) {
		t.Errorf("Invalid AllFields(): %v", got)
	}
}

func TestSchemaArchitecture(t *testing.T) {
	schema := createTestSchema()
	if arch, ok := schema.Architecture().(Architecture); !ok {
		t.Errorf("Unexpected result for Architecture(): %T", arch)
	}
}

func TestSchemaAccessors(t *testing.T) {
	schema := createTestSchema()
	if sim, ok := schema.GetSimilarity().(Similarity); !ok {
		t.Errorf("Unexpected result for GetSimilarity(): %T", sim)
	}
	if arch, ok := schema.GetArchitecture().(Architecture); !ok {
		t.Errorf("Unexpected result for GetArchitecture(): %T", arch)
	}
}

func TestSchemaDumpLoad(t *testing.T) {
	schema := createTestSchema()
	dupe := schema.load(schema.dump())
	if _, ok := dupe.(Schema); !ok {
		t.Errorf("Failed dump/load round trip produced a %T", dupe)
	}
}

func TestSchemaWrite(t *testing.T) {
	schema := createTestSchema()
	folder := NewRAMFolder("")
	schema.write(folder, "serialized_schema.json")
}

func TestSchemaEat(t *testing.T) {
	cannibal := NewSchema()
	cannibal.eat(createTestSchema())
	if _, ok := cannibal.FetchType("content").(FieldType); !ok {
		t.Error("Failed to eat other Schema")
	}
}

func runFieldTypeTests(t *testing.T, ft FieldType) {
	// Accessors, etc.
	ft.SetBoost(2.0)
	if got := ft.GetBoost(); got != 2.0 {
		t.Errorf("SetBoost/GetBoost: %d", got)
	}
	ft.SetIndexed(false)
	if ft.Indexed() {
		t.Errorf("SetIndexed/Indexed")
	}
	ft.SetStored(false)
	if ft.Stored() {
		t.Errorf("SetStored/Stored")
	}
	ft.SetSortable(false)
	if ft.Sortable() {
		t.Errorf("SetSortable/Sortable")
	}
	ft.Binary()
	ft.primitiveID()

	// compareValues, makeTermStepper
	if comparison := ft.compareValues("foo", "bar"); comparison <= 0 {
		t.Errorf("Unexpected compareValues result: %d", comparison)
	}
	switch ft.(type) {
	case BlobType:
	default:
		if stepper, ok := ft.makeTermStepper().(TermStepper); !ok {
			t.Errorf("makeTermStepper failed: %v", stepper)
		}
	}

	// Equals, dump/load
	if !ft.Equals(ft) {
		t.Error("Equals self")
	}
	if ft.Equals("foo") {
		t.Error("Equals Go string")
	}
	if ft.Equals(NewStringType()) {
		t.Error("Equals different field type")
	}
	ft.dumpForSchema()
	dupe := ft.load(ft.dump()).(FieldType)
	if !ft.Equals(dupe) {
		t.Errorf("Round-trip through dump/load produced %v", dupe)
	}
	dupe.SetIndexed(true)
	if ft.Equals(dupe) {
		t.Error("Equals with altered dupe")
	}
}

func TestFieldTypeBasics(t *testing.T) {
	runFieldTypeTests(t, NewFullTextType(NewStandardTokenizer()))
	runFieldTypeTests(t, NewStringType())
	runFieldTypeTests(t, NewBlobType(true))
	runFieldTypeTests(t, NewInt32Type())
	runFieldTypeTests(t, NewInt64Type())
	runFieldTypeTests(t, NewFloat32Type())
	runFieldTypeTests(t, NewFloat64Type())
}

func TestFullTextTypeMisc(t *testing.T) {
	ft := NewFullTextType(NewStandardTokenizer())
	ft.SetHighlightable(true)
	if !ft.Highlightable() {
		t.Error("SetHighlightable/Highlightable")
	}
	if sim, ok := ft.makeSimilarity().(Similarity); !ok {
		t.Errorf("makeSimilarity: %v", sim)
	}
	if _, ok := ft.GetAnalyzer().(StandardTokenizer); !ok {
		t.Error("GetAnalyzer")
	}
}

func TestStringTypeMisc(t *testing.T) {
	ft := NewStringType();
	if sim, ok := ft.makeSimilarity().(Similarity); !ok {
		t.Errorf("makeSimilarity: %v", sim)
	}
}

func TestArchitectureBasics(t *testing.T) {
	arch := NewArchitecture()

	if got := arch.indexInterval(); got < 0 {
		t.Errorf("indexInterval: %d", got)
	}
	if got := arch.skipInterval(); got < 0 {
		t.Errorf("skipInterval: %d", got)
	}

	if sim, ok := arch.makeSimilarity().(Similarity); !ok {
		t.Errorf("makeSimilarity: %v", sim)
	}

	if !arch.Equals(arch) {
		t.Error("Equals")
	}
}
