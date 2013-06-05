@echo off

rem Licensed to the Apache Software Foundation (ASF) under one or more
rem contributor license agreements.  See the NOTICE file distributed with
rem this work for additional information regarding copyright ownership.
rem The ASF licenses this file to You under the Apache License, Version 2.0
rem (the "License"); you may not use this file except in compliance with
rem the License.  You may obtain a copy of the License at
rem
rem     http://www.apache.org/licenses/LICENSE-2.0
rem
rem Unless required by applicable law or agreed to in writing, software
rem distributed under the License is distributed on an "AS IS" BASIS,
rem WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
rem See the License for the specific language governing permissions and
rem limitations under the License.

cd ..\..\compiler\c
call configure.bat
cd ..\..\..\c
echo.

echo Configuring Clownfish runtime...

cl >nul 2>nul
if not errorlevel 1 goto found_cl

gcc -v >nul 2>nul
if not errorlevel 1 goto found_gcc

echo No C compiler found
exit /b 1

:found_cl
echo Using C compiler 'cl'
echo cl /nologo ..\common\charmonizer.c
cl /nologo ..\common\charmonizer.c
if errorlevel 1 exit /b 1
echo Running charmonizer
charmonizer.exe --cc=cl --enable-c --enable-makefile %*
exit /b

:found_gcc
echo Using C compiler 'gcc'
echo gcc ..\common\charmonizer.c -o charmonizer.exe
gcc ..\common\charmonizer.c -o charmonizer.exe
if errorlevel 1 exit /b 1
echo Running charmonizer
charmonizer.exe --cc=gcc --enable-c --enable-makefile %*
exit /b
