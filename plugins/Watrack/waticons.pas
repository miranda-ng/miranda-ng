unit WATIcons;
interface

uses wat_api;

const // to not load icobuttons module
  AST_NORMAL  = 0;
  AST_HOVERED = 1;
  AST_PRESSED = 2;

// main Enable/Disable icons
const // name in icolib
  IcoBtnEnable :PAnsiChar='WATrack_Enabled';
  IcoBtnDisable:PAnsiChar='WATrack_Disabled';

function RegisterIcons:boolean;

// frame button icons
function RegisterButtonIcons:boolean;
function GetIcon(action:integer;stat:integer=AST_NORMAL):cardinal;
function DoAction(action:integer):integer;
function GetIconDescr(action:integer):pAnsiChar;
{
const
  AST_NORMAL  = 0;
  AST_HOVERED = 1;
  AST_PRESSED = 2;
}
implementation

uses m_api,windows;

{$include waticons.inc}

const
  ICOCtrlName = 'watrack_buttons.dll';

const
  IconsLoaded:bool = false;

function DoAction(action:integer):integer;
begin
  result:=CallService(MS_WAT_PRESSBUTTON,action,0);
end;

function RegisterIcons:boolean;
var
  sid:TSKINICONDESC;
  buf:array [0..511] of AnsiChar;
  hIconDLL:THANDLE;
begin
  result:=true;
  sid.szDefaultFile.a:='icons\'+ICOCtrlName;
//    ConvertFileName(sid.szDefaultFile.a,buf);
  PathToAbsolute(sid.szDefaultFile.a,buf);
//  CallService(MS_UTILS_PATHTOABSOLUTE,wparam(sid.szDefaultFile),lparam(@buf));

  hIconDLL:=LoadLibraryA(buf);
  if hIconDLL=0 then // not found
  begin
    sid.szDefaultFile.a:='plugins\'+ICOCtrlName;
//      ConvertFileName(sid.szDefaultFile.a,buf);
    PathToAbsolute(sid.szDefaultFile.a,buf);
//    CallService(MS_UTILS_PATHTOABSOLUTE,wparam(sid.szDefaultFile),lparam(@buf));
    hIconDLL:=LoadLibraryA(buf);
  end;

  if hIconDLL=0 then
    hIconDLL:=hInstance;

  FillChar(sid,SizeOf(TSKINICONDESC),0);
  sid.cbSize:=SizeOf(TSKINICONDESC);
  sid.cx:=16;
  sid.cy:=16;
  sid.szSection.a:='WATrack';

  sid.hDefaultIcon   :=LoadImage(hIconDLL,
      MAKEINTRESOURCE(IDI_PLUGIN_ENABLE),IMAGE_ICON,16,16,0);
  sid.pszName        :=IcoBtnEnable;
  sid.szDescription.a:='Plugin Enabled';
  Skin_AddIcon(@sid);
  DestroyIcon(sid.hDefaultIcon);

  sid.hDefaultIcon   :=LoadImage(hIconDLL,
      MAKEINTRESOURCE(IDI_PLUGIN_DISABLE),IMAGE_ICON,16,16,0);
  sid.pszName        :=IcoBtnDisable;
  sid.szDescription.a:='Plugin Disabled';
  Skin_AddIcon(@sid);
  DestroyIcon(sid.hDefaultIcon);

  if hIconDLL<>hInstance then
    FreeLibrary(hIconDLL);
end;

type
  PAWKIconButton = ^TAWKIconButton;
  TAWKIconButton = record
    descr:PAnsiChar;
    name :PAnsiChar;
    id   :int_ptr;
  end;
