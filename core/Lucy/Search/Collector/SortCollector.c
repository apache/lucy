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

#define C_LUCY_SORTCOLLECTOR
#define C_LUCY_MATCHDOC
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Search/Collector/SortCollector.h"
#include "Lucy/Index/SegReader.h"
#include "Lucy/Index/SortCache.h"
#include "Lucy/Index/SortCache/NumericSortCache.h"
#include "Lucy/Index/SortCache/TextSortCache.h"
#include "Lucy/Index/SortReader.h"
#include "Lucy/Plan/FieldType.h"
#include "Lucy/Plan/Schema.h"
#include "Lucy/Search/HitQueue.h"
#include "Lucy/Search/MatchDoc.h"
#include "Lucy/Search/Matcher.h"
#include "Lucy/Search/SortRule.h"
#include "Lucy/Search/SortSpec.h"

#define COMPARE_BY_SCORE             0x1
#define COMPARE_BY_SCORE_REV         0x2
#define COMPARE_BY_DOC_ID            0x3
#define COMPARE_BY_DOC_ID_REV        0x4
#define COMPARE_BY_ORD1              0x5
#define COMPARE_BY_ORD1_REV          0x6
#define COMPARE_BY_ORD2              0x7
#define COMPARE_BY_ORD2_REV          0x8
#define COMPARE_BY_ORD4              0x9
#define COMPARE_BY_ORD4_REV          0xA
#define COMPARE_BY_ORD8              0xB
#define COMPARE_BY_ORD8_REV          0xC
#define COMPARE_BY_ORD16             0xD
#define COMPARE_BY_ORD16_REV         0xE
#define COMPARE_BY_ORD32             0xF
#define COMPARE_BY_ORD32_REV         0x10
#define COMPARE_BY_NATIVE_ORD16      0x11
#define COMPARE_BY_NATIVE_ORD16_REV  0x12
#define COMPARE_BY_NATIVE_ORD32      0x13
#define COMPARE_BY_NATIVE_ORD32_REV  0x14
#define AUTO_ACCEPT                  0x15
#define AUTO_REJECT                  0x16
#define AUTO_TIE                     0x17
#define ACTIONS_MASK                 0x1F

// Pick an action based on a SortRule and if needed, a SortCache.
static int8_t
S_derive_action(SortRule *rule, SortCache *sort_cache);

// Decide whether a doc should be inserted into the HitQueue.
static INLINE bool_t
SI_competitive(SortCollector *self, int32_t doc_id);

SortCollector*
SortColl_new(Schema *schema, SortSpec *sort_spec, uint32_t wanted) {
    SortCollector *self = (SortCollector*)VTable_Make_Obj(SORTCOLLECTOR);
    return SortColl_init(self, schema, sort_spec, wanted);
}

// Default to sort-by-score-then-doc-id.
static VArray*
S_default_sort_rules() {
    VArray *rules = VA_new(1);
    VA_Push(rules, (Obj*)SortRule_new(SortRule_SCORE, NULL, false));
    VA_Push(rules, (Obj*)SortRule_new(SortRule_DOC_ID, NULL, false));
    return rules;
}

