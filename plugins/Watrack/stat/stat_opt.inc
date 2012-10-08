{statistic load/save options}
const
  opt_ModStatus :PAnsiChar = 'module/statistic';

  opt_StatName  :PAnsiChar = 'report/statname';
  opt_RepName   :PAnsiChar = 'report/repname';
  opt_TmplName  :PAnsiChar = 'report/tmplname';
  opt_SortMode  :PAnsiChar = 'report/sortmode';
  opt_ReportMask:PAnsiChar = 'report/reportmask';
  opt_ReportItem:PAnsiChar = 'report/reportitems';
  opt_Direction :PAnsiChar = 'report/direction';
  opt_RunReport :PAnsiChar = 'report/runreport';
  opt_AddExt    :PAnsiChar = 'report/addext';
  opt_AutoSort  :PAnsiChar = 'report/autosort';
  opt_LastSort  :PAnsiChar = 'report/lastsort';

function GetModStatus:integer;
begin
  result:=DBReadByte(0,PluginShort,opt_ModStatus,1);
end;

procedure SetModStatus(stat:integer);
begin
  DBWriteByte(0,PluginShort,opt_ModStatus,stat);
end;

procedure loadstat;
begin
  ReportName :=DBReadString(0,PluginShort,opt_RepName ,nil);
  StatName   :=DBReadString(0,PluginShort,opt_StatName,nil);
  TmplName   :=DBReadString(0,PluginShort,opt_TmplName,nil);
  DoAddExt   :=DBReadByte  (0,PluginShort,opt_AddExt    ,BST_CHECKED);
  RunReport  :=DBReadByte  (0,PluginShort,opt_RunReport ,BST_UNCHECKED);
  Direction  :=DBReadByte  (0,PluginShort,opt_Direction ,smDirect);
  SortMode   :=DBReadByte  (0,PluginShort,opt_SortMode  ,stArtist);
  ReportItems:=DBReadWord  (0,PluginShort,opt_ReportItem,10);
  ReportMask :=DBReadWord  (0,PluginShort,opt_ReportMask,$FFFF);
  AutoSort   :=DBReadByte  (0,PluginShort,opt_AutoSort  ,1);
  LastSort   :=DBReadDWord (0,PluginShort,opt_LastSort  ,0);
end;

procedure savestat;
begin
  DBWriteString(0,PluginShort,opt_RepName ,ReportName);
  DBWriteString(0,PluginShort,opt_StatName,StatName);
  DBWriteString(0,PluginShort,opt_TmplName,TmplName);
  DBWriteByte  (0,PluginShort,opt_AddExt     ,DoAddExt);
  DBWriteByte  (0,PluginShort,opt_RunReport  ,RunReport);
  DBWriteByte  (0,PluginShort,opt_Direction  ,Direction);
  DBWriteByte  (0,PluginShort,opt_SortMode   ,SortMode);
  DBWriteWord  (0,PluginShort,opt_ReportItem ,ReportItems);
  DBWriteWord  (0,PluginShort,opt_ReportMask ,ReportMask);
  DBWriteByte  (0,PluginShort,opt_AutoSort   ,AutoSort);
//  DBWriteDWord (0,PluginShort,opt_LastSort   ,LastSort);
end;

procedure FreeStat;
begin
  mFreeMem(ReportName);
  mFreeMem(StatName);
  mFreeMem(TmplName);
end;
