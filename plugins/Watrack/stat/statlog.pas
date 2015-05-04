{Statistic}
unit StatLog;
{$include compilers.inc}
interface
{$Resource stat.res}
implementation

uses windows,messages,shellapi,commctrl
     ,wrapper,io,wat_api,common,global,m_api,dbsettings,mirutils;

{$include resource.inc}
{$include stat_data.inc}
{$include stat_vars.inc}
{$include stat_opt.inc}

type
  pStatCell = ^tStatCell;
  tStatCell = record
    Count    :integer;
    AltCount :integer;
    LastTime :dword;
    Length   :integer;
    Artist   :PAnsiChar;
    Title    :PAnsiChar;
    MFile    :PAnsiChar;
    Album    :PAnsiChar;
    Next     :pStatCell; // only for fill
  end;

type
  pCells = ^tCells;
  tCells = record
    Count:integer;
    Cells:array [0..1] of pStatCell
  end;

const
  IcoBtnReport:PAnsiChar='WATrack_Report';
const
  DelimChar = '|';
const
  buflen = 2048;

const
  Lock:boolean=false;

procedure err(str:PWideChar);
begin
  MessageBoxW(0,TranslateW(str),TranslateW('Music Statistic'),MB_OK);
end;

function OnlyPath(dst,src:PAnsiChar):PAnsiChar;
var
  i:integer;
