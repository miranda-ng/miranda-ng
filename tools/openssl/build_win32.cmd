@echo off

cd /d "%TEMP%"
rd /s /q openssl
mkdir openssl
cd /d openssl

set LDFLAGS=/nologo /debug /SUBSYSTEM:CONSOLE",5.01"
perl "%OPENSSL_ROOT_DIR%\Configure" shared VC-WIN32 /D\"_USING_V110_SDK71_\" /D\"_WIN32_WINNT=0x0501\"

call %VS141COMNTOOLS%\..\..\VC\Auxiliary\Build\vcvars32.bat 
nmake 

copy libcrypto-1_1.mir %MIRANDA_BUILD_ROOT%\pre-build\bin15\32\Libs
copy libssl-1_1.mir    %MIRANDA_BUILD_ROOT%\pre-build\bin15\32\Libs

copy libcrypto-1_1.pdb %MIRANDA_BUILD_ROOT%\pre-symbols\bin15\32
copy libssl-1_1.pdb    %MIRANDA_BUILD_ROOT%\pre-symbols\bin15\32

copy libcrypto-1_1.mir %MIRANDA_DEV_ROOT%\bin15\Release\Libs
copy libcrypto-1_1.pdb %MIRANDA_DEV_ROOT%\bin15\Release\Libs
copy libssl-1_1.mir    %MIRANDA_DEV_ROOT%\bin15\Release\Libs
copy libssl-1_1.pdb    %MIRANDA_DEV_ROOT%\bin15\Release\Libs

copy libcrypto-1_1.mir %MIRANDA_DEV_ROOT%\bin15\Debug\Libs
copy libcrypto-1_1.pdb %MIRANDA_DEV_ROOT%\bin15\Debug\Libs
copy libssl-1_1.mir    %MIRANDA_DEV_ROOT%\bin15\Debug\Libs
copy libssl-1_1.pdb    %MIRANDA_DEV_ROOT%\bin15\Debug\Libs

copy libcrypto-1_1.mir %MIRANDA_DEV_ROOT%\bin16\Release\Libs
copy libcrypto-1_1.pdb %MIRANDA_DEV_ROOT%\bin16\Release\Libs
copy libssl-1_1.mir    %MIRANDA_DEV_ROOT%\bin16\Release\Libs
copy libssl-1_1.pdb    %MIRANDA_DEV_ROOT%\bin16\Release\Libs

copy libcrypto-1_1.mir %MIRANDA_DEV_ROOT%\bin16\Debug\Libs
copy libcrypto-1_1.pdb %MIRANDA_DEV_ROOT%\bin16\Debug\Libs
copy libssl-1_1.mir    %MIRANDA_DEV_ROOT%\bin16\Debug\Libs
copy libssl-1_1.pdb    %MIRANDA_DEV_ROOT%\bin16\Debug\Libs

copy libcrypto-1_1.mir %MIRANDA_DEV_ROOT%\bin17\Release\Libs
copy libcrypto-1_1.pdb %MIRANDA_DEV_ROOT%\bin17\Release\Libs
copy libssl-1_1.mir    %MIRANDA_DEV_ROOT%\bin17\Release\Libs
copy libssl-1_1.pdb    %MIRANDA_DEV_ROOT%\bin17\Release\Libs

copy libcrypto-1_1.mir %MIRANDA_DEV_ROOT%\bin17\Debug\Libs
copy libcrypto-1_1.pdb %MIRANDA_DEV_ROOT%\bin17\Debug\Libs
copy libssl-1_1.mir    %MIRANDA_DEV_ROOT%\bin17\Debug\Libs
copy libssl-1_1.pdb    %MIRANDA_DEV_ROOT%\bin17\Debug\Libs
