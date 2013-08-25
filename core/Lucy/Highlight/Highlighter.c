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
#include "Lucy/Document/HitDoc.h"
#include "Lucy/Search/Compiler.h"
#include "Lucy/Search/Query.h"
#include "Lucy/Search/Searcher.h"
#include "Lucy/Highlight/HeatMap.h"
#include "Lucy/Search/Span.h"
#include "Lucy/Index/DocVector.h"

const uint32_t ELLIPSIS_CODE_POINT = 0x2026;

/* If Highlighter_Encode has been overridden, return its output.  If not,
 * increment the refcount of the supplied encode_buf and call encode_entities.
 * Either way, the caller takes responsibility for one refcount.
 *
 * The point of this routine is to minimize CharBuf object creation when
 * possible.
 */
static CharBuf*
S_do_encode(Highlighter *self, CharBuf *text, CharBuf **encode_buf);

// Place HTML entity encoded version of [text] into [encoded].
static CharBuf*
S_encode_entities(CharBuf *text, CharBuf *encoded);

Highlighter*
Highlighter_new(Searcher *searcher, Obj *query, const CharBuf *field,
                uint32_t excerpt_length) {
    Highlighter *self = (Highlighter*)VTable_Make_Obj(HIGHLIGHTER);
    return Highlighter_init(self, searcher, query, field, excerpt_length);
}

