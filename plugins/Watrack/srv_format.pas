{format service}
unit srv_format;

interface

uses windows,wat_api;

// dialog procedures
procedure DefFillFormatList (hwndList:HWND);
procedure DefCheckFormatList(hwndList:HWND);

// init/free procedures
function ProcessFormatLink:integer;
procedure ClearFormats;

function CheckExt      (fname:pWideChar):integer;
// support functions
function DeleteKnownExt(fname:pWideChar):pWideChar;
function KnownFileType (fname:PWideChar):boolean;
function isContainer   (fname:PWideChar):boolean;

// miranda-like service function
function ServiceFormat(wParam:WPARAM;lParam:LPARAM):integer;cdecl;
// internal helper (can be moved to implementation section)
procedure RegisterFormat(ext:PAnsiChar;proc:tReadFormatProc;flags:dword=0);

type
  TMusEnumProc = function(param:PAnsiChar;lParam:LPARAM):bool;stdcall;

function EnumFormats(param:TMusEnumProc;lParam:LPARAM):bool;
function GetActiveFormat:pMusicFormat;

type
  pwFormat = ^twFormat;
  twFormat = record
    This:tMusicFormat;
    Next:pwFormat;
  end;

const
  FormatLink:pwFormat=nil;
  

implementation

uses
  CommCtrl,common;

type
  pFmtArray = ^tFmtArray;
  tFmtArray = array [0..10] of tMusicFormat;

const
  StartSize = 32;
  Step      = 8;

const
  fmtLink:pFmtArray=nil;
  FmtNum:integer=0;
  FmtMax:integer=0;


function GetActiveFormat:pMusicFormat;
begin
  result:=@fmtLink^[0];
end;

function EnumFormats(param:TMusEnumProc;lParam:LPARAM):bool;
var
  tmp:pFmtArray;
  i,j:integer;
  s:array [0..8] of AnsiChar;
begin
  if (FmtNum>0) and (@param<>nil) then
  begin
    GetMem(tmp,FmtNum*SizeOf(tMusicFormat));
    move(fmtLink^,tmp^,FmtNum*SizeOf(tMusicFormat));
    i:=0;
    j:=FmtNum;
    s[8]:=#0;
    repeat
      move(tmp^[i].ext,s,8);
      if not param(s,lParam) then break;
      inc(i);
    until i=j;
    FreeMem(tmp);
    result:=true;
  end
  else
    result:=false;
end;

function FindFormat(ext:PAnsiChar):integer;
var
  i:integer;
  ss:array [0..7] of AnsiChar;
begin
  i:=0;
  int64(ss):=0;
  StrCopy(ss,ext,7);
  while i<FmtNum do
  begin
    if int64(fmtLink^[i].ext)=int64(ss) then
    begin
      result:=i;
      exit;
    end;
    inc(i);
  end;
  result:=WAT_RES_NOTFOUND;
end;

//----- dialog procedures -----

procedure DefFillFormatList(hwndList:HWND);
var
  item:LV_ITEMA;
  lvc:LV_COLUMN;
  newItem:integer;
  i:integer;
  p:pMusicFormat;
begin
  FillChar(item,SizeOf(item),0);
  FillChar(lvc,SizeOf(lvc),0);
  ListView_SetExtendedListViewStyle(hwndList, LVS_EX_CHECKBOXES);
  lvc.mask:=LVCF_FMT;
  lvc.fmt :={LVCFMT_IMAGE or} LVCFMT_LEFT;
  ListView_InsertColumn(hwndList,0,lvc);

  item.mask:=LVIF_TEXT or LVIF_PARAM;
  item.iItem:=1000;
  i:=0;
  while i<FmtNum do
  begin
    p:=@fmtLink^[i];
    item.pszText:=@p^.ext;
    item.lParam := p^.flags;
    newItem:=SendMessageA(hwndList,LVM_INSERTITEMA,0,LPARAM(@item));
    if newItem>=0 then
    begin
      if (p^.flags and WAT_OPT_DISABLED)=0 then
        ListView_SetCheckState(hwndList,newItem,TRUE);
    end;
    inc(i);
  end;
  ListView_SetColumnWidth(hwndList,0,LVSCW_AUTOSIZE);
end;

procedure DefCheckFormatList(hwndList:HWND);
var
  i,j,k:integer;
  item:LV_ITEMA;
  szTemp:array [0..109] of AnsiChar;
  p:pMusicFormat;
begin
  FillChar(item,SizeOf(item),0);
  item.mask:=LVIF_TEXT;
  item.pszText:=@szTemp;
  item.cchTextMax:=100;
  k:=ListView_GetItemCount(hwndList)-1;
  for i:=0 to k do
  begin
    item.iItem:=i;
    SendMessageA(hwndList,LVM_GETITEMA,0,LPARAM(@item));
    j:=FindFormat(item.pszText);
    if j<>WAT_RES_NOTFOUND then // always?
    begin
      p:=@fmtLink^[j];
      if ListView_GetCheckState(hwndList,i)=0 then
        p^.flags:=p^.flags or WAT_OPT_DISABLED
      else
        p^.flags:=p^.flags and not WAT_OPT_DISABLED;
    end;
  end;
end;


//!! case-sensitive (but GetExt return Upper case ext)
function FindExt(fname:pWideChar):integer;
var
  s :array [0..7] of WideChar;
  ss:array [0..7] of AnsiChar;
  i:integer;
