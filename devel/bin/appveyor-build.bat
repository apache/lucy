@echo off

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
git clone -q --depth 1 https://git-wip-us.apache.org/repos/asf/lucy-clownfish.git
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

