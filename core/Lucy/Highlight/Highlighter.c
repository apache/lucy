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

#define C_LUCY_HIGHLIGHTER
#include <ctype.h>
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Highlight/Highlighter.h"
#include "Clownfish/CharBuf.h"
#include "Lucy/Document/HitDoc.h"
#include "Lucy/Search/Compiler.h"
#include "Lucy/Search/Query.h"
#include "Lucy/Search/Searcher.h"
#include "Lucy/Highlight/HeatMap.h"
#include "Lucy/Search/Span.h"
#include "Lucy/Index/DocVector.h"

const int32_t ELLIPSIS_CODE_POINT = 0x2026;

/* If Highlighter_Encode has been overridden, return its output.  If not,
 * increment the refcount of the supplied encode_buf and call encode_entities.
 * Either way, the caller takes responsibility for one refcount.
 *
 * The point of this routine is to minimize String object creation when
 * possible.
 */
static String*
S_do_encode(Highlighter *self, String *text, CharBuf **encode_buf);

// Place HTML entity encoded version of [text] into [encoded].
static String*
S_encode_entities(String *text, CharBuf *encoded);

Highlighter*
Highlighter_new(Searcher *searcher, Obj *query, String *field,
                uint32_t excerpt_length) {
    Highlighter *self = (Highlighter*)Class_Make_Obj(HIGHLIGHTER);
    return Highlighter_init(self, searcher, query, field, excerpt_length);
}

Highlighter*
Highlighter_init(Highlighter *self, Searcher *searcher, Obj *query,
                 String *field, uint32_t excerpt_length) {
    HighlighterIVARS *const ivars = Highlighter_IVARS(self);
    ivars->query          = Searcher_Glean_Query(searcher, query);
    ivars->searcher       = (Searcher*)INCREF(searcher);
    ivars->field          = Str_Clone(field);
    ivars->excerpt_length = excerpt_length;
    ivars->slop           = excerpt_length / 3;
    ivars->pre_tag        = Str_new_from_trusted_utf8("<strong>", 8);
    ivars->post_tag       = Str_new_from_trusted_utf8("</strong>", 9);
    if (Query_Is_A(ivars->query, COMPILER)) {
        ivars->compiler = (Compiler*)INCREF(ivars->query);
    }
    else {
        ivars->compiler = Query_Make_Compiler(ivars->query, searcher,
                                              Query_Get_Boost(ivars->query),
                                              false);
    }
    return self;
}

void
Highlighter_Destroy_IMP(Highlighter *self) {
    HighlighterIVARS *const ivars = Highlighter_IVARS(self);
    DECREF(ivars->searcher);
    DECREF(ivars->query);
    DECREF(ivars->compiler);
    DECREF(ivars->field);
    DECREF(ivars->pre_tag);
    DECREF(ivars->post_tag);
    SUPER_DESTROY(self, HIGHLIGHTER);
}

String*
Highlighter_Highlight_IMP(Highlighter *self, String *text) {
    HighlighterIVARS *const ivars = Highlighter_IVARS(self);
    size_t size = Str_Get_Size(text)
                  + Str_Get_Size(ivars->pre_tag)
                  + Str_Get_Size(ivars->post_tag);
    CharBuf *buf = CB_new(size);
    CB_Cat(buf, ivars->pre_tag);
    CB_Cat(buf, text);
    CB_Cat(buf, ivars->post_tag);
    String *retval = CB_Yield_String(buf);
    DECREF(buf);
    return retval;
}

void
Highlighter_Set_Pre_Tag_IMP(Highlighter *self, String *pre_tag) {
    HighlighterIVARS *const ivars = Highlighter_IVARS(self);
    DECREF(ivars->pre_tag);
    ivars->pre_tag = Str_Clone(pre_tag);
}

void
Highlighter_Set_Post_Tag_IMP(Highlighter *self, String *post_tag) {
    HighlighterIVARS *const ivars = Highlighter_IVARS(self);
    DECREF(ivars->post_tag);
    ivars->post_tag = Str_Clone(post_tag);
}

