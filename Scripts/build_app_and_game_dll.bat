@echo off

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
echo build_app.bat Release VK
echo.
goto EndScript

:StartScript
set BuildConfig=%1
set GraphicsAPI=%2
if "%BuildConfig%" NEQ "Debug" (
    if "%BuildConfig%" NEQ "Release" (
        echo Invalid build config specified.
        goto EndScript
        )
    )

if "%GraphicsAPI%" == "VK" (
    rem Vulkan
    ) else (
        if "%GraphicsAPI%" == "D3D12" (
        rem D3D12
        echo D3D12 not yet supported. Build canceled.
        echo.
        goto EndScript
        ) else (
	    set GraphicsAPIChosen="None"
	    echo Unsupported graphics API specified.
        goto EndScript
        )
    )

rem Run engine build script
call build_app.bat %BuildConfig% 

rem Run game build script
call build_game_dll.bat %BuildConfig% %GraphicsAPI%
echo.

:EndScript
