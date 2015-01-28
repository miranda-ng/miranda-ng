{Popup support}
unit Popups;
{$include compilers.inc}
interface
{$Resource popup.res}
implementation

uses windows,messages,commctrl,
     wat_api,waticons,global,
     wrapper,common,m_api,dbsettings,mirutils;

const
  MenuInfoPos = 500050002;
  PluginName  = 'Winamp Track';
const
  IcoBtnInfo:PAnsiChar='WATrack_Info';
const
  HKN_POPUP:PAnsiChar = 'WAT_Popup';

{$include pop_rc.inc}
{$include pop_vars.inc}
{$include pop_opt.inc}

const
  MainTmpl = 'artist: %ls'#13#10'title: "%ls"'#13#10'album: "%ls"'#13#10+
    'genre: %ls'#13#10'comment: %ls'#13#10'year: %ls'#13#10'track: %lu'#13#10+
    'bitrate: %lukbps %ls'#13#10'samplerate: %luKHz'#13#10+
    'channels: %lu'#13#10'length: %ls'#13#10'player: "%ls" v.%ls';
  AddTmpl = #13#10'file: "%ls"'#13#10'size: %lu bytes';

procedure ShowMusicInfo(si:pSongInfo);
var
  Tmpl:array [0..255] of WideChar;
  buf:pWideChar;
  lvars:array [0..15] of uint_ptr;
  s:array [0..31] of WideChar;
  p:PWideChar;
begin
  mGetMem(buf,16384);
  with si^ do
  begin
    lvars[0]:=uint_ptr(artist);
    lvars[1]:=uint_ptr(title);
    lvars[2]:=uint_ptr(album);
    lvars[3]:=uint_ptr(genre);
    lvars[4]:=uint_ptr(comment);
    lvars[5]:=uint_ptr(year);
    lvars[6]:=track;
    lvars[7]:=kbps;
    if vbr>0 then
      p:='VBR'
    else
      p:='CBR';
    lvars[8]:=uint_ptr(p);
    lvars[9]:=khz;
    lvars[10]:=channels;
    lvars[11]:=uint_ptr(IntToTime(s,total));
    lvars[12]:=uint_ptr(player);
    lvars[13]:=uint_ptr(txtver);
  end;
  StrCopyW(Tmpl,TranslateW(MainTmpl));
  if PopupFile=BST_CHECKED then
  begin
    lvars[14]:=uint_ptr(si^.mfile);
    lvars[15]:=si^.fsize;
    StrCatW(Tmpl,TranslateW(AddTmpl));
  end;

  wvsprintfw(buf,Tmpl,@lvars);
  MessageBoxW(0,buf,PluginName,MB_OK);
  mFreeMem(buf);
end;

function DumbPopupDlgProc(wnd:HWND;msg:dword;wParam:WPARAM;lParam:LPARAM):LRESULT; stdcall;
var
  si:pSongInfo;
  h:HBITMAP;
begin
  case msg of
    WM_COMMAND,WM_CONTEXTMENU: begin
      if msg=WM_CONTEXTMENU then
        wParam:=HiByte(PopupAction)
      else
        wParam:=LoByte(PopupAction);
      si:=pointer(CallService(MS_WAT_RETURNGLOBAL,0,0));
      case wParam of
        1: ShowMusicInfo(si);
        2: ShowWindow(si^.plwnd,SW_RESTORE);
        3: CallServiceSync(MS_WAT_PRESSBUTTON,WAT_CTRL_NEXT,0);
      end;
      SendMessage(wnd,UM_DESTROYPOPUP,0,0);
      result:=1;
    end;
    UM_POPUPACTION: begin
//      if wParam<>0 then
        result:=CallServiceSync(MS_WAT_PRESSBUTTON,lParam,0);
    end;
    UM_FREEPLUGINDATA: begin
      h:=0;
      h:=CallService(MS_POPUP_GETPLUGINDATA,wnd,h);
      if h<>0 then
        DeleteObject(h);
      result:=0;
    end;
  else
    result:=DefWindowProc(wnd,msg,wParam,lParam);
  end;
