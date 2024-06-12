setlocal
echo "BUILDING ENGINE"
@echo off

if not exist ".\build\" (
    mkdir ".\build\"
)
pushd .\build\

REM Define include paths
set INCLUDE_PATHS=/I ..\engine\src ^
                  /I %VULKAN_SDK%\Include ^
                  /I ..\engine\thirdparty\SDL\include ^
                  /I ..\engine\thirdparty\SDL\include\SDL2 ^
                  /I ..\engine\thirdparty\SDL\include-config-debug\SDL2
                 


REM Define library paths
set LIB_PATHS=/LIBPATH:%VULKAN_SDK%/Lib ^
	      /LIBPATH:..\engine\thirdparty\SDL\lib


REM Get source files
SetLocal EnableDelayedExpansion
SET cFilenames=
FOR /r ..\engine\src\ %%f in (*.cpp) do (
    SET cFilenames=!cFilenames! "%%f"
)

REM Define libraries
set LIBS= SDL2.lib ^
         SDL2main.lib ^
         user32.lib ^
         gdi32.lib ^
         shell32.lib ^
         msvcrt.lib ^
         vcruntime.lib ^
	 vulkan-1.lib

REM Compile the program
set OUTPUT_NAME=engine
cl /EHsc /RTC1 /LD /DAIM_EXPORTS /Zi /std:c++20 /MD /Fe%OUTPUT_NAME% %cFilenames% %INCLUDE_PATHS% /link %LIB_PATHS% %LIBS%
endlocal
         