begin
  i:=StrLen(src)-1;
  while (i>0) and (src[i]<>'\') do dec(i);
  StrCopy(dst,src,i);
  result:=dst;
end;

function PackTime(aTime:TSYSTEMTIME):dword;
begin
  with aTime do
    result:=wSecond+
      (wMinute shl 06)+
      (wHour   shl 12)+
      (wDay    shl 17)+
      (wMonth  shl 22)+
      (((wYear-2000) and $3F) shl 26);
end;

procedure UnPackTime(aTime:dword;var MyTime:TSYSTEMTIME);
begin
  with MyTime do
  begin
    wYear  :=(aTime shr 26)+2000;
    wMonth :=(aTime shr 22) and $0F;
    wDay   :=(aTime shr 17) and $1F;
    wHour  :=(aTime shr 12) and $1F;
    wMinute:=(aTime shr 6 ) and $3F;
    wSecond:=aTime and $3F;
  end;
end;

function ShowTime(buf:PAnsiChar;aTime:dword):PAnsiChar;
var
  MyTime:TSYSTEMTIME;
begin
  UnPackTime(aTime,MyTime);
  with MyTime do
  begin
    IntToStr(buf   ,wDay   ,2);
    IntToStr(buf+3 ,wMonth ,2);
    IntToStr(buf+6 ,wYear  ,2);
    IntToStr(buf+9 ,wHour  ,2);
    IntToStr(buf+12,wMinute,2);
    IntToStr(buf+15,wSecond,2);
  end;
  buf[2] :='.'; buf[5] :='.'; buf[8] :=' ';
  buf[11]:=':'; buf[14]:=':'; buf[17]:=#0;
  result:=buf;
end;

function AppendStr(src:PAnsiChar;var dst:PAnsiChar):PAnsiChar; overload;
begin
  dst^:=DelimChar; inc(dst);
  while src^<>#0 do
  begin
    dst^:=src^;
    inc(dst);
    inc(src);
  end;
  result:=dst;
end;

function AppendStr(src:PWideChar;var dst:PAnsiChar):PAnsiChar; overload;
var
  p,lp:PAnsiChar;
begin
  dst^:=DelimChar; inc(dst);
  lp:=WideToUTF8(src,p);
  while lp^<>#0 do
  begin
    dst^:=lp^;
    inc(dst);
    inc(lp);
  end;
  mFreeMem(p);
  result:=dst;
end;

procedure AppendStat(fname:PAnsiChar;si:pSongInfo);
var
  f:THANDLE;
  MyTime:TSYSTEMTIME;
  buf:array [0..buflen-1] of char;
  lp:PAnsiChar;
begin
  if Lock then
    exit;
  if (si^.artist=NIL) and (si^.title=NIL) and
     (si^.album =NIL) and (si^.mfile=NIL) then
    exit;
  f:=Append(fname);
//  if THANDLE(f)=INVALID_HANDLE_VALUE then f:=Rewrite(fname);
  if f=THANDLE(INVALID_HANDLE_VALUE) then exit;
  FillChar(buf,SizeOf(buf),0);
  lp:=@buf;
  buf[0]:='1'; buf[1]:=DelimChar; inc(lp,2); // Count

  GetLocalTime(MyTime);
  IntToStr(lp,PackTime(MyTime),9);
  inc(lp,9);
  lp^:=DelimChar;
  inc(lp);
  IntToStr(lp,si^.total); while lp^<>#0 do inc(lp);

  AppendStr(si^.artist,lp);
  AppendStr(si^.title ,lp);
  AppendStr(si^.mfile ,lp);
  AppendStr(si^.album ,lp);

  lp^:=#$0D; inc(lp); lp^:=#$0A;
  BlockWrite(f,buf,lp-PAnsiChar(@buf)+1);
  CloseHandle(f);
end;

procedure OutputStat(fname:PAnsiChar;aCells:pCells);
var
  f:THANDLE;
  buf:array [0..2047] of char;
  lp:PAnsiChar;
  i:integer;
begin
  f:=Rewrite(fname);
  if f=THANDLE(INVALID_HANDLE_VALUE) then
    exit;
  for i:=0 to aCells^.Count-1 do
  begin
    lp:=@buf;
    with aCells^.Cells[i]^ do
    begin
      IntToStr(buf,Count); while lp^<>#0 do inc(lp);
      lp^:=DelimChar; inc(lp);
      IntToStr(lp,LastTime,9); inc(lp,9);
      lp^:=DelimChar; inc(lp);
      IntToStr(lp,Length); while lp^<>#0 do inc(lp);
      AppendStr(Artist,lp);
      AppendStr(Title ,lp);
      AppendStr(MFile ,lp);
      AppendStr(Album ,lp);

      lp^:=#$0D; inc(lp); lp^:=#$0A;
      BlockWrite(f,buf,lp-PAnsiChar(@buf)+1);
    end;
  end;
  CloseHandle(f);
end;

function CutStr(var src:PAnsiChar):PAnsiChar;
begin
  result:=src;
  while (src^<>DelimChar) and (src^>=' ') do inc(src);
  src^:=#0;
  inc(src);
end;

procedure ClearStatCells(aCells:pCells);
begin
  with aCells^ do
    while Count>0 do
    begin
      dec(Count);
      mFreeMem(Cells[Count]);
    end;
  mFreeMem(aCells);
end;

function FillCell(src:PAnsiChar):pStatCell;
var
  Cell:pStatCell;
begin
  mGetMem(Cell,SizeOf(tStatCell));
  FillChar(Cell^,SizeOf(tStatCell),0);
  Cell^.Count   :=StrToInt(src);
  while src^<>DelimChar do inc(src); inc(src);
  Cell^.LastTime:=StrToInt(src);
  while src^<>DelimChar do inc(src); inc(src);
  Cell^.Length  :=StrToInt(src);
  while src^<>DelimChar do inc(src); inc(src);
  Cell^.Artist:=CutStr(src);
  Cell^.Title :=CutStr(src);
  Cell^.MFile :=CutStr(src);
  Cell^.Album :=CutStr(src);

  result:=Cell;
end;

function Compare(C1,C2:pStatCell; SortType:integer):integer;
var
  ls,ls1:array [0..511] of AnsiChar;
begin
  case SortType of
    stArtist: begin
      result:=lstrcmpia(C1^.Artist,C2^.Artist);
      if result=0 then
        result:=lstrcmpia(C1^.Title,C2^.Title);
      if result=0 then
        result:=lstrcmpia(C1^.Album,C2^.Album);
    end;
    stAlbum: result:=lstrcmpia(C1^.Album,C2^.Album);
    stPath : result:=lstrcmpia(OnlyPath(ls,C1^.MFile),OnlyPath(ls1,C2^.MFile));
    stDate    : result:=C2^.LastTime-C1^.LastTime;
    stCount   : result:=C2^.Count-C1^.Count;
    stLength  : result:=C2^.Length-C1^.Length;
    stAltCount: result:=C2^.AltCount-C1.AltCount;
  else
    result:=0;
  end;
end;

function SwapProc(var Root:pCells;First,Second:integer):integer;
var
  p:pStatCell;
begin
  p:=Root^.Cells[First];
  Root^.Cells[First]:=Root^.Cells[Second];
  Root^.Cells[Second]:=p;
  result:=0;
end;

procedure Resort(var Root:pCells;Sort:integer;aDirection:integer=smDirect);

  function CompareProc(First,Second:integer):integer;
  begin
    result:=Compare(Root^.Cells[First],Root^.Cells[Second],sort);
    if aDirection=smReverse then
      result:=-result;
  end;

var
  i,j,gap:longint;
begin
  gap:=Root^.Count shr 1;
  while gap>0 do
  begin
    for i:=gap to Root^.Count-1 do
    begin
      j:=i-gap;
      while (j>=0) and (CompareProc(j,UInt(j+gap))>0) do
      begin
        SwapProc(Root,j,UInt(j+gap));
        dec(j,gap);
      end;
    end;
    gap:=gap shr 1;
  end;
// now pack doubles
end;

function BuildTree(fname:PAnsiChar;var buffer:PAnsiChar):pCells;
var
  f:THANDLE;
  i,cnt:integer;
  FirstCell,CurCell,Cell:pStatCell;
  lRec:TWin32FindDataA;//WIN32_FIND_DATAA;
  h:THANDLE;
  p,p1,p2:PAnsiChar;
  ls,buf:PAnsiChar;
  arr:pCells;
begin
  result:=nil;
  buffer:=nil;
  h:=FindFirstFileA(fname,lRec);
  if h=THANDLE(INVALID_HANDLE_VALUE) then
    exit;
  i:=lRec.nFileSizeLow;
  FindClose(h);
  if i<22 then
    Exit;
  f:=Reset(fname);
  if f=THANDLE(INVALID_HANDLE_VALUE) then
    exit;
  mGetMem(buffer,i+1);
  p:=buffer;
  BlockRead(f,p^,i);
  CloseHandle(f);
  p1:=p;
  p2:=p+i;
  FirstCell:=nil;
  mGetMem(buf,buflen);
  buf^:=#0;
  cnt:=0;
  while p<p2 do
  begin
    while p^<>#$0D do inc(p);
    i:=p-p1;
    p^:=#0;
    if i>=20 then //min log template + min fname [d:\.e]
    begin
      ls:=p1;
// skip duplicates one-by-one
      while ls^<>DelimChar do inc(ls); inc(ls); // Count
      while ls^<>DelimChar do inc(ls); inc(ls); // time
      while ls^<>DelimChar do inc(ls); inc(ls); // length
      if StrCmp(buf,ls)<>0 then
      begin
        inc(cnt);
        StrCopy(buf,ls);
        Cell:=FillCell(p1);

        if FirstCell=nil then
        begin
          FirstCell:=Cell;
          CurCell  :=FirstCell;
        end
        else
        begin
          CurCell^.Next:=Cell;
          CurCell:=Cell;
        end;
      end;
    end;
    inc(p,2); p1:=p;
  end;
  mFreeMem(buf);
  // Fill array
  if cnt>0 then
  begin
    mGetMem(arr,SizeOf(integer)+cnt*SizeOf(pStatCell));
    arr^.Count:=cnt;
    CurCell:=FirstCell;
    i:=0;
    while CurCell<>nil do
    begin
      arr^.Cells[i]:=CurCell;
      CurCell:=CurCell.Next;
      inc(i);
    end;
    result:=arr;
    // sort & pack
    Resort(arr,stArtist);

    i:=1;
    Cell:=arr^.Cells[0];
    while i<arr^.Count do
    begin
      with arr^.Cells[i]^ do
        if (lstrcmpia(Cell^.Artist,Artist)=0) and
           (lstrcmpia(Cell^.Title,Title)=0) and
           (lstrcmpia(Cell^.Album,Album)=0) then
        begin
          if Cell^.LastTime<LastTime then
            Cell^.LastTime:=LastTime;
          inc(Cell^.Count,Count);
          dec(arr^.Count);
          if i<arr^.Count then
            move(arr^.Cells[i+1],arr^.Cells[i],SizeOf(pStatCell)*(arr^.Count-i));
          continue;
        end
        else
          Cell:=arr^.Cells[i];
      inc(i);
    end;

  end;
end;

procedure SortFile(fname:PAnsiChar;mode:integer;aDirection:integer);
var
  Root:pCells;
  buf:PAnsiChar;
  buf1:array [0..511] of AnsiChar;
begin
  Lock:=true;
  ConvertFileName(fname,buf1);
//  CallService(MS_UTILS_PATHTOABSOLUTE,twparam(fname),tlparam(@buf1));
  Root:=BuildTree(buf1,buf);
  if Root<>nil then
  begin
    if (mode<>stArtist) or (aDirection<>smDirect) then
      Resort(Root,mode,aDirection);
    OutputStat(buf1,Root);
    ClearStatCells(Root);
  end;
  mFreeMem(buf);
  Lock:=false;
end;

{$include report.inc}

// --------------- service functions -----------------

function ThAddToLog(param:pdword):dword; stdcall;
begin
  result:=0;
end;

procedure ThPackLog(param:pdword); cdecl;
begin
  SortFile(StatName,SortMode,Direction);
end;

function ThMakeReport(param:pdword):dword; stdcall;
begin
  result:=0;
end;

function AddToLog(wParam:WPARAM;lParam:LPARAM):integer;cdecl;
var
  fname:PAnsiChar;
  log:array [0..511] of AnsiChar;
begin
  result:=0;
  if (StatName=nil) or (StatName[0]=#0) then
    exit;
  if wParam=0 then
    fname:=StatName
  else
    fname:=PAnsiChar(wParam);
  ConvertFileName(fname,log);
//  CallService(MS_UTILS_PATHTOABSOLUTE,twparam(fname),tlparam(@log));
  AppendStat(log,pSongInfo(lParam));
end;

function PackLog(wParam:WPARAM;lParam:LPARAM):integer;cdecl;
begin
  result:=0;
  {CloseHandle}(mir_forkthread(@ThPackLog,nil));
end;

function MakeReport(wParam:WPARAM;lParam:LPARAM):integer;cdecl;
var
  report,log,template:array [0..511] of AnsiChar;
  l,r:PAnsiChar;
begin
  result:=0;
  if CallService(MS_WAT_PLUGINSTATUS,2,0)=WAT_RES_DISABLED then
    exit;
  if (wParam<>0) and (wParam<>MenuReportPos) then
    l:=PAnsiChar(wParam)
  else
    l:=TmplName;
  if PAnsiChar(lParam)<>nil then r:=PAnsiChar(lParam) else r:=ReportName;
  if (r=nil) or (r^=#0) then
    err('Report file name not defined')
  else if (StatName=nil) or (StatName^=#0) then
    err('Log file name not defined')
  else
  begin
    ConvertFileName(r,report);
    ConvertFileName(l,template);
    ConvertFileName(StatName,log);
//    CallService(MS_UTILS_PATHTOABSOLUTE,twparam(r),tlparam(@report));
//    CallService(MS_UTILS_PATHTOABSOLUTE,twparam(l),tlparam(@template));
//    CallService(MS_UTILS_PATHTOABSOLUTE,twparam(StatName),tlparam(@log));
    if DoAddExt=BST_CHECKED then
      ChangeExt(report,'htm');
    if StatOut(report,log,template) then
    begin
      if RunReport=BST_CHECKED then
      begin
        ShellExecuteA(0,nil{'open'},report,nil,nil,SW_SHOWNORMAL);
      end;
      result:=1;
    end
    else
      err('Oops, something wrong!');
  end;
end;

{$include stat_dlg.inc}

function NewPlStatus(wParam:WPARAM;lParam:LPARAM):int;cdecl;
var
  flag:integer;
  mi:tClistMenuItem;
  CurTime:dword;
begin
  result:=0;
  case wParam of
    WAT_EVENT_NEWTRACK: begin
      if (StatName<>nil) and (StatName[0]<>#0) then
      begin
        AppendStat(StatName,pSongInfo(lParam));
        if AutoSort>0 then
        begin
          CurTime:=GetCurrentTime;
          if (CurTime-LastSort)>=(86400*AutoSort) then
          begin
            SortFile(StatName,SortMode,Direction);  //PackLog(0,0);
            LastSort:=CurTime;
            DBWriteDWord(0,PluginShort,opt_LastSort,LastSort);
          end;
        end;
      end;
    end;
    WAT_EVENT_PLUGINSTATUS: begin
      case lParam of
        0: flag:=0;
        2: flag:=CMIF_GRAYED;
      else // like 1
        exit
      end;
      FillChar(mi,sizeof(mi),0);
      mi.cbSize:=sizeof(mi);
      mi.flags :=CMIM_FLAGS+flag;
      CallService(MS_CLIST_MODIFYMENUITEM,hMenuReport,tlparam(@mi));
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
  mi.hIcon :=CallService(MS_SKIN2_GETICON,0,tlparam(IcoBtnReport));
  CallService(MS_CLIST_MODIFYMENUITEM,hMenuReport,tlparam(@mi));
end;

// ------------ base interface functions -------------

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

  hPackLog   :=CreateServiceFunction(MS_WAT_PACKLOG   ,@PackLog);
  hMakeReport:=CreateServiceFunction(MS_WAT_MAKEREPORT,@MakeReport);
  hAddToLog  :=CreateServiceFunction(MS_WAT_ADDTOLOG  ,@AddToLog);
  loadstat;

  FillChar(sid,SizeOf(TSKINICONDESC),0);
  sid.cbSize:=SizeOf(TSKINICONDESC);
  sid.cx:=16;
  sid.cy:=16;
  sid.szSection.a:='WATrack';
  sid.hDefaultIcon   :=LoadImage(hInstance,MAKEINTRESOURCE(BTN_REPORT),IMAGE_ICON,16,16,0);
  sid.pszName        :=IcoBtnReport;
  sid.szDescription.a:='Create Report';
  Skin_AddIcon(@sid);
  DestroyIcon(sid.hDefaultIcon);
  sic:=HookEvent(ME_SKIN2_ICONSCHANGED,@IconChanged);

  FillChar(mi, sizeof(mi), 0);
  mi.cbSize       :=sizeof(mi);
  mi.flags        :=0;
  mi.szPopupName.a:=PluginShort;
  mi.hIcon        :=CallService(MS_SKIN2_GETICON,0,tlparam(IcoBtnReport));
  mi.szName.a     :='Create WATrack report';
  mi.pszService   :=MS_WAT_MAKEREPORT;
  mi.popupPosition:=MenuReportPos;
  hMenuReport :=Menu_AddMainMenuItem(@mi);
  plStatusHook:=HookEvent(ME_WAT_NEWSTATUS,@NewPlStatus);
end;

procedure DeInitProc(aSetDisable:boolean);
begin
  if aSetDisable then
    SetModStatus(0);

  CallService(MO_REMOVEMENUITEM,hMenuReport,0);
  UnhookEvent(plStatusHook);
  UnhookEvent(sic);
  DestroyServiceFunction(hPackLog);
  DestroyServiceFunction(hMakeReport);
  DestroyServiceFunction(hAddToLog);
  FreeStat;
end;

function AddOptionsPage(var tmpl:pAnsiChar;var proc:pointer;var name:PAnsiChar):integer;
begin
  tmpl:=PAnsiChar(IDD_OPT_STATS);
  proc:=@DlgProcOptions;
  name:='Statistics';
  result:=0;
end;

var
  Stat:twModule;

procedure Init;
begin
  Stat.Next      :=ModuleLink;
  Stat.Init      :=@InitProc;
  Stat.DeInit    :=@DeInitProc;
  Stat.AddOption :=@AddOptionsPage;
  Stat.Check     :=nil;
  Stat.ModuleName:='Statistic';
  ModuleLink     :=@Stat;
end;

begin
  Init;
end.
