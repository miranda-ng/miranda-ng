SET PROJ_DIR=Chess4Net_MI
SET CHESS4NET_DIR=%PROJ_DIR%\Chess4Net

rem -= Clean =-

RMDIR /S /Q ..\bin\%PROJ_DIR%

rem -= Environment =-

MD ..\bin\%PROJ_DIR%
MD ..\bin\%PROJ_DIR%\Chess4Net

rem -= Compilation =-

dcc32 -B Chess4Net_MI.dpr

rem -= Build =-

COPY ..\Readme.txt ..\bin\%PROJ_DIR%
COPY ..\Readme_RU.txt ..\bin\%PROJ_DIR%

COPY ..\Lang.ini ..\bin\%CHESS4NET_DIR%

COPY ..\Build\Chigorin.mov ..\bin\%CHESS4NET_DIR%
COPY ..\Build\Chigorin.pos ..\bin\%CHESS4NET_DIR%

COPY ..\Build\eco.mov ..\bin\%CHESS4NET_DIR%
COPY ..\Build\eco.pos ..\bin\%CHESS4NET_DIR%

COPY ..\Build\Fischer.mov ..\bin\%CHESS4NET_DIR%
COPY ..\Build\Fischer.pos ..\bin\%CHESS4NET_DIR%

COPY ..\Build\Tal.mov ..\bin\%CHESS4NET_DIR%
COPY ..\Build\Tal.pos ..\bin\%CHESS4NET_DIR%

MOVE ..\bin\Chess4Net_MI.dll ..\bin\%PROJ_DIR%
