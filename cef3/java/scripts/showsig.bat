@echo off
cls
echo Chromium 1, DownloadWindow 2, ChromeSettings 3
set /p i=
if %i%==1 goto a
if %i%==2 goto b
if %i%==3 goto c
:ok
pause
exit

:a
"%jdk%\javap" -cp ..\bin -s org.embedded.browser.Chromium
goto ok

:b
"%jdk%\javap" -cp ..\bin -s org.embedded.browser.DownloadWindow
goto ok

:c
"%jdk%\javap" -cp ..\bin -s org.embedded.browser.ChromeSettings
goto ok