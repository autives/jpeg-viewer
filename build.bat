@echo off

setlocal

set SourceFiles=../../src/*.c
set OutputName=jpeg_viewer.exe

set CLFlags=-Od
set CLANGFlags=-g -gcodeview
set GCCFlags=-O

if "%1" neq "optimize" goto DoneConfig
set CLFlags=/Ox
set CLANGFlags=-O3 -gcodeview
set GCCFlags=-O3

echo -------------------------------------
echo Optimize Build configured
echo -------------------------------------
:DoneConfig

if not exist "libs" mkdir libs
pushd libs
where cl >nul 2>nul
IF %ERRORLEVEL% NEQ 0 goto CheckForClang
goto DownloadSDL
:CheckForClang
where clang >nul 2>nul
IF %ERRORLEVEL% NEQ 0 goto SkipDownloadSDL
:DownloadSDL
if exist "SDL2/" goto SkipDownloadSDL
mkdir SDL2
echo ----------------------------------------
echo Downloading SDL
echo ----------------------------------------
curl "https://www.libsdl.org/release/SDL2-devel-2.0.14-VC.zip" --output SDL2.zip
tar -zxvf SDL2.zip -C SDL2
del SDL2.zip
echo ----------------------------------------
ren "SDL2\SDL2-2.0.14\include" "SDL2"

:SkipDownloadSDL

where gcc >nul 2>nul
IF %ERRORLEVEL% NEQ 0 goto SkipDownloadSDLMinGw

if exist "SDL2MinGw/" goto SkipDownloadSDLMinGw
mkdir SDL2MinGw
echo ----------------------------------------
echo Downloading SDLMinGw
echo ----------------------------------------
curl "https://www.libsdl.org/release/SDL2-devel-2.0.14-mingw.tar.gz" --output SDL2MinGw.tar.gz
tar -xf SDL2MinGw.tar.gz -C SDL2MinGw
del SDL2MinGw.tar.gz

:SkipDownloadSDLMinGw
popd

set SDL2_Include="../../libs/SDL2/SDL2-2.0.14/"
set SDL2_Library="../../libs/SDL2/SDL2-2.0.14/lib/x64/"
set SDL2_DLL="..\..\libs\SDL2\SDL2-2.0.14\lib\x64\SDL2.dll"

set SDL2MinGw_Include="../../libs/SDL2MinGw/SDL2-2.0.14/x86_64-w64-mingw32/include/"
set SDL2MinGw_Library="../../libs/SDL2MinGw/SDL2-2.0.14/x86_64-w64-mingw32/lib/"
set SDL2MinGw_DLL="..\..\libs\SDL2MinGw\SDL2-2.0.14\x86_64-w64-mingw32\bin\SDL2.dll"

if not exist "bin" mkdir bin

echo ----------------------------------------
:MSVC
where cl >nul 2>nul
IF %ERRORLEVEL% NEQ 0 goto SkipMSVC
echo Building with MSVC
if not exist "bin\MSVCBuild" mkdir bin\MSVCBuild
pushd bin\MSVCBuild
xcopy %SDL2_DLL% .\ /Y
call cl -I%SDL2_Include% %CLFlags% -nologo -Zi -EHsc %SourceFiles% -Fe%OutputName% /link /LIBPATH:%SDL2_Library% SDL2.lib SDL2main.lib Shell32.lib Comdlg32.lib /subsystem:windows
popd
echo MSVC Build Complete
echo ----------------------------------------
goto CLANG

:SkipMSVC
echo MSVC not found. Skipping build with MSVC. 
echo ----------------------------------------

:CLANG
where clang >nul 2>nul
IF %ERRORLEVEL% NEQ 0 goto SkipCLANG
echo Building with CLANG
if not exist "bin\ClangBuild" mkdir bin\ClangBuild
pushd bin\ClangBuild
xcopy %SDL2_DLL% .\ /Y
call clang -I%SDL2_Include% -L%SDL2_Library% -L%SDL2_ttf_Library% %CLANGFlags% %SourceFiles% -o %OutputName% -lSDL2main -lSDL2 -lShell32 -lComdlg32 -Xlinker -subsystem:console
echo Clang Build Complete
echo ----------------------------------------
popd
goto GCC

:SkipCLANG
echo Clang not found. Skipping build with Clang. 
echo ----------------------------------------

:GCC
where gcc >nul 2>nul
IF %ERRORLEVEL% NEQ 0 goto SkipGCC
echo Building with GCC
if not exist "bin\GccBuild" mkdir bin\GccBuild
pushd bin\GccBuild
xcopy %SDL2MinGw_DLL% .\ /Y
call gcc -I%SDL2MinGw_Include% -L%SDL2MinGw_Library% -L%SDL2MinGw_ttf_Library% %GCCFlags% %SourceFiles% -o %OutputName% -w -Wl,-subsystem,windows -lmingw32 -lSDL2main -lSDL2 -lComdlg32
echo Gcc Build Complete
echo ----------------------------------------
popd
goto Finished

:SkipGCC
echo Gcc not found. Skipping build with Gcc. 
echo ----------------------------------------

:Finished
