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

#include "Charmonizer/Core/HeaderChecker.h"
#include "Charmonizer/Core/ConfWriter.h"
#include "Charmonizer/Core/Util.h"
#include "Charmonizer/Probe/Floats.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void
chaz_Floats_run(void) {
    chaz_ConfWriter_start_module("Floats");

    chaz_ConfWriter_append_conf(
        "typedef union { uint32_t i; float f; } chy_floatu32;\n"
        "typedef union { uint64_t i; double d; } chy_floatu64;\n"
        "static const chy_floatu32 chy_f32inf    = {UINT32_C(0x7f800000)};\n"
        "static const chy_floatu32 chy_f32neginf = {UINT32_C(0xff800000)};\n"
        "static const chy_floatu32 chy_f32nan    = {UINT32_C(0x7fc00000)};\n"
        "static const chy_floatu64 chy_f64inf    = {UINT64_C(0x7ff0000000000000)};\n"
        "static const chy_floatu64 chy_f64neginf = {UINT64_C(0xfff0000000000000)};\n"
        "static const chy_floatu64 chy_f64nan    = {UINT64_C(0x7ff8000000000000)};\n"
    );
    chaz_ConfWriter_add_def("F32_INF", "(chy_f32inf.f)");
    chaz_ConfWriter_add_def("F32_NEGINF", "(chy_f32neginf.f)");
    chaz_ConfWriter_add_def("F32_NAN", "(chy_f32nan.f)");
    chaz_ConfWriter_add_def("F64_INF", "(chy_f64inf.d)");
    chaz_ConfWriter_add_def("F64_NEGINF", "(chy_f64neginf.d)");
    chaz_ConfWriter_add_def("F64_NAN", "(chy_f64nan.d)");

    chaz_ConfWriter_end_module();
}

const char*
chaz_Floats_math_library(void) {
    static const char sqrt_code[] =
        CHAZ_QUOTE(  #include <math.h>                              )
        CHAZ_QUOTE(  #include <stdio.h>                             )
        CHAZ_QUOTE(  int main(void) {                               )
        CHAZ_QUOTE(      printf("%p\n", sqrt);                      )
        CHAZ_QUOTE(      return 0;                                  )
        CHAZ_QUOTE(  }                                              );
    chaz_CFlags *temp_cflags = chaz_CC_get_temp_cflags();
    char        *output = NULL;
    size_t       output_len;

    output = chaz_CC_capture_output(sqrt_code, &output_len);
    if (output != NULL) {
        /* Linking against libm not needed. */
        free(output);
        return NULL;
    }

    chaz_CFlags_add_library(temp_cflags, "m");
    output = chaz_CC_capture_output(sqrt_code, &output_len);
    chaz_CFlags_clear(temp_cflags);

    if (output == NULL) {
        chaz_Util_die("Don't know how to use math library.");
    }

    free(output);
    return "m";
}


