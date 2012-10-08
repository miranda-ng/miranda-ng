{Hotkey and timer related functions}
unit hotkeys;

interface

uses windows;

type
  AWKHotKeyProc = function(hotkey:integer):integer;

function AddProc(aproc:AWKHotKeyProc;ahotkey:integer;global:bool=false):integer; overload;
function AddProc(ahotkey:integer;wnd:HWND;aproc:AWKHotKeyProc         ):integer; overload;
function AddProc(ahotkey:integer;wnd:HWND;msg:uint_ptr                ):integer; overload;
function DelProc(hotkey:integer         ):integer;  overload;
function DelProc(hotkey:integer;wnd:HWND):integer; overload;

procedure InitHotKeys;
procedure FreeHotKeys;

implementation

uses messages;

const
  HWND_MESSAGE = HWND(-3);
  
var
  CurThread:THANDLE;

type
  PKBDLLHOOKSTRUCT = ^TKBDLLHOOKSTRUCT;
  TKBDLLHOOKSTRUCT = record
    vkCode     :dword;
    scanCode   :dword;
    flags      :dword;
    time       :dword;
    dwExtraInfo:dword;
  end;

const
  WH_KEYBOARD_LL = 13;
  WM_MYMESSAGE = WM_USER +13;

// const from commctrl module;
const
  HOTKEYF_SHIFT   = $01;
  HOTKEYF_CONTROL = $02;
  HOTKEYF_ALT     = $04;
  HOTKEYF_EXT     = $08;

const
  hkAssigned = 1;
  hkGlobal   = 2;
  hkMessage  = 4;
const
  kbHook:THANDLE=0;
  hiddenwindow:HWND=0;
  modifiers:dword=0;
const
  PageStep = 10;
type
  PHKRec = ^THKRec;
  THKRec = record
    proc  :AWKHotKeyProc; // procedure
    flags :integer;       // options
    handle:THANDLE;       // thread or window?
    atom  :TATOM;         // hotkey id
    hotkey:integer;       // hotkey
  end;
  PHKRecs = ^THKRecs;
  THKRecs = array [0..15] of THKRec;

const
  NumRecs:integer=0;
  MaxRecs:integer=10;
  hkRecs:pHKRecs=nil;

//----- simpler version of 'common' function -----

const
  HexDigitChr: array [0..15] of AnsiChar = ('0','1','2','3','4','5','6','7',
                                        '8','9','A','B','C','D','E','F');

function IntToHex(dst:PAnsiChar;Value:cardinal):PAnsiChar;
var
  Digits:integer;
begin
  dst[8]:=#0;
  Digits:=8;
  repeat
    Dec(Digits);
    dst[Digits]:=HexDigitChr[Value and $F];
    Value:=Value shr 4;
  until Digits=0;
  result:=dst;
end;

//----- utils -----

function GetAtom(hotkey:dword):dword;
const
  HKPrefix = 'awk_';
var
  p:array [0..15] of AnsiChar;
begin
  lstrcpya(p,HKPrefix);
  IntToHex(p+Length(HKPrefix),hotkey);
  result:=GlobalAddAtomA(p);
end;

function HotKeyDlgToHook(w:cardinal):cardinal; register;
asm
  movzx ecx,al
  xor   al,al
  test  ah,HOTKEYF_ALT
  je    @L1
  or    al,MOD_ALT
@L1:
  test ah,HOTKEYF_CONTROL
  je   @L2
  or   al,MOD_CONTROL
@L2:
  test ah,HOTKEYF_SHIFT
  je   @L3
  or   al,MOD_SHIFT
@L3:
  test ah,HOTKEYF_EXT
  je   @L4
  or   al,MOD_WIN
@L4:
  mov ch,al
  mov eax,ecx
{
begin
  result:=w and $FF;
  if (w and (HOTKEYF_ALT     shl 8))<>0 then result:=result or (MOD_ALT     shl 8);
  if (w and (HOTKEYF_CONTROL shl 8))<>0 then result:=result or (MOD_CONTROL shl 8);
  if (w and (HOTKEYF_SHIFT   shl 8))<>0 then result:=result or (MOD_SHIFT   shl 8);
  if (w and (HOTKEYF_EXT     shl 8))<>0 then result:=result or (MOD_WIN     shl 8);
}
end;

