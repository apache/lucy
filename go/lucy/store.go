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

package lucy

/*
#include <stdlib.h>

#include "Lucy/Store/Lock.h"
#include "Lucy/Store/Folder.h"
#include "Lucy/Store/InStream.h"
#include "Lucy/Store/OutStream.h"
#include "Lucy/Store/DirHandle.h"
#include "Lucy/Store/FSDirHandle.h"
#include "Lucy/Store/FileHandle.h"
#include "Lucy/Store/FSFileHandle.h"
#include "Lucy/Store/RAMFileHandle.h"
#include "Lucy/Store/CompoundFileReader.h"
#include "Lucy/Store/CompoundFileWriter.h"

#include "Clownfish/Err.h"
*/
import "C"
import "unsafe"
import "fmt"

import "git-wip-us.apache.org/repos/asf/lucy-clownfish.git/runtime/go/clownfish"

type DirHandleIMP struct {
	clownfish.ObjIMP
	err error
}

func (e *LockErrIMP) Error() string {
	self := ((*C.lucy_LockErr)(unsafe.Pointer(e.TOPTR())))
	return clownfish.CFStringToGo(unsafe.Pointer(C.LUCY_LockErr_Get_Mess(self)))
}

func OpenInStream(file interface{}) (in InStream, err error) {
	err = clownfish.TrapErr(func() {
		fileC := (*C.cfish_Obj)(clownfish.GoToClownfish(file, unsafe.Pointer(C.CFISH_OBJ), false))
		defer C.cfish_decref(unsafe.Pointer(fileC))
		cfObj := C.lucy_InStream_open(fileC)
		if cfObj == nil {
			cfErr := C.cfish_Err_get_error();

			panic(clownfish.WRAPAny(unsafe.Pointer(C.cfish_incref(unsafe.Pointer(cfErr)))).(error))
		}
		in = WRAPInStream(unsafe.Pointer(cfObj))
	})
	return in, err
}

func (in *InStreamIMP) Reopen(fileName string, offset int64, length int64) (InStream, error) {
	var retval InStream
	err := clownfish.TrapErr(func() {
		self := (*C.lucy_InStream)(clownfish.Unwrap(in, "in"))
		fileNameCF := (*C.cfish_String)(clownfish.GoToClownfish(fileName, unsafe.Pointer(C.CFISH_STRING), true))
		defer C.cfish_decref(unsafe.Pointer(fileNameCF))
		retvalCF := C.LUCY_InStream_Reopen(self, fileNameCF,
			C.int64_t(offset), C.int64_t(length))
		retval = WRAPInStream(unsafe.Pointer(retvalCF))
	})
	return retval, err
}

func (in *InStreamIMP) Close() error {
	return clownfish.TrapErr(func() {
		self := (*C.lucy_InStream)(clownfish.Unwrap(in, "in"))
		C.LUCY_InStream_Close(self)
	})
}

func (in *InStreamIMP) Seek(target int64) error {
	return clownfish.TrapErr(func() {
		self := (*C.lucy_InStream)(clownfish.Unwrap(in, "in"))
		C.LUCY_InStream_Seek(self, C.int64_t(target))
	})
}

func (in *InStreamIMP) ReadString() (string, error) {
	var retval string
	err := clownfish.TrapErr(func() {
		self := (*C.lucy_InStream)(clownfish.Unwrap(in, "in"))
		size := C.size_t(C.LUCY_InStream_Read_C32(self))
		buf := (*C.char)(C.malloc(size))
		defer C.free(unsafe.Pointer(buf))
		C.LUCY_InStream_Read_Bytes(self, buf, size)
		retval = C.GoStringN(buf, C.int(size))
	})
	return retval, err
}

func (in *InStreamIMP) ReadBytes(b []byte, size int) error {
	return clownfish.TrapErr(func() {
		self := (*C.lucy_InStream)(clownfish.Unwrap(in, "in"))
		buf := (*C.char)(C.malloc(C.size_t(size)))
		defer C.free(unsafe.Pointer(buf))
		C.LUCY_InStream_Read_Bytes(self, buf, C.size_t(size))
		dupe := []byte(C.GoStringN(buf, C.int(size)))
		for i := 0; i < size; i++ {
			b[i] = dupe[i]
		}
	})
}

