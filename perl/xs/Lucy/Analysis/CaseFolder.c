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

#define C_LUCY_CASEFOLDER
#define C_LUCY_BYTEBUF
#define C_LUCY_TOKEN
#include "XSBind.h"

#include "Lucy/Analysis/CaseFolder.h"
#include "Lucy/Analysis/Token.h"
#include "Lucy/Analysis/Inversion.h"
#include "Lucy/Object/ByteBuf.h"
#include "Lucy/Util/Memory.h"
#include "Lucy/Util/StringHelper.h"

static size_t
S_lc_to_work_buf(lucy_CaseFolder *self, uint8_t *source, size_t len,
                 uint8_t **buf, uint8_t **limit) {
    lucy_ByteBuf *const work_buf   = self->work_buf;
    uint8_t            *dest       = *buf;
    uint8_t            *dest_start = dest;
    uint8_t *const      end        = source + len;
    uint8_t             utf8_buf[7];
    dTHX;

    while (source < end) {
        STRLEN buf_utf8_len;
        #if (PERL_VERSION == 15 && PERL_SUBVERSION >= 6)
        Perl__to_utf8_lower_flags(aTHX_ source, utf8_buf, &buf_utf8_len,
                                  0, NULL);
        #else
        Perl_to_utf8_lower(aTHX_ source, utf8_buf, &buf_utf8_len);
        #endif

        // Grow if necessary.
        if (((STRLEN)(*limit - dest)) < buf_utf8_len) {
            size_t    bytes_so_far = dest - dest_start;
            size_t    amount       = bytes_so_far + (end - source) + 10;
            Lucy_BB_Set_Size(work_buf, bytes_so_far);
            *buf       = (uint8_t*)Lucy_BB_Grow(work_buf, amount);
            dest_start = *buf;
            dest       = dest_start + bytes_so_far;
            *limit     = dest_start + work_buf->cap;
        }
        memcpy(dest, utf8_buf, buf_utf8_len);

        source += lucy_StrHelp_UTF8_COUNT[*source];
        dest += buf_utf8_len;
    }

    size_t size = dest - dest_start;
    Lucy_BB_Set_Size(work_buf, size);
    return size;
}

lucy_Inversion*
lucy_CaseFolder_transform(lucy_CaseFolder *self, lucy_Inversion *inversion) {
    lucy_Token *token;
    uint8_t *buf   = (uint8_t*)Lucy_BB_Get_Buf(self->work_buf);
    uint8_t *limit = buf + Lucy_BB_Get_Capacity(self->work_buf);
    while (NULL != (token = Lucy_Inversion_Next(inversion))) {
        size_t size = S_lc_to_work_buf(self, (uint8_t*)token->text,
                                       token->len, &buf, &limit);
        if (size > token->len) {
            LUCY_FREEMEM(token->text);
            token->text = (char*)LUCY_MALLOCATE(size + 1);
        }
        memcpy(token->text, buf, size);
        token->text[size] = '\0';
        token->len = size;
    }
    Lucy_Inversion_Reset(inversion);
    return (lucy_Inversion*)CFISH_INCREF(inversion);
}

lucy_Inversion*
lucy_CaseFolder_transform_text(lucy_CaseFolder *self, lucy_CharBuf *text) {
    lucy_Inversion *retval;
    lucy_Token *token;
    uint8_t *buf   = (uint8_t*)Lucy_BB_Get_Buf(self->work_buf);
    uint8_t *limit = buf + Lucy_BB_Get_Capacity(self->work_buf);
    size_t size = S_lc_to_work_buf(self, Lucy_CB_Get_Ptr8(text),
                                   Lucy_CB_Get_Size(text), &buf, &limit);
    token = lucy_Token_new((char*)buf, size, 0, size, 1.0f, 1);
    retval = lucy_Inversion_new(token);
    CFISH_DECREF(token);
    return retval;
}


