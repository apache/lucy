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

package lucy

import "testing"

func TestHeatMapBasics(t *testing.T) {
	spans := make([]Span, 2)
	spans[0] = NewSpan(0, 3, 10.0)
	spans[1] = NewSpan(30, 5, 2.0)
	heatMap := NewHeatMap(spans, 133)
	if _, ok := heatMap.(HeatMap); !ok {
		t.Errorf("NewHeatMap")
	}
	boost := heatMap.calcProximityBoost(spans[0], spans[1])
	if boost <= 0.0 {
		t.Errorf("calcProximityBoost: %f", boost)
	}
	if length := len(heatMap.getSpans()); length <= 0 {
		t.Errorf("getSpans: %d", length)
	}
	boosts := heatMap.generateProximityBoosts(spans)
	if length := len(boosts); length <= 0 {
		t.Errorf("generateProximityBoosts: %d", length)
	}
	flattened := heatMap.flattenSpans(spans)
	if length := len(flattened); length <= 0 {
		t.Errorf("flattenSpans: %d", length)
	}
}
