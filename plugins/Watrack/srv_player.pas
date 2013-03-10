{player service}
unit srv_player;

interface

uses windows,common,wat_api;

function GetPlayerNote(name:PAnsiChar):pWideChar;

function SetPlayerIcons(fname:pAnsiChar):integer;

function LoadFromFile(fname:PAnsiChar):integer;
function ProcessPlayerLink:integer;

function ServicePlayer(wParam:WPARAM;lParam:LPARAM):int_ptr;cdecl;
function SendCommand  (wParam:WPARAM;lParam:LPARAM;flags:integer):int_ptr;

procedure ClearPlayers;

// options procedures
procedure DefFillPlayerList (hwndList:hwnd);
procedure DefCheckPlayerList(hwndList:hwnd);

type
  MusEnumProc = function(param:PAnsiChar;lParam:LPARAM):bool;stdcall;

function EnumPlayers(param:MusEnumProc;lParam:LPARAM):bool;

// "Get info" procedures
function CheckPlayers   (var dst:tSongInfo;flags:cardinal):integer;
function CheckFile      (var dst:tSongInfo;flags:cardinal;timeout:cardinal):integer;
function GetChangingInfo(var dst:tSongInfo;flags:cardinal):integer;
function GetInfo        (var dst:tSongInfo;flags:cardinal):integer;

// support procedures
procedure ClearSongInfoData(var dst:tSongInfo;withFree:bool);
procedure ClearPlayerInfo  (var dst:tSongInfo;withFree:bool);
procedure ClearFileInfo    (var dst:tSongInfo;withFree:bool);
procedure ClearChangingInfo(var dst:tSongInfo;withFree:bool);
procedure ClearTrackInfo   (var dst:tSongInfo;withFree:bool);

procedure CopyPlayerInfo  (const src:tSongInfo;var dst:tSongInfo);
procedure CopyFileInfo    (const src:tSongInfo;var dst:tSongInfo);
procedure CopyChangingInfo(const src:tSongInfo;var dst:tSongInfo);
procedure CopyTrackInfo   (const src:tSongInfo;var dst:tSongInfo);

type
  pwPlayer = ^twPlayer;
  twPlayer = record
    This:pPlayerCell;
    Next:pwPlayer;
  end;

const
  PlayerLink:pwPlayer=nil;
  
implementation

uses
  shellapi,CommCtrl
  ,appcmdapi,io,syswin,wrapper,srv_format,winampapi,msninfo,memini;

type
  pPlyArray = ^tPlyArray;
  tPlyArray = array [0..10] of tPlayerCell;

type
  pTmplCell = ^tTmplCell;
  tTmplCell = record
    p_class,
    p_text   :PAnsiChar;
    p_class1,
    p_text1  :PAnsiChar;
    p_file   :PAnsiChar;
    p_prefix :pWideChar;
    p_postfix:pWideChar;
  end;

const
  StartSize = 32;
  Step      = 8;
  buflen    = 2048;

const
  plyLink:pPlyArray=nil;
  PlyNum:integer=0;
  PlyMax:integer=0;

function ProcessPlayerLink:integer;
var
  ptr:pwPlayer;
begin
  ptr:=PlayerLink;
  result:=0;
  while ptr<>nil do
  begin
    ServicePlayer(WAT_ACT_REGISTER,lparam(ptr.This));
    ptr:=ptr^.Next;
    inc(result);
  end;
end;

function SetPlayerIcons(fname:pAnsiChar):integer;
var
  i,j:integer;
  buf:array [0..255] of AnsiChar;
  p,pp:pAnsiChar;
  lhIcon:HICON;
begin
  result:=LoadLibraryA(fname);
  if result<>0 then
  begin
    p:=StrCopyE(buf,'Player_');
    i:=0;
    while i<PlyNum do
    begin
      with plyLink^[i] do
      begin
        pp:=p;
        for j:=0 to StrLen(Desc)-1 do
        begin
          if Desc[j] in sLatWord then
            pp^:=UpCase(Desc[j])
          else
            pp^:='_';
          inc(pp);
        end;
        pp^:=#0;
        lhIcon:=LoadImageA(result,buf,IMAGE_ICON,16,16,0);
        if lhIcon>0 then
        begin
          if Icon<>0 then
            DestroyIcon(Icon);
          Icon:=lhIcon;
        end;
      end;
      inc(i);
    end;
    FreeLibrary(result);
  end;
end;

function EnumPlayers(param:MusEnumProc;lParam:LPARAM):bool;
var
  tmp:pPlyArray;
  i,j:integer;
begin
  if (PlyNum>0) and (@param<>nil) then
  begin
    GetMem(tmp,PlyNum*SizeOf(tPlayerCell));
    move(PlyLink^,tmp^,PlyNum*SizeOf(tPlayerCell));
    i:=0;
    j:=PlyNum;
    repeat
      if not param(tmp^[i].Desc,lParam) then break;
      inc(i);
    until i=j;
    FreeMem(tmp);
    result:=true;
  end
  else
    result:=false;
end;

