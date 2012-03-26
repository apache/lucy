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
    BitVector *self = (BitVector*)VTable_Make_Obj(BITVECTOR);
    return BitVec_init(self, capacity);
}

BitVector*
BitVec_init(BitVector *self, uint32_t capacity) {
    const uint32_t byte_size = (uint32_t)ceil(capacity / 8.0);

    // Derive.
    self->bits = capacity
                 ? (uint8_t*)CALLOCATE(byte_size, sizeof(uint8_t))
                 : NULL;

    // Assign.
    self->cap = byte_size * 8;

    return self;
}

void
BitVec_destroy(BitVector* self) {
    FREEMEM(self->bits);
    SUPER_DESTROY(self, BITVECTOR);
}

BitVector*
BitVec_clone(BitVector *self) {
    BitVector *twin = BitVec_new(self->cap);
    uint32_t   byte_size = (uint32_t)ceil(self->cap / 8.0);

    // Forbid inheritance.
    if (BitVec_Get_VTable(self) != BITVECTOR) {
        THROW(ERR, "Attempt by %o to inherit BitVec_Clone",
              BitVec_Get_Class_Name(self));
    }

    memcpy(twin->bits, self->bits, byte_size * sizeof(uint8_t));

    return twin;
}

uint8_t*
BitVec_get_raw_bits(BitVector *self) {
    return self->bits;
}

uint32_t
BitVec_get_capacity(BitVector *self) {
    return self->cap;
}

void
BitVec_mimic(BitVector *self, Obj *other) {
    BitVector *twin = (BitVector*)CERTIFY(other, BITVECTOR);
    const uint32_t my_byte_size = (uint32_t)ceil(self->cap / 8.0);
    const uint32_t twin_byte_size = (uint32_t)ceil(twin->cap / 8.0);
    if (my_byte_size > twin_byte_size) {
        uint32_t space = my_byte_size - twin_byte_size;
        memset(self->bits + twin_byte_size, 0, space);
    }
    else if (my_byte_size < twin_byte_size) {
        BitVec_Grow(self, twin->cap - 1);
    }
    memcpy(self->bits, twin->bits, twin_byte_size);
}

void
BitVec_grow(BitVector *self, uint32_t capacity) {
    if (capacity > self->cap) {
        const size_t old_byte_cap  = (size_t)ceil(self->cap / 8.0);
        const size_t new_byte_cap  = (size_t)ceil((capacity + 1) / 8.0);
        const size_t num_new_bytes = new_byte_cap - old_byte_cap;

        self->bits = (uint8_t*)REALLOCATE(self->bits, new_byte_cap);
        memset(self->bits + old_byte_cap, 0, num_new_bytes);
        self->cap = new_byte_cap * 8;
    }
}

void
BitVec_set(BitVector *self, uint32_t tick) {
    if (tick >= self->cap) {
        uint32_t new_cap = (uint32_t)Memory_oversize(tick + 1, 0);
        BitVec_Grow(self, new_cap);
    }
    NumUtil_u1set(self->bits, tick);
}

void
BitVec_clear(BitVector *self, uint32_t tick) {
    if (tick >= self->cap) {
        return;
    }
    NumUtil_u1clear(self->bits, tick);
}

void
BitVec_clear_all(BitVector *self) {
    const size_t byte_size = (size_t)ceil(self->cap / 8.0);
    memset(self->bits, 0, byte_size);
}

