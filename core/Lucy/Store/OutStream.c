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

#define C_LUCY_OUTSTREAM
#include "Lucy/Util/ToolSet.h"

#include "charmony.h"

#include "Lucy/Store/OutStream.h"
#include "Lucy/Store/FileHandle.h"
#include "Lucy/Store/FSFileHandle.h"
#include "Lucy/Store/FileWindow.h"
#include "Lucy/Store/InStream.h"
#include "Lucy/Store/RAMFile.h"
#include "Lucy/Store/RAMFileHandle.h"

// Inlined version of OutStream_Write_Bytes.
static CFISH_INLINE void
SI_write_bytes(OutStream *self, OutStreamIVARS *ivars,
               const void *bytes, size_t len);

// Inlined version of OutStream_Write_C32.
static CFISH_INLINE void
SI_write_c32(OutStream *self, OutStreamIVARS *ivars, uint32_t value);

// Flush content in the buffer to the FileHandle.
static void
S_flush(OutStream *self, OutStreamIVARS *ivars);

OutStream*
OutStream_open(Obj *file) {
    OutStream *self = (OutStream*)Class_Make_Obj(OUTSTREAM);
    return OutStream_do_open(self, file);
}

OutStream*
OutStream_do_open(OutStream *self, Obj *file) {
    OutStreamIVARS *const ivars = OutStream_IVARS(self);

    // Init.
    ivars->buf         = (char*)MALLOCATE(IO_STREAM_BUF_SIZE);
    ivars->buf_start   = 0;
    ivars->buf_pos     = 0;

    // Obtain a FileHandle.
    if (Obj_Is_A(file, FILEHANDLE)) {
        ivars->file_handle = (FileHandle*)INCREF(file);
    }
    else if (Obj_Is_A(file, RAMFILE)) {
        ivars->file_handle
            = (FileHandle*)RAMFH_open(NULL, FH_WRITE_ONLY, (RAMFile*)file);
    }
    else if (Obj_Is_A(file, STRING)) {
        ivars->file_handle = (FileHandle*)FSFH_open((String*)file,
                                                    FH_WRITE_ONLY | FH_CREATE | FH_EXCLUSIVE);
    }
    else {
        Err_set_error(Err_new(Str_newf("Invalid type for param 'file': '%o'",
                                       Obj_Get_Class_Name(file))));
        DECREF(self);
        return NULL;
    }
    if (!ivars->file_handle) {
        ERR_ADD_FRAME(Err_get_error());
        DECREF(self);
        return NULL;
    }

    // Derive filepath from FileHandle.
    ivars->path = Str_Clone(FH_Get_Path(ivars->file_handle));

    return self;
}

void
OutStream_Destroy_IMP(OutStream *self) {
    OutStreamIVARS *const ivars = OutStream_IVARS(self);
    if (ivars->file_handle != NULL) {
        // Inlined flush, ignoring errors.
        if (ivars->buf_pos) {
            FH_Write(ivars->file_handle, ivars->buf, ivars->buf_pos);
        }
        DECREF(ivars->file_handle);
    }
    DECREF(ivars->path);
    FREEMEM(ivars->buf);
    SUPER_DESTROY(self, OUTSTREAM);
}

String*
OutStream_Get_Path_IMP(OutStream *self) {
    return OutStream_IVARS(self)->path;
}

void
OutStream_Absorb_IMP(OutStream *self, InStream *instream) {
    OutStreamIVARS *const ivars = OutStream_IVARS(self);
    char buf[IO_STREAM_BUF_SIZE];
    int64_t bytes_left = InStream_Length(instream);

    // Read blocks of content into an intermediate buffer, than write them to
    // the OutStream.
    //
    // TODO: optimize by utilizing OutStream's buffer directly, while still
    // not flushing too frequently and keeping code complexity under control.
    OutStream_Grow(self, OutStream_Tell(self) + bytes_left);
    while (bytes_left) {
        const size_t bytes_this_iter = bytes_left < IO_STREAM_BUF_SIZE
                                       ? (size_t)bytes_left
                                       : IO_STREAM_BUF_SIZE;
        InStream_Read_Bytes(instream, buf, bytes_this_iter);
        SI_write_bytes(self, ivars, buf, bytes_this_iter);
        bytes_left -= bytes_this_iter;
    }
}

