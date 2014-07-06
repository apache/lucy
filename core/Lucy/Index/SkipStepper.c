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

#define C_LUCY_SKIPSTEPPER
#include <stdio.h>

#include "Lucy/Util/ToolSet.h"

#include "Lucy/Index/SkipStepper.h"
#include "Lucy/Store/Folder.h"
#include "Lucy/Store/InStream.h"
#include "Lucy/Store/OutStream.h"

SkipStepper*
SkipStepper_new() {
    SkipStepper *self = (SkipStepper*)Class_Make_Obj(SKIPSTEPPER);
    SkipStepperIVARS *const ivars = SkipStepper_IVARS(self);

    // Init.
    ivars->doc_id   = 0;
    ivars->filepos  = 0;

    return self;
}

void
SkipStepper_Set_ID_And_Filepos_IMP(SkipStepper *self, int32_t doc_id,
                                   int64_t filepos) {
    SkipStepperIVARS *const ivars = SkipStepper_IVARS(self);
    ivars->doc_id  = doc_id;
    ivars->filepos = filepos;
}

void
SkipStepper_Read_Record_IMP(SkipStepper *self, InStream *instream) {
    SkipStepperIVARS *const ivars = SkipStepper_IVARS(self);
    ivars->doc_id   += InStream_Read_C32(instream);
    ivars->filepos  += InStream_Read_C64(instream);
}

String*
SkipStepper_To_String_IMP(SkipStepper *self) {
    SkipStepperIVARS *const ivars = SkipStepper_IVARS(self);
    return Str_newf("skip doc: %u32 file pointer: %i64", ivars->doc_id,
                    ivars->filepos);
}

void
SkipStepper_Write_Record_IMP(SkipStepper *self, OutStream *outstream,
                             int32_t last_doc_id, int64_t last_filepos) {
    SkipStepperIVARS *const ivars = SkipStepper_IVARS(self);
    const int32_t delta_doc_id = ivars->doc_id - last_doc_id;
    const int64_t delta_filepos = ivars->filepos - last_filepos;

    // Write delta doc id.
    OutStream_Write_C32(outstream, delta_doc_id);

    // Write delta file pointer.
    OutStream_Write_C64(outstream, delta_filepos);
}



