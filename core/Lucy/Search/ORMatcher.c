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

#define C_LUCY_ORMATCHER
#define C_LUCY_ORSCORER
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Search/ORMatcher.h"
#include "Lucy/Index/Similarity.h"

// Add an element to the queue.  Unsafe -- bounds checking of queue size is
// left to the caller.
static void
S_add_element(ORMatcher *self, Matcher *matcher, int32_t doc_id);

// Empty out the queue.
static void
S_clear(ORMatcher *self);

// Call Matcher_Next() on the top queue element and adjust the queue,
// removing the element if Matcher_Next() returns false.
static INLINE int32_t
SI_top_next(ORMatcher *self);

// Call Matcher_Advance() on the top queue element and adjust the queue,
// removing the element if Matcher_Advance() returns false.
static INLINE int32_t
SI_top_advance(ORMatcher *self, int32_t target);

/* React to a change in the top element, or "root" -- presumably the update of
 * its doc_id resulting from a call to Matcher_Next() or Matcher_Advance().
 * If the Matcher has been exhausted, remove it from the queue and replace it
 * with the bottom node (i.e. perform "root replacement").  In either case,
 * perform a "sift down" to restore the heap property.  Return the doc id of
 * the root node, or 0 if the queue has been emptied.
 */
static int32_t
S_adjust_root(ORMatcher *self);

// Take the bottom node (which probably violates the heap property when this
// is called) and bubble it up through the heap until the heap property is
// restored.
static void
S_bubble_up(ORMatcher *self);

// Take the top node (which probably violates the heap property when this
// is called) and sift it down through the heap until the heap property is
// restored.
static void
S_sift_down(ORMatcher *self);

ORMatcher*
ORMatcher_new(VArray *children) {
    ORMatcher *self = (ORMatcher*)VTable_Make_Obj(ORMATCHER);
    return ORMatcher_init(self, children);
}

static ORMatcher*
S_ormatcher_init2(ORMatcher *self, VArray *children, Similarity *sim) {
    size_t amount_to_malloc;
    uint32_t i;

    // Init.
    PolyMatcher_init((PolyMatcher*)self, children, sim);
    self->size = 0;

    // Derive.
    self->max_size = VA_Get_Size(children);

    // Allocate.
    self->heap = (HeapedMatcherDoc**)CALLOCATE(self->max_size + 1, sizeof(HeapedMatcherDoc*));

    // Create a pool of HMDs.  Encourage CPU cache hits by using a single
    // allocation for all of them.
    amount_to_malloc = (self->max_size + 1) * sizeof(HeapedMatcherDoc);
    self->blob = (char*)MALLOCATE(amount_to_malloc);
    self->pool = (HeapedMatcherDoc**)CALLOCATE(self->max_size + 1, sizeof(HeapedMatcherDoc*));
    for (i = 1; i <= self->max_size; i++) {
        size_t offset = i * sizeof(HeapedMatcherDoc);
        HeapedMatcherDoc *hmd = (HeapedMatcherDoc*)(self->blob + offset);
        self->pool[i] = hmd;
    }

    // Prime queue.
    for (i = 0; i < self->max_size; i++) {
        Matcher *matcher = (Matcher*)VA_Fetch(children, i);
        if (matcher) {
            S_add_element(self, (Matcher*)INCREF(matcher), 0);
        }
    }

    return self;
}

ORMatcher*
ORMatcher_init(ORMatcher *self, VArray *children) {
    return S_ormatcher_init2(self, children, NULL);
}

void
ORMatcher_destroy(ORMatcher *self) {
    if (self->blob) { S_clear(self); }
    FREEMEM(self->blob);
    FREEMEM(self->pool);
    FREEMEM(self->heap);
    SUPER_DESTROY(self, ORMATCHER);
}

int32_t
ORMatcher_next(ORMatcher *self) {
    if (self->size == 0) {
        return 0;
    }
    else {
        int32_t last_doc_id = self->top_hmd->doc;
        while (self->top_hmd->doc == last_doc_id) {
            int32_t top_doc_id = SI_top_next(self);
            if (!top_doc_id && self->size == 0) {
                return 0;
            }
        }
        return self->top_hmd->doc;
    }
}

int32_t
ORMatcher_advance(ORMatcher *self, int32_t target) {
    if (!self->size) { return 0; }
    do {
        int32_t least = SI_top_advance(self, target);
        if (least >= target) { return least; }
        if (!least) {
            if (!self->size) { return 0; }
        }
    } while (true);
}

int32_t
ORMatcher_get_doc_id(ORMatcher *self) {
    return self->top_hmd->doc;
}

static void
S_clear(ORMatcher *self) {
    HeapedMatcherDoc **const heap = self->heap;
    HeapedMatcherDoc **const pool = self->pool;

    // Node 0 is held empty, to make the algo clearer.
    for (; self->size > 0; self->size--) {
        HeapedMatcherDoc *hmd = heap[self->size];
        heap[self->size] = NULL;
        DECREF(hmd->matcher);

        // Put HMD back in pool.
        pool[self->size] = hmd;
    }
}

static INLINE int32_t
SI_top_next(ORMatcher *self) {
    HeapedMatcherDoc *const top_hmd = self->top_hmd;
    top_hmd->doc = Matcher_Next(top_hmd->matcher);
    return S_adjust_root(self);
}

static INLINE int32_t
SI_top_advance(ORMatcher *self, int32_t target) {
    HeapedMatcherDoc *const top_hmd = self->top_hmd;
    top_hmd->doc = Matcher_Advance(top_hmd->matcher, target);
    return S_adjust_root(self);
}