end;

function MakeAction(var anAct:TPOPUPACTION;action:integer):PPOPUPACTION;
begin
  result:=@anAct;
  anAct.cbSize :=SizeOf(TPOPUPACTION);
  anAct.lchIcon:=GetIcon(action);
  anAct.flags  :=PAF_ENABLED;
  anAct.wParam :=1;
  anAct.lParam :=action;
  StrCopy(StrCopyE(anAct.lpzTitle,'Watrack/'),GetIconDescr(action));
end;

function MakeActions:PPOPUPACTION;
type
  anacts = array [0..6] of TPOPUPACTION;
var
  actions:^anacts;
begin
  if PopupButtons<>BST_UNCHECKED then
  begin
    mGetMem(actions,SizeOf(anacts));
    result:=PPOPUPACTION(actions);
    FillChar(actions^,SizeOf(actions^),0);
    MakeAction(actions[0],WAT_CTRL_PREV);
    MakeAction(actions[1],WAT_CTRL_PLAY);
    MakeAction(actions[2],WAT_CTRL_PAUSE);
    MakeAction(actions[3],WAT_CTRL_STOP);
    MakeAction(actions[4],WAT_CTRL_NEXT);
    MakeAction(actions[5],WAT_CTRL_VOLDN);
    MakeAction(actions[6],WAT_CTRL_VOLUP);
  end
  else
    result:=nil;
end;

procedure ThShowPopup(si:pSongInfo); cdecl;
var
  ppdu:PPOPUPDATAW;
  title,descr:pWideChar;
  flag:dword;
  ppd2:PPOPUPDATA2;
  Icon:HICON;
  sec:integer;
  cb,ct:TCOLORREF;
begin
  descr:=PWideChar(CallService(MS_WAT_REPLACETEXT,0,lparam(PopText)));
  title:=PWideChar(CallService(MS_WAT_REPLACETEXT,0,lparam(PopTitle)));

  if (descr<>nil) or (title<>nil) then
  begin
    if si^.icon<>0 then
      Icon:=si^.icon
    else
      Icon:=LoadSkinnedIcon(SKINICON_OTHER_MIRANDA);
    if PopupDelay<0 then
      sec:=-1
    else if PopupDelay>0 then
      sec:=PopupPause
    else
      sec:=0;
    case PopupColor of
      0: begin
        cb:=0;
        ct:=0;
      end;
      1: begin
        cb:=GetSysColor(COLOR_BTNFACE);
        ct:=GetSysColor(COLOR_BTNTEXT);
      end;
      2: begin
        cb:=PopupBack;
        ct:=PopupFore;
      end;
    else
      cb:=0;
      ct:=0;
    end;

    if IsPopup2Present then
    begin
      mGetMem (ppd2 ,SizeOf(TPOPUPDATA2));
      FillChar(ppd2^,SizeOf(TPOPUPDATA2),0);
      with ppd2^ do
      begin
        cbSize          :=SizeOf(TPOPUPDATA2);
        flags           :=PU2_UNICODE;
        lchIcon         :=Icon;
        colorBack       :=cb;
        colorText       :=ct;
        PluginWindowProc:=@DumbPopupDlgProc;

        pzTitle.w:=title;
        pzText .w:=descr;

        if ActionList=nil then
          flag:=0
        else
        begin
          flag       :=APF_NEWDATA;
          actionCount:=7;
          lpActions  :=ActionList;
        end;

        if si.cover<>nil then
        begin
          if IsFreeImagePresent then
            hbmAvatar:=CallService(MS_IMG_LOAD,wparam(si.cover),IMGL_WCHAR)
          else
            hbmAvatar:=0;
          if hbmAvatar=0 then
          begin
            hbmAvatar:=CallService(MS_UTILS_LOADBITMAPW,0,lparam(si.cover));
          end;
        end;
        PluginData:=pointer(hbmAvatar);
      end;
      CallService(MS_POPUP_ADDPOPUP2,wparam(ppd2),flag);
      mFreeMem(ppd2);
    end
    else
    begin
      mGetMem (ppdu ,SizeOf(TPOPUPDATAW));
      FillChar(ppdu^,SizeOf(TPOPUPDATAW),0);
      with ppdu^ do
      begin
        if title<>nil then
          StrCopyW(lpwzContactName,title,MAX_CONTACTNAME-1)
        else
          lpwzContactName[0]:=' ';
        if descr<>nil then
          StrCopyW(lpwzText,descr,MAX_SECONDLINE-1)
        else
          lpwzText[0]:=' ';

        lchIcon         :=Icon;
        PluginWindowProc:=@DumbPopupDlgProc;
        iSeconds        :=sec;
        colorBack       :=cb;
        colorText       :=ct;

    //    if ServiceExists(MS_POPUP_REGISTERACTIONS)=0 then
        if ActionList=nil then
          flag:=0
        else
        begin
          flag       :=APF_NEWDATA;
          icbSize    :=SizeOf(TPOPUPDATAW);
          actionCount:=7;
          lpActions  :=ActionList;
        end;
      end;
      CallService(MS_POPUP_ADDPOPUPW,wparam(ppdu),flag);
      mFreeMem(ppdu);
    end;
    mFreeMem(title);
    mFreeMem(descr);
  end;
