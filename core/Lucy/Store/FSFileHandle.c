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

#define C_LUCY_FSFILEHANDLE
#include "Lucy/Util/ToolSet.h"

#include "charmony.h"

#include "Lucy/Store/ErrorMessage.h"
#include "Lucy/Store/FSFileHandle.h"
#include "Lucy/Store/FileWindow.h"

#define IS_64_BIT (CHY_SIZEOF_PTR == 8 ? 1 : 0)

// Architecture- and OS- specific initialization.
static bool
S_init(FSFileHandleIVARS *ivars, String *path, uint32_t flags);

// Close file in an OS-specific way.
static bool
S_close(FSFileHandleIVARS *ivars);

// Memory map a region of the file with shared (read-only) permissions.  If
// the requested length is 0, return NULL.  If an error occurs, return NULL
// and set the global error object.
static CFISH_INLINE void*
SI_map(FSFileHandle *self, FSFileHandleIVARS *ivars, int64_t offset,
       int64_t len);

// Release a memory mapped region assigned by SI_map.
static CFISH_INLINE bool
SI_unmap(FSFileHandle *self, char *ptr, int64_t len);

// 32-bit or 64-bit inlined helpers for FSFH_window.
static CFISH_INLINE bool
SI_window(FSFileHandle *self, FSFileHandleIVARS *ivars, FileWindow *window,
          int64_t offset, int64_t len);

FSFileHandle*
FSFH_open(String *path, uint32_t flags) {
    FSFileHandle *self = (FSFileHandle*)Class_Make_Obj(FSFILEHANDLE);
    return FSFH_do_open(self, path, flags);
}

FSFileHandle*
FSFH_do_open(FSFileHandle *self, String *path, uint32_t flags) {
    if (!FH_do_open((FileHandle*)self, path, flags)) { return NULL; }
    FSFileHandleIVARS *const ivars = FSFH_IVARS(self);

    if (!path || !Str_Get_Size(path)) {
        ErrMsg_set("Missing required param 'path'");
        DECREF(self);
        return NULL;
    }

    // Attempt to open file.
    if (!S_init(ivars, path, flags)) {
        DECREF(self);
        return NULL;
    }

    // On 64-bit systems, map the whole file up-front.
    if (IS_64_BIT && (flags & FH_READ_ONLY) && ivars->len) {
        ivars->buf = (char*)SI_map(self, ivars, 0, ivars->len);
        if (!ivars->buf) {
            // An error occurred during SI_map, which has set
            // the global error object for us already.
            DECREF(self);
            return NULL;
        }
    }

    return self;
}

bool
FSFH_Close_IMP(FSFileHandle *self) {
    FSFileHandleIVARS *const ivars = FSFH_IVARS(self);

    // On 64-bit systems, cancel the whole-file mapping.
    if (IS_64_BIT && (ivars->flags & FH_READ_ONLY) && ivars->buf != NULL) {
        if (!SI_unmap(self, ivars->buf, ivars->len)) { return false; }
        ivars->buf = NULL;
    }

    return S_close(ivars);
}

int64_t
FSFH_Length_IMP(FSFileHandle *self) {
    return FSFH_IVARS(self)->len;
}

bool
FSFH_Window_IMP(FSFileHandle *self, FileWindow *window, int64_t offset,
                int64_t len) {
    FSFileHandleIVARS *const ivars = FSFH_IVARS(self);
    const int64_t end = offset + len;
    if (!(ivars->flags & FH_READ_ONLY)) {
        ErrMsg_set("Can't read from write-only handle");
        return false;
    }
    else if (offset < 0) {
        ErrMsg_set("Can't read from negative offset %i64", offset);
        return false;
    }
    else if (end > ivars->len) {
        ErrMsg_set("Tried to read past EOF: "
                   "offset %i64 + request %i64 > len %i64",
                   offset, len, ivars->len);
        return false;
    }
    else {
        return SI_window(self, ivars, window, offset, len);
    }
}

int
FSFH_Set_FD_IMP(FSFileHandle *self, int fd) {
    FSFileHandleIVARS *const ivars = FSFH_IVARS(self);
    int old_fd = ivars->fd;
    ivars->fd = fd;
    return old_fd;
}

/********************************* 64-bit *********************************/

#if IS_64_BIT

static CFISH_INLINE bool
SI_window(FSFileHandle *self, FSFileHandleIVARS *ivars, FileWindow *window,
          int64_t offset, int64_t len) {
    UNUSED_VAR(self);
    FileWindow_Set_Window(window, ivars->buf + offset, offset, len);
    return true;
}

