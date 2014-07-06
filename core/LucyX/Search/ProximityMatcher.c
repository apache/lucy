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

#define C_LUCY_PROXIMITYMATCHER
#define C_LUCY_POSTING
#define C_LUCY_SCOREPOSTING
#include "Lucy/Util/ToolSet.h"

#include "LucyX/Search/ProximityMatcher.h"
#include "Lucy/Index/Posting/ScorePosting.h"
#include "Lucy/Index/PostingList.h"
#include "Lucy/Index/Similarity.h"
#include "Lucy/Search/Compiler.h"


ProximityMatcher*
ProximityMatcher_new(Similarity *sim, VArray *plists, Compiler *compiler,
                     uint32_t within) {
    ProximityMatcher *self =
        (ProximityMatcher*)Class_Make_Obj(PROXIMITYMATCHER);
    return ProximityMatcher_init(self, sim, plists, compiler, within);

}

ProximityMatcher*
ProximityMatcher_init(ProximityMatcher *self, Similarity *similarity,
                      VArray *plists, Compiler *compiler, uint32_t within) {
    Matcher_init((Matcher*)self);
    ProximityMatcherIVARS *const ivars = ProximityMatcher_IVARS(self);

    // Init.
    ivars->anchor_set       = BB_new(0);
    ivars->proximity_freq   = 0.0;
    ivars->proximity_boost  = 0.0;
    ivars->first_time       = true;
    ivars->more             = true;
    ivars->within           = within;

    // Extract PostingLists out of VArray into local C array for quick access.
    ivars->num_elements = VA_Get_Size(plists);
    ivars->plists = (PostingList**)MALLOCATE(
                       ivars->num_elements * sizeof(PostingList*));
    for (size_t i = 0; i < ivars->num_elements; i++) {
        PostingList *const plist
            = (PostingList*)CERTIFY(VA_Fetch(plists, i), POSTINGLIST);
        if (plist == NULL) {
            THROW(ERR, "Missing element %u32", i);
        }
        ivars->plists[i] = (PostingList*)INCREF(plist);
    }

    // Assign.
    ivars->sim       = (Similarity*)INCREF(similarity);
    ivars->compiler  = (Compiler*)INCREF(compiler);
    ivars->weight    = Compiler_Get_Weight(compiler);

    return self;
}

void
ProximityMatcher_Destroy_IMP(ProximityMatcher *self) {
    ProximityMatcherIVARS *const ivars = ProximityMatcher_IVARS(self);
    if (ivars->plists) {
        for (size_t i = 0; i < ivars->num_elements; i++) {
            DECREF(ivars->plists[i]);
        }
        FREEMEM(ivars->plists);
    }
    DECREF(ivars->sim);
    DECREF(ivars->anchor_set);
    DECREF(ivars->compiler);
    SUPER_DESTROY(self, PROXIMITYMATCHER);
}

int32_t
ProximityMatcher_Next_IMP(ProximityMatcher *self) {
    ProximityMatcherIVARS *const ivars = ProximityMatcher_IVARS(self);
    if (ivars->first_time) {
        return ProximityMatcher_Advance(self, 1);
    }
    else if (ivars->more) {
        const int32_t target = PList_Get_Doc_ID(ivars->plists[0]) + 1;
        return ProximityMatcher_Advance(self, target);
    }
    else {
        return 0;
    }
}