func (in *InStreamIMP) ReadI8() (int8, error) {
	var retval int8
	err := clownfish.TrapErr(func() {
		self := (*C.lucy_InStream)(clownfish.Unwrap(in, "in"))
		retval = int8(C.LUCY_InStream_Read_I8(self))
	})
	return retval, err
}

func (in *InStreamIMP) ReadI32() (int32, error) {
	var retval int32
	err := clownfish.TrapErr(func() {
		self := (*C.lucy_InStream)(clownfish.Unwrap(in, "in"))
		retval = int32(C.LUCY_InStream_Read_I32(self))
	})
	return retval, err
}

func (in *InStreamIMP) ReadI64() (int64, error) {
	var retval int64
	err := clownfish.TrapErr(func() {
		self := (*C.lucy_InStream)(clownfish.Unwrap(in, "in"))
		retval = int64(C.LUCY_InStream_Read_I64(self))
	})
	return retval, err
}

func (in *InStreamIMP) ReadU8() (uint8, error) {
	var retval uint8
	err := clownfish.TrapErr(func() {
		self := (*C.lucy_InStream)(clownfish.Unwrap(in, "in"))
		retval = uint8(C.LUCY_InStream_Read_U8(self))
	})
	return retval, err
}

func (in *InStreamIMP) ReadU32() (uint32, error) {
	var retval uint32
	err := clownfish.TrapErr(func() {
		self := (*C.lucy_InStream)(clownfish.Unwrap(in, "in"))
		retval = uint32(C.LUCY_InStream_Read_U32(self))
	})
	return retval, err
}

func (in *InStreamIMP) ReadU64() (uint64, error) {
	var retval uint64
	err := clownfish.TrapErr(func() {
		self := (*C.lucy_InStream)(clownfish.Unwrap(in, "in"))
		retval = uint64(C.LUCY_InStream_Read_U64(self))
	})
	return retval, err
}

func (in *InStreamIMP) ReadC32() (uint32, error) {
	var retval uint32
	err := clownfish.TrapErr(func() {
		self := (*C.lucy_InStream)(clownfish.Unwrap(in, "in"))
		retval = uint32(C.LUCY_InStream_Read_C32(self))
	})
	return retval, err
}

func (in *InStreamIMP) ReadC64() (uint64, error) {
	var retval uint64
	err := clownfish.TrapErr(func() {
		self := (*C.lucy_InStream)(clownfish.Unwrap(in, "in"))
		retval = uint64(C.LUCY_InStream_Read_C64(self))
	})
	return retval, err
}

func (in *InStreamIMP) ReadF32() (float32, error) {
	var retval float32
	err := clownfish.TrapErr(func() {
		self := (*C.lucy_InStream)(clownfish.Unwrap(in, "in"))
		retval = float32(C.LUCY_InStream_Read_F32(self))
	})
	return retval, err
}

func (in *InStreamIMP) ReadF64() (float64, error) {
	var retval float64
	err := clownfish.TrapErr(func() {
		self := (*C.lucy_InStream)(clownfish.Unwrap(in, "in"))
		retval = float64(C.LUCY_InStream_Read_F64(self))
	})
	return retval, err
}

func OpenOutStream(file interface{}) (out OutStream, err error) {
	err = clownfish.TrapErr(func() {
		fileC := (*C.cfish_Obj)(clownfish.GoToClownfish(file, unsafe.Pointer(C.CFISH_OBJ), false))
		defer C.cfish_decref(unsafe.Pointer(fileC))
		cfObj := C.lucy_OutStream_open(fileC)
		if cfObj == nil {
			cfErr := C.cfish_Err_get_error();
			panic(clownfish.WRAPAny(unsafe.Pointer(C.cfish_incref(unsafe.Pointer(cfErr)))).(error))
		}
		out = WRAPOutStream(unsafe.Pointer(cfObj))
	})
	return out, err
}

func (out *OutStreamIMP) Close() error {
	return clownfish.TrapErr(func() {
		self := (*C.lucy_OutStream)(clownfish.Unwrap(out, "out"))
		C.LUCY_OutStream_Close(self)
	})
}

