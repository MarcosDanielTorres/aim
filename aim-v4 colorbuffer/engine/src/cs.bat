@echo off
setlocal
REM Check if the filename is provided
if "%1"=="" (
    echo Please provide a filename.
    exit /b 1
)

REM Extract the file path, name, and extension
set input_filepath=%~1
set filename=%~n1
set extension=%~x1
set extension=%extension:~1%
set directory=%~dp1

REM Construct the output filename
set output_filename=%directory%%filename%.%extension%.spv

REM Compile the shader
glslc "%input_filepath%" -o "%output_filename%"

REM Check if the compilation was successful
if "%ERRORLEVEL%"=="0" (
    echo Compilation successful: %output_filename%
) else (
    echo Compilation failed.
)
endlocal