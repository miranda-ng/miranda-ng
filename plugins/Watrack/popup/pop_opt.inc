{Popup options saving-loading}

const
  defPopupTitle    = 'Now listening to';
  defPopupText     = '%artist% - %title%';
  defAltPopupTitle = 'Now ?ifgreater(%width%,0,watching,listening to)';
  defAltPopupText  = '%artist% - %title%'#13#10'?iflonger(%album%,0, (from "%album%"),)';
const
  opt_ModStatus  :PAnsiChar = 'module/popups';

  opt_PopupFile   :PAnsiChar = 'popup/file';
  opt_PopupAction :PAnsiChar = 'popup/action';
  opt_PopupFore   :PAnsiChar = 'popup/fore';
  opt_PopupBack   :PAnsiChar = 'popup/back';
  opt_PopupPause  :PAnsiChar = 'popup/time';
  opt_PopupDelay  :PAnsiChar = 'popup/delay';
  opt_PopupColor  :PAnsiChar = 'popup/color';
  opt_ByRequest   :PAnsiChar = 'popup/byrequest';
  opt_PopTitle    :PAnsiChar = 'popup/poptitle';
  opt_PopText     :PAnsiChar = 'popup/poptext';
  opt_PopupButtons:PAnsiChar = 'popup/usebuttons';

  spref = 'strings/';

function GetModStatus:integer;
begin
  result:=DBReadByte(0,PluginShort,opt_ModStatus,1);
end;

procedure SetModStatus(stat:integer);
begin
  DBWriteByte(0,PluginShort,opt_ModStatus,stat);
end;

procedure loadpopup;
var
  def1,def2:pWideChar;
begin
  PopupButtons:=DBReadByte (0,PluginShort,opt_PopupButtons,BST_CHECKED);
  PopupFile   :=DBReadByte (0,PluginShort,opt_PopupFile   ,BST_CHECKED);
  PopupPause  :=DBReadByte (0,PluginShort,opt_PopupPause  ,0);
  PopupDelay  :=DBReadByte (0,PluginShort,opt_PopupDelay  ,0);
  PopupAction :=DBReadWord (0,PluginShort,opt_PopupAction ,0);
  PopupColor  :=DBReadByte (0,PluginShort,opt_PopupColor  ,0);
  PopupFore   :=DBReadDWord(0,PluginShort,opt_PopupFore   ,GetSysColor(COLOR_BTNTEXT));
  PopupBack   :=DBReadDWord(0,PluginShort,opt_PopupBack   ,GetSysColor(COLOR_BTNFACE));
  PopRequest  :=DBReadByte (0,PluginShort,opt_ByRequest   ,BST_UNCHECKED);
  if isVarsInstalled then
  begin
    def1:=defAltPopupTitle;
    def2:=defAltPopupText;
  end
  else
  begin
    def1:=defPopupTitle;
    def2:=defPopupText;
  end;
  PopTitle:=DBReadUnicode(0,PluginShort,opt_PopTitle,def1);
  PopText :=DBReadUnicode(0,PluginShort,opt_PopText ,def2);
end;

procedure savepopup;
begin
  DBWriteByte   (0,PluginShort,opt_PopupButtons,PopupButtons);
  DBWriteByte   (0,PluginShort,opt_PopupFile   ,PopupFile);
  DBWriteByte   (0,PluginShort,opt_PopupPause  ,PopupPause);
  DBWriteByte   (0,PluginShort,opt_PopupDelay  ,PopupDelay);
  DBWriteWord   (0,PluginShort,opt_PopupAction ,PopupAction);
  DBWriteByte   (0,PluginShort,opt_PopupColor  ,PopupColor);
  DBWriteDWord  (0,PluginShort,opt_PopupFore   ,PopupFore);
  DBWriteDWord  (0,PluginShort,opt_PopupBack   ,PopupBack);
  DBWriteByte   (0,PluginShort,opt_ByRequest   ,PopRequest);
  DBWriteUnicode(0,PluginShort,opt_PopTitle,PopTitle);
  DBWriteUnicode(0,PluginShort,opt_PopText ,PopText);
end;

procedure freepopup;
begin
  mFreeMem(PopTitle);
  mFreeMem(PopText);
end;
