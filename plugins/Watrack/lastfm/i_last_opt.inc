{}
const
  optLogin   :pAnsiChar='lfm/login';
  optPassword:pAnsiChar='lfm/password';
  optTries   :pAnsiChar='lfm/tries';
  optScrobble:pAnsiChar='lfm/scrobble';
  optLanguage:pAnsiChar='lfm/language';

procedure SaveOpt;
begin
  DBWriteString(0,PluginShort,optPassword,lfm_password);
  DBWriteString(0,PluginShort,optLogin   ,lfm_login);
  DBWriteByte  (0,PluginShort,optTries   ,lfm_tries);
  DBWriteByte  (0,PluginShort,optScrobble,lfm_on and 1);
  DBWriteWord  (0,PluginShort,optLanguage,lfm_lang);
end;

procedure LoadOpt;
begin
  lfm_lang :=DBReadWord(0,PluginShort,optLanguage,0);
  lfm_tries:=DBReadByte(0,PluginShort,optTries   ,3);
  lfm_on   :=DBReadByte(0,PluginShort,optScrobble,0);
  mFreeMem(lfm_login   ); lfm_login   :=DBReadString(0,PluginShort,optLogin);
  mFreeMem(lfm_password); lfm_password:=DBReadString(0,PluginShort,optPassword);
  if (lfm_login=nil) or (lfm_password=nil) then
    CallService(MS_POPUP_SHOWMESSAGEW,
      wparam(TranslateW('Don''t forget to enter Login and Password to use Last.fm service')),
      SM_WARNING);
end;

procedure FreeOpt;
begin
  mFreeMem(lfm_login);
  mFreeMem(lfm_password);
end;
