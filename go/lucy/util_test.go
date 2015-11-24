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
import "git-wip-us.apache.org/repos/asf/lucy-clownfish.git/runtime/go/clownfish"

func TestStepperMisc(t *testing.T) {
	var err error
	stepper := NewTextTermStepper()
	folder := NewRAMFolder("")
	out, _ := folder.OpenOut("foo.dat")
	err = stepper.WriteKeyFrame(out, "foo")
	if err != nil {
		t.Errorf("WriteKeyFrame: %v", err)
	}

	bb := clownfish.NewByteBuf(0)
	//bb.Cat([]byte("food"))
	err = stepper.WriteDelta(out, bb)
	if err != nil {
		t.Errorf("WriteDelta: %v", err)
	}
	out.Close()
	stepper.Reset()
	in, _ := folder.OpenIn("foo.dat")
	err = stepper.ReadKeyFrame(in)
	if err != nil {
		t.Errorf("ReadKeyFrame: %v", err)
	}
	err = stepper.ReadDelta(in)
	if err != nil {
		t.Errorf("ReadDelta: %v", err)
	}
	err = stepper.ReadDelta(in)
	if err == nil {
		t.Errorf("Expected error when reading past EOF")
	}
}
