@echo off
setlocal
setlocal enabledelayedexpansion

if "%1" == "-h" (goto PrintHelp)
if "%1" == "-help" (goto PrintHelp)
if "%1" == "help" (goto PrintHelp)
goto StartScript

:PrintHelp
echo Usage: build_engine.bat ^<build_mode^> ^<graphics_api^>
echo.
echo build_mode:
echo   Release
echo   Debug
echo.
echo graphics_api
echo   VK (uses VULKAN_SDK environment variable)
echo   D3D12
echo.
echo For example:
echo build_game.bat Release VK
echo.
goto EndScript

:StartScript
set BuildConfig=%1
set GraphicsAPI=%2

if "%BuildConfig%" NEQ "Debug" (
    if "%BuildConfig%" NEQ "Release" (
        echo Invalid build config specified. Must specify 'Release' or 'Debug'
        pause
        goto DoneBuild
        )
    )

if "%GraphicsAPI%" == "VK" (
    rem Vulkan
    echo Using Vulkan SDK: %VULKAN_SDK%
    echo.
    ) else (
        if "%GraphicsAPI%" == "D3D12" (
        echo D3D12 not yet supported. Build canceled.
        echo.
        goto DoneBuild
        ) else (
	    set GraphicsAPI="None"
	    echo Unsupported graphics API specified.
        goto DoneBuild
        )
    )

echo ***** Building Tinker Game *****

pushd ..
if NOT EXIST .\Build mkdir .\Build
pushd .\Build
del TinkerGame*.pdb > NUL 2> NUL

rem *********************************************************************************************************
rem /FAs for .asm file output
set CommonCompileFlags=/nologo /std:c++20 /W4 /WX /wd4127 /wd4530 /wd4201 /wd4324 /wd4100 /wd4189 /EHa- /GR- /Gm- /GS- /fp:fast /Zi /FS
set CommonLinkFlags=/incremental:no /opt:ref /DEBUG

if "%BuildConfig%" == "Debug" (
    echo Debug mode specified.
    set CommonCompileFlags=%CommonCompileFlags% /Zi /Od /MTd
    set CommonLinkFlags=%CommonLinkFlags% /debug:full
    ) else (
    echo Release mode specified.
    set CommonCompileFlags=%CommonCompileFlags% /O2 /MT
    )

set CompileIncludePaths=/I ../ 
set CompileIncludePaths=%CompileIncludePaths% /I ../Core  
set CompileIncludePaths=%CompileIncludePaths% /I ../Tools 
set CompileIncludePaths=%CompileIncludePaths% /I ../DebugUI 
set CompileIncludePaths=%CompileIncludePaths% /I ../ThirdParty/dxc_2022_07_18 
set CompileIncludePaths=%CompileIncludePaths% /I ../ThirdParty/MurmurHash3 
set CompileIncludePaths=%CompileIncludePaths% /I ../ThirdParty/imgui-docking 

rem *********************************************************************************************************
rem TinkerGame - shared library
set AbsolutePathPrefix=%cd%

set SourceListGame= 
set SourceListGame=%SourceListGame% %AbsolutePathPrefix%/../Game/GameMain.cpp 
set SourceListGame=%SourceListGame% %AbsolutePathPrefix%/../Graphics/Common/GraphicsCommon.cpp 
set SourceListGame=%SourceListGame% %AbsolutePathPrefix%/../Graphics/Common/ShaderManager.cpp 
set SourceListGame=%SourceListGame% %AbsolutePathPrefix%/../Graphics/Common/GPUTimestamps.cpp 
set SourceListGame=%SourceListGame% %AbsolutePathPrefix%/../Tools/ShaderCompiler/ShaderCompiler.cpp 
if "%GraphicsAPI%" == "VK" (
    set SourceListGame=!SourceListGame! %AbsolutePathPrefix%/../Graphics/Vulkan/Vulkan.cpp 
    set SourceListGame=!SourceListGame! %AbsolutePathPrefix%/../Graphics/Vulkan/VulkanCmds.cpp 
    set SourceListGame=!SourceListGame! %AbsolutePathPrefix%/../Graphics/Vulkan/VulkanTypes.cpp 
    set SourceListGame=!SourceListGame! %AbsolutePathPrefix%/../Graphics/Vulkan/VulkanCreation.cpp 
)
set SourceListGame=%SourceListGame% %AbsolutePathPrefix%/../ThirdParty/MurmurHash3/MurmurHash3.cpp 
if "%GraphicsAPI%" == "D3D12" ( echo No source files available for D3D12. )

