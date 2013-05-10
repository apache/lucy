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

#define C_LUCY_ZOMBIEKEYEDHASH
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Index/ZombieKeyedHash.h"
#include "Lucy/Plan/FieldType.h"
#include "Lucy/Util/MemoryPool.h"

ZombieKeyedHash*
ZKHash_new(MemoryPool *memory_pool, uint8_t primitive_id) {
    ZombieKeyedHash *self
        = (ZombieKeyedHash*)VTable_Make_Obj(ZOMBIEKEYEDHASH);
    Hash_init((Hash*)self, 0);
    ZombieKeyedHashIVARS *const ivars = ZKHash_IVARS(self);
    ivars->mem_pool = (MemoryPool*)INCREF(memory_pool);
    ivars->prim_id  = primitive_id;
    return self;
}

void
ZKHash_Destroy_IMP(ZombieKeyedHash *self) {
    ZombieKeyedHashIVARS *const ivars = ZKHash_IVARS(self);
    DECREF(ivars->mem_pool);
    SUPER_DESTROY(self, ZOMBIEKEYEDHASH);
}

Obj*
ZKHash_Make_Key_IMP(ZombieKeyedHash *self, Obj *key, int32_t hash_sum) {
    ZombieKeyedHashIVARS *const ivars = ZKHash_IVARS(self);
    UNUSED_VAR(hash_sum);
    Obj *retval = NULL;
    switch (ivars->prim_id & FType_PRIMITIVE_ID_MASK) {
        case FType_TEXT: {
                String *source = (String*)key;
                size_t size = SStr_size() + Str_Get_Size(source) + 1;
                void *allocation = MemPool_Grab(ivars->mem_pool, size);
                retval = (Obj*)SStr_new_from_str(allocation, size, source);
            }
            break;
        case FType_INT32: {
                size_t size = VTable_Get_Obj_Alloc_Size(INTEGER32);
                Integer32 *copy
                    = (Integer32*)MemPool_Grab(ivars->mem_pool, size);
                VTable_Init_Obj(INTEGER32, copy);
                Int32_init(copy, 0);
                Int32_Mimic(copy, key);
                retval = (Obj*)copy;
            }
            break;
        case FType_INT64: {
                size_t size = VTable_Get_Obj_Alloc_Size(INTEGER64);
                Integer64 *copy
                    = (Integer64*)MemPool_Grab(ivars->mem_pool, size);
                VTable_Init_Obj(INTEGER64, copy);
                Int64_init(copy, 0);
                Int64_Mimic(copy, key);
                retval = (Obj*)copy;
            }
            break;
        case FType_FLOAT32: {
                size_t size = VTable_Get_Obj_Alloc_Size(FLOAT32);
                Float32 *copy = (Float32*)MemPool_Grab(ivars->mem_pool, size);
                VTable_Init_Obj(FLOAT32, copy);
                Float32_init(copy, 0);
                Float32_Mimic(copy, key);
                retval = (Obj*)copy;
            }
            break;
        case FType_FLOAT64: {
                size_t size = VTable_Get_Obj_Alloc_Size(FLOAT64);
                Float64 *copy = (Float64*)MemPool_Grab(ivars->mem_pool, size);
                VTable_Init_Obj(FLOAT64, copy);
                Float64_init(copy, 0);
                Float64_Mimic(copy, key);
                retval = (Obj*)copy;
            }
            break;
        default:
            THROW(ERR, "Unrecognized primitive id: %i8", ivars->prim_id);
    }

    /* FIXME This is a hack.  It will leak memory if host objects get cached,
     * which in the present implementation will happen as soon as the refcount
     * reaches 4.  However, we must never call Destroy() for these objects,
     * because they will try to free() their initial allocation, which is
     * invalid because it's part of a MemoryPool arena. */
    retval = INCREF(retval);

    return retval;
}


