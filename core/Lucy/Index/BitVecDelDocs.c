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

#define C_LUCY_BITVECDELDOCS
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Index/BitVecDelDocs.h"
#include "Lucy/Store/Folder.h"
#include "Lucy/Store/InStream.h"

BitVecDelDocs*
BitVecDelDocs_new(Folder *folder, String *filename) {
    BitVecDelDocs *self = (BitVecDelDocs*)Class_Make_Obj(BITVECDELDOCS);
    return BitVecDelDocs_init(self, folder, filename);
}

BitVecDelDocs*
BitVecDelDocs_init(BitVecDelDocs *self, Folder *folder,
                   String *filename) {
    BitVec_init((BitVector*)self, 0);
    BitVecDelDocsIVARS *const ivars = BitVecDelDocs_IVARS(self);
    ivars->filename = Str_Clone(filename);
    ivars->instream = Folder_Open_In(folder, filename);
    if (!ivars->instream) {
        Err *error = (Err*)INCREF(Err_get_error());
        DECREF(self);
        RETHROW(error);
    }
    // Cast away const-ness of buffer as we have no immutable BitVector.
    int32_t len    = (int32_t)InStream_Length(ivars->instream);
    ivars->bits    = (uint8_t*)InStream_Buf(ivars->instream, len);
    ivars->cap     = (uint32_t)(len * 8);
    return self;
}

void
BitVecDelDocs_Destroy_IMP(BitVecDelDocs *self) {
    BitVecDelDocsIVARS *const ivars = BitVecDelDocs_IVARS(self);
    DECREF(ivars->filename);
    if (ivars->instream) {
        InStream_Close(ivars->instream);
        DECREF(ivars->instream);
    }
    ivars->bits = NULL;
    SUPER_DESTROY(self, BITVECDELDOCS);
}


