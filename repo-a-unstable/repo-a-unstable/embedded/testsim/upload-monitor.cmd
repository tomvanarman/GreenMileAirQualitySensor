@echo off
setlocal EnableDelayedExpansion
pushd "%~dp0"

set "PLATFORMIO_SETTING_ENABLE_TELEMETRY=no"
set "ENV_NAME=catm"
set "PORT_NAME="

if /I "%~1"=="--help" goto usage
if /I "%~1"=="-h" goto usage

if not "%~1"=="" (
  echo %~1 | findstr /R /I "^COM[0-9][0-9]*$" >nul
  if errorlevel 1 (
    set "ENV_NAME=%~1"
  ) else (
    set "PORT_NAME=%~1"
  )
)

if not "%~2"=="" set "PORT_NAME=%~2"

set "PIO=%USERPROFILE%\.platformio\penv\Scripts\pio.exe"

if not exist "%PIO%" (
  echo PlatformIO was not found at:
  echo %PIO%
  exit /b 1
)

if "%PORT_NAME%"=="" (
  set /a PORT_COUNT=0
  set "DEVICE_LIST=%TEMP%\testsim-device-list-%RANDOM%.txt"
  "%PIO%" device list > "!DEVICE_LIST!"

  for /f "tokens=1" %%P in ('findstr /R /I "^COM[0-9][0-9]*" "!DEVICE_LIST!"') do (
    set /a PORT_COUNT+=1
    set "DETECTED_PORT=%%P"
  )

  del "!DEVICE_LIST!" >nul 2>nul

  if "!PORT_COUNT!"=="1" (
    set "PORT_NAME=!DETECTED_PORT!"
  ) else (
    echo Could not auto-select a COM port.
    echo.
    echo Available devices:
    "%PIO%" device list
    echo.
    goto usage
  )
)

echo Uploading testsim environment "%ENV_NAME%" to "%PORT_NAME%"...
"%PIO%" run -e "%ENV_NAME%" -t upload --upload-port "%PORT_NAME%"
if errorlevel 1 exit /b %errorlevel%

echo.
echo Upload complete.
echo.
echo Open your serial monitor on "%PORT_NAME%" at 115200 baud now.
echo PlatformIO command, if you want to use it manually:
echo   "%PIO%" device monitor -b 115200 --port "%PORT_NAME%" --dtr 1 --rts 0 --raw
echo.

popd
exit /b 0

:wait_for_port
set "WAIT_PORT=%~1"
for /l %%I in (1,1,20) do (
  mode "%WAIT_PORT%" >nul 2>nul
  if not errorlevel 1 (
    timeout /t 2 /nobreak >nul
    exit /b 0
  )
  timeout /t 1 /nobreak >nul
)
echo Warning: "%WAIT_PORT%" did not become ready before monitor start.
exit /b 0

:usage
echo Usage:
echo   upload-monitor.cmd
echo   upload-monitor.cmd COM7
echo   upload-monitor.cmd catm COM7
echo   upload-monitor.cmd nbiot COM7
echo   upload-monitor.cmd auto COM7
echo   upload-monitor.cmd dc3test COM7
echo   upload-monitor.cmd dc3low COM7
echo   upload-monitor.cmd bldo1dc3 COM7
echo   upload-monitor.cmd bldo1dc3hold COM7
echo   upload-monitor.cmd bldo1dc3pins COM7
echo   upload-monitor.cmd bldo1dc3pwm COM7
echo   upload-monitor.cmd bldo1dc3nouvp COM7
echo   upload-monitor.cmd modemscan COM7
echo   upload-monitor.cmd modemscan33 COM7
echo   upload-monitor.cmd modemscanswap COM7
echo   upload-monitor.cmd modemscanbldo2 COM7
echo   upload-monitor.cmd modemkeyscan COM7
echo   upload-monitor.cmd vbuswatch COM7
echo   upload-monitor.cmd serialtest COM7
echo   upload-monitor.cmd blinktest COM7
popd
exit /b 1
