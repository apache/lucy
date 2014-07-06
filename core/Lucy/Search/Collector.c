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

#define C_LUCY_COLLECTOR
#define C_LUCY_BITCOLLECTOR
#define C_LUCY_OFFSETCOLLECTOR
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Search/Collector.h"
#include "Lucy/Index/SegReader.h"
#include "Lucy/Search/Matcher.h"

Collector*
Coll_init(Collector *self) {
    CollectorIVARS *const ivars = Coll_IVARS(self);
    ABSTRACT_CLASS_CHECK(self, COLLECTOR);
    ivars->reader  = NULL;
    ivars->matcher = NULL;
    ivars->base    = 0;
    return self;
}

void
Coll_Destroy_IMP(Collector *self) {
    CollectorIVARS *const ivars = Coll_IVARS(self);
    DECREF(ivars->reader);
    DECREF(ivars->matcher);
    SUPER_DESTROY(self, COLLECTOR);
}

void
Coll_Set_Reader_IMP(Collector *self, SegReader *reader) {
    CollectorIVARS *const ivars = Coll_IVARS(self);
    DECREF(ivars->reader);
    ivars->reader = (SegReader*)INCREF(reader);
}

void
Coll_Set_Matcher_IMP(Collector *self, Matcher *matcher) {
    CollectorIVARS *const ivars = Coll_IVARS(self);
    DECREF(ivars->matcher);
    ivars->matcher = (Matcher*)INCREF(matcher);
}

void
Coll_Set_Base_IMP(Collector *self, int32_t base) {
    Coll_IVARS(self)->base = base;
}

BitCollector*
BitColl_new(BitVector *bit_vec) {
    BitCollector *self = (BitCollector*)Class_Make_Obj(BITCOLLECTOR);
    return BitColl_init(self, bit_vec);
}

BitCollector*
BitColl_init(BitCollector *self, BitVector *bit_vec) {
    BitCollectorIVARS *const ivars = BitColl_IVARS(self);
    Coll_init((Collector*)self);
    ivars->bit_vec = (BitVector*)INCREF(bit_vec);
    return self;
}

void
BitColl_Destroy_IMP(BitCollector *self) {
    BitCollectorIVARS *const ivars = BitColl_IVARS(self);
    DECREF(ivars->bit_vec);
    SUPER_DESTROY(self, BITCOLLECTOR);
}

void
BitColl_Collect_IMP(BitCollector *self, int32_t doc_id) {
    BitCollectorIVARS *const ivars = BitColl_IVARS(self);

    // Add the doc_id to the BitVector.
    BitVec_Set(ivars->bit_vec, (ivars->base + doc_id));
}

bool
BitColl_Need_Score_IMP(BitCollector *self) {
    UNUSED_VAR(self);
    return false;
}

OffsetCollector*
OffsetColl_new(Collector *inner_coll, int32_t offset) {
    OffsetCollector *self
        = (OffsetCollector*)Class_Make_Obj(OFFSETCOLLECTOR);
    return OffsetColl_init(self, inner_coll, offset);
}

OffsetCollector*
OffsetColl_init(OffsetCollector *self, Collector *inner_coll, int32_t offset) {
    OffsetCollectorIVARS *const ivars = OffsetColl_IVARS(self);
    Coll_init((Collector*)self);
    ivars->offset     = offset;
    ivars->inner_coll = (Collector*)INCREF(inner_coll);
    return self;
}

void
OffsetColl_Destroy_IMP(OffsetCollector *self) {
    OffsetCollectorIVARS *const ivars = OffsetColl_IVARS(self);
    DECREF(ivars->inner_coll);
    SUPER_DESTROY(self, OFFSETCOLLECTOR);
}

void
OffsetColl_Set_Reader_IMP(OffsetCollector *self, SegReader *reader) {
    OffsetCollectorIVARS *const ivars = OffsetColl_IVARS(self);
    Coll_Set_Reader(ivars->inner_coll, reader);
}

void
OffsetColl_Set_Base_IMP(OffsetCollector *self, int32_t base) {
    OffsetCollectorIVARS *const ivars = OffsetColl_IVARS(self);
    Coll_Set_Base(ivars->inner_coll, base);
}

void
OffsetColl_Set_Matcher_IMP(OffsetCollector *self, Matcher *matcher) {
    OffsetCollectorIVARS *const ivars = OffsetColl_IVARS(self);
    Coll_Set_Matcher(ivars->inner_coll, matcher);
}

void
OffsetColl_Collect_IMP(OffsetCollector *self, int32_t doc_id) {
    OffsetCollectorIVARS *const ivars = OffsetColl_IVARS(self);
    Coll_Collect(ivars->inner_coll, (doc_id + ivars->offset));
}

bool
OffsetColl_Need_Score_IMP(OffsetCollector *self) {
    OffsetCollectorIVARS *const ivars = OffsetColl_IVARS(self);
    return Coll_Need_Score(ivars->inner_coll);
}


