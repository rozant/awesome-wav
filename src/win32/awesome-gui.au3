#cs **************************************************************
* This program is free software; you can redistribute it and/or	 *
* modify it under the terms of the GNU General Public License	 *
* version 2 as published by the Free Software Foundation.		 *
*																 *
* This program is distributed in the hope that it will be		 *
* useful, but WITHOUT ANY WARRANTY; without even the implied	 *
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR		 *
* PURPOSE.  See the GNU General Public License for more details. *
*																 *
* You should have received a copy of the GNU General Public 	 *
* License along with this program; if not, write to the Free 	 *
* Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139,	 *
* USA.															 *
#ce **************************************************************
Opt("MustDeclareVars",1)
#include <GUIConstantsEx.au3>

; Just to make this conform to my C-like mind
Main()

Func Main()
	; create the window handle
	Local $gui = GUICreate("Awesome-Wav GUI", 400, 443)
	; Create encode and decode radio options
	GUICtrlCreateGroup("Action",5,5,185,60)
	Local $encode = GUICtrlCreateRadio("Encode", 20,30,70)
	GUICtrlSetState($encode,1)
	Local $decode = GUICtrlCreateRadio("Decode", 100,30,70)
	; create flac and wav radio options
	GUICtrlCreateGroup("File Format",205,5,185,60)
	Local $FLAC = GUICtrlCreateRadio("FLAC", 220,30,70)
	GUICtrlSetState($FLAC,1)
	Local $Wav = GUICtrlCreateRadio("Wav", 300,30,70)
	
	; all the input boxes and related stuff
	GUICtrlCreateGroup("Input Data",5,70,390, 175)
	Local $inLab = GUICtrlCreateLabel("Data File", 15, 90,150)
	Local $in = GUICtrlCreateInput("",20,110, 340,22)
	Local $inBrowse = GUICtrlCreateIcon("shell32.dll",4,365,110,22,22)
	Local $datLab = GUICtrlCreateLabel("Audio File", 15, 140,150)
	Local $dat = GUICtrlCreateInput("",20,160, 340,22)
	Local $datBrowse = GUICtrlCreateIcon("shell32.dll",4,365,160,22,22)
	Local $outLab = GUICtrlCreateLabel("Output File", 15, 190,150)
	Local $out = GUICtrlCreateInput("",20,210, 340,22)
	Local $outBrowse = GUICtrlCreateIcon("shell32.dll",4,365,210,22,22)
	
	; the encryption group
	GUICtrlCreateGroup("Encryption", 5,250, 390, 80)
	Local $encryption = GUICtrlCreateCheckbox("Use Encryption",15, 270, 150,22)
	Local $key = GUICtrlCreateInput("",20,295,350, 22)
	GUICtrlSetState($key, $GUI_DISABLE)
	
	; the compression group
	GUICtrlCreateGroup("Compression", 5,330, 390, 80)
	Local $compression = GUICtrlCreateCheckbox("Use Compression",15,350,150)
	Local $complabelF = GUICtrlCreateLabel("Fast",15,375)
	Local $complabelS = GUICtrlCreateLabel("Small",360,375)
	Local $compSlider = GUICtrlCreateSlider(35,370,325,25)
	; yay, its got the right limits
	GUICtrlSetLimit($compSlider,9,1)
	GUICtrlSetData($compSlider,6)
	; disable it to start off with
	GUICtrlSetState($compslider,$GUI_DISABLE)
	GUICtrlSetState($complabelF,$GUI_DISABLE)
	GUICtrlSetState($complabelS,$GUI_DISABLE)
	
	; we do need to run the application after all...
	Local $execute = GUICtrlCreateButton("Execute",295,415,100,22)
	; and finally, show our beautiful creation to the user
	GUISetState(@SW_SHOW,$gui)
	
	Local $state
	; main event loop
	While 1
		Switch (GUIGetMsg())
		; triggered when the x is pressed
		Case -3
			Exit
		; browse for files
		Case $inBrowse
			GUICtrlSetData($in,FileOpenDialog("Input File",@UserProfileDir,"All Files (*.*)"))
		Case $datBrowse
			GUICtrlSetData($dat,FileOpenDialog("Output File",@UserProfileDir,"Wav Files (*.wav)|FLAC Files (*.flac)"))
		Case $outBrowse
			GUICtrlSetData($out,FileSaveDialog("Output File",@UserProfileDir,"Wav Files (*.wav)|FLAC Files (*.flac)"),18)
		; enable/disable encryption controls
		Case $encryption
			GUICtrlSetState($key,(GuiCtrlRead($encryption)==$GUI_CHECKED)*$GUI_ENABLE + (GuiCtrlRead($encryption)<>$GUI_CHECKED)*$GUI_DISABLE)
		; enable/disable controls for encoding and decoding
		Case $decode
			GUICtrlSetState($in, $GUI_DISABLE)
			GUICtrlSetState($inLab, $GUI_DISABLE)
			GUICtrlSetState($inBrowse, $GUI_DISABLE)
		Case $encode
			GUICtrlSetState($in, $GUI_ENABLE)
			GUICtrlSetState($inLab, $GUI_ENABLE)
			GUICtrlSetState($inBrowse, $GUI_ENABLE)
		; enable/disable compression controls
		Case $compression
			$state = (GuiCtrlRead($compression)==$GUI_CHECKED)*$GUI_ENABLE + (GuiCtrlRead($compression)<>$GUI_CHECKED)*$GUI_DISABLE
			GUICtrlSetState($compslider,$state)
			GUICtrlSetState($complabelF,$state)
			GUICtrlSetState($complabelS,$state)
		; little bit o black magic
		Case $execute
			; encode it
			If GUICtrlRead($encode)==$GUI_CHECKED Then
				; basic error checking to make sure that everything is filled in with valid data
				$state = CheckFiles("encode",$in, $dat)
				If ($state<>"") Then
					MsgBox(0,"ERROR!","The following error(s) occured while executing the request:"&@CRLF&$state)
					ContinueLoop
				EndIf
				; grab the basic options. format, etc
				$state = FormBasicOptions($Wav, $FLAC, $compression, $compSlider)
				; encryption goes here
				$state&= FormEncryptionCMDString($encryption, $key)
				; files to be manipulated
				$state&= "-e "&Chr(34)&GUICtrlRead($dat)&Chr(34) & " "
				$state&= Chr(34)&GUICtrlRead($out) &Chr(34)& " "
				$state&= Chr(34)&GUICtrlRead($in)&Chr(34)
			Else	; decode it
				; basic error checking to make sure that everything is filled in with valid data
				$state = CheckFiles("decode",$in, $dat)
				If ($state<>"") Then
					MsgBox(0,"ERROR!","The following error(s) occured while executing the request:"&@CRLF&$state)
					ContinueLoop
				EndIf
				; grab the basic options. format, etc.
				$state = FormBasicOptions($Wav, $FLAC, $compression, $compSlider)
				; files to be manipulated
				$state&= "-d "&Chr(34)&GUICtrlRead($dat) & Chr(34)&" "
				$state&= Chr(34)&GUICtrlRead($in) &Chr(34)& " "
				$state&= Chr(34)&GUICtrlRead($key) & Chr(34)
			EndIf
			ClipPut($state)
			;ShellExecuteWait("awesome-wav.exe", $state, @scriptdir)
			MsgBox(0,"Completion","Operation Completed")
		EndSwitch
		Sleep(15)
	WEnd
