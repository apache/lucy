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
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Highlight/HeatMap.h"
#include "Lucy/Search/Span.h"
#include "Clownfish/Util/SortUtils.h"

HeatMap*
HeatMap_new(VArray *spans, uint32_t window) {
    HeatMap *self = (HeatMap*)Class_Make_Obj(HEATMAP);
    return HeatMap_init(self, spans, window);
}

HeatMap*
HeatMap_init(HeatMap *self, VArray *spans, uint32_t window) {
    HeatMapIVARS *const ivars = HeatMap_IVARS(self);
    VArray *spans_copy = VA_Shallow_Copy(spans);
    VArray *spans_plus_boosts;

    ivars->spans  = NULL;
    ivars->window = window;

    VA_Sort(spans_copy, NULL, NULL);
    spans_plus_boosts = HeatMap_Generate_Proximity_Boosts(self, spans_copy);
    VA_Push_VArray(spans_plus_boosts, spans_copy);
    VA_Sort(spans_plus_boosts, NULL, NULL);
    ivars->spans = HeatMap_Flatten_Spans(self, spans_plus_boosts);

    DECREF(spans_plus_boosts);
    DECREF(spans_copy);

    return self;
}

void
HeatMap_Destroy_IMP(HeatMap *self) {
    HeatMapIVARS *const ivars = HeatMap_IVARS(self);
    DECREF(ivars->spans);
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
        bounds[i]             = Span_Get_Offset(span);
        bounds[i + num_spans] = Span_Get_Offset(span) + Span_Get_Length(span);
    }
    Sort_quicksort(bounds, num_spans * 2, sizeof(uint32_t),
                   S_compare_i32, NULL);
    uint32_t num_bounds = 0;
    int32_t  last       = INT32_MAX;
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
HeatMap_Flatten_Spans_IMP(HeatMap *self, VArray *spans) {
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
            int32_t source_span_offset = Span_Get_Offset(source_span);
            int32_t source_span_len    = Span_Get_Length(source_span);
            int32_t source_span_end    = source_span_offset + source_span_len;

            // Get the location of the flattened span that shares the source
            // span's offset.
            for (; dest_tick < num_raw_flattened; dest_tick++) {
                Span *dest_span = (Span*)VA_Fetch(flattened, dest_tick);
                if (Span_Get_Offset(dest_span) == source_span_offset) {
                    break;
                }
            }

            // Fill in scores.
            for (uint32_t j = dest_tick; j < num_raw_flattened; j++) {
                Span *dest_span = (Span*)VA_Fetch(flattened, j);
                if (Span_Get_Offset(dest_span) == source_span_end) {
                    break;
                }
                else {
                    float new_weight = Span_Get_Weight(dest_span)
                                       + Span_Get_Weight(source_span);
                    Span_Set_Weight(dest_span, new_weight);
                }
            }
        }

        // Leave holes instead of spans that don't have any score.
        dest_tick = 0;
        for (uint32_t i = 0; i < num_raw_flattened; i++) {
            Span *span = (Span*)VA_Fetch(flattened, i);
            if (Span_Get_Weight(span)) {
                VA_Store(flattened, dest_tick++, INCREF(span));
            }
        }
        VA_Excise(flattened, dest_tick, num_raw_flattened - dest_tick);

        return flattened;
    }
}

float
HeatMap_Calc_Proximity_Boost_IMP(HeatMap *self, Span *span1, Span *span2) {
    HeatMapIVARS *const ivars = HeatMap_IVARS(self);
    int32_t comparison = Span_Compare_To(span1, (Obj*)span2);
    Span *lower = comparison <= 0 ? span1 : span2;
    Span *upper = comparison >= 0 ? span1 : span2;
    int32_t lower_end_offset = Span_Get_Offset(lower) + Span_Get_Length(lower);
    int32_t distance = Span_Get_Offset(upper) - lower_end_offset;

    // If spans overlap, set distance to 0.
    if (distance < 0) { distance = 0; }

    if (distance > (int32_t)ivars->window) {
        return 0.0f;
    }
    else {
        float factor = (ivars->window - distance) / (float)ivars->window;
        // Damp boost with greater distance.
        factor *= factor;
        return factor * (Span_Get_Weight(lower) + Span_Get_Weight(upper));
    }
}

VArray*
HeatMap_Generate_Proximity_Boosts_IMP(HeatMap *self, VArray *spans) {
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
                    int32_t length = Span_Get_Offset(span2)
                                     - Span_Get_Offset(span1)
                                     + Span_Get_Length(span2);
                    VA_Push(boosts,
                            (Obj*)Span_new(Span_Get_Offset(span1), length,
                                           prox_score));
                }
            }
        }
    }

    return boosts;
}

VArray*
HeatMap_Get_Spans_IMP(HeatMap *self) {
    return HeatMap_IVARS(self)->spans;
}


