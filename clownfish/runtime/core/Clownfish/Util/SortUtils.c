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

#define C_CFISH_SORTUTILS
#define CFISH_USE_SHORT_NAMES
#define CHY_USE_SHORT_NAMES

#include <string.h>
#include "Clownfish/Util/SortUtils.h"
#include "Clownfish/Err.h"

// Define four-byte and eight-byte types so that we can dereference void
// pointers like integer pointers.  The only significance of using int32_t and
// int64_t is that they are 4 and 8 bytes.
#define FOUR_BYTE_TYPE  int32_t
#define EIGHT_BYTE_TYPE int64_t

/***************************** mergesort ************************************/

// Recursive merge sorting functions.
static void
S_msort4(void *velems, void *vscratch, uint32_t left, uint32_t right,
         CFISH_Sort_Compare_t compare, void *context);
static void
S_msort8(void *velems, void *vscratch, uint32_t left, uint32_t right,
         CFISH_Sort_Compare_t compare, void *context);
static void
S_msort_any(void *velems, void *vscratch, uint32_t left, uint32_t right,
            CFISH_Sort_Compare_t compare, void *context, size_t width);

static CFISH_INLINE void
SI_merge(void *left_vptr,  uint32_t left_size,
         void *right_vptr, uint32_t right_size,
         void *vdest, size_t width, CFISH_Sort_Compare_t compare, void *context);

void
Sort_mergesort(void *elems, void *scratch, uint32_t num_elems, uint32_t width,
               CFISH_Sort_Compare_t compare, void *context) {
    // Arrays of 0 or 1 items are already sorted.
    if (num_elems < 2) { return; }

    // Validate.
    if (num_elems >= INT32_MAX) {
        THROW(ERR, "Provided %u64 elems, but can't handle more than %i32",
              (uint64_t)num_elems, INT32_MAX);
    }

    // Dispatch by element size.
    switch (width) {
        case 0:
            THROW(ERR, "Parameter 'width' cannot be 0");
            break;
        case 4:
            S_msort4(elems, scratch, 0, num_elems - 1, compare, context);
            break;
        case 8:
            S_msort8(elems, scratch, 0, num_elems - 1, compare, context);
            break;
        default:
            S_msort_any(elems, scratch, 0, num_elems - 1, compare,
                        context, width);
            break;
    }
}

void
Sort_merge(void *left_ptr,  uint32_t left_size,
           void *right_ptr, uint32_t right_size,
           void *dest, size_t width, CFISH_Sort_Compare_t compare,
           void *context) {
    switch (width) {
        case 0:
            THROW(ERR, "Parameter 'width' cannot be 0");
            break;
        case 4:
            SI_merge(left_ptr, left_size, right_ptr, right_size,
                     dest, 4, compare, context);
            break;
        case 8:
            SI_merge(left_ptr, left_size, right_ptr, right_size,
                     dest, 8, compare, context);
            break;
        default:
            SI_merge(left_ptr, left_size, right_ptr, right_size,
                     dest, width, compare, context);
            break;
    }
}

#define WIDTH 4
static void
S_msort4(void *velems, void *vscratch, uint32_t left, uint32_t right,
         CFISH_Sort_Compare_t compare, void *context) {
    uint8_t *elems   = (uint8_t*)velems;
    uint8_t *scratch = (uint8_t*)vscratch;
    if (right > left) {
        const uint32_t mid = ((right + left) / 2) + 1;
        S_msort4(elems, scratch, left, mid - 1, compare, context);
        S_msort4(elems, scratch, mid,  right, compare, context);
        SI_merge((elems + left * WIDTH), (mid - left),
                 (elems + mid * WIDTH), (right - mid + 1),
                 scratch, WIDTH, compare, context);
        memcpy((elems + left * WIDTH), scratch, ((right - left + 1) * WIDTH));
    }
}

