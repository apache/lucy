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

#define C_LUCY_HEATMAP
#define C_LUCY_SPAN
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Highlight/HeatMap.h"
#include "Lucy/Search/Span.h"
#include "Lucy/Util/SortUtils.h"

HeatMap*
HeatMap_new(VArray *spans, uint32_t window) {
    HeatMap *self = (HeatMap*)VTable_Make_Obj(HEATMAP);
    return HeatMap_init(self, spans, window);
}

HeatMap*
HeatMap_init(HeatMap *self, VArray *spans, uint32_t window) {
    VArray *spans_copy = VA_Shallow_Copy(spans);
    VArray *spans_plus_boosts;

    self->spans  = NULL;
    self->window = window;

    VA_Sort(spans_copy, NULL, NULL);
    spans_plus_boosts = HeatMap_Generate_Proximity_Boosts(self, spans_copy);
    VA_Push_VArray(spans_plus_boosts, spans_copy);
    VA_Sort(spans_plus_boosts, NULL, NULL);
    self->spans = HeatMap_Flatten_Spans(self, spans_plus_boosts);

    DECREF(spans_plus_boosts);
    DECREF(spans_copy);

    return self;
}

void
HeatMap_destroy(HeatMap *self) {
    DECREF(self->spans);
    SUPER_DESTROY(self, HEATMAP);
}

static int
S_compare_i32(void *context, const void *va, const void *vb) {
    UNUSED_VAR(context);
    return *(int32_t*)va - *(int32_t*)vb;
}

// Create all the spans needed by HeatMap_Flatten_Spans, based on the source
// offsets and lengths... but leave the scores at 0.
static VArray*
S_flattened_but_empty_spans(VArray *spans) {
    const uint32_t num_spans = VA_Get_Size(spans);
    int32_t *bounds = (int32_t*)MALLOCATE((num_spans * 2) * sizeof(int32_t));

    // Assemble a list of all unique start/end boundaries.
    for (uint32_t i = 0; i < num_spans; i++) {
        Span *span            = (Span*)VA_Fetch(spans, i);
        bounds[i]             = span->offset;
        bounds[i + num_spans] = span->offset + span->length;
    }
    Sort_quicksort(bounds, num_spans * 2, sizeof(uint32_t),
                   S_compare_i32, NULL);
    uint32_t num_bounds = 0;
    int32_t  last       = I32_MAX;
    for (uint32_t i = 0; i < num_spans * 2; i++) {
        if (bounds[i] != last) {
            bounds[num_bounds++] = bounds[i];
            last = bounds[i];
        }
    }

    // Create one Span for each zone between two bounds.
    VArray *flattened = VA_new(num_bounds - 1);
    for (uint32_t i = 0; i < num_bounds - 1; i++) {
        int32_t  start   = bounds[i];
        int32_t  length  = bounds[i + 1] - start;
        VA_Push(flattened, (Obj*)Span_new(start, length, 0.0f));
    }

    FREEMEM(bounds);
    return flattened;
}

VArray*
HeatMap_flatten_spans(HeatMap *self, VArray *spans) {
    const uint32_t num_spans = VA_Get_Size(spans);
    UNUSED_VAR(self);

    if (!num_spans) {
        return VA_new(0);
    }
    else {
        VArray *flattened = S_flattened_but_empty_spans(spans);
        const uint32_t num_raw_flattened = VA_Get_Size(flattened);

        // Iterate over each of the source spans, contributing their scores to
        // any destination span that falls within range.
        uint32_t dest_tick = 0;
        for (uint32_t i = 0; i < num_spans; i++) {
            Span *source_span = (Span*)VA_Fetch(spans, i);
            int32_t source_span_end
                = source_span->offset + source_span->length;

            // Get the location of the flattened span that shares the source
            // span's offset.
            for (; dest_tick < num_raw_flattened; dest_tick++) {
                Span *dest_span = (Span*)VA_Fetch(flattened, dest_tick);
                if (dest_span->offset == source_span->offset) {
                    break;
                }
            }

            // Fill in scores.
            for (uint32_t j = dest_tick; j < num_raw_flattened; j++) {
                Span *dest_span = (Span*)VA_Fetch(flattened, j);
                if (dest_span->offset == source_span_end) {
                    break;
                }
                else {
                    dest_span->weight += source_span->weight;
                }
            }
        }

        // Leave holes instead of spans that don't have any score.
        dest_tick = 0;
        for (uint32_t i = 0; i < num_raw_flattened; i++) {
            Span *span = (Span*)VA_Fetch(flattened, i);
            if (span->weight) {
                VA_Store(flattened, dest_tick++, INCREF(span));
            }
        }
        VA_Excise(flattened, dest_tick, num_raw_flattened - dest_tick);

        return flattened;
    }
}

float
HeatMap_calc_proximity_boost(HeatMap *self, Span *span1, Span *span2) {
    int32_t comparison = Span_Compare_To(span1, (Obj*)span2);
    Span *lower = comparison <= 0 ? span1 : span2;
    Span *upper = comparison >= 0 ? span1 : span2;
    int32_t lower_end_offset = lower->offset + lower->length;
    int32_t distance = upper->offset - lower_end_offset;

    // If spans overlap, set distance to 0.
    if (distance < 0) { distance = 0; }

    if (distance > (int32_t)self->window) {
        return 0.0f;
    }
    else {
        float factor = (self->window - distance) / (float)self->window;
        // Damp boost with greater distance.
        factor *= factor;
        return factor * (lower->weight + upper->weight);
    }
}

VArray*
HeatMap_generate_proximity_boosts(HeatMap *self, VArray *spans) {
    VArray *boosts = VA_new(0);
    const uint32_t num_spans = VA_Get_Size(spans);

    if (num_spans > 1) {
        for (uint32_t i = 0, max = num_spans - 1; i < max; i++) {
            Span *span1 = (Span*)VA_Fetch(spans, i);

            for (uint32_t j = i + 1; j <= max; j++) {
                Span *span2 = (Span*)VA_Fetch(spans, j);
                float prox_score
                    = HeatMap_Calc_Proximity_Boost(self, span1, span2);
                if (prox_score == 0) {
                    break;
                }
                else {
                    int32_t length = (span2->offset - span1->offset)
                                     + span2->length;
                    VA_Push(boosts,
                            (Obj*)Span_new(span1->offset, length, prox_score));
                }
            }
        }
    }

    return boosts;
}

VArray*
HeatMap_get_spans(HeatMap *self) {
    return self->spans;
}


