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

#define C_LUCY_SIMILARITY
#include "Lucy/Util/ToolSet.h"

#include "math.h"

#include "Lucy/Index/Similarity.h"

#include "Lucy/Index/Posting/ScorePosting.h"
#include "Lucy/Index/Segment.h"
#include "Lucy/Index/Snapshot.h"
#include "Lucy/Index/PolyReader.h"
#include "Lucy/Index/Posting/MatchPosting.h"
#include "Lucy/Plan/Schema.h"
#include "Lucy/Store/InStream.h"
#include "Lucy/Store/OutStream.h"
#include "Lucy/Util/Freezer.h"

// The exponent range [-31;32] is mapped to [0;63]. Values outside
// of the range are clamped resulting in 6 bits for the exponent.
// The IEEE bias is 127, so we have to subtract 127 and add 31 to
// the upper bits.
#define EXP_OFFSET ((127 - 31) << 2)

Similarity*
Sim_new() {
    Similarity *self = (Similarity*)Class_Make_Obj(SIMILARITY);
    return Sim_init(self);
}

Similarity*
Sim_init(Similarity *self) {
    SimilarityIVARS *const ivars = Sim_IVARS(self);
    ivars->norm_decoder = NULL;
    return self;
}

void
Sim_Destroy_IMP(Similarity *self) {
    SimilarityIVARS *const ivars = Sim_IVARS(self);
    if (ivars->norm_decoder) {
        FREEMEM(ivars->norm_decoder);
    }
    SUPER_DESTROY(self, SIMILARITY);
}

Posting*
Sim_Make_Posting_IMP(Similarity *self) {
    return (Posting*)ScorePost_new(self);
}

PostingWriter*
Sim_Make_Posting_Writer_IMP(Similarity *self, Schema *schema,
                            Snapshot *snapshot, Segment *segment,
                            PolyReader *polyreader, int32_t field_num) {
    UNUSED_VAR(self);
    return (PostingWriter*)MatchPostWriter_new(schema, snapshot, segment,
                                               polyreader, field_num);
}

float*
Sim_Get_Norm_Decoder_IMP(Similarity *self) {
    SimilarityIVARS *const ivars = Sim_IVARS(self);
    if (!ivars->norm_decoder) {
        // Cache decoded boost bytes.
        ivars->norm_decoder = (float*)MALLOCATE(256 * sizeof(float));
        for (uint32_t i = 0; i < 256; i++) {
            ivars->norm_decoder[i] = Sim_Decode_Norm(self, i);
        }
    }
    return ivars->norm_decoder;
}

Obj*
Sim_Dump_IMP(Similarity *self) {
    Hash *dump = Hash_new(0);
    Hash_Store_Utf8(dump, "_class", 6,
                    (Obj*)Str_Clone(Sim_Get_Class_Name(self)));
    return (Obj*)dump;
}

Similarity*
Sim_Load_IMP(Similarity *self, Obj *dump) {
    Hash *source = (Hash*)CERTIFY(dump, HASH);
    String *class_name 
        = (String*)CERTIFY(Hash_Fetch_Utf8(source, "_class", 6), STRING);
    Class *klass = Class_singleton(class_name, NULL);
    Similarity *loaded = (Similarity*)Class_Make_Obj(klass);
    UNUSED_VAR(self);
    return Sim_init(loaded);
}

void
Sim_Serialize_IMP(Similarity *self, OutStream *target) {
    // Only the class name.
    Freezer_serialize_string(Sim_Get_Class_Name(self), target);
}

Similarity*
Sim_Deserialize_IMP(Similarity *self, InStream *instream) {
    String *class_name = Freezer_read_string(instream);
    if (!Str_Equals(class_name, (Obj*)Sim_Get_Class_Name(self))) {
        THROW(ERR, "Class name mismatch: '%o' '%o'", Sim_Get_Class_Name(self),
              class_name);
    }
    DECREF(class_name);
    Sim_init(self);
    return self;
}

bool
Sim_Equals_IMP(Similarity *self, Obj *other) {
    if (Sim_Get_Class(self) != Obj_Get_Class(other)) { return false; }
    return true;
}

float
Sim_IDF_IMP(Similarity *self, int64_t doc_freq, int64_t total_docs) {
    UNUSED_VAR(self);
    if (total_docs == 0) {
        // Guard against log of zero error, return meaningless number.
        return 1;
    }
    else {
        double total_documents = (double)total_docs;
        double document_freq   = (double)doc_freq;
        return (float)(1 + log(total_documents / (1 + document_freq)));
    }
}

float
Sim_TF_IMP(Similarity *self, float freq) {
    UNUSED_VAR(self);
    return (float)sqrt(freq);
}

uint32_t
Sim_Encode_Norm_IMP(Similarity *self, float f) {
    uint32_t norm;
    UNUSED_VAR(self);

    if (f < 0.0) {
        f = 0.0;
    }

    if (f == 0.0) {
        norm = 0;
    }
    else {
        const uint32_t bits = *(uint32_t*)&f;

        // The normalized value contains two bits of mantissa (excluding
        // the implicit leading bit) in the least significant bits and the
        // exponent in the upper bits.
        norm = (bits >> 21) & 0x3ff;

        if (norm <= EXP_OFFSET) {
            norm = 0;
        }
        else {
            norm -= EXP_OFFSET;
            if (norm > 255) {
                norm = 255;
            }
        }
    }

    return norm;
}

float
Sim_Decode_Norm_IMP(Similarity *self, uint32_t input) {
    uint8_t  byte = input & 0xFF;
    uint32_t result;
    UNUSED_VAR(self);

    if (byte == 0) {
        result = 0;
    }
    else {
        result = (input + EXP_OFFSET) << 21;
    }

    return *(float*)&result;
}

float
Sim_Length_Norm_IMP(Similarity *self, uint32_t num_tokens) {
    UNUSED_VAR(self);
    if (num_tokens == 0) { // guard against div by zero
        return 0;
    }
    else {
        return (float)(1.0 / sqrt((double)num_tokens));
    }
}

float
Sim_Query_Norm_IMP(Similarity *self, float sum_of_squared_weights) {
    UNUSED_VAR(self);
    if (sum_of_squared_weights == 0.0f) { // guard against div by zero
        return 0;
    }
    else {
        return (float)(1.0 / sqrt(sum_of_squared_weights));
    }
}

float
Sim_Coord_IMP(Similarity *self, uint32_t overlap, uint32_t max_overlap) {
    UNUSED_VAR(self);
    if (max_overlap == 0) {
        return 1;
    }
    else {
        return (float)overlap / (float)max_overlap;
    }
}


