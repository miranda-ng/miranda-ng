{}
const
  opt_LoCaseType:PAnsiChar = 'locase';
  opt_FSPrec    :PAnsiChar = 'precision';
  opt_FSizePost :PAnsiChar = 'fsizepost';
  opt_FSizeMode :PAnsiChar = 'fsizemode';
  opt_WriteCBR  :PAnsiChar = 'writecbr';
  opt_ReplaceSpc:PAnsiChar = 'replacespc';
  opt_PlayerCaps:PAnsiChar = 'playercaps';
  opt_ExportText:PAnsiChar = 'exporttext';

  opt_export    :PAnsiChar = 'template/export';
  spref = 'strings/';

procedure LoadOpt;
var
  setting:array [0..63] of AnsiChar;
begin
  PlayerCaps :=DBReadByte   (0,PluginShort,opt_PlayerCaps,0);
  LoCaseType :=DBReadByte   (0,PluginShort,opt_LoCaseType,BST_UNCHECKED);
  ReplaceSpc :=DBReadByte   (0,PluginShort,opt_ReplaceSpc,BST_CHECKED);
  FSPrecision:=DBReadByte   (0,PluginShort,opt_FSPrec    ,0);
  FSizePost  :=DBReadByte   (0,PluginShort,opt_FSizePost ,0);
  FSizeMode  :=DBReadDWord  (0,PluginShort,opt_FSizeMode ,1);
  WriteCBR   :=DBReadByte   (0,PluginShort,opt_WriteCBR  ,0);
  if DBGetSettingType(0,PluginShort,opt_ExportText)=DBVT_DELETED then
  begin
    IntToStr(StrCopyE(setting,spref),DBReadWord(0,PluginShort,opt_export,3));
    ExportText:=DBReadUnicode(0,PluginShort,setting,nil);
  end
  else
    ExportText:=DBReadUnicode(0,PluginShort,opt_ExportText);
end;

procedure SaveOpt;
begin
  DBWriteByte   (0,PluginShort,opt_PlayerCaps,PlayerCaps);
  DBWriteByte   (0,PluginShort,opt_LoCaseType,LoCaseType);
  DBWriteByte   (0,PluginShort,opt_ReplaceSpc,ReplaceSpc);
  DBWriteByte   (0,PluginShort,opt_FSPrec    ,FSPrecision);
  DBWriteByte   (0,PluginShort,opt_FSizePost ,FSizePost);
  DBWriteDWord  (0,PluginShort,opt_FSizeMode ,FSizeMode);
  DBWriteByte   (0,PluginShort,opt_WriteCBR  ,WriteCBR);
  DBWriteUnicode(0,PluginShort,opt_ExportText,ExportText);
end;

procedure FreeOpt;
begin
  mFreeMem(ExportText);
end;
