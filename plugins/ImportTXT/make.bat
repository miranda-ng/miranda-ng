set COMPDIR=-$A8 -$D- -$J+ -$L- -$O+ -$Q- -$R- -$Y- -$C-
set INCDIR="c:\program files\borland\delphi7\lib\kol;..\..\include;"
set OUTDIR="..\plugins"
set DCUDIR="tmp"
md %DCUDIR% 2>nul
brcc32 -foImpTxt_Ver.res ImpTxt_Ver.rc
brcc32 -foImpTxtDlg.res ImpTxtDlg.rc
brcc32 -foImpTxtWiz.res ImpTxtWiz.rc
dcc32 -B -CG -U%INCDIR% -R%INCDIR% -I%INCDIR% -E%OUTDIR% -LE%DCUDIR% -LN%DCUDIR% -N%DCUDIR% %COMPDIR% importtxt.dpr
rd /q /s %DCUDIR%