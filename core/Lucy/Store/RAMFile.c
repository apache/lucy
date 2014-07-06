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

#define C_LUCY_RAMFILE
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Store/RAMFile.h"

RAMFile*
RAMFile_new(ByteBuf *contents, bool read_only) {
    RAMFile *self = (RAMFile*)Class_Make_Obj(RAMFILE);
    return RAMFile_init(self, contents, read_only);
}

RAMFile*
RAMFile_init(RAMFile *self, ByteBuf *contents, bool read_only) {
    RAMFileIVARS *const ivars = RAMFile_IVARS(self);
    ivars->contents = contents ? (ByteBuf*)INCREF(contents) : BB_new(0);
    ivars->read_only = read_only;
    return self;
}

void
RAMFile_Destroy_IMP(RAMFile *self) {
    RAMFileIVARS *const ivars = RAMFile_IVARS(self);
    DECREF(ivars->contents);
    SUPER_DESTROY(self, RAMFILE);
}

ByteBuf*
RAMFile_Get_Contents_IMP(RAMFile *self) {
    return RAMFile_IVARS(self)->contents;
}

bool
RAMFile_Read_Only_IMP(RAMFile *self) {
    return RAMFile_IVARS(self)->read_only;
}

void
RAMFile_Set_Read_Only_IMP(RAMFile *self, bool read_only) {
    RAMFile_IVARS(self)->read_only = read_only;
}


