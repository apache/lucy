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

#define C_LUCY_TERMSTEPPER
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Index/TermStepper.h"
#include "Lucy/Plan/Schema.h"
#include "Lucy/Store/InStream.h"
#include "Lucy/Store/OutStream.h"
#include "Lucy/Util/StringHelper.h"

TermStepper*
TermStepper_init(TermStepper *self) {
    Stepper_init((Stepper*)self);
    self->value = NULL;
    return self;
}

void
TermStepper_destroy(TermStepper *self) {
    DECREF(self->value);
    SUPER_DESTROY(self, TERMSTEPPER);
}

void
TermStepper_reset(TermStepper *self) {
    DECREF(self->value);
    self->value = NULL;
}

Obj*
TermStepper_get_value(TermStepper *self) {
    return self->value;
}

void
TermStepper_set_value(TermStepper *self, Obj *value) {
    DECREF(self->value);
    self->value = value ? INCREF(value) : NULL;
}