function HotKeyHookToDlg(w:cardinal):cardinal; register;
asm
  movzx ecx,al
  xor   al,al
  test  ah,MOD_ALT
  je    @L1
  or    al,HOTKEYF_ALT
@L1:
  test ah,MOD_CONTROL
  je   @L2
  or   al,HOTKEYF_CONTROL
@L2:
  test ah,MOD_SHIFT
  je   @L3
  or   al,HOTKEYF_SHIFT
@L3:
  test ah,MOD_WIN
  je   @L4
  or   al,HOTKEYF_EXT
@L4:
  mov  ch,al
  mov  eax,ecx
{
begin
  result:=w and $FF;
  if (w and (MOD_ALT     shl 8))<>0 then result:=result or (HOTKEYF_ALT     shl 8);
  if (w and (MOD_CONTROL shl 8))<>0 then result:=result or (HOTKEYF_CONTROL shl 8);
  if (w and (MOD_SHIFT   shl 8))<>0 then result:=result or (HOTKEYF_SHIFT   shl 8);
  if (w and (MOD_WIN     shl 8))<>0 then result:=result or (HOTKEYF_EXT     shl 8);
}
end;

//----- Hook -----

function FindHotkey(keycode:integer;local:boolean):pointer;
var
  i:integer;
  p:pHKRec;
begin
  i:=NumRecs;
  p:=pointer(HKRecs);
  while i>0 do
  begin
    dec(i);
    with p^ do
    begin
      if (flags and hkAssigned)<>0 then
      begin
        if (local xor ((flags and hkGlobal)<>0)) then
        begin
          if hotkey=keycode then
          begin
            if handle<>0 then
            begin
              if GetFocus=handle then
              begin
                if (flags and hkMessage)<>0 then
                begin
                  PostMessage(handle,wparam(@proc),keycode,0);
                  result:=pointer(-1);
                end
                else
                  result:=@proc;
                exit;
              end;
            end
            else
            begin
              result:=@proc;
              exit;
            end;
          end;
        end;
      end;
    end;
    inc(p);
  end;
  result:=nil;
end;

function wmKeyboard_hook(code:integer;wParam:WPARAM;lParam:LPARAM):longint; stdcall;
var
  key:dword;
  proc:pointer;
begin
  if (code=HC_ACTION) and
     (lParam>0) and (LoWord(lParam)=1) then
  begin
    key:=0;
    if (GetKeyState(VK_SHIFT  ) and $8000)<>0 then key:=key or (MOD_SHIFT   shl 8);
    if (GetKeyState(VK_CONTROL) and $8000)<>0 then key:=key or (MOD_CONTROL shl 8);
    if (GetKeyState(VK_MENU   ) and $8000)<>0 then key:=key or (MOD_ALT     shl 8);
    if (GetKeyState(VK_LWIN   ) and $8000)<>0 then key:=key or (MOD_WIN     shl 8);
    if (GetKeyState(VK_RWIN   ) and $8000)<>0 then key:=key or (MOD_WIN     shl 8);
//    if (GetKeyState(VK_APPS) and $8000)<>0 then
//    if (GetKeyState(VK_SLEEP) and $8000)<>0 then
    key:=key or (cardinal(wParam) and $FF);
    proc:=FindHotkey(key,true);
    if proc<>nil then
    begin
      if proc<>pointer(-1) then
        PostMessageA(hiddenwindow,WM_MYMESSAGE,key,windows.lparam(proc));
      result:=1;
      exit;
    end;
  end;
  result:=CallNextHookEx(KbHook,code,wParam,lParam);
end;

function wmKeyboardLL_hook(code:integer;wParam:WPARAM;lParam:LPARAM):integer; stdcall;
const
  lastkey:dword=0;
var
  mask:dword;
  key:dword;
  proc:pointer;
begin
  if code=HC_ACTION then
  begin
    case PKBDLLHOOKSTRUCT(lParam)^.vkCode of
      VK_MENU,
      VK_LMENU,
      VK_RMENU:    mask:=MOD_ALT     shl 8;
      VK_LWIN,
      VK_RWIN:     mask:=MOD_WIN     shl 8;
      VK_SHIFT,
      VK_LSHIFT,
      VK_RSHIFT:   mask:=MOD_SHIFT   shl 8;
      VK_CONTROL,
      VK_LCONTROL,
      VK_RCONTROL: mask:=MOD_CONTROL shl 8;
    else
      if (PKBDLLHOOKSTRUCT(lParam)^.flags and 128)=0 then
      begin
        // local only
