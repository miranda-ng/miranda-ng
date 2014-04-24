{player service}
unit srv_player;

interface

uses windows,common,wat_api;

// init/free procedures
function ProcessPlayerLink:integer;
procedure ClearPlayers;
// miranda-style function for new player registering
function ServicePlayer(wParam:WPARAM;lParam:LPARAM):int_ptr;cdecl;
// next 2 functions are public coz in watrack they going with GetAddonFileName funcion
// which uses some miranda things
function SetPlayerIcons(fname:pAnsiChar):integer;
function LoadFromFile(fname:PAnsiChar):integer;

function SendCommand(wParam:WPARAM;lParam:LPARAM;flags:integer):int_ptr;

// options procedures
procedure DefFillPlayerList (hwndList:HWND);
procedure DefCheckPlayerList(hwndList:HWND);

type
  MusEnumProc = function(param:PAnsiChar;lParam:LPARAM):bool;stdcall;

function EnumPlayers(param:MusEnumProc;lParam:LPARAM):bool;
function GetPlayerNote(name:PAnsiChar):pWideChar;
function CheckAllPlayers(flags:integer;var status:integer; var PlayerChanged:bool):HWND;
function GetActivePlayer:pPlayerCell;

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
  CommCtrl, shellapi,
  appcmdapi,winampapi,memini,syswin;

type
  pPlyArray = ^tPlyArray;
  tPlyArray = array [0..10] of tPlayerCell;

const
  StartSize = 32;
  Step      = 8;
  buflen    = 2048;

const
  plyLink:pPlyArray=nil;
  PlyNum:integer=0;
  PlyMax:integer=0;

//----- Support functions -----

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

      break;
    end;
    inc(i);
  end;
  if (plyLink^[0].flags and WAT_OPT_LAST)<>0 then
  begin
    tmp:=plyLink^[0];
    move(plyLink^[1],plyLink^[0],SizeOf(tPlayerCell)*(PlyNum-1));
    plyLink^[PlyNum-1]:=tmp;
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

//----- public functions -----

function GetActivePlayer:pPlayerCell;
begin
  result:=@plyLink^[0];
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

//----- Init/free -----

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

    p:=FindPlayer(pPlayerCell(lParam)^.Desc);
    if (p=WAT_RES_NOTFOUND) or ((wParam and WAT_ACT_REPLACE)<>0) then
    begin
      if (p<>WAT_RES_NOTFOUND) and ((plyLink^[p].flags and WAT_OPT_ONLYONE)<>0) then
        exit;

      if p=WAT_RES_NOTFOUND then
      begin
        p:=PlyNum;
        result:=WAT_RES_OK;

        if PlyNum=PlyMax then // expand array when append
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

        inc(PlyNum);
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
  end
  else
  begin
    p:=FindPlayer(PAnsiChar(lParam));
    if p<>WAT_RES_NOTFOUND then
      case LoWord(wParam) of
        WAT_ACT_UNREGISTER: begin
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

        WAT_ACT_DISABLE: begin
          plyLink^[p].flags:=plyLink^[p].flags or WAT_OPT_DISABLED;
          result:=WAT_RES_DISABLED;
        end;

        WAT_ACT_ENABLE: begin
          plyLink^[p].flags:=plyLink^[p].flags and not WAT_OPT_DISABLED;
          result:=WAT_RES_ENABLED;
        end;

        WAT_ACT_GETSTATUS: begin
          if (plyLink^[p].flags and WAT_OPT_DISABLED)<>0 then
            result:=WAT_RES_DISABLED
          else
            result:=WAT_RES_ENABLED;
        end;

        WAT_ACT_SETACTIVE: begin
          if p>0 then
          begin
            tmp:=plyLink^[p];
            move(plyLink^[0],plyLink^[1],SizeOf(tPlayerCell)*p);
            plyLink^[0]:=tmp;
          end;
    //      PreProcess;
    //      PostProcess;
        end;

      end;

  end;
end;

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

//----- options procedures -----

procedure DefFillPlayerList(hwndList:HWND);
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

procedure DefCheckPlayerList(hwndList:HWND);
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

//----- Active player search -----

function CheckTmpl(lwnd:HWND;cell:pTmplCell;flags:integer):HWND;
var
  tmp,EXEName:PAnsiChar;
  lclass,ltext:PAnsiChar;
  ltmp,lcycle:boolean;
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
    // repeat for alternative window
    if lcycle then break;
    lclass:=cell.p_class1;
    ltext :=cell.p_text1;
    if (lclass=nil) and (ltext=nil) then break;
    lcycle:=not lcycle;
  until false;
end;

// find active player
function CheckAllPlayers(flags:integer;var status:integer; var PlayerChanged:bool):HWND;
const
  PrevPlayerName:PAnsiChar=nil;
var
  wwnd,lwnd:HWND;
  tmp:tPlayerCell;
  fname:pWideChar;
  stat,act,oldstat,i,j:integer;
begin
  result:=HWND(WAT_RES_NOTFOUND);

  i:=0;
  PlayerChanged:=true;
  PreProcess;
  oldstat:=-1;
  act:=-1;
  // for case when no any player enabled/registered
  stat:=WAT_PLS_UNKNOWN;
  wwnd:=0;

  while i<PlyNum do
  begin
    if (plyLink^[i].flags and WAT_OPT_DISABLED)=0 then
    begin

      lwnd:=0;
      repeat
        wwnd:=0;
        stat:=WAT_PLS_UNKNOWN;

        // template player
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
        // separate processing
        else
        begin
          with plyLink^[i] do
          begin
            lwnd:=tCheckProc(Check)(lwnd,flags);
            if (lwnd<>THANDLE(WAT_RES_NOTFOUND)) and (lwnd<>0) and (GetStatus<>nil) then
              stat:=tStatusProc(GetStatus)(lwnd);
          end;
        end;

        // player window found
        if (lwnd<>THANDLE(WAT_RES_NOTFOUND)) and (lwnd<>0) then
        begin
          if (stat=WAT_PLS_PLAYING) or ((flags and WAT_OPT_CHECKALL)=0) then
          begin
            act   :=i;
            result:=lwnd;
            break;
          end
          else
          begin
            case stat of
              WAT_PLS_STOPPED: j:=00;
              WAT_PLS_UNKNOWN: j:=10;
              WAT_PLS_PAUSED : j:=20;
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

      if (result<>HWND(WAT_RES_NOTFOUND)) and (result<>0) and
         ((stat=WAT_PLS_PLAYING) or ((flags and WAT_OPT_CHECKALL)=0)) then
        break;
    end;
    inc(i);
  end;

  // hmm, we found player
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
    end;

    if PrevPlayerName=plyLink^[0].Desc then
      PlayerChanged:=false
    else
    begin
      PrevPlayerName:=plyLink^[0].Desc;
      // get player icon if didn't has it before
      if (plyLink^[0].icon=0) and (result>0) then
      begin
        if GetEXEByWnd(result,fname)<>nil then
        begin
          plyLink^[0].icon:=ExtractIconW(hInstance,fname,0);
          if plyLink^[0].icon=1 then
            plyLink^[0].icon:=0;
          mFreeMem(fname);
        end;
      end;
    end;
    status:=stat;
  end
  else
  begin
    PrevPlayerName:=nil;
    status:=WAT_PLS_NOTFOUND;
  end;

  PostProcess;
end;

//----- Send command to player -----

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

end.
