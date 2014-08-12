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

#define C_LUCY_INSTREAM
#include "Lucy/Util/ToolSet.h"

#include "charmony.h"

#include "Lucy/Store/InStream.h"
#include "Lucy/Store/FileHandle.h"
#include "Lucy/Store/FSFileHandle.h"
#include "Lucy/Store/FileWindow.h"
#include "Lucy/Store/RAMFile.h"
#include "Lucy/Store/RAMFileHandle.h"

// Inlined version of InStream_Tell.
static CFISH_INLINE int64_t
SI_tell(InStream *self);

// Inlined version of InStream_Read_Bytes.
static CFISH_INLINE void
SI_read_bytes(InStream *self, char* buf, size_t len);

// Inlined version of InStream_Read_U8.
static CFISH_INLINE uint8_t
SI_read_u8(InStream *self, InStreamIVARS *const ivars);

// Ensure that the buffer contains exactly the specified amount of data.
static void
S_fill(InStream *self, int64_t amount);

// Refill the buffer, with either IO_STREAM_BUF_SIZE bytes or all remaining
// file content -- whichever is smaller. Throw an error if we're at EOF and
// can't load at least one byte.
static int64_t
S_refill(InStream *self);

InStream*
InStream_open(Obj *file) {
    InStream *self = (InStream*)Class_Make_Obj(INSTREAM);
    return InStream_do_open(self, file);
}

