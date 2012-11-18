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

/* Charmonizer/Probe/Booleans.h -- bool type.
 *
 * If stdbool.h is is available, it will be pound-included in the configuration
 * header.  If it is not, the following typedef will be defined:
 *
 * bool
 *
 * These symbols will be defined if they are not already:
 *
 * true
 * false
 */

#ifndef H_CHAZ_BOOLEANS
#define H_CHAZ_BOOLEANS

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

/* Run the Booleans module.
 */
void chaz_Booleans_run(void);

#ifdef __cplusplus
}
#endif

#endif /* H_CHAZ_BOOLEANS */