func (out *OutStreamIMP) Grow(length int64) error {
	return clownfish.TrapErr(func() {
		self := (*C.lucy_OutStream)(clownfish.Unwrap(out, "out"))
		C.LUCY_OutStream_Grow(self, C.int64_t(length))
	})
}

func (out *OutStreamIMP) Align(modulus int64) error {
	return clownfish.TrapErr(func() {
		self := (*C.lucy_OutStream)(clownfish.Unwrap(out, "out"))
		C.LUCY_OutStream_Align(self, C.int64_t(modulus))
	})
}

func (out *OutStreamIMP) WriteBytes(content []byte, size int) error {
	return clownfish.TrapErr(func() {
		self := (*C.lucy_OutStream)(clownfish.Unwrap(out, "out"))
		if size > len(content) {
			panic(clownfish.NewErr(fmt.Sprintf("Buf only %d long, can't write %d bytes",
				len(content), size)))
		}
		octets := C.CString(string(content))
		defer C.free(unsafe.Pointer(octets))
		C.LUCY_OutStream_Write_Bytes(self, unsafe.Pointer(octets), C.size_t(size))
	})
}

func (out *OutStreamIMP) WriteString(content string) error {
	return clownfish.TrapErr(func() {
		self := (*C.lucy_OutStream)(clownfish.Unwrap(out, "out"))
		octets := C.CString(content)
		defer C.free(unsafe.Pointer(octets))
		size := len(content)
		C.LUCY_OutStream_Write_String(self, octets, C.size_t(size))
	})
}

func (out *OutStreamIMP) WriteI8(value int8) error {
	return clownfish.TrapErr(func() {
		self := (*C.lucy_OutStream)(clownfish.Unwrap(out, "out"))
		C.LUCY_OutStream_Write_I8(self, C.int8_t(value))
	})
}

func (out *OutStreamIMP) WriteI32(value int32) error {
	return clownfish.TrapErr(func() {
		self := (*C.lucy_OutStream)(clownfish.Unwrap(out, "out"))
		C.LUCY_OutStream_Write_I32(self, C.int32_t(value))
	})
}

func (out *OutStreamIMP) WriteI64(value int64) error {
	return clownfish.TrapErr(func() {
		self := (*C.lucy_OutStream)(clownfish.Unwrap(out, "out"))
		C.LUCY_OutStream_Write_I64(self, C.int64_t(value))
	})
}

func (out *OutStreamIMP) WriteU8(value uint8) error {
	return clownfish.TrapErr(func() {
		self := (*C.lucy_OutStream)(clownfish.Unwrap(out, "out"))
		C.LUCY_OutStream_Write_U8(self, C.uint8_t(value))
	})
}

func (out *OutStreamIMP) WriteU32(value uint32) error {
	return clownfish.TrapErr(func() {
		self := (*C.lucy_OutStream)(clownfish.Unwrap(out, "out"))
		C.LUCY_OutStream_Write_U32(self, C.uint32_t(value))
	})
}

func (out *OutStreamIMP) WriteU64(value uint64) error {
	return clownfish.TrapErr(func() {
		self := (*C.lucy_OutStream)(clownfish.Unwrap(out, "out"))
		C.LUCY_OutStream_Write_U64(self, C.uint64_t(value))
	})
}

func (out *OutStreamIMP) WriteC32(value uint32) error {
	return clownfish.TrapErr(func() {
		self := (*C.lucy_OutStream)(clownfish.Unwrap(out, "out"))
		C.LUCY_OutStream_Write_C32(self, C.uint32_t(value))
	})
}

func (out *OutStreamIMP) WriteC64(value uint64) error {
	return clownfish.TrapErr(func() {
		self := (*C.lucy_OutStream)(clownfish.Unwrap(out, "out"))
		C.LUCY_OutStream_Write_C64(self, C.uint64_t(value))
	})
}

func (out *OutStreamIMP) WriteF32(value float32) error {
	return clownfish.TrapErr(func() {
		self := (*C.lucy_OutStream)(clownfish.Unwrap(out, "out"))
		C.LUCY_OutStream_Write_F32(self, C.float(value))
	})
}

