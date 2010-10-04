@echo off
setlocal enabledelayedexpansion
title Awesome-WAV Batch Checker

set PROGRAM=..\bin\awesome-wav.exe
set FILE=""
set DATA=""
set CURR_SERIES=""
set E_FILE=E_FILE.wav
set D_DATA=D_data.txt
set /A COMPRESSION_LEVEL=0
set /A TEST_NUM=1
set /A TEST_PASS=0
set /A NUM_TESTS=0
set /A AES_ENABLED=0
set AES_PASSWORD=""

set FILE_8_BIT_PCM=8_Bit_PCM.wav
set DATA_8_BIT_PCM1=test_input-1b-8kbps-21791276.txt
set DATA_8_BIT_PCM2=test_input-2b-8kbps-21791276.txt
set DATA_8_BIT_PCM3=test_input-4b-8kbps-21791276.txt
set NUM_8_BIT_PCM=3

set FILE_16_BIT_PCM=16_Bit_PCM.wav
set DATA_16_BIT_PCM1=test_input-1b-16kbps-43582508.txt
set DATA_16_BIT_PCM2=test_input-2b-16kbps-43582508.txt
set DATA_16_BIT_PCM3=test_input-4b-16kbps-43582508.txt
set DATA_16_BIT_PCM4=test_input-8b-16kbps-43582508.txt
set NUM_16_BIT_PCM=4

set FILE_24_BIT_PCM=24_Bit_PCM.wav
set DATA_24_BIT_PCM1=test_input-1b-24kbps-65373740.txt
set DATA_24_BIT_PCM2=test_input-2b-24kbps-65373740.txt
set DATA_24_BIT_PCM3=test_input-4b-24kbps-65373740.txt
set DATA_24_BIT_PCM4=test_input-8b-24kbps-65373740.txt
set DATA_24_BIT_PCM5=test_input-12b-24kbps-65373740.txt
set NUM_24_BIT_PCM=5

set FILE_32_BIT_PCM=32_Bit_PCM.wav
set DATA_32_BIT_PCM1=test_input-1b-32kbps-87164972.txt
set DATA_32_BIT_PCM2=test_input-2b-32kbps-87164972.txt
set DATA_32_BIT_PCM3=test_input-4b-32kbps-87164972.txt
set DATA_32_BIT_PCM4=test_input-8b-32kbps-87164972.txt
set DATA_32_BIT_PCM5=test_input-12b-32kbps-87164972.txt
set DATA_32_BIT_PCM6=test_input-16b-32kbps-87164972.txt
set NUM_32_BIT_PCM=6

set FILE_32_BIT_IEEE=32_BIT_IEEE.wav
set DATA_32_BIT_IEEE1=test_input-1b-32kbps-87164972.txt
set DATA_32_BIT_IEEE2=test_input-2b-32kbps-87164972.txt
set DATA_32_BIT_IEEE3=test_input-4b-32kbps-87164972.txt
set DATA_32_BIT_IEEE4=test_input-8b-32kbps-87164972.txt
set DATA_32_BIT_IEEE5=test_input-12b-32kbps-87164972.txt
set DATA_32_BIT_IEEE6=test_input-16b-32kbps-87164972.txt
set NUM_32_BIT_IEEE=6

set FILE_64_BIT_IEEE=64_BIT_IEEE.wav
set DATA_64_BIT_IEEE1=test_input-1b-64kbps-174329944.txt
set DATA_64_BIT_IEEE2=test_input-2b-64kbps-174329944.txt
set DATA_64_BIT_IEEE3=test_input-4b-64kbps-174329944.txt
set DATA_64_BIT_IEEE4=test_input-8b-64kbps-174329944.txt
set DATA_64_BIT_IEEE5=test_input-12b-64kbps-174329944.txt
set DATA_64_BIT_IEEE6=test_input-16b-64kbps-174329944.txt
set DATA_64_BIT_IEEE7=test_input-32b-64kbps-174329944.txt
set NUM_64_BIT_IEEE=7

:START
cls
if %AES_ENABLED%==1 echo AES is enabled.
if %AES_ENABLED%==0 echo AES is disabled.
if %COMPRESSION_LEVEL% NEQ 0 echo Compression is enabled (level %COMPRESSION_LEVEL%).
if %COMPRESSION_LEVEL%==0 echo Compression is disabled.
echo.