// maybe process will better choice?
        if //(lastkey=0) and
           (CurThread=GetWindowThreadProcessId(GetForegroundWindow,nil)) then
        begin
          key:=PKBDLLHOOKSTRUCT(lParam)^.vkCode or modifiers;
          proc:=FindHotkey(key,true);
          if proc<>nil then
          begin
            lastkey:=PKBDLLHOOKSTRUCT(lParam)^.vkCode;
            if proc<>pointer(-1) then
              PostMessageA(hiddenwindow,WM_MYMESSAGE,key,windows.lparam(proc));
            result:=1;
            exit;
          end;
        end;
      end
      else if (lastkey<>0) and (lastkey=PKBDLLHOOKSTRUCT(lParam)^.vkCode) then
      begin
        lastkey:=0;
        result :=1;
        exit;
      end;
      mask:=0;
    end;
    if mask<>0 then
    begin
      if (PKBDLLHOOKSTRUCT(lParam)^.flags and 128)=0 then
        modifiers:=modifiers or mask
      else
        modifiers:=modifiers and not mask;
    end
  end;
  result:=CallNextHookEx(KbHook,code,wParam,lParam);
end;

function HiddenWindProc(wnd:HWnd;msg:UINT;wParam:WPARAM;lParam:LPARAM):lresult; stdcall;
var
  key:dword;
begin
  if Msg=WM_HOTKEY then
  begin
    key:=(lParam shr 16)+((lParam and $FF) shl 8);
    result:=lresult(FindHotKey(key,false));
    if result<>0 then
    begin
      result:=AWKHotKeyProc(result)(HotkeyHookToDlg(key));
      exit;
    end;
  end
  else if Msg=WM_MYMESSAGE then
  begin
    result:=AWKHotKeyProc(lParam)(HotkeyHookToDlg(wParam));
    exit;
  end;
  result:=DefWindowProcA(wnd,msg,wparam,lparam);
end;

procedure DestroyHiddenWindow;
begin
  if hiddenwindow<>0 then
  begin
    DestroyWindow(hiddenwindow);
    hiddenwindow:=0;
  end;
end;

procedure CreateHiddenWindow;
var
  wnd:HWND;
begin
  if hiddenwindow=0 then
  begin
    wnd:=CreateWindowExA(0,'STATIC',nil,0,
       1,1,1,1,HWND_MESSAGE,0,hInstance,nil);
    if wnd<>0 then
    begin
      SetWindowLongPtrA(wnd,GWL_WNDPROC,LONG_PTR(@HiddenWindProc));
      hiddenwindow:=wnd;
    end
  end
end;
//----- interface -----

function CheckTable(ahotkey:integer;global:bool):integer;
var
  tmp:pHKRecs;
  i:integer;
  p:pHKRec;
begin
  if HKRecs=nil then
  begin
    MaxRecs:=PageStep;
    GetMem  (HKRecs ,MaxRecs*SizeOf(THKRec));
    FillChar(HKRecs^,MaxRecs*SizeOf(THKRec),0);
    NumRecs:=0;
  end;
  // search existing
  i:=0;
  p:=pointer(HKRecs);
  while i<NumRecs do
  begin
    if (p^.flags and hkAssigned)<>0 then
    begin
      if (p^.hotkey=ahotkey) and
         (((p^.flags and hkGlobal)<>0) xor not global) then
        break;
    end;
    inc(p);
    inc(i);
  end;
  //search empty
  if i=NumRecs then
  begin
    i:=0;
    p:=pointer(HKRecs);
    while i<NumRecs do
    begin
      if (p^.flags and hkAssigned)=0 then
        break;
      inc(p);
      inc(i);
    end;
  end;
  if i=NumRecs then // allocate if not found
  begin
    if NumRecs=MaxRecs then
    begin
      inc(MaxRecs,PageStep);
      GetMem  (tmp ,MaxRecs*SizeOf(THKRec));
      FillChar(tmp^,MaxRecs*SizeOf(THKRec),0);
      move(HKRecs^,tmp^,NumRecs*SizeOf(THKRec));
      FreeMem(HKRecs);
      HKRecs:=tmp;
    end;
    inc(NumRecs);
  end;
  if global then
    HKRecs^[i].flags:=hkAssigned or hkGlobal
  else
    HKRecs^[i].flags:=hkAssigned;
  HKRecs^[i].hotkey:=HotKeyDlgToHook(ahotkey);
  result:=i;
