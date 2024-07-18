@ECHO OFF

START /min "Ambulance Voice Chat" "%~dp0VoiceChat\client.exe"

START "scrcpy" "%~dp0scrcpy-win64-v2.1.1/scrcpy" --crop 1730:974:1934:450 -d --window-title AmbulancePOV

START "VNC Server" "%~dp0VNC Server App\R2R Ambulance.exe"

TIMEOUT /T 1 /NOBREAK

setlocal

rem Define the process name and window title
set "processName=basicServer"
set "windowTitle=My Window"

rem Execute the PowerShell script to minimize the window
powershell -ExecutionPolicy Bypass -File "ShowWindow.ps1" -ProcessName "%processName%" -WindowTitle "%windowTitle%"


rem Define the process name and window title
set "processName=scrcpy"
set "windowTitle=AmbulancePOV"

rem Execute the PowerShell script to minimize the window
powershell -ExecutionPolicy Bypass -File "ShowWindow.ps1" -ProcessName "%processName%" -WindowTitle "%windowTitle%"


:WAIT_LOOP
rem Check if Server process is still running
tasklist /FI "IMAGENAME eq R2R Ambulance.exe" 2>NUL | find /I /N "R2R Ambulance.exe">NUL
if "%ERRORLEVEL%"=="0" (
    rem Process is still running, wait and check again
    TIMEOUT /T 1 /NOBREAK
    GOTO WAIT_LOOP
)

:endlocal

REM Once PowerShell script is executed and scrcpy window is closed, close the remaining windows
taskkill /FI "WINDOWTITLE eq Ambulance Voice Chat"
taskkill /FI "WINDOWTITLE eq VNC Server"

EXIT