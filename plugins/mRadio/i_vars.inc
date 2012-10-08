{used variables}

{$include m_radio.inc}

const
  optActiveCodec:PAnsiChar = 'ActiveCodec';
  optLastStn    :PAnsiChar = 'LastStation';
  optConnect    :PAnsiChar = 'AutoConnect';
  optAutoMute   :PAnsiChar = 'AutoMute';
  optEQ_OFF     :PAnsiChar = 'eqoff';
  optStatusMsg  :PAnsiChar = 'StatusMsg';
  optCurElement :PAnsiChar = 'LastPlayed';
  optPlayFirst  :PAnsiChar = 'PlayFromFirst'; 
  optActiveURL  :PAnsiChar = 'ActiveURL';
  optContRec    :PAnsiChar = 'ContRec';
  optLoop       :PAnsiChar = 'Loop';
  optShuffle    :PAnsiChar = 'Shuffle';
  optRecPath    :PAnsiChar = 'RecordPath';
  optStatus     :PAnsiChar = 'Status';
  optVolume     :PAnsiChar = 'Volume';
  optBuffer     :PAnsiChar = 'Buffer';
  optPreBuf     :PAnsiChar = 'PreBuf';
  optTimeout    :PAnsiChar = 'Timeout';
  optVersion    :PAnsiChar = 'version';
  optStatusTmpl :PAnsiChar = 'StatusTmpl';
  optNumTries   :PAnsiChar = 'NumTries';
  optOffline    :PAnsiChar = 'asOffline';
  // mRadio compatibility
  optStationURL :PAnsiChar = 'StationURL';
  optMyHandle   :PAnsiChar = 'MyHandle';
  optGenre      :PAnsiChar = 'Genre';
  optBitrate    :PAnsiChar = 'Bitrate';
  // UserInfo compatibility
  optFirstName  :PAnsiChar = 'FirstName';
  optNick       :PAnsiChar = 'Nick';
  optLastName   :PAnsiChar = 'LastName';
  optAge        :PAnsiChar = 'Age';
  // 3D sound support
  optEAXType    :PAnsiChar = 'EAXtype';
  optForcedMono :PAnsiChar = 'ForcedMono';

  optGroup      :PAnsiChar = 'Group';
  optBASSPath   :PAnsiChar = 'BASSpath';
  optTitle      :PAnsiChar = 'Title';
  optArtist     :PAnsiChar = 'Artist';

var
  hhRadioStatus,
  // service handles
  hsPlayStop,
  hsRecord,
  hsSettings,
  hsSetVol,
  hsGetVol,
  hsMute,
  hsCommand,
  hsExport,
  hsImport,
  hsTrayMenu,
  hsEqOnOff,

  hNetLib,
  hDblClick,
  hHookShutdown,
  hCMenuItemRec,
  hCMenuItemPlay,
  contexthook,
  opthook,
  onsetting,
  ondelete,
  onloadhook:THANDLE;
  hiddenwindow:HWND;
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
  sBuffer,
  sTimeout,
  sPreBuf:cardinal;
const
  hVolCtrl    :HWND=0;
  hVolFrmCtrl :HWND=0;
const
  {$IFDEF KOL_MCK}
  btnMute:pIcoButton = nil;
  {$ELSE}
  hMuteFrmCtrl:HWND=0;
  {$ENDIF}
const
  Inited:boolean=false;
const
  StatusTmpl:pWideChar = nil;
  proxy:pAnsiChar = nil;
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
const
  IcoBtnSettings:PAnsiChar = 'Radio_Setting';
  IcoBtnOn      :PAnsiChar = 'Radio_On';
  IcoBtnOff     :PAnsiChar = 'Radio_Off';
  IcoBtnRecUp   :PAnsiChar = 'Radio_RecUp';
  IcoBtnRecDn   :PAnsiChar = 'Radio_RecDn';
  IcoBtnAdd     :PAnsiChar = 'Radio_Add';
  IcoBtnDel     :PAnsiChar = 'Radio_Del';
const
  hRecord      :THANDLE = 0;
  chan         :HSTREAM = 0;
  ActiveContact:THANDLE = 0;
  ActiveURLw   :PWideChar = nil;

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

type
  tPreset = record
    name  :PWideChar;
    preset:array [0..9] of shortint;
  end;
var
  Presets: array of tPreset;
