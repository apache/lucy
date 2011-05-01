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

/* Charmonizer/Probe/FuncMacro.h
 */

#ifndef H_CHAZ_FUNC_MACRO
#define H_CHAZ_FUNC_MACRO

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

/* Run the FuncMacro module.
 *
 * If __func__ successfully resolves, this will be defined:
 *
 * HAS_ISO_FUNC_MACRO
 *
 * If __FUNCTION__ successfully resolves, this will be defined:
 *
 * HAS_GNUC_FUNC_MACRO
 *
 * If one or the other succeeds, these will be defined:
 *
 * HAS_FUNC_MACRO
 * FUNC_MACRO
 *
 * The "inline" keyword will also be probed for.  If it is available, the
 * following macro will be defined to "inline", otherwise it will be defined
 * to nothing.
 *
 * INLINE
 */
void chaz_FuncMacro_run(void);

#ifdef CHAZ_USE_SHORT_NAMES
  #define FuncMacro_run    chaz_FuncMacro_run
#endif

#ifdef __cplusplus
}
#endif

#endif /* H_CHAZ_FUNC_MACRO */



