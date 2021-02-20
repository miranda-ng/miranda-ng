@echo off

cd /d "%TEMP%"
rd /s /q openssl
mkdir openssl
cd /d openssl

perl "%OPENSSL_ROOT_DIR%\Configure" shared VC-WIN64A

call %VS141COMNTOOLS%\..\..\VC\Auxiliary\Build\vcvars64.bat 
nmake 

copy libcrypto-1_1.mir %MIRANDA_BUILD_ROOT%\pre-build\bin15\64\Libs
copy libssl-1_1.mir    %MIRANDA_BUILD_ROOT%\pre-build\bin15\64\Libs

copy libcrypto-1_1.pdb %MIRANDA_BUILD_ROOT%\pre-symbols\bin15\64
copy libssl-1_1.pdb    %MIRANDA_BUILD_ROOT%\pre-symbols\bin15\64

copy libcrypto-1_1.mir %MIRANDA_DEV_ROOT%\bin15\Release64\Libs
copy libcrypto-1_1.pdb %MIRANDA_DEV_ROOT%\bin15\Release64\Libs
copy libssl-1_1.mir    %MIRANDA_DEV_ROOT%\bin15\Release64\Libs
copy libssl-1_1.pdb    %MIRANDA_DEV_ROOT%\bin15\Release64\Libs

copy libcrypto-1_1.mir %MIRANDA_DEV_ROOT%\bin15\Debug64\Libs
copy libcrypto-1_1.pdb %MIRANDA_DEV_ROOT%\bin15\Debug64\Libs
copy libssl-1_1.mir    %MIRANDA_DEV_ROOT%\bin15\Debug64\Libs
copy libssl-1_1.pdb    %MIRANDA_DEV_ROOT%\bin15\Debug64\Libs

copy libcrypto-1_1.mir %MIRANDA_DEV_ROOT%\bin16\Release64\Libs
copy libcrypto-1_1.pdb %MIRANDA_DEV_ROOT%\bin16\Release64\Libs
copy libssl-1_1.mir    %MIRANDA_DEV_ROOT%\bin16\Release64\Libs
copy libssl-1_1.pdb    %MIRANDA_DEV_ROOT%\bin16\Release64\Libs

copy libcrypto-1_1.mir %MIRANDA_DEV_ROOT%\bin16\Debug64\Libs
copy libcrypto-1_1.pdb %MIRANDA_DEV_ROOT%\bin16\Debug64\Libs
copy libssl-1_1.mir    %MIRANDA_DEV_ROOT%\bin16\Debug64\Libs
copy libssl-1_1.pdb    %MIRANDA_DEV_ROOT%\bin16\Debug64\Libs
