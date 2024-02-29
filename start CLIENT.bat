@ECHO OFF

START "%~dp0" "VoiceChat\Medic VoiceChat\pythonclient.exe"

TIMEOUT /T 2 /NOBREAK

START "%~dp0" "VNC Client App\basicViewerWin.exe"

EXIT