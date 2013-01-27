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

#include <string.h>
#include <ctype.h>

#ifndef true
    #define true 1
    #define false 0
#endif

#define CFC_NEED_BASE_STRUCT_DEF
#include "CFCBase.h"
#include "CFCVersion.h"
#include "CFCUtil.h"

struct CFCVersion {
    CFCBase base;
    uint32_t *numbers;
    size_t num_numbers;
    char *vstring;
};

const static CFCMeta CFCVERSION_META = {
    "Clownfish::CFC::Model::Version",
    sizeof(CFCVersion),
    (CFCBase_destroy_t)CFCVersion_destroy
};

CFCVersion*
CFCVersion_new(const char *vstring) {
    CFCVersion *self = (CFCVersion*)CFCBase_allocate(&CFCVERSION_META);
    return CFCVersion_init(self, vstring);
}

CFCVersion*
CFCVersion_init(CFCVersion *self, const char *vstring) {
    CFCUTIL_NULL_CHECK(vstring);
    if (*vstring != 'v' || !isdigit(vstring[1])) {
        CFCBase_decref((CFCBase*)self);
        CFCUtil_die("Bad version string: '%s'", vstring);
    }
    self->vstring = CFCUtil_strdup(vstring);
    vstring++;
    uint32_t num = 0;
    self->num_numbers = 0;
    self->numbers = (uint32_t*)CALLOCATE(1, sizeof(uint32_t));
    while (1) {
        if (isdigit(*vstring)) {
            num = num * 10 + *vstring - '0';
        }
        else {
            if (*vstring != 0 && *vstring != '.') {
                CFCBase_decref((CFCBase*)self);
                CFCUtil_die("Bad version string: '%s'", self->vstring);
            }
            size_t size = (self->num_numbers + 1) * sizeof(uint32_t);
            self->numbers = (uint32_t*)REALLOCATE(self->numbers, size);
            self->numbers[self->num_numbers++] = num;
            if (*vstring == 0) {
                break;
            }
            num = 0;
        }
        vstring++;
    }

    return self;
}

void
CFCVersion_destroy(CFCVersion *self) {
    FREEMEM(self->numbers);
    FREEMEM(self->vstring);
    CFCBase_destroy((CFCBase*)self);
}

int
CFCVersion_compare_to(CFCVersion *self, CFCVersion *other) {
    for (size_t i = 0;
         i < self->num_numbers || i < other->num_numbers;
         i++
        ) {
        uint32_t my_number = i >= self->num_numbers
                             ? 0
                             : self->numbers[i];
        uint32_t other_number = i >= other->num_numbers
                                ? 0
                                : other->numbers[i];
        if (my_number > other_number) {
            return 1;
        }
        else if (other_number > my_number) {
            return -1;
        }
    }
    return 0;
}

uint32_t
CFCVersion_get_major(CFCVersion *self) {
    return self->numbers[0];
}

const char*
CFCVersion_get_vstring(CFCVersion *self) {
    return self->vstring;
}

