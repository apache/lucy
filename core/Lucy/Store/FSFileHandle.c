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

#include <errno.h>
#include <stdio.h>
#include <fcntl.h> // open, POSIX flags

#ifdef CHY_HAS_UNISTD_H
  #include <unistd.h> // close
#endif

#ifdef CHY_HAS_SYS_MMAN_H
  #include <sys/mman.h>
#elif defined(CHY_HAS_WINDOWS_H)
  #include <windows.h>
  #include <io.h>
#else
  #error "No support for memory mapped files"
#endif

#include "Lucy/Store/FSFileHandle.h"
#include "Lucy/Store/FileWindow.h"

// Convert FileHandle flags to POSIX flags.
static CFISH_INLINE int
SI_posix_flags(uint32_t fh_flags) {
    int posix_flags = 0;
    if (fh_flags & FH_WRITE_ONLY) { posix_flags |= O_WRONLY; }
    if (fh_flags & FH_READ_ONLY)  { posix_flags |= O_RDONLY; }
    if (fh_flags & FH_CREATE)     { posix_flags |= O_CREAT; }
    if (fh_flags & FH_EXCLUSIVE)  { posix_flags |= O_EXCL; }
#ifdef O_LARGEFILE
    posix_flags |= O_LARGEFILE;
#endif
#ifdef _O_BINARY
    posix_flags |= _O_BINARY;
#endif
    return posix_flags;
}

#define IS_64_BIT (CHY_SIZEOF_PTR == 8 ? 1 : 0)

// Memory map a region of the file with shared (read-only) permissions.  If
// the requested length is 0, return NULL.  If an error occurs, return NULL
// and set Err_error.
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

// Architecture- and OS- specific initialization for a read-only FSFileHandle.
static CFISH_INLINE bool
SI_init_read_only(FSFileHandle *self, FSFileHandleIVARS *ivars);

// Windows-specific routine needed for closing read-only handles.
#ifdef CHY_HAS_WINDOWS_H
static CFISH_INLINE bool
SI_close_win_handles(FSFileHandle *self);
#endif

FSFileHandle*
FSFH_open(String *path, uint32_t flags) {
    FSFileHandle *self = (FSFileHandle*)Class_Make_Obj(FSFILEHANDLE);
    return FSFH_do_open(self, path, flags);
}

