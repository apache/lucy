#define CHAZ_USE_SHORT_NAMES

#include "Charmonizer/Core/HeadCheck.h"
#include "Charmonizer/Core/ModHandler.h"
#include "Charmonizer/Core/Util.h"
#include "Charmonizer/Probe/Floats.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void
Floats_run(void) 
{
    START_RUN("Floats");

    ModHand_append_conf(
        "typedef float chy_f32_t;\n"
        "typedef double chy_f64_t;\n"
        "#define CHY_HAS_F32_T\n"
        "#define CHY_HAS_F64_T\n"
    );

    ModHand_append_conf(
        "typedef union { chy_i32_t i; float f; } chy_floati32;\n"
        "static const chy_floati32 chy_f32inf    = {CHY_I32_C(0x7f800000)};\n"
        "static const chy_floati32 chy_f32neginf = {CHY_I32_C(0xff800000)};\n"
        "static const chy_floati32 chy_f32nan    = {CHY_I32_C(0x7fc00000)};\n"
        "#define CHY_F32_INF (chy_f32inf.f)\n"
        "#define CHY_F32_NEGINF (chy_f32neginf.f)\n"
        "#define CHY_F32_NAN (chy_f32nan.f)\n"
    );

    /* Shorten. */
    START_SHORT_NAMES;
    ModHand_shorten_typedef("f32_t");
    ModHand_shorten_typedef("f64_t");
    ModHand_shorten_macro("HAS_F32_T");
    ModHand_shorten_macro("HAS_F64_T");
    ModHand_shorten_macro("F32_INF");
    ModHand_shorten_macro("F32_NEGINF");
    ModHand_shorten_macro("F32_NAN");
    END_SHORT_NAMES;
    
    END_RUN;
}

/**
 * Copyright 2009 The Apache Software Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

