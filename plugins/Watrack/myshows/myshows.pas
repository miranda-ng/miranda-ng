unit myshows;
{$include compilers.inc}
interface
{$Resource myshows.res}
implementation

uses windows, messages, commctrl,
  common,
  m_api,dbsettings,wrapper, mirutils,
  wat_api,global;

{$include resource.inc}

const
  DefTimerValue = 10*60*1000; // 10 minutes
const
  opt_ModStatus:PAnsiChar = 'module/myshows';
const
  IcoMyShows:pAnsiChar = 'WATrack_myshows';
type
  tMyShowsData = record
    series      :PAnsiChar;
    series_id   :PAnsiChar;
    kinopoisk_id:PAnsiChar;
    episode     :PAnsiChar;
    episode_id  :PAnsiChar;
    info        :PAnsiChar;
    image       :PAnsiChar;
  end;
var
  msh_tries,
  msh_scrobpos:integer;
  slast:THANDLE;
  MSData:tMyShowsData;
const
  msh_on      :integer=0;
  hMenuMyShows:HMENU = 0;
  msh_login   :pAnsiChar=nil;
  msh_password:pAnsiChar=nil;
  session_id  :pAnsiChar=nil;
  np_url      :pAnsiChar=nil;
  sub_url     :pAnsiChar=nil;

procedure ClearData;
begin
  mFreeMem(MSData.series);
  mFreeMem(MSData.series_id);
  mFreeMem(MSData.kinopoisk_id);
  mFreeMem(MSData.episode);
  mFreeMem(MSData.episode_id);
  mFreeMem(MSData.info);
  mFreeMem(MSData.image);
  FillChar(MSData,SizeOf(MSData),0);
end;

function GetModStatus:integer;
begin
  result:=DBReadByte(0,PluginShort,opt_ModStatus,1);
end;

procedure SetModStatus(stat:integer);
begin
  DBWriteByte(0,PluginShort,opt_ModStatus,stat);
end;

{$i i_const.inc}
{$i i_myshows_opt.inc}
{$i i_myshows_api.inc}

procedure ThScrobble(param:LPARAM); cdecl;
var
  count:integer;
begin
  count:=msh_tries;
  repeat
    dec(count);
    if Scrobble(count<=0) then break;
  until count<=0;
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

  if (msh_login   <>nil) and (msh_login^   <>#0) and
     (msh_password<>nil) and (msh_password^<>#0) then
    {CloseHandle}(mir_forkthread(@ThScrobble,nil));
end;

function NewPlStatus(wParam:WPARAM;lParam:LPARAM):int;cdecl;
var
  flag:integer;
  timervalue:integer;
begin
  result:=0;
  case wParam of
    WAT_EVENT_NEWTRACK: begin
      if hTimer<>0 then
        KillTimer(0,hTimer);
      // need to use half of movie len if presents
      if msh_on=0 then
      begin
        if pSongInfo(lParam).width>0 then // for video only
        begin
//!!          if ServiceExists(MS_JSON_GETINTERFACE)<>0 then
          begin
            timervalue:=integer(pSongInfo(lParam).total)*10*msh_scrobpos; // 1000(msec) div 100(%)
            if timervalue=0 then
              timervalue:=DefTimerValue;
            hTimer:=SetTimer(0,0,timervalue,@TimerProc);
          end;
        end;
      end;
    end;

    WAT_EVENT_PLUGINSTATUS: begin
      case lParam of
        dsEnabled: begin
          msh_on:=msh_on and not 2;
          flag:=0;
        end;
        dsPermanent: begin
          msh_on:=msh_on or 2;
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
      Menu_ModifyItem(hMenuMyShows, nil, INVALID_HANDLE_VALUE, flag);
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

{$i i_myshows_dlg.inc}

(* kinopoisk link, cover, series?
function SrvMyShowsInfo(wParam:WPARAM;lParam:LPARAM):int;cdecl;
//var
//  data:tMyShowsInfo;
begin
  result:=0;
{
  case wParam of
    0: result:=GetArtistInfo(data,lParam);
    1: result:=GetAlbumInfo (data,lParam);
    2: result:=GetTrackInfo (data,lParam);
  else
    result:=0;
  end;
}
end;
*)
function SrvMyShows(wParam:WPARAM;lParam:LPARAM):int;cdecl;
begin
  if odd(msh_on) then
  begin
    Menu_ModifyItem(hMenuMyShows,'Disable scrobbling');
    msh_on:=msh_on and not 1;
  end
  else
  begin
    Menu_ModifyItem(hMenuMyShows,'Enable scrobbling');
    msh_on:=msh_on or 1;
    if hTimer<>0 then
    begin
      KillTimer(0,hTimer);
      hTimer:=0;
    end;
  end;
  result:=ord(not odd(msh_on));
end;

procedure CreateMenus;
var
  mi:TMO_MenuItem;
  sid:TSKINICONDESC;
begin
  FillChar(sid,SizeOf(TSKINICONDESC),0);
  sid.cx:=16;
  sid.cy:=16;
  sid.szSection.a:='WATrack';

  sid.hDefaultIcon   :=LoadImage(hInstance,'IDI_MYSHOWS',IMAGE_ICON,16,16,0);
  sid.pszName        :=IcoMyShows;
  sid.szDescription.a:='MyShows';
  Skin_AddIcon(@sid);
  DestroyIcon(sid.hDefaultIcon);

  FillChar(mi, sizeof(mi), 0);
  SET_UID(@mi, '201D5C0E-E7BA-4DAA-B80D-C9691CA296F5');
  mi.root      :=Menu_CreateRoot(MO_MAIN, 'Watrack', 500050000, 0, 0);
  mi.hIcon     :=IcoLib_GetIcon(IcoMyShows,0);
  mi.szName.a  :='Disable scrobbling';
  mi.pszService:=MS_WAT_MYSHOWS;
  mi.position  :=500050000;
  hMenuMyShows:=Menu_AddMainMenuItem(@mi);
end;

// ------------ base interface functions -------------

function AddOptionsPage(var tmpl:pAnsiChar;var proc:pointer;var name:PAnsiChar):integer;
begin
  tmpl:=PAnsiChar(IDD_OPT_MYSHOWS);
  proc:=@DlgProcOptions;
  name:='MyShows';
  result:=0;
end;

function InitProc(aGetStatus:boolean=false):integer;
begin
//  slastinf:=CreateServiceFunction(MS_WAT_MYSHOWSINFO,@SrvMyShowsInfo);
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
    msh_on:=msh_on and not 4;
  end;
  result:=1;

  LoadOpt;

  slast:=CreateServiceFunction(MS_WAT_MYSHOWS,@SrvMyShows);
  if hMenuMyShows=0 then
    CreateMenus;
  if (msh_on and 4)=0 then
    HookEvent(ME_WAT_NEWSTATUS,@NewPlStatus);
end;

procedure DeInitProc(aSetDisable:boolean);
begin
  if aSetDisable then
    SetModStatus(0)
  else
;//    DestroyServiceFunction(slastinf);

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

  msh_on:=msh_on or 4;

  mFreeMem(cookies); //!!
end;

var
  mmyshows:twModule;

procedure Init;
begin
  mmyshows.Next      :=ModuleLink;
  mmyshows.Init      :=@InitProc;
  mmyshows.DeInit    :=@DeInitProc;
  mmyshows.AddOption :=@AddOptionsPage;
  mmyshows.Check     :=nil;
  mmyshows.ModuleName:='MyShows.me';
  ModuleLink         :=@mmyshows;
end;

begin
  Init;
end.