String*
Highlighter_Get_Pre_Tag_IMP(Highlighter *self) {
    return Highlighter_IVARS(self)->pre_tag;
}

String*
Highlighter_Get_Post_Tag_IMP(Highlighter *self) {
    return Highlighter_IVARS(self)->post_tag;
}

String*
Highlighter_Get_Field_IMP(Highlighter *self) {
    return Highlighter_IVARS(self)->field;
}

Query*
Highlighter_Get_Query_IMP(Highlighter *self) {
    return Highlighter_IVARS(self)->query;
}

Searcher*
Highlighter_Get_Searcher_IMP(Highlighter *self) {
    return Highlighter_IVARS(self)->searcher;
}

Compiler*
Highlighter_Get_Compiler_IMP(Highlighter *self) {
    return Highlighter_IVARS(self)->compiler;
}

uint32_t
Highlighter_Get_Excerpt_Length_IMP(Highlighter *self) {
    return Highlighter_IVARS(self)->excerpt_length;
}

String*
Highlighter_Create_Excerpt_IMP(Highlighter *self, HitDoc *hit_doc) {
    HighlighterIVARS *const ivars = Highlighter_IVARS(self);
    String *field_val = (String*)HitDoc_Extract(hit_doc, ivars->field);
    String *retval;

    if (!field_val || !Obj_Is_A((Obj*)field_val, STRING)) {
        retval = NULL;
    }
    else if (!Str_Get_Size(field_val)) {
        // Empty string yields empty string.
        retval = Str_new_from_trusted_utf8("", 0);
    }
    else {
        DocVector *doc_vec
            = Searcher_Fetch_Doc_Vec(ivars->searcher,
                                     HitDoc_Get_Doc_ID(hit_doc));
        VArray *maybe_spans
            = Compiler_Highlight_Spans(ivars->compiler, ivars->searcher,
                                       doc_vec, ivars->field);
        VArray *score_spans = maybe_spans ? maybe_spans : VA_new(0);
        VA_Sort(score_spans, NULL, NULL);
        HeatMap *heat_map
            = HeatMap_new(score_spans, (ivars->excerpt_length * 2) / 3);

        int32_t top;
        String *raw_excerpt
            = Highlighter_Raw_Excerpt(self, field_val, &top, heat_map);
        String *highlighted
            = Highlighter_Highlight_Excerpt(self, score_spans, raw_excerpt,
                                            top);

        DECREF(raw_excerpt);
        DECREF(heat_map);
        DECREF(score_spans);
        DECREF(doc_vec);

        retval = highlighted;
    }

    DECREF(field_val);
    return retval;
}

static int32_t
S_hottest(HeatMap *heat_map) {
    float max_score = 0.0f;
    int32_t retval = 0;
    VArray *spans = HeatMap_Get_Spans(heat_map);
    for (uint32_t i = VA_Get_Size(spans); i--;) {
        Span *span = (Span*)VA_Fetch(spans, i);
        if (Span_Get_Weight(span) >= max_score) {
            retval = Span_Get_Offset(span);
            max_score = Span_Get_Weight(span);
        }
    }
    return retval;
}

