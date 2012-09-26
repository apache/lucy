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

#define C_LUCY_NUMBERUTILS
#define LUCY_USE_SHORT_NAMES
#define CHY_USE_SHORT_NAMES

#include <string.h>

#include "Clownfish/Util/NumberUtils.h"

const uint8_t NumUtil_u1masks[8] = {
    0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80
};

const uint8_t NumUtil_u2shifts[4] = { 0x0, 0x2, 0x4,  0x6  };
const uint8_t NumUtil_u2masks[4]  = { 0x3, 0xC, 0x30, 0xC0 };

const uint8_t NumUtil_u4shifts[2] = { 0x00, 0x04 };
const uint8_t NumUtil_u4masks[2]  = { 0x0F, 0xF0 };


