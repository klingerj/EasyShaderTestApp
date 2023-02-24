@echo off
pushd ..
pushd .\Build

set DevenvPathComm="C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\IDE\devenv.exe"
set DevenvPathPro="C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\Common7\IDE\devenv.exe"
set DevenvPathEnt="C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\Common7\IDE\devenv.exe"
IF NOT EXIST %DevenvPathComm% (
    IF NOT EXIST %DevenvPathPro% (
        IF NOT EXIST %DevenvPathEnt% (
            echo Could not find 'devenv.exe'
            echo Searched for these files:
            echo %DevenvPathComm%
            echo %DevenvPathPro%
            echo %DevenvPathEnt%
            ) ELSE set DevenvPath=%DevenvPathEnt%
        ) ELSE set DevenvPath=%DevenvPathPro%
    ) ELSE set DevenvPath=%DevenvPathComm%
start "" %DevenvPath% /nosplash /debugexe .\%1 

popd
popd