echo Select an option:
echo 1.Run 8  Bit PCM tests
echo 2.Run 16 Bit PCM tests
echo 3.Run 24 Bit PCM tests
echo 4.Run 32 Bit PCM tests
echo 5.Run 32 Bit IEEE tests
echo 6.Run 64 Bit IEEE tests
echo 7.Set compression level
echo 8.Set AES encryption
echo 9.Quit
echo.
set /p choice=Enter your choice (1-9): 
if %choice%==1 goto 8_BIT_PCM
if %choice%==2 goto 16_BIT_PCM
if %choice%==3 goto 24_BIT_PCM
if %choice%==4 goto 32_BIT_PCM
if %choice%==5 goto 32_BIT_IEEE
if %choice%==6 goto 64_BIT_IEEE
if %choice%==7 goto SET_COMPRESSION_LEVEL
if %choice%==8 goto SET_AES_ENCRYPTION
if %choice%==9 goto QUIT
goto START

:SHOW_RESULTS
echo ===============================================================================
echo Passed %TEST_PASS% / %TEST_NUM%
echo ===============================================================================
set /A TEST_NUM=1
set /A TEST_PASS=0
pause
goto START

:NEXT_TEST
if %NUM_TESTS%==%TEST_NUM% goto SHOW_RESULTS
set /A TEST_NUM+=1
goto %CURR_SERIES%%TEST_NUM%

:RUN_TEST
echo ===============================================================================
echo Test %TEST_NUM%
echo ===============================================================================
echo WAV FILE:    "%FILE%"
echo E_WAV FILE:  "%E_FILE%"
echo DATA FILE:   "%DATA%"
echo D_DATAFILE:  "%D_DATA%"
echo COMPRESSION LEVEL: %COMPRESSION_LEVEL%
echo.
if not exist %PROGRAM% goto NO_PROGRAM
echo Encoding and decoding files.
if %AES_ENABLED%==1 %PROGRAM% -aes %AES_PASSWORD% -c%COMPRESSION_LEVEL% -t "%FILE%" "%E_FILE%" "%DATA%" "%D_DATA%"
if %AES_ENABLED%==0 %PROGRAM% -c%COMPRESSION_LEVEL% -t "%FILE%" "%E_FILE%" "%DATA%" "%D_DATA%"
if %ERRORLEVEL% NEQ 0 goto PROGRAM_FAIL
echo Program succeeded.
echo.
echo Comparing data files.
fc "%DATA%" "%D_DATA%" > NUL
if %ERRORLEVEL% NEQ 0 goto COMPARE_FAIL
echo File comparison succeeded.
echo.
echo Passed.
echo.
set /A TEST_PASS+=1
goto NEXT_TEST

:PROGRAM_FAIL
echo Program failed.
echo.
goto NEXT_TEST

:COMPARE_FAIL
echo File comparison failed.
echo.
goto NEXT_TEST

:SET_COMPRESSION_LEVEL
cls
set /p choice=Enter file compression level (0-9): 
if %choice% LSS 0 goto SET_COMPRESSION_LEVEL
if %choice% GTR 9 goto SET_COMPRESSION_LEVEL
set /A COMPRESSION_LEVEL=%choice%
goto START

:SET_AES_ENCRYPTION
cls
set /p choice=Enable AES encryption? (y/n): 
if %choice% NEQ y goto AES_DISABLE
if %choice%==y goto AES_ENABLE
goto START

:AES_ENABLE
set /p choice=Enter a passphrase: 
set AES_PASSWORD=%choice%
set /A AES_ENABLED=1
echo AES enabled.
echo.
pause
goto START

:AES_DISABLE
set /A AES_ENABLED=0
set AES_PASSWORD=""
echo AES disabled.
echo.
pause
goto START


:NO_PROGRAM
echo %PROGRAM% not found.
pause
goto START

:QUIT
cls
set /p choice=Are you sure you wish to quit? (y/n): 
if %choice%==n goto START
if %choice% NEQ y goto QUIT
exit

:8_BIT_PCM
set CURR_SERIES=T_8_BIT_PCM
set FILE=%FILE_8_BIT_PCM%
set /A NUM_TESTS=%NUM_8_BIT_PCM%
cls
goto T_8_BIT_PCM1

:T_8_BIT_PCM1
set DATA=%DATA_8_BIT_PCM1%
goto RUN_TEST

:T_8_BIT_PCM2
set DATA=%DATA_8_BIT_PCM2%
goto RUN_TEST

:T_8_BIT_PCM3
set DATA=%DATA_8_BIT_PCM3%
goto RUN_TEST

