setlocal

set LIBS=SDL2d.lib ^
         SDL2main.lib ^
         shell32.lib ^


cl /std:c++20 main.cpp /FC /Zi /W4 /WX /analyze /MDd /I..\thirdparty\SDL\include /I..\thirdparty\SDL\include/SDL2 /I..\thirdparty\SDL\include-config-debug/SDL2 /link /SUBSYSTEM:CONSOLE /LIBPATH:..\thirdparty\SDL\lib %LIBS%

endlocal
