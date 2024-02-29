@ECHO OFF

START /min "Ambulance Voice Chat" "%~dp0VoiceChat\Ambulance VoiceChat\pythonclient.exe"

START "VNC Server" "%~dp0VNC Server App\basicServer.exe"

START "scrcpy" "%~dp0scrcpy-win64-v2.1.1/scrcpy" --crop 1730:974:1934:450 -d --window-title 'AmbulancePOV'

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

endlocal

EXIT