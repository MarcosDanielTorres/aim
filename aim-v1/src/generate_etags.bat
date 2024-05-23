REM The problem is that if I only want to update the current directory I need to clear the tags first because it will contain duplicates
REM thirdparty scan could be way better


setlocal enabledelayedexpansion

REM Check for the -all flag
set "RUN_ALL=false"
if "%1"=="-all" (
    set "RUN_ALL=true"
)

REM Collect files from the current directory
for /r . %%f in (*) do (
    etags -a "%%f"
)

if "%RUN_ALL%"=="true" (
    REM Collect files from the thirdparty directory
    REM GLFW
    for /r ..\thirdparty\glfw-3.4.bin.WIN64 %%f in (*) do (
        etags -a "%%f"
    )

    REM Collect files from the VULKAN directory
    for /r "%VULKAN_SDK%/Include" %%f in (*) do (
        etags -a "%%f"
    )
)

endlocal