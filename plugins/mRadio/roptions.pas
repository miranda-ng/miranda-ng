{Different settings}
unit roptions;

interface

//----- Presets -----

type
  tPreset = record
    name  :PWideChar;
    preset:array [0..9] of shortint;
  end;
var
  Presets: array of tPreset;
  OldEQPreset:integer;

procedure LoadPresets;
procedure SavePresets;
procedure FreePresets;

procedure FreeSettings;
procedure LoadSettings;
procedure SaveCommonSettings;
procedure SaveTechSettings;


implementation

uses
  windows,
  common,dbsettings,mircontacts,m_api,
  rglobal;

const
  optRecPath    :PAnsiChar = 'RecordPath';
  optStatusTmpl :PAnsiChar = 'StatusTmpl';

  optLoop       :PAnsiChar = 'Loop';
  optShuffle    :PAnsiChar = 'Shuffle';
  optPlayFirst  :PAnsiChar = 'PlayFromFirst'; 
  optContRec    :PAnsiChar = 'ContRec';
  optConnect    :PAnsiChar = 'AutoConnect';
  optAutoMute   :PAnsiChar = 'AutoMute';
  optOffline    :PAnsiChar = 'asOffline';

  optEQ_OFF     :PAnsiChar = 'eqoff';
  optForcedMono :PAnsiChar = 'ForcedMono';
  optPreBuf     :PAnsiChar = 'PreBuf';
  optBuffer     :PAnsiChar = 'Buffer';
  optTimeout    :PAnsiChar = 'Timeout';
  optNumTries   :PAnsiChar = 'NumTries';
  optDevice     :PAnsichar = 'Device';

{
const
  optVolume     :PAnsiChar = 'Volume';
}
//----- Presets -----

const
  optOldPreset:pAnsiChar='oldpreset';
  optPresets  :pAnsiChar='numpreset';
  optPreset   :pAnsiChar='preset_';
  optPreDescr :pAnsiChar='predescr_';

const
  PresetMax = 17;

procedure InitPresets;
type
  tP = array [0..9] of AnsiChar;
begin
  StrDupW(Presets[00].name,'Acoustic' ); tP(Presets[00].preset):=#$01#$02#$00#$00#$00#$00#$00#$01#$01#$03;
  StrDupW(Presets[01].name,'Blues'    ); tP(Presets[01].preset):=#$01#$02#$01#$00#$00#$00#$00#$00#$00#$FE;
  StrDupW(Presets[02].name,'Classical'); tP(Presets[02].preset):=#$01#$04#$01#$00#$00#$00#$00#$00#$01#$01;
  StrDupW(Presets[03].name,'Country'  ); tP(Presets[03].preset):=#$00#$00#$01#$01#$00#$00#$01#$01#$01#$01;
  StrDupW(Presets[04].name,'Dance'    ); tP(Presets[04].preset):=#$04#$03#$02#$01#$FF#$FF#$02#$04#$05#$05;
  StrDupW(Presets[05].name,'Folk'     ); tP(Presets[05].preset):=#$FF#$00#$00#$01#$01#$00#$01#$01#$01#$00;
  StrDupW(Presets[06].name,'Grunge'   ); tP(Presets[06].preset):=#$01#$00#$FF#$00#$00#$02#$01#$FF#$FE#$FD;
  StrDupW(Presets[07].name,'Jazz'     ); tP(Presets[07].preset):=#$00#$01#$02#$03#$03#$01#$01#$03#$04#$05;
  StrDupW(Presets[08].name,'Metall'   ); tP(Presets[08].preset):=#$FE#$00#$00#$00#$00#$03#$02#$04#$06#$05;
  StrDupW(Presets[09].name,'New Age'  ); tP(Presets[09].preset):=#$03#$03#$00#$00#$00#$00#$00#$00#$02#$02;
  StrDupW(Presets[10].name,'Opera'    ); tP(Presets[10].preset):=#$00#$01#$02#$04#$01#$02#$00#$00#$00#$00;
  StrDupW(Presets[11].name,'Rap'      ); tP(Presets[11].preset):=#$00#$04#$04#$00#$00#$00#$01#$04#$05#$07;
  StrDupW(Presets[12].name,'Reggae'   ); tP(Presets[12].preset):=#$03#$00#$FD#$00#$05#$00#$02#$03#$04#$05;
  StrDupW(Presets[13].name,'Rock'     ); tP(Presets[13].preset):=#$00#$02#$04#$00#$00#$00#$02#$04#$05#$06;
  StrDupW(Presets[14].name,'Speech'   ); tP(Presets[14].preset):=#$FE#$02#$00#$00#$00#$00#$FF#$FE#$FD#$FC;
  StrDupW(Presets[15].name,'Swing'    ); tP(Presets[15].preset):=#$FF#$00#$00#$02#$02#$00#$02#$02#$03#$03;
  StrDupW(Presets[16].name,'Techno'   ); tP(Presets[16].preset):=#$05#$08#$FF#$FE#$FD#$FF#$04#$06#$06#$06;
end;

procedure LoadPresets;
var
  num,preset,descr:array [0..63] of AnsiChar;
  i:integer;
  p,pd:pAnsiChar;
begin
  p :=StrCopyE(preset,optPreset);
  pd:=StrCopyE(descr ,optPreDescr);

  i:=DBReadByte(0,PluginName,optPresets);
  if i=0 then
  begin
    SetLength(Presets,PresetMax);
    InitPresets;
    OldEQPreset:=-1;
  end
  else
  begin
    OldEQPreset:=integer(DBReadByte(0,PluginName,optOldPreset,byte(-1)));
    SetLength(Presets,i);
    for i:=0 to HIGH(Presets) do
    begin
      StrCopy(p ,IntToStr(num,i));      DBReadStruct (0,PluginName,preset,@Presets[i].preset,10);
      StrCopy(pd,num); Presets[i].name:=DBReadUnicode(0,PluginName,descr);
    end;
  end;
