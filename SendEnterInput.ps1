# Find the window with the specified title
$window = Get-Process | Where-Object { $_.MainWindowTitle -eq "Medic Voice Chat" }

# If the window is found
if ($window) {
    # Activate the window
    $signature = @"
    [DllImport("user32.dll")]
    public static extern bool SetForegroundWindow(IntPtr hWnd);
"@
    $type = Add-Type -MemberDefinition $signature -Name ActivateWindow -PassThru
    $type::SetForegroundWindow($window.MainWindowHandle) | Out-Null
    
    # Send a message to the window (e.g., type "exit" followed by Enter)
    Add-Type -AssemblyName System.Windows.Forms
    [System.Windows.Forms.SendKeys]::SendWait("exit{ENTER}")
}
