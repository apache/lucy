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

/* Charmonizer/Core/SharedLibrary.h
 */

#ifndef H_CHAZ_SHARED_LIB
#define H_CHAZ_SHARED_LIB

#ifdef __cplusplus
extern "C" {
#endif

typedef struct chaz_SharedLib chaz_SharedLib;

chaz_SharedLib*
chaz_SharedLib_new(const char *name, const char *version,
                   const char *major_version);

void
chaz_SharedLib_destroy(chaz_SharedLib *flags);

const char*
chaz_SharedLib_get_name(chaz_SharedLib *lib);

const char*
chaz_SharedLib_get_version(chaz_SharedLib *lib);

const char*
chaz_SharedLib_get_major_version(chaz_SharedLib *lib);

char*
chaz_SharedLib_filename(chaz_SharedLib *lib);

char*
chaz_SharedLib_major_version_filename(chaz_SharedLib *lib);

char*
chaz_SharedLib_no_version_filename(chaz_SharedLib *lib);

char*
chaz_SharedLib_implib_filename(chaz_SharedLib *lib);

char*
chaz_SharedLib_export_filename(chaz_SharedLib *lib);

#ifdef __cplusplus
}
#endif

#endif /* H_CHAZ_SHARED_LIB */


