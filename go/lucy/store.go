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
#include "Lucy/Store/InStream.h"
#include "Lucy/Store/OutStream.h"

#include "Clownfish/Err.h"
*/
import "C"
import "unsafe"
import "fmt"

import "git-wip-us.apache.org/repos/asf/lucy-clownfish.git/runtime/go/clownfish"

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
