{}
const
  opt_ModStatus  :PAnsiChar = 'module/statuses';

  opt_UseStatus  :PAnsiChar = 'usestatus';
  opt_UseExtStat :PAnsiChar = 'useextstat';
  opt_UseMsgs    :PAnsiChar = 'usemsgs';
  opt_XStatusSet :PAnsiChar = 'xstatusset';
  opt_KeepStatus :PAnsiChar = 'keepstatus';
  opt_Independed :PAnsiChar = 'independed';
  opt_ClearXStat :PAnsiChar = 'clearxstat';
  opt_SimplMode  :PAnsiChar = 'simplemode';
  opt_ListeningTo:PAnsiChar = 'listeningto';

function GetModStatus:integer;
begin
  result:=DBReadByte(0,PluginShort,opt_ModStatus,1);
end;

procedure SetModStatus(stat:integer);
begin
  DBWriteByte(0,PluginShort,opt_ModStatus,stat);
end;

procedure LoadOpt;
begin
  SimpleMode    :=DBReadByte(0,PluginShort,opt_SimplMode  ,BST_CHECKED);
  UseStatus     :=DBReadByte(0,PluginShort,opt_UseStatus  ,BST_UNCHECKED);
  UseExtStatus  :=DBReadByte(0,PluginShort,opt_UseExtStat ,BST_CHECKED);
  UseMessages   :=DBReadByte(0,PluginShort,opt_UseMsgs    ,BST_CHECKED);
  KeepStatus    :=DBReadByte(0,PluginShort,opt_KeepStatus ,BST_UNCHECKED);
  XIndepended   :=DBReadByte(0,PluginShort,opt_Independed ,BST_CHECKED);
  ClearXStat    :=DBReadByte(0,PluginShort,opt_ClearXStat ,BST_UNCHECKED);
  UseListeningTo:=DBReadByte(0,PluginShort,opt_ListeningTo,BST_CHECKED);
  XStatusSet    :=DBReadByte(0,PluginShort,opt_XStatusSet ,1);
end;

procedure SaveOpt;
begin
  DBWriteByte(0,PluginShort,opt_SimplMode  ,SimpleMode);
  DBWriteByte(0,PluginShort,opt_UseStatus  ,UseStatus);
  DBWriteByte(0,PluginShort,opt_UseExtStat ,UseExtStatus);
  DBWriteByte(0,PluginShort,opt_UseMsgs    ,UseMessages);
  DBWriteByte(0,PluginShort,opt_KeepStatus ,KeepStatus);
  DBWriteByte(0,PluginShort,opt_Independed ,XIndepended);
  DBWriteByte(0,PluginShort,opt_ClearXStat ,ClearXStat);
  DBWriteByte(0,PluginShort,opt_ListeningTo,UseListeningTo);
  DBWriteByte(0,PluginShort,opt_XStatusSet ,XStatusSet);
end;
