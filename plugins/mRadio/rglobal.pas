{used variables}
unit rglobal;

interface

uses
  windows,
  Dynamic_Bass,
  m_api,
  playlist;

{$include m_radio.inc}

const
  cPluginName = 'mRadio';
const
  PluginName:PAnsiChar = cPluginName;

const
  BassStatus:(rbs_null,rbs_load,rbs_init) = rbs_null;

const
  chan         :HSTREAM = 0;
  ActiveContact:TMCONTACT = 0;
  ActiveURL    :PWideChar = nil;

const
  optVolume     :PAnsiChar = 'Volume';

  optVersion    :PAnsiChar = 'version'; //??

  // mRadio compatibility
//optStationurl <<
  optMyHandle   :PAnsiChar = 'MyHandle';
  optGenre      :PAnsiChar = 'Genre';
  optBitrate    :PAnsiChar = 'Bitrate';
  // UserInfo compatibility
  optFirstName  :PAnsiChar = 'FirstName';
  optNick       :PAnsiChar = 'Nick';
  optLastName   :PAnsiChar = 'LastName';
  optAge        :PAnsiChar = 'Age';

  optGroup      :PAnsiChar = 'Group';

const
  optEAXType    :PAnsiChar = 'EAXtype';
  optStatusMsg  :PAnsiChar = 'StatusMsg';
  optBASSPath   :PAnsiChar = 'BASSpath';

  optLastStn    :PAnsiChar = 'LastStation';
  optActiveCodec:PAnsiChar = 'ActiveCodec';
  optStationURL :PAnsiChar = 'StationURL'; // mRadio compatibility
const
  optTitle      :PAnsiChar = 'Title';
  optArtist     :PAnsiChar = 'Artist';
  optStatus     :PAnsiChar = 'Status';

var
  hhRadioStatus,
  hNetLib:THANDLE;
var
  plist:tPlaylist;
//  plFile:pWideChar; // playlist file name (for delete after using?)
//  plLocal:boolean;  // true - no need to delete playlist
var
  RemoteSong:bool;
  gVolume:integer;
  NumTries:cardinal;
  doLoop:cardinal;
  PlayFirst:cardinal;
  doShuffle:cardinal;
  ForcedMono:cardinal;
  doContRec:cardinal;
  AuConnect:cardinal;
  AuMute:cardinal;
  AsOffline:cardinal;
  isEQ_OFF:cardinal;
  PluginStatus:integer;
  storagep,storage:PAnsiChar;
  recpath:pWideChar;
  StatusTmpl:pWideChar;
  sBuffer,
  sTimeout,
  sPreBuf:cardinal;
  usedevice:PAnsiChar;

const // inside INC files only
  hVolCtrl:HWND=0;

//----- Equalizer -----

type
  tEQRec = record
    fx    :HFX;
    wnd   :HWND;
    param :BASS_DX8_PARAMEQ;
    text  :PAnsiChar;
  end;
var
  eq:array [0..9] of tEQRec = (
    (fx:0;wnd:0;param:(fCenter:80   ;fBandwidth:18;fGain:0);text:'80'),
    (fx:0;wnd:0;param:(fCenter:170  ;fBandwidth:18;fGain:0);text:'170'),
    (fx:0;wnd:0;param:(fCenter:310  ;fBandwidth:18;fGain:0);text:'310'),
    (fx:0;wnd:0;param:(fCenter:600  ;fBandwidth:18;fGain:0);text:'600'),
    (fx:0;wnd:0;param:(fCenter:1000 ;fBandwidth:18;fGain:0);text:'1k'),
    (fx:0;wnd:0;param:(fCenter:3000 ;fBandwidth:18;fGain:0);text:'3k'),
    (fx:0;wnd:0;param:(fCenter:6000 ;fBandwidth:18;fGain:0);text:'6k'),
    (fx:0;wnd:0;param:(fCenter:12000;fBandwidth:18;fGain:0);text:'12k'),
    (fx:0;wnd:0;param:(fCenter:14000;fBandwidth:18;fGain:0);text:'14k'),
    (fx:0;wnd:0;param:(fCenter:16000;fBandwidth:18;fGain:0);text:'16k'));

//----- Button icons -----

const
  IcoBtnSettings:PAnsiChar = 'Radio_Setting';
  IcoBtnOn      :PAnsiChar = 'Radio_On';
  IcoBtnOff     :PAnsiChar = 'Radio_Off';
  IcoBtnRecUp   :PAnsiChar = 'Radio_RecUp';
  IcoBtnRecDn   :PAnsiChar = 'Radio_RecDn';
  IcoBtnAdd     :PAnsiChar = 'Radio_Add';
  IcoBtnDel     :PAnsiChar = 'Radio_Del';
  IcoBtnOpen    :PAnsiChar = 'Radio_Open';

//----- EAX -----

type
  TEAXItem = record
    name:PWideChar;
    code:dword;
  end;
