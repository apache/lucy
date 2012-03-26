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

/* Charmonizer/Probe/AtomicOps.h
 */

#ifndef H_CHAZ_ATOMICOPS
#define H_CHAZ_ATOMICOPS

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

/* Run the AtomicOps module.
 *
 * These following symbols will be defined if the associated headers are
 * available:
 *
 * HAS_LIBKERN_OSATOMIC_H  <libkern/OSAtomic.h> (Mac OS X)
 * HAS_SYS_ATOMIC_H        <sys/atomic.h>       (Solaris)
 * HAS_INTRIN_H            <intrin.h>           (Windows)
 *
 * This symbol is defined if OSAtomicCompareAndSwapPtr is available:
 *
 * HAS_OSATOMIC_CAS_PTR
 */
void chaz_AtomicOps_run(void);

#ifdef CHAZ_USE_SHORT_NAMES
  #define AtomicOps_run    chaz_AtomicOps_run
#endif

#ifdef __cplusplus
}
#endif

#endif /* H_CHAZ_ATOMICOPS */