#undef WIDTH
#define WIDTH 8
static void
S_msort8(void *velems, void *vscratch, uint32_t left, uint32_t right,
         CFISH_Sort_Compare_t compare, void *context) {
    uint8_t *elems   = (uint8_t*)velems;
    uint8_t *scratch = (uint8_t*)vscratch;
    if (right > left) {
        const uint32_t mid = ((right + left) / 2) + 1;
        S_msort8(elems, scratch, left, mid - 1, compare, context);
        S_msort8(elems, scratch, mid,  right, compare, context);
        SI_merge((elems + left * WIDTH), (mid - left),
                 (elems + mid * WIDTH), (right - mid + 1),
                 scratch, WIDTH, compare, context);
        memcpy((elems + left * WIDTH), scratch, ((right - left + 1) * WIDTH));
    }
}

#undef WIDTH
static void
S_msort_any(void *velems, void *vscratch, uint32_t left, uint32_t right,
            CFISH_Sort_Compare_t compare, void *context, size_t width) {
    uint8_t *elems   = (uint8_t*)velems;
    uint8_t *scratch = (uint8_t*)vscratch;
    if (right > left) {
        const uint32_t mid = ((right + left) / 2) + 1;
        S_msort_any(elems, scratch, left, mid - 1, compare, context, width);
        S_msort_any(elems, scratch, mid,  right,   compare, context, width);
        SI_merge((elems + left * width), (mid - left),
                 (elems + mid * width), (right - mid + 1),
                 scratch, width, compare, context);
        memcpy((elems + left * width), scratch, ((right - left + 1) * width));
    }
}

static CFISH_INLINE void
SI_merge(void *left_vptr,  uint32_t left_size,
         void *right_vptr, uint32_t right_size,
         void *vdest, size_t width, CFISH_Sort_Compare_t compare,
         void *context) {
    uint8_t *left_ptr    = (uint8_t*)left_vptr;
    uint8_t *right_ptr   = (uint8_t*)right_vptr;
    uint8_t *left_limit  = left_ptr + left_size * width;
    uint8_t *right_limit = right_ptr + right_size * width;
    uint8_t *dest        = (uint8_t*)vdest;

    while (left_ptr < left_limit && right_ptr < right_limit) {
        if (compare(context, left_ptr, right_ptr) < 1) {
            memcpy(dest, left_ptr, width);
            dest += width;
            left_ptr += width;
        }
        else {
            memcpy(dest, right_ptr, width);
            dest += width;
            right_ptr += width;
        }
    }

    const size_t left_remaining = left_limit - left_ptr;
    memcpy(dest, left_ptr, left_remaining);
    dest += left_remaining;
    const size_t right_remaining = right_limit - right_ptr;
    memcpy(dest, right_ptr, right_remaining);
}

/***************************** quicksort ************************************/

// Quicksort implementations optimized for four-byte and eight-byte elements.
static void
S_qsort4(FOUR_BYTE_TYPE *elems, int32_t left, int32_t right,
         CFISH_Sort_Compare_t compare, void *context);
static void
S_qsort8(EIGHT_BYTE_TYPE *elems, int32_t left, int32_t right,
         CFISH_Sort_Compare_t compare, void *context);

// Swap two elements.
static CFISH_INLINE void
SI_exchange4(FOUR_BYTE_TYPE *elems, int32_t left, int32_t right);
static CFISH_INLINE void
SI_exchange8(EIGHT_BYTE_TYPE *elems, int32_t left, int32_t right);

/* Select a pivot by choosing the median of three values, guarding against
 * the worst-case behavior of quicksort.  Place the pivot in the rightmost
 * slot.
 *
 * Possible states:
 *
 *   abc => abc => abc => acb
 *   acb => acb => acb => acb
 *   bac => abc => abc => acb
 *   bca => bca => acb => acb
 *   cba => bca => acb => acb
 *   cab => acb => acb => acb
 *   aab => aab => aab => aba
 *   aba => aba => aba => aba
 *   baa => aba => aba => aba
 *   bba => bba => abb => abb
 *   bab => abb => abb => abb
 *   abb => abb => abb => abb
 *   aaa => aaa => aaa => aaa
 */
