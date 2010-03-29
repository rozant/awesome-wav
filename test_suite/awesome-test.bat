@echo off
setlocal enabledelayedexpansion
title Awesome-WAV Batch Checker

set PCM8bit="PCM8bit.wav"
set PCM16bit="PCM16bit.wav"
set PCM24bit="PCM24bit.wav"
set E_PCM="E_PCM.wav"
set DATA="DATA1bit.txt"
set D_DATA="D_DATA1bit.txt"

:start
set /A TEST_NUM=0
cls
echo Select an option:
echo 1.Run Tests
echo 2.Quit
echo.
set /p choice=Enter your choice (1-2): 
if %choice%==1 goto nexttest
if %choice%==2 goto quit
goto start

:test1
cls
set PCM=%PCM8bit%
goto tester

:test2
set PCM=%PCM16bit%
goto tester

:test3
set PCM=%PCM24bit%
goto tester

:test4
pause
goto start

:nexttest
set /A TEST_NUM+=1
goto test%TEST_NUM%

:tester
echo ===================================================================================
echo Test %TEST_NUM%
echo ===================================================================================
echo WAV FILE:   %PCM%
echo E_WAV FILE: %E_PCM%
echo DATA FILE:  %DATA%
echo D_DATAFILE: %D_DATA%
echo.
echo Encoding and decoding files.
awesome-wav.exe -t %PCM% %E_PCM% %DATA% %D_DATA%
if %ERRORLEVEL% NEQ 0 goto programfail
echo Program succeeded.
echo.
echo Comparing data files.
fc %DATA% %D_DATA% > NUL
if %ERRORLEVEL% NEQ 0 goto comparefail
echo File comparison succeeded.
echo.
echo Passed.
echo.
goto nexttest



:programfail
echo Program failed.
echo.
goto nexttest

:comparefail
echo File comparison failed.
echo.
goto nexttest

:quit
cls
set /p choice=Are you sure you wish to quit? (y/n): 
if %choice%==n goto start
if %choice% NEQ y goto quit
exit