int32_t
ProximityMatcher_Advance_IMP(ProximityMatcher *self, int32_t target) {
    ProximityMatcherIVARS *const ivars = ProximityMatcher_IVARS(self);
    PostingList **const plists       = ivars->plists;
    const uint32_t      num_elements = ivars->num_elements;
    int32_t             highest      = 0;

    // Reset match variables to indicate no match.  New values will be
    // assigned if a match succeeds.
    ivars->proximity_freq = 0.0;
    ivars->doc_id         = 0;

    // Find the lowest possible matching doc ID greater than the current doc
    // ID.  If any one of the PostingLists is exhausted, we're done.
    if (ivars->first_time) {
        ivars->first_time = false;

        // On the first call to Advance(), advance all PostingLists.
        for (size_t i = 0, max = ivars->num_elements; i < max; i++) {
            int32_t candidate = PList_Advance(plists[i], target);
            if (!candidate) {
                ivars->more = false;
                return 0;
            }
            else if (candidate > highest) {
                // Remember the highest doc ID so far.
                highest = candidate;
            }
        }
    }
    else {
        // On subsequent iters, advance only one PostingList.  Its new doc ID
        // becomes the minimum target which all the others must move up to.
        highest = PList_Advance(plists[0], target);
        if (highest == 0) {
            ivars->more = false;
            return 0;
        }
    }

    // Find a doc which contains all the terms.
    while (1) {
        bool agreement = true;

        // Scoot all posting lists up to at least the current minimum.
        for (uint32_t i = 0; i < num_elements; i++) {
            PostingList *const plist = plists[i];
            int32_t candidate = PList_Get_Doc_ID(plist);

            // Is this PostingList already beyond the minimum?  Then raise the
            // bar for everyone else.
            if (highest < candidate) { highest = candidate; }
            if (target < highest)    { target = highest; }

            // Scoot this posting list up.
            if (candidate < target) {
                candidate = PList_Advance(plist, target);

                // If this PostingList is exhausted, we're done.
                if (candidate == 0) {
                    ivars->more = false;
                    return 0;
                }

                // After calling PList_Advance(), we are guaranteed to be
                // either at or beyond the minimum, so we can assign without
                // checking and the minumum will either go up or stay the
                // same.
                highest = candidate;
            }
        }

        // See whether all the PostingLists have managed to converge on a
        // single doc ID.
        for (uint32_t i = 0; i < num_elements; i++) {
            const int32_t candidate = PList_Get_Doc_ID(plists[i]);
            if (candidate != highest) { agreement = false; }
        }

        // If we've found a doc with all terms in it, see if they form a
        // phrase.
        if (agreement && highest >= target) {
            ivars->proximity_freq = ProximityMatcher_Calc_Proximity_Freq(self);
            if (ivars->proximity_freq == 0.0) {
                // No phrase.  Move on to another doc.
                target += 1;
            }
            else {
                // Success!
                ivars->doc_id = highest;
                return highest;
            }
        }
    }
}


static CFISH_INLINE uint32_t
SI_winnow_anchors(uint32_t *anchors_start, const uint32_t *const anchors_end,
                  const uint32_t *candidates, const uint32_t *const candidates_end,
                  uint32_t offset, uint32_t within) {
    uint32_t *anchors = anchors_start;
    uint32_t *anchors_found = anchors_start;
    uint32_t target_anchor;
    uint32_t target_candidate;

    // Safety check, so there's no chance of a bad dereference.
    if (anchors_start == anchors_end || candidates == candidates_end) {
        return 0;
    }

    /* This function is a loop that finds terms that can continue a phrase.
     * It overwrites the anchors in place, and returns the number remaining.
     * The basic algorithm is to alternately increment the candidates' pointer
     * until it is at or beyond its target position, and then increment the
     * anchors' pointer until it is at or beyond its target.  The non-standard
     * form is to avoid unnecessary comparisons.  This loop has not been
     * tested for speed, but glancing at the object code produced (objdump -S)
     * it appears to be significantly faster than the nested loop alternative.
     * But given the vagaries of modern processors, it merits actual
     * testing.*/

SPIN_CANDIDATES:
    target_candidate = *anchors + offset;
    while (*candidates < target_candidate) {
        if (++candidates == candidates_end) { goto DONE; }
    }
    if ((*candidates - target_candidate) < within) { goto MATCH; }
    goto SPIN_ANCHORS;

SPIN_ANCHORS:
    target_anchor = *candidates - offset;
    while (*anchors < target_anchor) {
        if (++anchors == anchors_end) { goto DONE; }
    };
    if (*anchors == target_anchor) { goto MATCH; }
    goto SPIN_CANDIDATES;

MATCH:
    *anchors_found++ = *anchors;
    if (++anchors == anchors_end) { goto DONE; }
    goto SPIN_CANDIDATES;

DONE:
    // Return number of anchors remaining.
    return anchors_found - anchors_start;
}

