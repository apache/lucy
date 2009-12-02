#define C_LUCY_SORTUTILS
#define LUCY_USE_SHORT_NAMES
#define CHY_USE_SHORT_NAMES

#include <string.h>
#include "Lucy/Util/SortUtils.h"
#include "Lucy/Object/Err.h"

/* Define four-byte and eight-byte types so that we can dereference void
 * pointers like integer pointers.  The only significance of using i32_t and
 * i64_t is that they are 4 and 8 bytes.
 */
#define FOUR_BYTE_TYPE  i32_t
#define EIGHT_BYTE_TYPE i64_t

/***************************** mergesort ************************************/

/* Recursive merge sorting functions. 
 */
static void
S_msort4(FOUR_BYTE_TYPE *elems, FOUR_BYTE_TYPE *scratch,
         u32_t left, u32_t right, Sort_compare_t compare, void *context);
static void
S_msort8(EIGHT_BYTE_TYPE *elems, EIGHT_BYTE_TYPE *scratch,
         u32_t left, u32_t right, Sort_compare_t compare, void *context);

/* Static inline versions of Sort_merge4 and Sort_merge8.
 */
static INLINE void
SI_merge4(FOUR_BYTE_TYPE *left_ptr,  u32_t left_size,
          FOUR_BYTE_TYPE *right_ptr, u32_t right_size,
          FOUR_BYTE_TYPE *dest, Sort_compare_t compare, void *context);
static INLINE void
SI_merge8(EIGHT_BYTE_TYPE *left_ptr,  u32_t left_size,
          EIGHT_BYTE_TYPE *right_ptr, u32_t right_size,
          EIGHT_BYTE_TYPE *dest, Sort_compare_t compare, void *context);

void
Sort_mergesort(void *elems, void *scratch, u32_t num_elems, u32_t width,
               Sort_compare_t compare, void *context) 
{
    /* Arrays of 0 or 1 items are already sorted. */
    if (num_elems < 2) { return; }

    /* Validate. */
    if (num_elems >= I32_MAX) {
        THROW(ERR, "Provided %u64 elems, but can't handle more than %i32",
            (u64_t)num_elems, I32_MAX);
    }

    /* Dispatch by element size. */
    if (width == 4) {
        S_msort4((FOUR_BYTE_TYPE*)elems, (FOUR_BYTE_TYPE*)scratch, 
            0, num_elems - 1, compare, context);
    }
    else if (width == 8) {
        S_msort8((EIGHT_BYTE_TYPE*)elems, (EIGHT_BYTE_TYPE*)scratch, 
            0, num_elems - 1, compare, context);
    }
    else {
        THROW(ERR, "Can't sort elements which are %u32 bytes", width);
    }
}

void
Sort_merge4(void *left_ptr,  u32_t left_size,
            void *right_ptr, u32_t right_size,
            void *dest, Sort_compare_t compare, void *context) 
{
    SI_merge4((FOUR_BYTE_TYPE*)left_ptr, left_size, 
              (FOUR_BYTE_TYPE*)right_ptr, right_size, 
              (FOUR_BYTE_TYPE*)dest, compare, context);
}

void
Sort_merge8(void *left_ptr,  u32_t left_size,
            void *right_ptr, u32_t right_size,
            void *dest, Sort_compare_t compare, void *context) 
{
    SI_merge8((EIGHT_BYTE_TYPE*)left_ptr, left_size, 
              (EIGHT_BYTE_TYPE*)right_ptr, right_size, 
              (EIGHT_BYTE_TYPE*)dest, compare, context);
}

static void
S_msort4(FOUR_BYTE_TYPE *elems, FOUR_BYTE_TYPE *scratch,
         u32_t left, u32_t right, Sort_compare_t compare, void *context)
{
    if (right > left) {
        const u32_t mid = ( (right+left)/2 ) + 1;
        S_msort4(elems, scratch, left, mid - 1, compare, context);
        S_msort4(elems, scratch, mid,  right, compare, context);
        Sort_merge4( (elems + left),  (mid - left), 
            (elems + mid), (right - mid + 1), scratch, compare, context);
        memcpy((elems + left), scratch,
            ((right - left + 1) * sizeof(FOUR_BYTE_TYPE)) );
    }
}

