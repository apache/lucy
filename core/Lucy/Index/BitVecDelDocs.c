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
BitVecDelDocs_new(Folder *folder, const CharBuf *filename) {
    BitVecDelDocs *self = (BitVecDelDocs*)VTable_Make_Obj(BITVECDELDOCS);
    return BitVecDelDocs_init(self, folder, filename);
}

BitVecDelDocs*
BitVecDelDocs_init(BitVecDelDocs *self, Folder *folder,
                   const CharBuf *filename) {
    int32_t len;

    BitVec_init((BitVector*)self, 0);
    self->filename = CB_Clone(filename);
    self->instream = Folder_Open_In(folder, filename);
    if (!self->instream) {
        Err *error = (Err*)INCREF(Err_get_error());
        DECREF(self);
        RETHROW(error);
    }
    len            = (int32_t)InStream_Length(self->instream);
    self->bits     = (uint8_t*)InStream_Buf(self->instream, len);
    self->cap      = (uint32_t)(len * 8);
    return self;
}

void
BitVecDelDocs_destroy(BitVecDelDocs *self) {
    DECREF(self->filename);
    if (self->instream) {
        InStream_Close(self->instream);
        DECREF(self->instream);
    }
    self->bits = NULL;
    SUPER_DESTROY(self, BITVECDELDOCS);
}


