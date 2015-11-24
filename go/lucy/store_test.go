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

import _ "git-wip-us.apache.org/repos/asf/lucy-clownfish.git/runtime/go/clownfish"

func TestRAMFileBasics(t *testing.T) {
	/*
	fooBytes := []byte("foo")
	contents := clownfish.NewByteBuf(5)
	contents.Cat(fooBytes)
	ramFile := NewRAMFile(contents, false)

	if ramFile.readOnly() {
		t.Error("readOnly")
	}
	ramFile.setReadOnly(true)
	if !ramFile.readOnly() {
		t.Error("setReadOnly/readOnly")
	}

	if got := ramFile.getContents().YieldBlob(); !reflect.DeepEqual(got, fooBytes) {
		t.Errorf("GetContents: %v", got)
	}
	*/
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
	if fileName := dupe.getFilename(); fileName != "foo.dat" {
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
	if got := outStream.tell(); got != 4 {
		t.Errorf("OutStream.tell: %d", got)
	}

	err = outStream.Grow(32)
	if err != nil {
		t.Errorf("Grow error: %v", err)
	}
	if got := outStream.length(); got != 4 {
		t.Errorf("Grow/Length: %d", got)
	}

	err = outStream.Align(8)
	if err != nil {
		t.Errorf("Align error: %v", err)
	}
	if got := outStream.tell(); got != 8 {
		t.Errorf("Align/tell: %d", got)
	}

	outStream.Close()
	inStream, _ := OpenInStream(file)
	if got := inStream.length(); got != 8 {
		t.Errorf("InStream.length: %d", got)
	}

	err = inStream.Seek(4)
	if err != nil {
	}
	if got := inStream.tell(); got != 4 {
		t.Errorf("InStream.tell: %d", got)
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
	/*
	barContents := clownfish.NewByteBuf(5)
	barContents.Cat([]byte{3, 'b', 'a', 'r'})
	barInStream, _ := OpenInStream(NewRAMFile(barContents, true))
	if err = outStream.Absorb(barInStream); err != nil {
		t.Errorf("Aborb: %s", err)
	}
	*/

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
	/*
	if got, err := inStream.ReadString(); got != "bar" || err != nil {
		t.Errorf("WriteString/ReadString: %s, %v", got, err)
	}
	*/
}

func TestIOStreamMisc(t *testing.T) {
	folder := NewRAMFolder("mydir")
	outStream, _ := folder.OpenOut("file.dat")
	if got := outStream.getPath(); got != "mydir/file.dat" {
		t.Errorf("getPath: %s", got)
	}
	outStream.WriteU32(1)
	outStream.flush()
	outStream.Close()

	inStream, _ := folder.OpenIn("file.dat")
	if got := inStream.getFilename(); got != "mydir/file.dat" {
		t.Errorf("getFilename: %s", got)
	}
}

func runFolderTests(t *testing.T, folder Folder) {
	var err error

	err = folder.Initialize()
	if err != nil {
		t.Errorf("Initialize: %v", err)
	}
	if !folder.check() {
		t.Errorf("check")
	}

	if problem := folder.MkDir("stuff"); problem != nil {
		t.Errorf("MkDir: %v", problem)
	}
	outStream, err := folder.OpenOut("stuff/hello")
	if outStream == nil || err != nil {
		t.Errorf("OpenOut: %v", err)
	}
	outStream.WriteBytes([]byte("hi"), 2)
	outStream.Close()
	inStream, err := folder.OpenIn("stuff/hello")
	if inStream == nil || err != nil {
		t.Errorf("OpenIn: %s", err)
	}
	inStream.Close()
	fh, err := folder.OpenFileHandle("stuff/hello", 0x1) // 0x1 == FH_READ_ONLY
	if fh == nil || err != nil {
		t.Errorf("OpenFileHandle: %v", err)
	}
	fh.Close()
	dh, err := folder.OpenDir("stuff")
	if dh == nil || err != nil {
		t.Errorf("OpenDir: %v", err)
	}
	dh.Close()

	if got, err := folder.SlurpFile("stuff/hello"); !reflect.DeepEqual(got, []byte("hi")) || err != nil {
		t.Errorf("SlurpFile: %v, %v", got, err)
	}
	if got, err := folder.SlurpFile("nope/nyet/nada"); got != nil || err == nil {
		t.Errorf("SlurpFile [non-existent file]: %v, %v", got, err)
	}

	if !folder.exists("stuff") {
		t.Errorf("Exists [directory]")
	}
	if !folder.exists("stuff/hello") {
		t.Errorf("Exists [file]")
	}
	if folder.exists("stuff/nope") {
		t.Errorf("Exists [non-existent entry]")
	}

	if !folder.isDirectory("stuff") {
		t.Errorf("isDirectory [directory]")
	}
	if folder.isDirectory("stuff/hello") {
		t.Errorf("isDirectory [file]")
	}
	if folder.isDirectory("nope") {
		t.Errorf("isDirectory [non-existent entry]")
	}

	listExpected := []string{"stuff"}
	if got, err := folder.List(""); !reflect.DeepEqual(got, listExpected) || err != nil {
		t.Errorf("Unexpected result for List: %v, %v", got, err)
	}
	listRExpected := []string{"stuff", "stuff/hello"}
	if got, err := folder.ListR(""); !reflect.DeepEqual(got, listRExpected) || err != nil {
		t.Errorf("Unexpected result for ListR: %v, %v", got, err)
	}
	if stuff := folder.findFolder("stuff"); stuff == nil {
		t.Errorf("findFolder")
	}
	if nope := folder.findFolder("nope"); nope != nil {
		t.Errorf("findFolder [non-existent]")
	}
	if stuff := folder.enclosingFolder("stuff/hello"); stuff == nil {
		t.Errorf("enclosingFolder")
	}
	if nope := folder.enclosingFolder("nada/nope/nyet"); nope != nil {
		t.Errorf("enclosingFolder [non-existent]")
	}

	if err := folder.HardLink("stuff/hello", "aloha"); err != nil {
		t.Errorf("HardLink: %v", err)
	}
	if err := folder.Rename("stuff/hello", "stuff/hola"); err != nil {
		t.Errorf("Rename: %v", err)
	}
	if success := folder.delete("stuff/hola"); !success {
		t.Errorf("delete")
	}

	if fh, err := folder.LocalOpenFileHandle("aloha", 0x1); fh == nil || err != nil {
		t.Errorf("LocalOpenFileHandle: %v", err)
	}
	if in, err := folder.LocalOpenIn("aloha"); in == nil || err != nil {
		t.Errorf("LocalOpenIn: %v", err)
	} else {
		in.Close()
	}
	if err = folder.LocalMkDir("things"); err != nil {
		t.Errorf("LocalMkdir: %v", err)
	}
	if !folder.localExists("things") {
		t.Errorf("localExists")
	}
	if !folder.localIsDirectory("things") {
		t.Errorf("localIsDirectory")
	}
	if things := folder.localFindFolder("things"); things == nil {
		t.Errorf("localFindFolder")
	}
	if dh, err := folder.LocalOpenDir(); dh == nil || err != nil {
		t.Errorf("LocalOpenDir: %v", err)
	} else {
		dh.Close()
	}
	if !folder.localDelete("things") {
		t.Errorf("localDelete")
	}

	folder.Consolidate("stuff")

	if success := folder.deleteTree("stuff"); !success {
		t.Errorf("deleteTree")
	}

	folder.close()
}

func TestRAMFolderBasics(t *testing.T) {
	folder := NewRAMFolder("orig")
	if folder.getPath() != "orig" {
		t.Error("getPath")
	}
	folder.setPath("basedir")
	if folder.getPath() != "basedir" {
		t.Error("setPath/getPath")
	}
	runFolderTests(t, folder)
}

func TestFSFolderBasics(t *testing.T) {
	folder := NewFSFolder("_fsfolder_go_test")
	defer os.RemoveAll("_fsfolder_go_test")
	runFolderTests(t, folder)
}

func TestFileWindowBasics(t *testing.T) {
	window := NewFileWindow()
	window.setOffset(30)
	if got := window.getOffset(); got != 30 {
		t.Errorf("setOffset/getOffset: %d", got)
	}
	if got := window.getLen(); got != 0 {
		t.Errorf("getLen: %d", got)
	}
}

func runFileHandleCommonTests(t *testing.T, makeFH func(uint32) FileHandle) {
	var err error
	fh := makeFH(0x2 | 0x4) // FH_WRITE_ONLY | FH_CREATE
	if fh == nil {
		t.Errorf("Failed to open FileHandle for write: %v", err)
		return
	}
	fh.setPath("fake")
	if got := fh.getPath(); got != "fake" {
		t.Errorf("setPath/getPath: %v", got)
	}
	err = fh.Grow(20)
	if err != nil {
		t.Errorf("Grow: %v", err)
	}
	fh.Write([]byte("hello"), 5)
	err = fh.Close()
	if err != nil {
		t.Errorf("Close: %v", err)
	}

	fh = makeFH(0x1) // FH_READ_ONLY
	if fh == nil {
		t.Errorf("Failed to open FileHandle for read: %v", err)
	}
	fh.setPath("fake")
	if got := fh.getPath(); got != "fake" {
		t.Errorf("setPath/getPath: %v", got)
	}
	if got := fh.length(); got != 5 {
		t.Errorf("Unexpected Length: %d", got)
	}
	buf := make([]byte, 3)
	fh.Read(buf, 1, 3)
	if !reflect.DeepEqual(buf, []byte("ell")) {
		t.Errorf("FH read/write: %v", buf)
	}

	window := NewFileWindow()
	err = fh.Window(window, 1, 2)
	if err != nil {
		t.Errorf("Window: %v", err)
	}
	err = fh.Window(window, 1, 2)
	if err != nil {
		t.Errorf("ReleaseWindow: %v", err)
	}

	err = fh.Close()
	if err != nil {
		t.Errorf("Close: %v", err)
	}
}

func TestRAMFileHandleAll(t *testing.T) {
	ramFile := NewRAMFile(nil, false)
	makeFH := func(flags uint32) FileHandle {
		fh, err := OpenRAMFileHandle("content", flags, ramFile)
		if fh == nil || err != nil {
			t.Errorf("OpenRAMFileHandle: %v", err)
		}
		return fh
	}
	runFileHandleCommonTests(t, makeFH)
	fh := makeFH(0x1).(RAMFileHandle) // FH_READ_ONLY
	if _, ok := fh.getFile().(RAMFile); !ok {
		t.Errorf("getFile")
	}
}

func TestFSFileHandleAll(t *testing.T) {
	path := "_fsfilehandle_test"
	defer os.Remove(path)
	makeFH := func(flags uint32) FileHandle {
		fh, err := OpenFSFileHandle(path, flags)
		if fh == nil || err != nil {
			t.Errorf("OpenFSFileHandle: %v", err)
		}
		return fh
	}
	runFileHandleCommonTests(t, makeFH)
}

func runDirHandleCommonTests(t *testing.T, folder Folder, makeDH func() DirHandle) {
	var err error
	err = folder.Initialize()
	if err != nil {
		t.Errorf("Initialize: %v", err)
		return
	}
	err = folder.MkDir("stuff")
	if err != nil {
		t.Errorf("MkDir: %v", err)
		return
	}
	out, err := folder.OpenOut("hello")
	if err != nil {
		t.Errorf("OpenOut: %v", err)
		return
	}
	out.Close()
	if err != nil {
		t.Errorf("Close OutStream: %v", err)
		return
	}

	dh := makeDH()
	if dh == nil {
		t.Errorf("Failed to open DirHandle: %v", err)
		return
	}
	if got := dh.getDir(); got != folder.getPath() {
		t.Errorf("getDir didn't match: '%v' '%v'", got, folder.getPath())
	}
	count := 0
	for dh.next() {
		count += 1
		entry := dh.getEntry()
		switch entry {
		case "hello":
			if dh.entryIsDir() {
				t.Errorf("Entry should not be directory")
			}
			if dh.entryIsSymlink() {
				t.Errorf("File should not be symlink")
			}
		case "stuff":
			if !dh.entryIsDir() {
				t.Errorf("Entry should be directory")
			}
			if dh.entryIsSymlink() {
				t.Errorf("Dir should not be symlink")
			}
		default:
			t.Errorf("Unexpected entry: '%s'", entry)
		}
	}
	if got := dh.Error(); got != nil {
		t.Errorf("Error() should return nil after iteration: %v", got)
	}
	if count != 2 {
		t.Errorf("Didn't get to all entries, found only %d", count)
	}

	err = dh.Close()
	if err != nil {
		t.Errorf("Close: %v", err)
	}
}

func TestRAMDirHandleAll(t *testing.T) {
	folder := NewRAMFolder("myramdir")
	makeDH := func() DirHandle {
		return NewRAMDirHandle(folder)
	}
	runDirHandleCommonTests(t, folder, makeDH)
}

func TestFSDirHandleAll(t *testing.T) {
	path := "_fsdirhandle_go_tests"
	defer os.RemoveAll(path)
	folder := NewFSFolder(path)
	makeDH := func() DirHandle {
		dh, err := OpenFSDirHandle(folder.getPath())
		if err != nil {
			t.Errorf("Failed to open DirHandle: %v", err)
			return nil
		}
		return dh
	}
	runDirHandleCommonTests(t, folder, makeDH)
}

func runLockCommonTests(t *testing.T, makeLock func(string, string) Lock) {
	var err error
	lock := makeLock("foo", "dev.example.com")
	other := makeLock("foo", "dev.example.com")

	if got := lock.getName(); got != "foo" {
		t.Errorf("getName: %v", got)
	}
	if got := lock.getHost(); got != "dev.example.com" {
		t.Errorf("getHost: %v", got)
	}

	err = lock.Request()
	if err != nil {
		t.Errorf("Request: %v", err)
	}
	if !lock.IsLocked() {
		t.Errorf("Lock should be locked, but IsLocked returned false")
	}
	if got := lock.getLockPath(); len(got) == 0 {
		// Lock path only valid when locked for shared locks.
		t.Errorf("getLockPath should work")
	}
	err = other.Request()
	if other.Shared() && err != nil {
		t.Errorf("SharedLock Request should succeed: %v", err)
	} else if !other.Shared() && err == nil {
		t.Errorf("Request should fail for locked resource")
	}
	err = lock.Release()
	if err != nil {
		t.Errorf("Request: %v", err)
	}
	other.Release()
	err = lock.Obtain()
	if err != nil {
		t.Errorf("Obtain: %v", err)
	}

	err = lock.ClearStale()
	if err != nil {
		t.Errorf("Nothing for ClearStale to do, but should still suceed: %v", err)
	}
}

func TestLockFileLockAll(t *testing.T) {
	folder := NewRAMFolder("myindex")
	makeLock := func(name, host string) Lock {
		return NewLockFileLock(folder, name, host, 0, 1)
	}
	runLockCommonTests(t, makeLock)
}

func TestSharedLockAll(t *testing.T) {
	folder := NewRAMFolder("myindex")
	makeLock := func(name, host string) Lock {
		return NewSharedLock(folder, name, host, 0, 1)
	}
	runLockCommonTests(t, makeLock)
}

func TestLockFactoryAll(t *testing.T) {
	folder := NewRAMFolder("")
	factory := NewLockFactory(folder, "dev.example.com")
	lock := factory.MakeLock("write", 10, 42)
	if _, ok := lock.(Lock); !ok {
		t.Errorf("MakeLock")
	}
	shlock := factory.MakeSharedLock("read", 10, 42)
	if _, ok := shlock.(SharedLock); !ok {
		t.Errorf("MakeSharedLock")
	}
}

func TestCompoundFiles(t *testing.T) {
	var err error
	folder := NewRAMFolder("seg_6b")
	out, _ := folder.OpenOut("foo")
	out.WriteI32(42)
	out.Close()

	writer := NewCompoundFileWriter(folder)
	err = writer.Consolidate()
	if err != nil {
		t.Errorf("Consolidate: %v", err)
	}
	err = writer.Consolidate()
	if err == nil {
		t.Errorf("Consolidating twice should fail")
	}

	reader, err := OpenCompoundFileReader(folder)
	if err != nil {
		t.Errorf("OpenCompoundFileReader: %v", err)
	}
	in, err := reader.OpenIn("foo")
	if _, ok := in.(InStream); !ok || err != nil {
		t.Errorf("Failed to open file within compound file: %v", err)
	}
	in.Close()
}