FSFileHandle*
FSFH_do_open(FSFileHandle *self, String *path, uint32_t flags) {
    FH_do_open((FileHandle*)self, path, flags);
    FSFileHandleIVARS *const ivars = FSFH_IVARS(self);
    if (!path || !Str_Get_Size(path)) {
        Err_set_error(Err_new(Str_newf("Missing required param 'path'")));
        CFISH_DECREF(self);
        return NULL;
    }

    // Attempt to open file.
    if (flags & FH_WRITE_ONLY) {
        char *path_ptr = Str_To_Utf8(path);
        ivars->fd = open(path_ptr, SI_posix_flags(flags), 0666);
        FREEMEM(path_ptr);
        if (ivars->fd == -1) {
            ivars->fd = 0;
            Err_set_error(Err_new(Str_newf("Attempt to open '%o' failed: %s",
                                           path, strerror(errno))));
            CFISH_DECREF(self);
            return NULL;
        }
        if (flags & FH_EXCLUSIVE) {
            ivars->len = 0;
        }
        else {
            // Derive length.
            ivars->len = chy_lseek64(ivars->fd, INT64_C(0), SEEK_END);
            if (ivars->len == -1) {
                Err_set_error(Err_new(Str_newf("lseek64 on %o failed: %s",
                                               ivars->path, strerror(errno))));
                CFISH_DECREF(self);
                return NULL;
            }
            else {
                int64_t check_val
                    = chy_lseek64(ivars->fd, INT64_C(0), SEEK_SET);
                if (check_val == -1) {
                    Err_set_error(Err_new(Str_newf("lseek64 on %o failed: %s",
                                                   ivars->path, strerror(errno))));
                    CFISH_DECREF(self);
                    return NULL;
                }
            }
        }
    }
    else if (flags & FH_READ_ONLY) {
        if (SI_init_read_only(self, ivars)) {
            // On 64-bit systems, map the whole file up-front.
            if (IS_64_BIT && ivars->len) {
                ivars->buf = (char*)SI_map(self, ivars, 0, ivars->len);
                if (!ivars->buf) {
                    // An error occurred during SI_map, which has set
                    // Err_error for us already.
                    CFISH_DECREF(self);
                    return NULL;
                }
            }
        }
        else {
            CFISH_DECREF(self);
            return NULL;
        }
    }
    else {
        Err_set_error(Err_new(Str_newf("Must specify FH_READ_ONLY or FH_WRITE_ONLY to open '%o'",
                                       path)));
        CFISH_DECREF(self);
        return NULL;
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

    // Close system-specific handles.
    if (ivars->fd) {
        if (close(ivars->fd)) {
            Err_set_error(Err_new(Str_newf("Failed to close file: %s",
                                           strerror(errno))));
            return false;
        }
        ivars->fd  = 0;
    }
    #if (defined(CHY_HAS_WINDOWS_H) && !defined(CHY_HAS_SYS_MMAN_H))
    if (ivars->win_fhandle) {
        if (!SI_close_win_handles(self)) { return false; }
    }
    #endif

    return true;
}

bool
FSFH_Write_IMP(FSFileHandle *self, const void *data, size_t len) {
    FSFileHandleIVARS *const ivars = FSFH_IVARS(self);

    if (len) {
        // Write data, track file length, check for errors.
        int64_t check_val = write(ivars->fd, data, len);
        ivars->len += check_val;
        if ((size_t)check_val != len) {
            if (check_val == -1) {
                Err_set_error(Err_new(Str_newf("Error when writing %u64 bytes: %s",
                                               (uint64_t)len, strerror(errno))));
            }
            else {
                Err_set_error(Err_new(Str_newf("Attempted to write %u64 bytes, but wrote %i64",
                                               (uint64_t)len, check_val)));
            }
            return false;
        }
    }

    return true;
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
        Err_set_error(Err_new(Str_newf("Can't read from write-only handle")));
        return false;
    }
    else if (offset < 0) {
        Err_set_error(Err_new(Str_newf("Can't read from negative offset %i64",
                                       offset)));
        return false;
    }
    else if (end > ivars->len) {
        Err_set_error(Err_new(Str_newf("Tried to read past EOF: offset %i64 + request %i64 > len %i64",
                                       offset, len, ivars->len)));
        return false;
    }
    else {
        return SI_window(self, ivars, window, offset, len);
    }
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
    const int64_t end = offset + len;

    if (ivars->flags & FH_WRITE_ONLY) {
        Err_set_error(Err_new(Str_newf("Can't read from write-only filehandle")));
        return false;
    }
    if (offset < 0) {
        Err_set_error(Err_new(Str_newf("Can't read from an offset less than 0 (%i64)",
                                       offset)));
        return false;
    }
    else if (end > ivars->len) {
        Err_set_error(Err_new(Str_newf("Tried to read past EOF: offset %i64 + request %u64 > len %i64",
                                       offset, (uint64_t)len, ivars->len)));
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

static CFISH_INLINE bool
SI_init_read_only(FSFileHandle *self, FSFileHandleIVARS *ivars) {
    UNUSED_VAR(self);

    // Open.
    char *path_ptr = Str_To_Utf8(ivars->path);
    ivars->fd = open(path_ptr, SI_posix_flags(ivars->flags), 0666);
    FREEMEM(path_ptr);
    if (ivars->fd == -1) {
        ivars->fd = 0;
        Err_set_error(Err_new(Str_newf("Can't open '%o': %s", ivars->path,
                                       strerror(errno))));
        return false;
    }

    // Derive len.
    ivars->len = chy_lseek64(ivars->fd, INT64_C(0), SEEK_END);
    if (ivars->len == -1) {
        Err_set_error(Err_new(Str_newf("lseek64 on %o failed: %s", ivars->path,
                                       strerror(errno))));
        return false;
    }
    else {
        int64_t check_val = chy_lseek64(ivars->fd, INT64_C(0), SEEK_SET);
        if (check_val == -1) {
            Err_set_error(Err_new(Str_newf("lseek64 on %o failed: %s",
                                           ivars->path, strerror(errno))));
            return false;
        }
    }

    // Get system page size.
#if defined(_SC_PAGESIZE)
    ivars->page_size = sysconf(_SC_PAGESIZE);
#elif defined(_SC_PAGE_SIZE)
    ivars->page_size = sysconf(_SC_PAGE_SIZE);
#else
    #error "Can't determine system memory page size"
#endif

    return true;
}

static CFISH_INLINE void*
SI_map(FSFileHandle *self, FSFileHandleIVARS *ivars, int64_t offset,
       int64_t len) {
    UNUSED_VAR(self);
    void *buf = NULL;

    if (len) {
        // Read-only memory mapping.
        buf = mmap(NULL, len, PROT_READ, MAP_SHARED, ivars->fd, offset);
        if (buf == (void*)(-1)) {
            Err_set_error(Err_new(Str_newf("mmap of offset %i64 and length %i64 (page size %i64) "
                                           "against '%o' failed: %s",
                                           offset, len, ivars->page_size,
                                           ivars->path, strerror(errno))));
            return NULL;
        }
    }

    return buf;
}

static CFISH_INLINE bool
SI_unmap(FSFileHandle *self, char *buf, int64_t len) {
    if (buf != NULL) {
        if (munmap(buf, len)) {
            Err_set_error(Err_new(Str_newf("Failed to munmap '%o': %s",
                                           FSFH_IVARS(self)->path,
                                           strerror(errno))));
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
        Err_set_error(Err_new(Str_newf("Can't read from an offset less than 0 (%i64)",
                                       offset)));
        return false;
    }

    // Read.
    check_val = chy_pread64(ivars->fd, dest, len, offset);
    if (check_val != (int64_t)len) {
        if (check_val == -1) {
            Err_set_error(Err_new(Str_newf("Tried to read %u64 bytes, got %i64: %s",
                                           (uint64_t)len, check_val, strerror(errno))));
        }
        else {
            Err_set_error(Err_new(Str_newf("Tried to read %u64 bytes, got %i64",
                                           (uint64_t)len, check_val)));
        }
        return false;
    }

    return true;
}
#endif // !IS_64_BIT

/********************************* WINDOWS **********************************/

#elif defined(CHY_HAS_WINDOWS_H)

static CFISH_INLINE bool
SI_init_read_only(FSFileHandle *self, FSFileHandleIVARS *ivars) {
    char *filepath = Str_To_Utf8(ivars->path);
    SYSTEM_INFO sys_info;

    // Get system page size.
    GetSystemInfo(&sys_info);
    ivars->page_size = sys_info.dwAllocationGranularity;

    // Open.
    ivars->win_fhandle = CreateFile(
                            filepath,
                            GENERIC_READ,
                            FILE_SHARE_READ,
                            NULL,
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_READONLY | FILE_FLAG_OVERLAPPED,
                            NULL
                        );
    FREEMEM(filepath);
    if (ivars->win_fhandle == INVALID_HANDLE_VALUE) {
        char *win_error = Err_win_error();
        Err_set_error(Err_new(Str_newf("CreateFile for %o failed: %s",
                                       ivars->path, win_error)));
        FREEMEM(win_error);
        return false;
    }

    // Derive len.
    DWORD file_size_hi;
    DWORD file_size_lo = GetFileSize(ivars->win_fhandle, &file_size_hi);
    if (file_size_lo == INVALID_FILE_SIZE && GetLastError() != NO_ERROR) {
        Err_set_error(Err_new(Str_newf("GetFileSize for %o failed",
                                       ivars->path)));
        return false;
    }
    ivars->len = ((uint64_t)file_size_hi << 32) | file_size_lo;

    // Init mapping handle.
    ivars->buf = NULL;
    if (ivars->len) {
        ivars->win_maphandle = CreateFileMapping(ivars->win_fhandle, NULL,
                                                 PAGE_READONLY, 0, 0, NULL);
        if (ivars->win_maphandle == NULL) {
            char *win_error = Err_win_error();
            Err_set_error(Err_new(Str_newf("CreateFileMapping for %o failed: %s",
                                           ivars->path, win_error)));
            FREEMEM(win_error);
            return false;
        }
    }

    return true;
}

static CFISH_INLINE void*
SI_map(FSFileHandle *self, FSFileHandleIVARS *ivars, int64_t offset,
       int64_t len) {
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
            char *win_error = Err_win_error();
            Err_set_error(Err_new(Str_newf("MapViewOfFile for %o failed: %s",
                                           ivars->path, win_error)));
            FREEMEM(win_error);
        }
    }

    return buf;
}

static CFISH_INLINE bool
SI_unmap(FSFileHandle *self, char *buf, int64_t len) {
    if (buf != NULL) {
        if (!UnmapViewOfFile(buf)) {
            char *win_error = Err_win_error();
            Err_set_error(Err_new(Str_newf("Failed to unmap '%o': %s",
                                           FSFH_IVARS(self)->path,
                                           win_error)));
            FREEMEM(win_error);
            return false;
        }
    }
    return true;
}

static CFISH_INLINE bool
SI_close_win_handles(FSFileHandle *self) {
    FSFileHandleIVARS *ivars = FSFH_IVARS(self);
    // Close both standard handle and mapping handle.
    if (ivars->win_maphandle) {
        if (!CloseHandle(ivars->win_maphandle)) {
            char *win_error = Err_win_error();
            Err_set_error(Err_new(Str_newf("Failed to close file mapping handle: %s",
                                           win_error)));
            FREEMEM(win_error);
            return false;
        }
        ivars->win_maphandle = NULL;
    }
    if (ivars->win_fhandle) {
        if (!CloseHandle(ivars->win_fhandle)) {
            char *win_error = Err_win_error();
            Err_set_error(Err_new(Str_newf("Failed to close file handle: %s",
                                           win_error)));
            FREEMEM(win_error);
            return false;
        }
        ivars->win_fhandle = NULL;
    }

    return true;
}

#if !IS_64_BIT
bool
FSFH_Read_IMP(FSFileHandle *self, char *dest, int64_t offset, size_t len) {
    FSFileHandleIVARS *const ivars = FSFH_IVARS(self);
    BOOL check_val;
    DWORD got;
    OVERLAPPED read_op_state;
    uint64_t offs = (uint64_t)offset;

    read_op_state.hEvent     = NULL;
    read_op_state.OffsetHigh = offs >> 32;
    read_op_state.Offset     = offs & 0xFFFFFFFF;

    // Sanity check.
    if (offset < 0) {
        Err_set_error(Err_new(Str_newf("Can't read from an offset less than 0 (%i64)",
                                       offset)));
        return false;
    }

    // ReadFile() takes a DWORD (unsigned 32-bit integer) as a length
    // argument, so throw a sensible error rather than wrap around.
    if (len > UINT32_MAX) {
        Err_set_error(Err_new(Str_newf("Can't read more than 4 GB (%u64)",
                                       (uint64_t)len)));
        return false;
    }

    // Read.
    check_val = ReadFile(ivars->win_fhandle, dest, len, &got, &read_op_state);
    if (!check_val && GetLastError() == ERROR_IO_PENDING) {
        // Read has been queued by the OS and will soon complete.  Wait for
        // it, since this is a blocking IO call from the point of the rest of
        // the library.
        check_val = GetOverlappedResult(ivars->win_fhandle, &read_op_state,
                                        &got, TRUE);
    }

    // Verify that the read has succeeded by now.
    if (!check_val) {
        char *win_error = Err_win_error();
        Err_set_error(Err_new(Str_newf("Failed to read %u64 bytes: %s",
                                       (uint64_t)len, win_error)));
        FREEMEM(win_error);
        return false;
    }

    return true;
}
#endif // !IS_64_BIT

#endif // CHY_HAS_SYS_MMAN_H vs. CHY_HAS_WINDOWS_H


