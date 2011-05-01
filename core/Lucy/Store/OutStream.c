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

#include "Lucy/Store/OutStream.h"
#include "Lucy/Store/FileHandle.h"
#include "Lucy/Store/FSFileHandle.h"
#include "Lucy/Store/FileWindow.h"
#include "Lucy/Store/InStream.h"
#include "Lucy/Store/RAMFile.h"
#include "Lucy/Store/RAMFileHandle.h"

// Inlined version of OutStream_Write_Bytes.
static INLINE void
SI_write_bytes(OutStream *self, const void *bytes, size_t len);

// Inlined version of OutStream_Write_C32.
static INLINE void
SI_write_c32(OutStream *self, uint32_t value);

// Flush content in the buffer to the FileHandle.
static void
S_flush(OutStream *self);

OutStream*
OutStream_open(Obj *file) {
    OutStream *self = (OutStream*)VTable_Make_Obj(OUTSTREAM);
    return OutStream_do_open(self, file);
}

OutStream*
OutStream_do_open(OutStream *self, Obj *file) {
    // Init.
    self->buf         = (char*)MALLOCATE(IO_STREAM_BUF_SIZE);
    self->buf_start   = 0;
    self->buf_pos     = 0;

    // Obtain a FileHandle.
    if (Obj_Is_A(file, FILEHANDLE)) {
        self->file_handle = (FileHandle*)INCREF(file);
    }
    else if (Obj_Is_A(file, RAMFILE)) {
        self->file_handle
            = (FileHandle*)RAMFH_open(NULL, FH_WRITE_ONLY, (RAMFile*)file);
    }
    else if (Obj_Is_A(file, CHARBUF)) {
        self->file_handle = (FileHandle*)FSFH_open((CharBuf*)file,
                                                   FH_WRITE_ONLY | FH_CREATE | FH_EXCLUSIVE);
    }
    else {
        Err_set_error(Err_new(CB_newf("Invalid type for param 'file': '%o'",
                                      Obj_Get_Class_Name(file))));
        DECREF(self);
        return NULL;
    }
    if (!self->file_handle) {
        ERR_ADD_FRAME(Err_get_error());
        DECREF(self);
        return NULL;
    }

    // Derive filepath from FileHandle.
    self->path = CB_Clone(FH_Get_Path(self->file_handle));

    return self;
}

void
OutStream_destroy(OutStream *self) {
    if (self->file_handle != NULL) {
        // Inlined flush, ignoring errors.
        if (self->buf_pos) {
            FH_Write(self->file_handle, self->buf, self->buf_pos);
        }
        DECREF(self->file_handle);
    }
    DECREF(self->path);
    FREEMEM(self->buf);
    SUPER_DESTROY(self, OUTSTREAM);
}

CharBuf*
OutStream_get_path(OutStream *self) {
    return self->path;
}

void
OutStream_absorb(OutStream *self, InStream *instream) {
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
        SI_write_bytes(self, buf, bytes_this_iter);
        bytes_left -= bytes_this_iter;
    }
}

void
OutStream_grow(OutStream *self, int64_t length) {
    if (!FH_Grow(self->file_handle, length)) {
        RETHROW(INCREF(Err_get_error()));
    }
}

int64_t
OutStream_tell(OutStream *self) {
    return self->buf_start + self->buf_pos;
}

int64_t
OutStream_align(OutStream *self, int64_t modulus) {
    int64_t len = OutStream_Tell(self);
    int64_t filler_bytes = (modulus - (len % modulus)) % modulus;
    while (filler_bytes--) { OutStream_Write_U8(self, 0); }
    return OutStream_Tell(self);
}

void
OutStream_flush(OutStream *self) {
    S_flush(self);
}

static void
S_flush(OutStream *self) {
    if (self->file_handle == NULL) {
        THROW(ERR, "Can't write to a closed OutStream for %o", self->path);
    }
    if (!FH_Write(self->file_handle, self->buf, self->buf_pos)) {
        RETHROW(INCREF(Err_get_error()));
    }
    self->buf_start += self->buf_pos;
    self->buf_pos = 0;
}

int64_t
OutStream_length(OutStream *self) {
    return OutStream_tell(self);
}

