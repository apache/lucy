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

#define C_LUCY_BITVECTOR
#include "Lucy/Util/ToolSet.h"

#include <math.h>

#include "Lucy/Object/BitVector.h"

// Shared subroutine for performing both OR and XOR ops.
#define DO_OR 1
#define DO_XOR 2
static void
S_do_or_or_xor(BitVector *self, const BitVector *other, int operation);

// Number of 1 bits given a u8 value.
static const uint32_t BYTE_COUNTS[256] = {
    0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8
};


BitVector*
BitVec_new(uint32_t capacity) {
    BitVector *self = (BitVector*)Class_Make_Obj(BITVECTOR);
    return BitVec_init(self, capacity);
}

BitVector*
BitVec_init(BitVector *self, uint32_t capacity) {
    BitVectorIVARS *const ivars = BitVec_IVARS(self);
    const uint32_t byte_size = (uint32_t)ceil(capacity / 8.0);

    // Derive.
    ivars->bits = capacity
                 ? (uint8_t*)CALLOCATE(byte_size, sizeof(uint8_t))
                 : NULL;

    // Assign.
    ivars->cap = byte_size * 8;

    return self;
}

void
BitVec_Destroy_IMP(BitVector* self) {
    BitVectorIVARS *const ivars = BitVec_IVARS(self);
    FREEMEM(ivars->bits);
    SUPER_DESTROY(self, BITVECTOR);
}

BitVector*
BitVec_Clone_IMP(BitVector *self) {
    BitVectorIVARS *const ivars = BitVec_IVARS(self);
    BitVector *other = BitVec_new(ivars->cap);
    uint32_t   byte_size = (uint32_t)ceil(ivars->cap / 8.0);
    BitVectorIVARS *const ovars = BitVec_IVARS(other);

    // Forbid inheritance.
    if (BitVec_Get_Class(self) != BITVECTOR) {
        THROW(ERR, "Attempt by %o to inherit BitVec_Clone",
              BitVec_Get_Class_Name(self));
    }

    memcpy(ovars->bits, ivars->bits, byte_size * sizeof(uint8_t));

    return other;
}

uint8_t*
BitVec_Get_Raw_Bits_IMP(BitVector *self) {
    return BitVec_IVARS(self)->bits;
}

uint32_t
BitVec_Get_Capacity_IMP(BitVector *self) {
    return BitVec_IVARS(self)->cap;
}

void
BitVec_Mimic_IMP(BitVector *self, Obj *other) {
    CERTIFY(other, BITVECTOR);
    BitVectorIVARS *const ivars = BitVec_IVARS(self);
    BitVectorIVARS *const ovars = BitVec_IVARS((BitVector*)other);
    const uint32_t my_byte_size = (uint32_t)ceil(ivars->cap / 8.0);
    const uint32_t other_byte_size = (uint32_t)ceil(ovars->cap / 8.0);
    if (my_byte_size > other_byte_size) {
        uint32_t space = my_byte_size - other_byte_size;
        memset(ivars->bits + other_byte_size, 0, space);
    }
    else if (my_byte_size < other_byte_size) {
        BitVec_Grow(self, ovars->cap - 1);
    }
    memcpy(ivars->bits, ovars->bits, other_byte_size);
}

void
BitVec_Grow_IMP(BitVector *self, uint32_t capacity) {
    BitVectorIVARS *const ivars = BitVec_IVARS(self);
    if (capacity > ivars->cap) {
        const size_t old_byte_cap  = (size_t)ceil(ivars->cap / 8.0);
        const size_t new_byte_cap  = (size_t)ceil((capacity + 1) / 8.0);
        const size_t num_new_bytes = new_byte_cap - old_byte_cap;

        ivars->bits = (uint8_t*)REALLOCATE(ivars->bits, new_byte_cap);
        memset(ivars->bits + old_byte_cap, 0, num_new_bytes);
        ivars->cap = new_byte_cap * 8;
    }
}

void
BitVec_Set_IMP(BitVector *self, uint32_t tick) {
    BitVectorIVARS *const ivars = BitVec_IVARS(self);
    if (tick >= ivars->cap) {
        uint32_t new_cap = (uint32_t)Memory_oversize(tick + 1, 0);
        BitVec_Grow(self, new_cap);
    }
    NumUtil_u1set(ivars->bits, tick);
}

void
BitVec_Clear_IMP(BitVector *self, uint32_t tick) {
    BitVectorIVARS *const ivars = BitVec_IVARS(self);
    if (tick >= ivars->cap) {
        return;
    }
    NumUtil_u1clear(ivars->bits, tick);
}

