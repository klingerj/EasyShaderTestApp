@echo off
setlocal

rem This is meant to be an easy way to build and run the app/benchmarks/tests.
rem This script can just be double clicked in Windows Explorer.
rem All other scripts should be run from the command line, after running shell.bat.

rem Run vcvars since this isn't executing from the shell - this won't hurt, though.
set VcVarsPathComm="C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat"
set VcVarsPathPro="C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvarsall.bat"
set VcVarsPathEnt="C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvarsall.bat"

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
call %VcVarsCmd%
echo.

rem Run other build scripts
call build_app_and_game_dll.bat Release VK

echo.
echo Build finished.
echo Try running run_game.bat, run_benchmarks.bat, or run_tests.bat!
pause