func (out *OutStreamIMP) WriteF64(value float64) error {
	return clownfish.TrapErr(func() {
		self := (*C.lucy_OutStream)(clownfish.Unwrap(out, "out"))
		C.LUCY_OutStream_Write_F64(self, C.double(value))
	})
}

func (out *OutStreamIMP) Absorb(in InStream) error {
	return clownfish.TrapErr(func() {
		self := (*C.lucy_OutStream)(clownfish.Unwrap(out, "out"))
		inStreamC := (*C.lucy_InStream)(clownfish.Unwrap(in, "in"))
		C.LUCY_OutStream_Absorb(self, inStreamC)
	})
}

func (f *FolderIMP) Initialize() error {
	return clownfish.TrapErr(func() {
		self := (*C.lucy_Folder)(clownfish.Unwrap(f, "f"))
		C.LUCY_Folder_Initialize(self)
	})
}

func (f *FolderIMP) OpenOut(path string) (retval OutStream, err error) {
	err = clownfish.TrapErr(func() {
		self := (*C.lucy_Folder)(clownfish.Unwrap(f, "f"))
		pathC := (*C.cfish_String)(clownfish.GoToClownfish(path, unsafe.Pointer(C.CFISH_STRING), false))
		defer C.cfish_decref(unsafe.Pointer(pathC))
		retvalC := C.LUCY_Folder_Open_Out(self, pathC)
		if retvalC == nil {
			cfErr := C.cfish_Err_get_error();
			panic(clownfish.WRAPAny(unsafe.Pointer(C.cfish_incref(unsafe.Pointer(cfErr)))).(error))
		}
		retval = WRAPOutStream(unsafe.Pointer(retvalC))
	})
	return retval, err
}

func (f *FolderIMP) OpenIn(path string) (retval InStream, err error) {
	err = clownfish.TrapErr(func() {
		self := (*C.lucy_Folder)(clownfish.Unwrap(f, "f"))
		pathC := (*C.cfish_String)(clownfish.GoToClownfish(path, unsafe.Pointer(C.CFISH_STRING), false))
		defer C.cfish_decref(unsafe.Pointer(pathC))
		retvalC := C.LUCY_Folder_Open_In(self, pathC)
		if retvalC == nil {
			cfErr := C.cfish_Err_get_error();
			panic(clownfish.WRAPAny(unsafe.Pointer(C.cfish_incref(unsafe.Pointer(cfErr)))).(error))
		}
		retval = WRAPInStream(unsafe.Pointer(retvalC))
	})
	return retval, err
}

func (f *FolderIMP) OpenFileHandle(path string, flags uint32) (retval FileHandle, err error) {
	err = clownfish.TrapErr(func() {
		self := (*C.lucy_Folder)(clownfish.Unwrap(f, "f"))
		pathC := (*C.cfish_String)(clownfish.GoToClownfish(path, unsafe.Pointer(C.CFISH_STRING), false))
		defer C.cfish_decref(unsafe.Pointer(pathC))
		retvalC := C.LUCY_Folder_Open_FileHandle(self, pathC, C.uint32_t(flags))
		if retvalC == nil {
			cfErr := C.cfish_Err_get_error();
			panic(clownfish.WRAPAny(unsafe.Pointer(C.cfish_incref(unsafe.Pointer(cfErr)))).(error))
		}
		retval = WRAPFileHandle(unsafe.Pointer(retvalC))
	})
	return retval, err
}

func (f *FolderIMP) OpenDir(path string) (retval DirHandle, err error) {
	err = clownfish.TrapErr(func() {
		self := (*C.lucy_Folder)(clownfish.Unwrap(f, "f"))
		pathC := (*C.cfish_String)(clownfish.GoToClownfish(path, unsafe.Pointer(C.CFISH_STRING), false))
		defer C.cfish_decref(unsafe.Pointer(pathC))
		retvalC := C.LUCY_Folder_Open_Dir(self, pathC)
		if retvalC == nil {
			cfErr := C.cfish_Err_get_error();
			panic(clownfish.WRAPAny(unsafe.Pointer(C.cfish_incref(unsafe.Pointer(cfErr)))).(error))
		}
		retval = WRAPDirHandle(unsafe.Pointer(retvalC))
	})
	return retval, err
}

