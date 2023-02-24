@echo off
setlocal
setlocal enabledelayedexpansion

if "%1" == "-h" (goto PrintHelp)
if "%1" == "-help" (goto PrintHelp)
if "%1" == "help" (goto PrintHelp)
goto StartScript

:PrintHelp
echo Usage: build_sc.bat ^<build_mode^> 
echo.
echo build_mode:
echo   Release
echo   Debug
echo.
echo For example:
echo build_sc.bat Release VK
echo.
goto EndScript

:StartScript
set BuildConfig=%1
set GraphicsAPI=%2
if "%BuildConfig%" NEQ "Debug" (
    if "%BuildConfig%" NEQ "Release" (
        echo Invalid build config specified.
        goto DoneBuild
        )
    )

echo ***** Building Tinker Shader Compiler *****

pushd ..
pushd .\ToolsBin
del TinkerSC.pdb > NUL 2> NUL

rem *********************************************************************************************************
rem /FAs for .asm file output
set CommonCompileFlags=/nologo /std:c++20 /W4 /WX /wd4127 /wd4530 /wd4201 /wd4324 /wd4100 /wd4189 /EHa- /GR- /Gm- /GS- /fp:fast /Zi /FS
set CommonLinkFlags=/incremental:no /opt:ref /DEBUG

if "%BuildConfig%" == "Debug" (
    echo Debug mode specified.
    set CommonCompileFlags=%CommonCompileFlags% /Od /MTd
    set CommonLinkFlags=%CommonLinkFlags% /debug:full
    ) else (
    echo Release mode specified.
    set CommonCompileFlags=%CommonCompileFlags% /O2 /MT
    )

rem *********************************************************************************************************
rem TinkerSC - primary exe
set AbsolutePathPrefix=%cd%

set SourceListSC= 
set SourceListSC=%SourceListSC% %AbsolutePathPrefix%/../Tools/ShaderCompiler/Main.cpp 
set SourceListSC=%SourceListSC% %AbsolutePathPrefix%/../Tools/ShaderCompiler/ShaderCompiler.cpp 
set SourceListSC=%SourceListSC% %AbsolutePathPrefix%/../Core/DataStructures/Vector.cpp 
set SourceListSC=%SourceListSC% %AbsolutePathPrefix%/../Core/Mem.cpp 
set SourceListSC=%SourceListSC% %AbsolutePathPrefix%/../Core/Platform/Win32File.cpp 
set SourceListSC=%SourceListSC% %AbsolutePathPrefix%/../Core/Platform/Win32Logging.cpp 
set SourceListSC=%SourceListSC% %AbsolutePathPrefix%/../Core/Platform/Win32PlatformGameAPI.cpp 
set SourceListSC=%SourceListSC% %AbsolutePathPrefix%/../ThirdParty/MurmurHash3/MurmurHash3.cpp 

rem Calculate absolute path prefix for application path parameters here
set AbsolutePathPrefix=%AbsolutePathPrefix:\=\\%
set CompileDefines=/DASSERTS_ENABLE=1 /D_SHADERS_SRC_DIR=%AbsolutePathPrefix%\\..\\Shaders\\hlsl\\ /D_SHADERS_SPV_DIR=%AbsolutePathPrefix%\\..\\Shaders\\spv\\

if "%BuildConfig%" == "Debug" (
    set DebugCompileFlagsSC=/FdTinkerSC.pdb
    set DebugLinkFlagsSC=/pdb:TinkerSC.pdb 
    set CompileDefines=!CompileDefines!
    ) else (
    set DebugCompileFlagsSC=/FdTinkerSC.pdb
    set DebugLinkFlagsSC=/pdb:TinkerSC.pdb 
    set CompileDefines=!CompileDefines!
    )

set CompileIncludePaths= /I ../Core /I ../ThirdParty/dxc_2022_07_18 /I ../ThirdParty/MurmurHash3
set LibsToLink=user32.lib ws2_32.lib ../ThirdParty/dxc_2022_07_18/lib/x64/dxcompiler.lib

echo.
echo Building TinkerSC.exe...

set OBJDir=%cd%\obj_sc\
if NOT EXIST %OBJDir% mkdir %OBJDir%
set CommonCompileFlags=%CommonCompileFlags% /Fo:%OBJDir%

cl %CommonCompileFlags% %CompileIncludePaths% %CompileDefines% %DebugCompileFlagsSC% %SourceListSC% /link %LibsToLink% %CommonLinkFlags% %DebugLinkFlagsSC% /out:TinkerSC.exe

echo.
if EXIST TinkerSC.exp (
    echo Deleting unnecessary file TinkerSC.exp
    echo.
    del TinkerSC.exp
    )

rem Copy needed DLLs to exe directory
echo.
echo Copying required dlls dxcompiler.dll and dxil.dll to exe dir...
copy ..\ThirdParty\dxc_2022_07_18\bin\x64\dxcompiler.dll 
copy ..\ThirdParty\dxc_2022_07_18\bin\x64\dxil.dll 
echo Done.

:DoneBuild
echo.
popd
popd

:EndScript
