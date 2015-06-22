unit lastfm;
{$include compilers.inc}
interface
{$Resource lastfm.res}
implementation

uses windows, messages, commctrl,
  common,
  m_api,dbsettings,wrapper,mirutils, datetime,
  wat_api,global;

{$include resource.inc}

const
  opt_ModStatus:PAnsiChar = 'module/lastfm';
const
  IcoLastFM:pAnsiChar = 'WATrack_lasfm';
var
  lfm_tries:integer;
  slastinf:THANDLE;
  slast:THANDLE;
const
  lfm_lang    :integer=0;
  lfm_on      :integer=0;
  hMenuLast   :HMENU = 0;
  lfm_login   :pAnsiChar=nil;
  lfm_password:pAnsiChar=nil;
  session_id  :pAnsiChar=nil;
  np_url      :pAnsiChar=nil;
  sub_url     :pAnsiChar=nil;

function GetModStatus:integer;
begin
  result:=DBReadByte(0,PluginShort,opt_ModStatus,1);
end;

procedure SetModStatus(stat:integer);
begin
  DBWriteByte(0,PluginShort,opt_ModStatus,stat);
end;

{$i i_const.inc}
{$i i_last_opt.inc}
{$i i_last_api.inc}

procedure ThScrobble(param:LPARAM); cdecl;
var
  count:integer;
  npisok:bool;
begin
  count:=lfm_tries;
  npisok:=false;
  while count>0 do
  begin
    if not npisok then
      npisok:=SendNowPlaying>=0;
    if Scrobble>=0 then break;
    HandShake(lfm_login,lfm_password, count=1); // just last time
    dec(count);
  end;
  if count=0 then ;
end;

const
  hTimer:THANDLE=0;

procedure TimerProc(wnd:HWND;uMsg:uint;idEvent:uint_ptr;dwTime:dword); stdcall;
begin
  if hTimer<>0 then
  begin
    KillTimer(0,hTimer);
    hTimer:=0;
  end;

  if (lfm_login   <>nil) and (lfm_login^   <>#0) and
     (lfm_password<>nil) and (lfm_password^<>#0) then
    {CloseHandle}(mir_forkthread(@ThScrobble,nil));
end;

function NewPlStatus(wParam:WPARAM;lParam:LPARAM):int;cdecl;
var
  flag:integer;
begin
  result:=0;
  case wParam of
    WAT_EVENT_NEWTRACK: begin
      if hTimer<>0 then
      begin
        KillTimer(0,hTimer);
        hTimer:=0;
      end;
      if lfm_on=0 then
        hTimer:=SetTimer(0,0,30000,@TimerProc)
    end;

    WAT_EVENT_PLUGINSTATUS: begin
      case lParam of
        dsEnabled: begin
          lfm_on:=lfm_on and not 2;
          flag:=0;
        end;
        dsPermanent: begin
          lfm_on:=lfm_on or 2;
          if hTimer<>0 then
          begin
            KillTimer(0,hTimer);
            hTimer:=0;
          end;
          flag:=CMIF_GRAYED;
        end;
      else // like 1
        exit
      end;
      Menu_ModifyItem(hMenuLast, nil, INVALID_HANDLE_VALUE, flag);
    end;

    WAT_EVENT_PLAYERSTATUS: begin
      case integer(loword(lParam)) of
        WAT_PLS_STOPPED,
        WAT_PLS_NOTFOUND: begin
          if hTimer<>0 then
          begin
            KillTimer(0,hTimer);
            hTimer:=0;
          end;
        end;
      end;
    end;
  end;
end;

{$i i_last_dlg.inc}

function SrvLastFMInfo(wParam:WPARAM;lParam:LPARAM):int;cdecl;
var
  data:tLastFMInfo;
begin
  case wParam of
    0: result:=GetArtistInfo(data,lParam);
    1: result:=GetAlbumInfo (data,lParam);
    2: result:=GetTrackInfo (data,lParam);
  else
    result:=0;
  end;
end;

function SrvLastFM(wParam:WPARAM;lParam:LPARAM):int;cdecl;
begin
  if odd(lfm_on) then
  begin
    Menu_ModifyItem(hMenuLast,'Disable scrobbling');
    lfm_on:=lfm_on and not 1;
  end
  else
  begin
    Menu_ModifyItem(hMenuLast,'Enable scrobbling');
    lfm_on:=lfm_on or 1;
    if hTimer<>0 then
    begin
      KillTimer(0,hTimer);
      hTimer:=0;
    end;
  end;
  result:=ord(not odd(lfm_on));
end;

procedure CreateMenus;
var
  mi:TCListMenuItem;
  sid:TSKINICONDESC;
begin
  FillChar(sid,SizeOf(TSKINICONDESC),0);
  sid.cx:=16;
  sid.cy:=16;
  sid.szSection.a:='WATrack';

  sid.hDefaultIcon   :=LoadImage(hInstance,'IDI_LAST',IMAGE_ICON,16,16,0);
  sid.pszName        :=IcoLastFM;
  sid.szDescription.a:='LastFM';
  Skin_AddIcon(@sid);
  DestroyIcon(sid.hDefaultIcon);

  FillChar(mi, sizeof(mi), 0);
  mi.szPopupName.a:=PluginShort;

  mi.hIcon        :=IcoLib_GetIcon(IcoLastFM,0);
  mi.szName.a     :='Disable scrobbling';
  mi.pszService   :=MS_WAT_LASTFM;
  mi.popupPosition:=500050000;
  hMenuLast:=Menu_AddMainMenuItem(@mi);
end;

// ------------ base interface functions -------------

function AddOptionsPage(var tmpl:pAnsiChar;var proc:pointer;var name:PAnsiChar):integer;
begin
  tmpl:=PAnsiChar(IDD_OPT_LASTFM);
  proc:=@DlgProcOptions;
  name:='LastFM';
  result:=0;
end;

function InitProc(aGetStatus:boolean=false):integer;
begin
  slastinf:=CreateServiceFunction(MS_WAT_LASTFMINFO,@SrvLastFMInfo);
  if aGetStatus then
  begin
    if GetModStatus=0 then
    begin
      result:=0;
      exit;
    end;
  end
  else
  begin
    SetModStatus(1);
    lfm_on:=lfm_on and not 4;
  end;
  result:=1;

  LoadOpt;

  slast:=CreateServiceFunction(MS_WAT_LASTFM,@SrvLastFM);
  if hMenuLast=0 then
    CreateMenus;
  if (lfm_on and 4)=0 then
    HookEvent(ME_WAT_NEWSTATUS,@NewPlStatus);
end;

procedure DeInitProc(aSetDisable:boolean);
begin
  if aSetDisable then
    SetModStatus(0)
  else
    DestroyServiceFunction(slastinf);

  CallService(MO_REMOVEMENUITEM,hMenuLast,0);
  hMenuLast:=0;
  DestroyServiceFunction(slast);

  if hTimer<>0 then
  begin
    KillTimer(0,hTimer);
    hTimer:=0;
  end;

  FreeOpt;

  mFreeMem(session_id);
  mFreeMem(np_url);
  mFreeMem(sub_url);

  lfm_on:=lfm_on or 4;
end;

var
  last:twModule;

procedure Init;
begin
  last.Next      :=ModuleLink;
  last.Init      :=@InitProc;
  last.DeInit    :=@DeInitProc;
  last.AddOption :=@AddOptionsPage;
  last.Check     :=nil;
  last.ModuleName:='Last.FM';
  ModuleLink     :=@last;

end;

begin
  Init;
end.
