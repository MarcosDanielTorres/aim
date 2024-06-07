setlocal
@echo off

if not exist "..\..\build\" (
    mkdir "..\..\build\"
)
pushd ..\..\build\

REM Define include paths
set INCLUDE_PATHS=/I ..\engine\src\

REM Define libraries names
set ENGINE_LIB=engine.lib

REM Define library paths
set LIB_PATHS=/LIBPATH:..\build\

REM Get source files
SetLocal EnableDelayedExpansion
SET cFilenames=
FOR /r ..\juego\src\ %%f in (*.cpp) do (
    SET cFilenames=!cFilenames! "%%f"
)

REM Define libraries
set LIBS=%ENGINE_LIB% ^
         user32.lib ^
         gdi32.lib ^
         shell32.lib ^
         msvcrt.lib ^
         vcruntime.lib

REM Compile the program
set OUTPUT_NAME=juego.exe
cl /EHsc /Zi /std:c++20 /MD /Fe%OUTPUT_NAME% %cFilenames% %INCLUDE_PATHS% /link %LIB_PATHS% %LIBS%
endlocal
