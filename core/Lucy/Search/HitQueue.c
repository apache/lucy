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

#define C_LUCY_HITQUEUE
#define C_LUCY_MATCHDOC
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Search/HitQueue.h"
#include "Lucy/Index/SegReader.h"
#include "Lucy/Index/SortCache.h"
#include "Lucy/Plan/FieldType.h"
#include "Lucy/Plan/Schema.h"
#include "Lucy/Search/MatchDoc.h"
#include "Lucy/Search/SortRule.h"
#include "Lucy/Search/SortSpec.h"


#define COMPARE_BY_SCORE      1
#define COMPARE_BY_SCORE_REV  2
#define COMPARE_BY_DOC_ID     3
#define COMPARE_BY_DOC_ID_REV 4
#define COMPARE_BY_VALUE      5
#define COMPARE_BY_VALUE_REV  6
#define ACTIONS_MASK          0xF

HitQueue*
HitQ_new(Schema *schema, SortSpec *sort_spec, uint32_t wanted) {
    HitQueue *self = (HitQueue*)Class_Make_Obj(HITQUEUE);
    return HitQ_init(self, schema, sort_spec, wanted);
}

HitQueue*
HitQ_init(HitQueue *self, Schema *schema, SortSpec *sort_spec,
          uint32_t wanted) {
    HitQueueIVARS *const ivars = HitQ_IVARS(self);
    if (sort_spec) {
        VArray   *rules      = SortSpec_Get_Rules(sort_spec);
        uint32_t  num_rules  = VA_Get_Size(rules);
        uint32_t  action_num = 0;

        if (!schema) {
            THROW(ERR, "Can't supply sort_spec without schema");
        }

        ivars->need_values = false;
        ivars->num_actions = num_rules;
        ivars->actions     = (uint8_t*)MALLOCATE(num_rules * sizeof(uint8_t));
        ivars->field_types = (FieldType**)CALLOCATE(num_rules, sizeof(FieldType*));

        for (uint32_t i = 0; i < num_rules; i++) {
            SortRule *rule      = (SortRule*)VA_Fetch(rules, i);
            int32_t   rule_type = SortRule_Get_Type(rule);
            bool      reverse   = SortRule_Get_Reverse(rule);

            if (rule_type == SortRule_SCORE) {
                ivars->actions[action_num++] = reverse
                                              ? COMPARE_BY_SCORE_REV
                                              : COMPARE_BY_SCORE;
            }
            else if (rule_type == SortRule_DOC_ID) {
                ivars->actions[action_num++] = reverse
                                              ? COMPARE_BY_DOC_ID_REV
                                              : COMPARE_BY_DOC_ID;
            }
            else if (rule_type == SortRule_FIELD) {
                String    *field = SortRule_Get_Field(rule);
                FieldType *type  = Schema_Fetch_Type(schema, field);
                if (type) {
                    ivars->field_types[action_num] = (FieldType*)INCREF(type);
                    ivars->actions[action_num++] = reverse
                                                  ? COMPARE_BY_VALUE_REV
                                                  : COMPARE_BY_VALUE;
                    ivars->need_values = true;
                }
                else {
                    // Skip over fields we don't know how to sort on.
                    continue;
                }
            }
            else {
                THROW(ERR, "Unknown SortRule type: %i32", rule_type);
            }
        }
    }
    else {
        ivars->num_actions = 2;
        ivars->actions     = (uint8_t*)MALLOCATE(ivars->num_actions * sizeof(uint8_t));
        ivars->actions[0]  = COMPARE_BY_SCORE;
        ivars->actions[1]  = COMPARE_BY_DOC_ID;
    }

    return (HitQueue*)PriQ_init((PriorityQueue*)self, wanted);
}

void
HitQ_Destroy_IMP(HitQueue *self) {
    HitQueueIVARS *const ivars = HitQ_IVARS(self);
    FieldType **types = ivars->field_types;
    FieldType **const limit = types + ivars->num_actions - 1;
    for (; types < limit; types++) {
        if (types) { DECREF(*types); }
    }
    FREEMEM(ivars->actions);
    FREEMEM(ivars->field_types);
    SUPER_DESTROY(self, HITQUEUE);
}

Obj*
HitQ_Jostle_IMP(HitQueue *self, Obj *element) {
    HitQueueIVARS *const ivars = HitQ_IVARS(self);
    MatchDoc *match_doc = (MatchDoc*)CERTIFY(element, MATCHDOC);
    HitQ_Jostle_t super_jostle
        = SUPER_METHOD_PTR(HITQUEUE, LUCY_HitQ_Jostle);
    if (ivars->need_values) {
        MatchDocIVARS *const match_doc_ivars = MatchDoc_IVARS(match_doc);
        CERTIFY(match_doc_ivars->values, VARRAY);
    }
    return super_jostle(self, element);
}

static CFISH_INLINE int32_t
SI_compare_by_value(HitQueueIVARS *ivars, uint32_t tick,
                    MatchDocIVARS *a_ivars, MatchDocIVARS *b_ivars) {
    Obj *a_val = VA_Fetch(a_ivars->values, tick);
    Obj *b_val = VA_Fetch(b_ivars->values, tick);
    FieldType *field_type = ivars->field_types[tick];
    return FType_null_back_compare_values(field_type, a_val, b_val);
}

bool
HitQ_Less_Than_IMP(HitQueue *self, Obj *obj_a, Obj *obj_b) {
    HitQueueIVARS *const ivars = HitQ_IVARS(self);
    MatchDoc *const a = (MatchDoc*)obj_a;
    MatchDoc *const b = (MatchDoc*)obj_b;
    MatchDocIVARS *a_ivars = MatchDoc_IVARS(a);
    MatchDocIVARS *b_ivars = MatchDoc_IVARS(b);
    uint32_t i = 0;
    uint8_t *const actions = ivars->actions;

    do {
        switch (actions[i] & ACTIONS_MASK) {
            case COMPARE_BY_SCORE:
                // Prefer high scores.
                if (a_ivars->score > b_ivars->score)      { return false; }
                else if (a_ivars->score < b_ivars->score) { return true;  }
                break;
            case COMPARE_BY_SCORE_REV:
                if (a_ivars->score > b_ivars->score)      { return true;  }
                else if (a_ivars->score < b_ivars->score) { return false; }
                break;
            case COMPARE_BY_DOC_ID:
                // Prefer low doc ids.
                if (a_ivars->doc_id > b_ivars->doc_id)      { return true;  }
                else if (a_ivars->doc_id < b_ivars->doc_id) { return false; }
                break;
            case COMPARE_BY_DOC_ID_REV:
                if (a_ivars->doc_id > b_ivars->doc_id)      { return false; }
                else if (a_ivars->doc_id < b_ivars->doc_id) { return true;  }
                break;
            case COMPARE_BY_VALUE: {
                    int32_t comparison
                        = SI_compare_by_value(ivars, i, a_ivars, b_ivars);
                    if (comparison > 0)      { return true;  }
                    else if (comparison < 0) { return false; }
                }
                break;
            case COMPARE_BY_VALUE_REV: {
                    int32_t comparison
                        = SI_compare_by_value(ivars, i, b_ivars, a_ivars);
                    if (comparison > 0)      { return true;  }
                    else if (comparison < 0) { return false; }
                }
                break;
            default:
                THROW(ERR, "Unexpected action %u8", actions[i]);
        }

    } while (++i < ivars->num_actions);

    return false;
}