procedure PreProcess; // BASS to start
var
  i:integer;
  tmp:tPlayerCell;
begin
  i:=1;
  while i<(PlyNum-1) do
  begin
    if (plyLink^[i].flags and WAT_OPT_FIRST)<>0 then
    begin
      tmp:=plyLink^[i];
      move(plyLink^[0],plyLink^[1],SizeOf(tPlayerCell)*i);
      plyLink^[0]:=tmp;
{
      move(plyLink^[i],tmp,SizeOf(tPlayerCell));
      move(plyLink^[0],plyLink^[1],SizeOf(tPlayerCell)*i);
      move(tmp,plyLink^[0],SizeOf(tPlayerCell));
}
      break;
    end;
    inc(i);
  end;
  if (plyLink^[0].flags and WAT_OPT_LAST)<>0 then
  begin
    tmp:=plyLink^[0];
    move(plyLink^[1],plyLink^[0],SizeOf(tPlayerCell)*(PlyNum-1));
    plyLink^[PlyNum-1]:=tmp;
{
    move(plyLink^[0],tmp,SizeOf(tPlayerCell));
    move(plyLink^[1],plyLink^[0],SizeOf(tPlayerCell)*(PlyNum-1));
    move(tmp,plyLink^[PlyNum-1],SizeOf(tPlayerCell));
}
  end;
end;

procedure PostProcess; // Winamp clone to the end
var
  i,j:integer;
  tmp:tPlayerCell;
begin
  i:=1;
  j:=PlyNum-1;
  while i<j do
  begin
    if (plyLink^[i].flags and WAT_OPT_LAST)<>0 then
    begin
      tmp:=plyLink^[i];
      move(plyLink^[i+1],plyLink^[i],SizeOf(tPlayerCell)*(PlyNum-i-1));
      plyLink^[PlyNum-1]:=tmp;
{
      move(plyLink^[i],tmp,SizeOf(tPlayerCell));
      move(plyLink^[i+1],plyLink^[i],SizeOf(tPlayerCell)*(PlyNum-i-1));
      move(tmp,plyLink^[PlyNum-1],SizeOf(tPlayerCell));
}//      break;
      i:=1;
      dec(j);
      continue;
    end;
    inc(i);
  end;
end;

function FindPlayer(desc:PAnsiChar):integer;
var
  i:integer;
