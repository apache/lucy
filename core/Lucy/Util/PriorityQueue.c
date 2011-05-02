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

#define C_LUCY_PRIORITYQUEUE
#include "Lucy/Util/ToolSet.h"

#include <string.h>

#include "Lucy/Util/PriorityQueue.h"

// Add an element to the heap.  Throw an error if too many elements
// are added.
static void
S_put(PriorityQueue *self, Obj *element);

// Free all the elements in the heap and set size to 0.
static void
S_clear(PriorityQueue *self);

// Heap adjuster.
static void
S_up_heap(PriorityQueue *self);

// Heap adjuster.  Should be called when the item at the top changes.
static void
S_down_heap(PriorityQueue *self);

PriorityQueue*
PriQ_init(PriorityQueue *self, uint32_t max_size) {
    if (max_size == U32_MAX) {
        THROW(ERR, "max_size too large: %u32", max_size);
    }
    uint32_t heap_size = max_size + 1;

    // Init.
    self->size = 0;

    // Assign.
    self->max_size = max_size;

    // Allocate space for the heap, assign all slots to NULL.
    self->heap = (Obj**)CALLOCATE(heap_size, sizeof(Obj*));

    ABSTRACT_CLASS_CHECK(self, PRIORITYQUEUE);
    return self;
}

void
PriQ_destroy(PriorityQueue *self) {
    if (self->heap) {
        S_clear(self);
        FREEMEM(self->heap);
    }
    SUPER_DESTROY(self, PRIORITYQUEUE);
}

uint32_t
PriQ_get_size(PriorityQueue *self) {
    return self->size;
}

static void
S_put(PriorityQueue *self, Obj *element) {
    // Increment size.
    if (self->size >= self->max_size) {
        THROW(ERR, "PriorityQueue exceeded max_size: %u32 %u32", self->size,
              self->max_size);
    }
    self->size++;

    // Put element into heap.
    self->heap[self->size] = element;

    // Adjust heap.
    S_up_heap(self);
}

bool_t
PriQ_insert(PriorityQueue *self, Obj *element) {
    Obj *least = PriQ_Jostle(self, element);
    DECREF(least);
    if (element == least) { return false; }
    else                  { return true; }
}

Obj*
PriQ_jostle(PriorityQueue *self, Obj *element) {
    // Absorb element if there's a vacancy.
    if (self->size < self->max_size) {
        S_put(self, element);
        return NULL;
    }
    // Otherwise, compete for the slot.
    else if (self->size == 0) {
        return element;
    }
    else {
        Obj *scratch = PriQ_Peek(self);
        if (!PriQ_Less_Than(self, element, scratch)) {
            // If the new element belongs in the queue, replace something.
            Obj *retval = self->heap[1];
            self->heap[1] = element;
            S_down_heap(self);
            return retval;
        }
        else {
            return element;
        }
    }
}

Obj*
PriQ_pop(PriorityQueue *self) {
    if (self->size > 0) {
        // Save the first value.
        Obj *result = self->heap[1];

        // Move last to first and adjust heap.
        self->heap[1] = self->heap[self->size];
        self->heap[self->size] = NULL;
        self->size--;
        S_down_heap(self);

        // Return the value, leaving a refcount for the caller.
        return result;
    }
    else {
        return NULL;
    }
}

VArray*
PriQ_pop_all(PriorityQueue *self) {
    VArray *retval = VA_new(self->size);

    // Map the queue nodes onto the array in reverse order.
    if (self->size) {
        uint32_t i;
        for (i = self->size; i--;) {
            Obj *const elem = PriQ_Pop(self);
            VA_Store(retval, i, elem);
        }
    }

    return retval;
}

Obj*
PriQ_peek(PriorityQueue *self) {
    if (self->size > 0) {
        return self->heap[1];
    }
    else {
        return NULL;
    }
}

static void
S_clear(PriorityQueue *self) {
    uint32_t i;
    Obj **elem_ptr = (self->heap + 1);

    // Node 0 is held empty, to make the algo clearer.
    for (i = 1; i <= self->size; i++) {
        DECREF(*elem_ptr);
        *elem_ptr = NULL;
        elem_ptr++;
    }
    self->size = 0;
}

static void
S_up_heap(PriorityQueue *self) {
    uint32_t i = self->size;
    uint32_t j = i >> 1;
    Obj *const node = self->heap[i]; // save bottom node

    while (j > 0
           && PriQ_Less_Than(self, node, self->heap[j])
          ) {
        self->heap[i] = self->heap[j];
        i = j;
        j = j >> 1;
    }
    self->heap[i] = node;
}

static void
S_down_heap(PriorityQueue *self) {
    uint32_t i = 1;
    uint32_t j = i << 1;
    uint32_t k = j + 1;
    Obj *node = self->heap[i]; // save top node

    // Find smaller child.
    if (k <= self->size
        && PriQ_Less_Than(self, self->heap[k], self->heap[j])
       ) {
        j = k;
    }

    while (j <= self->size
           && PriQ_Less_Than(self, self->heap[j], node)
          ) {
        self->heap[i] = self->heap[j];
        i = j;
        j = i << 1;
        k = j + 1;
        if (k <= self->size
            && PriQ_Less_Than(self, self->heap[k], self->heap[j])
           ) {
            j = k;
        }
    }
    self->heap[i] = node;
}


