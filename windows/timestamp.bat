@echo off
echo ======== %CD% %1========
@REM set TIMESTAMPFILE=..\windows\timestamp.h
set TIMESTAMPFILE=%1\timestamp.h

echo #ifndef __TIMESTAMP_H__ > %TIMESTAMPFILE%
echo #define __TIMESTAMP_H__ >> %TIMESTAMPFILE%
@REM echo -n #define __TIMESTAMP  >> %TIMESTAMPFILE%
@REM date +%Y-%m-%d\ %H:%M:%S\ %Z >> %TIMESTAMPFILE%
echo #define __TIMESTAMP %DATE% %TIME% >> %TIMESTAMPFILE%
echo #define STR(x) #x >> %TIMESTAMPFILE%
echo #define _TIMESTAMP(x) STR(x) >> %TIMESTAMPFILE%
echo #define TIMESTAMP _TIMESTAMP(__TIMESTAMP) >> %TIMESTAMPFILE%
echo #endif >> %TIMESTAMPFILE%
