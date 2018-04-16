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

#define C_LUCY_COMPILER
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Search/Compiler.h"

#include "Clownfish/CharBuf.h"
#include "Lucy/Index/SegReader.h"
#include "Lucy/Index/DocVector.h"
#include "Lucy/Index/Similarity.h"
#include "Lucy/Plan/Schema.h"
#include "Lucy/Search/Matcher.h"
#include "Lucy/Search/Query.h"
#include "Lucy/Search/Searcher.h"
#include "Lucy/Store/InStream.h"
#include "Lucy/Store/OutStream.h"
#include "Lucy/Util/Freezer.h"

Compiler*
Compiler_init(Compiler *self) {
    ABSTRACT_CLASS_CHECK(self, COMPILER);
    return self;
}

float
Compiler_Sum_Of_Squared_Weights_IMP(Compiler *self) {
    UNUSED_VAR(self);
    return 1.0f;
}

void
Compiler_Apply_Norm_Factor_IMP(Compiler *self, float factor) {
    UNUSED_VAR(self);
    UNUSED_VAR(factor);
}

void
Compiler_Normalize_IMP(Compiler *self, Similarity *sim) {
    // factor = (tf_q * idf_t)
    float factor = Compiler_Sum_Of_Squared_Weights(self);

    // factor /= norm_q
    factor = Sim_Query_Norm(sim, factor);

    // weight *= factor
    Compiler_Apply_Norm_Factor(self, factor);
}

Vector*
Compiler_Highlight_Spans_IMP(Compiler *self, Searcher *searcher,
                             DocVector *doc_vec, String *field) {
    UNUSED_VAR(self);
    UNUSED_VAR(searcher);
    UNUSED_VAR(doc_vec);
    UNUSED_VAR(field);
    return Vec_new(0);
}

