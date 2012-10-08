{}
const
  optLogin   :pAnsiChar='myshows/login';
  optPassword:pAnsiChar='myshows/password';
  optTries   :pAnsiChar='myshows/tries';
//  optTimeout :PAnsiChar='myshows/timeout';
  optScrobPos:pAnsiChar='myshows/scrobpos';
  optScrobble:pAnsiChar='myshows/scrobble';

procedure SaveOpt;
var
  tmppass:array [0..255] of AnsiChar;
begin
  if msh_password<>nil then
  begin
    StrCopy(tmppass,msh_password);
    CallService(MS_DB_CRYPT_ENCODESTRING,StrLen(tmppass)+1,LPARAM(@tmppass));
  end;
  DBWriteString(0,PluginShort,optPassword,tmppass);
  DBWriteString(0,PluginShort,optLogin   ,msh_login);
  DBWriteByte  (0,PluginShort,optTries   ,msh_tries);
  DBWriteByte  (0,PluginShort,optScrobPos,msh_scrobpos);
//  DBWriteWord  (0,PluginShort,optTries   ,msh_timeout);
  DBWriteByte  (0,PluginShort,optScrobble,msh_on and 1);
end;

procedure LoadOpt;
begin
//  msh_timeout :=DBReadWord(0,PluginShort,optTimeout ,0);
  msh_scrobpos:=DBReadByte(0,PluginShort,optScrobPos,30);
  msh_tries   :=DBReadByte(0,PluginShort,optTries   ,3);
  msh_on      :=DBReadByte(0,PluginShort,optScrobble,0);
  mFreeMem(msh_login   ); msh_login   :=DBReadString(0,PluginShort,optLogin);
  mFreeMem(msh_password); msh_password:=DBReadString(0,PluginShort,optPassword);
  if msh_password<>nil then
    CallService(MS_DB_CRYPT_DECODESTRING,StrLen(msh_password)+1,LPARAM(msh_password));
  if (msh_login=nil) or (msh_password=nil) then
    CallService(MS_POPUP_SHOWMESSAGEW,
      WPARAM(TranslateW('Don''t forget to enter Login and Password to use MyShows service')),
      SM_WARNING);
end;

procedure FreeOpt;
begin
  mFreeMem(msh_login);
  mFreeMem(msh_password);
end;