static void
S_add_element(ORMatcher *self, Matcher *matcher, int32_t doc_id) {
    HeapedMatcherDoc **const heap = self->heap;
    HeapedMatcherDoc **const pool = self->pool;
    HeapedMatcherDoc *hmd;

    // Increment size.
    self->size++;

    // Put element at the bottom of the heap.
    hmd          = pool[self->size];
    hmd->matcher = matcher;
    hmd->doc     = doc_id;
    heap[self->size] = hmd;

    // Adjust heap.
    S_bubble_up(self);
}

static int32_t
S_adjust_root(ORMatcher *self) {
    HeapedMatcherDoc *const top_hmd = self->top_hmd;

    // Inlined pop.
    if (!top_hmd->doc) {
        HeapedMatcherDoc *const last_hmd = self->heap[self->size];

        // Last to first.
        DECREF(top_hmd->matcher);
        top_hmd->matcher = last_hmd->matcher;
        top_hmd->doc     = last_hmd->doc;
        self->heap[self->size] = NULL;

        // Put back in pool.
        self->pool[self->size] = last_hmd;

        self->size--;
        if (self->size == 0) {
            return 0;
        }
    }

    // Move queue no matter what.
    S_sift_down(self);

    return self->top_hmd->doc;
}

static void
S_bubble_up(ORMatcher *self) {
    HeapedMatcherDoc **const heap = self->heap;
    uint32_t i = self->size;
    uint32_t j = i >> 1;
    HeapedMatcherDoc *const node = heap[i]; // save bottom node

    while (j > 0 && node->doc < heap[j]->doc) {
        heap[i] = heap[j];
        i = j;
        j = j >> 1;
    }
    heap[i] = node;
    self->top_hmd = heap[1];
}

static void
S_sift_down(ORMatcher *self) {
    HeapedMatcherDoc **const heap = self->heap;
    uint32_t i = 1;
    uint32_t j = i << 1;
    uint32_t k = j + 1;
    HeapedMatcherDoc *const node = heap[i]; // save top node

    // Find smaller child.
    if (k <= self->size && heap[k]->doc < heap[j]->doc) {
        j = k;
    }

    while (j <= self->size && heap[j]->doc < node->doc) {
        heap[i] = heap[j];
        i = j;
        j = i << 1;
        k = j + 1;
        if (k <= self->size && heap[k]->doc < heap[j]->doc) {
            j = k;
        }
    }
    heap[i] = node;

    self->top_hmd = heap[1];
}

/***************************************************************************/

/* When this is called, all children are past the current self->doc_id.  The
 * least doc_id amongst them becomes the new self->doc_id, and they are all
 * advanced so that they are once again out in front of it.  While they are
 * advancing, their scores are cached in an array, to be summed during
 * Score().
 */
static int32_t
S_advance_after_current(ORScorer *self);

ORScorer*
ORScorer_new(VArray *children, Similarity *sim) {
    ORScorer *self = (ORScorer*)VTable_Make_Obj(ORSCORER);
    return ORScorer_init(self, children, sim);
}

ORScorer*
ORScorer_init(ORScorer *self, VArray *children, Similarity *sim) {
    S_ormatcher_init2((ORMatcher*)self, children, sim);
    self->doc_id = 0;
    self->scores = (float*)MALLOCATE(self->num_kids * sizeof(float));

    // Establish the state of all child matchers being past the current doc
    // id, by invoking ORMatcher's Next() method.
    ORMatcher_next((ORMatcher*)self);

    return self;
}

void
ORScorer_destroy(ORScorer *self) {
    FREEMEM(self->scores);
    SUPER_DESTROY(self, ORSCORER);
}

int32_t
ORScorer_next(ORScorer *self) {
    return S_advance_after_current(self);
}

static int32_t
S_advance_after_current(ORScorer *self) {
    float *const     scores = self->scores;
    Matcher *child;

    // Get the top Matcher, or bail because there are no Matchers left.
    if (!self->size) { return 0; }
    else             { child = self->top_hmd->matcher; }

    // The top matcher will already be at the correct doc, so start there.
    self->doc_id        = self->top_hmd->doc;
    scores[0]           = Matcher_Score(child);
    self->matching_kids = 1;

    do {
        // Attempt to advance past current doc.
        int32_t top_doc_id = SI_top_next((ORMatcher*)self);
        if (!top_doc_id) {
            if (!self->size) {
                break; // bail, no more to advance
            }
        }

        if (top_doc_id != self->doc_id) {
            // Bail, least doc in queue is now past the one we're scoring.
            break;
        }
        else {
            // Accumulate score.
            child = self->top_hmd->matcher;
            scores[self->matching_kids] = Matcher_Score(child);
            self->matching_kids++;
        }
    } while (true);

    return self->doc_id;
}

int32_t
ORScorer_advance(ORScorer *self, int32_t target) {
    // Return sentinel once exhausted.
    if (!self->size) { return 0; }

    // Succeed if we're already past and still on a valid doc.
    if (target <= self->doc_id) {
        return self->doc_id;
    }

    do {
        // If all matchers are caught up, accumulate score and return.
        if (self->top_hmd->doc >= target) {
            return S_advance_after_current(self);
        }

        // Not caught up yet, so keep skipping matchers.
        if (!SI_top_advance((ORMatcher*)self, target)) {
            if (!self->size) { return 0; }
        }
    } while (true);
}

int32_t
ORScorer_get_doc_id(ORScorer *self) {
    return self->doc_id;
}

float
ORScorer_score(ORScorer *self) {
    float *const scores = self->scores;
    float score = 0.0f;
    uint32_t i;

    // Accumulate score, then factor in coord bonus.
    for (i = 0; i < self->matching_kids; i++) {
        score += scores[i];
    }
    score *= self->coord_factors[self->matching_kids];

    return score;
}