SortCollector*
SortColl_init(SortCollector *self, Schema *schema, SortSpec *sort_spec,
              uint32_t wanted) {
    VArray *rules = sort_spec
                    ? (VArray*)INCREF(SortSpec_Get_Rules(sort_spec))
                    : S_default_sort_rules();
    uint32_t num_rules = VA_Get_Size(rules);
    uint32_t i;

    // Validate.
    if (sort_spec && !schema) {
        THROW(ERR, "Can't supply a SortSpec without a Schema.");
    }
    if (!num_rules) {
        THROW(ERR, "Can't supply a SortSpec with no SortRules.");
    }

    // Init.
    Coll_init((Collector*)self);
    self->total_hits    = 0;
    self->bubble_doc    = I32_MAX;
    self->bubble_score  = F32_NEGINF;
    self->seg_doc_max   = 0;

    // Assign.
    self->wanted        = wanted;

    // Derive.
    self->hit_q         = HitQ_new(schema, sort_spec, wanted);
    self->rules         = rules; // absorb refcount.
    self->num_rules     = num_rules;
    self->sort_caches   = (SortCache**)CALLOCATE(num_rules, sizeof(SortCache*));
    self->ord_arrays    = (void**)CALLOCATE(num_rules, sizeof(void*));
    self->actions       = (uint8_t*)CALLOCATE(num_rules, sizeof(uint8_t));

    // Build up an array of "actions" which we will execute during each call
    // to Collect(). Determine whether we need to track scores and field
    // values.
    self->need_score  = false;
    self->need_values = false;
    for (i = 0; i < num_rules; i++) {
        SortRule *rule   = (SortRule*)VA_Fetch(rules, i);
        int32_t rule_type  = SortRule_Get_Type(rule);
        self->actions[i] = S_derive_action(rule, NULL);
        if (rule_type == SortRule_SCORE) {
            self->need_score = true;
        }
        else if (rule_type == SortRule_FIELD) {
            CharBuf *field = SortRule_Get_Field(rule);
            FieldType *type = Schema_Fetch_Type(schema, field);
            if (!type || !FType_Sortable(type)) {
                THROW(ERR, "'%o' isn't a sortable field", field);
            }
            self->need_values = true;
        }
    }

    // Perform an optimization.  So long as we always collect docs in
    // ascending order, Collect() will favor lower doc numbers -- so we may
    // not need to execute a final COMPARE_BY_DOC_ID action.
    self->num_actions = num_rules;
    if (self->actions[num_rules - 1] == COMPARE_BY_DOC_ID) {
        self->num_actions--;
    }

    // Override our derived actions with an action which will be excecuted
    // autmatically until the queue fills up.
    self->auto_actions    = (uint8_t*)MALLOCATE(1);
    self->auto_actions[0] = wanted ? AUTO_ACCEPT : AUTO_REJECT;
    self->derived_actions = self->actions;
    self->actions         = self->auto_actions;


    // Prepare a MatchDoc-in-waiting.
    {
        VArray *values = self->need_values ? VA_new(num_rules) : NULL;
        float   score  = self->need_score  ? F32_NEGINF : F32_NAN;
        self->bumped = MatchDoc_new(I32_MAX, score, values);
        DECREF(values);
    }

    return self;
}

void
SortColl_destroy(SortCollector *self) {
    DECREF(self->hit_q);
    DECREF(self->rules);
    DECREF(self->bumped);
    FREEMEM(self->sort_caches);
    FREEMEM(self->ord_arrays);
    FREEMEM(self->auto_actions);
    FREEMEM(self->derived_actions);
    SUPER_DESTROY(self, SORTCOLLECTOR);
}

static int8_t
S_derive_action(SortRule *rule, SortCache *cache) {
    int32_t  rule_type = SortRule_Get_Type(rule);
    bool_t reverse   = !!SortRule_Get_Reverse(rule);

    if (rule_type == SortRule_SCORE) {
        return COMPARE_BY_SCORE + reverse;
    }
    else if (rule_type == SortRule_DOC_ID) {
        return COMPARE_BY_DOC_ID + reverse;
    }
    else if (rule_type == SortRule_FIELD) {
        if (cache) {
            int8_t width = SortCache_Get_Ord_Width(cache);
            switch (width) {
                case 1:  return COMPARE_BY_ORD1  + reverse;
                case 2:  return COMPARE_BY_ORD2  + reverse;
                case 4:  return COMPARE_BY_ORD4  + reverse;
                case 8:  return COMPARE_BY_ORD8  + reverse;
                case 16:
                    if (SortCache_Get_Native_Ords(cache)) {
                        return COMPARE_BY_NATIVE_ORD16 + reverse;
                    }
                    else {
                        return COMPARE_BY_ORD16 + reverse;
                    }
                case 32:
                    if (SortCache_Get_Native_Ords(cache)) {
                        return COMPARE_BY_NATIVE_ORD32 + reverse;
                    }
                    else {
                        return COMPARE_BY_ORD32 + reverse;
                    }
                default: THROW(ERR, "Unknown width: %i8", width);
            }
        }
        else {
            return AUTO_TIE;
        }
    }
    else {
        THROW(ERR, "Unrecognized SortRule type %i32", rule_type);
    }
    UNREACHABLE_RETURN(int8_t);
}

