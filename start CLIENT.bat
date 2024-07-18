@ECHO OFF

START /min "Medic Voice Chat" "%~dp0VoiceChat\client.exe"

START /W "VNC Client" "%~dp0VNC Client App\R2R Medic.exe"

REM Once PowerShell script is executed, close the remaining windows
taskkill /FI "WINDOWTITLE eq Medic Voice Chat"

EXIT