end;

procedure ShowPopup(si:pSongInfo);
begin
  mir_forkthread(@ThShowPopup,si);
end;

// --------------- Services and Hooks ----------------

function OpenPopup(wParam:WPARAM;lParam:LPARAM):int;cdecl;
var
  si:pSongInfo;
begin
  result:=0;
  if DisablePlugin<>dsEnabled then
    exit;
  if CallService(MS_WAT_GETMUSICINFO,0,tlparam(@si))=WAT_RES_OK then
  begin
    if PopupPresent then
      ShowPopup(si)
    else
      ShowMusicInfo(si);
  end;
end;

procedure regpophotkey;
var
  hkrec:THOTKEYDESC;
begin
  if DisablePlugin=dsPermanent then
    exit;
  FillChar(hkrec,SizeOf(hkrec),0);
  with hkrec do
  begin
    cbSize          :=SizeOf(hkrec);
    pszName         :=HKN_POPUP;
    pszDescription.a:='WATrack popup hotkey';
    pszSection.a    :=PluginName;
    pszService      :=MS_WAT_SHOWMUSICINFO;
    DefHotKey:=((HOTKEYF_ALT or HOTKEYF_CONTROL) shl 8) or VK_F7 or HKF_MIRANDA_LOCAL;
  end;
  CallService(MS_HOTKEY_REGISTER,0,lparam(@hkrec));
end;

{$include pop_dlg.inc}

function NewPlStatus(wParam:WPARAM;lParam:LPARAM):int;cdecl;
var
  mi:TCListMenuItem;
  flag:integer;
begin
  result:=0;
  case wParam of
    WAT_EVENT_NEWTRACK: begin
      if PopupPresent and (PopRequest=BST_UNCHECKED) then
        ShowPopup(pSongInfo(lParam));
    end;
    WAT_EVENT_PLUGINSTATUS: begin
      DisablePlugin:=lParam;
      case lParam of
        dsEnabled: begin
          flag:=0;
        end;
        dsPermanent: begin
          flag:=CMIF_GRAYED;
        end;
      else // like 1
        exit
      end;
      FillChar(mi,sizeof(mi),0);
      mi.cbSize:=sizeof(mi);
      mi.flags :=CMIM_FLAGS+flag;
      CallService(MS_CLIST_MODIFYMENUITEM,hMenuInfo,tlparam(@mi));
    end;
  end;
end;

function IconChanged(wParam:WPARAM;lParam:LPARAM):int;cdecl;
var
  mi:TCListMenuItem;
