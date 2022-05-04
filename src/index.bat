@REM ------ Set Configs ---------
FOR /F "tokens=1-2 delims==" %%a IN (phone-connection.config) DO (set %%a=%%b)


@REM ------ Run the actual commands -------
adb tcpip %port%
adb connect %ipAddress%:%port% 
cd src
"scrcpy-noconsole.vbs" --shortcut-mod=lctrl --max-size 1536

@REM ------ Start watching scrcpy.exe (mobile screen casting program) for when it closes
"scrcpy-watcher.vbs"

@REM ------ Start sndcpy to send audio stream -------
"sndcpy.vbs"