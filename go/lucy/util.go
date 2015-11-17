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
#include "Lucy/Util/Stepper.h"
*/
import "C"
import "unsafe"

import "git-wip-us.apache.org/repos/asf/lucy-clownfish.git/runtime/go/clownfish"

func (s *StepperIMP) WriteKeyFrame(outstream OutStream, value interface{}) error {
	return clownfish.TrapErr(func() {
		self := (*C.lucy_Stepper)(clownfish.Unwrap(s, "s"))
		outstreamCF := (*C.lucy_OutStream)(clownfish.Unwrap(outstream, "outstream"))
		valueCF := (*C.cfish_Obj)(clownfish.GoToClownfish(value, unsafe.Pointer(C.CFISH_OBJ), false))
		defer C.cfish_decref(unsafe.Pointer(valueCF))
		C.LUCY_Stepper_Write_Key_Frame(self, outstreamCF, valueCF)
	})
}

func (s *StepperIMP) WriteDelta(outstream OutStream, value interface{}) error {
	return clownfish.TrapErr(func() {
		self := (*C.lucy_Stepper)(clownfish.Unwrap(s, "s"))
		outstreamCF := (*C.lucy_OutStream)(clownfish.Unwrap(outstream, "outstream"))
		valueCF := (*C.cfish_Obj)(clownfish.GoToClownfish(value, unsafe.Pointer(C.CFISH_OBJ), false))
		defer C.cfish_decref(unsafe.Pointer(valueCF))
		C.LUCY_Stepper_Write_Delta(self, outstreamCF, valueCF)
	})
}

func (s *StepperIMP) ReadKeyFrame(instream InStream) error {
	return clownfish.TrapErr(func() {
		self := (*C.lucy_Stepper)(clownfish.Unwrap(s, "s"))
		instreamCF := (*C.lucy_InStream)(clownfish.Unwrap(instream, "instream"))
		C.LUCY_Stepper_Read_Key_Frame(self, instreamCF)
	})
}

func (s *StepperIMP) ReadDelta(instream InStream) error {
	return clownfish.TrapErr(func() {
		self := (*C.lucy_Stepper)(clownfish.Unwrap(s, "s"))
		instreamCF := (*C.lucy_InStream)(clownfish.Unwrap(instream, "instream"))
		C.LUCY_Stepper_Read_Delta(self, instreamCF)
	})
}

func (s *StepperIMP) readRecord(instream InStream) error {
	return clownfish.TrapErr(func() {
		self := (*C.lucy_Stepper)(clownfish.Unwrap(s, "s"))
		instreamCF := (*C.lucy_InStream)(clownfish.Unwrap(instream, "instream"))
		C.LUCY_Stepper_Read_Record(self, instreamCF)
	})
}
