{}
const
  defProtoText = '%artist% - %title%';
const
  opt_ModStatus:PAnsiChar = 'module/protocol';

  opt_histmask :PAnsiChar = 'historymask';
  opt_prototext:PAnsiChar = 'prototext';

function GetModStatus:integer;
begin
  result:=DBReadByte(0,PluginShort,opt_ModStatus,1);
end;

procedure SetModStatus(stat:integer);
begin
  DBWriteByte(0,PluginShort,opt_ModStatus,stat);
end;

procedure ReadOptions;
begin
  HistMask :=DBReadWord   (0,PluginShort,opt_histmask,0);
  ProtoText:=DBReadUnicode(0,PluginShort,opt_prototext,defProtoText);
end;

procedure WriteOptions;
begin
  DBWriteWord   (0,PluginShort,opt_histmask ,HistMask);
  DBWriteUnicode(0,PluginShort,opt_prototext,ProtoText);
end;

procedure FreeOptions;
begin
  mFreeMem(ProtoText);
end;
