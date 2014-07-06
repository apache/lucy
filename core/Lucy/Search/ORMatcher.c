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
S_add_element(ORMatcher *self, ORMatcherIVARS *ivars, Matcher *matcher,
              int32_t doc_id);

// Empty out the queue.
static void
S_clear(ORMatcher *self, ORMatcherIVARS *ivars);

// Call Matcher_Next() on the top queue element and adjust the queue,
// removing the element if Matcher_Next() returns false.
static CFISH_INLINE int32_t
SI_top_next(ORMatcher *self, ORMatcherIVARS *ivars);

// Call Matcher_Advance() on the top queue element and adjust the queue,
// removing the element if Matcher_Advance() returns false.
static CFISH_INLINE int32_t
SI_top_advance(ORMatcher *self, ORMatcherIVARS *ivars, int32_t target);

/* React to a change in the top element, or "root" -- presumably the update of
 * its doc_id resulting from a call to Matcher_Next() or Matcher_Advance().
 * If the Matcher has been exhausted, remove it from the queue and replace it
 * with the bottom node (i.e. perform "root replacement").  In either case,
 * perform a "sift down" to restore the heap property.  Return the doc id of
 * the root node, or 0 if the queue has been emptied.
 */
static int32_t
S_adjust_root(ORMatcher *self, ORMatcherIVARS *ivars);

// Take the bottom node (which probably violates the heap property when this
// is called) and bubble it up through the heap until the heap property is
// restored.
static void
S_bubble_up(ORMatcher *self, ORMatcherIVARS *ivars);

// Take the top node (which probably violates the heap property when this
// is called) and sift it down through the heap until the heap property is
// restored.
static void
S_sift_down(ORMatcher *self, ORMatcherIVARS *ivars);

ORMatcher*
ORMatcher_new(VArray *children) {
    ORMatcher *self = (ORMatcher*)Class_Make_Obj(ORMATCHER);
    return ORMatcher_init(self, children);
}

static ORMatcher*
S_ormatcher_init2(ORMatcher *self, ORMatcherIVARS *ivars, VArray *children,
                  Similarity *sim) {
    // Init.
    PolyMatcher_init((PolyMatcher*)self, children, sim);
    ivars->size = 0;

    // Derive.
    ivars->max_size = VA_Get_Size(children);

    // Allocate.
    ivars->heap = (HeapedMatcherDoc**)CALLOCATE(ivars->max_size + 1, sizeof(HeapedMatcherDoc*));

    // Create a pool of HMDs.  Encourage CPU cache hits by using a single
    // allocation for all of them.
    size_t amount_to_malloc = (ivars->max_size + 1) * sizeof(HeapedMatcherDoc);
    ivars->blob = (char*)MALLOCATE(amount_to_malloc);
    ivars->pool = (HeapedMatcherDoc**)CALLOCATE(ivars->max_size + 1, sizeof(HeapedMatcherDoc*));
    for (uint32_t i = 1; i <= ivars->max_size; i++) {
        size_t offset = i * sizeof(HeapedMatcherDoc);
        HeapedMatcherDoc *hmd = (HeapedMatcherDoc*)(ivars->blob + offset);
        ivars->pool[i] = hmd;
    }

    // Prime queue.
    for (uint32_t i = 0; i < ivars->max_size; i++) {
        Matcher *matcher = (Matcher*)VA_Fetch(children, i);
        if (matcher) {
            S_add_element(self, ivars, (Matcher*)INCREF(matcher), 0);
        }
    }

    return self;
}

ORMatcher*
ORMatcher_init(ORMatcher *self, VArray *children) {
    ORMatcherIVARS *const ivars = ORMatcher_IVARS(self);
    return S_ormatcher_init2(self, ivars, children, NULL);
}

void
ORMatcher_Destroy_IMP(ORMatcher *self) {
    ORMatcherIVARS *const ivars = ORMatcher_IVARS(self);
    if (ivars->blob) { S_clear(self, ivars); }
    FREEMEM(ivars->blob);
    FREEMEM(ivars->pool);
    FREEMEM(ivars->heap);
    SUPER_DESTROY(self, ORMATCHER);
}

int32_t
ORMatcher_Next_IMP(ORMatcher *self) {
    ORMatcherIVARS *const ivars = ORMatcher_IVARS(self);
    if (ivars->size == 0) {
        return 0;
    }
    else {
        int32_t last_doc_id = ivars->top_hmd->doc;
        while (ivars->top_hmd->doc == last_doc_id) {
            int32_t top_doc_id = SI_top_next(self, ivars);
            if (!top_doc_id && ivars->size == 0) {
                return 0;
            }
        }
        return ivars->top_hmd->doc;
    }
}