void
OutStream_Grow_IMP(OutStream *self, int64_t length) {
    OutStreamIVARS *const ivars = OutStream_IVARS(self);
    if (!FH_Grow(ivars->file_handle, length)) {
        RETHROW(INCREF(Err_get_error()));
    }
}

int64_t
OutStream_Tell_IMP(OutStream *self) {
    OutStreamIVARS *const ivars = OutStream_IVARS(self);
    return ivars->buf_start + ivars->buf_pos;
}

int64_t
OutStream_Align_IMP(OutStream *self, int64_t modulus) {
    int64_t len = OutStream_Tell(self);
    int64_t filler_bytes = (modulus - (len % modulus)) % modulus;
    while (filler_bytes--) { OutStream_Write_U8(self, 0); }
    return OutStream_Tell(self);
}

void
OutStream_Flush_IMP(OutStream *self) {
    OutStreamIVARS *const ivars = OutStream_IVARS(self);
    S_flush(self, ivars);
}

static void
S_flush(OutStream *self, OutStreamIVARS *ivars) {
    UNUSED_VAR(self);
    if (ivars->file_handle == NULL) {
        THROW(ERR, "Can't write to a closed OutStream for %o", ivars->path);
    }
    if (!FH_Write(ivars->file_handle, ivars->buf, ivars->buf_pos)) {
        RETHROW(INCREF(Err_get_error()));
    }
    ivars->buf_start += ivars->buf_pos;
    ivars->buf_pos = 0;
}

int64_t
OutStream_Length_IMP(OutStream *self) {
    return OutStream_Tell_IMP(self);
}

void
OutStream_Write_Bytes_IMP(OutStream *self, const void *bytes, size_t len) {
    SI_write_bytes(self, OutStream_IVARS(self), bytes, len);
}

static CFISH_INLINE void
SI_write_bytes(OutStream *self, OutStreamIVARS *ivars,
               const void *bytes, size_t len) {
    // If this data is larger than the buffer size, flush and write.
    if (len >= IO_STREAM_BUF_SIZE) {
        S_flush(self, ivars);
        if (!FH_Write(ivars->file_handle, bytes, len)) {
            RETHROW(INCREF(Err_get_error()));
        }
        ivars->buf_start += len;
    }
    // If there's not enough room in the buffer, flush then add.
    else if (ivars->buf_pos + len >= IO_STREAM_BUF_SIZE) {
        S_flush(self, ivars);
        memcpy((ivars->buf + ivars->buf_pos), bytes, len);
        ivars->buf_pos += len;
    }
    // If there's room, just add these bytes to the buffer.
    else {
        memcpy((ivars->buf + ivars->buf_pos), bytes, len);
        ivars->buf_pos += len;
    }
}

static CFISH_INLINE void
SI_write_u8(OutStream *self, OutStreamIVARS *ivars, uint8_t value) {
    if (ivars->buf_pos >= IO_STREAM_BUF_SIZE) {
        S_flush(self, ivars);
    }
    ivars->buf[ivars->buf_pos++] = (char)value;
}

void
OutStream_Write_I8_IMP(OutStream *self, int8_t value) {
    OutStreamIVARS *const ivars = OutStream_IVARS(self);
    SI_write_u8(self, ivars, (uint8_t)value);
}

void
OutStream_Write_U8_IMP(OutStream *self, uint8_t value) {
    OutStreamIVARS *const ivars = OutStream_IVARS(self);
    SI_write_u8(self, ivars, value);
}

static CFISH_INLINE void
SI_write_u32(OutStream *self, OutStreamIVARS *ivars, uint32_t value) {
#ifdef CHY_BIG_END
    SI_write_bytes(self, ivars, &value, 4);
#else
    char  buf[4];
    char *buf_copy = buf;
    NumUtil_encode_bigend_u32(value, &buf_copy);
    SI_write_bytes(self, ivars, buf, 4);
#endif
}

