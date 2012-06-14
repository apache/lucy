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

#define CHAZ_USE_SHORT_NAMES

#include "Charmonizer/Core/HeaderChecker.h"
#include "Charmonizer/Core/ConfWriter.h"
#include "Charmonizer/Core/Util.h"
#include "Charmonizer/Probe/Floats.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void
Floats_run(void) {
    ConfWriter_start_module("Floats");

    ConfWriter_append_conf(
        "typedef union { uint32_t i; float f; } chy_floatu32;\n"
        "typedef union { uint64_t i; double d; } chy_floatu64;\n"
        "static const chy_floatu32 chy_f32inf    = {CHY_U32_C(0x7f800000)};\n"
        "static const chy_floatu32 chy_f32neginf = {CHY_U32_C(0xff800000)};\n"
        "static const chy_floatu32 chy_f32nan    = {CHY_U32_C(0x7fc00000)};\n"
        "static const chy_floatu64 chy_f64inf    = {CHY_U64_C(0x7ff0000000000000)};\n"
        "static const chy_floatu64 chy_f64neginf = {CHY_U64_C(0xfff0000000000000)};\n"
        "static const chy_floatu64 chy_f64nan    = {CHY_U64_C(0x7ff8000000000000)};\n"
    );
    ConfWriter_add_def("F32_INF", "(chy_f32inf.f)");
    ConfWriter_add_def("F32_NEGINF", "(chy_f32neginf.f)");
    ConfWriter_add_def("F32_NAN", "(chy_f32nan.f)");
    ConfWriter_add_def("F64_INF", "(chy_f64inf.d)");
    ConfWriter_add_def("F64_NEGINF", "(chy_f64neginf.d)");
    ConfWriter_add_def("F64_NAN", "(chy_f64nan.d)");

    ConfWriter_end_module();
}


