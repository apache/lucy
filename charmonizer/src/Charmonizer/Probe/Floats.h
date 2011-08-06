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

/* Charmonizer/Probe/Floats.h -- floating point types.
 *
 * The following symbols will be created if the platform supports IEEE 754
 * floating point types:
 *
 * F32_NAN
 * F32_INF
 * F32_NEGINF
 * F64_NAN
 * F64_INF
 * F64_NEGINF
 *
 * TODO: Actually test to see whether IEEE 754 is supported, rather than just
 * lying about it.
 */

#ifndef H_CHAZ_FLOATS
#define H_CHAZ_FLOATS

#ifdef __cplusplus
extern "C" {
#endif

/* Run the Floats module.
 */
void
chaz_Floats_run(void);

#ifdef CHAZ_USE_SHORT_NAMES
  #define Floats_run    chaz_Floats_run
#endif

#ifdef __cplusplus
}
#endif

#endif /* H_CHAZ_FLOATS */