static CFISH_INLINE FOUR_BYTE_TYPE*
SI_choose_pivot4(FOUR_BYTE_TYPE *elems, int32_t left, int32_t right,
                 CFISH_Sort_Compare_t compare, void *context);
static CFISH_INLINE EIGHT_BYTE_TYPE*
SI_choose_pivot8(EIGHT_BYTE_TYPE *elems, int32_t left, int32_t right,
                 CFISH_Sort_Compare_t compare, void *context);

void
Sort_quicksort(void *elems, size_t num_elems, size_t width,
               CFISH_Sort_Compare_t compare, void *context) {
    // Arrays of 0 or 1 items are already sorted.
    if (num_elems < 2) { return; }

    // Validate.
    if (num_elems >= INT32_MAX) {
        THROW(ERR, "Provided %u64 elems, but can't handle more than %i32",
              (uint64_t)num_elems, INT32_MAX);
    }

    if (width == 4) {
        S_qsort4((FOUR_BYTE_TYPE*)elems, 0, num_elems - 1, compare, context);
    }
    else if (width == 8) {
        S_qsort8((EIGHT_BYTE_TYPE*)elems, 0, num_elems - 1, compare, context);
    }
    else {
        THROW(ERR, "Unsupported width: %i64", (int64_t)width);
    }
}

/************************* quicksort 4 byte *********************************/

static CFISH_INLINE void
SI_exchange4(FOUR_BYTE_TYPE *elems, int32_t left, int32_t right) {
    FOUR_BYTE_TYPE saved = elems[left];
    elems[left]  = elems[right];
    elems[right] = saved;
}

static CFISH_INLINE FOUR_BYTE_TYPE*
SI_choose_pivot4(FOUR_BYTE_TYPE *elems, int32_t left, int32_t right,
                 CFISH_Sort_Compare_t compare, void *context) {
    if (right - left > 1) {
        int32_t mid = left + (right - left) / 2;
        if (compare(context, elems + left, elems + mid) > 0) {
            SI_exchange4(elems, left, mid);
        }
        if (compare(context, elems + left, elems + right) > 0) {
            SI_exchange4(elems, left, right);
        }
        if (compare(context, elems + right, elems + mid) > 0) {
            SI_exchange4(elems, right, mid);
        }
    }
    return elems + right;
}

static void
S_qsort4(FOUR_BYTE_TYPE *elems, int32_t left, int32_t right,
         CFISH_Sort_Compare_t compare, void *context) {
    FOUR_BYTE_TYPE *const pivot
        = SI_choose_pivot4(elems, left, right, compare, context);
    int32_t i = left - 1;
    int32_t j = right;
    int32_t p = left - 1;
    int32_t q = right;

    if (right <= left) { return; }

    /* TODO: A standard optimization for quicksort is to fall back to an
     * insertion sort when the the number of elements to be sorted becomes
     * small enough. */

    while (1) {
        int comparison1;
        int comparison2;

        // Find an element from the left that is greater than or equal to the
        // pivot (i.e. that should move to the right).
        while (1) {
            i++;
            comparison1 = compare(context, elems + i, pivot);
            if (comparison1 >= 0) { break; }
            if (i == right)       { break; }
        }

        // Find an element from the right that is less than or equal to the
        // pivot (i.e. that should move to the left).
        while (1) {
            j--;
            comparison2 = compare(context, elems + j, pivot);
            if (comparison2 <= 0) { break; }
            if (j == left)        { break; }
        }

        // Bail out of loop when we meet in the middle.
        if (i >= j) { break; }

        // Swap the elements we found, so the lesser element moves left and
        // the greater element moves right.
        SI_exchange4(elems, i, j);

        // Move any elements which test as "equal" to the pivot to the outside
        // edges of the array.
        if (comparison2 == 0) {
            p++;
            SI_exchange4(elems, p, i);
        }
        if (comparison1 == 0) {
            q--;
            SI_exchange4(elems, j, q);
        }
    }

    /* Move "equal" elements from the outside edges to the center.
     *
     * Before:
     *
     *    equal  |  less_than  |  greater_than  |  equal
     *
     * After:
     *
     *    less_than  |       equal       |  greater_than
     */
    SI_exchange4(elems, i, right);
    j = i - 1;
    i++;
    for (int32_t k = left; k < p; k++, j--)      { SI_exchange4(elems, k, j); }
    for (int32_t k = right - 1; k > q; k--, i++) { SI_exchange4(elems, i, k); }

    // Recurse.
    S_qsort4(elems, left, j, compare, context);   // Sort less_than.
    S_qsort4(elems, i, right, compare, context);  // Sort greater_than.
}

