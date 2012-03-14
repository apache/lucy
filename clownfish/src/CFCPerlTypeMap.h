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

#ifndef H_CFCPERLTYPEMAP
#define H_CFCPERLTYPEMAP

#ifdef __cplusplus
extern "C" {
#endif

/** Clownfish::CFC::Binding::Perl::TypeMap - Convert between Clownfish and
 * Perl via XS.
 *
 * TypeMap serves up C code fragments for translating between Perl data
 * structures and Clownfish data structures.  The functions to_perl() and
 * from_perl() achieve this for individual types; write_xs_typemap() exports
 * all types using the XS "typemap" format documented in "perlxs".
 */

struct CFCHierarchy;
struct CFCType;

/** Return an expression which converts from a Perl scalar to a variable of
 * the specified type.
 * 
 * @param type A Clownfish::CFC::Model::Type, which will be used to select the
 * mapping code.
 * @param xs_var The C name of the Perl scalar from which we are extracting
 * a value.
 */
char*
CFCPerlTypeMap_from_perl(struct CFCType *type, const char *xs_var);

/** Return an expression converts from a variable of type $type to a Perl
 * scalar.
 * 
 * @param type A Clownfish::CFC::Model::Type, which will be used to select the
 * mapping code.
 * @param cf_var The name of the variable from which we are extracting a
 * value.
 */ 
char*
CFCPerlTypeMap_to_perl(struct CFCType *type, const char *cf_var);

/** Auto-generate a "typemap" file that adheres to the conventions documented
 * in "perlxs".
 * 
 * We generate this file on the fly rather than maintain a static copy because
 * we want an entry for each Clownfish type so that we can differentiate
 * between them when checking arguments.  Keeping the entries up-to-date
 * manually as classes come and go would be a pain.
 * 
 * @param hierarchy A Clownfish::CFC::Hierarchy.
 */
void
CFCPerlTypeMap_write_xs_typemap(struct CFCHierarchy *hierarchy);

#ifdef __cplusplus
}
#endif

#endif /* H_CFCPERLTYPEMAP */