bool
FSFH_Release_Window_IMP(FSFileHandle *self, FileWindow *window) {
    UNUSED_VAR(self);
    FileWindow_Set_Window(window, NULL, 0, 0);
    return true;
}

bool
FSFH_Read_IMP(FSFileHandle *self, char *dest, int64_t offset, size_t len) {
    FSFileHandleIVARS *const ivars = FSFH_IVARS(self);
    const int64_t end = offset + (int64_t)len;

    if (ivars->flags & FH_WRITE_ONLY) {
        ErrMsg_set("Can't read from write-only filehandle");
        return false;
    }
    if (offset < 0 || end < offset) {
        ErrMsg_set("Invalid offset and len (%i64, %u64)",
                   offset, (uint64_t)len);
        return false;
    }
    else if (end > ivars->len) {
        ErrMsg_set("Tried to read past EOF: "
                   "offset %i64 + request %u64 > len %i64",
                   offset, (uint64_t)len, ivars->len);
        return false;
    }
    memcpy(dest, ivars->buf + offset, len);
    return true;
}

/********************************* 32-bit *********************************/

#else

static CFISH_INLINE bool
SI_window(FSFileHandle *self, FSFileHandleIVARS *ivars, FileWindow *window,
          int64_t offset, int64_t len) {
    // Release the previously mmap'd region, if any.
    FSFH_Release_Window_IMP(self, window);

    // Start map on a page boundary.  Ensure that the window is at
    // least wide enough to view all the data spec'd in the original
    // request.
    const int64_t remainder       = offset % ivars->page_size;
    const int64_t adjusted_offset = offset - remainder;
    const int64_t adjusted_len    = len + remainder;
    char *const buf
        = (char*)SI_map(self, ivars, adjusted_offset, adjusted_len);
    if (len && buf == NULL) {
        return false;
    }
    else {
        FileWindow_Set_Window(window, buf, adjusted_offset, adjusted_len);
    }

    return true;
}

bool
FSFH_Release_Window_IMP(FSFileHandle *self, FileWindow *window) {
    char *buf = FileWindow_Get_Buf(window);
    int64_t len = FileWindow_Get_Len(window);
    if (!SI_unmap(self, buf, len)) { return false; }
    FileWindow_Set_Window(window, NULL, 0, 0);
    return true;
}

#endif // IS_64_BIT vs. 32-bit

/********************************* UNIXEN *********************************/

#ifdef CHY_HAS_SYS_MMAN_H

#include <fcntl.h> // open, POSIX flags
#include <sys/mman.h>
#include <unistd.h> // close

static bool
S_init(FSFileHandleIVARS *ivars, String *path, uint32_t flags) {
    int posix_flags = 0;
    if (flags & FH_WRITE_ONLY) { posix_flags |= O_WRONLY; }
    if (flags & FH_READ_ONLY)  { posix_flags |= O_RDONLY; }
    if (flags & FH_CREATE)     { posix_flags |= O_CREAT; }
    if (flags & FH_EXCLUSIVE)  { posix_flags |= O_EXCL; }
#ifdef O_LARGEFILE
    posix_flags |= O_LARGEFILE;
#endif

    char *path_ptr = Str_To_Utf8(path);
    int fd = open(path_ptr, posix_flags, 0666);
    FREEMEM(path_ptr);
    if (fd == -1) {
        ErrMsg_set_with_errno("Attempt to open '%o' failed", path);
        return false;
    }

    if (flags & FH_EXCLUSIVE) {
        ivars->len = 0;
    }
    else {
        // Derive length.
        ivars->len = chy_lseek64(fd, INT64_C(0), SEEK_END);
        if (ivars->len == -1) {
            ErrMsg_set_with_errno("lseek64 on %o failed", path);
            return false;
        }
        int64_t check_val = chy_lseek64(fd, INT64_C(0), SEEK_SET);
        if (check_val == -1) {
            ErrMsg_set_with_errno("lseek64 on %o failed", path);
            return false;
        }
    }

    if (flags & FH_READ_ONLY) {
        // Get system page size.
#if defined(_SC_PAGESIZE)
        ivars->page_size = sysconf(_SC_PAGESIZE);
#elif defined(_SC_PAGE_SIZE)
        ivars->page_size = sysconf(_SC_PAGE_SIZE);
#else
        #error "Can't determine system memory page size"
#endif
    }

    ivars->fd = fd;

    return true;
}