// Find a starting boundary after the current position given by the iterator.
// Skip up to max_skip code points plus potential whitespace. Update the
// iterator and return number of code points skipped. Return true if a
// starting edge (sentence) was found.
bool
S_find_starting_boundary(StringIterator *top, uint32_t max_skip,
                         uint32_t *num_skipped_ptr) {
    // Keep track of the first word boundary.
    StringIterator *word = NULL;
    uint32_t word_offset = 0;

    // Check if we're at a starting boundary already.

    StringIterator *iter = StrIter_Clone(top);

    while (true) {
        int32_t code_point = StrIter_Prev(iter);

        if (code_point == STRITER_DONE || code_point == '.') {
            // Skip remaining whitespace.
            *num_skipped_ptr = StrIter_Skip_Next_Whitespace(top);
            DECREF(iter);
            return true;
        }

        if (StrHelp_is_whitespace(code_point)) {
            if (word == NULL) { word = StrIter_Clone(top); }
        }
        else {
            break;
        }
    }

    // Try to start on a boundary.

    uint32_t num_skipped = 0;
    bool     found_edge  = false;

    StrIter_Assign(iter, top);

    for (uint32_t i = 0; i < max_skip; ++i) {
        int32_t code_point = StrIter_Next(iter);

        if (code_point == STRITER_DONE || code_point == '.') {
            found_edge = true;
            StrIter_Assign(top, iter);
            num_skipped = i + 1;
            break;
        }

        if (word == NULL && StrHelp_is_whitespace(code_point)) {
            word = StrIter_Clone(iter);
            word_offset = i + 1;
        }
    }

    // Try to use word boundary if no sentence boundary was found.
    if (!found_edge && word != NULL) {
        StrIter_Assign(top, word);
        num_skipped = word_offset;
    }

    // Skip remaining whitespace.
    num_skipped += StrIter_Skip_Next_Whitespace(top);
    *num_skipped_ptr = num_skipped;

    DECREF(word);
    DECREF(iter);
    return found_edge;
}

// Find an ending boundary before the current position given by the iterator.
// Skip up to max_skip code points plus potential whitespace. Update the
// iterator and return number of code points skipped. Return true if a
// ending edge (sentence) was found.
bool
S_find_ending_boundary(StringIterator *tail, uint32_t max_skip,
                       uint32_t *num_skipped_ptr) {
    int32_t code_point;

    // Check if we're at an ending boundary already. Don't check for a word
    // boundary because we need space for a trailing ellipsis.

    StringIterator *iter = StrIter_Clone(tail);

    do {
        code_point = StrIter_Next(iter);

        if (code_point == STRITER_DONE) {
            // Skip remaining whitespace.
            *num_skipped_ptr = StrIter_Skip_Prev_Whitespace(tail);
            DECREF(iter);
            return true;
        }
    } while (StrHelp_is_whitespace(code_point));

    // Keep track of the first word boundary.
    StringIterator *word = NULL;
    uint32_t word_offset = 0;

    StrIter_Assign(iter, tail);

    for (uint32_t i = 0;
         STRITER_DONE != (code_point = StrIter_Prev(iter));
         ++i)
    {
        if (code_point == '.') {
            StrIter_Assign(tail, iter);
            StrIter_Advance(tail, 1); // Include period.
            *num_skipped_ptr = i;
            DECREF(word);
            DECREF(iter);
            return true;
        }

        if (StrHelp_is_whitespace(code_point)) {
            if (word == NULL) {
                word = StrIter_Clone(iter);
                word_offset = i + 1;
            }
        }
        else if (i >= max_skip) {
            // Break only at non-whitespace to allow another sentence
            // boundary to be found.
            break;
        }
    }

    if (word == NULL) {
        // Make space for ellipsis.
        *num_skipped_ptr = StrIter_Recede(tail, 1);
    }
    else {
        // Use word boundary if no sentence boundary was found.
        StrIter_Assign(tail, word);

        // Strip whitespace and punctuation that collides with an ellipsis.
        while (STRITER_DONE != (code_point = StrIter_Prev(tail))) {
            if (!StrHelp_is_whitespace(code_point)
                && code_point != '.'
                && code_point != ','
                && code_point != ';'
                && code_point != ':'
                && code_point != ':'
                && code_point != '?'
                && code_point != '!'
               ) {
                StrIter_Advance(tail, 1); // Back up.
                break;
            }
            ++word_offset;
        }

        *num_skipped_ptr = word_offset;
    }

    DECREF(word);
    DECREF(iter);
    return false;
}

