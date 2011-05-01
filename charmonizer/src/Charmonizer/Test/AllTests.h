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

/* Charmonizer/Test.h - test Charmonizer's output.
 */

#ifndef H_CHAZ_ALL_TESTS
#define H_CHAZ_ALL_TESTS

#ifdef __cplusplus
extern "C" {
#endif

#include "Charmonizer/Test.h"

/* Initialize the AllTests module.
 */
void
chaz_AllTests_init();

/* Run all tests.
 */
void
chaz_AllTests_run();

/* These tests all require the file charmony.h.
 *
 * Since Charmonizer conditionally defines many symbols, it can be difficult
 * to tell whether a symbol is missing because it should not have been
 * generated, or whether it is missing because an error occurred.  These test
 * functions make the assumption that any missing symbols have a good excuse
 * for their absence, and test only defined symbols.  This may result in
 * undetected failure some of the time.  However, missing symbols required by
 * your application will trigger compile-time errors, so the theoretical
 * problem of silent failure is less severe than it appears, affecting only
 * fallbacks.
 */

chaz_TestBatch*
chaz_TestDirManip_prepare();

chaz_TestBatch*
chaz_TestFuncMacro_prepare();

chaz_TestBatch*
chaz_TestHeaders_prepare();

chaz_TestBatch*
chaz_TestIntegers_prepare();

chaz_TestBatch*
chaz_TestLargeFiles_prepare();

chaz_TestBatch*
chaz_TestUnusedVars_prepare();

chaz_TestBatch*
chaz_TestVariadicMacros_prepare();

void
chaz_TestDirManip_run(chaz_TestBatch *batch);

void
chaz_TestFuncMacro_run(chaz_TestBatch *batch);

void
chaz_TestHeaders_run(chaz_TestBatch *batch);

void
chaz_TestIntegers_run(chaz_TestBatch *batch);

void
chaz_TestLargeFiles_run(chaz_TestBatch *batch);

void
chaz_TestUnusedVars_run(chaz_TestBatch *batch);

void
chaz_TestVariadicMacros_run(chaz_TestBatch *batch);

#ifdef CHAZ_USE_SHORT_NAMES
  #define TestDirManip_prepare            chaz_TestDirManip_prepare
  #define TestDirManip_run                chaz_TestDirManip_run
  #define TestFuncMacro_prepare           chaz_TestFuncMacro_prepare
  #define TestHeaders_prepare             chaz_TestHeaders_prepare
  #define TestIntegers_prepare            chaz_TestIntegers_prepare
  #define TestLargeFiles_prepare          chaz_TestLargeFiles_prepare
  #define TestUnusedVars_prepare          chaz_TestUnusedVars_prepare
  #define TestVariadicMacros_prepare      chaz_TestVariadicMacros_prepare
  #define TestFuncMacro_run               chaz_TestFuncMacro_run
  #define TestHeaders_run                 chaz_TestHeaders_run
  #define TestIntegers_run                chaz_TestIntegers_run
  #define TestLargeFiles_run              chaz_TestLargeFiles_run
  #define TestUnusedVars_run              chaz_TestUnusedVars_run
  #define TestVariadicMacros_run          chaz_TestVariadicMacros_run
#endif

#ifdef __cplusplus
}
#endif

#endif /* H_CHAZ_TEST */



