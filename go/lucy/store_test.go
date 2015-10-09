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

import "testing"
import "reflect"
import "os"

import "git-wip-us.apache.org/repos/asf/lucy-clownfish.git/runtime/go/clownfish"

func TestRAMFileBasics(t *testing.T) {
	fooBytes := []byte("foo")
	contents := clownfish.NewByteBuf(5)
	contents.Cat(fooBytes)
	ramFile := NewRAMFile(contents, false)

	if ramFile.ReadOnly() {
		t.Error("ReadOnly")
	}
	ramFile.SetReadOnly(true)
	if !ramFile.ReadOnly() {
		t.Error("SetReadOnly/ReadOnly")
	}

	if got := ramFile.GetContents().YieldBlob(); !reflect.DeepEqual(got, fooBytes) {
		t.Errorf("GetContents: %v", got)
	}
}

func TestIOStreamOpenClose(t *testing.T) {
	var outStream OutStream
	var inStream InStream
	var err error

	ramFile := NewRAMFile(nil, false)
	outStream, err = OpenOutStream(ramFile)
	if err != nil {
		t.Errorf("OpenOutStream(RAMFile): %s", err)
	}
	err = outStream.Close()
	if err != nil {
		t.Errorf("OutStream(RAMFile).Close(): %s", err)
	}
	inStream, err = OpenInStream(ramFile)
	if err != nil {
		t.Errorf("OpenInStream(RAMFile): %s", err)
	}
	err = inStream.Close()
	if err != nil {
		t.Errorf("InStream(RAMFile).Close(): %s", err)
	}

	path := "_iostream_open_test"
	defer os.Remove(path)
	outStream, err = OpenOutStream(path)
	if err != nil {
		t.Errorf("OpenOutStream(string): %s", err)
	}
	err = outStream.Close()
	if err != nil {
		t.Errorf("OutStream(string).Close(): %s", err)
	}
	inStream, err = OpenInStream(path)
	if err != nil {
		t.Errorf("OpenInStream(string): %s", err)
	}
	err = inStream.Close()
	if err != nil {
		t.Errorf("InStream(string).Close(): %s", err)
	}

	outStream, err = OpenOutStream(42)
	if err == nil || outStream != nil {
		t.Errorf("OpenOutStream(number) should fail: %v, %s", outStream, err)
	}
	inStream, err = OpenInStream(42)
	if err == nil || inStream != nil {
		t.Errorf("OpenInStream(number) should fail: %v, %s", inStream, err)
	}

	outStream, err = OpenOutStream(nil)
	if err == nil || outStream != nil {
		t.Errorf("OpenOutStream(nil) should fail: %v, %s", outStream, err)
	}
	inStream, err = OpenInStream(nil)
	if err == nil || inStream != nil {
		t.Errorf("OpenInStream(nil) should fail: %v, %s", inStream, err)
	}
}

func TestIOStreamDupe(t *testing.T) {
	var err error
	file := NewRAMFile(nil, false)
	outStream, _ := OpenOutStream(file)
	for i := 0; i < 100; i++ {
		outStream.WriteU8(uint8(i + 100))
	}
	outStream.Close()

	inStream, _ := OpenInStream(file)
	inStream.Seek(50)
	if got, err := inStream.ReadU8(); got != 150 || err != nil {
		t.Errorf("ReadU8: %d, %v", got, err)
	}

	clone := inStream.Clone().(InStream)
	if got, err := clone.ReadU8(); got != 151 || err != nil {
		t.Errorf("Clone had unexpected read: %d, %v", got, err)
	}

	dupe, err := inStream.Reopen("foo.dat", 99, 1)
	if fileName := dupe.GetFilename(); fileName != "foo.dat" {
		t.Errorf("Reopen filename: %s", fileName)
	}
	if err != nil {
		t.Errorf("Bad Reopen: %v", err)
	}
	if got, err := dupe.ReadU8(); got != 199 || err != nil {
		t.Errorf("Reopen had unexpected read: %d, %v", got, err)
	}
}

func TestIOStreamFilePos(t *testing.T) {
	var err error
	file := NewRAMFile(nil, false)
	outStream, _ := OpenOutStream(file)

	err = outStream.WriteI32(42)
	if err != nil {
		t.Errorf("WriteI32 error: %v", err)
	}
	if got := outStream.Tell(); got != 4 {
		t.Errorf("OutStream.Tell: %d", got)
	}

	err = outStream.Grow(32)
	if err != nil {
		t.Errorf("Grow error: %v", err)
	}
	if got := outStream.Length(); got != 4 {
		t.Errorf("Grow/Length: %d", got)
	}

	err = outStream.Align(8)
	if err != nil {
		t.Errorf("Align error: %v", err)
	}
	if got := outStream.Tell(); got != 8 {
		t.Errorf("Align/Tell: %d", got)
	}

	outStream.Close()
	inStream, _ := OpenInStream(file)
	if got := inStream.Length(); got != 8 {
		t.Errorf("InStream.Length: %d", got)
	}

	err = inStream.Seek(4)
	if err != nil {
	}
	if got := inStream.Tell(); got != 4 {
		t.Errorf("InStream.Tell: %d", got)
	}

	err = inStream.Seek(30)
	if err == nil {
		t.Error("Out of bounds seek should fail")
	}
}

