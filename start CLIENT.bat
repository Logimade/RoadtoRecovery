@ECHO OFF

START /min "Medic Voice Chat" "%~dp0VoiceChat\Medic VoiceChat\pythonclient.exe"

START /W /max "VNC Client" "%~dp0VNC Client App\basicViewerWin.exe"

REM Once PowerShell script is executed, close the remaining windows
taskkill /FI "WINDOWTITLE eq Medic Voice Chat"

EXIT