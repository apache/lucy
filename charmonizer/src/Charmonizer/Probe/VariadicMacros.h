/* Charmonizer/Probe/VariadicMacros.h
 */

#ifndef H_CHAZ_VARIADIC_MACROS
#define H_CHAZ_VARIADIC_MACROS 

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

/* Run the VariadicMacros module.
 *
 * If your compiler supports ISO-style variadic macros, this will be defined:
 * 
 * HAS_ISO_VARIADIC_MACROS
 * 
 * If your compiler supports GNU-style variadic macros, this will be defined:
 * 
 * HAS_GNUC_VARIADIC_MACROS
 * 
 * If you have at least one of the above, this will be defined:
 * 
 * HAS_VARIADIC_MACROS
 */
void chaz_VariadicMacros_run(void);

#ifdef CHAZ_USE_SHORT_NAMES
  #define VariadicMacros_run    chaz_VariadicMacros_run
#endif

#ifdef __cplusplus
}
#endif

#endif /* H_CHAZ_VARIADIC_MACROS */


/**
 * Copyright 2006 The Apache Software Foundation
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