const
  CtrlIcoLib:array [WAT_CTRL_FIRST..WAT_CTRL_LAST,AST_NORMAL..AST_PRESSED] of
    TAWKIconButton = (
    ((descr:'Prev'               ;name:'WATrack_Prev'   ; id:IDI_PREV_NORMAL),
     (descr:'Prev Hovered'       ;name:'WATrack_PrevH'  ; id:IDI_PREV_HOVERED),
     (descr:'Prev Pushed'        ;name:'WATrack_PrevP'  ; id:IDI_PREV_PRESSED)),

    ((descr:'Play'               ;name:'WATrack_Play'   ; id:IDI_PLAY_NORMAL),
     (descr:'Play Hovered'       ;name:'WATrack_PlayH'  ; id:IDI_PLAY_HOVERED),
     (descr:'Play Pushed'        ;name:'WATrack_PlayP'  ; id:IDI_PLAY_PRESSED)),

    ((descr:'Pause'              ;name:'WATrack_Pause'  ; id:IDI_PAUSE_NORMAL),
     (descr:'Pause Hovered'      ;name:'WATrack_PauseH' ; id:IDI_PAUSE_HOVERED),
     (descr:'Pause Pushed'       ;name:'WATrack_PauseP' ; id:IDI_PAUSE_PRESSED)),

    ((descr:'Stop'               ;name:'WATrack_Stop'   ; id:IDI_STOP_NORMAL),
     (descr:'Stop Hovered'       ;name:'WATrack_StopH'  ; id:IDI_STOP_HOVERED),
     (descr:'Stop Pushed'        ;name:'WATrack_StopP'  ; id:IDI_STOP_PRESSED)),

    ((descr:'Next'               ;name:'WATrack_Next'   ; id:IDI_NEXT_NORMAL),
     (descr:'Next Hovered'       ;name:'WATrack_NextH'  ; id:IDI_NEXT_HOVERED),
     (descr:'Next Pushed'        ;name:'WATrack_NextP'  ; id:IDI_NEXT_PRESSED)),

    ((descr:'Volume Down'        ;name:'WATrack_VolDn'  ; id:IDI_VOLDN_NORMAL),
     (descr:'Volume Down Hovered';name:'WATrack_VolDnH' ; id:IDI_VOLDN_HOVERED),
     (descr:'Volume Down Pushed' ;name:'WATrack_VolDnP' ; id:IDI_VOLDN_PRESSED)),

    ((descr:'Volume Up'          ;name:'WATrack_VolUp'  ; id:IDI_VOLUP_NORMAL),
     (descr:'Volume Up Hovered'  ;name:'WATrack_VolUpH' ; id:IDI_VOLUP_HOVERED),
     (descr:'Volume Up Pushed'   ;name:'WATrack_VolUpP' ; id:IDI_VOLUP_PRESSED)),

    ((descr:'Slider'             ;name:'WATrack_Slider' ; id:IDI_SLIDER_NORMAL),
     (descr:'Slider Hovered'     ;name:'WATrack_SliderH'; id:IDI_SLIDER_HOVERED),
     (descr:'Slider Pushed'      ;name:'WATrack_SliderP'; id:IDI_SLIDER_PRESSED))
    );

function RegisterButtonIcons:boolean;
var
  sid:TSKINICONDESC;
  buf:array [0..511] of AnsiChar;
  hIconDLL:THANDLE;
  i,j:integer;
  path:pAnsiChar;
begin
  if not IconsLoaded then
  begin
    path:='icons\'+ICOCtrlName;
//    ConvertFileName(sid.szDefaultFile.a,buf);
    PathToAbsolute(path,buf);
//    CallService(MS_UTILS_PATHTOABSOLUTE,wparam(path),lparam(@buf));

    hIconDLL:=LoadLibraryA(buf);
    if hIconDLL=0 then // not found
    begin
      sid.szDefaultFile.a:='plugins\'+ICOCtrlName;
//      ConvertFileName(sid.szDefaultFile.a,buf);
      PathToAbsolute(path,buf);
//      CallService(MS_UTILS_PATHTOABSOLUTE,wparam(path),lparam(@buf));
      hIconDLL:=LoadLibraryA(buf);
    end;

    if hIconDLL<>0 then
    begin
      FreeLibrary(hIconDLL);
      FillChar(sid,SizeOf(sid),0);
      sid.flags:=0;
      sid.cbSize:=SizeOf(TSKINICONDESC);
      sid.cx:=16;
      sid.cy:=16;

      sid.szSection.a    :='WATrack/Frame Controls';
      sid.szDefaultFile.a:=path;
      i:=WAT_CTRL_FIRST;
      repeat
        j:=AST_NORMAL;
        repeat
          // increment from 1 by order, so - just decrease number (for iconpack import)
          sid.iDefaultIndex  :=CtrlIcoLib[i][j].id-1;
          sid.pszName        :=CtrlIcoLib[i][j].name;
          sid.szDescription.a:=CtrlIcoLib[i][j].descr;

          Skin_AddIcon(@sid);
          Inc(j);
        until j>AST_PRESSED;
        Inc(i);
      until i>WAT_CTRL_LAST;
      IconsLoaded:=true;
    end;
  end;

  result:=IconsLoaded;
end;

function GetIcon(action:integer;stat:integer):cardinal;
begin
  result:=CallService(MS_SKIN2_GETICON,0,
      lparam(CtrlIcoLib[action][stat].name));
end;

function GetIconDescr(action:integer):pAnsiChar;
begin
  result:=CtrlIcoLib[action][AST_NORMAL].descr;
end;

end.
