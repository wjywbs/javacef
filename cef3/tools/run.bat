@echo off
if "%JAVA_HOME%" == "" goto no_java

echo JAVA_HOME: %JAVA_HOME%
echo.
echo Please choose the java class to run:
echo.
echo 1. SampleBrowserSWT
echo 2. SampleWindow
echo 3. SampleBrowserAWT

set /p classnum=
if %classnum%==1 set JAVACLASS=SampleBrowserSWT
if %classnum%==2 set JAVACLASS=SampleWindow
if %classnum%==3 set JAVACLASS=SampleBrowserAWT

echo.
echo Please choose the platform you built:
echo Note that your java should have the same platform.
echo.
echo 1. win32
echo 2. win64

set /p i=
if %i%==1 set PLATFORM=32
if %i%==2 set PLATFORM=64

echo.
echo Please choose the build type to run:
echo.
echo 1. Debug
echo 2. Release
echo 3. Debug_x64   (if you built x64 with ninja)
echo 4. Release_x64 (if you built x64 with ninja)

set /p type=
if %type%==1 set BUILD_TYPE=Debug
if %type%==2 set BUILD_TYPE=Release
if %type%==3 set BUILD_TYPE=Debug_x64
if %type%==4 set BUILD_TYPE=Release_x64

"%JAVA_HOME%\bin\java" -cp "%~dp0..\java\lib\win\swt%PLATFORM%.jar;%~dp0..\out\%BUILD_TYPE%\javacef3_win%PLATFORM%.jar" -Djava.library.path="%~dp0..\cef_runtime\win%PLATFORM%;%~dp0..\out\%BUILD_TYPE%" -Duser.dir="%~dp0..\java" org.embedded.browser.%JAVACLASS%
pause
exit

:no_java
echo Error: JAVA_HOME environment variable is not set.
pause
exit 1