static bool
S_close(FSFileHandleIVARS *ivars) {
    // Close system-specific handles.
    if (ivars->fd) {
        if (close(ivars->fd)) {
            ErrMsg_set_with_errno("Failed to close file '%o'", ivars->path);
            return false;
        }
        ivars->fd = 0;
    }
    return true;
}

bool
FSFH_Write_IMP(FSFileHandle *self, const void *data, size_t len) {
    FSFileHandleIVARS *const ivars = FSFH_IVARS(self);

    if (len) {
        // Write data, track file length, check for errors.
        int64_t check_val = write(ivars->fd, data, len);
        if (check_val == -1) {
            ErrMsg_set_with_errno("Error when writing %u64 bytes",
                                  (uint64_t)len);
            return false;
        }

        ivars->len += check_val;
        if ((size_t)check_val != len) {
            ErrMsg_set("Attempted to write %u64 bytes, but wrote %i64",
                       (uint64_t)len, check_val);
            return false;
        }
    }

    return true;
}

static CFISH_INLINE void*
SI_map(FSFileHandle *self, FSFileHandleIVARS *ivars, int64_t offset,
       int64_t len) {
    UNUSED_VAR(self);
    void *buf = NULL;

    if (len) {
        // Read-only memory mapping.
        buf = mmap(NULL, (size_t)len, PROT_READ, MAP_SHARED, ivars->fd, offset);
        if (buf == (void*)(-1)) {
            ErrMsg_set_with_errno("mmap of offset %i64 and length %i64 "
                                  "(page size %i64) against '%o' failed",
                                  offset, len, ivars->page_size, ivars->path);
            return NULL;
        }
    }

    return buf;
}

static CFISH_INLINE bool
SI_unmap(FSFileHandle *self, char *buf, int64_t len) {
    if (buf != NULL) {
        if (munmap(buf, (size_t)len)) {
            ErrMsg_set_with_errno("Failed to munmap '%o'",
                                  FSFH_IVARS(self)->path);
            return false;
        }
    }
    return true;
}

#if !IS_64_BIT
bool
FSFH_Read_IMP(FSFileHandle *self, char *dest, int64_t offset, size_t len) {
    FSFileHandleIVARS *const ivars = FSFH_IVARS(self);
    int64_t check_val;

    // Sanity check.
    if (offset < 0) {
        ErrMsg_set("Can't read from an offset less than 0 (%i64)", offset);
        return false;
    }

    // Read.
    check_val = chy_pread64(ivars->fd, dest, len, offset);
    if (check_val != (int64_t)len) {
        if (check_val == -1) {
            ErrMsg_set_with_errno("Tried to read %u64 bytes, got %i64",
                                  (uint64_t)len, check_val);
        }
        else {
            ErrMsg_set("Tried to read %u64 bytes, got %i64",
                       (uint64_t)len, check_val);
        }
        return false;
    }

    return true;
}
#endif // !IS_64_BIT

/********************************* WINDOWS **********************************/

#elif defined(CHY_HAS_WINDOWS_H)

#include <windows.h>

static bool
S_init(FSFileHandleIVARS *ivars, String *path, uint32_t flags) {
    DWORD desired_access       = flags & FH_READ_ONLY
                                 ? GENERIC_READ
                                 : GENERIC_WRITE;
    DWORD share_mode           = FILE_SHARE_READ;
    DWORD creation_disposition = flags & FH_CREATE
                                 ? flags & FH_EXCLUSIVE
                                   ? CREATE_NEW
                                   : OPEN_ALWAYS
                                 : OPEN_EXISTING;

    share_mode |= FILE_SHARE_DELETE;
    char *path_ptr = Str_To_Utf8(path);
    HANDLE handle
        = CreateFileA(path_ptr, desired_access, share_mode, NULL,
                      creation_disposition, FILE_ATTRIBUTE_NOT_CONTENT_INDEXED,
                      NULL);
    FREEMEM(path_ptr);

    if (handle == INVALID_HANDLE_VALUE) {
        ErrMsg_set_with_win_error("CreateFile for '%o' failed", path);
        return false;
    }

    if (flags & FH_EXCLUSIVE) {
        ivars->len = 0;
    }
    else {
        // Derive len.
        DWORD file_size_hi;
        DWORD file_size_lo = GetFileSize(handle, &file_size_hi);
        if (file_size_lo == INVALID_FILE_SIZE && GetLastError() != NO_ERROR) {
            ErrMsg_set_with_win_error("GetFileSize for '%o' failed", path);
            return false;
        }
        ivars->len = ((uint64_t)file_size_hi << 32) | file_size_lo;
    }

    if (flags & FH_READ_ONLY) {
        // Get system page size.
        SYSTEM_INFO sys_info;
        GetSystemInfo(&sys_info);
        ivars->page_size = sys_info.dwAllocationGranularity;

        // Init mapping handle.
        if (ivars->len) {
            ivars->win_maphandle
                = CreateFileMapping(handle, NULL, PAGE_READONLY, 0, 0, NULL);
            if (ivars->win_maphandle == NULL) {
                ErrMsg_set_with_win_error("CreateFileMapping for '%o' failed",
                                          path);
                return false;
            }
        }
    }

    ivars->win_fhandle = handle;
    return true;
}