void
BitVec_Clear_All_IMP(BitVector *self) {
    BitVectorIVARS *const ivars = BitVec_IVARS(self);
    const size_t byte_size = (size_t)ceil(ivars->cap / 8.0);
    memset(ivars->bits, 0, byte_size);
}

bool
BitVec_Get_IMP(BitVector *self, uint32_t tick) {
    BitVectorIVARS *const ivars = BitVec_IVARS(self);
    if (tick >= ivars->cap) {
        return false;
    }
    return NumUtil_u1get(ivars->bits, tick);
}

static int32_t
S_first_bit_in_nonzero_byte(uint8_t num) {
    int32_t first_bit = 0;
    if ((num & 0xF) == 0) { first_bit += 4; num >>= 4; }
    if ((num & 0x3) == 0) { first_bit += 2; num >>= 2; }
    if ((num & 0x1) == 0) { first_bit += 1; }
    return first_bit;
}

int32_t
BitVec_Next_Hit_IMP(BitVector *self, uint32_t tick) {
    BitVectorIVARS *const ivars = BitVec_IVARS(self);
    size_t byte_size = (size_t)ceil(ivars->cap / 8.0);
    uint8_t *const limit = ivars->bits + byte_size;
    uint8_t *ptr = ivars->bits + (tick >> 3);

    if (ptr >= limit) {
        return -1;
    }
    else if (*ptr != 0) {
        // Special case the first byte.
        const int32_t base = (ptr - ivars->bits) * 8;
        const int32_t min_sub_tick = tick & 0x7;
        unsigned int byte = *ptr >> min_sub_tick;
        if (byte) {
            const int32_t candidate 
                = base + min_sub_tick + S_first_bit_in_nonzero_byte(byte);
            return candidate < (int32_t)ivars->cap ? candidate : -1;
        }
    }

    for (ptr++ ; ptr < limit; ptr++) {
        if (*ptr != 0) {
            // There's a non-zero bit in this byte.
            const int32_t base = (ptr - ivars->bits) * 8;
            const int32_t candidate = base + S_first_bit_in_nonzero_byte(*ptr);
            return candidate < (int32_t)ivars->cap ? candidate : -1;
        }
    }

    return -1;
}

void
BitVec_And_IMP(BitVector *self, const BitVector *other) {
    BitVectorIVARS *const ivars = BitVec_IVARS(self);
    const BitVectorIVARS *const ovars = BitVec_IVARS((BitVector*)other);
    uint8_t *bits_a = ivars->bits;
    uint8_t *bits_b = ovars->bits;
    const uint32_t min_cap = ivars->cap < ovars->cap
                             ? ivars->cap
                             : ovars->cap;
    const size_t byte_size = (size_t)ceil(min_cap / 8.0);
    uint8_t *const limit = bits_a + byte_size;

    // Intersection.
    while (bits_a < limit) {
        *bits_a &= *bits_b;
        bits_a++, bits_b++;
    }

    // Set all remaining to zero.
    if (ivars->cap > min_cap) {
        const size_t self_byte_size = (size_t)ceil(ivars->cap / 8.0);
        memset(bits_a, 0, self_byte_size - byte_size);
    }
}

void
BitVec_Or_IMP(BitVector *self, const BitVector *other) {
    S_do_or_or_xor(self, other, DO_OR);
}

void
BitVec_Xor_IMP(BitVector *self, const BitVector *other) {
    S_do_or_or_xor(self, other, DO_XOR);
}

static void
S_do_or_or_xor(BitVector *self, const BitVector *other, int operation) {
    BitVectorIVARS *const ivars = BitVec_IVARS(self);
    const BitVectorIVARS *const ovars = BitVec_IVARS((BitVector*)other);
    uint8_t *bits_a, *bits_b;
    uint32_t max_cap, min_cap;
    uint8_t *limit;
    double byte_size;

    // Sort out what the minimum and maximum caps are.
    if (ivars->cap < ovars->cap) {
        max_cap = ovars->cap;
        min_cap = ivars->cap;
    }
    else {
        max_cap = ivars->cap;
        min_cap = ovars->cap;
    }

    // Grow self if smaller than other, then calc pointers.
    if (max_cap > ivars->cap) { BitVec_Grow(self, max_cap); }
    bits_a        = ivars->bits;
    bits_b        = ovars->bits;
    byte_size     = ceil(min_cap / 8.0);
    limit         = ivars->bits + (size_t)byte_size;

    // Perform union of common bits.
    if (operation == DO_OR) {
        while (bits_a < limit) {
            *bits_a |= *bits_b;
            bits_a++, bits_b++;
        }
    }
    else if (operation == DO_XOR) {
        while (bits_a < limit) {
            *bits_a ^= *bits_b;
            bits_a++, bits_b++;
        }
    }
    else {
        THROW(ERR, "Unrecognized operation: %i32", (int32_t)operation);
    }

    // Copy remaining bits if other is bigger than self.
    if (ovars->cap > min_cap) {
        const double other_byte_size = ceil(ovars->cap / 8.0);
        const size_t bytes_to_copy = (size_t)(other_byte_size - byte_size);
        memcpy(bits_a, bits_b, bytes_to_copy);
    }
}