func (f *FolderIMP) MkDir(path string) error {
	self := (*C.lucy_Folder)(clownfish.Unwrap(f, "f"))
	pathC := (*C.cfish_String)(clownfish.GoToClownfish(path, unsafe.Pointer(C.CFISH_STRING), false))
	defer C.cfish_decref(unsafe.Pointer(pathC))
	success := C.LUCY_Folder_MkDir(self, pathC)
	if !success {
		cfErr := C.cfish_Err_get_error();
		return clownfish.WRAPAny(unsafe.Pointer(C.cfish_incref(unsafe.Pointer(cfErr)))).(error)
	}
	return nil
}

func (f *FolderIMP) List(path string) (retval []string, err error) {
	err = clownfish.TrapErr(func() {
		self := (*C.lucy_Folder)(clownfish.Unwrap(f, "f"))
		pathC := (*C.cfish_String)(clownfish.GoToClownfish(path, unsafe.Pointer(C.CFISH_STRING), false))
		defer C.cfish_decref(unsafe.Pointer(pathC))
		retvalC := C.LUCY_Folder_List(self, pathC)
		if retvalC == nil {
			cfErr := C.cfish_Err_get_error();
			panic(clownfish.WRAPAny(unsafe.Pointer(C.cfish_incref(unsafe.Pointer(cfErr)))).(error))
		}
		defer C.cfish_decref(unsafe.Pointer(retvalC))
		retval = vecToStringSlice(retvalC)
	})
	return retval, err
}

func (f *FolderIMP) ListR(path string) (retval []string, err error) {
	err = clownfish.TrapErr(func() {
		self := (*C.lucy_Folder)(clownfish.Unwrap(f, "f"))
		pathC := (*C.cfish_String)(clownfish.GoToClownfish(path, unsafe.Pointer(C.CFISH_STRING), false))
		defer C.cfish_decref(unsafe.Pointer(pathC))
		retvalC := C.LUCY_Folder_List_R(self, pathC)
		if retvalC == nil {
			cfErr := C.cfish_Err_get_error();
			panic(clownfish.WRAPAny(unsafe.Pointer(C.cfish_incref(unsafe.Pointer(cfErr)))).(error))
		}
		defer C.cfish_decref(unsafe.Pointer(retvalC))
		retval = vecToStringSlice(retvalC)
	})
	return retval, err
}

func (f *FolderIMP) Rename(from, to string) error {
	self := (*C.lucy_Folder)(clownfish.Unwrap(f, "f"))
	fromC := (*C.cfish_String)(clownfish.GoToClownfish(from, unsafe.Pointer(C.CFISH_STRING), false))
	toC := (*C.cfish_String)(clownfish.GoToClownfish(to, unsafe.Pointer(C.CFISH_STRING), false))
	defer C.cfish_decref(unsafe.Pointer(fromC))
	defer C.cfish_decref(unsafe.Pointer(toC))
	success := C.LUCY_Folder_Rename(self, fromC, toC)
	if !success {
		cfErr := C.cfish_Err_get_error();
		return clownfish.WRAPAny(unsafe.Pointer(C.cfish_incref(unsafe.Pointer(cfErr)))).(error)
	}
	return nil
}

func (f *FolderIMP) HardLink(from, to string) error {
	self := (*C.lucy_Folder)(clownfish.Unwrap(f, "f"))
	fromC := (*C.cfish_String)(clownfish.GoToClownfish(from, unsafe.Pointer(C.CFISH_STRING), false))
	toC := (*C.cfish_String)(clownfish.GoToClownfish(to, unsafe.Pointer(C.CFISH_STRING), false))
	defer C.cfish_decref(unsafe.Pointer(fromC))
	defer C.cfish_decref(unsafe.Pointer(toC))
	success := C.LUCY_Folder_Hard_Link(self, fromC, toC)
	if !success {
		cfErr := C.cfish_Err_get_error();
		return clownfish.WRAPAny(unsafe.Pointer(C.cfish_incref(unsafe.Pointer(cfErr)))).(error)
	}
	return nil
}

