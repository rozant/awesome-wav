@echo off
setlocal enabledelayedexpansion
title Awesome-WAV Batch Checker

set PCM8bit="PCM8bit.wav"
set PCM16bit="PCM16bit.wav"
set PCM24bit="PCM24bit.wav"
set E_PCM="E_PCM.wav"
set DATA=""
set D_DATA="D_data.txt"
set DATA8bit1b="test_input-1b-8kbps-21791276.txt"
set DATA8bit2b="test_input-2b-8kbps-21791276.txt"
set DATA8bit4b="test_input-4b-8kbps-21791276.txt"
set DATA16bit1b="test_input-1b-16kbps-43582508.txt"
set DATA16bit2b="test_input-2b-16kbps-43582508.txt"
set DATA16bit4b="test_input-4b-16kbps-43582508.txt"
set DATA16bit8b="test_input-8b-16kbps-43582508.txt"
set DATA24bit12b="test_input-12b-24kbps.txt"

set /A COMPRESSION_LEVEL=0
set /A TEST_NUM=0
set /A TEST_PASS=0

:start
cls
echo Select an option:
echo 1.Run tests
echo 2.Set compression level
echo 3.Quit
echo.
set /p choice=Enter your choice (1-3): 
if %choice%==1 goto nexttest
if %choice%==2 goto setcompression
if %choice%==3 goto quit
goto start

:test1
cls
set PCM=%PCM8bit%
set DATA=%DATA8bit1b%
goto tester

:test2
set PCM=%PCM8bit%
set DATA=%DATA8bit2b%
goto tester

:test3
set PCM=%PCM8bit%
set DATA=%DATA8bit4b%
goto tester

:test4
set PCM=%PCM16bit%
set DATA=%DATA16bit1b%
goto tester

:test5
set PCM=%PCM16bit%
set DATA=%DATA16bit2b%
goto tester

:test6
set PCM=%PCM16bit%
set DATA=%DATA16bit4b%
goto tester

:test7
set PCM=%PCM16bit%
set DATA=%DATA16bit8b%
goto tester

:test8
set PCM=%PCM24bit%
set DATA=%DATA24bit12b%
goto tester

:test9
set /A TEST_NUM-=1
echo ===================================================================================
echo Passed %TEST_PASS% / %TEST_NUM%
echo ===================================================================================
set /A TEST_NUM=0
set /A TEST_PASS=0
pause
goto start

:nexttest
set /A TEST_NUM+=1
goto test%TEST_NUM%

:tester
echo ===================================================================================
echo Test %TEST_NUM%
echo ===================================================================================
echo WAV FILE:    %PCM%
echo E_WAV FILE:  %E_PCM%
echo DATA FILE:   %DATA%
echo D_DATAFILE:  %D_DATA%
echo COMPRESSION LEVEL: %COMPRESSION_LEVEL%
echo.
echo Encoding and decoding files.
awesome-wav.exe -c%COMPRESSION_LEVEL% -t %PCM% %E_PCM% %DATA% %D_DATA%
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
set /A TEST_PASS+=1
goto nexttest

:programfail
echo Program failed.
echo.
goto nexttest

:comparefail
echo File comparison failed.
echo.
goto nexttest

:setcompression
cls
set /p choice=Enter file compression level (0-9): 
if %choice% LSS 0 goto setcompression
if %choice% GTR 9 goto setcompression
set /A COMPRESSION_LEVEL = %choice%
goto start

:quit
cls
set /p choice=Are you sure you wish to quit? (y/n): 
if %choice%==n goto start
if %choice% NEQ y goto quit
exit