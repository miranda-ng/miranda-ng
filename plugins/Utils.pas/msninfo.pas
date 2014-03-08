unit MSNInfo;

interface

type
  pMSNInfo = ^tMSNInfo;
  tMSNInfo = record
    msnPlayer:pWideChar;
    msnArtist:pWideChar;
    msnTitle :pWideChar;
    msnAlbum :pWideChar;
  end;


procedure StartMSNHook;
procedure StopMSNHook;

function GetMSNInfo:pMSNInfo;

implementation

uses windows, messages, common;

const
  HWND_MESSAGE = HWND(-3);

const
  MSNClassName = 'MsnMsgrUIManager';
const
  hMSNWindow:THANDLE = 0;

const
  RealMSNData:PWideChar = nil;
  anMSNInfo:tMSNInfo =(
    msnPlayer:nil;
    msnArtist:nil;
    msnTitle :nil;
    msnAlbum :nil
  );

function GetMSNInfo:pMSNInfo;
begin
  if ((anMSNInfo.msnPlayer=nil) or (anMSNInfo.msnPlayer^=#0)) and
     ((anMSNInfo.msnArtist=nil) or (anMSNInfo.msnArtist^=#0)) and
     ((anMSNInfo.msnTitle =nil) or (anMSNInfo.msnTitle ^=#0)) and
     ((anMSNInfo.msnAlbum =nil) or (anMSNInfo.msnAlbum ^=#0)) then
    result:=nil
  else
    result:=@anMSNInfo;
end;

procedure ClearMSNInfo;
begin
  if RealMSNData<>nil then
  begin
    mFreeMem(RealMSNData);
    RealMSNData:=nil;
  end;
  FillChar(anMSNInfo,SizeOf(anMSNInfo),0);
  {FreeMem(anMSNInfo.msnPlayer);} //anMSNInfo.msnPlayer:=nil;
  {FreeMem(anMSNInfo.msnArtist);} //anMSNInfo.msnArtist:=nil;
  {FreeMem(anMSNInfo.msnTitle); } //anMSNInfo.msnTitle :=nil;
  {FreeMem(anMSNInfo.msnAlbum); } //anMSNInfo.msnAlbum :=nil;
end;

procedure Split(pc:pWideChar);
var
  lpc:pWideChar;
begin
  // Player
  anMSNInfo.msnPlayer:=pc;
  while (pc^<>'\') or ((pc+1)^<>'0') do inc(pc);
  pc^:=#0;
  inc(pc,2);

  // Type
  lpc:=pc;

  while (pc^<>'\') or ((pc+1)^<>'0') do inc(pc);
  pc^:=#0;
  inc(pc,2);
  
  if StrCmpW(lpc,'Music')<>0 then
  begin
    anMSNInfo.msnPlayer:=nil;
    exit;
  end;

  // Status
  lpc:=pc;
  if lpc^='0' then // stop track
  begin
    anMSNInfo.msnPlayer:=nil;
    exit;
  end;

  while (pc^<>'\') or ((pc+1)^<>'0') do inc(pc);
  pc^:=#0;
  inc(pc,2);

  // Format - just skip
  while (pc^<>'\') or ((pc+1)^<>'0') do inc(pc);
  pc^:=#0;
  inc(pc,2);

  // Artist
  anMSNInfo.msnArtist:=pc;

  while (pc^<>'\') or ((pc+1)^<>'0') do inc(pc);
  pc^:=#0;
  inc(pc,2);
  
  // Title
  anMSNInfo.msnTitle:=pc;

  while (pc^<>'\') or ((pc+1)^<>'0') do inc(pc);
  pc^:=#0;
  inc(pc,2);

  // Album
  anMSNInfo.msnAlbum:=pc;

  while (pc^<>'\') or ((pc+1)^<>'0') do inc(pc);
  pc^:=#0;
  
  // WMContentID - not needs
end;

function dlgMSNHook(Dialog:HWND;hMessage:uint;wParam:WPARAM;lParam:LPARAM):LRESULT; stdcall;
var
  pMyCDS:PCOPYDATASTRUCT;
begin
  result:=0;
  case hMessage of
    WM_COPYDATA: begin
      pMyCDS:=PCOPYDATASTRUCT(lParam);
      if pMyCDS^.dwData=1351 then // Media player info
      begin
        ClearMSNInfo;
        Split(StrDupW(RealMSNData,pWideChar(pMyCDS^.lpData)));
      end;
    end;
  else
    result:=DefWindowProc(Dialog,hMessage,wParam,lParam);
  end;
end;

procedure StartMSNhook;
var
  msnClass:TWNDCLASSW;
  hMSNClass:THANDLE;
begin
  FillChar(msnClass,SizeOf(TWNDCLASS),0);

  msnClass.hInstance    :=hInstance;
  msnClass.lpszClassName:=MSNClassName;
  msnClass.lpfnWndProc  :=@dlgMSNHook;
  hMSNClass:=RegisterClassW(msnClass);

  if (hMSNClass<>0) and (hMSNWindow=0) then
  begin
    hMSNWindow:=CreateWindowExW(0,PWideChar(hMSNClass),nil,0,1,1,1,1,HWND_MESSAGE,0,hInstance,nil);
  end;
  ClearMSNInfo;
end;

procedure StopMSNHook;
begin
  if hMSNWindow<>0 then
  begin
    DestroyWindow(hMSNWindow);
    hMSNWindow:=0;

    ClearMSNInfo;

    UnRegisterClass(MSNClassName,hInstance);
  end;
end;

//finalization
//  StopMSNHook;
end.
