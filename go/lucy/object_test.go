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
import _ "git-wip-us.apache.org/repos/asf/lucy-clownfish.git/runtime/go/clownfish"

func TestBitVecSingle(t *testing.T) {
	bitVec := NewBitVector(0)
	if bitVec.Get(31) {
		t.Error("Get for unset bit")
	}
	bitVec.Set(31)
	if !bitVec.Get(31) {
		t.Error("Get for true bit")
	}
	bitVec.Clear(31)
	if bitVec.Get(31) {
		t.Error("Get for cleared bit")
	}
	bitVec.Flip(31)
	if !bitVec.Get(31) {
		t.Error("Get for flipped bit")
	}
	if hit := bitVec.NextHit(0); hit != 31 {
		t.Error("NextHit should find hit")
	}
	if hit := bitVec.NextHit(32); hit != -1 {
		t.Error("NextHit exhausted")
	}
}

func TestBitVecMisc(t *testing.T) {
	bitVec := NewBitVector(0)
	oldCap := bitVec.getCapacity()
	bitVec.Grow(64)
	if newCap := bitVec.getCapacity(); newCap <= oldCap {
		t.Error("Grow/getCapacity had unexpected result: %v %v", oldCap, newCap)
	}
	bitVec.Set(0)
	bitVec.Set(63)
	bools := bitVec.ToArray()
	count := 0;
	for _, val := range bools {
		if val {
			count++
		}
	}
	if count != 2 {
		t.Error("ToArray yielded bad count: %d", count)
	}
}

func TestBitVecBlock(t *testing.T) {
	bitVec := NewBitVector(0)
	bitVec.FlipBlock(10, 10)
	if count := bitVec.Count(); count != 10 {
		t.Error("FlipBlock")
	}
	bitVec.FlipBlock(15, 5)
	if count := bitVec.Count(); count != 5 {
		t.Error("FlipBlock mixed")
	}
	bitVec.ClearAll()
	if count := bitVec.Count(); count != 0 {
		t.Error("ClearAll")
	}
}

func TestBitVecBool(t *testing.T) {
	var dupe BitVector
	seven := NewBitVector(0);
	twelve := NewBitVector(0);
	seven.FlipBlock(0, 3) // 0111
	twelve.FlipBlock(2, 2) // 1100

	dupe = seven.Clone().(BitVector)
	dupe.And(twelve)
	if count := dupe.Count(); count != 1 {
		t.Errorf("And: %d", count)
	}

	dupe = seven.Clone().(BitVector)
	dupe.Or(twelve)
	if count := dupe.Count(); count != 4 {
		t.Errorf("Or: %d", count)
	}

	dupe = seven.Clone().(BitVector)
	dupe.Xor(twelve)
	if count := dupe.Count(); count != 3 {
		t.Errorf("Xor: %d", count)
	}

	dupe = seven.Clone().(BitVector)
	dupe.AndNot(twelve)
	if count := dupe.Count(); count != 2 {
		t.Errorf("AndNot: %d", count)
	}

	dupe = seven.Clone().(BitVector)
	dupe.mimic(twelve)
	if count := dupe.Count(); count != twelve.Count() {
		t.Errorf("Mimic: %d", count)
	}
}

func TestI32ArrBasics(t *testing.T) {
	arr := NewI32Array([]int32{42, 43})
	if size := arr.GetSize(); size != 2 {
		t.Errorf("Unexpected size: %d", size)
	}
	arr.Set(1, 101)
	if got := arr.Get(1); got != 101 {
		t.Errorf("Unexpected value: %d", got)
	}
}