void
SortColl_set_reader(SortCollector *self, SegReader *reader) {
    SortReader *sort_reader
        = (SortReader*)SegReader_Fetch(reader, VTable_Get_Name(SORTREADER));

    // Reset threshold variables and trigger auto-action behavior.
    self->bumped->doc_id = I32_MAX;
    self->bubble_doc     = I32_MAX;
    self->bumped->score  = self->need_score ? F32_NEGINF : F32_NAN;
    self->bubble_score   = self->need_score ? F32_NEGINF : F32_NAN;
    self->actions        = self->auto_actions;

    // Obtain sort caches. Derive actions array for this segment.
    if (self->need_values && sort_reader) {
        uint32_t i, max;
        for (i = 0, max = self->num_rules; i < max; i++) {
            SortRule  *rule  = (SortRule*)VA_Fetch(self->rules, i);
            CharBuf   *field = SortRule_Get_Field(rule);
            SortCache *cache = field
                               ? SortReader_Fetch_Sort_Cache(sort_reader, field)
                               : NULL;
            self->sort_caches[i] = cache;
            self->derived_actions[i] = S_derive_action(rule, cache);
            if (cache) { self->ord_arrays[i] = SortCache_Get_Ords(cache); }
            else       { self->ord_arrays[i] = NULL; }
        }
    }
    self->seg_doc_max = reader ? SegReader_Doc_Max(reader) : 0;
    Coll_set_reader((Collector*)self, reader);
}

VArray*
SortColl_pop_match_docs(SortCollector *self) {
    return HitQ_Pop_All(self->hit_q);
}

uint32_t
SortColl_get_total_hits(SortCollector *self) {
    return self->total_hits;
}

bool_t
SortColl_need_score(SortCollector *self) {
    return self->need_score;
}

void
SortColl_collect(SortCollector *self, int32_t doc_id) {
    // Add to the total number of hits.
    self->total_hits++;

    // Collect this hit if it's competitive.
    if (SI_competitive(self, doc_id)) {
        MatchDoc *const match_doc = self->bumped;
        match_doc->doc_id = doc_id + self->base;

        if (self->need_score && match_doc->score == F32_NEGINF) {
            match_doc->score = Matcher_Score(self->matcher);
        }

        // Fetch values so that cross-segment sorting can work.
        if (self->need_values) {
            VArray *values = match_doc->values;
            uint32_t i, max;

            for (i = 0, max = self->num_rules; i < max; i++) {
                SortCache *cache   = self->sort_caches[i];
                Obj       *old_val = (Obj*)VA_Delete(values, i);
                if (cache) {
                    int32_t ord = SortCache_Ordinal(cache, doc_id);
                    Obj *blank = old_val
                                 ? old_val
                                 : SortCache_Make_Blank(cache);
                    Obj *val = SortCache_Value(cache, ord, blank);
                    if (val) { VA_Store(values, i, (Obj*)val); }
                    else     { DECREF(blank); }
                }
            }
        }

        // Insert the new MatchDoc.
        self->bumped = (MatchDoc*)HitQ_Jostle(self->hit_q, (Obj*)match_doc);

        if (self->bumped) {
            if (self->bumped == match_doc) {
                /* The queue is full, and we have established a threshold for
                 * this segment as to what sort of document is definitely not
                 * acceptable.  Turn off AUTO_ACCEPT and start actually
                 * testing whether hits are competitive. */
                self->bubble_score  = match_doc->score;
                self->bubble_doc    = doc_id;
                self->actions       = self->derived_actions;
            }

            // Recycle.
            self->bumped->score = self->need_score ? F32_NEGINF : F32_NAN;
        }
        else {
            // The queue isn't full yet, so create a fresh MatchDoc.
            VArray *values = self->need_values
                             ? VA_new(self->num_rules)
                             : NULL;
            float fake_score = self->need_score ? F32_NEGINF : F32_NAN;
            self->bumped = MatchDoc_new(I32_MAX, fake_score, values);
            DECREF(values);
        }

    }
}