float
ProximityMatcher_Calc_Proximity_Freq_IMP(ProximityMatcher *self) {
    ProximityMatcherIVARS *const ivars = ProximityMatcher_IVARS(self);
    PostingList **const plists = ivars->plists;

    /* Create a overwriteable "anchor set" from the first posting.
     *
     * Each "anchor" is a position, measured in tokens, corresponding to a a
     * term which might start a phrase.  We start off with an "anchor set"
     * comprised of all positions at which the first term in the phrase occurs
     * in the field.
     *
     * There can never be more proximity matches than instances of this first
     * term.  There may be fewer however, which we will determine by seeing
     * whether all the other terms line up at subsequent position slots.
     *
     * Every time we eliminate an anchor from the anchor set, we splice it out
     * of the array.  So if we begin with an anchor set of (15, 51, 72) and we
     * discover that matches occur at the first and last instances of the
     * first term but not the middle one, the final array will be (15, 72).
     *
     * The number of elements in the anchor set when we are finished winnowing
     * is our proximity freq.
     */
    ScorePosting *posting = (ScorePosting*)PList_Get_Posting(plists[0]);
    ScorePostingIVARS *const post_ivars = ScorePost_IVARS(posting);
    uint32_t anchors_remaining = post_ivars->freq;
    if (!anchors_remaining) { return 0.0f; }

    size_t    amount        = anchors_remaining * sizeof(uint32_t);
    uint32_t *anchors_start = (uint32_t*)BB_Grow(ivars->anchor_set, amount);
    uint32_t *anchors_end   = anchors_start + anchors_remaining;
    memcpy(anchors_start, post_ivars->prox, amount);

    // Match the positions of other terms against the anchor set.
    for (uint32_t i = 1, max = ivars->num_elements; i < max; i++) {
        // Get the array of positions for the next term.  Unlike the anchor
        // set (which is a copy), these won't be overwritten.
        ScorePosting *next_post = (ScorePosting*)PList_Get_Posting(plists[i]);
        ScorePostingIVARS *const next_post_ivars = ScorePost_IVARS(next_post);
        uint32_t *candidates_start = next_post_ivars->prox;
        uint32_t *candidates_end   = candidates_start + next_post_ivars->freq;

        // Splice out anchors that don't match the next term.  Bail out if
        // we've eliminated all possible anchors.
        if (ivars->within == 1) { // exact phrase match
            anchors_remaining = SI_winnow_anchors(anchors_start, anchors_end,
                                                  candidates_start,
                                                  candidates_end, i, 1);
        }
        else {  // fuzzy-phrase match
            anchors_remaining = SI_winnow_anchors(anchors_start, anchors_end,
                                                  candidates_start,
                                                  candidates_end, i,
                                                  ivars->within);
        }
        if (!anchors_remaining) { return 0.0f; }

        // Adjust end for number of anchors that remain.
        anchors_end = anchors_start + anchors_remaining;
    }

    // The number of anchors left is the proximity freq.
    return (float)anchors_remaining;
}

int32_t
ProximityMatcher_Get_Doc_ID_IMP(ProximityMatcher *self) {
    return ProximityMatcher_IVARS(self)->doc_id;
}

float
ProximityMatcher_Score_IMP(ProximityMatcher *self) {
    ProximityMatcherIVARS *const ivars = ProximityMatcher_IVARS(self);
    ScorePosting *posting = (ScorePosting*)PList_Get_Posting(ivars->plists[0]);
    float score = Sim_TF(ivars->sim, ivars->proximity_freq)
                  * ivars->weight
                  * ScorePost_IVARS(posting)->weight;
    return score;
}


