@echo off
rem ---------------------------------------------------------------------------
rem  Size-optimized MSVC build (32-bit).
rem
rem  Produces build\pikachu.exe with the tiny CRT in mini_crt.cpp
rem  and no default libraries, so the result imports only KERNEL32, USER32,
rem  GDI32, WINMM and DSOUND.
rem
rem  Run from anywhere; the script locates Visual Studio with vswhere.
rem ---------------------------------------------------------------------------
setlocal
cd /d "%~dp0"

if not exist "assets" (
    echo [!] assets\ is missing.
    exit /b 1
)

set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" set "VSWHERE=%ProgramFiles%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" (
    echo [!] vswhere.exe not found. Run this from a "Developer Command Prompt for VS".
    exit /b 1
)

set "VSPATH="
for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VSPATH=%%i"
if not defined VSPATH (
    echo [!] Visual Studio with the C++ x86 tools was not found.
    exit /b 1
)

call "%VSPATH%\VC\Auxiliary\Build\vcvars32.bat" >nul
if errorlevel 1 exit /b 1

if not exist build mkdir build
if not exist build_msvc mkdir build_msvc

rc /nologo /c 65001 /I src /fo build_msvc\pikachu.res res\pikachu.rc
if errorlevel 1 exit /b 1

cl /nologo /O1 /Gy /GR- /EHs-c- /GS- /Gs999999 /MT /DPIKA_MINI_CRT ^
   /D_CRT_SECURE_NO_WARNINGS /utf-8 /std:c++17 /Isrc ^
   /Fo:build_msvc\ /Fd:build_msvc\ src\*.cpp build_msvc\pikachu.res ^
   /link /SUBSYSTEM:WINDOWS /NODEFAULTLIB /ENTRY:WinMainCRTStartup ^
   /OPT:REF /OPT:ICF /INCREMENTAL:NO /OUT:build\pikachu.exe ^
   kernel32.lib user32.lib gdi32.lib winmm.lib dsound.lib
if errorlevel 1 exit /b 1

echo BUILD OK: build\pikachu.exe