rem Calculate absolute path prefix for application path parameters here
set AbsolutePathPrefix=%AbsolutePathPrefix:\=\\%
set CompileDefines=/DTINKER_GAME 
set CompileDefines=%CompileDefines% /DENABLE_MEM_TRACKING 
set CompileDefines=%CompileDefines% /DASSERTS_ENABLE=1 
set CompileDefines=%CompileDefines% /D_ASSETS_DIR=%AbsolutePathPrefix%\\..\\Assets\\ 
set CompileDefines=%CompileDefines% /D_SHADERS_SPV_DIR=%AbsolutePathPrefix%\\..\\Shaders\\spv\\ 
set CompileDefines=%CompileDefines% /D_SHADERS_SRC_DIR=%AbsolutePathPrefix%\\..\\Shaders\\hlsl\\ 
if "%GraphicsAPI%" == "VK" (
    set CompileDefines=!CompileDefines! /DVULKAN 
)

if "%TIME:~0,1%" == " " (
    set BuildTimestamp=%DATE:~4,2%_%DATE:~7,2%_%DATE:~10,4%__0%TIME:~1,1%_%TIME:~3,2%_%TIME:~6,2%
    ) else (
        set BuildTimestamp=%DATE:~4,2%_%DATE:~7,2%_%DATE:~10,4%__%TIME:~0,2%_%TIME:~3,2%_%TIME:~6,2%
        )
set GameDllPdbName=TinkerGame_%BuildTimestamp%.pdb

if "%BuildConfig%" == "Debug" (
    set DebugCompileFlagsGame=/Fd%GameDllPdbName%
    set DebugLinkFlagsGame=/pdb:%GameDllPdbName% 
    set CompileDefines=!CompileDefines! 
    ) else (
    set DebugCompileFlagsGame=/Fd%GameDllPdbName%
    set DebugLinkFlagsGame=/pdb:%GameDllPdbName% 
    set CompileDefines=!CompileDefines! 
    )

set LibsToLink=TinkerApp.lib 
set LibsToLink=%LibsToLink% ../ThirdParty/dxc_2022_07_18/lib/x64/dxcompiler.lib 
if "%GraphicsAPI%" == "VK" (
    set CompileIncludePaths=!CompileIncludePaths! /I %VULKAN_SDK%/Include 
    set LibsToLink=!LibsToLink! %VULKAN_SDK%\Lib\vulkan-1.lib
)

set OBJDir=%cd%\obj_game\
if NOT EXIST %OBJDir% mkdir %OBJDir%
set CommonCompileFlags=%CommonCompileFlags% /Fo:%OBJDir%

echo.
echo Building TinkerGame.dll...
cl %CommonCompileFlags% %CompileIncludePaths% %CompileDefines% %DebugCompileFlagsGame% %SourceListGame% /link %CommonLinkFlags% %LibsToLink% /DLL /export:GameUpdate /export:GameDestroy /export:GameWindowResize %DebugLinkFlagsGame% /out:TinkerGame.dll

rem Delete unnecessary files
echo.
if EXIST TinkerGame.lib (
    echo Deleting unnecessary file TinkerGame.lib
    del TinkerGame.lib
    )
if EXIST TinkerGame.exp (
    echo Deleting unnecessary file TinkerGame.exp
    echo.
    del TinkerGame.exp
    )

:DoneBuild
popd
popd
