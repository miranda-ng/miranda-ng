@if not defined INCLUDE goto :FAIL

@setlocal

@if "%1"=="debug-5.1" goto :DEBUG_5_1

rem These should not have quotes
@set LUA_INCLUDE=Z:\c\lua-5.2.0\src
@set LUA_LIB=Z:\c\lua-5.2.0\lua5.2.lib
@set LUA_EXE=Z:\c\lua-5.2.0\lua.exe
rem This the name of the dll that can be handed to LoadLibrary. This should not have a path.
@set LUA_DLL=lua5.2.dll
@goto :DEBUG

:DEBUG_5_1
@set LUA_INCLUDE=Z:\c\lua-5.1.4\src
@set LUA_LIB=Z:\c\lua-5.1.4\lua5.1.lib
@set LUA_EXE=Z:\c\lua-5.1.4\lua.exe
@set LUA_DLL=lua5.1.dll

:DEBUG
@set DO_CL=cl.exe /nologo /c /MDd /FC /Zi /Od /W3 /WX /D_CRT_SECURE_NO_DEPRECATE /DLUA_FFI_BUILD_AS_DLL /I"msvc"
@set DO_LINK=link /nologo /debug
@set DO_MT=mt /nologo

@if "%1"=="debug" goto :COMPILE
@if "%1"=="debug-5.1" goto :COMPILE
@if "%1"=="test" goto :COMPILE
@if "%1"=="clean" goto :CLEAN
@if "%1"=="release" goto :RELEASE
@if "%1"=="test-release" goto :RELEASE

:RELEASE
@set DO_CL=cl.exe /nologo /c /MD /Ox /W3 /Zi /WX /D_CRT_SECURE_NO_DEPRECATE /DLUA_FFI_BUILD_AS_DLL /I"msvc"
@set DO_LINK=link.exe /nologo /debug
@set DO_MT=mt.exe /nologo
@goto :COMPILE

:COMPILE
"%LUA_EXE%" dynasm\dynasm.lua -LNE -D X32WIN -o call_x86.h call_x86.dasc
"%LUA_EXE%" dynasm\dynasm.lua -LNE -D X64 -o call_x64.h call_x86.dasc
"%LUA_EXE%" dynasm\dynasm.lua -LNE -D X64 -D X64WIN -o call_x64win.h call_x86.dasc
"%LUA_EXE%" dynasm\dynasm.lua -LNE -o call_arm.h call_arm.dasc
%DO_CL% /I"." /I"%LUA_INCLUDE%" /DLUA_DLL_NAME="%LUA_DLL%" call.c ctype.c ffi.c parser.c
%DO_LINK% /DLL /OUT:ffi.dll "%LUA_LIB%" *.obj
if exist ffi.dll.manifest^
    %DO_MT% -manifest ffi.dll.manifest -outputresource:"ffi.dll;2"

%DO_CL% /Gd test.c /Fo"test_cdecl.obj"
%DO_CL% /Gz test.c /Fo"test_stdcall.obj"
%DO_CL% /Gr test.c /Fo"test_fastcall.obj"
%DO_LINK% /DLL /OUT:test_cdecl.dll test_cdecl.obj
%DO_LINK% /DLL /OUT:test_stdcall.dll test_stdcall.obj
%DO_LINK% /DLL /OUT:test_fastcall.dll test_fastcall.obj
if exist test_cdecl.dll.manifest^
    %DO_MT% -manifest test_cdecl.dll.manifest -outputresource:"test_cdecl.dll;2"
if exist test_stdcall.dll.manifest^
    %DO_MT% -manifest test_stdcall.dll.manifest -outputresource:"test_stdcall.dll;2"
if exist test_fastcall.dll.manifest^
    %DO_MT% -manifest test_fastcall.dll.manifest -outputresource:"test_fastcall.dll;2"

@if "%1"=="test" "%LUA_EXE%" test.lua
@if "%1"=="test-5.2" "%LUA_EXE%" test.lua
@if "%1"=="test-release" "%LUA_EXE%" test.lua
@goto :CLEAN_OBJ

:CLEAN
del *.dll
:CLEAN_OBJ
del *.obj *.manifest
@goto :END

:FAIL
@echo You must open a "Visual Studio .NET Command Prompt" to run this script
:END