end;

procedure SavePresets;
var
  num,preset,descr:array [0..63] of AnsiChar;
  i,j:integer;
  p,pd:pAnsiChar;
begin
  p :=StrCopyE(preset,optPreset);
  pd:=StrCopyE(descr ,optPreDescr);
  j:=DBReadByte(0,PluginName,optPresets);
  for i:=0 to HIGH(Presets) do
  begin
    StrCopy(p ,IntToStr(num,i)); DBWriteStruct (0,PluginName,preset,@Presets[i].preset,10);
    StrCopy(pd,num);             DBWriteUnicode(0,PluginName,descr ,Presets[i].name);
  end;
  while j>Length(Presets) do
  begin
    dec(j);
    StrCopy(p ,IntToStr(num,j)); DBDeleteSetting(0,PluginName,preset);
    StrCopy(pd,num);             DBDeleteSetting(0,PluginName,descr);
  end;
  DBWriteByte(0,PluginName,optOldPreset,OldEQPreset);
  DBWriteByte(0,PluginName,optPresets  ,Length(Presets));
end;

procedure FreePresets;
var
  i:integer;
begin
  for i:=0 to HIGH(Presets) do
    mFreeMem(Presets[i].name);
end;

//----- Common settings -----

procedure FreeSettings;
begin
  mFreeMem(recpath);
  mFreeMem(StatusTmpl);
  mFreeMem(usedevice);
end;

procedure LoadSettings;
var
  szTemp:array [0..7] of AnsiChar;
  i:integer;
begin
  //-- record
  recpath:=DBReadUnicode(0,PluginName,optRecPath);
  if recpath=nil then
    recpath:=GetDefaultRecPath;

  //-- mRadio settings
  doLoop    :=DBReadByte(0,PluginName,optLoop);
  doShuffle :=DBReadByte(0,PluginName,optShuffle);
  doContRec :=DBReadByte(0,PluginName,optContRec);
  PlayFirst :=DBReadByte(0,PluginName,optPlayFirst);
  AuConnect :=DBReadByte(0,PluginName,optConnect);
  AuMute    :=DBReadByte(0,PluginName,optAutoMute);
  AsOffline :=DBReadByte(0,PluginName,optOffline);
  StatusTmpl:=DBReadUnicode(0,PluginName,optStatusTmpl,'%radio_title%');

  //-- Tech settings
  usedevice :=DBReadString(0,PluginName,optDevice);

  sPreBuf   :=DBReadWord(0,PluginName,optPreBuf ,75);
  sBuffer   :=DBReadWord(0,PluginName,optBuffer ,5000);
  sTimeout  :=DBReadWord(0,PluginName,optTimeout,5000);

  isEQ_OFF  :=DBReadByte(0,PluginName,optEQ_OFF);
  gVolume   :=integer(shortint(DBReadByte(0,PluginName,optVolume,50)));
  ForcedMono:=DBReadByte(0,PluginName,optForcedMono);
  NumTries  :=DBReadByte(0,PluginName,optNumTries,1);
  if NumTries<1 then NumTries:=1;

  // volume changes
  if AuMute<>0 then
  begin
    gVolume:=ABS(gVolume);
    if DBReadByte(0,'Skin','UseSound')=0 then
      CallService(MS_RADIO_MUTE,0,0);
  end;

  //-- Equalizer
  szTemp[0]:='E';
  szTemp[1]:='Q';
  szTemp[2]:='_';
  szTemp[4]:=#0;
  for i:=0 to 9 do
  begin
    szTemp[3]:=AnsiChar(ORD('0')+i);
    eq[i].param.fGain:=DBReadByte(0,PluginName,szTemp,15)-15;
  end;

  if AuConnect<>BST_UNCHECKED then
    ActiveContact:=LoadContact(PluginName,optLastStn)
  else
    ActiveContact:=0;
end;

procedure SaveCommonSettings;
begin
  DBWriteByte(0,PluginName,optLoop     ,doLoop);
  DBWriteByte(0,PluginName,optShuffle  ,doShuffle);
  DBWriteByte(0,PluginName,optContRec  ,doContRec);
  DBWriteByte(0,PluginName,optPlayFirst,PlayFirst);
  DBWriteByte(0,PluginName,optConnect  ,AuConnect);
  DBWriteByte(0,PluginName,optAutoMute ,AuMute);
  DBWriteByte(0,PluginName,optOffline  ,AsOffline);

  DBWriteUnicode(0,PluginName,optStatusTmpl,StatusTmpl);
  DBWriteUnicode(0,PluginName,optRecPath   ,recpath);
end;

procedure SaveTechSettings;
var
  szTemp:array [0..7] of AnsiChar;
  i:integer;
begin
  DBWriteWord(0,PluginName,optTimeout,sTimeout);
  DBWriteWord(0,PluginName,optBuffer,sBuffer);
  DBWriteWord(0,PluginName,optPreBuf,sPreBuf);

  DBWriteByte(0,PluginName,optForcedMono,ForcedMono);
  DBWriteByte(0,PluginName,optNumTries,NumTries);
  DBWriteByte(0,PluginName,optEQ_OFF ,isEQ_OFF);

  DBWriteString(0,PluginName,optDevice,usedevice);

  szTemp[0]:='E';
  szTemp[1]:='Q';
  szTemp[2]:='_';
  szTemp[4]:=#0;
  for i:=0 to 9 do
  begin
    szTemp[3]:=AnsiChar(ORD('0')+i);
    DBWriteByte(0,PluginName,szTemp,trunc(eq[i].param.fGain)+15);
  end;
end;

end.
