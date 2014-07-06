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
#include "Clownfish/CharBuf.h"
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
    ivars->value  = (Obj*)CB_new(0);
    ivars->string = NULL;
    return self;
}

void
TextTermStepper_Destroy_IMP(TextTermStepper *self) {
    TextTermStepperIVARS *const ivars = TextTermStepper_IVARS(self);
    DECREF(ivars->string);
    SUPER_DESTROY(self, TEXTTERMSTEPPER);
}

void
TextTermStepper_Set_Value_IMP(TextTermStepper *self, Obj *value) {
    TextTermStepperIVARS *const ivars = TextTermStepper_IVARS(self);
    CERTIFY(value, STRING);
    Obj_Mimic(ivars->value, value);
    // Invalidate string.
    DECREF(ivars->string);
    ivars->string = NULL;
}

Obj*
TextTermStepper_Get_Value_IMP(TextTermStepper *self) {
    TextTermStepperIVARS *const ivars = TextTermStepper_IVARS(self);
    if (ivars->string == NULL) {
        ivars->string = CB_To_String((CharBuf*)ivars->value);
    }
    return (Obj*)ivars->string;
}

void
TextTermStepper_Reset_IMP(TextTermStepper *self) {
    TextTermStepperIVARS *const ivars = TextTermStepper_IVARS(self);
    CB_Set_Size((CharBuf*)ivars->value, 0);
    // Invalidate string.
    DECREF(ivars->string);
    ivars->string = NULL;
}

void
TextTermStepper_Write_Key_Frame_IMP(TextTermStepper *self,
                                    OutStream *outstream, Obj *value) {
    TextTermStepperIVARS *const ivars = TextTermStepper_IVARS(self);
    CharBuf *charbuf = (CharBuf*)ivars->value;
    CB_Mimic(charbuf, value);
    const char *buf  = CB_Get_Ptr8(charbuf);
    size_t      size = CB_Get_Size(charbuf);
    OutStream_Write_C32(outstream, size);
    OutStream_Write_Bytes(outstream, buf, size);
    // Invalidate string.
    DECREF(ivars->string);
    ivars->string = NULL;
}

void
TextTermStepper_Write_Delta_IMP(TextTermStepper *self, OutStream *outstream,
                                Obj *value) {
    TextTermStepperIVARS *const ivars = TextTermStepper_IVARS(self);
    CharBuf    *charbuf   = (CharBuf*)ivars->value;
    const char *last_text = CB_Get_Ptr8(charbuf);
    size_t      last_size = CB_Get_Size(charbuf);
    const char *new_text  = NULL;
    size_t      new_size  = 0;

    if (Obj_Is_A(value, STRING)) {
        String *new_string = (String*)value;
        new_text = Str_Get_Ptr8(new_string);
        new_size = Str_Get_Size(new_string);
    }
    else if (Obj_Is_A(value, CHARBUF)) {
        CharBuf *new_charbuf = (CharBuf*)value;
        new_text = CB_Get_Ptr8(new_charbuf);
        new_size = CB_Get_Size(new_charbuf);
    }
    else {
        THROW(ERR, "'value' must be a String or CharBuf");
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
    CB_Mimic_Utf8(charbuf, new_text, new_size);

    // Invalidate string.
    DECREF(ivars->string);
    ivars->string = NULL;
}

void
TextTermStepper_Read_Key_Frame_IMP(TextTermStepper *self,
                                   InStream *instream) {
    TextTermStepperIVARS *const ivars = TextTermStepper_IVARS(self);
    const uint32_t text_len = InStream_Read_C32(instream);

    // Allocate space.
    CharBuf *charbuf = (CharBuf*)ivars->value;
    char    *ptr     = CB_Grow(charbuf, text_len);

    // Set the value text.
    InStream_Read_Bytes(instream, ptr, text_len);
    CB_Set_Size(charbuf, text_len);
    if (!StrHelp_utf8_valid(ptr, text_len)) {
        THROW(ERR, "Invalid UTF-8 sequence in '%o' at byte %i64",
              InStream_Get_Filename(instream),
              InStream_Tell(instream) - text_len);
    }

    // Null-terminate.
    ptr[text_len] = '\0';

    // Invalidate string.
    DECREF(ivars->string);
    ivars->string = NULL;
}

void
TextTermStepper_Read_Delta_IMP(TextTermStepper *self, InStream *instream) {
    TextTermStepperIVARS *const ivars = TextTermStepper_IVARS(self);
    const uint32_t text_overlap     = InStream_Read_C32(instream);
    const uint32_t finish_chars_len = InStream_Read_C32(instream);
    const uint32_t total_text_len   = text_overlap + finish_chars_len;

    // Allocate space.
    CharBuf *charbuf = (CharBuf*)ivars->value;
    char    *ptr     = CB_Grow(charbuf, total_text_len);

    // Set the value text.
    InStream_Read_Bytes(instream, ptr + text_overlap, finish_chars_len);
    CB_Set_Size(charbuf, total_text_len);
    if (!StrHelp_utf8_valid(ptr, total_text_len)) {
        THROW(ERR, "Invalid UTF-8 sequence in '%o' at byte %i64",
              InStream_Get_Filename(instream),
              InStream_Tell(instream) - finish_chars_len);
    }

    // Null-terminate.
    ptr[total_text_len] = '\0';

    // Invalidate string.
    DECREF(ivars->string);
    ivars->string = NULL;
}