static void
S_msort8(EIGHT_BYTE_TYPE *elems, EIGHT_BYTE_TYPE *scratch,
         u32_t left, u32_t right, Sort_compare_t compare, void *context)
{
    if (right > left) {
        const u32_t mid = ( (right+left)/2 ) + 1;
        S_msort8(elems, scratch, left, mid - 1, compare, context);
        S_msort8(elems, scratch, mid,  right, compare, context);
        Sort_merge8( (elems + left),  (mid - left), 
            (elems + mid), (right - mid + 1), scratch, compare, context);
        memcpy((elems + left), scratch,
            ((right - left + 1) * sizeof(EIGHT_BYTE_TYPE)) );
    }
}

static INLINE void
SI_merge4(FOUR_BYTE_TYPE *left_ptr,  u32_t left_size,
          FOUR_BYTE_TYPE *right_ptr, u32_t right_size,
          FOUR_BYTE_TYPE *dest, Sort_compare_t compare, void *context) 
{
    FOUR_BYTE_TYPE *left_limit  = left_ptr  + left_size;
    FOUR_BYTE_TYPE *right_limit = right_ptr + right_size;

    while (left_ptr < left_limit && right_ptr < right_limit) {
        if (compare(context, left_ptr, right_ptr) < 1) {
            *dest++ = *left_ptr++;
        }
        else {
            *dest++ = *right_ptr++;
        }
    }
    while (left_ptr < left_limit) {
        *dest++ = *left_ptr++;
    }
    while (right_ptr < right_limit) {
        *dest++ = *right_ptr++;
    }
}

static INLINE void
SI_merge8(EIGHT_BYTE_TYPE *left_ptr,  u32_t left_size,
          EIGHT_BYTE_TYPE *right_ptr, u32_t right_size,
          EIGHT_BYTE_TYPE *dest, Sort_compare_t compare, void *context)
{
    EIGHT_BYTE_TYPE *left_limit  = left_ptr  + left_size;
    EIGHT_BYTE_TYPE *right_limit = right_ptr + right_size;

    while (left_ptr < left_limit && right_ptr < right_limit) {
        if (compare(context, left_ptr, right_ptr) < 1) {
            *dest++ = *left_ptr++;
        }
        else {
            *dest++ = *right_ptr++;
        }
    }
    while (left_ptr < left_limit) {
        *dest++ = *left_ptr++;
    }
    while (right_ptr < right_limit) {
        *dest++ = *right_ptr++;
    }
}

/***************************** quicksort ************************************/

/* Quicksort implementations optimized for four-byte and eight-byte elements.
 */
static void 
S_qsort4(FOUR_BYTE_TYPE *elems, i32_t left, i32_t right,
         Sort_compare_t compare, void *context);
static void 
S_qsort8(EIGHT_BYTE_TYPE *elems, i32_t left, i32_t right,
         Sort_compare_t compare, void *context);

/* Swap two elements. 
 */
static INLINE void
SI_exchange4(FOUR_BYTE_TYPE *elems, i32_t left, i32_t right);
static INLINE void
SI_exchange8(EIGHT_BYTE_TYPE *elems, i32_t left, i32_t right);

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
static INLINE FOUR_BYTE_TYPE*
SI_choose_pivot4(FOUR_BYTE_TYPE *elems, i32_t left, i32_t right,
                 Sort_compare_t compare, void *context);
static INLINE EIGHT_BYTE_TYPE*
SI_choose_pivot8(EIGHT_BYTE_TYPE *elems, i32_t left, i32_t right,
                 Sort_compare_t compare, void *context);

void
Sort_quicksort(void *elems, size_t num_elems, size_t width, 
               Sort_compare_t compare, void *context)
{
    /* Arrays of 0 or 1 items are already sorted. */
    if (num_elems < 2) { return; }

    /* Validate. */
    if (num_elems >= I32_MAX) {
        THROW(ERR, "Provided %u64 elems, but can't handle more than %i32",
            (u64_t)num_elems, I32_MAX);
    }

    if (width == 4) { 
        S_qsort4((FOUR_BYTE_TYPE*)elems, 0, num_elems - 1, compare, context); 
    }
    else if (width == 8) { 
        S_qsort8((EIGHT_BYTE_TYPE*)elems, 0, num_elems - 1, compare, context);
    }
    else {
        THROW(ERR, "Unsupported width: %i64", (i64_t)width);
    }
}

/************************* quicksort 4 byte *********************************/

static INLINE void
SI_exchange4(FOUR_BYTE_TYPE *elems, i32_t left, i32_t right)
{
    FOUR_BYTE_TYPE saved = elems[left];
    elems[left]  = elems[right];
    elems[right] = saved;
}