Highlighter*
Highlighter_init(Highlighter *self, Searcher *searcher, Obj *query,
                 const CharBuf *field, uint32_t excerpt_length) {
    HighlighterIVARS *const ivars = Highlighter_IVARS(self);
    ivars->query          = Searcher_Glean_Query(searcher, query);
    ivars->searcher       = (Searcher*)INCREF(searcher);
    ivars->field          = CB_Clone(field);
    ivars->excerpt_length = excerpt_length;
    ivars->slop           = excerpt_length / 3;
    ivars->window_width   = excerpt_length + (ivars->slop * 2);
    ivars->pre_tag        = CB_new_from_trusted_utf8("<strong>", 8);
    ivars->post_tag       = CB_new_from_trusted_utf8("</strong>", 9);
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

CharBuf*
Highlighter_Highlight_IMP(Highlighter *self, const CharBuf *text) {
    HighlighterIVARS *const ivars = Highlighter_IVARS(self);
    size_t size = CB_Get_Size(text)
                  + CB_Get_Size(ivars->pre_tag)
                  + CB_Get_Size(ivars->post_tag);
    CharBuf *retval = CB_new(size);
    CB_Cat(retval, ivars->pre_tag);
    CB_Cat(retval, text);
    CB_Cat(retval, ivars->post_tag);
    return retval;
}

void
Highlighter_Set_Pre_Tag_IMP(Highlighter *self, const CharBuf *pre_tag) {
    HighlighterIVARS *const ivars = Highlighter_IVARS(self);
    CB_Mimic(ivars->pre_tag, (Obj*)pre_tag);
}

void
Highlighter_Set_Post_Tag_IMP(Highlighter *self, const CharBuf *post_tag) {
    HighlighterIVARS *const ivars = Highlighter_IVARS(self);
    CB_Mimic(ivars->post_tag, (Obj*)post_tag);
}

CharBuf*
Highlighter_Get_Pre_Tag_IMP(Highlighter *self) {
    return Highlighter_IVARS(self)->pre_tag;
}

CharBuf*
Highlighter_Get_Post_Tag_IMP(Highlighter *self) {
    return Highlighter_IVARS(self)->post_tag;
}

CharBuf*
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

CharBuf*
Highlighter_Create_Excerpt_IMP(Highlighter *self, HitDoc *hit_doc) {
    HighlighterIVARS *const ivars = Highlighter_IVARS(self);
    ZombieCharBuf *field_val
        = (ZombieCharBuf*)HitDoc_Extract(hit_doc, ivars->field,
                                         (ViewCharBuf*)ZCB_BLANK());

    if (!field_val || !Obj_Is_A((Obj*)field_val, CHARBUF)) {
        return NULL;
    }
    else if (!ZCB_Get_Size(field_val)) {
        // Empty string yields empty string.
        return CB_new(0);
    }
    else {
        ZombieCharBuf *fragment = ZCB_WRAP((CharBuf*)field_val);
        CharBuf *raw_excerpt = CB_new(ivars->excerpt_length + 10);
        CharBuf *highlighted = CB_new((ivars->excerpt_length * 3) / 2);
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
        int32_t top
            = Highlighter_Find_Best_Fragment(self, (CharBuf*)field_val,
                                             (ViewCharBuf*)fragment, heat_map);
        VArray *sentences
            = Highlighter_Find_Sentences(self, (CharBuf*)field_val, 0,
                                         top + ivars->window_width);

        top = Highlighter_Raw_Excerpt(self, (CharBuf*)field_val,
                                      (CharBuf*)fragment, raw_excerpt, top,
                                      heat_map, sentences);
        Highlighter_Highlight_Excerpt(self, score_spans, raw_excerpt,
                                      highlighted, top);

        DECREF(sentences);
        DECREF(heat_map);
        DECREF(score_spans);
        DECREF(doc_vec);
        DECREF(raw_excerpt);

        return highlighted;
    }
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

int32_t
Highlighter_Find_Best_Fragment_IMP(Highlighter *self,
                                   const CharBuf *field_val,
                                   ViewCharBuf *fragment, HeatMap *heat_map) {
    HighlighterIVARS *const ivars = Highlighter_IVARS(self);

    // Window is 1.66 * excerpt_length, with the loc in the middle.
    int32_t best_location = S_hottest(heat_map);

    if (best_location < (int32_t)ivars->slop) {
        // If the beginning of the string falls within the window centered
        // around the hottest point in the field, start the fragment at the
        // beginning.
        ViewCB_Assign(fragment, (CharBuf*)field_val);
        int32_t top = ViewCB_Trim_Top(fragment);
        ViewCB_Truncate(fragment, ivars->window_width);
        return top;
    }
    else {
        int32_t top = best_location - ivars->slop;
        ViewCB_Assign(fragment, (CharBuf*)field_val);
        ViewCB_Nip(fragment, top);
        top += ViewCB_Trim_Top(fragment);
        int32_t chars_left = ViewCB_Truncate(fragment, ivars->excerpt_length);
        int32_t overrun = ivars->excerpt_length - chars_left;

        if (!overrun) {
            // We've found an acceptable window.
            ViewCB_Assign(fragment, (CharBuf*)field_val);
            ViewCB_Nip(fragment, top);
            top += ViewCB_Trim_Top(fragment);
            ViewCB_Truncate(fragment, ivars->window_width);
            return top;
        }
        else if (overrun > top) {
            // The field is very short, so make the whole field the
            // "fragment".
            ViewCB_Assign(fragment, (CharBuf*)field_val);
            return ViewCB_Trim_Top(fragment);
        }
        else {
            // The fragment is too close to the end, so slide it back.
            top -= overrun;
            ViewCB_Assign(fragment, (CharBuf*)field_val);
            ViewCB_Nip(fragment, top);
            top += ViewCB_Trim_Top(fragment);
            ViewCB_Truncate(fragment, ivars->excerpt_length);
            return top;
        }
    }
}

// Return true if the window represented by "offset" and "length" overlaps a
// score span, or if there are no score spans so that no excerpt is measurably
// superior.
static bool
S_has_heat(HeatMap *heat_map, int32_t offset, int32_t length) {
    VArray   *spans     = HeatMap_Get_Spans(heat_map);
    uint32_t  num_spans = VA_Get_Size(spans);
    int32_t   end       = offset + length;

    if (length == 0)    { return false; }
    if (num_spans == 0) { return true; }

    for (uint32_t i = 0; i < num_spans; i++) {
        Span *span  = (Span*)VA_Fetch(spans, i);
        int32_t span_start = Span_Get_Offset(span);
        int32_t span_end   = span_start + Span_Get_Length(span);;
        if (offset >= span_start && offset <  span_end) { return true; }
        if (end    >  span_start && end    <= span_end) { return true; }
        if (offset <= span_start && end    >= span_end) { return true; }
        if (span_start > end) { break; }
    }

    return false;
}

int32_t
Highlighter_Raw_Excerpt_IMP(Highlighter *self, const CharBuf *field_val,
                            const CharBuf *fragment, CharBuf *raw_excerpt,
                            int32_t top, HeatMap *heat_map,
                            VArray *sentences) {
    HighlighterIVARS *const ivars = Highlighter_IVARS(self);
    bool     found_starting_edge = false;
    bool     found_ending_edge   = false;
    int32_t  start = top;
    int32_t  end   = 0;
    double   field_len = CB_Length(field_val);
    uint32_t min_len = field_len < ivars->excerpt_length * 0.6666
                       ? (uint32_t)field_len
                       : (uint32_t)(ivars->excerpt_length * 0.6666);

    // Try to find a starting sentence boundary.
    const uint32_t num_sentences = VA_Get_Size(sentences);
    if (num_sentences) {
        for (uint32_t i = 0; i < num_sentences; i++) {
            Span *sentence = (Span*)VA_Fetch(sentences, i);
            int32_t candidate = Span_Get_Offset(sentence);;

            if (candidate > top + (int32_t)ivars->window_width) {
                break;
            }
            else if (candidate >= top) {
                // Try to start on the first sentence boundary, but only if
                // there's enough relevant material left after it in the
                // fragment.
                ZombieCharBuf *temp = ZCB_WRAP(fragment);
                ZCB_Nip(temp, candidate - top);
                uint32_t chars_left = ZCB_Truncate(temp, ivars->excerpt_length);
                if (chars_left >= min_len
                    && S_has_heat(heat_map, candidate, chars_left)
                   ) {
                    start = candidate;
                    found_starting_edge = true;
                    break;
                }
            }
        }
    }

    // Try to end on a sentence boundary (but don't try very hard).
    if (num_sentences) {
        ZombieCharBuf *start_trimmed = ZCB_WRAP(fragment);
        ZCB_Nip(start_trimmed, start - top);

        for (uint32_t i = num_sentences; i--;) {
            Span    *sentence  = (Span*)VA_Fetch(sentences, i);
            int32_t  last_edge = Span_Get_Offset(sentence)
                                 + Span_Get_Length(sentence);

            if (last_edge <= start) {
                break;
            }
            else if (last_edge - start > (int32_t)ivars->excerpt_length) {
                continue;
            }
            else {
                uint32_t chars_left = last_edge - start;
                if (chars_left > min_len
                    && S_has_heat(heat_map, start, chars_left)
                   ) {
                    found_ending_edge = true;
                    end = last_edge;
                    break;
                }
                else {
                    ZombieCharBuf *temp = ZCB_WRAP((CharBuf*)start_trimmed);
                    ZCB_Nip(temp, chars_left);
                    ZCB_Trim_Tail(temp);
                    if (ZCB_Get_Size(temp) == 0) {
                        // Short, but ending on a boundary already.
                        found_ending_edge = true;
                        end = last_edge;
                        break;
                    }
                }
            }
        }
    }
    int32_t this_excerpt_len = found_ending_edge
                               ? end - start
                               : (int32_t)ivars->excerpt_length;
    if (!this_excerpt_len) { return start; }

    ZombieCharBuf *substring = ZCB_WRAP((CharBuf*)field_val);

    if (found_starting_edge) {
        ZCB_Nip(substring, start);
        ZCB_Truncate(substring, this_excerpt_len);
    }
    // If not starting on a sentence boundary, prepend an ellipsis.
    else {
        const size_t ELLIPSIS_LEN = 2; // Unicode ellipsis plus a space.

        // If the excerpt is already shorter than the spec'd length, we might
        // not need to make room.
        this_excerpt_len += ELLIPSIS_LEN;

        // Remember original position
        int32_t orig_start = start;
        int32_t orig_len   = this_excerpt_len;

        // Move the start back one in case the character right before the
        // excerpt starts is whitespace.
        if (start) {
            this_excerpt_len += 1;
            start -= 1;
            ZCB_Nip(substring, start);
        }

        do {
            uint32_t code_point = ZCB_Nibble(substring);
            start++;
            this_excerpt_len--;

            if (StrHelp_is_whitespace(code_point)) {
                if (!found_ending_edge) {
                    // If we still need room, we'll lop it off the end since
                    // we don't know a solid end point yet.
                    break;
                }
                else if (this_excerpt_len <= (int32_t)ivars->excerpt_length) {
                    break;
                }
            }
        } while (ZCB_Get_Size(substring));

        if (ZCB_Get_Size(substring) == 0) {
            // Word is longer than excerpt_length. Reset to original position
            // truncating the word.
            ZCB_Assign(substring, (CharBuf*)field_val);
            start            = orig_start;
            this_excerpt_len = orig_len;
            int32_t diff = this_excerpt_len - ivars->excerpt_length;
            if (diff > 0) {
                ZCB_Nip(substring, diff);
                start            += diff;
                this_excerpt_len -= diff;
            }
        }

        ZCB_Truncate(substring, ivars->excerpt_length - ELLIPSIS_LEN);
    }

    // If excerpt doesn't end on a sentence boundary, tack on an ellipsis.
    if (found_ending_edge) {
        ZCB_Truncate(substring, end - start);
        ZCB_Trim_Tail(substring);
    }
    else {
        // Remember original excerpt
        ZombieCharBuf *orig_substring = ZCB_WRAP((CharBuf*)substring);
        // Check for prepended ellipsis
        uint32_t min_size = found_starting_edge ? 0 : 4;

        do {
            uint32_t code_point = ZCB_Code_Point_From(substring, 1);
            ZCB_Chop(substring, 1);
            if (StrHelp_is_whitespace(code_point)) {
                ZCB_Trim_Tail(substring);

                // Strip punctuation that collides with an ellipsis.
                code_point = ZCB_Code_Point_From(substring, 1);
                while (code_point == '.'
                       || code_point == ','
                       || code_point == ';'
                       || code_point == ':'
                       || code_point == ':'
                       || code_point == '?'
                       || code_point == '!'
                      ) {
                    ZCB_Chop(substring, 1);
                    code_point = ZCB_Code_Point_From(substring, 1);
                }

                break;
            }
        } while (ZCB_Get_Size(substring) > min_size);

        if (ZCB_Get_Size(substring) == min_size) {
            // Word is longer than excerpt_length. Reset to original excerpt
            // truncating the word.
            ZCB_Assign(substring, (CharBuf*)orig_substring);
            ZCB_Chop(substring, 1);
        }
    }

    if (!found_starting_edge) {
        CB_Cat_Char(raw_excerpt, ELLIPSIS_CODE_POINT);
        CB_Cat_Char(raw_excerpt, ' ');
        const size_t ELLIPSIS_LEN = 2; // Unicode ellipsis plus a space.
        start -= ELLIPSIS_LEN;
    }

    CB_Cat(raw_excerpt, (CharBuf*)substring);

    if (!found_ending_edge) {
        CB_Cat_Char(raw_excerpt, ELLIPSIS_CODE_POINT);
    }

    return start;
}

void
Highlighter_Highlight_Excerpt_IMP(Highlighter *self, VArray *spans,
                                  CharBuf *raw_excerpt, CharBuf *highlighted,
                                  int32_t top) {
    int32_t        hl_start        = 0;
    int32_t        hl_end          = 0;
    ZombieCharBuf *temp            = ZCB_WRAP(raw_excerpt);
    CharBuf       *encode_buf      = NULL;
    int32_t        raw_excerpt_end = top + CB_Length(raw_excerpt);

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
                CharBuf *encoded;

                if (hl_start < hl_end) {
                    // Highlight previous section
                    int32_t highlighted_len = hl_end - hl_start;
                    ZombieCharBuf *to_cat = ZCB_WRAP((CharBuf*)temp);
                    ZCB_Truncate(to_cat, highlighted_len);
                    encoded = S_do_encode(self, (CharBuf*)to_cat, &encode_buf);
                    CharBuf *hl_frag = Highlighter_Highlight(self, encoded);
                    CB_Cat(highlighted, hl_frag);
                    ZCB_Nip(temp, highlighted_len);
                    DECREF(hl_frag);
                    DECREF(encoded);
                }

                int32_t non_highlighted_len = relative_start - hl_end;
                ZombieCharBuf *to_cat = ZCB_WRAP((CharBuf*)temp);
                ZCB_Truncate(to_cat, non_highlighted_len);
                encoded = S_do_encode(self, (CharBuf*)to_cat, &encode_buf);
                CB_Cat(highlighted, (CharBuf*)encoded);
                ZCB_Nip(temp, non_highlighted_len);
                DECREF(encoded);

                hl_start = relative_start;
                hl_end   = relative_end;
            }
        }
    }

    if (hl_start < hl_end) {
        // Highlight final section
        int32_t highlighted_len = hl_end - hl_start;
        ZombieCharBuf *to_cat = ZCB_WRAP((CharBuf*)temp);
        ZCB_Truncate(to_cat, highlighted_len);
        CharBuf *encoded = S_do_encode(self, (CharBuf*)to_cat, &encode_buf);
        CharBuf *hl_frag = Highlighter_Highlight(self, encoded);
        CB_Cat(highlighted, hl_frag);
        ZCB_Nip(temp, highlighted_len);
        DECREF(hl_frag);
        DECREF(encoded);
    }

    // Last text, beyond last highlight span.
    if (ZCB_Get_Size(temp)) {
        CharBuf *encoded = S_do_encode(self, (CharBuf*)temp, &encode_buf);
        CB_Cat(highlighted, encoded);
        DECREF(encoded);
    }

    DECREF(encode_buf);
}

