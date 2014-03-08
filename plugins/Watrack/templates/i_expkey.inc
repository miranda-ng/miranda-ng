{main hotkey code}
const
  HKN_EXPORT:PAnsiChar = 'WAT_Export';

function ExportProc(wParam:WPARAM;lParam:LPARAM):int_ptr; cdecl;
var
  p:pWideChar;
begin
  result:=0;
  if DisablePlugin<>dsPermanent then
  begin
    p:=pointer(CallService(MS_WAT_REPLACETEXT,0,tlparam(ExportText)));
    SendString(0,p);
    mFreeMem(p);
  end;
end;

procedure reginshotkey;
var
  hkrec:THOTKEYDESC;
begin
  FillChar(hkrec,SizeOf(hkrec),0);
  with hkrec do
  begin
    cbSize          :=SizeOf(hkrec);
    pszName         :=HKN_EXPORT;
    pszDescription.a:='WATrack data insert hotkey';
    pszSection.a    :=PluginName;
    pszService      :=MS_WAT_EXPORT;
    DefHotKey       :=((HOTKEYF_ALT or HOTKEYF_SHIFT) shl 8) or VK_F7;
//    lParam          :=0;
  end;
  CallService(MS_HOTKEY_REGISTER,0,lparam(@hkrec));
end;
