@echo off
setlocal

set VcVarsPathComm="C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat"
set VcVarsPathPro="C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvarsall.bat"
set VcVarsPathEnt="C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvarsall.bat"

set VsDevCmdPathComm="C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"
set VsDevCmdPathPro="C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\Common7\Tools\VsDevCmd.bat"
set VsDevCmdPathEnt="C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\Common7\Tools\VsDevCmd.bat"

IF NOT EXIST %VcVarsPathComm% (
    IF NOT EXIST %VcVarsPathPro% (
        IF NOT EXIST %VcVarsPathEnt% (
            echo Could not find VCVarsall.
            echo Searched for these files:
            echo %VcVarsPathComm%
            echo %VcVarsPathPro%
            echo %VcVarsPathEnt%
            ) ELSE set VcVarsBat=%VcVarsPathEnt%
        ) ELSE set VcVarsBat=%VcVarsPathPro%
    ) ELSE set VcVarsBat=%VcVarsPathComm%
set VcVarsCmd=%VcVarsBat% x64

IF NOT EXIST %VsDevCmdPathComm% (
    IF NOT EXIST %VsDevCmdPathPro% (
        IF NOT EXIST %VsDevCmdPathEnt% (
            echo Could not find VsDevCmd.
            echo Searched for these files:
            echo %VsDevCmdPathComm%
            echo %VsDevCmdPathPro%
            echo %VsDevCmdPathEnt%
            ) ELSE set VsDevBat=%VsDevCmdPathEnt%
        ) ELSE set VsDevBat=%VsDevCmdPathPro%
    ) ELSE set VsDevBat=%VsDevCmdPathComm%
set VsDevCmd=%VsDevBat% -arch=x64

%comspec% /k "%VcVarsCmd% && %VsDevCmd%"

pause