static Span*
S_start_sentence(int32_t pos) {
    return Span_new(pos, 0, 0.0);
}

static void
S_close_sentence(VArray *sentences, Span **sentence_ptr,
                 int32_t sentence_end) {
    Span *sentence = *sentence_ptr;
    int32_t length = sentence_end - Span_Get_Offset(sentence);
    const int32_t MIN_SENTENCE_LENGTH = 3; // e.g. "OK.", but not "2."
    if (length >= MIN_SENTENCE_LENGTH) {
        Span_Set_Length(sentence, length);
        VA_Push(sentences, (Obj*)sentence);
        *sentence_ptr = NULL;
    }
}

VArray*
Highlighter_Find_Sentences_IMP(Highlighter *self, CharBuf *text,
                               int32_t offset, int32_t length) {
    /* When [sentence] is NULL, that means a sentence start has not yet been
     * found.  When it is a Span object, we have a start, but we haven't found
     * an end.  Once we find the end, we add the sentence to the [sentences]
     * array and set [sentence] back to NULL to indicate that we're looking
     * for a start once more.
     */
    Span    *sentence       = NULL;
    VArray  *sentences      = VA_new(10);
    int32_t  stop           = length == 0
                              ? INT32_MAX
                              : offset + length;
    ZombieCharBuf *fragment = ZCB_WRAP(text);
    int32_t  pos            = ZCB_Trim_Top(fragment);
    UNUSED_VAR(self);

    /* Our first task will be to find a sentence that either starts at the top
     * of the fragment, or overlaps its start. Starting at the top of the
     * field is a special case: we define the first non-whitespace character
     * to begin a sentence, rather than look for the first character following
     * a period and whitespace.  Everywhere else, we have to define sentence
     * starts based on a sentence end that has just passed by.
     */
    if (offset <= pos) {
        // Assume that first non-whitespace character begins a sentence.
        if (pos < stop && ZCB_Get_Size(fragment) > 0) {
            sentence = S_start_sentence(pos);
        }
    }
    else {
        ZCB_Nip(fragment, offset - pos);
        pos = offset;
    }

    while (1) {
        uint32_t code_point = ZCB_Code_Point_At(fragment, 0);
        if (!code_point) {
            // End of fragment.  If we have a sentence open, close it,
            // then bail.
            if (sentence) { S_close_sentence(sentences, &sentence, pos); }
            break;
        }
        else if (code_point == '.') {
            uint32_t whitespace_count;
            pos += ZCB_Nip(fragment, 1); // advance past "."

            if (pos == stop && ZCB_Get_Size(fragment) == 0) {
                // Period ending the field string.
                if (sentence) { S_close_sentence(sentences, &sentence, pos); }
                break;
            }
            else if (0 != (whitespace_count = ZCB_Trim_Top(fragment))) {
                // We've found a period followed by whitespace.  Close out the
                // existing sentence, if there is one. */
                if (sentence) { S_close_sentence(sentences, &sentence, pos); }

                // Advance past whitespace.
                pos += whitespace_count;
                if (pos < stop && ZCB_Get_Size(fragment) > 0) {
                    // Not at the end of the string? Then we've found a
                    // sentence start.
                    sentence = S_start_sentence(pos);
                }
            }

            // We may not have reached the end of the field yet, but it's
            // entirely possible that our last sentence overlapped the end of
            // the fragment -- in which case, it's time to bail.
            if (pos >= stop) { break; }
        }
        else {
            ZCB_Nip(fragment, 1);
            pos++;
        }
    }

    return sentences;
}