static INLINE int32_t
SI_compare_by_ord1(SortCollector *self, uint32_t tick, int32_t a, int32_t b) {
    void *const ords = self->ord_arrays[tick];
    int32_t a_ord = NumUtil_u1get(ords, a);
    int32_t b_ord = NumUtil_u1get(ords, b);
    return a_ord - b_ord;
}
static INLINE int32_t
SI_compare_by_ord2(SortCollector *self, uint32_t tick, int32_t a, int32_t b) {
    void *const ords = self->ord_arrays[tick];
    int32_t a_ord = NumUtil_u2get(ords, a);
    int32_t b_ord = NumUtil_u2get(ords, b);
    return a_ord - b_ord;
}
static INLINE int32_t
SI_compare_by_ord4(SortCollector *self, uint32_t tick, int32_t a, int32_t b) {
    void *const ords = self->ord_arrays[tick];
    int32_t a_ord = NumUtil_u4get(ords, a);
    int32_t b_ord = NumUtil_u4get(ords, b);
    return a_ord - b_ord;
}
static INLINE int32_t
SI_compare_by_ord8(SortCollector *self, uint32_t tick, int32_t a, int32_t b) {
    uint8_t *ords = (uint8_t*)self->ord_arrays[tick];
    int32_t a_ord = ords[a];
    int32_t b_ord = ords[b];
    return a_ord - b_ord;
}
static INLINE int32_t
SI_compare_by_ord16(SortCollector *self, uint32_t tick, int32_t a, int32_t b) {
    uint8_t *ord_bytes = (uint8_t*)self->ord_arrays[tick];
    uint8_t *address_a = ord_bytes + a * sizeof(uint16_t);
    uint8_t *address_b = ord_bytes + b * sizeof(uint16_t);
    int32_t  ord_a = NumUtil_decode_bigend_u16(address_a);
    int32_t  ord_b = NumUtil_decode_bigend_u16(address_b);
    return ord_a - ord_b;
}
static INLINE int32_t
SI_compare_by_ord32(SortCollector *self, uint32_t tick, int32_t a, int32_t b) {
    uint8_t *ord_bytes = (uint8_t*)self->ord_arrays[tick];
    uint8_t *address_a = ord_bytes + a * sizeof(uint32_t);
    uint8_t *address_b = ord_bytes + b * sizeof(uint32_t);
    int32_t  ord_a = NumUtil_decode_bigend_u32(address_a);
    int32_t  ord_b = NumUtil_decode_bigend_u32(address_b);
    return ord_a - ord_b;
}
static INLINE int32_t
SI_compare_by_native_ord16(SortCollector *self, uint32_t tick,
                           int32_t a, int32_t b) {
    uint16_t *ords = (uint16_t*)self->ord_arrays[tick];
    int32_t a_ord = ords[a];
    int32_t b_ord = ords[b];
    return a_ord - b_ord;
}
static INLINE int32_t
SI_compare_by_native_ord32(SortCollector *self, uint32_t tick,
                           int32_t a, int32_t b) {
    int32_t *ords = (int32_t*)self->ord_arrays[tick];
    return ords[a] - ords[b];
}

// Bounds checking for doc id against the segment doc_max.  We assume that any
// sort cache ord arrays can accomodate lookups up to this number.
static INLINE int32_t
SI_validate_doc_id(SortCollector *self, int32_t doc_id) {
    // Check as uint32_t since we're using these doc ids as array indexes.
    if ((uint32_t)doc_id > (uint32_t)self->seg_doc_max) {
        THROW(ERR, "Doc ID %i32 greater than doc max %i32", doc_id,
              self->seg_doc_max);
    }
    return doc_id;
}

