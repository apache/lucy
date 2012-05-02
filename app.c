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

#include <stdio.h>
#include "core.h"
#include "module.h"

int main() {
    Core_bootstrap();
    Module_bootstrap();

    Dog *fido = Dog_new("Fido");
    printf("This is my dog, Fido:\n    ");
    Dog_speak(fido);
    printf("Here, Fido!\n    ");
    Dog_ignore_name(fido);
    printf("OK, never mind then.  Bye Fido!\n\n");
    Dog_destroy(fido);
    
    Boxer *drago = Boxer_new("Drago");
    printf("This is my my friend's dog, Drago:\n    ");
    Boxer_speak(drago);
    printf("Here, Drago!\n    ");
    Boxer_ignore_name(drago);
    printf("OK, never mind.\nDrago is a boxer.\n    ");
    Boxer_drool(drago);
    printf("Bye, Drago!\n");
    Boxer_destroy(drago);

    Module_tear_down();
    Core_tear_down();
}

