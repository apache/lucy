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

func TestRAMFileBasics(t *testing.T) {
	fooBytes := []byte("foo")
	contents := clownfish.NewByteBuf(5)
	contents.Cat(fooBytes)
	ramFile := NewRAMFile(contents, false)

	if ramFile.ReadOnly() {
		t.Error("ReadOnly")
	}
	ramFile.SetReadOnly(true)
	if !ramFile.ReadOnly() {
		t.Error("SetReadOnly/ReadOnly")
	}

	if got := ramFile.GetContents().YieldBlob(); !reflect.DeepEqual(got, fooBytes) {
		t.Errorf("GetContents: %v", got)
	}
}
