{Database options}
const
  DefaultXStatus = $080B; // TV + Music
const
  defhotkey    = (HOTKEYF_CONTROL+HOTKEYF_ALT  )*256+VK_F5;
  definshotkey = (HOTKEYF_CONTROL+HOTKEYF_SHIFT)*256+VK_F7;

const
  defcoverpaths = 'cover.jpg'#13#10'..\cover.jpg'#13#10'*.jpg'#13#10'..\*.jpg';

const
  WATFormats:PAnsiChar = 'formats/';
const
  WATPlayers   :PAnsiChar = 'players/';

const
  opt_disable  :PAnsiChar = 'disableplugin';

  opt_InsHotKey :PAnsiChar = 'inshotkey';
  opt_HotKey    :PAnsiChar = 'hotkey';
  opt_Timer     :PAnsiChar = 'timer';
  opt_UserCP    :PAnsiChar = 'usercp';
  opt_CheckTime :PAnsiChar = 'checktime';
  opt_coverpaths:PAnsiChar = 'coverpaths';
  opt_Implantant:PAnsiChar = 'useimplantant';
  opt_KeepOld   :PAnsiChar = 'keepold';
  opt_mmkeyemu  :PAnsiChar = 'mmkeyemu';
  opt_CheckAll  :PAnsiChar = 'checkall';
  opt_CheckUnkn :PAnsiChar = 'checkunknown';

procedure _loadopt;
begin
  DisablePlugin:=DBReadByte(0,PluginShort,opt_disable,0);
  if DisablePlugin<>dsPermanent then
    DisablePlugin:=dsEnabled;

  inshotkey    :=DBReadWord(0,PluginShort,opt_InsHotKey ,definshotkey);
  globhotkey   :=DBReadWord(0,PluginShort,opt_HotKey    ,defhotkey);
  CheckTime    :=DBReadByte(0,PluginShort,opt_CheckTime ,BST_CHECKED);
  UseImplant   :=DBReadByte(0,PluginShort,opt_Implantant,BST_UNCHECKED);
  KeepOld      :=DBReadByte(0,PluginShort,opt_KeepOld   ,BST_UNCHECKED);
  CheckAll     :=DBReadByte(0,PluginShort,opt_CheckAll  ,BST_UNCHECKED);
  CheckUnknown :=DBReadByte(0,PluginShort,opt_CheckUnkn ,BST_UNCHECKED);
  mTimer       :=DBReadWord(0,PluginShort,opt_Timer     ,3000);
  if mTimer<500 then
    mTimer:=mTimer*1000;
  UserCP       :=DBReadWord(0,PluginShort,opt_UserCP    ,CP_ACP);
  CoverPaths   :=DBReadUnicode(0,PluginShort,opt_coverpaths,defcoverpaths);

  mmkeyemu     :=DBReadByte (0,PluginShort,opt_mmkeyemu  ,BST_UNCHECKED);
end;

procedure _saveopt;
begin
  DBWriteWord(0,PluginShort,opt_InsHotKey ,inshotkey);
  DBWriteWord(0,PluginShort,opt_HotKey    ,globhotkey);
  DBWriteByte(0,PluginShort,opt_CheckTime ,CheckTime);
  DBWriteByte(0,PluginShort,opt_Implantant,UseImplant);
  DBWriteByte(0,PluginShort,opt_KeepOld   ,KeepOld);
  DBWriteByte(0,PluginShort,opt_CheckAll  ,CheckAll);
  DBWriteByte(0,PluginShort,opt_CheckUnkn ,CheckUnknown);
  DBWriteWord(0,PluginShort,opt_Timer     ,mTimer);
  DBWriteWord(0,PluginShort,opt_UserCP    ,UserCP);

  DBWriteUnicode(0,PluginShort,opt_coverpaths,CoverPaths);

  DBWriteByte (0,PluginShort,opt_mmkeyemu  ,mmkeyemu);
end;

function enumwp(desc:PAnsiChar;lParam:LPARAM):bool; stdcall;
var
  i:integer;
  buf:array [0..63] of AnsiChar;
begin
  i:=CallService(MS_WAT_PLAYER,WAT_ACT_GETSTATUS,tlparam(desc));
  if i=WAT_RES_ENABLED then
    i:=1
  else
    i:=0;
  StrCopy(StrCopyE(buf,WATPlayers),desc);
  DBWriteByte(0,PluginShort,buf,i);
  result:=true;
end;

procedure WritePlayers;
begin
  EnumPlayers(@enumwp,0);
end;

function enumrp(desc:PAnsiChar;lParam:LPARAM):bool; stdcall;
var
  i:integer;
  buf:array [0..63] of AnsiChar;
begin
  StrCopy(StrCopyE(buf,WATPlayers),desc);
  i:=DBReadByte(0,PluginShort,buf,1);
  if i=1 then
    i:=WAT_ACT_ENABLE
  else
    i:=WAT_ACT_DISABLE;
  CallService(MS_WAT_PLAYER,i,tlparam(desc));
  result:=true;
end;

procedure ReadPlayers;
begin
  EnumPlayers(@enumrp,0);
{!!  p:=DBReadString(0,PluginShort,opt_DefPlayer,nil);
  CallService(MS_WAT_PLAYER,WAT_ACT_SETACTIVE,TLPARAM(p));
  mFreeMem(p);
}
end;

function enumwf(ext:PAnsiChar;lParam:LPARAM):bool; stdcall;
var
  i:integer;
  buf:array [0..63] of AnsiChar;
begin
  i:=CallService(MS_WAT_FORMAT,WAT_ACT_GETSTATUS,tlparam(ext));
  if i=WAT_RES_ENABLED then
    i:=1
  else
    i:=0;
  StrCopy(StrCopyE(buf,WATFormats),ext);
  DBWriteByte(0,PluginShort,buf,i);
  result:=true;
end;

procedure WriteFormats;
begin
  EnumFormats(@enumwf,0);
end;

function enumrf(ext:PAnsiChar;lParam:LPARAM):bool; stdcall;
var
  i:integer;
  buf:array [0..63] of AnsiChar;
begin
  StrCopy(StrCopyE(buf,WATFormats),ext);
  i:=DBReadByte(0,PluginShort,buf,1);
  if i=1 then
    i:=WAT_ACT_ENABLE
  else
    i:=WAT_ACT_DISABLE;
  CallService(MS_WAT_FORMAT,i,tlparam(ext));
  result:=true;
end;

procedure ReadFormats;
begin
  EnumFormats(@enumrf,0);
end;

procedure SaveOpt;
begin
  _saveopt;
  WriteFormats;
  WritePlayers;
end;

procedure LoadOpt;
begin
  _loadopt;
  ReadPlayers;
  ReadFormats;
end;