String*
Highlighter_Raw_Excerpt_IMP(Highlighter *self, String *field_val,
                            int32_t *start_ptr, HeatMap *heat_map) {
    HighlighterIVARS *const ivars = Highlighter_IVARS(self);

    // Find start of excerpt.

    StringIterator *top = Str_Top(field_val);

    int32_t  best_location = S_hottest(heat_map);
    int32_t  start;
    uint32_t max_skip;

    if ((uint32_t)best_location <= ivars->slop) {
        // If the beginning of the string falls within the window centered
        // around the hottest point in the field, start the fragment at the
        // beginning.
        start    = 0;
        max_skip = best_location;
    }
    else {
        start    = best_location - ivars->slop;
        max_skip = ivars->slop;
        StrIter_Advance(top, start);
    }

    uint32_t num_skipped;
    bool found_starting_edge
        = S_find_starting_boundary(top, max_skip, &num_skipped);
    start += num_skipped;

    // Find end of excerpt.

    StringIterator *tail = StrIter_Clone(top);

    uint32_t max_len = ivars->excerpt_length;
    if (!found_starting_edge) {
        // Leave space for starting ellipsis and space character.
        max_len -= 2;
    }

    bool found_ending_edge = true;
    uint32_t excerpt_len = StrIter_Advance(tail, max_len);

    // Skip up to slop code points but keep at least max_len - slop.
    if (excerpt_len > max_len - ivars->slop) {
        max_skip = excerpt_len - (max_len - ivars->slop);
        found_ending_edge
            = S_find_ending_boundary(tail, max_skip, &num_skipped);
        if (num_skipped >= excerpt_len) {
            excerpt_len = 0;
        }
        else {
            excerpt_len -= num_skipped;
        }
    }

    // Extract excerpt.

    String *raw_excerpt;

    if (!excerpt_len) {
        raw_excerpt = Str_new_from_trusted_utf8("", 0);
    }
    else {
        String  *substring = StrIter_substring(top, tail);
        CharBuf *buf       = CB_new(Str_Get_Size(substring) + 8);

        // If not starting on a sentence boundary, prepend an ellipsis.
        if (!found_starting_edge) {
            CB_Cat_Char(buf, ELLIPSIS_CODE_POINT);
            CB_Cat_Char(buf, ' ');
            start -= 2;
        }

        CB_Cat(buf, substring);

        // If not ending on a sentence boundary, append an ellipsis.
        if (!found_ending_edge) {
            CB_Cat_Char(buf, ELLIPSIS_CODE_POINT);
        }

        raw_excerpt = CB_Yield_String(buf);

        DECREF(buf);
        DECREF(substring);
    }

    *start_ptr = start;

    DECREF(top);
    DECREF(tail);
    return raw_excerpt;
}

String*
Highlighter_Highlight_Excerpt_IMP(Highlighter *self, VArray *spans,
                                  String *raw_excerpt, int32_t top) {
    int32_t         hl_start        = 0;
    int32_t         hl_end          = 0;
    StringIterator *iter            = Str_Top(raw_excerpt);
    StringIterator *temp            = Str_Top(raw_excerpt);
    CharBuf        *buf             = CB_new(Str_Get_Size(raw_excerpt) + 32);
    CharBuf        *encode_buf      = NULL;
    int32_t         raw_excerpt_end = top + Str_Length(raw_excerpt);

    for (uint32_t i = 0, max = VA_Get_Size(spans); i < max; i++) {
        Span *span = (Span*)VA_Fetch(spans, i);
        int32_t offset = Span_Get_Offset(span);
        if (offset < top) {
            continue;
        }
        else if (offset >= raw_excerpt_end) {
            break;
        }
        else {
            int32_t relative_start = offset - top;
            int32_t relative_end   = relative_start + Span_Get_Length(span);

            if (relative_start <= hl_end) {
                if (relative_end > hl_end) {
                    hl_end = relative_end;
                }
            }
            else {
                if (hl_start < hl_end) {
                    // Highlight previous section
                    int32_t highlighted_len = hl_end - hl_start;
                    StrIter_Assign(temp, iter);
                    StrIter_Advance(iter, highlighted_len);
                    String *to_cat = StrIter_substring(temp, iter);
                    String *encoded = S_do_encode(self, to_cat, &encode_buf);
                    String *hl_frag = Highlighter_Highlight(self, encoded);
                    CB_Cat(buf, hl_frag);
                    DECREF(hl_frag);
                    DECREF(encoded);
                    DECREF(to_cat);
                }

                int32_t non_highlighted_len = relative_start - hl_end;
                StrIter_Assign(temp, iter);
                StrIter_Advance(iter, non_highlighted_len);
                String *to_cat = StrIter_substring(temp, iter);
                String *encoded = S_do_encode(self, to_cat, &encode_buf);
                CB_Cat(buf, (String*)encoded);
                DECREF(encoded);
                DECREF(to_cat);

                hl_start = relative_start;
                hl_end   = relative_end;
            }
        }
    }

    if (hl_start < hl_end) {
        // Highlight final section
        int32_t highlighted_len = hl_end - hl_start;
        StrIter_Assign(temp, iter);
        StrIter_Advance(iter, highlighted_len);
        String *to_cat = StrIter_substring(temp, iter);
        String *encoded = S_do_encode(self, to_cat, &encode_buf);
        String *hl_frag = Highlighter_Highlight(self, encoded);
        CB_Cat(buf, hl_frag);
        DECREF(hl_frag);
        DECREF(encoded);
        DECREF(to_cat);
    }

    // Last text, beyond last highlight span.
    if (StrIter_Has_Next(iter)) {
        String *to_cat = StrIter_substring(iter, NULL);
        String *encoded = S_do_encode(self, to_cat, &encode_buf);
        CB_Cat(buf, encoded);
        DECREF(encoded);
        DECREF(to_cat);
    }

    String *highlighted = CB_Yield_String(buf);
    DECREF(encode_buf);
    DECREF(buf);
    DECREF(temp);
    DECREF(iter);
    return highlighted;
}

