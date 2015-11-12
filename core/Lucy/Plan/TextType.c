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

#define C_LUCY_TEXTTYPE
#define C_LUCY_TEXTTERMSTEPPER
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Plan/TextType.h"
#include "Lucy/Store/InStream.h"
#include "Lucy/Store/OutStream.h"
#include "Clownfish/ByteBuf.h"
#include "Clownfish/Util/StringHelper.h"

TermStepper*
TextType_Make_Term_Stepper_IMP(TextType *self) {
    UNUSED_VAR(self);
    return (TermStepper*)TextTermStepper_new();
}

int8_t
TextType_Primitive_ID_IMP(TextType *self) {
    UNUSED_VAR(self);
    return FType_TEXT;
}

/***************************************************************************/

TextTermStepper*
TextTermStepper_new() {
    TextTermStepper *self
        = (TextTermStepper*)Class_Make_Obj(TEXTTERMSTEPPER);
    return TextTermStepper_init(self);
}

TextTermStepper*
TextTermStepper_init(TextTermStepper *self) {
    TermStepper_init((TermStepper*)self);
    TextTermStepperIVARS *const ivars = TextTermStepper_IVARS(self);
    ivars->bytebuf = BB_new(0);
    return self;
}

void
TextTermStepper_Destroy_IMP(TextTermStepper *self) {
    TextTermStepperIVARS *const ivars = TextTermStepper_IVARS(self);
    DECREF(ivars->bytebuf);
    SUPER_DESTROY(self, TEXTTERMSTEPPER);
}

static void
S_set_value(TextTermStepper *self, Obj *value) {
    TextTermStepperIVARS *const ivars = TextTermStepper_IVARS(self);
    if (ivars->value != value) {
        DECREF(ivars->value);
        ivars->value = INCREF(value);
    }
}

void
TextTermStepper_Set_Value_IMP(TextTermStepper *self, Obj *value) {
    S_set_value(self, CERTIFY(value, STRING));
}

Obj*
TextTermStepper_Get_Value_IMP(TextTermStepper *self) {
    TextTermStepperIVARS *const ivars = TextTermStepper_IVARS(self);
    if (ivars->value == NULL) {
        ivars->value = (Obj*)BB_Trusted_Utf8_To_String(ivars->bytebuf);
    }
    return ivars->value;
}

void
TextTermStepper_Reset_IMP(TextTermStepper *self) {
    TextTermStepperIVARS *const ivars = TextTermStepper_IVARS(self);
    DECREF(ivars->value);
    ivars->value = NULL;
    BB_Set_Size(ivars->bytebuf, 0);
}

void
TextTermStepper_Write_Key_Frame_IMP(TextTermStepper *self,
                                    OutStream *outstream, Obj *value) {
    String     *string = (String*)CERTIFY(value, STRING);
    const char *buf    = Str_Get_Ptr8(string);
    size_t      size   = Str_Get_Size(string);
    OutStream_Write_C32(outstream, size);
    OutStream_Write_Bytes(outstream, buf, size);

    S_set_value(self, value);
}

void
TextTermStepper_Write_Delta_IMP(TextTermStepper *self, OutStream *outstream,
                                Obj *value) {
    TextTermStepperIVARS *const ivars = TextTermStepper_IVARS(self);
    ByteBuf    *bytebuf  = (ByteBuf*)CERTIFY(value, BYTEBUF);
    const char *new_text = BB_Get_Buf(bytebuf);
    size_t      new_size = BB_Get_Size(bytebuf);

    const char *last_text;
    size_t      last_size;
    if (ivars->value) {
        String *last_string = (String*)ivars->value;
        last_text = Str_Get_Ptr8(last_string);
        last_size = Str_Get_Size(last_string);
    }
    else {
        last_text = BB_Get_Buf(ivars->bytebuf);
        last_size = BB_Get_Size(ivars->bytebuf);
    }

    // Count how many bytes the strings share at the top.
    const int32_t overlap = StrHelp_overlap(last_text, new_text,
                                            last_size, new_size);
    const char *const diff_start_str = new_text + overlap;
    const size_t diff_len            = new_size - overlap;

    // Write number of common bytes and common bytes.
    OutStream_Write_C32(outstream, overlap);
    OutStream_Write_String(outstream, diff_start_str, diff_len);

    // Update value.
    BB_Mimic_Bytes(ivars->bytebuf, new_text, new_size);

    // Invalidate string value.
    DECREF(ivars->value);
    ivars->value = NULL;
}

void
TextTermStepper_Read_Key_Frame_IMP(TextTermStepper *self,
                                   InStream *instream) {
    TextTermStepperIVARS *const ivars = TextTermStepper_IVARS(self);
    const uint32_t text_len = InStream_Read_C32(instream);

    // Allocate space.
    char *ptr = BB_Grow(ivars->bytebuf, text_len);

    // Set the value text.
    InStream_Read_Bytes(instream, ptr, text_len);
    BB_Set_Size(ivars->bytebuf, text_len);
    if (!StrHelp_utf8_valid(ptr, text_len)) {
        THROW(ERR, "Invalid UTF-8 sequence in '%o' at byte %i64",
              InStream_Get_Filename(instream),
              InStream_Tell(instream) - text_len);
    }

    // Invalidate string value.
    DECREF(ivars->value);
    ivars->value = NULL;
}

void
TextTermStepper_Read_Delta_IMP(TextTermStepper *self, InStream *instream) {
    TextTermStepperIVARS *const ivars = TextTermStepper_IVARS(self);
    const uint32_t text_overlap     = InStream_Read_C32(instream);
    const uint32_t finish_chars_len = InStream_Read_C32(instream);
    const uint32_t total_text_len   = text_overlap + finish_chars_len;

    // Allocate space.
    if (ivars->value) {
        BB_Mimic(ivars->bytebuf, ivars->value);
    }
    char *ptr = BB_Grow(ivars->bytebuf, total_text_len);

    // Set the value text.
    InStream_Read_Bytes(instream, ptr + text_overlap, finish_chars_len);
    BB_Set_Size(ivars->bytebuf, total_text_len);
    if (!StrHelp_utf8_valid(ptr, total_text_len)) {
        THROW(ERR, "Invalid UTF-8 sequence in '%o' at byte %i64",
              InStream_Get_Filename(instream),
              InStream_Tell(instream) - finish_chars_len);
    }

    // Invalidate string value.
    DECREF(ivars->value);
    ivars->value = NULL;
}


