#! /bin/sh

PCM8bit="PCM8bit.wav"
PCM16bit="PCM16bit.wav"
PCM24bit="PCM24bit.wav"
PCM32bit="PCM32bit.wav"
E_PCM="E_PCM.wav"
DATA="DATA1bit.txt"
D_DATA="D_DATA1bit.txt"
PCM=""

TEST_NUM=0

function start {
	clear;
	echo "Select an option:"
	echo "1.Run Tests"
	echo "2.Quit"
	echo ""
	echo -e "Enter your choice (1-2): \c"
	read choice
	if [ $choice -eq 1 ]; then
		next_test
	else 
		if [$choice -eq 2]; then 
			exit;
		else
			exit;
		fi
	fi
}

function next_test {
	let TEST_NUM=TEST_NUM+1
	if [ $TEST_NUM -eq 1 ]; then
		test_1
	else 
		if [ $TEST_NUM -eq 2 ]; then
			test_2
		else 
			if [ $TEST_NUM -eq 3 ]; then
				test_3
			else 
				if [ $TEST_NUM -eq 4 ]; then
					test_4
				else
					test_5
				fi
			fi
		fi
	fi
}

function test_1 {
	clear;
	PCM="$PCM8bit"
	tester
}

function test_2 {
	clear;
	PCM="$PCM16bit"
	tester
}

function test_3 {
	clear;
	PCM="$PCM24bit"
	tester
}

function test_4 {
	clear;
	PCM="$PCM32bit"
	tester
}

function test_5 {
	exit;
}

function tester {
	echo "==================================================================================="
	echo "Test $TEST_NUM"
	echo "==================================================================================="
	echo "WAV FILE:   $PCM"
	echo "E_WAV FILE: $E_PCM"
	echo "DATA FILE:  $DATA"
	echo "D_DATAFILE: $D_DATA"
	echo ""
	echo "Encoding and decoding files."
	if [ -e "./awesome-wav" ]; then 
		`./awesome-wav -t $PCM $E_PCM $DATA $D_DATA`
	else
		echo "Error: awesome-wav does not exist." 1>&2 
		exit;
	fi
	
}

start
