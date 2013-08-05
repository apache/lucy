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

#ifndef DSO_H
#define DSO_H

#include "oo.h"

extern class_t *OBJ;
extern size_t Obj_Hello_OFFSET;
extern method_t Obj_Hello_THUNK_PTR;
#define Obj_Hello_FIXED_OFFSET (5 * sizeof(void*))

void bootstrap();

obj_t *Obj_new(void);

void Obj_Hello_THUNK(obj_t *obj);

#endif /* DSO_H */