EndFunc

; checks for basic input errors
Func CheckFiles($mode, $data, $audio)
	Local $errors = ""
	If (FileExists(GuiCtrlRead($data))<>True And $mode=="encode") Then
		$errors &= "Must have a valid data file" & @CRLF
	EndIf
	If (FileExists(GuiCtrlRead($audio))<>True) Then
		$errors &= "Must have a valid audio file" & @CRLF
	EndIf
	Return $errors
EndFunc

; cobble some stuff together in a logical order
Func FormBasicOptions($wav, $FLAC, $compress, $level)
	Local $out = ""
	If GUICtrlRead($wav)==$GUI_CHECKED Then
		$out &= " -wav "
	EndIf
	If GUICtrlRead($FLAC)==$GUI_CHECKED Then
		$out &= " -flac "
	EndIf
	$out &= FormCompressionCMDString($compress, $level)
EndFunc

Func FormCompressionCMDString($compress, $level)
	If GUICtrlRead($compress) == $GUI_CHECKED Then
		Return " -zlib"&GUICtrlRead($level)&" "
	EndIf
	Return ""
EndFunc

Func FormEncryptionCMDString($encrypt, $key)
	If GUICtrlRead($encrypt) == $GUI_CHECKED Then
		Return " -aes "&GUICtrlRead($key)&" "
	EndIf
	Return ""
EndFunc
