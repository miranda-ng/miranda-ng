:@echo off

:if %1 == "" goto USAGE
:if %2 NEQ "" goto USAGE

call _publishbin.cmd "Release" %1
call _publishbin.cmd "Release Unicode" %1
call _publishsrc.cmd %1
goto END

:USAGE
echo.
echo ===============================================================================
echo UserInfoEx Postbuilt Script for automatic preparation of published binaries.
echo.
echo USAGE: _publishall [VERSION]
echo.
echo Example: _publishall "0.8.3.2"
echo.
echo ===============================================================================
echo.

:END