/************************* quicksort 8 byte *********************************/

static CFISH_INLINE void
SI_exchange8(EIGHT_BYTE_TYPE *elems, int32_t left, int32_t right) {
    EIGHT_BYTE_TYPE saved = elems[left];
    elems[left]  = elems[right];
    elems[right] = saved;
}

static CFISH_INLINE EIGHT_BYTE_TYPE*
SI_choose_pivot8(EIGHT_BYTE_TYPE *elems, int32_t left, int32_t right,
                 CFISH_Sort_Compare_t compare, void *context) {
    if (right - left > 1) {
        int32_t mid = left + (right - left) / 2;
        if (compare(context, elems + left, elems + mid) > 0) {
            SI_exchange8(elems, left, mid);
        }
        if (compare(context, elems + left, elems + right) > 0) {
            SI_exchange8(elems, left, right);
        }
        if (compare(context, elems + right, elems + mid) > 0) {
            SI_exchange8(elems, right, mid);
        }
    }
    return elems + right;
}

static void
S_qsort8(EIGHT_BYTE_TYPE *elems, int32_t left, int32_t right,
         CFISH_Sort_Compare_t compare, void *context) {
    EIGHT_BYTE_TYPE *const pivot
        = SI_choose_pivot8(elems, left, right, compare, context);
    int32_t i = left - 1;
    int32_t j = right;
    int32_t p = left - 1;
    int32_t q = right;

    if (right <= left) { return; }

    /* TODO: A standard optimization for quicksort is to fall back to an
     * insertion sort when the the number of elements to be sorted becomes
     * small enough. */

    while (1) {
        int comparison1;
        int comparison2;

        // Find an element from the left that is greater than or equal to the
        // pivot (i.e. that should move to the right).
        while (1) {
            i++;
            comparison1 = compare(context, elems + i, pivot);
            if (comparison1 >= 0) { break; }
            if (i == right)       { break; }
        }

        // Find an element from the right that is less than or equal to the
        // pivot (i.e. that should move to the left).
        while (1) {
            j--;
            comparison2 = compare(context, elems + j, pivot);
            if (comparison2 <= 0) { break; }
            if (j == left)        { break; }
        }

        // Bail out of loop when we meet in the middle.
        if (i >= j) { break; }

        // Swap the elements we found, so the lesser element moves left and
        // the greater element moves right.
        SI_exchange8(elems, i, j);

        // Move any elements which test as "equal" to the pivot to the outside
        // edges of the array.
        if (comparison2 == 0) {
            p++;
            SI_exchange8(elems, p, i);
        }
        if (comparison1 == 0) {
            q--;
            SI_exchange8(elems, j, q);
        }
    }

    /* Move "equal" elements from the outside edges to the center.
     *
     * Before:
     *
     *    equal  |  less_than  |  greater_than  |  equal
     *
     * After:
     *
     *    less_than  |       equal       |  greater_than
     */
    SI_exchange8(elems, i, right);
    j = i - 1;
    i++;
    for (int32_t k = left; k < p; k++, j--)      { SI_exchange8(elems, k, j); }
    for (int32_t k = right - 1; k > q; k--, i++) { SI_exchange8(elems, i, k); }

    // Recurse.
    S_qsort8(elems, left, j, compare, context);   // Sort less_than.
    S_qsort8(elems, i, right, compare, context);  // Sort greater_than.
}


