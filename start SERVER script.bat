@ECHO OFF

START "%~dp0" "VoiceChat\Ambulance VoiceChat\pythonclient.exe"

TIMEOUT /T 2 /NOBREAK

START "%~dp0" "VNC Server App\basicServer.exe"

EXIT