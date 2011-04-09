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

/* Charmonizer/Core/Stat.h - stat a file, if possible.
 *
 * This component works by attempting to compile a utility program called
 * "_charm_stat".  When Charmonizer needs to stat a file, it shells out to
 * this utility, which communicates via a file a la capture_output().
 *
 * Since we don't know whether we have 64-bit integers when Charmonizer itself
 * gets compiled, the items in the stat structure are whatever size longs are.
 *
 * TODO: probe for which fields are available.
 */

#ifndef H_CHAZ_STAT
#define H_CHAZ_STAT

#ifdef __cplusplus
extern "C" {
#endif

#include "Charmonizer/Core/Defines.h"

typedef struct chaz_Stat chaz_Stat;

struct chaz_Stat {
    chaz_bool_t valid;
    long size;
    long blocks;
};

/* Attempt to stat a file.  If successful, store the set target->valid to true
 * and store the results in the stat structure.  If unsuccessful, set
 * target->valid to false.
 */
void
chaz_Stat_stat(const char *filepath, chaz_Stat *target);

#ifdef CHAZ_USE_SHORT_NAMES
  #define Stat                  chaz_Stat
  #define Stat_stat             chaz_Stat_stat
#endif

#ifdef __cplusplus
}
#endif

#endif /* H_CHAZ_COMPILER */