func (f *FolderIMP) SlurpFile(path string) (retval []byte, err error) {
	err = clownfish.TrapErr(func() {
		self := (*C.lucy_Folder)(clownfish.Unwrap(f, "f"))
		pathC := (*C.cfish_String)(clownfish.GoToClownfish(path, unsafe.Pointer(C.CFISH_STRING), false))
		defer C.cfish_decref(unsafe.Pointer(pathC))
		retvalC := C.LUCY_Folder_Slurp_File(self, pathC)
		defer C.cfish_decref(unsafe.Pointer(retvalC))
		retval = clownfish.ToGo(unsafe.Pointer(retvalC)).([]byte)
	})
	return retval, err
}

func (f *FolderIMP) Consolidate(path string) error {
	return clownfish.TrapErr(func() {
		self := (*C.lucy_Folder)(clownfish.Unwrap(f, "f"))
		pathC := (*C.cfish_String)(clownfish.GoToClownfish(path, unsafe.Pointer(C.CFISH_STRING), false))
		defer C.cfish_decref(unsafe.Pointer(pathC))
		C.LUCY_Folder_Consolidate(self, pathC)
	})
}

func (f *FolderIMP) LocalOpenIn(name string) (retval InStream, err error) {
	err = clownfish.TrapErr(func() {
		self := (*C.lucy_Folder)(clownfish.Unwrap(f, "f"))
		nameC := (*C.cfish_String)(clownfish.GoToClownfish(name, unsafe.Pointer(C.CFISH_STRING), false))
		defer C.cfish_decref(unsafe.Pointer(nameC))
		retvalC := C.LUCY_Folder_Local_Open_In(self, nameC)
		if retvalC == nil {
			cfErr := C.cfish_Err_get_error();
			panic(clownfish.WRAPAny(unsafe.Pointer(C.cfish_incref(unsafe.Pointer(cfErr)))).(error))
		}
		retval = WRAPInStream(unsafe.Pointer(retvalC))
	})
	return retval, err
}

func (f *FolderIMP) LocalOpenFileHandle(name string, flags uint32) (retval FileHandle, err error) {
	err = clownfish.TrapErr(func() {
		self := (*C.lucy_Folder)(clownfish.Unwrap(f, "f"))
		nameC := (*C.cfish_String)(clownfish.GoToClownfish(name, unsafe.Pointer(C.CFISH_STRING), false))
		defer C.cfish_decref(unsafe.Pointer(nameC))
		retvalC := C.LUCY_Folder_Local_Open_FileHandle(self, nameC, C.uint32_t(flags))
		if retvalC == nil {
			cfErr := C.cfish_Err_get_error();
			panic(clownfish.WRAPAny(unsafe.Pointer(C.cfish_incref(unsafe.Pointer(cfErr)))).(error))
		}
		retval = WRAPFileHandle(unsafe.Pointer(retvalC))
	})
	return retval, err
}

func (f *FolderIMP) LocalOpenDir() (retval DirHandle, err error) {
	err = clownfish.TrapErr(func() {
		self := (*C.lucy_Folder)(clownfish.Unwrap(f, "f"))
		retvalC := C.LUCY_Folder_Local_Open_Dir(self)
		if retvalC == nil {
			cfErr := C.cfish_Err_get_error();
			panic(clownfish.WRAPAny(unsafe.Pointer(C.cfish_incref(unsafe.Pointer(cfErr)))).(error))
		}
		retval = WRAPDirHandle(unsafe.Pointer(retvalC))
	})
	return retval, err
}

func (f *FolderIMP) LocalMkDir(name string) error {
	self := (*C.lucy_Folder)(clownfish.Unwrap(f, "f"))
	nameC := (*C.cfish_String)(clownfish.GoToClownfish(name, unsafe.Pointer(C.CFISH_STRING), false))
	defer C.cfish_decref(unsafe.Pointer(nameC))
	success := C.LUCY_Folder_Local_MkDir(self, nameC)
	if !success {
		cfErr := C.cfish_Err_get_error();
		return clownfish.WRAPAny(unsafe.Pointer(C.cfish_incref(unsafe.Pointer(cfErr)))).(error)
	}
	return nil
}

