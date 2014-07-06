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

#define C_TESTLUCY_TESTARCHITECTURE
#define TESTLUCY_USE_SHORT_NAMES
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Test.h"
#include "Lucy/Test/Plan/TestArchitecture.h"
#include "Lucy/Plan/Architecture.h"

TestArchitecture*
TestArch_new() {
    TestArchitecture *self
        = (TestArchitecture*)Class_Make_Obj(TESTARCHITECTURE);
    return TestArch_init(self);
}

TestArchitecture*
TestArch_init(TestArchitecture *self) {
    Arch_init((Architecture*)self);
    return self;
}

int32_t
TestArch_Index_Interval_IMP(TestArchitecture *self) {
    UNUSED_VAR(self);
    return 5;
}

int32_t
TestArch_Skip_Interval_IMP(TestArchitecture *self) {
    UNUSED_VAR(self);
    return 3;
}