int32_t
ORMatcher_Advance_IMP(ORMatcher *self, int32_t target) {
    ORMatcherIVARS *const ivars = ORMatcher_IVARS(self);
    if (!ivars->size) { return 0; }
    do {
        int32_t least = SI_top_advance(self, ivars, target);
        if (least >= target) { return least; }
        if (!least) {
            if (!ivars->size) { return 0; }
        }
    } while (true);
}

int32_t
ORMatcher_Get_Doc_ID_IMP(ORMatcher *self) {
    return ORMatcher_IVARS(self)->top_hmd->doc;
}

static void
S_clear(ORMatcher *self, ORMatcherIVARS *ivars) {
    UNUSED_VAR(self);
    HeapedMatcherDoc **const heap = ivars->heap;
    HeapedMatcherDoc **const pool = ivars->pool;

    // Node 0 is held empty, to make the algo clearer.
    for (; ivars->size > 0; ivars->size--) {
        HeapedMatcherDoc *hmd = heap[ivars->size];
        heap[ivars->size] = NULL;
        DECREF(hmd->matcher);

        // Put HMD back in pool.
        pool[ivars->size] = hmd;
    }
}

static CFISH_INLINE int32_t
SI_top_next(ORMatcher *self, ORMatcherIVARS *ivars) {
    HeapedMatcherDoc *const top_hmd = ivars->top_hmd;
    top_hmd->doc = Matcher_Next(top_hmd->matcher);
    return S_adjust_root(self, ivars);
}

static CFISH_INLINE int32_t
SI_top_advance(ORMatcher *self, ORMatcherIVARS *ivars, int32_t target) {
    HeapedMatcherDoc *const top_hmd = ivars->top_hmd;
    top_hmd->doc = Matcher_Advance(top_hmd->matcher, target);
    return S_adjust_root(self, ivars);
}

static void
S_add_element(ORMatcher *self, ORMatcherIVARS *ivars, Matcher *matcher,
              int32_t doc_id) {
    HeapedMatcherDoc **const heap = ivars->heap;
    HeapedMatcherDoc **const pool = ivars->pool;
    HeapedMatcherDoc *hmd;

    // Increment size.
    ivars->size++;

    // Put element at the bottom of the heap.
    hmd          = pool[ivars->size];
    hmd->matcher = matcher;
    hmd->doc     = doc_id;
    heap[ivars->size] = hmd;

    // Adjust heap.
    S_bubble_up(self, ivars);
}

static int32_t
S_adjust_root(ORMatcher *self, ORMatcherIVARS *ivars) {
    HeapedMatcherDoc *const top_hmd = ivars->top_hmd;

    // Inlined pop.
    if (!top_hmd->doc) {
        HeapedMatcherDoc *const last_hmd = ivars->heap[ivars->size];

        // Last to first.
        DECREF(top_hmd->matcher);
        top_hmd->matcher = last_hmd->matcher;
        top_hmd->doc     = last_hmd->doc;
        ivars->heap[ivars->size] = NULL;

        // Put back in pool.
        ivars->pool[ivars->size] = last_hmd;

        ivars->size--;
        if (ivars->size == 0) {
            return 0;
        }
    }

    // Move queue no matter what.
    S_sift_down(self, ivars);

    return ivars->top_hmd->doc;
}

static void
S_bubble_up(ORMatcher *self, ORMatcherIVARS *ivars) {
    UNUSED_VAR(self);
    HeapedMatcherDoc **const heap = ivars->heap;
    uint32_t i = ivars->size;
    uint32_t j = i >> 1;
    HeapedMatcherDoc *const node = heap[i]; // save bottom node

    while (j > 0 && node->doc < heap[j]->doc) {
        heap[i] = heap[j];
        i = j;
        j = j >> 1;
    }
    heap[i] = node;
    ivars->top_hmd = heap[1];
}

static void
S_sift_down(ORMatcher *self, ORMatcherIVARS *ivars) {
    UNUSED_VAR(self);
    HeapedMatcherDoc **const heap = ivars->heap;
    uint32_t i = 1;
    uint32_t j = i << 1;
    uint32_t k = j + 1;
    HeapedMatcherDoc *const node = heap[i]; // save top node

    // Find smaller child.
    if (k <= ivars->size && heap[k]->doc < heap[j]->doc) {
        j = k;
    }

    while (j <= ivars->size && heap[j]->doc < node->doc) {
        heap[i] = heap[j];
        i = j;
        j = i << 1;
        k = j + 1;
        if (k <= ivars->size && heap[k]->doc < heap[j]->doc) {
            j = k;
        }
    }
    heap[i] = node;

    ivars->top_hmd = heap[1];
}