begin
  if (desc<>nil) and (desc^<>#0) then
  begin
    i:=0;
    while i<PlyNum do
    begin
      if lstrcmpia(plyLink^[i].Desc,desc)=0 then
      begin
        result:=i;
        exit;
      end;
      inc(i);
    end;
  end;
  result:=WAT_RES_NOTFOUND;
end;

function GetPlayerNote(name:PAnsiChar):pWideChar;
var
  i:integer;
begin
  i:=FindPlayer(name);
  if i>=0 then
    result:=plyLink^[i].Notes
  else
    result:=nil;
end;

procedure DefFillPlayerList(hwndList:hwnd);
var
  item:LV_ITEMA;
  lvc:TLVCOLUMN;
  i,newItem:integer;

  il:HIMAGELIST; //!!
begin
  FillChar(item,SizeOf(item),0);
  FillChar(lvc,SizeOf(lvc),0);
  ListView_SetExtendedListViewStyle(hwndList, LVS_EX_CHECKBOXES);
  lvc.mask:=LVCF_FMT or LVCF_WIDTH;

  lvc.fmt:=LVCFMT_LEFT;
  lvc.cx:=160;
  ListView_InsertColumn(hwndList,0,lvc);

  item.mask:=LVIF_TEXT or LVIF_IMAGE; //!!
  i:=0;

  il:=ImageList_Create(16,16,ILC_COLOR32 or ILC_MASK,0,1); //!!
  while i<PlyNum do
  begin
    item.iImage:=ImageList_AddIcon(il,plyLink^[i].Icon);
    item.iItem:=i;
    item.pszText:=plyLink^[i].Desc;
    newItem:=SendMessageA(hwndList,LVM_INSERTITEMA,0,lparam(@item));
    if newItem>=0 then
    begin
      if (plyLink^[i].flags and WAT_OPT_DISABLED)=0 then
        ListView_SetCheckState(hwndList,newItem,TRUE);
    end;
    inc(i);
  end;
  ImageList_Destroy(SendMessage(hwndList,LVM_SETIMAGELIST,LVSIL_SMALL,il)); //!!
//  ListView_SetColumnWidth(hwndList,0,LVSCW_AUTOSIZE);
end;

procedure DefCheckPlayerList(hwndList:hwnd);
var
  i,j,k:integer;
  item:LV_ITEMA;
  szTemp:array [0..109] of AnsiChar;
  p:pPlayerCell;
begin
  FillChar(item,SizeOf(item),0);
  item.mask      :=LVIF_TEXT;
  item.pszText   :=@szTemp;
  item.cchTextMax:=100;
  k:=ListView_GetItemCount(hwndList)-1;
  for i:=0 to k do
  begin
    item.iItem:=i;
    SendMessageA(hwndList,LVM_GETITEMA,0,lparam(@item));
    j:=FindPlayer(item.pszText);
    if j<>WAT_RES_NOTFOUND then
    begin
      p:=@plyLink^[j];
      if ListView_GetCheckState(hwndList,i)=0 then
        p^.flags:=p^.flags or WAT_OPT_DISABLED
      else
        p^.flags:=p^.flags and not WAT_OPT_DISABLED;
    end;
  end;
end;

procedure ClearTemplate(tmpl:pTmplCell);
begin
  with tmpl^ do
  begin
    mFreeMem(p_class);
    mFreeMem(p_text);
    mFreeMem(p_class1);
    mFreeMem(p_text1);
    mFreeMem(p_file);
    mFreeMem(p_prefix);
    mFreeMem(p_postfix);
  end;
  FreeMem(tmpl);
end;

function ServicePlayer(wParam:WPARAM;lParam:LPARAM):int_ptr;cdecl;
var
  p:integer;
  i:integer;
  nl:pPlyArray;
  tmp:tPlayerCell;
begin
  result:=WAT_RES_ERROR;
  if LoWord(wParam)=WAT_ACT_REGISTER then
  begin
    if pPlayerCell(lParam)^.Check=nil then
      exit;
    p:=0;
  end
  else
    p:=FindPlayer(PAnsiChar(lParam));
  case LoWord(wParam) of

    WAT_ACT_REGISTER: begin
      p:=FindPlayer(pPlayerCell(lParam)^.Desc);
      if (p=WAT_RES_NOTFOUND) or ((wParam and WAT_ACT_REPLACE)<>0) then
      begin
        if (p<>WAT_RES_NOTFOUND) and ((plyLink^[p].flags and WAT_OPT_ONLYONE)<>0) then
          exit;

        if p=WAT_RES_NOTFOUND then
        begin
          p:=PlyNum;
          result:=WAT_RES_OK;
          inc(PlyNum);

          if PlyNum>PlyMax then // expand array when append
          begin
            if PlyMax=0 then
              PlyMax:=StartSize
            else
              inc(PlyMax,Step);
            GetMem(nl,PlyMax*SizeOf(tPlayerCell));
            if plyLink<>nil then
            begin
              move(plyLink^,nl^,PlyNum*SizeOf(tPlayerCell));
              FreeMem(plyLink);
            end;
            plyLink:=nl;
          end;
          FillChar(plyLink^[p],SizeOf(tPlayerCell),0);
// doubling notes
          if  (pPlayerCell(lParam)^.Notes<>nil) and
             ((pPlayerCell(lParam)^.flags and WAT_OPT_TEMPLATE)=0) then
          begin
            i:=(StrLenW(pPlayerCell(lParam)^.Notes)+1)*SizeOf(WideChar);
            GetMem(plyLink^[p].Notes,i);
            move(pPlayerCell(lParam)^.Notes^,plyLink^[p].Notes^,i);
          end
          else
            plyLink^[p].Notes:=pPlayerCell(lParam)^.Notes;

// doubling description
          i:=StrLen(pPlayerCell(lParam)^.Desc)+1;
          GetMem(plyLink^[p].Desc,i);
          move(pPlayerCell(lParam)^.Desc^,plyLink^[p].Desc^,i);

// doubling url

          if pPlayerCell(lParam)^.URL<>nil then
          begin
            with plyLink^[p] do
            begin
              i:=StrLen(pPlayerCell(lParam)^.URL)+1;
              GetMem(URL,i);
              move(pPlayerCell(lParam)^.URL^,URL^,i);
            end;
          end
          else
            plyLink^[p].URL:=nil;

        end
        else // existing player
        begin
          if (plyLink^[p].flags and WAT_OPT_TEMPLATE)=0 then
            result:=int_ptr(plyLink^[p].Check)
          else
          begin // remove any info from templates
            result:=WAT_RES_OK;
            ClearTemplate(pTmplCell(plyLink^[p].Check));
          end;
        end;
        // fill info
        with plyLink^[p] do
        begin
          flags:=pPlayerCell(lParam)^.flags;
          if URL<>nil then
            flags:=flags or WAT_OPT_HASURL;
          if pPlayerCell(lParam)^.Icon<>0 then
          begin
            if icon<>0 then
              DestroyIcon(icon);
            icon:=CopyIcon(pPlayerCell(lParam)^.Icon);
          end;
          Init     :=pPlayerCell(lParam)^.Init;
          DeInit   :=pPlayerCell(lParam)^.DeInit;
          Check    :=pPlayerCell(lParam)^.Check;
          GetStatus:=pPlayerCell(lParam)^.GetStatus;
          GetName  :=pPlayerCell(lParam)^.GetName;
          GetInfo  :=pPlayerCell(lParam)^.GetInfo;
          Command  :=pPlayerCell(lParam)^.Command;
          if Init<>nil then
            tInitProc(Init);
        end;

//        PreProcess;
        PostProcess;
      end;
    end;

    WAT_ACT_UNREGISTER: begin
      if p<>WAT_RES_NOTFOUND then
      begin
        dec(PlyNum);
        if plyLink^[p].DeInit<>nil then
          tDeInitProc(plyLink^[p].DeInit);
        FreeMem(plyLink^[p].Desc);
        if (plyLink^[p].flags and WAT_OPT_TEMPLATE)<>0 then
          ClearTemplate(pTmplCell(plyLink^[p].Check));
        if p<PlyNum then // not last
          Move(plyLink^[p+1],plyLink^[p],SizeOf(tPlayerCell)*(PlyNum-p));
        result:=WAT_RES_OK;
      end;
    end;

    WAT_ACT_DISABLE: begin
      if p<>WAT_RES_NOTFOUND then
      begin
        plyLink^[p].flags:=plyLink^[p].flags or WAT_OPT_DISABLED;
        result:=WAT_RES_DISABLED
      end;
    end;

    WAT_ACT_ENABLE: begin
      if p<>WAT_RES_NOTFOUND then
      begin
        plyLink^[p].flags:=plyLink^[p].flags and not WAT_OPT_DISABLED;
        result:=WAT_RES_ENABLED
      end;
    end;

    WAT_ACT_GETSTATUS: begin
      if p<>WAT_RES_NOTFOUND then
      begin
        if (plyLink^[p].flags and WAT_OPT_DISABLED)<>0 then
          result:=WAT_RES_DISABLED
        else
          result:=WAT_RES_ENABLED;
      end;
    end;

    WAT_ACT_SETACTIVE: begin
      if p>0 then
      begin
        tmp:=plyLink^[p];
        move(plyLink^[0],plyLink^[1],SizeOf(tPlayerCell)*p);
        plyLink^[0]:=tmp;
{
        move(plyLink^[p],tmp        ,SizeOf(tPlayerCell));
        move(plyLink^[0],plyLink^[1],SizeOf(tPlayerCell)*p);
        move(tmp        ,plyLink^[0],SizeOf(tPlayerCell));
}
      end;
//      PreProcess;
//      PostProcess;
    end;

  end;
end;

function LoadFromFile(fname:PAnsiChar):integer;
var
  buf:pAnsiChar;
  ptr:PAnsiChar;
  NumPlayers:integer;
  pcell:pTmplCell;
  rec:tPlayerCell;
  st,sec:pointer;
begin
  result:=0;
  st:=OpenStorage(fname);
  if st=nil then exit;

  buf:=GetSectionList(st);
  ptr:=buf;
  NumPlayers:=0;
  while ptr^<>#0 do
  begin
    sec:=SearchSection(st,ptr);

    FillChar(rec,SizeOf(rec),0);

    GetMem(pcell,SizeOf(tTmplCell));
    StrDup(pcell^.p_class ,GetParamSectionStr(sec,'class' ));
    StrDup(pcell^.p_text  ,GetParamSectionStr(sec,'text'  ));
    StrDup(pcell^.p_class1,GetParamSectionStr(sec,'class1'));
    StrDup(pcell^.p_text1 ,GetParamSectionStr(sec,'text1' ));
    StrDup(pcell^.p_file  ,GetParamSectionStr(sec,'file'  ));

    AnsiToWide(GetParamSectionStr(sec,'prefix' ),pcell^.p_prefix );
    AnsiToWide(GetParamSectionStr(sec,'postfix'),pcell^.p_postfix);

    rec.URL  :=GetParamSectionStr(sec,'url');
    rec.Desc :=ptr;
    rec.flags:=GetParamSectionInt(sec,'flags') or WAT_OPT_TEMPLATE;
    rec.Check:=pointer(pcell);

    UTF8ToWide(GetParamSectionStr(sec,'notes'),rec.Notes);

    if ServicePlayer(WAT_ACT_REGISTER,lparam(@rec))=WAT_RES_ERROR then
    begin
      ClearTemplate(pcell);
//      mFreeMem(rec.URL);
      mFreeMem(rec.Notes);
    end
    else
      inc(NumPlayers);

    while ptr^<>#0 do inc(ptr);
    inc(ptr);
  end;

  FreeSectionList(buf);
  CloseStorage(st);
  result:=NumPlayers;
end;

function CheckTmpl(lwnd:HWND;cell:pTmplCell;flags:integer):HWND;
var
  tmp,EXEName:PAnsiChar;
  ltmp,lcycle:boolean;
  lclass,ltext:PAnsiChar;
begin
  lclass:=cell.p_class;
  ltext :=cell.p_text;
  lcycle:=false;
  repeat
    result:=lwnd;
    if (lclass<>nil) or (ltext<>nil) then
      repeat
        result:=FindWindowExA(0,result,lclass,ltext);
        if result=0 then
          break;
//  check filename
        if cell.p_file<>NIL then
        begin
          tmp:=Extract(GetEXEByWnd(result,EXEName),true);
          mFreeMem(EXEName);
          ltmp:=lstrcmpia(tmp,cell.p_file)=0;
          mFreeMem(tmp);
          if not ltmp then
            continue;
        end;
        exit;
      until false;
    if lcycle then break;
    lclass:=cell.p_class1;
    ltext :=cell.p_text1;
    if (lclass=nil) and (ltext=nil) then break;
    lcycle:=not lcycle;
  until false;
end;

// find active player
function CheckAllPlayers(flags:integer;var status:integer; var PlayerChanged:bool):integer;
const
  PrevPlayerName:PAnsiChar=nil;
var
  stat,act,oldstat,i,j:integer;
  tmp:tPlayerCell;
  wwnd,lwnd:HWND;
begin
  i:=0;
  result:=WAT_RES_NOTFOUND;
  PlayerChanged:=true;
  PreProcess;
  oldstat:=-1;
  act:=-1;
  stat:=WAT_MES_UNKNOWN;
  wwnd:=0;
  while i<PlyNum do
  begin
    if (plyLink^[i].flags and WAT_OPT_DISABLED)=0 then
    begin

      lwnd:=0;
      repeat
        wwnd:=0;
        stat:=WAT_MES_UNKNOWN;
        if (plyLink^[i].flags and WAT_OPT_TEMPLATE)<>0 then
        begin
          lwnd:=CheckTmpl(lwnd,plyLink^[i].Check,plyLink^[i].flags);
// find "Winamp" window
          if (lwnd<>THANDLE(WAT_RES_NOTFOUND)) and (lwnd<>0) and
             ((plyLink^[i].flags and WAT_OPT_WINAMPAPI)<>0) then
          begin
            wwnd:=WinampFindWindow(lwnd);
            if wwnd<>0 then
              stat:=WinampGetStatus(wwnd);
          end;
        end
        else
        begin
          with plyLink^[i] do
          begin
            lwnd:=tCheckProc(Check)(lwnd,flags);
            if (lwnd<>THANDLE(WAT_RES_NOTFOUND)) and (lwnd<>0) and (GetStatus<>nil) then
              stat:=tStatusProc(GetStatus)(lwnd);
          end;
        end;
        if (lwnd<>THANDLE(WAT_RES_NOTFOUND)) and (lwnd<>0) then
        begin
          if (stat=WAT_MES_PLAYING) or ((flags and WAT_OPT_CHECKALL)=0) then
          begin
            act   :=i;
            result:=lwnd;
            break;
          end
          else
          begin
            case stat of
              WAT_MES_STOPPED: j:=00;
              WAT_MES_UNKNOWN: j:=10;
              WAT_MES_PAUSED : j:=20;
            else
              j:=00;
            end;
            if oldstat<j then
            begin
              oldstat:=j;
              act    :=i;
              result :=lwnd;
            end;
          end;
        end
        else
          break;
        if (plyLink^[i].flags and WAT_OPT_SINGLEINST)<>0 then
          break;
      until false;
      if (result<>WAT_RES_NOTFOUND) and (result<>0) and
         ((stat=WAT_MES_PLAYING) or ((flags and WAT_OPT_CHECKALL)=0)) then
        break;
    end;
    inc(i);
  end;

  if act>=0 then
  begin
    if result=1 then result:=0 //!! for example, mradio
    else if wwnd<>0 then
      result:=wwnd;
    if act>0 then // to first position
    begin
      tmp:=plyLink^[act];
      move(plyLink^[0],plyLink^[1],SizeOf(tPlayerCell)*act);
      plyLink^[0]:=tmp;
{
      move(plyLink^[act],tmp        ,SizeOf(tPlayerCell));
      move(plyLink^[0  ],plyLink^[1],SizeOf(tPlayerCell)*act);
      move(tmp          ,plyLink^[0],SizeOf(tPlayerCell));
}
    end;
    if PrevPlayerName=plyLink^[0].Desc then
      PlayerChanged:=false
    else
      PrevPlayerName:=plyLink^[0].Desc;
    status:=stat;
  end
  else
  begin
    PrevPlayerName:=nil;
    status:=WAT_PLS_NOTFOUND+WAT_MES_UNKNOWN shl 16;
  end;
  PostProcess;
end;

function TranslateToApp(code:integer):integer;
begin
  case code of
    WAT_CTRL_PREV : result:=APPCOMMAND_MEDIA_PREVIOUSTRACK;
    WAT_CTRL_PLAY : begin
      if IsW2K then // Win2k+ only
        result:=APPCOMMAND_MEDIA_PLAY_PAUSE
      else
        result:=APPCOMMAND_MEDIA_PLAY;
    end;
    WAT_CTRL_PAUSE: result:=APPCOMMAND_MEDIA_PLAY_PAUSE;
    WAT_CTRL_STOP : result:=APPCOMMAND_MEDIA_STOP;
    WAT_CTRL_NEXT : result:=APPCOMMAND_MEDIA_NEXTTRACK;
    WAT_CTRL_VOLDN: result:=APPCOMMAND_VOLUME_DOWN;
    WAT_CTRL_VOLUP: result:=APPCOMMAND_VOLUME_UP;
  else
    result:=-1;
  end;
end;

function SendCommand(wParam:WPARAM;lParam:LPARAM;flags:integer):int_ptr;
var
  dummy:bool;
  wnd:HWND;
  lstat:integer;
begin
  result:=WAT_RES_ERROR;
  wnd:=CheckAllPlayers(flags,lstat,dummy);
  if wnd<>THANDLE(WAT_RES_NOTFOUND) then
    if plyLink^[0].Command<>nil then
      result:=tCommandProc(plyLink^[0].Command)(wnd,wParam,lParam)
    else if (plyLink^[0].flags and WAT_OPT_WINAMPAPI)<>0 then
      result:=WinampCommand(wnd,wParam+(lParam shl 16))
    else if (flags and WAT_OPT_APPCOMMAND)<>0 then
    begin
      result:=TranslateToApp(wParam);
      if result>=0 then
        result:=SendMMCommand(wnd,result);
    end;
end;

// Get Info (default)

function GetSeparator(str:pWideChar):dword;
begin
  result:=StrIndexW(str,' '#$2013' ');
  if result=0 then
    result:=StrIndexW(str,' - ');
  if result<>0 then
  begin
    result:=result-1 + (3 SHL 16);
    exit;
  end;
  result:=StrIndexW(str,#$2013);
  if result=0 then
    result:=StrIndexW(str,'-');
  if result>0 then
    result:=result-1 + (1 SHL 16);
end;

function DefGetTitle(wnd:HWND;fname,wndtxt:pWideChar):pWideChar;
var
  i:integer;
  tmp:pWideChar;
begin
  if fname<>nil then
    tmp:=DeleteKnownExt(ExtractW(fname,true))
  else
    tmp:=wndtxt;
  if tmp=nil then
  begin
    result:=nil;
    exit;
  end;
  StrDupW(result,tmp);
  i:=GetSeparator(result);
  if i>0 then
    StrCopyW(result,result+LoWord(i)+HiWord(i));
  if fname<>nil then
    mFreeMem(tmp);
end;

function DefGetArtist(wnd:HWND;fname,wndtxt:pWideChar):pWideChar;
var
  i:integer;
  tmp:pWideChar;
begin
  if fname<>nil then
    tmp:=DeleteKnownExt(ExtractW(fname,true))
  else
    tmp:=wndtxt;
  if tmp=nil then
  begin
    result:=nil;
    exit;
  end;
  StrDupW(result,tmp);
  i:=GetSeparator(result);
  if i>0 then
    result[LoWord(i)]:=#0;
  if fname<>nil then
    mFreeMem(tmp);
end;

function DefGetVersionText(ver:integer):pWideChar;
begin
  if ver<>0 then
  begin
    mGetMem(result,10*SizeOf(WideChar));
    IntToHex(result,ver);
  end
  else
    result:=nil;
end;

function DefGetWndText(wnd:HWND):pWideChar;
var
  p:pWideChar;
begin
  if wnd<>0 then
  begin
    result:=GetDlgText(wnd);
    if result<>nil then
    begin
      if (plyLink^[0].flags and WAT_OPT_TEMPLATE)<>0 then
      begin
        with pTmplCell(plyLink^[0].Check)^ do
        begin
          if p_prefix<>nil then
          begin
            p:=StrPosW(result,p_prefix);
            if p=result then
              StrCopyW(result,result+StrLenW(p_prefix));
          end;
          if p_postfix<>nil then
          begin
            p:=StrPosW(result,p_postfix);
            if p<>nil then
              p^:=#0;
          end;
        end;
      end;
    end;
  end
  else
    result:=nil;
end;

procedure ClearSongInfoData(var dst:tSongInfo;withFree:bool);
begin
  ClearPlayerInfo  (dst,withFree);
  ClearChangingInfo(dst,withFree);
  ClearFileInfo    (dst,withFree);
  ClearTrackInfo   (dst,withFree);
end;

procedure CopyChangingInfo(const src:tSongInfo;var dst:tSongInfo);
begin
  dst.time   :=src.time;
  dst.volume :=src.volume;
  dst.wndtext:=src.wndtext;
end;

procedure ClearChangingInfo(var dst:tSongInfo;withFree:bool);
begin
  dst.time  :=0;
  dst.volume:=0;

  if withFree then
    mFreeMem(dst.wndtext)
  else
    dst.wndtext:=nil;
end;

procedure CopyFileInfo(const src:tSongInfo;var dst:tSongInfo);
begin
  dst.fsize:=src.fsize;
  dst.date :=src.date;
  dst.mfile:=src.mfile;
end;

procedure ClearFileInfo(var dst:tSongInfo;withFree:bool);
begin
  if withFree then
    mFreeMem(dst.mfile)
  else
    dst.mfile:=nil;
  dst.fsize:=0;
  dst.date :=0;
end;

procedure CopyPlayerInfo(const src:tSongInfo;var dst:tSongInfo);
begin
  dst.player   :=src.player;
  dst.txtver   :=src.txtver;
  dst.url      :=src.url;
  dst.icon     :=src.icon;
  dst.plyver   :=src.plyver;
  dst.plwnd    :=src.plwnd;
  dst.winampwnd:=src.winampwnd;
end;

procedure ClearPlayerInfo(var dst:tSongInfo;withFree:bool);
begin
  if withFree then
  begin
    mFreeMem(dst.player);
    mFreeMem(dst.txtver);
    mFreeMem(dst.url);
    if dst.icon<>0 then
      DestroyIcon(dst.icon);
  end
  else
  begin
    dst.player:=nil;
    dst.txtver:=nil;
    dst.url   :=nil;
  end;
  dst.icon     :=0;
  dst.plyver   :=0;
  dst.plwnd    :=0;
  dst.winampwnd:=0;
end;

procedure CopyTrackInfo(const src:tSongInfo;var dst:tSongInfo);
begin
  dst.artist  :=src.artist;
  dst.title   :=src.title;
  dst.album   :=src.album;
  dst.genre   :=src.genre;
  dst.comment :=src.comment;
  dst.year    :=src.year;
  dst.lyric   :=src.lyric;
  dst.cover   :=src.cover;
  dst.kbps    :=src.kbps;
  dst.khz     :=src.khz;
  dst.channels:=src.channels;
  dst.track   :=src.track;
  dst.total   :=src.total;
  dst.vbr     :=src.vbr;
  dst.codec   :=src.codec;
  dst.width   :=src.width;
  dst.height  :=src.height;
  dst.fps     :=src.fps;
end;

procedure ClearTrackInfo(var dst:tSongInfo;withFree:bool);
begin
  if withFree then
  begin
    mFreeMem(dst.artist);
    mFreeMem(dst.title);
    mFreeMem(dst.album);
    mFreeMem(dst.genre);
    mFreeMem(dst.comment);
    mFreeMem(dst.year);
    mFreeMem(dst.lyric);
    mFreeMem(dst.cover);
  end
  else
  begin
    dst.artist :=nil;
    dst.title  :=nil;
    dst.album  :=nil;
    dst.genre  :=nil;
    dst.comment:=nil;
    dst.year   :=nil;
    dst.lyric  :=nil;
    dst.cover  :=nil;
  end;
  dst.kbps    :=0;
  dst.khz     :=0;
  dst.channels:=0;
  dst.track   :=0;
  dst.total   :=0;
  dst.vbr     :=0;
  dst.codec   :=0;
  dst.width   :=0;
  dst.height  :=0;
  dst.fps     :=0;
end;

function CheckPlayers(var dst:tSongInfo;flags:cardinal):integer;
var
  PlayerChanged:bool;
  fname:pWideChar;
begin
  result:=CheckAllPlayers(flags,dst.status,PlayerChanged);

  if result<>WAT_RES_NOTFOUND then
  begin
    if PlayerChanged then
    begin
      ClearPlayerInfo(dst,false);
      AnsiToWide(plyLink^[0].Desc,dst.player);
      dst.plwnd:=result;
      FastAnsiToWide(plyLink^[0].URL,dst.url);
      if plyLink^[0].icon<>0 then
        dst.icon:=CopyIcon(plyLink^[0].icon)
      else if result<>0 then
      begin
        if GetEXEByWnd(dst.plwnd,fname)<>nil then
        begin
          dst.icon:=ExtractIconW(hInstance,fname,0);
          if dst.icon=1 then
            dst.icon:=0;
          if dst.icon<>0 then
            plyLink^[0].icon:=CopyIcon(dst.icon);
          mFreeMem(fname);
        end;
      end;

      if plyLink^[0].GetInfo<>nil then
        tInfoProc(plyLink^[0].GetInfo)(dst,flags or WAT_OPT_PLAYERDATA)
      else if (plyLink^[0].flags and WAT_OPT_WINAMPAPI)<>0 then
        WinampGetInfo(wparam(@dst),flags or WAT_OPT_PLAYERDATA);
      
     if (plyLink^[0].flags and WAT_OPT_PLAYERINFO)=0 then
       if dst.txtver=NIL then dst.txtver:=DefGetVersionText(dst.plyver);

      result:=WAT_RES_NEWPLAYER;
    end
    else
    begin
      dst.plwnd:=result; // to prevent same player, another instance
      result:=WAT_RES_OK;
    end
  end;
end;

function CheckFile(var dst:tSongInfo;flags:cardinal;timeout:cardinal):integer;
var
  fname:pWideChar;
  tmp:integer;
  remote,FileChanged:boolean;
  f:THANDLE;
  ftime:int64;
begin
  if plyLink^[0].GetName<>nil then
    fname:=tNameProc(plyLink^[0].GetName)(dst.plwnd,flags)
  else
    fname:=nil;

  if (fname=nil) and (dst.plwnd<>0) then
  begin
   tmp:=0;
   if (flags and WAT_OPT_MULTITHREAD)<>0 then tmp:=tmp or gffdMultiThread;
   if (flags and WAT_OPT_KEEPOLD    )<>0 then tmp:=tmp or gffdOld;
   fname:=GetFileFromWnd(dst.plwnd,KnownFileType,tmp,timeout);
  end;

  if fname<>nil then
  begin
    remote:=StrPosW(fname,'://')<>nil;
    // file changing time (local/lan only)
    if not remote then
    begin
      f:=Reset(fname);

      if f<>THANDLE(INVALID_HANDLE_VALUE) then
      begin
        GetFileTime(f,nil,nil,@ftime);
        CloseHandle(f);
      end;
    end;
    // same file
    if (dst.mfile<>nil) and (lstrcmpiw(dst.mfile,fname)=0) then
    begin
      if (not remote) and ((flags and WAT_OPT_CHECKTIME)<>0) then
        FileChanged:=dst.date<>ftime
      else
        FileChanged:=false;
    end
    else  // new filename
    begin
      FileChanged:=true;
    end;

    // if not proper ext (we don't working with it)
    //!!!! check for remotes
    if (not remote) and (CheckExt(fname)=WAT_RES_NOTFOUND) then
    begin
      mFreeMem(fname);
      result:=WAT_RES_NOTFOUND;
      exit;
    end;
    if FileChanged {or isContainer(fname)} then
    begin
      ClearFileInfo(dst,false);
      dst.mfile:=fname; //!! must be when format recognized or remote
      dst.date:=ftime; //!!
      dst.fsize:=GetFSize(dst.mfile);
      result:=WAT_RES_NEWFILE;
    end
    else
    begin
      result:=WAT_RES_OK;
      mFreeMem(fname);
    end;
  end
  else
  begin
    result:=WAT_RES_NOTFOUND;
  end;
end;

// Get Info - main procedure
function GetChangingInfo(var dst:tSongInfo;flags:cardinal):integer;
begin
  result:=WAT_RES_OK;

  ClearChangingInfo(dst,false);

  if plyLink^[0].GetInfo<>nil then
    tInfoProc(plyLink^[0].GetInfo)(dst,flags or WAT_OPT_CHANGES)
  else if (plyLink^[0].flags and WAT_OPT_WINAMPAPI)<>0 then
    WinampGetInfo(wparam(@dst),flags or WAT_OPT_CHANGES);

  if (plyLink^[0].flags and WAT_OPT_PLAYERINFO)=0 then
    if dst.wndtext=NIL then dst.wndtext:=DefGetWndText(dst.plwnd);
end;

function GetInfo(var dst:tSongInfo;flags:cardinal):integer;
var
  oldartist,oldtitle:pWideChar;
  fname:pWideChar;
  remote:boolean;
  lmsnInfo:pMSNInfo;
begin
  result:=WAT_RES_OK;
  remote:=StrPosW(dst.mfile,'://')<>nil;

//  if remote or ((plyLink^[0].flags and WAT_OPT_PLAYERINFO)<>0) then
  oldartist:=dst.artist; oldtitle:=dst.title;

  ClearTrackInfo(dst,false);

  // info from player
  if plyLink^[0].GetInfo<>nil then
    tInfoProc(plyLink^[0].GetInfo)(dst,flags and not WAT_OPT_CHANGES)
  else if (plyLink^[0].flags and WAT_OPT_WINAMPAPI)<>0 then
    WinampGetInfo(wparam(@dst),flags and not WAT_OPT_CHANGES);
  // info from file
  GetFileFormatInfo(dst);

  if (plyLink^[0].flags and WAT_OPT_PLAYERINFO)=0 then
    with dst do
    begin
      if remote then
        fname:=nil
      else
        fname:=mfile;

      lmsnInfo:=GetMSNInfo;

      if lmsnInfo<>nil then
      begin
        if artist=NIL then StrDupW(artist,lmsnInfo.msnArtist);
        if title =NIL then StrDupW(title ,lmsnInfo.msnTitle);
        if album =NIL then StrDupW(album ,lmsnInfo.msnAlbum);
      end;

      if artist=NIL then artist:=DefGetArtist(plwnd,fname,wndtext);
      if title =NIL then title :=DefGetTitle (plwnd,fname,wndtext);
    end;
  if remote or ((plyLink^[0].flags and WAT_OPT_PLAYERINFO)<>0) or
     isContainer(dst.mfile) then
  begin
    if (oldartist=oldtitle) or
       ((oldartist<>nil) and (StrCmpW(dst.artist,oldartist)<>0)) or
       ((oldtitle <>nil) and (StrCmpW(dst.title ,oldtitle )<>0)) then
    begin
      result:=WAT_RES_NEWFILE;
    end;
  end;
end;

procedure ClearPlayers;
begin
  if PlyNum>0 then
  begin
    repeat
      dec(PlyNum);
      with plyLink^[PlyNum] do
      begin
        if DeInit<>nil then
          tDeInitProc(DeInit);
        FreeMem(Desc);
        if URL<>nil then
          FreeMem(URL);
        if icon<>0 then
          DestroyIcon(icon);
        if (flags and WAT_OPT_TEMPLATE)<>0 then
        begin
          ClearTemplate(pTmplCell(Check));
          mFreeMem(Notes);
        end
        else if Notes<>nil then
          FreeMem(Notes);
      end;
    until PlyNum=0;
    FreeMem(plyLink);
  end;
end;

end.
