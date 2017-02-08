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

if "%BUILD_ENV%" == "msys2" goto test_msys2

rem Install Clownfish.
git clone -q --depth 1 https://git-wip-us.apache.org/repos/asf/lucy-clownfish.git

if "%CLOWNFISH_HOST%" == "c" goto test_c
if "%CLOWNFISH_HOST%" == "perl" goto test_perl

echo unknown CLOWNFISH_HOST: %CLOWNFISH_HOST%
exit /b 1

:test_c

rem Needed to find DLL.
path C:\install\bin;%path%

if "%BUILD_ENV%" == "msvc" goto test_msvc
if "%BUILD_ENV%" == "mingw32" goto test_mingw32

echo unknown BUILD_ENV: %BUILD_ENV%
exit /b 1

:test_msvc

if "%MSVC_VERSION%" == "10" goto msvc_10

call "C:\Program Files (x86)\Microsoft Visual Studio %MSVC_VERSION%.0\VC\vcvarsall.bat" amd64
goto msvc_build

:msvc_10
call "C:\Program Files\Microsoft SDKs\Windows\v7.1\Bin\SetEnv.cmd" /x64

:msvc_build

cd lucy-clownfish\compiler\c
call configure --prefix C:\install && nmake install || exit /b
cd ..\..\runtime\c
call configure --prefix C:\install && nmake install || exit /b

cd ..\..\..\c
call configure --clownfish-prefix C:\install && nmake && nmake test

exit /b

:test_mingw32

path C:\MinGW\bin;%path%

cd lucy-clownfish\compiler\c
call configure --prefix C:\install && mingw32-make install || exit /b
cd ..\..\runtime\c
call configure --prefix C:\install && mingw32-make install || exit /b

cd ..\..\..\c
call configure --clownfish-prefix C:\install && mingw32-make test

exit /b

:test_msys2

C:\msys64\usr\bin\sh -lc "cd /c/projects/lucy && devel/bin/travis-test.sh"

exit /b

:test_perl

path C:\MinGW\bin;%path%
call ppm install dmake

perl -V

cd lucy-clownfish\compiler\perl
perl Build.PL && call Build install || exit /b
cd ..\..\runtime\perl
perl Build.PL && call Build install || exit /b

cd ..\..\..\perl
perl Build.PL && call Build && call Build test

exit /b

