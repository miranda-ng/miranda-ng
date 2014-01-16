{statistic variables}
var
  SortMode:dword;
  ReportMask:dword;
  ReportItems:cardinal;
  Direction:cardinal;
  RunReport:cardinal;
  DoAddExt:cardinal;
  AutoSort:cardinal;
  LastSort:dword;
const
  StatName  :PAnsiChar=nil;
  ReportName:PAnsiChar=nil;
  TmplName  :PAnsiChar=nil;
var
  hPackLog,
  hMakeReport,
  hAddToLog,
  plStatusHook,
  sic,
  hMenuReport:THANDLE;