end;

function AddProc(aproc:AWKHotKeyProc;ahotkey:integer;global:bool=false):integer;
begin
  result:=1;
  if @aproc=nil then exit;

  with HKRecs^[CheckTable(ahotkey,global)] do
  begin
    proc  :=aproc;
    handle:=0;
    if global then
    begin
      atom:=GetAtom(hotkey);
      if not RegisterHotKey(hiddenwindow,atom,((hotkey and $FF00) shr 8),(hotkey and $FF)) then
        result:=0;
    end;
  end;
end;

// search needed
function AddProcWin(ahotkey:integer;wnd:HWND):integer;
begin
  result:=CheckTable(ahotkey,false);
  with HKRecs^[result] do
  begin
    handle:=wnd;
  end;
end;

function AddProc(ahotkey:integer;wnd:HWND;aproc:AWKHotKeyProc):integer;
begin
  if @aproc=nil then
  begin
    result:=0;
    exit;
  end;

  result:=AddProcWin(ahotkey,wnd);
  if result<0 then
    result:=0
  else
  begin
    HKRecs^[result].proc:=@aproc;
  end;
end;

function AddProc(ahotkey:integer;wnd:HWND;msg:uint_ptr):integer;
begin
  result:=AddProcWin(ahotkey,wnd);
  if result<0 then
    result:=0
  else
  begin
    HKRecs^[result].flags:=HKRecs^[result].flags or hkMessage;
    HKRecs^[result].proc:=pointer(msg);
  end;
end;

function DelProc(hotkey:integer):integer;
var
  i:integer;
  p:pHKRec;
begin
  hotkey:=HotKeyDlgToHook(hotkey); //!!
  p:=pointer(HKRecs);
  i:=NumRecs;
  while i>0 do
  begin
    dec(i);
    if ((p^.flags and hkAssigned)<>0) and (p^.handle=0) then
      if p^.hotkey=hotkey then
      begin
        if (p^.flags and hkGlobal)<>0 then
        begin
          UnregisterHotKey(hiddenwindow,p^.atom);
          GlobalDeleteAtom(p^.atom);
        end;
        p^.flags:=p^.flags and not hkAssigned;
        result:=i;
        exit;
      end;
    inc(p);
  end;
  result:=0;
end;

function DelProc(hotkey:integer;wnd:HWND):integer;
var
  i:integer;
  p:pHKRec;
begin
  hotkey:=HotKeyDlgToHook(hotkey); //!!
  p:=pointer(HKRecs);
  i:=NumRecs;
  while i>0 do
  begin
    dec(i);
    if (p^.flags and hkAssigned)<>0 then
      if (p^.handle=wnd) {and ((p^.flags and hkGlobal)=0)} then
      begin
        if (hotkey=0) or (hotkey=p^.hotkey) then
        begin
          p^.flags:=p^.flags and not hkAssigned;
          result:=i;
          exit;
        end;
      end;
    inc(p);
  end;
  result:=0;
end;

procedure InitHotKeys;
begin
  MaxRecs:=10;
  GetMem(HKRecs,SizeOf(THKRec)*MaxRecs);
  FillChar(HKRecs^,SizeOf(THKRec)*MaxRecs,0);
  NumRecs:=0;
  CreateHiddenWindow;
  kbhook:=SetWindowsHookExA(WH_KEYBOARD_LL,@wmKeyboardLL_hook,hInstance,0);

  if KbHook=0 then
    KbHook:=SetWindowsHookExA(WH_KEYBOARD,@wmKeyboard_hook,0,GetCurrentThreadId);
end;

procedure FreeHotKeys;
var
  i:integer;
  p:pHKRec;
begin
  i:=NumRecs;
  p:=pointer(HKRecs);
  while i>0 do
  begin
    dec(i);
    if (p^.flags and (hkAssigned or hkGlobal))=(hkAssigned or hkGlobal) then
    begin
      UnregisterHotKey(hiddenwindow,p^.atom);
      GlobalDeleteAtom(p^.atom);
    end;
    inc(p);
  end;
  DestroyHiddenWindow;
  if kbhook<>0 then
    UnhookWindowsHookEx(kbhook);
  FreeMem(HKRecs);
  HKRecs:=nil;
  MaxRecs:=0;
  NumRecs:=0;
end;

initialization
  CurThread:=GetCurrentThreadId();
end.