void
OutStream_Write_I32_IMP(OutStream *self, int32_t value) {
    SI_write_u32(self, OutStream_IVARS(self), (uint32_t)value);
}

void
OutStream_Write_U32_IMP(OutStream *self, uint32_t value) {
    SI_write_u32(self, OutStream_IVARS(self), value);
}

static CFISH_INLINE void
SI_write_u64(OutStream *self, OutStreamIVARS *ivars, uint64_t value) {
#ifdef CHY_BIG_END
    SI_write_bytes(self, ivars, &value, 8);
#else
    char  buf[sizeof(uint64_t)];
    char *buf_copy = buf;
    NumUtil_encode_bigend_u64(value, &buf_copy);
    SI_write_bytes(self, ivars, buf, sizeof(uint64_t));
#endif
}

void
OutStream_Write_I64_IMP(OutStream *self, int64_t value) {
    SI_write_u64(self, OutStream_IVARS(self), (uint64_t)value);
}

void
OutStream_Write_U64_IMP(OutStream *self, uint64_t value) {
    SI_write_u64(self, OutStream_IVARS(self), value);
}

void
OutStream_Write_F32_IMP(OutStream *self, float value) {
    OutStreamIVARS *const ivars = OutStream_IVARS(self);
    char  buf[sizeof(float)];
    char *buf_copy = buf;
    NumUtil_encode_bigend_f32(value, &buf_copy);
    SI_write_bytes(self, ivars, buf_copy, sizeof(float));
}

void
OutStream_Write_F64_IMP(OutStream *self, double value) {
    OutStreamIVARS *const ivars = OutStream_IVARS(self);
    char  buf[sizeof(double)];
    char *buf_copy = buf;
    NumUtil_encode_bigend_f64(value, &buf_copy);
    SI_write_bytes(self, ivars, buf_copy, sizeof(double));
}

void
OutStream_Write_C32_IMP(OutStream *self, uint32_t value) {
    SI_write_c32(self, OutStream_IVARS(self), value);
}

static CFISH_INLINE void
SI_write_c32(OutStream *self, OutStreamIVARS *ivars, uint32_t value) {
    uint8_t buf[C32_MAX_BYTES];
    uint8_t *ptr = buf + sizeof(buf) - 1;

    // Write last byte first, which has no continue bit.
    *ptr = value & 0x7f;
    value >>= 7;

    while (value) {
        // Work backwards, writing bytes with continue bits set.
        *--ptr = ((value & 0x7f) | 0x80);
        value >>= 7;
    }

    SI_write_bytes(self, ivars, ptr, (buf + sizeof(buf)) - ptr);
}

void
OutStream_Write_C64_IMP(OutStream *self, uint64_t value) {
    OutStreamIVARS *const ivars = OutStream_IVARS(self);
    uint8_t buf[C64_MAX_BYTES];
    uint8_t *ptr = buf + sizeof(buf) - 1;

    // Write last byte first, which has no continue bit.
    *ptr = value & 0x7f;
    value >>= 7;

    while (value) {
        // Work backwards, writing bytes with continue bits set.
        *--ptr = ((value & 0x7f) | 0x80);
        value >>= 7;
    }

    SI_write_bytes(self, ivars, ptr, (buf + sizeof(buf)) - ptr);
}

void
OutStream_Write_String_IMP(OutStream *self, const char *string, size_t len) {
    OutStreamIVARS *const ivars = OutStream_IVARS(self);
    SI_write_c32(self, ivars, (uint32_t)len);
    SI_write_bytes(self, ivars, string, len);
}

void
OutStream_Close_IMP(OutStream *self) {
    OutStreamIVARS *const ivars = OutStream_IVARS(self);
    if (ivars->file_handle) {
        S_flush(self, ivars);
        if (!FH_Close(ivars->file_handle)) {
            RETHROW(INCREF(Err_get_error()));
        }
        DECREF(ivars->file_handle);
        ivars->file_handle = NULL;
    }
}