const
  EAXItems:array [0..EAX_ENVIRONMENT_COUNT] of TEAXItem=(
    (name:'Off'             ; code:0),
    (name:'Generic'         ; code:EAX_ENVIRONMENT_GENERIC),
    (name:'Padded Cell'     ; code:EAX_ENVIRONMENT_PADDEDCELL),
    (name:'Room'            ; code:EAX_ENVIRONMENT_ROOM),
    (name:'Bathroom'        ; code:EAX_ENVIRONMENT_BATHROOM),
    (name:'Living Room'     ; code:EAX_ENVIRONMENT_LIVINGROOM),
    (name:'Stone Room'      ; code:EAX_ENVIRONMENT_STONEROOM),
    (name:'Auditorium'      ; code:EAX_ENVIRONMENT_AUDITORIUM),
    (name:'Concert Hall'    ; code:EAX_ENVIRONMENT_CONCERTHALL),
    (name:'Cave'            ; code:EAX_ENVIRONMENT_CAVE),
    (name:'Arena'           ; code:EAX_ENVIRONMENT_ARENA),
    (name:'Hangar'          ; code:EAX_ENVIRONMENT_HANGAR),
    (name:'Carpeted Hallway'; code:EAX_ENVIRONMENT_CARPETEDHALLWAY),
    (name:'Hallway'         ; code:EAX_ENVIRONMENT_HALLWAY),
    (name:'Stone Corridor'  ; code:EAX_ENVIRONMENT_STONECORRIDOR),
    (name:'Alley'           ; code:EAX_ENVIRONMENT_ALLEY),
    (name:'Forrest'         ; code:EAX_ENVIRONMENT_FOREST),
    (name:'City'            ; code:EAX_ENVIRONMENT_CITY),
    (name:'Mountains'       ; code:EAX_ENVIRONMENT_MOUNTAINS),
    (name:'Quarry'          ; code:EAX_ENVIRONMENT_QUARRY),
    (name:'Plain'           ; code:EAX_ENVIRONMENT_PLAIN),
    (name:'Parking Lot'     ; code:EAX_ENVIRONMENT_PARKINGLOT),
    (name:'Sewer Pipe'      ; code:EAX_ENVIRONMENT_SEWERPIPE),
    (name:'Under Water'     ; code:EAX_ENVIRONMENT_UNDERWATER),
    (name:'Drugged'         ; code:EAX_ENVIRONMENT_DRUGGED),
    (name:'Dizzy'           ; code:EAX_ENVIRONMENT_DIZZY),
    (name:'Psychotic'       ; code:EAX_ENVIRONMENT_PSYCHOTIC));


function MakeMessage:pWideChar;
procedure SetStatus(hContact:TMCONTACT;status:integer);
function GetDefaultRecPath:pWideChar;
function GetStatusText(status:integer;toCList:boolean=false):PWideChar;

implementation

uses
  common, dbsettings;

procedure SetStatus(hContact:TMCONTACT;status:integer);
begin
//  if Status=ID_STATUS_OFFLINE then
//    MyStopBass;

  if status=ID_STATUS_OFFLINE then
  begin
    if (AsOffline=BST_UNCHECKED) or (PluginStatus<>ID_STATUS_OFFLINE) then
      status:=ID_STATUS_INVISIBLE;
  end;

  if hContact=0 then
  begin
    hContact:=db_find_first(PluginName);
    while hContact<>0 do
    begin
      DBWriteWord(hContact,PluginName,optStatus,status);
      hContact:=db_find_next(hContact,PluginName);
    end;
  end
  else
    DBWriteWord(hContact,PluginName,optStatus,status);
end;

function MakeMessage:pWideChar;
var
  p,artist,title:pWideChar;
  len:cardinal;
begin
  artist:=DBReadUnicode(0,PluginName,optArtist);
  title :=DBReadUnicode(0,PluginName,optTitle);
  len:=StrLenW(artist);
  if (artist<>nil) and (title<>nil) then
    inc(len,3);
  inc(len,StrLenW(title));

  if len>0 then
  begin
    mGetMem(result,(len+1)*SizeOf(WideChar));
    p:=result;
    if artist<>nil then
    begin
      p:=StrCopyEW(p,artist);
      if title<>nil then
        p:=StrCopyEW(p,' - ');
      mFreeMem(artist);
    end;
    if title<>nil then
    begin
      StrCopyW(p,title);
      mFreeMem(title);
    end;
  end
  else
    result:=nil;
end;

function GetDefaultRecPath:pWideChar;
var
  dat:TREPLACEVARSDATA;
  mstr,szData:pWideChar;
  buf:array [0..MAX_PATH-1] of WideChar;
begin
  FillChar(dat,SizeOf(dat),0);
  dat.cbSize :=SizeOf(TREPLACEVARSDATA);
  dat.dwFlags:=RVF_UNICODE;
  szData:='%miranda_userdata%'+'\'+cPluginName;
  mstr:=pWideChar(CallService(MS_UTILS_REPLACEVARS, WPARAM(szData), LPARAM(@dat)));
  PathToRelativeW(mstr,buf);
  StrDupW(result,buf);
  mir_free(mstr);
end;

function GetStatusText(status:integer;toCList:boolean=false):PWideChar;
begin
  case status of
    RD_STATUS_PAUSED : result:='paused';
    RD_STATUS_STOPPED: if toCList then result:=nil else result:='stopped';
    RD_STATUS_CONNECT: result:='connecting';
    RD_STATUS_ABORT  : result:='aborting';
    RD_STATUS_PLAYING: if toCList then result:=nil else result:='playing';
  else
    result:=nil;
  end;
end;

end.