begin
  result:=0;
  FillChar(mi,SizeOf(mi),0);
  mi.cbSize:=sizeof(mi);
  mi.flags :=CMIM_ICON;
  mi.hIcon :=CallService(MS_SKIN2_GETICON,0,tlparam(IcoBtnInfo));
  CallService(MS_CLIST_MODIFYMENUITEM,hMenuInfo,tlparam(@mi));
  if ActionList<>nil then
  begin
    mFreeMem(ActionList);
    ActionList:=MakeActions;
    CallService(MS_POPUP_REGISTERACTIONS,twparam(ActionList),7);
  end;
end;

function OnOptInitialise(wParam:WPARAM;lParam:LPARAM):int;cdecl;
var
  odp:TOPTIONSDIALOGPAGE;
begin
  FillChar(odp,SizeOf(odp),0);
  odp.cbSize     :=SizeOf(odp);
  odp.flags      :=ODPF_BOLDGROUPS;
  odp.Position   :=900003000;
  odp.hInstance  :=hInstance;
  odp.szTitle.a  :=PluginName;

  odp.szGroup.a  :='Popups';
  odp.pszTemplate:=DLGPOPUP;
  odp.pfnDlgProc :=@DlgPopupOpt;
  CallService(MS_OPT_ADDPAGE,wParam,tlparam(@odp));
  result:=0;
end;

function OnTTBLoaded(wParam:WPARAM;lParam:LPARAM):int;cdecl;
var
  ttb:TTBButton;
begin
  result:=0;
  if onttbhook<>0 then
    UnhookEvent(onttbhook);
  // get info button
  FillChar(ttb,SizeOf(ttb),0);
  ttb.cbSize    :=SizeOf(ttb);
  ttb.dwFlags   :=TTBBF_VISIBLE{ or TTBBF_SHOWTOOLTIP};
  ttb.hIconUp   :=CallService(MS_SKIN2_GETICON,0,tlparam(IcoBtnInfo));
  ttb.hIconDn   :=ttb.hIconUp;
  ttb.pszService:=MS_WAT_SHOWMUSICINFO;
  ttb.name      :='Music Info';
  ttbInfo:=TopToolbar_AddButton(@ttb);
  if ttbInfo=THANDLE(-1) then
    ttbInfo:=0;
end;

// ------------ base interface functions -------------

function CheckProc(load:boolean):boolean;
var
  newstate:boolean;
begin
  result:=true;
  // Popups
  newstate:=ServiceExists(MS_POPUP_ADDPOPUPW)<>0;
  if newstate=PopupPresent then
    exit;

  PopupPresent:=newstate;
  if PopupPresent then
  begin
    IsFreeImagePresent:=ServiceExists(MS_IMG_LOAD       )<>0;
    IsPopup2Present   :=ServiceExists(MS_POPUP_ADDPOPUP2)<>0;
    opthook:=HookEvent(ME_OPT_INITIALISE,@OnOptInitialise);

    if ServiceExists(MS_POPUP_REGISTERACTIONS)<>0 then
    begin
      if RegisterButtonIcons then
      begin
        ActionList:=MakeActions;
        if ActionList<>nil then
          CallService(MS_POPUP_REGISTERACTIONS,wparam(ActionList),7);
      end;
    end;
  end
  else
  begin
    UnhookEvent(opthook);
    mFreeMem(ActionList);
  end;

  // TTB
  newstate:=ServiceExists(MS_TTB_ADDBUTTON)<>0;
  if newstate=(ttbInfo<>0) then
    exit;

  if ttbInfo=0 then
  begin
    onttbhook:=0;
    OnTTBLoaded(0,0);
    if ttbInfo=0 then
      onttbhook:=HookEvent(ME_TTB_MODULELOADED,@OnTTBLoaded);
  end
  else
  begin
    if ServiceExists(MS_TTB_REMOVEBUTTON)>0 then
      CallService(MS_TTB_REMOVEBUTTON,WPARAM(ttbInfo),0);
    ttbInfo:=0;
  end;

  // Variables ?
end;

function InitProc(aGetStatus:boolean=false):integer;
var
  mi:TCListMenuItem;
  sid:TSKINICONDESC;
