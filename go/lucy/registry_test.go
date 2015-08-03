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
import "math/rand"

func TestRegistrySingle(t *testing.T) {
	reg := newObjRegistry(4)
	index := reg.store(42)
	if intVal, ok := reg.fetch(index).(int); !ok || intVal != 42 {
		t.Error("Failed to store/fetch int")
	}
	reg.delete(index)
	if reg.fetch(index) != nil {
		t.Error("Failed to delete int")
	}
}

func TestRegistryMany(t *testing.T) {
	reg := newObjRegistry(4)
	stored := make(map[int]uintptr)
	deleted := make(map[int]uintptr)
	for i := 0; i < 1000; i++ {
		if rand.Intn(10) == 0 {
			// Randomly delete an element 10% of the time.
			goner := rand.Intn(i - 1)
			if index, ok := stored[goner]; ok {
				reg.delete(index)
				delete(stored, goner)
				deleted[goner] = index
			}
		}
		stored[i] = reg.store(i)
	}
	for expected, index := range stored {
		got, ok := reg.fetch(index).(int)
		if !ok {
			t.Errorf("Failed to fetch stored value %d at index %d", expected, index)
		} else if got != expected {
			t.Errorf("Expected %d got %d", expected, got)
		}
	}
	for i := 0; i < len(*reg.entries) - 1; i++ {
		got, ok := reg.fetch(uintptr(i)).(int)
		if ok {
			if _, wasDeleted := deleted[got]; wasDeleted {
				t.Errorf("Deleted item %d still present at index %d", got, i)
			}
		}
	}
}

func TestRegistryStringSlice(t *testing.T) {
	reg := newObjRegistry(4)
	s := make([]int, 2)
	index := reg.store(&s)
	s2 := reg.fetch(index).(*[]int)
	(*s2)[1] = 1000
	if s[1] != 1000 {
		t.Error("Not the same slice")
	}
}

func TestRegistryRange(t *testing.T) {
	reg := newObjRegistry(4)
	if reg.fetch(uintptr(10)) != nil {
		t.Error("Out of range index should return nil")
	}
}