InStream*
InStream_do_open(InStream *self, Obj *file) {
    InStreamIVARS *const ivars = InStream_IVARS(self);

    // Init.
    ivars->buf          = NULL;
    ivars->limit        = NULL;
    ivars->offset       = 0;
    ivars->window       = FileWindow_new();

    // Obtain a FileHandle.
    if (Obj_Is_A(file, FILEHANDLE)) {
        ivars->file_handle = (FileHandle*)INCREF(file);
    }
    else if (Obj_Is_A(file, RAMFILE)) {
        ivars->file_handle
            = (FileHandle*)RAMFH_open(NULL, FH_READ_ONLY, (RAMFile*)file);
    }
    else if (Obj_Is_A(file, STRING)) {
        ivars->file_handle
            = (FileHandle*)FSFH_open((String*)file, FH_READ_ONLY);
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

    // Get length and filename from the FileHandle.
    ivars->filename     = Str_Clone(FH_Get_Path(ivars->file_handle));
    ivars->len          = FH_Length(ivars->file_handle);
    if (ivars->len == -1) {
        ERR_ADD_FRAME(Err_get_error());
        DECREF(self);
        return NULL;
    }

    return self;
}

void
InStream_Close_IMP(InStream *self) {
    InStreamIVARS *const ivars = InStream_IVARS(self);
    if (ivars->file_handle) {
        FH_Release_Window(ivars->file_handle, ivars->window);
        // Note that we don't close the FileHandle, because it's probably
        // shared.
        DECREF(ivars->file_handle);
        ivars->file_handle = NULL;
    }
}

void
InStream_Destroy_IMP(InStream *self) {
    InStreamIVARS *const ivars = InStream_IVARS(self);
    if (ivars->file_handle) {
        InStream_Close(self);
    }
    DECREF(ivars->filename);
    DECREF(ivars->window);
    SUPER_DESTROY(self, INSTREAM);
}

InStream*
InStream_Reopen_IMP(InStream *self, String *filename, int64_t offset,
                    int64_t len) {
    InStreamIVARS *const ivars = InStream_IVARS(self);
    if (!ivars->file_handle) {
        THROW(ERR, "Can't Reopen() closed InStream %o", ivars->filename);
    }
    if (offset + len > FH_Length(ivars->file_handle)) {
        THROW(ERR, "Offset + length too large (%i64 + %i64 > %i64)",
              offset, len, FH_Length(ivars->file_handle));
    }

    Class *klass = InStream_Get_Class(self);
    InStream *other = (InStream*)Class_Make_Obj(klass);
    InStreamIVARS *const ovars = InStream_IVARS(other);
    InStream_do_open(other, (Obj*)ivars->file_handle);
    if (filename != NULL) {
        DECREF(ovars->filename);
        ovars->filename = Str_Clone(filename);
    }
    ovars->offset = offset;
    ovars->len    = len;
    InStream_Seek(other, 0);

    return other;
}

InStream*
InStream_Clone_IMP(InStream *self) {
    InStreamIVARS *const ivars = InStream_IVARS(self);
    Class *klass = InStream_Get_Class(self);
    InStream *twin = (InStream*)Class_Make_Obj(klass);
    InStream_do_open(twin, (Obj*)ivars->file_handle);
    InStream_Seek(twin, SI_tell(self));
    return twin;
}

String*
InStream_Get_Filename_IMP(InStream *self) {
    return InStream_IVARS(self)->filename;
}

static int64_t
S_refill(InStream *self) {
    InStreamIVARS *const ivars = InStream_IVARS(self);

    // Determine the amount to request.
    const int64_t sub_file_pos = SI_tell(self);
    const int64_t remaining    = ivars->len - sub_file_pos;
    const int64_t amount       = remaining < IO_STREAM_BUF_SIZE
                                 ? remaining
                                 : IO_STREAM_BUF_SIZE;
    if (!remaining) {
        THROW(ERR, "Read past EOF of '%o' (offset: %i64 len: %i64)",
              ivars->filename, ivars->offset, ivars->len);
    }

    // Make the request.
    S_fill(self, amount);

    return amount;
}

void
InStream_Refill_IMP(InStream *self) {
    S_refill(self);
}

static void
S_fill(InStream *self, int64_t amount) {
    InStreamIVARS *const ivars     = InStream_IVARS(self);
    FileWindow *const window       = ivars->window;
    const int64_t virtual_file_pos = SI_tell(self);
    const int64_t real_file_pos    = virtual_file_pos + ivars->offset;
    const int64_t remaining        = ivars->len - virtual_file_pos;

    // Throw an error if the requested amount would take us beyond EOF.
    if (amount > remaining) {
        THROW(ERR,  "Read past EOF of %o (pos: %u64 len: %u64 request: %u64)",
              ivars->filename, virtual_file_pos, ivars->len, amount);
    }

    // Make the request.
    if (FH_Window(ivars->file_handle, window, real_file_pos, amount)) {
        char    *fw_buf    = FileWindow_Get_Buf(window);
        int64_t  fw_offset = FileWindow_Get_Offset(window);
        int64_t  fw_len    = FileWindow_Get_Len(window);
        char *const window_limit = fw_buf + fw_len;
        ivars->buf = fw_buf
                     - fw_offset          // theoretical start of real file
                     + ivars->offset      // top of virtual file
                     + virtual_file_pos;  // position within virtual file
        ivars->limit = window_limit - ivars->buf > remaining
                       ? ivars->buf + remaining
                       : window_limit;
    }
    else {
        Err *error = Err_get_error();
        String *str = Str_newf(" (%o)", ivars->filename);
        Err_Cat_Mess(error, str);
        DECREF(str);
        RETHROW(INCREF(error));
    }
}

void
InStream_Fill_IMP(InStream *self, int64_t amount) {
    S_fill(self, amount);
}

void
InStream_Seek_IMP(InStream *self, int64_t target) {
    InStreamIVARS *const ivars = InStream_IVARS(self);
    FileWindow *const window = ivars->window;
    char    *fw_buf    = FileWindow_Get_Buf(window);
    int64_t  fw_offset = FileWindow_Get_Offset(window);
    int64_t  fw_len    = FileWindow_Get_Len(window);
    int64_t  virtual_window_top = fw_offset - ivars->offset;
    int64_t  virtual_window_end = virtual_window_top + fw_len;

    if (target < 0) {
        THROW(ERR, "Can't Seek '%o' to negative target %i64", ivars->filename,
              target);
    }
    // Seek within window if possible.
    else if (target >= virtual_window_top
             && target <= virtual_window_end
            ) {
        ivars->buf = fw_buf - fw_offset + ivars->offset + target;
    }
    else if (target > ivars->len) {
        THROW(ERR, "Can't Seek '%o' past EOF (%i64 > %i64)", ivars->filename,
              target, ivars->len);
    }
    else {
        // Target is outside window.  Set all buffer and limit variables to
        // NULL to trigger refill on the next read.  Store the file position
        // in the FileWindow's offset.
        FH_Release_Window(ivars->file_handle, window);
        ivars->buf   = NULL;
        ivars->limit = NULL;
        FileWindow_Set_Offset(window, ivars->offset + target);
    }
}

static CFISH_INLINE int64_t
SI_tell(InStream *self) {
    InStreamIVARS *const ivars = InStream_IVARS(self);
    char *fw_buf = FileWindow_Get_Buf(ivars->window);
    int64_t pos_in_buf = CHY_PTR_TO_I64(ivars->buf) - CHY_PTR_TO_I64(fw_buf);
    return pos_in_buf + FileWindow_Get_Offset(ivars->window) - ivars->offset;
}

int64_t
InStream_Tell_IMP(InStream *self) {
    return SI_tell(self);
}

int64_t
InStream_Length_IMP(InStream *self) {
    return InStream_IVARS(self)->len;
}

const char*
InStream_Buf_IMP(InStream *self, size_t request) {
    InStreamIVARS *const ivars = InStream_IVARS(self);
    const int64_t bytes_in_buf
        = CHY_PTR_TO_I64(ivars->limit) - CHY_PTR_TO_I64(ivars->buf);

    /* It's common for client code to overestimate how much is needed, because
     * the request has to figure in worst-case for compressed data.  However,
     * if we can still serve them everything they request (e.g. they ask for 5
     * bytes, they really need 1 byte, and there's 1k in the buffer), we can
     * skip the following refill block. */
    if ((int64_t)request > bytes_in_buf) {
        const int64_t remaining_in_file = ivars->len - SI_tell(self);
        int64_t amount = request;

        // Try to bump up small requests.
        if (amount < IO_STREAM_BUF_SIZE) { amount = IO_STREAM_BUF_SIZE; }

        // Don't read past EOF.
        if (remaining_in_file < amount) { amount = remaining_in_file; }

        // Only fill if the recalculated, possibly smaller request exceeds the
        // amount available in the buffer.
        if (amount > bytes_in_buf) {
            S_fill(self, amount);
        }
    }

    return ivars->buf;
}

void
InStream_Advance_Buf_IMP(InStream *self, const char *buf) {
    InStreamIVARS *const ivars = InStream_IVARS(self);
    if (buf > ivars->limit) {
        int64_t overrun = CHY_PTR_TO_I64(buf) - CHY_PTR_TO_I64(ivars->limit);
        THROW(ERR, "Supplied value is %i64 bytes beyond end of buffer",
              overrun);
    }
    else if (buf < ivars->buf) {
        int64_t underrun = CHY_PTR_TO_I64(ivars->buf) - CHY_PTR_TO_I64(buf);
        THROW(ERR, "Can't Advance_Buf backwards: (underrun: %i64))", underrun);
    }
    else {
        ivars->buf = buf;
    }
}

void
InStream_Read_Bytes_IMP(InStream *self, char* buf, size_t len) {
    SI_read_bytes(self, buf, len);
}

static CFISH_INLINE void
SI_read_bytes(InStream *self, char* buf, size_t len) {
    InStreamIVARS *const ivars = InStream_IVARS(self);
    const int64_t available
        = CHY_PTR_TO_I64(ivars->limit) - CHY_PTR_TO_I64(ivars->buf);
    if (available >= (int64_t)len) {
        // Request is entirely within buffer, so copy.
        memcpy(buf, ivars->buf, len);
        ivars->buf += len;
    }
    else {
        // Pass along whatever we've got in the buffer.
        if (available > 0) {
            memcpy(buf, ivars->buf, (size_t)available);
            buf += available;
            len -= (size_t)available;
            ivars->buf += available;
        }

        if (len < IO_STREAM_BUF_SIZE) {
            // Ensure that we have enough mapped, then copy the rest.
            int64_t got = S_refill(self);
            if (got < (int64_t)len) {
                int64_t orig_pos = SI_tell(self) - available;
                int64_t orig_len = len + available;
                THROW(ERR,  "Read past EOF of %o (pos: %i64 len: %i64 "
                      "request: %i64)", ivars->filename, orig_pos,
                      ivars->len, orig_len);
            }
            memcpy(buf, ivars->buf, len);
            ivars->buf += len;
        }
        else {
            // Too big to handle via the buffer, so resort to a brute-force
            // read.
            const int64_t sub_file_pos  = SI_tell(self);
            const int64_t real_file_pos = sub_file_pos + ivars->offset;
            bool success
                = FH_Read(ivars->file_handle, buf, real_file_pos, len);
            if (!success) {
                RETHROW(INCREF(Err_get_error()));
            }
            InStream_Seek_IMP(self, sub_file_pos + len);
        }
    }
}

int8_t
InStream_Read_I8_IMP(InStream *self) {
    InStreamIVARS *const ivars = InStream_IVARS(self);
    return (int8_t)SI_read_u8(self, ivars);
}

static CFISH_INLINE uint8_t
SI_read_u8(InStream *self, InStreamIVARS *ivars) {
    if (ivars->buf >= ivars->limit) { S_refill(self); }
    return (uint8_t)(*ivars->buf++);
}

uint8_t
InStream_Read_U8_IMP(InStream *self) {
    InStreamIVARS *const ivars = InStream_IVARS(self);
    return SI_read_u8(self, ivars);
}

static CFISH_INLINE uint32_t
SI_read_u32(InStream *self) {
    uint32_t retval;
    SI_read_bytes(self, (char*)&retval, 4);
#ifdef CHY_LITTLE_END
    retval = NumUtil_decode_bigend_u32((char*)&retval);
#endif
    return retval;
}

uint32_t
InStream_Read_U32_IMP(InStream *self) {
    return SI_read_u32(self);
}

int32_t
InStream_Read_I32_IMP(InStream *self) {
    return (int32_t)SI_read_u32(self);
}

static CFISH_INLINE uint64_t
SI_read_u64(InStream *self) {
    uint64_t retval;
    SI_read_bytes(self, (char*)&retval, 8);
#ifdef CHY_LITTLE_END
    retval = NumUtil_decode_bigend_u64((char*)&retval);
#endif
    return retval;
}

uint64_t
InStream_Read_U64_IMP(InStream *self) {
    return SI_read_u64(self);
}

int64_t
InStream_Read_I64_IMP(InStream *self) {
    return (int64_t)SI_read_u64(self);
}

float
InStream_Read_F32_IMP(InStream *self) {
    union { float f; uint32_t u32; } duo;
    SI_read_bytes(self, (char*)&duo, sizeof(float));
#ifdef CHY_LITTLE_END
    duo.u32 = NumUtil_decode_bigend_u32(&duo.u32);
#endif
    return duo.f;
}

double
InStream_Read_F64_IMP(InStream *self) {
    union { double d; uint64_t u64; } duo;
    SI_read_bytes(self, (char*)&duo, sizeof(double));
#ifdef CHY_LITTLE_END
    duo.u64 = NumUtil_decode_bigend_u64(&duo.u64);
#endif
    return duo.d;
}

uint32_t
InStream_Read_C32_IMP(InStream *self) {
    InStreamIVARS *const ivars = InStream_IVARS(self);
    uint32_t retval = 0;
    while (1) {
        const uint8_t ubyte = SI_read_u8(self, ivars);
        retval = (retval << 7) | (ubyte & 0x7f);
        if ((ubyte & 0x80) == 0) {
            break;
        }
    }
    return retval;
}

uint64_t
InStream_Read_C64_IMP(InStream *self) {
    InStreamIVARS *const ivars = InStream_IVARS(self);
    uint64_t retval = 0;
    while (1) {
        const uint8_t ubyte = SI_read_u8(self, ivars);
        retval = (retval << 7) | (ubyte & 0x7f);
        if ((ubyte & 0x80) == 0) {
            break;
        }
    }
    return retval;
}

int
InStream_Read_Raw_C64_IMP(InStream *self, char *buf) {
    InStreamIVARS *const ivars = InStream_IVARS(self);
    uint8_t *dest = (uint8_t*)buf;
    do {
        *dest = SI_read_u8(self, ivars);
    } while ((*dest++ & 0x80) != 0);
    return dest - (uint8_t*)buf;
}