begin
  if aGetStatus then
  begin
    if GetModStatus=0 then
    begin
      result:=0;
      exit;
    end;
  end
  else
    SetModStatus(1);
  result:=1;

  ssmi:=CreateServiceFunction(MS_WAT_SHOWMUSICINFO,@OpenPopup);

  FillChar(sid,SizeOf(TSKINICONDESC),0);
  sid.cbSize:=SizeOf(TSKINICONDESC);
  sid.cx:=16;
  sid.cy:=16;
  sid.szSection.a:='WATrack';
  sid.hDefaultIcon   :=LoadImage(hInstance,MAKEINTRESOURCE(BTN_INFO),IMAGE_ICON,16,16,0);
  sid.pszName        :=IcoBtnInfo;
  sid.szDescription.a:='Music Info';
  Skin_AddIcon(@sid);
  DestroyIcon(sid.hDefaultIcon);
  sic:=HookEvent(ME_SKIN2_ICONSCHANGED,@IconChanged);

  FillChar(mi,SizeOf(mi),0);
  mi.cbSize       :=SizeOf(mi);
  mi.szPopupName.a:=PluginShort;
  mi.hIcon        :=CallService(MS_SKIN2_GETICON,0,lparam(IcoBtnInfo));
  mi.szName.a     :='Music Info';
  mi.pszService   :=MS_WAT_SHOWMUSICINFO;
  mi.popupPosition:=MenuInfoPos;
  hMenuInfo       :=Menu_AddMainMenuItem(@mi);

  ActionList:=nil;
  if ServiceExists(MS_POPUP_ADDPOPUPW)<>0 then
  begin
    IsFreeImagePresent:=ServiceExists(MS_IMG_LOAD       )<>0;
    IsPopup2Present   :=ServiceExists(MS_POPUP_ADDPOPUP2)<>0;
    PopupPresent:=true;
    opthook:=HookEvent(ME_OPT_INITIALISE,@OnOptInitialise);
    loadpopup;

    if ServiceExists(MS_POPUP_REGISTERACTIONS)<>0 then
    begin
      if RegisterButtonIcons then
      begin
        ActionList:=MakeActions;
        if ActionList<>nil then
          CallService(MS_POPUP_REGISTERACTIONS,wparam(ActionList),7);
      end;
    end;
  end
  else
  begin
    PopupPresent:=false;
    opthook:=0;
  end;
  regpophotkey;

  plStatusHook:=HookEvent(ME_WAT_NEWSTATUS,@NewPlStatus);

  if ServiceExists(MS_TTB_ADDBUTTON)>0 then
  begin
    onttbhook:=0;
    OnTTBLoaded(0,0);
    if ttbInfo=0 then
      onttbhook:=HookEvent(ME_TTB_MODULELOADED,@OnTTBLoaded);
  end
  else
    ttbInfo:=0;
end;

procedure DeInitProc(aSetDisable:boolean);
begin
  if aSetDisable then
    SetModStatus(0);

  CallService(MO_REMOVEMENUITEM,hMenuInfo,0);
  UnhookEvent(plStatusHook);
  DestroyServiceFunction(ssmi);
  UnhookEvent(sic);

  freepopup;

  if ttbInfo<>0 then
  begin
    if ServiceExists(MS_TTB_REMOVEBUTTON)>0 then
      CallService(MS_TTB_REMOVEBUTTON,WPARAM(ttbInfo),0);
    ttbInfo:=0;
  end;

  if PopupPresent then
  begin
    UnhookEvent(opthook);
    mFreeMem(ActionList);
  end;
end;

var
  Popup:twModule;

procedure Init;
begin
  Popup.Next      :=ModuleLink;
  Popup.Init      :=@InitProc;
  Popup.DeInit    :=@DeInitProc;
  Popup.AddOption :=nil;
  Popup.Check     :=@CheckProc;
  Popup.ModuleName:='Popups';
  ModuleLink      :=@Popup;
end;

begin
  Init;
end.
