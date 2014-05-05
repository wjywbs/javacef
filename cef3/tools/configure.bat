@echo off
if not exist %~dp0gyp goto download_gyp
:gyp_ok

if "%JAVA_HOME%" == "" goto no_java

echo JAVA_HOME: %JAVA_HOME%
echo.
echo Please choose your build environment:
echo Note that ninja does not work well with non-English MSVC compilers.
echo.
echo 1. msvs 2013
echo 2. msvs 2012
echo 3. msvs 2010
echo 4. msvs 2008
echo 5. ninja

set /p i=
if %i%==1 goto msvs_2013
if %i%==2 goto msvs_2012
if %i%==3 goto msvs_2010
if %i%==4 goto msvs_2008
if %i%==5 goto ninja

echo.
echo Error: No valid build environment is chosen.
pause
exit 1

:msvs_2013
set GYP_MSVS_VERSION=2013
goto msvs

:msvs_2012
set GYP_MSVS_VERSION=2012
goto msvs

:msvs_2010
set GYP_MSVS_VERSION=2010
goto msvs

:msvs_2008
set GYP_MSVS_VERSION=2008
goto msvs

:msvs
set GYP_GENERATORS=msvs
goto generate_gyp

:ninja
set GYP_GENERATORS=ninja
echo.
echo Please choose build platform:
echo Note that your java should have the same platform.
echo.
echo 1. win32
echo 2. win64

set /p i=
if %i%==1 goto ninja_win32
if %i%==2 goto ninja_win64

:ninja_win32
set GYP_DEFINES="target_arch=ia32"
goto generate_gyp

:ninja_win64
set GYP_DEFINES="target_arch=x64"
goto generate_gyp

:generate_gyp
cd %~dp0..
python %~dp0gyp_chromium --toplevel-dir="." --depth="tools" -Djava_include_path="%JAVA_HOME%\include" javacef.gyp
pause
exit

:no_java
echo Error: JAVA_HOME environment variable is not set.
pause
exit 1

:download_gyp
echo Checking out gyp into tools\gyp...
call svn checkout --force http://gyp.googlecode.com/svn/trunk %~dp0gyp --revision 1895
goto gyp_ok