CharBuf*
Highlighter_Encode_IMP(Highlighter *self, CharBuf *text) {
    CharBuf *encoded = CB_new(0);
    UNUSED_VAR(self);
    return S_encode_entities(text, encoded);
}

static CharBuf*
S_do_encode(Highlighter *self, CharBuf *text, CharBuf **encode_buf) {
    VTable *vtable = Highlighter_Get_VTable(self);
    Highlighter_Encode_t my_meth
        = (Highlighter_Encode_t)METHOD_PTR(vtable, LUCY_Highlighter_Encode);
    Highlighter_Encode_t orig_meth
        = (Highlighter_Encode_t)METHOD_PTR(HIGHLIGHTER, LUCY_Highlighter_Encode);

    if (my_meth != orig_meth) {
        return my_meth(self, text);
    }
    else {
        if (*encode_buf == NULL) { *encode_buf = CB_new(0); }
        (void)S_encode_entities(text, *encode_buf);
        return (CharBuf*)INCREF(*encode_buf);
    }
}

static CharBuf*
S_encode_entities(CharBuf *text, CharBuf *encoded) {
    ZombieCharBuf *temp = ZCB_WRAP(text);
    size_t space = 0;
    const int MAX_ENTITY_BYTES = 9; // &#dddddd;

    // Scan first so that we only allocate once.
    uint32_t code_point;
    while (0 != (code_point = ZCB_Nibble(temp))) {
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

    CB_Grow(encoded, space);
    CB_Set_Size(encoded, 0);
    ZCB_Assign(temp, text);
    while (0 != (code_point = ZCB_Nibble(temp))) {
        if (code_point > 127
            || (!isgraph(code_point) && !isspace(code_point))
           ) {
            CB_catf(encoded, "&#%u32;", code_point);
        }
        else if (code_point == '<') {
            CB_Cat_Trusted_Str(encoded, "&lt;", 4);
        }
        else if (code_point == '>') {
            CB_Cat_Trusted_Str(encoded, "&gt;", 4);
        }
        else if (code_point == '&') {
            CB_Cat_Trusted_Str(encoded, "&amp;", 5);
        }
        else if (code_point == '"') {
            CB_Cat_Trusted_Str(encoded, "&quot;", 6);
        }
        else {
            CB_Cat_Char(encoded, code_point);
        }
    }

    return encoded;
}



