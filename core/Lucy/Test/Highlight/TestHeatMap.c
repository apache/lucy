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

#define TESTLUCY_USE_SHORT_NAMES
#include "Lucy/Util/ToolSet.h"

#include "Clownfish/TestHarness/TestBatchRunner.h"
#include "Lucy/Test.h"
#include "Lucy/Test/Highlight/TestHeatMap.h"
#include "Lucy/Highlight/HeatMap.h"

#include "Lucy/Search/Span.h"

TestHeatMap*
TestHeatMap_new() {
    return (TestHeatMap*)Class_Make_Obj(TESTHEATMAP);
}

static void
test_calc_proximity_boost(TestBatchRunner *runner) {
    VArray  *spans    = VA_new(0);
    HeatMap *heat_map = HeatMap_new(spans, 133);
    Span    *span1    = Span_new(  0, 10, 1.0f);
    Span    *span2    = Span_new( 10, 10, 1.0f);
    Span    *span3    = Span_new(  5,  4, 1.0f);
    Span    *span4    = Span_new(100, 10, 1.0f);
    Span    *span5    = Span_new(150, 10, 1.0f);

    float big_boost     = HeatMap_Calc_Proximity_Boost(heat_map, span1, span2);
    float eq_big_boost  = HeatMap_Calc_Proximity_Boost(heat_map, span1, span3);
    float smaller_boost = HeatMap_Calc_Proximity_Boost(heat_map, span1, span4);
    float zero_boost    = HeatMap_Calc_Proximity_Boost(heat_map, span1, span5);

    TEST_TRUE(runner, big_boost == eq_big_boost,
              "overlapping and abutting produce the same proximity boost");
    TEST_TRUE(runner, big_boost > smaller_boost, "closer is better");
    TEST_TRUE(runner, zero_boost == 0.0,
              "distance outside of window yields no prox boost");

    DECREF(span1);
    DECREF(span2);
    DECREF(span3);
    DECREF(span4);
    DECREF(span5);
    DECREF(heat_map);
    DECREF(spans);
}