bool_t
BitVec_get(BitVector *self, uint32_t tick) {
    if (tick >= self->cap) {
        return false;
    }
    return NumUtil_u1get(self->bits, tick);
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
BitVec_next_hit(BitVector *self, uint32_t tick) {
    size_t byte_size = (size_t)ceil(self->cap / 8.0);
    uint8_t *const limit = self->bits + byte_size;
    uint8_t *ptr = self->bits + (tick >> 3);

    if (ptr >= limit) {
        return -1;
    }
    else if (*ptr != 0) {
        // Special case the first byte.
        const int32_t base = (ptr - self->bits) * 8;
        const int32_t min_sub_tick = tick & 0x7;
        unsigned int byte = *ptr >> min_sub_tick;
        if (byte) {
            const int32_t candidate 
                = base + min_sub_tick + S_first_bit_in_nonzero_byte(byte);
            return candidate < (int32_t)self->cap ? candidate : -1;
        }
    }

    for (ptr++ ; ptr < limit; ptr++) {
        if (*ptr != 0) {
            // There's a non-zero bit in this byte.
            const int32_t base = (ptr - self->bits) * 8;
            const int32_t candidate = base + S_first_bit_in_nonzero_byte(*ptr);
            return candidate < (int32_t)self->cap ? candidate : -1;
        }
    }

    return -1;
}

void
BitVec_and(BitVector *self, const BitVector *other) {
    uint8_t *bits_a = self->bits;
    uint8_t *bits_b = other->bits;
    const uint32_t min_cap = self->cap < other->cap
                             ? self->cap
                             : other->cap;
    const size_t byte_size = (size_t)ceil(min_cap / 8.0);
    uint8_t *const limit = bits_a + byte_size;

    // Intersection.
    while (bits_a < limit) {
        *bits_a &= *bits_b;
        bits_a++, bits_b++;
    }

    // Set all remaining to zero.
    if (self->cap > min_cap) {
        const size_t self_byte_size = (size_t)ceil(self->cap / 8.0);
        memset(bits_a, 0, self_byte_size - byte_size);
    }
}

void
BitVec_or(BitVector *self, const BitVector *other) {
    S_do_or_or_xor(self, other, DO_OR);
}

void
BitVec_xor(BitVector *self, const BitVector *other) {
    S_do_or_or_xor(self, other, DO_XOR);
}

static void
S_do_or_or_xor(BitVector *self, const BitVector *other, int operation) {
    uint8_t *bits_a, *bits_b;
    uint32_t max_cap, min_cap;
    uint8_t *limit;
    double byte_size;

    // Sort out what the minimum and maximum caps are.
    if (self->cap < other->cap) {
        max_cap = other->cap;
        min_cap = self->cap;
    }
    else {
        max_cap = self->cap;
        min_cap = other->cap;
    }

    // Grow self if smaller than other, then calc pointers.
    if (max_cap > self->cap) { BitVec_Grow(self, max_cap); }
    bits_a        = self->bits;
    bits_b        = other->bits;
    byte_size     = ceil(min_cap / 8.0);
    limit         = self->bits + (size_t)byte_size;

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
    if (other->cap > min_cap) {
        const double other_byte_size = ceil(other->cap / 8.0);
        const size_t bytes_to_copy = (size_t)(other_byte_size - byte_size);
        memcpy(bits_a, bits_b, bytes_to_copy);
    }
}

void
BitVec_and_not(BitVector *self, const BitVector *other) {
    uint8_t *bits_a = self->bits;
    uint8_t *bits_b = other->bits;
    const uint32_t min_cap = self->cap < other->cap
                             ? self->cap
                             : other->cap;
    const size_t byte_size = (size_t)ceil(min_cap / 8.0);
    uint8_t *const limit = bits_a + byte_size;

    // Clear bits set in other.
    while (bits_a < limit) {
        *bits_a &= ~(*bits_b);
        bits_a++, bits_b++;
    }
}

void
BitVec_flip(BitVector *self, uint32_t tick) {
    if (tick >= self->cap) {
        uint32_t new_cap = (uint32_t)Memory_oversize(tick + 1, 0);
        BitVec_Grow(self, new_cap);
    }
    NumUtil_u1flip(self->bits, tick);
}

void
BitVec_flip_block(BitVector *self, uint32_t offset, uint32_t length) {
    uint32_t first = offset;
    uint32_t last  = offset + length - 1;

    // Bail if there's nothing to flip.
    if (!length) { return; }

    if (last >= self->cap) { BitVec_Grow(self, last + 1); }

    // Flip partial bytes.
    while (last % 8 != 0 && last > first) {
        NumUtil_u1flip(self->bits, last);
        last--;
    }
    while (first % 8 != 0 && first < last) {
        NumUtil_u1flip(self->bits, first);
        first++;
    }

    // Are first and last equal?
    if (first == last) {
        // There's only one bit left to flip.
        NumUtil_u1flip(self->bits, last);
    }
    // They must be multiples of 8, then.
    else {
        const uint32_t start_tick = first >> 3;
        const uint32_t limit_tick = last  >> 3;
        uint8_t *bits  = self->bits + start_tick;
        uint8_t *limit = self->bits + limit_tick;

        // Last actually belongs to the following byte (e.g. 8, in byte 2).
        NumUtil_u1flip(self->bits, last);

        // Flip whole bytes.
        for (; bits < limit; bits++) {
            *bits = ~(*bits);
        }
    }
}

uint32_t
BitVec_count(BitVector *self) {
    uint32_t count = 0;
    const size_t byte_size = (size_t)ceil(self->cap / 8.0);
    uint8_t *ptr = self->bits;
    uint8_t *const limit = ptr + byte_size;

    for (; ptr < limit; ptr++) {
        count += BYTE_COUNTS[*ptr];
    }

    return count;
}

I32Array*
BitVec_to_array(BitVector *self) {
    uint32_t        count     = BitVec_Count(self);
    uint32_t        num_left  = count;
    const uint32_t  capacity  = self->cap;
    uint32_t *const array     = (uint32_t*)CALLOCATE(count, sizeof(uint32_t));
    const size_t    byte_size = (size_t)ceil(self->cap / 8.0);
    uint8_t *const  bits      = self->bits;
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


