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
S_put(PriorityQueue *self, PriorityQueueIVARS *ivars, Obj *element);

// Free all the elements in the heap and set size to 0.
static void
S_clear(PriorityQueue *self, PriorityQueueIVARS *ivars);

// Heap adjuster.
static void
S_up_heap(PriorityQueue *self, PriorityQueueIVARS *ivars);

// Heap adjuster.  Should be called when the item at the top changes.
static void
S_down_heap(PriorityQueue *self, PriorityQueueIVARS *ivars);

PriorityQueue*
PriQ_init(PriorityQueue *self, uint32_t max_size) {
    PriorityQueueIVARS *const ivars = PriQ_IVARS(self);
    if (max_size == UINT32_MAX) {
        THROW(ERR, "max_size too large: %u32", max_size);
    }
    uint32_t heap_size = max_size + 1;

    // Init.
    ivars->size = 0;

    // Assign.
    ivars->max_size = max_size;

    // Allocate space for the heap, assign all slots to NULL.
    ivars->heap = (Obj**)CALLOCATE(heap_size, sizeof(Obj*));

    ABSTRACT_CLASS_CHECK(self, PRIORITYQUEUE);
    return self;
}

void
PriQ_Destroy_IMP(PriorityQueue *self) {
    PriorityQueueIVARS *const ivars = PriQ_IVARS(self);
    if (ivars->heap) {
        S_clear(self, ivars);
        FREEMEM(ivars->heap);
    }
    SUPER_DESTROY(self, PRIORITYQUEUE);
}

uint32_t
PriQ_Get_Size_IMP(PriorityQueue *self) {
    return PriQ_IVARS(self)->size;
}

static void
S_put(PriorityQueue *self, PriorityQueueIVARS *ivars, Obj *element) {
    // Increment size.
    if (ivars->size >= ivars->max_size) {
        THROW(ERR, "PriorityQueue exceeded max_size: %u32 %u32", ivars->size,
              ivars->max_size);
    }
    ivars->size++;

    // Put element into heap.
    ivars->heap[ivars->size] = element;

    // Adjust heap.
    S_up_heap(self, ivars);
}

bool
PriQ_Insert_IMP(PriorityQueue *self, Obj *element) {
    Obj *least = PriQ_Jostle(self, element);
    DECREF(least);
    if (element == least) { return false; }
    else                  { return true; }
}

Obj*
PriQ_Jostle_IMP(PriorityQueue *self, Obj *element) {
    PriorityQueueIVARS *const ivars = PriQ_IVARS(self);

    // Absorb element if there's a vacancy.
    if (ivars->size < ivars->max_size) {
        S_put(self, ivars, element);
        return NULL;
    }
    // Otherwise, compete for the slot.
    else if (ivars->size == 0) {
        return element;
    }
    else {
        Obj *scratch = PriQ_Peek(self);
        if (!PriQ_Less_Than(self, element, scratch)) {
            // If the new element belongs in the queue, replace something.
            Obj *retval = ivars->heap[1];
            ivars->heap[1] = element;
            S_down_heap(self, ivars);
            return retval;
        }
        else {
            return element;
        }
    }
}

Obj*
PriQ_Pop_IMP(PriorityQueue *self) {
    PriorityQueueIVARS *const ivars = PriQ_IVARS(self);
    if (ivars->size > 0) {
        // Save the first value.
        Obj *result = ivars->heap[1];

        // Move last to first and adjust heap.
        ivars->heap[1] = ivars->heap[ivars->size];
        ivars->heap[ivars->size] = NULL;
        ivars->size--;
        S_down_heap(self, ivars);

        // Return the value, leaving a refcount for the caller.
        return result;
    }
    else {
        return NULL;
    }
}

VArray*
PriQ_Pop_All_IMP(PriorityQueue *self) {
    PriorityQueueIVARS *const ivars = PriQ_IVARS(self);
    VArray *retval = VA_new(ivars->size);

    // Map the queue nodes onto the array in reverse order.
    if (ivars->size) {
        for (uint32_t i = ivars->size; i--;) {
            Obj *const elem = PriQ_Pop(self);
            VA_Store(retval, i, elem);
        }
    }

    return retval;
}

Obj*
PriQ_Peek_IMP(PriorityQueue *self) {
    PriorityQueueIVARS *const ivars = PriQ_IVARS(self);
    if (ivars->size > 0) {
        return ivars->heap[1];
    }
    else {
        return NULL;
    }
}

static void
S_clear(PriorityQueue *self, PriorityQueueIVARS *ivars) {
    UNUSED_VAR(self);
    Obj **elem_ptr = (ivars->heap + 1);

    // Node 0 is held empty, to make the algo clearer.
    for (uint32_t i = 1; i <= ivars->size; i++) {
        DECREF(*elem_ptr);
        *elem_ptr = NULL;
        elem_ptr++;
    }
    ivars->size = 0;
}

static void
S_up_heap(PriorityQueue *self, PriorityQueueIVARS *ivars) {
    uint32_t i = ivars->size;
    uint32_t j = i >> 1;
    Obj *const node = ivars->heap[i]; // save bottom node

    while (j > 0
           && PriQ_Less_Than(self, node, ivars->heap[j])
          ) {
        ivars->heap[i] = ivars->heap[j];
        i = j;
        j = j >> 1;
    }
    ivars->heap[i] = node;
}

static void
S_down_heap(PriorityQueue *self, PriorityQueueIVARS *ivars) {
    uint32_t i = 1;
    uint32_t j = i << 1;
    uint32_t k = j + 1;
    Obj *node = ivars->heap[i]; // save top node

    // Find smaller child.
    if (k <= ivars->size
        && PriQ_Less_Than(self, ivars->heap[k], ivars->heap[j])
       ) {
        j = k;
    }

    while (j <= ivars->size
           && PriQ_Less_Than(self, ivars->heap[j], node)
          ) {
        ivars->heap[i] = ivars->heap[j];
        i = j;
        j = i << 1;
        k = j + 1;
        if (k <= ivars->size
            && PriQ_Less_Than(self, ivars->heap[k], ivars->heap[j])
           ) {
            j = k;
        }
    }
    ivars->heap[i] = node;
}