func (fh *FileHandleIMP) Write(data []byte, size int) error {
	return clownfish.TrapErr(func() {
		self := (*C.lucy_FileHandle)(clownfish.Unwrap(fh, "fh"))
		if size > len(data) {
			panic(clownfish.NewErr(fmt.Sprintf("Buf only %d long, can't write %d bytes",
				len(data), size)))
		}
		octets := C.CString(string(data))
		defer C.free(unsafe.Pointer(octets))
		C.LUCY_FH_Write(self, unsafe.Pointer(octets), C.size_t(size))
	})
}

func (fh *FileHandleIMP) Read(b []byte, offset int64, length int) error {
	return clownfish.TrapErr(func() {
		self := (*C.lucy_FileHandle)(clownfish.Unwrap(fh, "fh"))
		buf := (*C.char)(C.malloc(C.size_t(length)))
		defer C.free(unsafe.Pointer(buf))
		C.LUCY_FH_Read(self, buf, C.int64_t(offset), C.size_t(length))
		dupe := []byte(C.GoStringN(buf, C.int(length)))
		for i := 0; i < length; i++ {
			b[i] = dupe[i]
		}
	})
}

func (fh *FileHandleIMP) Grow(length int64) error {
	return clownfish.TrapErr(func() {
		self := (*C.lucy_FileHandle)(clownfish.Unwrap(fh, "fh"))
		C.LUCY_FH_Grow(self, C.int64_t(length))
	})
}

func (fh *FileHandleIMP) Window(window FileWindow, offset, length int64) error {
	self := (*C.lucy_FileHandle)(clownfish.Unwrap(fh, "fh"))
	windowC := (*C.lucy_FileWindow)(clownfish.Unwrap(window, "window"))
	success := C.LUCY_FH_Window(self, windowC, C.int64_t(offset), C.int64_t(length))
	if !success {
		cfErr := C.cfish_Err_get_error();
		return clownfish.WRAPAny(unsafe.Pointer(C.cfish_incref(unsafe.Pointer(cfErr)))).(error)
	}
	return nil
}

func (fh *FileHandleIMP) ReleaseWindow(window FileWindow) error {
	self := (*C.lucy_FileHandle)(clownfish.Unwrap(fh, "fh"))
	windowC := (*C.lucy_FileWindow)(clownfish.Unwrap(window, "window"))
	success := C.LUCY_FH_Release_Window(self, windowC)
	if !success {
		cfErr := C.cfish_Err_get_error();
		return clownfish.WRAPAny(unsafe.Pointer(C.cfish_incref(unsafe.Pointer(cfErr)))).(error)
	}
	return nil
}

func (fh *FileHandleIMP) Close() error {
	return clownfish.TrapErr(func() {
		self := (*C.lucy_FileHandle)(clownfish.Unwrap(fh, "fh"))
		C.LUCY_FH_Close(self)
	})
}

func OpenFSFileHandle(path string, flags uint32) (fh FSFileHandle, err error) {
	err = clownfish.TrapErr(func() {
		pathC := (*C.cfish_String)(clownfish.GoToClownfish(path, unsafe.Pointer(C.CFISH_STRING), false))
		defer C.cfish_decref(unsafe.Pointer(pathC))
		cfObj := C.lucy_FSFH_open(pathC, C.uint32_t(flags))
		if cfObj == nil {
			cfErr := C.cfish_Err_get_error();
			panic(clownfish.WRAPAny(unsafe.Pointer(C.cfish_incref(unsafe.Pointer(cfErr)))).(error))
		}
		fh = WRAPFSFileHandle(unsafe.Pointer(cfObj))
	})
	return fh, err
}

