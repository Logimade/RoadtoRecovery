param (
    [string]$ProcessName,
    [string]$WindowTitle
)

# Get the process and window handle
$windowHandle = (Get-Process | Where-Object { $_.MainWindowTitle -eq $WindowTitle -and $_.ProcessName -eq $ProcessName }).MainWindowHandle

if ([string]::IsNullOrEmpty($windowHandle)) {
    Write-Host "Window not found."
} else {
    try {
        # Show the window
        $pinvokeCode = @'
        [DllImport("user32.dll")]
        public static extern bool ShowWindowAsync(IntPtr hWnd, int nCmdShow);
'@
        $type = Add-Type -MemberDefinition $pinvokeCode -Name Win32Utils -Namespace Win32Functions -PassThru

	$type::ShowWindowAsync($windowHandle, 6) # 6 = Minimize
        Write-Host "Window showed successfully."
    } catch {
        Write-Host "Error occurred while minimizing window: $_"
    }
}