void
OutStream_write_bytes(OutStream *self, const void *bytes, size_t len) {
    SI_write_bytes(self, bytes, len);
}

static INLINE void
SI_write_bytes(OutStream *self, const void *bytes, size_t len) {
    // If this data is larger than the buffer size, flush and write.
    if (len >= IO_STREAM_BUF_SIZE) {
        S_flush(self);
        if (!FH_Write(self->file_handle, bytes, len)) {
            RETHROW(INCREF(Err_get_error()));
        }
        self->buf_start += len;
    }
    // If there's not enough room in the buffer, flush then add.
    else if (self->buf_pos + len >= IO_STREAM_BUF_SIZE) {
        S_flush(self);
        memcpy((self->buf + self->buf_pos), bytes, len);
        self->buf_pos += len;
    }
    // If there's room, just add these bytes to the buffer.
    else {
        memcpy((self->buf + self->buf_pos), bytes, len);
        self->buf_pos += len;
    }
}

static INLINE void
SI_write_u8(OutStream *self, uint8_t value) {
    if (self->buf_pos >= IO_STREAM_BUF_SIZE) {
        S_flush(self);
    }
    self->buf[self->buf_pos++] = (char)value;
}

void
OutStream_write_i8(OutStream *self, int8_t value) {
    SI_write_u8(self, (uint8_t)value);
}

void
OutStream_write_u8(OutStream *self, uint8_t value) {
    SI_write_u8(self, value);
}

static INLINE void
SI_write_u32(OutStream *self, uint32_t value) {
#ifdef BIG_END
    SI_write_bytes(self, &value, 4);
#else
    char  buf[4];
    char *buf_copy = buf;
    NumUtil_encode_bigend_u32(value, &buf_copy);
    SI_write_bytes(self, buf, 4);
#endif
}

void
OutStream_write_i32(OutStream *self, int32_t value) {
    SI_write_u32(self, (uint32_t)value);
}

void
OutStream_write_u32(OutStream *self, uint32_t value) {
    SI_write_u32(self, value);
}

static INLINE void
SI_write_u64(OutStream *self, uint64_t value) {
#ifdef BIG_END
    SI_write_bytes(self, &value, 8);
#else
    char  buf[sizeof(uint64_t)];
    char *buf_copy = buf;
    NumUtil_encode_bigend_u64(value, &buf_copy);
    SI_write_bytes(self, buf, sizeof(uint64_t));
#endif
}

void
OutStream_write_i64(OutStream *self, int64_t value) {
    SI_write_u64(self, (uint64_t)value);
}

void
OutStream_write_u64(OutStream *self, uint64_t value) {
    SI_write_u64(self, value);
}

void
OutStream_write_f32(OutStream *self, float value) {
    char  buf[sizeof(float)];
    char *buf_copy = buf;
    NumUtil_encode_bigend_f32(value, &buf_copy);
    SI_write_bytes(self, buf_copy, sizeof(float));
}

void
OutStream_write_f64(OutStream *self, double value) {
    char  buf[sizeof(double)];
    char *buf_copy = buf;
    NumUtil_encode_bigend_f64(value, &buf_copy);
    SI_write_bytes(self, buf_copy, sizeof(double));
}

void
OutStream_write_c32(OutStream *self, uint32_t value) {
    SI_write_c32(self, value);
}

static INLINE void
SI_write_c32(OutStream *self, uint32_t value) {
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

    SI_write_bytes(self, ptr, (buf + sizeof(buf)) - ptr);
}

void
OutStream_write_c64(OutStream *self, uint64_t value) {
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

    SI_write_bytes(self, ptr, (buf + sizeof(buf)) - ptr);
}

void
OutStream_write_string(OutStream *self, const char *string, size_t len) {
    SI_write_c32(self, (uint32_t)len);
    SI_write_bytes(self, string, len);
}

void
OutStream_close(OutStream *self) {
    if (self->file_handle) {
        S_flush(self);
        if (!FH_Close(self->file_handle)) {
            RETHROW(INCREF(Err_get_error()));
        }
        DECREF(self->file_handle);
        self->file_handle = NULL;
    }
}


