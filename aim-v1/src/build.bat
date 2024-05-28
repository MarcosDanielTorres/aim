setlocal
@echo off

if not exist "..\build\" (
    mkdir "..\build\"
)
pushd ..\build\

REM Define include paths
set INCLUDE_PATHS=/I ..\src ^
                  /I %VULKAN_SDK%\Include ^
                  /I ..\thirdparty\glfw-3.4.bin.WIN64\include ^
                  /I ..\thirdparty\glm-1.0.1 ^
                  /I ..\thirdparty\spdlog-1.14.1
                

REM Define libraries names
set VULKAN_LIB=vulkan-1.lib
set GLFW_LIB=glfw3.lib

REM Define library paths
set LIB_PATHS=/LIBPATH:%VULKAN_SDK%\Lib ^
              /LIBPATH:..\thirdparty\glfw-3.4.bin.WIN64\lib-vc2022

REM Get source files
SetLocal EnableDelayedExpansion
SET cFilenames=
FOR /r ..\src\ %%f in (*.cpp) do (
    SET cFilenames=!cFilenames! "%%f"
)

REM Define libraries
set LIBS=%VULKAN_LIB% ^
         %GLFW_LIB% ^
         user32.lib ^
         gdi32.lib ^
         shell32.lib ^
         msvcrt.lib ^
         vcruntime.lib

REM Compile the program
set OUTPUT_NAME=main.exe
cl /EHsc /Zi /std:c++17 /MD /Fe%OUTPUT_NAME% %cFilenames% %INCLUDE_PATHS% /link %LIB_PATHS% %LIBS%
endlocal
