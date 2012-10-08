{Popup options saving-loading}

const
  defPopupTitle    = 'Now listening to';
  defPopupText     = '%artist% - %title%';
  defAltPopupTitle = 'Now ?ifgreater(%width%,0,watching,listening to)';
  defAltPopupText  = '%artist% - %title%'#13#10'?iflonger(%album%,0, (from "%album%"),)';
const
  opt_ModStatus  :PAnsiChar = 'module/popups';

  opt_PopUpFile   :PAnsiChar = 'popup/file';
  opt_PopUpAction :PAnsiChar = 'popup/action';
  opt_PopUpFore   :PAnsiChar = 'popup/fore';
  opt_PopUpBack   :PAnsiChar = 'popup/back';
  opt_PopUpPause  :PAnsiChar = 'popup/time';
  opt_PopUpDelay  :PAnsiChar = 'popup/delay';
  opt_PopUpColor  :PAnsiChar = 'popup/color';
  opt_ByRequest   :PAnsiChar = 'popup/byrequest';
  opt_PopTitle    :PAnsiChar = 'popup/poptitle';
  opt_PopText     :PAnsiChar = 'popup/poptext';
  opt_PopUpButtons:PAnsiChar = 'popup/usebuttons';

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
  PopUpButtons:=DBReadByte (0,PluginShort,opt_PopUpButtons,BST_CHECKED);
  PopUpFile   :=DBReadByte (0,PluginShort,opt_PopUpFile   ,BST_CHECKED);
  PopUpPause  :=DBReadByte (0,PluginShort,opt_PopUpPause  ,0);
  PopUpDelay  :=DBReadByte (0,PluginShort,opt_PopUpDelay  ,0);
  PopUpAction :=DBReadWord (0,PluginShort,opt_PopUpAction ,0);
  PopUpColor  :=DBReadByte (0,PluginShort,opt_PopUpColor  ,0);
  PopUpFore   :=DBReadDWord(0,PluginShort,opt_PopUpFore   ,GetSysColor(COLOR_BTNTEXT));
  PopUpBack   :=DBReadDWord(0,PluginShort,opt_PopUpBack   ,GetSysColor(COLOR_BTNFACE));
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
  DBWriteByte   (0,PluginShort,opt_PopUpButtons,PopUpButtons);
  DBWriteByte   (0,PluginShort,opt_PopUpFile   ,PopUpFile);
  DBWriteByte   (0,PluginShort,opt_PopUpPause  ,PopUpPause);
  DBWriteByte   (0,PluginShort,opt_PopUpDelay  ,PopUpDelay);
  DBWriteWord   (0,PluginShort,opt_PopUpAction ,PopUpAction);
  DBWriteByte   (0,PluginShort,opt_PopUpColor  ,PopUpColor);
  DBWriteDWord  (0,PluginShort,opt_PopUpFore   ,PopUpFore);
  DBWriteDWord  (0,PluginShort,opt_PopUpBack   ,PopUpBack);
  DBWriteByte   (0,PluginShort,opt_ByRequest   ,PopRequest);
  DBWriteUnicode(0,PluginShort,opt_PopTitle,PopTitle);
  DBWriteUnicode(0,PluginShort,opt_PopText ,PopText);
end;

procedure freepopup;
begin
  mFreeMem(PopTitle);
  mFreeMem(PopText);
end;
