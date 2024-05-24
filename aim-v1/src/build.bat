@echo off
setlocal

set VULKAN_LIB=vulkan-1.lib
set GLFW_LIB=glfw3.lib

REM Define include paths
set INCLUDE_PATHS=/I %VULKAN_SDK%\Include ^
                  /I ..\thirdparty\glfw-3.4.bin.WIN64\include ^
		  /I ..\thirdparty\glm-1.0.1 ^
		  /I ..\thirdparty\spdlog-1.14.1
                

REM Define library paths
set LIB_PATHS=/LIBPATH:%VULKAN_SDK%\Lib ^
              /LIBPATH:..\thirdparty\glfw-3.4.bin.WIN64\lib-vc2022

REM Get source files
SetLocal EnableDelayedExpansion
SET cFilenames=
FOR /R %%f in (*.cpp) do (
    SET cFilenames=!cFilenames! %%f
)
@echo on
echo %cFilenames%
@echo off

REM Define libraries
set LIBS=%VULKAN_LIB% ^
         %GLFW_LIB% ^
         user32.lib ^
         gdi32.lib ^
         shell32.lib ^
         msvcrt.lib ^
         vcruntime.lib

REM Compile the program
if not exist "..\build\" (
    mkdir "..\build\"
    echo Directory created: ..\build\
)
pushd ..\build\
cl /EHsc /Zi /std:c++17 /MD ..\src\main.cpp %INCLUDE_PATHS% /link %LIB_PATHS% %LIBS%
endlocal