func TestIOStreamReadWrite(t *testing.T) {
	var err error
	file := NewRAMFile(nil, false)
	outStream, _ := OpenOutStream(file)

	err = outStream.WriteString("foo")
	if err != nil {
		t.Errorf("WriteString error: %v", err)
	}
	err = outStream.WriteBytes([]byte("bar"), 3)
	if err != nil {
		t.Errorf("WriteBytes error: %v", err)
	}
	if err = outStream.WriteI8(42); err != nil {
		t.Errorf("WriteI8: %s", err)
	}
	if err = outStream.WriteI32(42); err != nil {
		t.Errorf("WriteI32: %s", err)
	}
	if err = outStream.WriteI64(42); err != nil {
		t.Errorf("WriteI64: %s", err)
	}
	if err = outStream.WriteU8(42); err != nil {
		t.Errorf("WriteU8: %s", err)
	}
	if err = outStream.WriteU32(42); err != nil {
		t.Errorf("WriteU32: %s", err)
	}
	if err = outStream.WriteU64(42); err != nil {
		t.Errorf("WriteU64: %s", err)
	}
	if err = outStream.WriteC32(42); err != nil {
		t.Errorf("WriteC32: %s", err)
	}
	if err = outStream.WriteC64(42); err != nil {
		t.Errorf("WriteC64: %s", err)
	}
	if err = outStream.WriteF32(1.5); err != nil {
		t.Errorf("WriteF32: %s", err)
	}
	if err = outStream.WriteF64(1.5); err != nil {
		t.Errorf("WriteF64: %s", err)
	}
	barContents := clownfish.NewByteBuf(5)
	barContents.Cat([]byte{3, 'b', 'a', 'r'})
	barInStream, _ := OpenInStream(NewRAMFile(barContents, true))
	if err = outStream.Absorb(barInStream); err != nil {
		t.Errorf("Aborb: %s", err)
	}

	outStream.Close()
	inStream, _ := OpenInStream(file)

	if got, err := inStream.ReadString(); got != "foo" || err != nil {
		t.Errorf("WriteString/ReadString: %s, %v", got, err)
	}
	buf := make([]byte, 3)
	err = inStream.ReadBytes(buf, 3)
	if !reflect.DeepEqual(buf, []byte("bar")) || err != nil {
		t.Errorf("WriteBytes/ReadBytes: %v, %v", buf, err)
	}
	if got, err := inStream.ReadI8(); got != 42 || err != nil {
		t.Errorf("ReadI8: %d, %s", got, err)
	}
	if got, err := inStream.ReadI32(); got != 42 || err != nil {
		t.Errorf("ReadI32: %d, %s", got, err)
	}
	if got, err := inStream.ReadI64(); got != 42 || err != nil {
		t.Errorf("ReadI64: %d, %s", got, err)
	}
	if got, err := inStream.ReadU8(); got != 42 || err != nil {
		t.Errorf("ReadU8: %d, %s", got, err)
	}
	if got, err := inStream.ReadU32(); got != 42 || err != nil {
		t.Errorf("ReadU32: %d, %s", got, err)
	}
	if got, err := inStream.ReadU64(); got != 42 || err != nil {
		t.Errorf("ReadU64: %d, %s", got, err)
	}
	if got, err := inStream.ReadC32(); got != 42 || err != nil {
		t.Errorf("ReadC32: %d, %s", got, err)
	}
	if got, err := inStream.ReadC64(); got != 42 || err != nil {
		t.Errorf("ReadC64: %d, %s", got, err)
	}
	if got, err := inStream.ReadF32(); got != 1.5 || err != nil {
		t.Errorf("ReadF32: %d, %s", got, err)
	}
	if got, err := inStream.ReadF64(); got != 1.5 || err != nil {
		t.Errorf("ReadF64: %d, %s", got, err)
	}
	if got, err := inStream.ReadString(); got != "bar" || err != nil {
		t.Errorf("WriteString/ReadString: %s, %v", got, err)
	}
}

func TestIOStreamMisc(t *testing.T) {
	folder := NewRAMFolder("mydir")
	outStream := folder.OpenOut("file.dat")
	if got := outStream.GetPath(); got != "mydir/file.dat" {
		t.Errorf("GetPath: %s", got)
	}
	outStream.WriteU32(1)
	outStream.Flush()
	outStream.Close()

	inStream := folder.OpenIn("file.dat")
	if got := inStream.GetFilename(); got != "mydir/file.dat" {
		t.Errorf("GetFilename: %s", got)
	}
}
