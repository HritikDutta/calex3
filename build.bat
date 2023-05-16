@echo off

set executable_name="calex.exe"

if "%1"=="release" (
    set defines= /DGN_CUSTOM_MAIN /DGN_USE_OPENGL /DGN_PLATFORM_WINDOWS /DGN_USE_DEDICATED_GPU /DGN_RELEASE /DNDEBUG /DGN_COMPILER_MSVC
    set compile_flags= /O2 /EHsc /std:c++17 /cgthreads8 /MP7 /GL
    set link_flags= /NODEFAULTLIB:LIBCMT /LTCG

    echo BUILDING RELEASE EXECUTABLE
) else (
    set defines= /DGN_CUSTOM_MAIN /DGN_USE_OPENGL /DGN_PLATFORM_WINDOWS /DGN_USE_DEDICATED_GPU /DGN_DEBUG /DGN_COMPILER_MSVC
    set compile_flags= /Zi /EHsc /std:c++17 /cgthreads8 /MP7 /GL
    set link_flags= /DEBUG /NODEFAULTLIB:LIBCMT /LTCG

    echo BUILDING DEBUG EXECUTABLE
)

set includes= /I src ^
              /I dependencies\glad\include   ^
              /I dependencies\wglext\include ^
              /I dependencies\stb\include    ^
              /I dependencies\miniz\include

set libs= shell32.lib                     ^
          user32.lib                      ^
          gdi32.lib                       ^
          openGL32.lib                    ^
          msvcrt.lib                      ^
          comdlg32.lib                    ^
          dependencies\glad\lib\glad.lib  ^
          dependencies\stb\lib\stb.lib    ^
          dependencies\miniz\lib\miniz.lib

rem Source
cl %compile_flags% /c src/serialization/json/*.cpp %defines% %includes%   &^
cl %compile_flags% /c src/serialization/binary/*.cpp %defines% %includes% &^
cl %compile_flags% /c src/fileio/*.cpp %defines% %includes%               &^
cl %compile_flags% /c src/graphics/*.cpp %defines% %includes%             &^
cl %compile_flags% /c src/platform/*.cpp %defines% %includes%             &^
cl %compile_flags% /c src/application/*.cpp %defines% %includes%          &^
cl %compile_flags% /c src/core/*.cpp %defines% %includes%                 &^
cl %compile_flags% /c src/platform/*.cpp %defines% %includes%             &^
cl %compile_flags% /c src/math/*.cpp %defines% %includes%                 &^
cl %compile_flags% /c src/engine/*.cpp %defines% %includes%               &^
cl %compile_flags% /c src/calculator/*.cpp %defines% %includes%           &^
cl %compile_flags% /c src/main.cpp %defines% %includes%

link *.obj %libs% /OUT:%executable_name% %link_flags%

rem Remove intermediate files
del *.obj *.exp *.lib