static void
test_flatten_spans(TestBatchRunner *runner) {
    VArray  *spans    = VA_new(8);
    VArray  *wanted   = VA_new(8);
    HeatMap *heat_map = HeatMap_new(spans, 133);

    VArray *flattened, *boosts;

    VA_Push(spans, (Obj*)Span_new(10, 10, 1.0f));
    VA_Push(spans, (Obj*)Span_new(16, 14, 2.0f));
    flattened = HeatMap_Flatten_Spans(heat_map, spans);
    VA_Push(wanted, (Obj*)Span_new(10,  6, 1.0f));
    VA_Push(wanted, (Obj*)Span_new(16,  4, 3.0f));
    VA_Push(wanted, (Obj*)Span_new(20, 10, 2.0f));
    TEST_TRUE(runner, VA_Equals(flattened, (Obj*)wanted),
              "flatten two overlapping spans");
    VA_Clear(wanted);
    boosts = HeatMap_Generate_Proximity_Boosts(heat_map, spans);
    VA_Push(wanted, (Obj*)Span_new(10, 20, 3.0f));
    TEST_TRUE(runner, VA_Equals(boosts, (Obj*)wanted),
              "prox boosts for overlap");
    VA_Clear(wanted);
    VA_Clear(spans);
    DECREF(boosts);
    DECREF(flattened);

    VA_Push(spans, (Obj*)Span_new(10, 10, 1.0f));
    VA_Push(spans, (Obj*)Span_new(16, 14, 2.0f));
    VA_Push(spans, (Obj*)Span_new(50,  1, 1.0f));
    flattened = HeatMap_Flatten_Spans(heat_map, spans);
    VA_Push(wanted, (Obj*)Span_new(10,  6, 1.0f));
    VA_Push(wanted, (Obj*)Span_new(16,  4, 3.0f));
    VA_Push(wanted, (Obj*)Span_new(20, 10, 2.0f));
    VA_Push(wanted, (Obj*)Span_new(50,  1, 1.0f));
    TEST_TRUE(runner, VA_Equals(flattened, (Obj*)wanted),
              "flatten two overlapping spans, leave hole, then third span");
    VA_Clear(wanted);
    boosts = HeatMap_Generate_Proximity_Boosts(heat_map, spans);
    TEST_TRUE(runner, VA_Get_Size(boosts) == 2 + 1,
              "boosts generated for each unique pair, since all were in range");
    VA_Clear(spans);
    DECREF(boosts);
    DECREF(flattened);

    VA_Push(spans, (Obj*)Span_new(10, 10, 1.0f));
    VA_Push(spans, (Obj*)Span_new(14,  4, 4.0f));
    VA_Push(spans, (Obj*)Span_new(16, 14, 2.0f));
    flattened = HeatMap_Flatten_Spans(heat_map, spans);
    VA_Push(wanted, (Obj*)Span_new(10,  4, 1.0f));
    VA_Push(wanted, (Obj*)Span_new(14,  2, 5.0f));
    VA_Push(wanted, (Obj*)Span_new(16,  2, 7.0f));
    VA_Push(wanted, (Obj*)Span_new(18,  2, 3.0f));
    VA_Push(wanted, (Obj*)Span_new(20, 10, 2.0f));
    TEST_TRUE(runner, VA_Equals(flattened, (Obj*)wanted),
              "flatten three overlapping spans");
    VA_Clear(wanted);
    boosts = HeatMap_Generate_Proximity_Boosts(heat_map, spans);
    TEST_TRUE(runner, VA_Get_Size(boosts) == 2 + 1,
              "boosts generated for each unique pair, since all were in range");
    VA_Clear(spans);
    DECREF(boosts);
    DECREF(flattened);

    VA_Push(spans, (Obj*)Span_new(10, 10,  1.0f));
    VA_Push(spans, (Obj*)Span_new(16, 14,  4.0f));
    VA_Push(spans, (Obj*)Span_new(16, 14,  2.0f));
    VA_Push(spans, (Obj*)Span_new(30, 10, 10.0f));
    flattened = HeatMap_Flatten_Spans(heat_map, spans);
    VA_Push(wanted, (Obj*)Span_new(10,  6,  1.0f));
    VA_Push(wanted, (Obj*)Span_new(16,  4,  7.0f));
    VA_Push(wanted, (Obj*)Span_new(20, 10,  6.0f));
    VA_Push(wanted, (Obj*)Span_new(30, 10, 10.0f));
    TEST_TRUE(runner, VA_Equals(flattened, (Obj*)wanted),
              "flatten 4 spans, middle two have identical range");
    VA_Clear(wanted);
    boosts = HeatMap_Generate_Proximity_Boosts(heat_map, spans);
    TEST_TRUE(runner, VA_Get_Size(boosts) == 3 + 2 + 1,
              "boosts generated for each unique pair, since all were in range");
    VA_Clear(spans);
    DECREF(boosts);
    DECREF(flattened);

    VA_Push(spans, (Obj*)Span_new( 10, 10,  1.0f));
    VA_Push(spans, (Obj*)Span_new( 16,  4,  4.0f));
    VA_Push(spans, (Obj*)Span_new( 16, 14,  2.0f));
    VA_Push(spans, (Obj*)Span_new(230, 10, 10.0f));
    flattened = HeatMap_Flatten_Spans(heat_map, spans);
    VA_Push(wanted, (Obj*)Span_new( 10,  6,  1.0f));
    VA_Push(wanted, (Obj*)Span_new( 16,  4,  7.0f));
    VA_Push(wanted, (Obj*)Span_new( 20, 10,  2.0f));
    VA_Push(wanted, (Obj*)Span_new(230, 10, 10.0f));
    TEST_TRUE(runner, VA_Equals(flattened, (Obj*)wanted),
              "flatten 4 spans, middle two have identical starts but different ends");
    VA_Clear(wanted);
    boosts = HeatMap_Generate_Proximity_Boosts(heat_map, spans);
    TEST_TRUE(runner, VA_Get_Size(boosts) == 2 + 1,
              "boosts not generated for out of range span");
    VA_Clear(spans);
    DECREF(boosts);
    DECREF(flattened);

    DECREF(heat_map);
    DECREF(wanted);
    DECREF(spans);
}

void
TestHeatMap_Run_IMP(TestHeatMap *self, TestBatchRunner *runner) {
    TestBatchRunner_Plan(runner, (TestBatch*)self, 13);
    test_calc_proximity_boost(runner);
    test_flatten_spans(runner);
}