/***************************************************************************/

/* When this is called, all children are past the current ivars->doc_id.  The
 * least doc_id amongst them becomes the new ivars->doc_id, and they are all
 * advanced so that they are once again out in front of it.  While they are
 * advancing, their scores are cached in an array, to be summed during
 * Score().
 */
static int32_t
S_advance_after_current(ORScorer *self, ORScorerIVARS *ivars);

ORScorer*
ORScorer_new(VArray *children, Similarity *sim) {
    ORScorer *self = (ORScorer*)Class_Make_Obj(ORSCORER);
    return ORScorer_init(self, children, sim);
}

ORScorer*
ORScorer_init(ORScorer *self, VArray *children, Similarity *sim) {
    ORScorerIVARS *const ivars = ORScorer_IVARS(self);
    S_ormatcher_init2((ORMatcher*)self, (ORMatcherIVARS*)ivars, children, sim);
    ivars->doc_id = 0;
    ivars->scores = (float*)MALLOCATE(ivars->num_kids * sizeof(float));

    // Establish the state of all child matchers being past the current doc
    // id, by invoking ORMatcher's Next() method.
    ORMatcher_Next_IMP((ORMatcher*)self);

    return self;
}

void
ORScorer_Destroy_IMP(ORScorer *self) {
    ORScorerIVARS *const ivars = ORScorer_IVARS(self);
    FREEMEM(ivars->scores);
    SUPER_DESTROY(self, ORSCORER);
}

int32_t
ORScorer_Next_IMP(ORScorer *self) {
    ORScorerIVARS *const ivars = ORScorer_IVARS(self);
    return S_advance_after_current(self, ivars);
}

static int32_t
S_advance_after_current(ORScorer *self, ORScorerIVARS *ivars) {
    float *const     scores = ivars->scores;
    Matcher *child;

    // Get the top Matcher, or bail because there are no Matchers left.
    if (!ivars->size) { return 0; }
    else              { child = ivars->top_hmd->matcher; }

    // The top matcher will already be at the correct doc, so start there.
    ivars->doc_id        = ivars->top_hmd->doc;
    scores[0]            = Matcher_Score(child);
    ivars->matching_kids = 1;

    do {
        // Attempt to advance past current doc.
        int32_t top_doc_id
            = SI_top_next((ORMatcher*)self, (ORMatcherIVARS*)ivars);
        if (!top_doc_id) {
            if (!ivars->size) {
                break; // bail, no more to advance
            }
        }

        if (top_doc_id != ivars->doc_id) {
            // Bail, least doc in queue is now past the one we're scoring.
            break;
        }
        else {
            // Accumulate score.
            child = ivars->top_hmd->matcher;
            scores[ivars->matching_kids] = Matcher_Score(child);
            ivars->matching_kids++;
        }
    } while (true);

    return ivars->doc_id;
}

int32_t
ORScorer_Advance_IMP(ORScorer *self, int32_t target) {
    ORScorerIVARS *const ivars = ORScorer_IVARS(self);

    // Return sentinel once exhausted.
    if (!ivars->size) { return 0; }

    // Succeed if we're already past and still on a valid doc.
    if (target <= ivars->doc_id) {
        return ivars->doc_id;
    }

    do {
        // If all matchers are caught up, accumulate score and return.
        if (ivars->top_hmd->doc >= target) {
            return S_advance_after_current(self, ivars);
        }

        // Not caught up yet, so keep skipping matchers.
        if (!SI_top_advance((ORMatcher*)self, (ORMatcherIVARS*)ivars, target)) {
            if (!ivars->size) { return 0; }
        }
    } while (true);
}

int32_t
ORScorer_Get_Doc_ID_IMP(ORScorer *self) {
    return ORScorer_IVARS(self)->doc_id;
}

float
ORScorer_Score_IMP(ORScorer *self) {
    ORScorerIVARS *const ivars = ORScorer_IVARS(self);
    float *const scores = ivars->scores;
    float score = 0.0f;

    // Accumulate score, then factor in coord bonus.
    for (uint32_t i = 0; i < ivars->matching_kids; i++) {
        score += scores[i];
    }
    score *= ivars->coord_factors[ivars->matching_kids];

    return score;
}

