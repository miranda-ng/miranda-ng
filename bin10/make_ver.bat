@echo off

for /F "tokens=1,2" %%i in ('svn info miranda32.sln') do call :AddBuild %%i %%j
goto :eof

:AddBuild
if (%1) == (Revision:) (
	for /F "tokens=1,2,3 delims= " %%i in (build.no) do call :WriteVer %%i %%j %2
   )

goto :eof

:WriteVer
copy m_version.h.in ..\include\m_version.h

echo #define MIRANDA_VERSION_FILEVERSION 0,%1,%2,%3                                >>..\include\m_version.h
echo #define MIRANDA_VERSION_STRING      "0.%1.%2.%3"                              >>..\include\m_version.h
echo #define MIRANDA_VERSION_DISPLAY     "0.%1.%2 alpha build #%3"                 >>..\include\m_version.h
echo #define MIRANDA_VERSION_DWORD       MIRANDA_MAKE_VERSION(0, %1, %2, %3)       >>..\include\m_version.h
echo #define MIRANDA_VERSION_CORE        MIRANDA_MAKE_VERSION(0, %1, %2, 0)        >>..\include\m_version.h
echo #define MIRANDA_VERSION_CORE_STRING "0.%1.%2.0"                               >>..\include\m_version.h
echo.                                                                              >>..\include\m_version.h
echo #endif // M_VERSION_H__                                                       >>..\include\m_version.h
goto :eof
