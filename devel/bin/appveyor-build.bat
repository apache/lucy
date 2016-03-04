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

if "%CLOWNFISH_HOST%" == "c" goto test_c
if "%CLOWNFISH_HOST%" == "perl" goto test_perl

echo unknown CLOWNFISH_HOST: %CLOWNFISH_HOST%
exit /b 1

:test_c

if "%MSVC_VERSION%" == "10" goto msvc_10

call "C:\Program Files (x86)\Microsoft Visual Studio %MSVC_VERSION%.0\VC\vcvarsall.bat" amd64
goto msvc_build

:msvc_10
call "C:\Program Files\Microsoft SDKs\Windows\v7.1\Bin\SetEnv.cmd" /x64

:msvc_build

rem Install Clownfish.
cd \projects
git clone -q -b 0.5 --depth 1 https://git-wip-us.apache.org/repos/asf/lucy-clownfish.git
cd lucy-clownfish\runtime\c
call configure && nmake || exit /b
call install --prefix C:\install

cd \projects\lucy\c
call configure --clownfish-prefix C:\install && nmake && nmake test

exit /b

:test_perl

perl -V

cd perl
perl Build.PL && call Build && call Build test

exit /b