void
BitVec_And_Not_IMP(BitVector *self, const BitVector *other) {
    BitVectorIVARS *const ivars = BitVec_IVARS(self);
    const BitVectorIVARS *const ovars = BitVec_IVARS((BitVector*)other);
    uint8_t *bits_a = ivars->bits;
    uint8_t *bits_b = ovars->bits;
    const uint32_t min_cap = ivars->cap < ovars->cap
                             ? ivars->cap
                             : ovars->cap;
    const size_t byte_size = (size_t)ceil(min_cap / 8.0);
    uint8_t *const limit = bits_a + byte_size;

    // Clear bits set in other.
    while (bits_a < limit) {
        *bits_a &= ~(*bits_b);
        bits_a++, bits_b++;
    }
}

void
BitVec_Flip_IMP(BitVector *self, uint32_t tick) {
    BitVectorIVARS *const ivars = BitVec_IVARS(self);
    if (tick >= ivars->cap) {
        uint32_t new_cap = (uint32_t)Memory_oversize(tick + 1, 0);
        BitVec_Grow(self, new_cap);
    }
    NumUtil_u1flip(ivars->bits, tick);
}

void
BitVec_Flip_Block_IMP(BitVector *self, uint32_t offset, uint32_t length) {
    BitVectorIVARS *const ivars = BitVec_IVARS(self);
    uint32_t first = offset;
    uint32_t last  = offset + length - 1;

    // Bail if there's nothing to flip.
    if (!length) { return; }

    if (last >= ivars->cap) { BitVec_Grow(self, last + 1); }

    // Flip partial bytes.
    while (last % 8 != 0 && last > first) {
        NumUtil_u1flip(ivars->bits, last);
        last--;
    }
    while (first % 8 != 0 && first < last) {
        NumUtil_u1flip(ivars->bits, first);
        first++;
    }

    // Are first and last equal?
    if (first == last) {
        // There's only one bit left to flip.
        NumUtil_u1flip(ivars->bits, last);
    }
    // They must be multiples of 8, then.
    else {
        const uint32_t start_tick = first >> 3;
        const uint32_t limit_tick = last  >> 3;
        uint8_t *bits  = ivars->bits + start_tick;
        uint8_t *limit = ivars->bits + limit_tick;

        // Last actually belongs to the following byte (e.g. 8, in byte 2).
        NumUtil_u1flip(ivars->bits, last);

        // Flip whole bytes.
        for (; bits < limit; bits++) {
            *bits = ~(*bits);
        }
    }
}

uint32_t
BitVec_Count_IMP(BitVector *self) {
    BitVectorIVARS *const ivars = BitVec_IVARS(self);
    uint32_t count = 0;
    const size_t byte_size = (size_t)ceil(ivars->cap / 8.0);
    uint8_t *ptr = ivars->bits;
    uint8_t *const limit = ptr + byte_size;

    for (; ptr < limit; ptr++) {
        count += BYTE_COUNTS[*ptr];
    }

    return count;
}

I32Array*
BitVec_To_Array_IMP(BitVector *self) {
    BitVectorIVARS *const ivars = BitVec_IVARS(self);
    uint32_t        count     = BitVec_Count(self);
    uint32_t        num_left  = count;
    const uint32_t  capacity  = ivars->cap;
    uint32_t *const array     = (uint32_t*)CALLOCATE(count, sizeof(uint32_t));
    const size_t    byte_size = (size_t)ceil(ivars->cap / 8.0);
    uint8_t *const  bits      = ivars->bits;
    uint8_t *const  limit     = bits + byte_size;
    uint32_t        num       = 0;
    uint32_t        i         = 0;

    while (num_left) {
        uint8_t *ptr = bits + (num >> 3);
        while (ptr < limit && *ptr == 0) {
            num += 8;
            ptr++;
        }
        do {
            if (BitVec_Get(self, num)) {
                array[i++] = num;
                if (--num_left == 0) {
                    break;
                }
            }
            if (num >= capacity) {
                THROW(ERR, "Exceeded capacity: %u32 %u32", num, capacity);
            }
        } while (++num % 8);
    }

    return I32Arr_new_steal((int32_t*)array, count);
}


