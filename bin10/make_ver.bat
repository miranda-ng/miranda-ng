rem @echo off

for /F "tokens=2,3" %%i in (..\include\m_version.h) do if "%%i"=="MIRANDA_VERSION_FILEVERSION" (set OldVer=%%j)
echo %OldVer%

for /F "tokens=1,2" %%i in ('svn info miranda32.sln') do call :AddBuild %%i %%j
goto :eof

:AddBuild
if (%1) == (Revision:) (
	for /F "tokens=1,2,3 delims= " %%i in (build.no) do call :WriteVer %%i %%j %%k %2
   )

goto :eof

:WriteVer
if "%OldVer%" == "%1,%2,%3,%4" (goto :eof)

copy m_version.h.in ..\include\m_version.h

echo #define MIRANDA_VERSION_FILEVERSION %1,%2,%3,%4                               >>..\include\m_version.h
echo #define MIRANDA_VERSION_STRING      "%1.%2.%3.%4"                             >>..\include\m_version.h
echo #define MIRANDA_VERSION_DISPLAY     "%1.%2.%3 alpha build #%4"                >>..\include\m_version.h
echo #define MIRANDA_VERSION_DWORD       MIRANDA_MAKE_VERSION(%1, %2, %3, %4)      >>..\include\m_version.h
echo #define MIRANDA_VERSION_CORE        MIRANDA_MAKE_VERSION(%1, %2, %3, 0)       >>..\include\m_version.h
echo #define MIRANDA_VERSION_CORE_STRING "%1.%2.%3.0"                              >>..\include\m_version.h
echo.                                                                              >>..\include\m_version.h
echo #endif // M_VERSION_H__                                                       >>..\include\m_version.h
goto :eof
