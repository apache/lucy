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

import "sync"

type indexInt uintptr

type objRegistry struct {
	// Use pointer to array to guarantee atomic update for lock-free reads.
	// Assume that loads and stores of the pointer are atomic.
	entries *[]interface{}
	freeListHead indexInt
	mutex sync.Mutex
}

func newObjRegistry(size uintptr) *objRegistry {
	entries := make([]interface{}, size)

	// Each empty entry points to the index of the next empty entry.  Index 0
	// is unused.  The last slot is seet to a terminating sentry value of 0.
	entries[0] = indexInt(0) // unused
	for i := uintptr(1); i < size - 1; i++ {
		entries[i] = indexInt(i + 1)
	}
	entries[size-1] = indexInt(0)

	reg := &objRegistry{}
	reg.entries = &entries
	reg.freeListHead = indexInt(1)

	return reg
}

func (reg *objRegistry) store(obj interface{}) uintptr {
	reg.mutex.Lock()

	// Find the index of the next empty slot.
	index := uintptr(reg.freeListHead)

	entries := reg.entries

	if (index != 0) {
		// A slot is available.  It contains the index of the next available
		// slot; put that index into the freeListHead.
		reg.freeListHead = (*entries)[index].(indexInt)
	} else {
		// The sentinel value was encountered, indicating that we are out of
		// space and must grow the entries array.

		// The list head was 0, a slot we don't want to use.  Figure out what
		// slot we're going to use instead.  If the current size of the
		// entries array is 8, and will soon be 16, use slot 8.
		index = uintptr(len(*entries))

		// Duplicate the array and copy in the existing entries data.
		newSize := index * 2
		newEntries := make([]interface{}, newSize)
		copy(newEntries, *entries)

		// Set up each new empty slot to point at another new empty slot, up
		// to the final slot which will get the sentinel value 0.
		for i := index + 1; i < newSize - 1; i++ {
			newEntries[i] = indexInt(i + 1)
		}
		newEntries[newSize - 1] = indexInt(0)
		entries = &newEntries
		reg.entries = entries

		// Set the freeListHead to one greater than the slot we're using this
		// time -- i.e. if the current size is 8, the new size is 16, and the
		// slot we use for the supplied value is 8, then the new list head
		// will be 9.
		reg.freeListHead = indexInt(index + 1)
	}

	// Store the supplied value in the slot.
	(*entries)[index] = obj

	reg.mutex.Unlock()

	return index
}

func (reg *objRegistry) fetch(index uintptr) interface{} {

	// Ignore an out of range request.
	if index >= uintptr(len(*reg.entries)) {
		return nil
	}
	entry := (*reg.entries)[index]
	if _, ok := entry.(indexInt); ok {
		// Return nil if the slot is empty.
		return nil
	}
	return entry
}

func (reg *objRegistry) delete(index uintptr) {
	reg.mutex.Lock()

	// Overwrite the value at the supplied index with the freeListHead.  For
	// example, if you are storing strings and the entries array consists of
	// {0, "A", "B", C", 5, 6, 7, 0}, with freeListHead at 4, then deleting
	// index 2 (string value "B") will result in the following state:
	// {0, "A", 4, "C", 5, 6, 7, 0} and freeListHead at 2.
	//
	// Some potential errors are ignored:
	// *   Index is greater than the size of the array.
	// *   Slot is empty.
	if index < uintptr(len(*reg.entries)) {
		_, isIndexInt := (*reg.entries)[index].(indexInt)
		if !isIndexInt {
			(*reg.entries)[index] = reg.freeListHead
			reg.freeListHead = indexInt(index)
		}
	}

	reg.mutex.Unlock()
}