static bool
S_close(FSFileHandleIVARS *ivars) {
    if (ivars->win_fhandle) {
        // Close both standard handle and mapping handle.
        if (ivars->win_maphandle) {
            if (CloseHandle(ivars->win_maphandle) == 0) {
                ErrMsg_set_with_win_error("CloseHandle for '%o' mapping"
                                          " failed", ivars->path);
                return false;
            }
            ivars->win_maphandle = NULL;
        }

        if (CloseHandle(ivars->win_fhandle) == 0) {
            ErrMsg_set_with_win_error("CloseHandle for '%o' failed",
                                      ivars->path);
            return false;
        }
        ivars->win_fhandle = NULL;
    }

    return true;
}

bool
FSFH_Write_IMP(FSFileHandle *self, const void *vdata, size_t len) {
    FSFileHandleIVARS *const ivars = FSFH_IVARS(self);
    const char *data = (const char*)vdata;

    while (len) {
        // Write in 2GB chunks.
        DWORD to_write = len > 0x80000000 ? 0x80000000 : len;
        DWORD written = 0;

        // Write data, track file length, check for errors.
        BOOL result
            = WriteFile(ivars->win_fhandle, data, to_write, &written, NULL);
        ivars->len += written;
        if (result == 0) {
            ErrMsg_set_with_win_error("WriteFile for '%o' failed",
                                      ivars->path);
            return false;
        }

        data += to_write;
        len  -= to_write;
    }

    return true;
}

static CFISH_INLINE void*
SI_map(FSFileHandle *self, FSFileHandleIVARS *ivars, int64_t offset,
       int64_t len) {
    UNUSED_VAR(self);
    void *buf = NULL;

    if (len) {
        // Read-only memory map.
        uint64_t offs = (uint64_t)offset;
        DWORD file_offset_hi = offs >> 32;
        DWORD file_offset_lo = offs & 0xFFFFFFFF;
        size_t amount = (size_t)len;
        buf = MapViewOfFile(ivars->win_maphandle, FILE_MAP_READ,
                            file_offset_hi, file_offset_lo, amount);
        if (buf == NULL) {
            ErrMsg_set_with_win_error("MapViewOfFile for %o failed",
                                      ivars->path);
        }
    }

    return buf;
}

static CFISH_INLINE bool
SI_unmap(FSFileHandle *self, char *buf, int64_t len) {
    UNUSED_VAR(self);
    UNUSED_VAR(len);
    if (buf != NULL) {
        if (!UnmapViewOfFile(buf)) {
            ErrMsg_set_with_win_error("Failed to unmap '%o'",
                                      FSFH_IVARS(self)->path);
            return false;
        }
    }
    return true;
}

#if !IS_64_BIT
bool
FSFH_Read_IMP(FSFileHandle *self, char *dest, int64_t offset, size_t len) {
    FSFileHandleIVARS *const ivars = FSFH_IVARS(self);

    // Sanity check.
    if (offset < 0) {
        ErrMsg_set("Can't read from an offset less than 0 (%i64)", offset);
        return false;
    }

    DWORD      got;
    OVERLAPPED overlapped;
    overlapped.hEvent     = NULL;
    overlapped.OffsetHigh = offset >> 32;
    overlapped.Offset     = offset & 0xFFFFFFFF;

    if (ReadFile(ivars->win_fhandle, dest, len, &got, &overlapped) == 0) {
        ErrMsg_set_with_win_error("Failed to read %u64 bytes", (uint64_t)len);
        return false;
    }
    if (got < len) {
        ErrMsg_set("Tried to read %u64 bytes, got %u32", (uint64_t)len, got);
        return false;
    }

    return true;
}
#endif // !IS_64_BIT

#else // CHY_HAS_SYS_MMAN_H vs. CHY_HAS_WINDOWS_H

#error "No support for memory mapped files"

#endif // CHY_HAS_SYS_MMAN_H vs. CHY_HAS_WINDOWS_H