static INLINE FOUR_BYTE_TYPE*
SI_choose_pivot4(FOUR_BYTE_TYPE *elems, i32_t left, i32_t right,
                 Sort_compare_t compare, void *context)
{
    if (right - left > 1) { 
        i32_t mid = left + (right - left) / 2;
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
S_qsort4(FOUR_BYTE_TYPE *elems, i32_t left, i32_t right,
         Sort_compare_t compare, void *context)
{ 
    FOUR_BYTE_TYPE *const pivot 
        = SI_choose_pivot4(elems, left, right, compare, context);
    i32_t i = left - 1;
    i32_t j = right; 
    i32_t p = left - 1;
    i32_t q = right; 

    if (right <= left) { return; }

    while (1) {
        int comparison1;
        int comparison2;

        /* Find an element from the left that is greater than or equal to the
         * pivot (i.e. that should move to the right). */
        while (1) {
            i++;
            comparison1 = compare(context, elems + i, pivot);
            if (comparison1 >= 0) { break; }
        }

        /* Find an element from the right that is less than or equal to the
         * pivot (i.e. that should move to the left). */
        while (1) {
            j--;
            comparison2 = compare(context, elems + j, pivot);
            if (comparison2 <= 0) { break; }
            if (j == left)         { break; }
        }

        /* Bail out of loop when we meet in the middle. */
        if (i >= j) { break; }

        /* Swap the elements we found, so the lesser element moves left and
         * the greater element moves right. */
        SI_exchange4(elems, i, j);

        /* Move any elements which test as "equal" to the pivot to the outside
         * edges of the array. */
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
    {
        i32_t k;
        SI_exchange4(elems, i, right);
        j = i - 1;
        i++;
        for (k = left; k < p; k++, j--)      { SI_exchange4(elems, k, j); }
        for (k = right - 1; k > q; k--, i++) { SI_exchange4(elems, i, k); }
    }

    /* Recurse. */
    S_qsort4(elems, left, j, compare, context);   /* Sort less_than. */
    S_qsort4(elems, i, right, compare, context);  /* Sort greater_than. */
} 

/************************* quicksort 8 byte *********************************/

static INLINE void
SI_exchange8(EIGHT_BYTE_TYPE *elems, i32_t left, i32_t right)
{
    EIGHT_BYTE_TYPE saved = elems[left];
    elems[left]  = elems[right];
    elems[right] = saved;
}

static INLINE EIGHT_BYTE_TYPE*
SI_choose_pivot8(EIGHT_BYTE_TYPE *elems, i32_t left, i32_t right,
                 Sort_compare_t compare, void *context)
{
    if (right - left > 1) { 
        i32_t mid = left + (right - left) / 2;
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
S_qsort8(EIGHT_BYTE_TYPE *elems, i32_t left, i32_t right,
         Sort_compare_t compare, void *context)
{ 
    EIGHT_BYTE_TYPE *const pivot 
        = SI_choose_pivot8(elems, left, right, compare, context);
    i32_t i = left - 1;
    i32_t j = right; 
    i32_t p = left - 1;
    i32_t q = right; 

    if (right <= left) { return; }

    while (1) {
        int comparison1;
        int comparison2;

        /* Find an element from the left that is greater than or equal to the
         * pivot (i.e. that should move to the right). */
        while (1) {
            i++;
            comparison1 = compare(context, elems + i, pivot);
            if (comparison1 >= 0) { break; }
        }

        /* Find an element from the right that is less than or equal to the
         * pivot (i.e. that should move to the left). */
        while (1) {
            j--;
            comparison2 = compare(context, elems + j, pivot);
            if (comparison2 <= 0) { break; }
            if (j == left)         { break; }
        }

        /* Bail out of loop when we meet in the middle. */
        if (i >= j) { break; }

        /* Swap the elements we found, so the lesser element moves left and
         * the greater element moves right. */
        SI_exchange8(elems, i, j);

        /* Move any elements which test as "equal" to the pivot to the outside
         * edges of the array. */
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
    {
        i32_t k;
        SI_exchange8(elems, i, right);
        j = i - 1;
        i++;
        for (k = left; k < p; k++, j--)      { SI_exchange8(elems, k, j); }
        for (k = right - 1; k > q; k--, i++) { SI_exchange8(elems, i, k); }
    }

    /* Recurse. */
    S_qsort8(elems, left, j, compare, context);   /* Sort less_than. */
    S_qsort8(elems, i, right, compare, context);  /* Sort greater_than. */
} 

/* Copyright 2009 The Apache Software Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

