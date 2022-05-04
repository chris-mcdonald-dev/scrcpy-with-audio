SETLOCAL EnableExtensions
set EXE=scrcpy.exe

:LOOPSTART
@REM Watch and wait for scrcpy.exe (mobile screen casting program) to close
FOR /F %%x IN ('tasklist /NH /FI "IMAGENAME eq %EXE%"') DO IF %%x == %EXE% goto FOUND
goto FIN

:FOUND
CSCRIPT utils\millisecond-sleep.vbs 100
goto LOOPSTART

:FIN
@REM Get VLC's process ID
for /f %%p in ('wmic process WHERE "CommandLine LIKE '%%tcp://%%' AND Caption = 'vlc.exe' AND name != 'wmic.exe'" get ProcessId^|findstr [0-9]') do set vlcProcessId=%%p

@REM Stop VLC so it no longer streams audio
taskkill /PID %vlcProcessId% -f