begin
  GetExt(fname,s);
  if s[0]<>#0 then
  begin
    // translate ext to int64
    i:=0;
    int64(ss):=0;
    while (s[i]<>#0) and (i<8) do
    begin
      ss[i]:=AnsiChar(s[i]);
      inc(i);
    end;

    // search number
    i:=0;
    while i<FmtNum do
    begin
      if int64(fmtLink^[i].ext)=int64(ss) then
      begin
        result:=i;
        exit;
      end;
      inc(i);
    end;
  end;

  result:=-1;
end;

function DeleteKnownExt(fname:pWideChar):pWideChar;
var
  i:integer;
begin
  i:=FindExt(fname);
  if i>=0 then
  begin
    i:=StrLen(fmtLink^[i].ext);
    fname[integer(StrLenW(fname))-i-1]:=#0;
  end;

  result:=fname;
end;

function KnownFileType(fname:PWideChar):boolean;
var
  i:integer;
begin
  result:=false;

  i:=FindExt(fname);
  if i>=0 then
  begin
    if ((fmtLink^[i].flags and WAT_OPT_DISABLED)=0) then
      result:=true;
  end;
end;

function CheckExt(fname:pWideChar):integer;
var
  tmp:tMusicFormat;
  i:integer;
begin
  i:=FindExt(fname);
  if i>=0 then
  begin
    if (fmtLink^[i].flags and WAT_OPT_DISABLED)=0 then
    begin
      // move to top
      tmp:=fmtLink^[i];
      move(fmtLink^[0],fmtLink^[1],SizeOf(tMusicFormat)*i);
      fmtLink^[0]:=tmp;

      result:=WAT_RES_OK;
    end
    else
      result:=WAT_RES_DISABLED;
  end
  else
    result:=WAT_RES_NOTFOUND;
end;

function isContainer(fname:PWideChar):boolean;
begin
  if CheckExt(fname)=WAT_RES_OK then
  begin
    result:=(fmtLink^[0].flags and WAT_OPT_CONTAINER)<>0;
  end
  else
    result:=false;
end;


function ServiceFormat(wParam:WPARAM;lParam:LPARAM):integer;cdecl;
var
  p:integer;
  nl:pFmtArray;
begin
  result:=WAT_RES_NOTFOUND;

  if LoWord(wParam)=WAT_ACT_REGISTER then
  begin
    if @pMusicFormat(lParam)^.proc=nil then
      exit;

    p:=FindFormat(pMusicFormat(lParam)^.ext);
    if (p=WAT_RES_NOTFOUND) or ((wParam and WAT_ACT_REPLACE)<>0) then
    begin
      if (p<>WAT_RES_NOTFOUND) and ((fmtLink^[p].flags and WAT_OPT_ONLYONE)<>0) then
        exit;

      if FmtNum=FmtMax then // expand array when append
      begin
        if FmtMax=0 then
          FmtMax:=StartSize
        else
          inc(FmtMax,Step);
        GetMem(nl,FmtMax*SizeOf(tMusicFormat));
        if fmtLink<>nil then
        begin
          move(fmtLink^,nl^,FmtNum*SizeOf(tMusicFormat));
          FreeMem(fmtLink);
        end;
        fmtLink:=nl;
      end;

      if p=WAT_RES_NOTFOUND then
      begin
        p:=FmtNum;
        result:=WAT_RES_OK;
        inc(FmtNum);
      end
      else
        result:=int_ptr(@fmtLink^[p].proc);

      move(pMusicFormat(lParam)^,fmtLink^[p],SizeOf(tMusicFormat));// fill
    end;
  end
  else
  begin
    p:=FindFormat(PAnsiChar(lParam));
    if p<>WAT_RES_NOTFOUND then
      case LoWord(wParam) of
        WAT_ACT_UNREGISTER: begin
          dec(FmtNum);
          if p<FmtNum then // last
            Move(fmtLink^[p+1],fmtLink^[p],SizeOf(tMusicFormat)*(FmtNum-p));
          result:=WAT_RES_OK;
        end;

        WAT_ACT_DISABLE: begin
          fmtLink^[p].flags:=fmtLink^[p].flags or WAT_OPT_DISABLED;
          result:=WAT_RES_DISABLED;
        end;

        WAT_ACT_ENABLE: begin
          fmtLink^[p].flags:=fmtLink^[p].flags and not WAT_OPT_DISABLED;
          result:=WAT_RES_ENABLED;
        end;

        WAT_ACT_GETSTATUS: begin
          if (fmtLink^[p].flags and WAT_OPT_DISABLED)<>0 then
            result:=WAT_RES_DISABLED
          else
            result:=WAT_RES_ENABLED;
        end;
      end;

  end;
end;

//----- init/free procedures -----

procedure RegisterFormat(ext:PAnsiChar;proc:tReadFormatProc;flags:dword=0);
var
  tmp:tMusicFormat;
begin
  FillChar(tmp,SizeOf(tMusicFormat),0);
  StrCopy (tmp.ext,ext,7);
  tmp.proc :=proc;
  tmp.flags:=flags;
  ServiceFormat(WAT_ACT_REGISTER,LPARAM(@tmp));
end;

function ProcessFormatLink:integer;
var
  ptr:pwFormat;
begin
  result:=0;
  ptr:=FormatLink;
  while ptr<>nil do
  begin
    RegisterFormat(@ptr.This.ext, ptr.This.proc, ptr.This.flags);
    inc(result);
    ptr:=ptr^.Next;
  end;
end;

procedure ClearFormats;
begin
  if FmtNum>0 then
    FreeMem(fmtLink);
end;

end.