static INLINE bool_t
SI_competitive(SortCollector *self, int32_t doc_id) {
    /* Ordinarily, we would cache local copies of more member variables in
     * const automatic variables in order to improve code clarity and provide
     * more hints to the compiler about what variables are actually invariant
     * for the duration of this routine:
     *
     *     uint8_t *const actions    = self->actions;
     *     const uint32_t num_rules  = self->num_rules;
     *     const int32_t bubble_doc = self->bubble_doc;
     *
     * However, our major goal is to return as quickly as possible, and the
     * common case is that we'll have our answer before the first loop iter
     * finishes -- so we don't worry about the cost of performing extra
     * dereferencing on subsequent loop iters.
     *
     * The goal of returning quickly also drives the choice of a "do-while"
     * loop instead of a "for" loop, and the switch statement optimized for
     * compilation to a jump table.
     */
    uint8_t *const actions = self->actions;
    uint32_t i = 0;

    // Iterate through our array of actions, returning as quickly as possible.
    do {
        switch (actions[i] & ACTIONS_MASK) {
            case AUTO_ACCEPT:
                return true;
            case AUTO_REJECT:
                return false;
            case AUTO_TIE:
                break;
            case COMPARE_BY_SCORE: {
                    float score = Matcher_Score(self->matcher);
                    if (*(int32_t*)&score == *(int32_t*)&self->bubble_score) {
                        break;
                    }
                    if (score > self->bubble_score) {
                        self->bumped->score = score;
                        return true;
                    }
                    else if (score < self->bubble_score) {
                        return false;
                    }
                }
                break;
            case COMPARE_BY_SCORE_REV: {
                    float score = Matcher_Score(self->matcher);
                    if (*(int32_t*)&score == *(int32_t*)&self->bubble_score) {
                        break;
                    }
                    if (score < self->bubble_score) {
                        self->bumped->score = score;
                        return true;
                    }
                    else if (score > self->bubble_score) {
                        return false;
                    }
                }
                break;
            case COMPARE_BY_DOC_ID:
                if (doc_id > self->bubble_doc)      { return false; }
                else if (doc_id < self->bubble_doc) { return true; }
                break;
            case COMPARE_BY_DOC_ID_REV:
                if (doc_id > self->bubble_doc)      { return true; }
                else if (doc_id < self->bubble_doc) { return false; }
                break;
            case COMPARE_BY_ORD1: {
                    int32_t comparison
                        = SI_compare_by_ord1(
                              self, i, SI_validate_doc_id(self, doc_id),
                              self->bubble_doc);
                    if (comparison < 0)      { return true; }
                    else if (comparison > 0) { return false; }
                }
                break;
            case COMPARE_BY_ORD1_REV: {
                    int32_t comparison
                        = SI_compare_by_ord1(
                              self, i, self->bubble_doc,
                              SI_validate_doc_id(self, doc_id));
                    if (comparison < 0)      { return true; }
                    else if (comparison > 0) { return false; }
                }
                break;
            case COMPARE_BY_ORD2: {
                    int32_t comparison
                        = SI_compare_by_ord2(
                              self, i, SI_validate_doc_id(self, doc_id),
                              self->bubble_doc);
                    if (comparison < 0)      { return true; }
                    else if (comparison > 0) { return false; }
                }
                break;
            case COMPARE_BY_ORD2_REV: {
                    int32_t comparison
                        = SI_compare_by_ord2(
                              self, i, self->bubble_doc,
                              SI_validate_doc_id(self, doc_id));
                    if (comparison < 0)      { return true; }
                    else if (comparison > 0) { return false; }
                }
                break;
            case COMPARE_BY_ORD4: {
                    int32_t comparison
                        = SI_compare_by_ord4(
                              self, i, SI_validate_doc_id(self, doc_id),
                              self->bubble_doc);
                    if (comparison < 0)      { return true; }
                    else if (comparison > 0) { return false; }
                }
                break;
            case COMPARE_BY_ORD4_REV: {
                    int32_t comparison
                        = SI_compare_by_ord4(
                              self, i, self->bubble_doc,
                              SI_validate_doc_id(self, doc_id));
                    if (comparison < 0)      { return true; }
                    else if (comparison > 0) { return false; }
                }
                break;
            case COMPARE_BY_ORD8: {
                    int32_t comparison
                        = SI_compare_by_ord8(
                              self, i, SI_validate_doc_id(self, doc_id),
                              self->bubble_doc);
                    if (comparison < 0)      { return true; }
                    else if (comparison > 0) { return false; }
                }
                break;
            case COMPARE_BY_ORD8_REV: {
                    int32_t comparison
                        = SI_compare_by_ord8(
                              self, i, self->bubble_doc,
                              SI_validate_doc_id(self, doc_id));
                    if (comparison < 0)      { return true; }
                    else if (comparison > 0) { return false; }
                }
                break;
            case COMPARE_BY_ORD16: {
                    int32_t comparison
                        = SI_compare_by_ord16(
                              self, i, SI_validate_doc_id(self, doc_id),
                              self->bubble_doc);
                    if (comparison < 0)      { return true; }
                    else if (comparison > 0) { return false; }
                }
                break;
            case COMPARE_BY_ORD16_REV: {
                    int32_t comparison
                        = SI_compare_by_ord16(
                              self, i, self->bubble_doc,
                              SI_validate_doc_id(self, doc_id));
                    if (comparison < 0)      { return true; }
                    else if (comparison > 0) { return false; }
                }
                break;
            case COMPARE_BY_ORD32: {
                    int32_t comparison
                        = SI_compare_by_ord32(
                              self, i, SI_validate_doc_id(self, doc_id),
                              self->bubble_doc);
                    if (comparison < 0)      { return true; }
                    else if (comparison > 0) { return false; }
                }
                break;
            case COMPARE_BY_ORD32_REV: {
                    int32_t comparison
                        = SI_compare_by_ord32(
                              self, i, self->bubble_doc,
                              SI_validate_doc_id(self, doc_id));
                    if (comparison < 0)      { return true; }
                    else if (comparison > 0) { return false; }
                }
                break;
            case COMPARE_BY_NATIVE_ORD16: {
                    int32_t comparison
                        = SI_compare_by_native_ord16(
                              self, i, SI_validate_doc_id(self, doc_id),
                              self->bubble_doc);
                    if (comparison < 0)      { return true; }
                    else if (comparison > 0) { return false; }
                }
                break;
            case COMPARE_BY_NATIVE_ORD16_REV: {
                    int32_t comparison
                        = SI_compare_by_native_ord16(
                              self, i, self->bubble_doc,
                              SI_validate_doc_id(self, doc_id));
                    if (comparison < 0)      { return true; }
                    else if (comparison > 0) { return false; }
                }
                break;
            case COMPARE_BY_NATIVE_ORD32: {
                    int32_t comparison
                        = SI_compare_by_native_ord32(
                              self, i, SI_validate_doc_id(self, doc_id),
                              self->bubble_doc);
                    if (comparison < 0)      { return true; }
                    else if (comparison > 0) { return false; }
                }
                break;
            case COMPARE_BY_NATIVE_ORD32_REV: {
                    int32_t comparison
                        = SI_compare_by_native_ord32(
                              self, i, self->bubble_doc,
                              SI_validate_doc_id(self, doc_id));
                    if (comparison < 0)      { return true; }
                    else if (comparison > 0) { return false; }
                }
                break;
            default:
                THROW(ERR, "UNEXPECTED action %u8", actions[i]);
        }
    } while (++i < self->num_actions);

    // If we've made it this far and we're still tied, reject the doc so that
    // we prefer items already in the queue.  This has the effect of
    // implicitly breaking ties by doc num, since docs are collected in order.
    return false;
}


