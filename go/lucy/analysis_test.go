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

func TestTokenBasics(t *testing.T) {
	token := NewToken("foo")
	if got := token.GetText(); got != "foo" {
		t.Errorf("GetText: %v", got)
	}
	token.SetText("bar")
	if got := token.GetText(); got != "bar" {
		t.Errorf("SetText/GetText: %v", got)
	}

	// Verify that these were bound.
	token.GetStartOffset()
	token.GetEndOffset()
	token.GetBoost()
	token.GetPosInc()
	token.GetPos()
	token.GetLen()
}

func TestInversionBasics(t *testing.T) {
	inv := NewInversion(nil)
	inv.Append(NewToken("foo"))
	inv.Append(NewToken("bar"))
	inv.Append(NewToken("baz"))
	if size := inv.GetSize(); size != 3 {
		t.Errorf("Unexpected size: %d", size)
	}

	if got := inv.Next().GetText(); got != "foo" {
		t.Errorf("Next yielded %s", got)
	}

	inv.Reset()
	if got := inv.Next().GetText(); got != "foo" {
		t.Errorf("Next after Reset yielded %s", got)
	}

	inv.Reset()
	inv.Invert()
	if got := inv.Next().GetText(); got != "bar" {
		t.Errorf("Next after Invert yielded %s", got)
	}
}

func TestRegexTokenizerSplit(t *testing.T) {
	tokenizer := NewRegexTokenizer("\\S+")
	expected := []string{"foo", "bar", "baz"}
	got := tokenizer.Split("foo bar baz")
	if !reflect.DeepEqual(got, expected) {
		t.Errorf("Expected %v, got %v", expected, got)
	}
}