String*
Highlighter_Encode_IMP(Highlighter *self, String *text) {
    UNUSED_VAR(self);
    CharBuf *encode_buf = CB_new(0);
    String *encoded = S_encode_entities(text, encode_buf);
    DECREF(encode_buf);
    return encoded;
}

static String*
S_do_encode(Highlighter *self, String *text, CharBuf **encode_buf) {
    Class *klass = Highlighter_Get_Class(self);
    Highlighter_Encode_t my_meth
        = (Highlighter_Encode_t)METHOD_PTR(klass, LUCY_Highlighter_Encode);
    Highlighter_Encode_t orig_meth
        = (Highlighter_Encode_t)METHOD_PTR(HIGHLIGHTER, LUCY_Highlighter_Encode);

    if (my_meth != orig_meth) {
        return my_meth(self, text);
    }
    else {
        if (*encode_buf == NULL) { *encode_buf = CB_new(0); }
        return S_encode_entities(text, *encode_buf);
    }
}

static String*
S_encode_entities(String *text, CharBuf *buf) {
    StringIterator *iter = Str_Top(text);
    size_t space = 0;
    const int MAX_ENTITY_BYTES = 9; // &#dddddd;

    // Scan first so that we only allocate once.
    int32_t code_point;
    while (STRITER_DONE != (code_point = StrIter_Next(iter))) {
        if (code_point > 127
            || (!isgraph(code_point) && !isspace(code_point))
            || code_point == '<'
            || code_point == '>'
            || code_point == '&'
            || code_point == '"'
           ) {
            space += MAX_ENTITY_BYTES;
        }
        else {
            space += 1;
        }
    }

    CB_Grow(buf, space);
    CB_Set_Size(buf, 0);
    DECREF(iter);
    iter = Str_Top(text);
    while (STRITER_DONE != (code_point = StrIter_Next(iter))) {
        if (code_point > 127
            || (!isgraph(code_point) && !isspace(code_point))
           ) {
            CB_catf(buf, "&#%u32;", code_point);
        }
        else if (code_point == '<') {
            CB_Cat_Trusted_Utf8(buf, "&lt;", 4);
        }
        else if (code_point == '>') {
            CB_Cat_Trusted_Utf8(buf, "&gt;", 4);
        }
        else if (code_point == '&') {
            CB_Cat_Trusted_Utf8(buf, "&amp;", 5);
        }
        else if (code_point == '"') {
            CB_Cat_Trusted_Utf8(buf, "&quot;", 6);
        }
        else {
            CB_Cat_Char(buf, code_point);
        }
    }

    DECREF(iter);
    return CB_To_String(buf);
}