:16_BIT_PCM
set CURR_SERIES=T_16_BIT_PCM
set FILE=%FILE_16_BIT_PCM%
set /A NUM_TESTS=%NUM_16_BIT_PCM%
cls
goto T_16_BIT_PCM1

:T_16_BIT_PCM1
set DATA=%DATA_16_BIT_PCM1%
goto RUN_TEST

:T_16_BIT_PCM2
set DATA=%DATA_16_BIT_PCM2%
goto RUN_TEST

:T_16_BIT_PCM3
set DATA=%DATA_16_BIT_PCM3%
goto RUN_TEST

:T_16_BIT_PCM4
set DATA=%DATA_16_BIT_PCM4%
goto RUN_TEST

:24_BIT_PCM
set CURR_SERIES=T_24_BIT_PCM
set FILE=%FILE_24_BIT_PCM%
set /A NUM_TESTS=%NUM_24_BIT_PCM%
cls
goto T_24_BIT_PCM1

:T_24_BIT_PCM1
set DATA=%DATA_24_BIT_PCM1%
goto RUN_TEST

:T_24_BIT_PCM2
set DATA=%DATA_24_BIT_PCM2%
goto RUN_TEST

:T_24_BIT_PCM3
set DATA=%DATA_24_BIT_PCM3%
goto RUN_TEST

:T_24_BIT_PCM4
set DATA=%DATA_24_BIT_PCM4%
goto RUN_TEST

:T_24_BIT_PCM5
set DATA=%DATA_24_BIT_PCM5%
goto RUN_TEST

:32_BIT_PCM
set CURR_SERIES=T_32_BIT_PCM
set FILE=%FILE_32_BIT_PCM%
set /A NUM_TESTS=%NUM_32_BIT_PCM%
cls
goto T_32_BIT_PCM1

:T_32_BIT_PCM1
set DATA=%DATA_32_BIT_PCM1%
goto RUN_TEST

:T_32_BIT_PCM2
set DATA=%DATA_32_BIT_PCM2%
goto RUN_TEST

:T_32_BIT_PCM3
set DATA=%DATA_32_BIT_PCM3%
goto RUN_TEST

:T_32_BIT_PCM4
set DATA=%DATA_32_BIT_PCM4%
goto RUN_TEST

:T_32_BIT_PCM5
set DATA=%DATA_32_BIT_PCM5%
goto RUN_TEST

:T_32_BIT_PCM6
set DATA=%DATA_32_BIT_PCM6%
goto RUN_TEST

:32_BIT_IEEE
set CURR_SERIES=T_32_BIT_IEEE
set FILE=%FILE_32_BIT_IEEE%
set /A NUM_TESTS=%NUM_32_BIT_IEEE%
cls
goto T_32_BIT_IEEE1

:T_32_BIT_IEEE1
set DATA=%DATA_32_BIT_IEEE1%
goto RUN_TEST

:T_32_BIT_IEEE2
set DATA=%DATA_32_BIT_IEEE2%
goto RUN_TEST

:T_32_BIT_IEEE3
set DATA=%DATA_32_BIT_IEEE3%
goto RUN_TEST

:T_32_BIT_IEEE4
set DATA=%DATA_32_BIT_IEEE4%
goto RUN_TEST

:T_32_BIT_IEEE5
set DATA=%DATA_32_BIT_IEEE5%
goto RUN_TEST

:T_32_BIT_IEEE6
set DATA=%DATA_32_BIT_IEEE6%
goto RUN_TEST

:64_BIT_IEEE
set CURR_SERIES=T_64_BIT_IEEE
set FILE=%FILE_64_BIT_IEEE%
set /A NUM_TESTS=%NUM_64_BIT_IEEE%
cls
goto T_64_BIT_IEEE1

:T_64_BIT_IEEE1
set DATA=%DATA_64_BIT_IEEE1%
goto RUN_TEST

:T_64_BIT_IEEE2
set DATA=%DATA_64_BIT_IEEE2%
goto RUN_TEST

:T_64_BIT_IEEE3
set DATA=%DATA_64_BIT_IEEE3%
goto RUN_TEST

:T_64_BIT_IEEE4
set DATA=%DATA_64_BIT_IEEE4%
goto RUN_TEST

:T_64_BIT_IEEE5
set DATA=%DATA_64_BIT_IEEE5%
goto RUN_TEST

:T_64_BIT_IEEE6
set DATA=%DATA_64_BIT_IEEE6%
goto RUN_TEST

:T_64_BIT_IEEE7
set DATA=%DATA_64_BIT_IEEE7%
goto RUN_TEST