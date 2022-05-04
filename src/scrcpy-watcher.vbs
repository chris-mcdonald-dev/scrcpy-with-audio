Set WshShell = CreateObject("WScript.Shell" ) 
WshShell.Run chr(34) & "scrcpy-watcher.bat" & Chr(34), 0 
Set WshShell = Nothing 