func OpenRAMFileHandle(path string, flags uint32, ramFile RAMFile) (fh RAMFileHandle, err error) {
	err = clownfish.TrapErr(func() {
		pathC := (*C.cfish_String)(clownfish.GoToClownfish(path, unsafe.Pointer(C.CFISH_STRING), false))
		ramFileC := (*C.lucy_RAMFile)(clownfish.GoToClownfish(ramFile, unsafe.Pointer(C.LUCY_RAMFILE), true))
		defer C.cfish_decref(unsafe.Pointer(pathC))
		cfObj := C.lucy_RAMFH_open(pathC, C.uint32_t(flags), ramFileC)
		if cfObj == nil {
			cfErr := C.cfish_Err_get_error();
			panic(clownfish.WRAPAny(unsafe.Pointer(C.cfish_incref(unsafe.Pointer(cfErr)))).(error))
		}
		fh = WRAPRAMFileHandle(unsafe.Pointer(cfObj))
	})
	return fh, err
}

func (dh *DirHandleIMP) Error() error {
	return dh.err
}

func (dh *DirHandleIMP) next() bool {
	var retval bool
	dh.err = clownfish.TrapErr(func() {
		self := (*C.lucy_DirHandle)(clownfish.Unwrap(dh, "dh"))
		retval = bool(C.LUCY_DH_Next(self))
	})
	if dh.err != nil {
		return false
	}
	return retval
}

func (dh *DirHandleIMP) Close() error {
	return clownfish.TrapErr(func() {
		self := (*C.lucy_DirHandle)(clownfish.Unwrap(dh, "dh"))
		C.LUCY_DH_Close(self)
	})
}

func OpenFSDirHandle(path string) (dh FSDirHandle, err error) {
	err = clownfish.TrapErr(func() {
		pathC := (*C.cfish_String)(clownfish.GoToClownfish(path, unsafe.Pointer(C.CFISH_STRING), false))
		defer C.cfish_decref(unsafe.Pointer(pathC))
		cfObj := C.lucy_FSDH_open(pathC)
		if cfObj == nil {
			cfErr := C.cfish_Err_get_error();
			panic(clownfish.WRAPAny(unsafe.Pointer(C.cfish_incref(unsafe.Pointer(cfErr)))).(error))
		}
		dh = WRAPFSDirHandle(unsafe.Pointer(cfObj))
	})
	return dh, err
}

func (lock *LockIMP) Request() error {
	self := (*C.lucy_Lock)(clownfish.Unwrap(lock, "lock"))
	success := C.LUCY_Lock_Request(self)
	if !success {
		cfErr := C.cfish_Err_get_error();
		return clownfish.WRAPAny(unsafe.Pointer(C.cfish_incref(unsafe.Pointer(cfErr)))).(error)
	}
	return nil
}

func (lock *LockIMP) Obtain() error {
	self := (*C.lucy_Lock)(clownfish.Unwrap(lock, "lock"))
	success := C.LUCY_Lock_Obtain(self)
	if !success {
		cfErr := C.cfish_Err_get_error();
		return clownfish.WRAPAny(unsafe.Pointer(C.cfish_incref(unsafe.Pointer(cfErr)))).(error)
	}
	return nil
}

func (lock *LockIMP) Release() error {
	return clownfish.TrapErr(func() {
		self := (*C.lucy_Lock)(clownfish.Unwrap(lock, "lock"))
		C.LUCY_Lock_Release(self)
	})
}


func (lock *LockIMP) ClearStale() error {
	return clownfish.TrapErr(func() {
		self := (*C.lucy_Lock)(clownfish.Unwrap(lock, "lock"))
		C.LUCY_Lock_Clear_Stale(self)
	})
}

func OpenCompoundFileReader(folder Folder) (reader CompoundFileReader, err error) {
	err = clownfish.TrapErr(func() {
		folderC := (*C.lucy_Folder)(clownfish.Unwrap(folder, "Folder"))
		cfObj := C.lucy_CFReader_open(folderC)
		if cfObj == nil {
			cfErr := C.cfish_Err_get_error();
			panic(clownfish.WRAPAny(unsafe.Pointer(C.cfish_incref(unsafe.Pointer(cfErr)))).(error))
		}
		reader = WRAPCompoundFileReader(unsafe.Pointer(cfObj))
	})
	return reader, err
}

func (writer *CompoundFileWriterIMP) Consolidate() error {
	return clownfish.TrapErr(func() {
		self := (*C.lucy_CompoundFileWriter)(clownfish.Unwrap(writer, "writer"))
		C.LUCY_CFWriter_Consolidate(self)
	})
}
