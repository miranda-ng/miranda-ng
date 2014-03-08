{}
const
  hRadioHotkey:THANDLE=0;
const
  MS_RADIO_HOTKEY:PAnsiChar  = 'mRadio/Hotkey';

  HKN_PLAYPAUSE  :PAnsiChar = 'mRadio_PlayPause';
  HKN_STOP       :PAnsiChar = 'mRadio_Stop';
  HKN_MUTE       :PAnsiChar = 'mRadio_Mute';
  HKN_RECORD     :PAnsiChar = 'mRadio_Record';

  DefRadioKeyRecord = ((HOTKEYF_EXT or HOTKEYF_SHIFT) shl 8) or ORD('R');
  DefRadioKeyMute   = ((HOTKEYF_EXT or HOTKEYF_SHIFT) shl 8) or ORD('M');
  DefRadioKeyPlay   = ((HOTKEYF_EXT or HOTKEYF_SHIFT) shl 8) or ORD('P');
  DefRadioKeyStop   = ((HOTKEYF_EXT or HOTKEYF_SHIFT) shl 8) or ORD('S');

function mRadio_Hotkey(wParam:WPARAM;lParam:LPARAM):int_ptr; cdecl;
begin
  if lParam=MRC_MUTE then
    result:=Service_RadioMute(0,0)
  else
    result:=CallService(MS_RADIO_COMMAND,lParam,0);
end;

procedure RegisterHotKey;
var
  hkrec:THOTKEYDESC;
begin
  hRadioHotkey:=CreateServiceFunction(MS_RADIO_HOTKEY,@mRadio_Hotkey);

  FillChar(hkrec,SizeOf(hkrec),0);

  hkrec.cbSize          := SizeOf(hkrec);
  hkrec.pszSection.a    := PluginName;
  hkrec.pszService      := MS_RADIO_HOTKEY;

  hkrec.DefHotKey       := DefRadioKeyPlay;
  hkrec.pszName         := HKN_PLAYPAUSE;
  hkrec.pszDescription.a:= 'Play/Pause';
  hkrec.lParam          := MRC_PAUSE;
  CallService(MS_HOTKEY_REGISTER,0,lparam(@hkrec));

  hkrec.DefHotKey       := DefRadioKeyStop;
  hkrec.pszName         := HKN_STOP;
  hkrec.pszDescription.a:= 'Stop';
  hkrec.lParam          := MRC_STOP;
  CallService(MS_HOTKEY_REGISTER,0,lparam(@hkrec));

  hkrec.DefHotKey       := DefRadioKeyMute;
  hkrec.pszName         := HKN_MUTE;
  hkrec.pszDescription.a:= 'Mute';
  hkrec.lParam          := MRC_MUTE;
  CallService(MS_HOTKEY_REGISTER,0,lparam(@hkrec));

  hkrec.DefHotKey       := DefRadioKeyRecord;
  hkrec.pszName         := HKN_RECORD;
  hkrec.pszDescription.a:= 'Start/Stop Record';
  hkrec.lParam          := MRC_RECORD;
  CallService(MS_HOTKEY_REGISTER,0,lparam(@hkrec));
end;

procedure UnregisterHotKey;
begin
  CallService(MS_HOTKEY_UNREGISTER,0,lparam(HKN_PLAYPAUSE));
  CallService(MS_HOTKEY_UNREGISTER,0,lparam(HKN_STOP));
  CallService(MS_HOTKEY_UNREGISTER,0,lparam(HKN_MUTE));
  CallService(MS_HOTKEY_UNREGISTER,0,lparam(HKN_RECORD));

  DestroyServiceFunction(hRadioHotkey);
end;
