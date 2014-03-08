{search station code}
(*
procedure SetAvatar(hContact:THANDLE);
var
  success:boolean;
  fname:pAnsiChar;
  url:pWideChar;
  buf,buf1,buf2:array [0..MAX_PATH-1] of AnsiChar;
  ext:array [0..15] of AnsiChar;
  section:pAnsiChar;
  pc:pAnsiChar;
  i:integer;
begin
  // get url
  url:=DBReadUnicode(hContact,PluginName,optStationURL);
  // translate to Ansi
  FastWideToAnsiBuf(url,buf);
  mFreeMem(url);

  // search in INI
  i:=GetFSize(storage);
  if i=0 then
    i:=32767;
  mGetMem(pc,i+1);
  pc^:=#0;
  GetPrivateProfileSectionNamesA(pc,i,storage);
  section:=pc;

  while section^<>#0 do
  begin
    GetPrivateProfileStringA(section,'URL','',buf1,SizeOf(buf1),storage);
    if StrCmp(@buf,@buf1)=0 then
      break;

    while section^<>#0 do inc(section);
    inc(section);
  end;
  mFreeMem(pc);

  if section^<>#0 then
  begin
    // get avatar link
    buf[0]:=#0;
    GetPrivateProfileStringA(section,'Avatar','',buf,SizeOf(buf),storage);

    if buf[0]<>#0 then
    begin
      // Here we trying to get Avatar chache directory
      // (create it if needs)
      // and copy (load) owr avatars there
      // in : buf = source avatar path
      // out: fname = destination (file name in cache)
      // rule for name is?..


      if StrPos(buf,'://')=nil then
      begin
        fname:=@buf;
        success:=FileExists(fname);
        // need to copy this file to avatar cache
      end
      else
      begin
        // download file
{
        GetTempPathA(MAX_PATH,pAnsiChar(@buf1));
        pc:=extract(pAnsiChar(@buf),true);
        StrCat(pAnsiChar(@buf1),pc);
        mFreeMem(pc);
}
        //mrAvt

        GetExt(pAnsiChar(@buf),pAnsiChar(@ext));
        GetTempPathA(MAX_PATH,pAnsiChar(@buf2));
        GetTempFileNameA(pAnsiChar(@buf2),'mrAvt',GetCurrentTime,pAnsiChar(@buf1));
        ChangeExt(pAnsiChar(@buf1),PAnsiChar(@ext));
        //
        fname:=@buf1;
        success:=GetFile(pAnsiChar(@buf),fname);
      end;

      if success then
        CallService(MS_AV_SETAVATAR,hContact,LPARAM(fname));
    end;
  end;
end;
*)
type
  TMySearchFilter = record
    lStation,
    lStationURL,
    lGenre:PAnsiChar;
    lBitrate:integer;
    lBitrateMode:integer;
  end;

const
  AckHandle = 427;

// Since mRadio is unicode version only now, translate all strings to Wide
// coz f*cking jinn will set unicode flag in any cases
procedure LoadOneStation(section:pointer;const filter:TMySearchFilter);
var
  bitrate:integer;
  l:bool;
  columns:array [0..3] of TCHAR;
  csr:TCUSTOMSEARCHRESULTS;
  pc:pAnsiChar;
  buf:array [0..127] of AnsiChar;
begin
  pc:=GetParamSectionStr(section,'URL');
  if pc<>nil then
  begin
    if (filter.lStationURL=nil) or (StrPos(CharLowerA(pc),filter.lStationURL)<>nil) then
    begin
      FillChar(csr,SizeOf(csr),0);
      csr.psr.cbSize:=SizeOf(csr.psr);
      csr.psr.Flags:=PSR_UNICODE;
      AnsiToWide(pc,csr.psr.firstname.w,MirandaCP);
      pc:=GetParamSectionStr(section,optBitrate,'0');
      bitrate:=StrToInt(pc);
      if (bitrate<>0) and (filter.lBitrate<>0) then
      begin
        if      filter.lBitrateMode<0 then l:=bitrate<=filter.lBitrate
        else if filter.lBitrateMode=0 then l:=bitrate =filter.lBitrate
        else{if filter.lBitrateMode>0}     l:=bitrate>=filter.lBitrate;
      end
      else
        l:=true;
      if l then
      begin
        AnsiToWide(pc,csr.psr.email.w,MirandaCP);

        StrCopy(buf,GetParamSectionStr(section,'Name',GetSectionName(section)),127);
        if (filter.lStation=nil) or (StrPos(CharLowerA(@buf),filter.lStation)<>nil) then
        begin
          AnsiToWide(@buf,csr.psr.nick.w,MirandaCP);

          StrCopy(buf,GetParamSectionStr(section,optGenre,'unknown'),127);
          if (filter.lGenre=nil) or (StrPos(CharLowerA(@buf),filter.lGenre)<>nil) then
            AnsiToWide(@buf,csr.psr.lastname.w,MirandaCP)
          else
            l:=false;
        end
        else
          l:=false;
        if l then
        begin
          columns[0].w:=csr.psr.nick.w;      // Station name
          columns[1].w:=csr.psr.firstname.w; // URL
          columns[2].w:=csr.psr.lastname.w;  // Genre
          columns[3].w:=csr.psr.email.w;     // Bitrate

          csr.nSize      :=SizeOf(csr);
          csr.nFieldCount:=4;
          csr.szFields   :=@columns;
          ProtoBroadcastAck(PluginName,0,ACKTYPE_SEARCH,ACKRESULT_SEARCHRESULT,AckHandle,lparam(@csr));
        end;
      end;
      // initial value - nil, so we don't worry
      mFreeMem(csr.psr.nick);
      mFreeMem(csr.psr.firstname);
      mFreeMem(csr.psr.lastname);
      mFreeMem(csr.psr.email);
    end;
  end;
end;

procedure ProcessSearch(var filter:TMySearchFilter;ini:PAnsiChar);
var
  pc:PAnsiChar;
  csr:TCUSTOMSEARCHRESULTS;
  columns:array [0..3] of TCHAR;
  lstorage,section,list:pointer;
begin
  columns[0].w:='Station Name';
  columns[1].w:='Station URL';
  columns[2].w:='Genre';
  columns[3].w:='Bitrate';

  csr.nSize      :=SizeOf(csr);
  csr.nFieldCount:=4;
  csr.szFields   :=@columns;
  csr.psr.cbSize :=0;
  ProtoBroadcastAck(PluginName,0,ACKTYPE_SEARCH,ACKRESULT_SEARCHRESULT,AckHandle,lparam(@csr));

  lstorage:=OpenStorage(ini);
  if lstorage<>nil then
  begin
    list:=GetSectionList(lstorage);
    pc:=list;

    while pc^<>#0 do
    begin
      section:=SearchSection(lstorage,pc);
      LoadOneStation(section,filter); //!!
      while pc^<>#0 do inc(pc);
      inc(pc);
    end;

    FreeSectionList(list);
    CloseStorage(lstorage);
  end;

  ProtoBroadcastAck(PluginName,0,ACKTYPE_SEARCH,ACKRESULT_SUCCESS,AckHandle,0);
  mFreeMem(filter.lStation);
  mFreeMem(filter.lStationURL);
  mFreeMem(filter.lGenre);
end;

procedure BasicSearch(name:PAnsiChar); cdecl;
var
  filter:TMySearchFilter;
  ini:array [0..MAX_PATH-1] of AnsiChar;
begin
  FillChar(filter,SizeOf(filter),0);
  StrCopy(ini,storage);

  StrDup(filter.lStation,name);
  if filter.lStation<>nil then
    CharLowerA(filter.lStation);

  ProcessSearch(filter,ini);
end;

procedure ExtSearch(wnd:HWND); cdecl;
var
  filter:TMySearchFilter;
  ltmp:longbool;
  ini1,ini:array [0..MAX_PATH-1] of AnsiChar;
begin
  FillChar(filter,SizeOf(filter),0);
  ini1[0]:=#0;
  GetDlgItemTextA(wnd,IDC_CUSTOMINI,@ini1,SizeOf(ini1));
  if ini1[0]=#0 then
    StrCopy(ini,storage)
  else
    ConvertFileName(ini1,ini);

  with filter do
  begin
//      CallService(MS_UTILS_PATHTOABSOLUTE,TWPARAM(@ini1),TLPARAM(@ini));
    lBitrate:=GetDlgItemInt(wnd,IDC_BITRATE,ltmp,false);
    if      IsDlgButtonChecked(wnd,IDC_LT)=BST_CHECKED then lBitrateMode:=-1
    else if IsDlgButtonChecked(wnd,IDC_EQ)=BST_CHECKED then lBitrateMode:=0
    else{if IsDlgButtonChecked(lParam,IDC_GT)=BST_CHECKED}  lBitrateMode:=1;

    lStation:=GetDlgText(wnd,IDC_STATION,true);
    if lStation<>nil then
      CharLowerA(lStation);

    lStationURL:=GetDlgText(wnd,IDC_STATIONURL,true);
    if lStationURL<>nil then
      CharLowerA(lStationURL);

    lGenre:=GetDlgText(wnd,IDC_GENRE,true);
    if lGenre<>nil then
      CharLowerA(lGenre);
  end;

  ProcessSearch(filter,ini);
end;

function Service_SearchBasic(wParam:WPARAM;lParam:LPARAM):int_ptr;cdecl;
begin
  if lParam=0 then
    result:=0
  else
  begin
    result:=AckHandle;
    {CloseHandle}(mir_forkthread(@BasicSearch,StrDup(PAnsiChar(lParam),PAnsiChar(lParam))));
  end;
end;

function Service_SearchByAdvanced(wParam:WPARAM;lParam:LPARAM):int_ptr;cdecl;
begin
  if lParam=0 then
    result:=0
  else
  begin
    result:=AckHandle;
    {CloseHandle}(mir_forkthread(@ExtSearch,pointer(lParam)));
  end;
end;

function Service_GetCaps(wParam:WPARAM;lParam:LPARAM):int_ptr;cdecl; forward;

function ExtSearchProc(Dialog:HWND;hMessage:uint;wParam:WPARAM;lParam:LPARAM):LRESULT; stdcall;
var
  buf:array [0..MAX_PATH-1] of AnsiChar;
begin
  result:=0;
  case hMessage of
    WM_INITDIALOG: begin
      CheckDlgButton(Dialog,IDC_EQ,BST_CHECKED);
      TranslateDialogDefault(Dialog);
    end;

    WM_COMMAND: if (wParam shr 16)=BN_CLICKED then
    begin
      if loword(wParam)=IDOK then
      begin
        SendMessage(GetParent(Dialog),WM_COMMAND,IDOK+(BN_CLICKED) shl 16,
            GetDlgItem(GetParent(Dialog),IDOK));
      end
      else if loword(wParam)=IDC_BN_INIPATH then
      begin
        if ShowDlg(@buf,storage,'*.ini'#0'*.ini'#0#0) then
          SetDlgItemTextA(Dialog,IDC_CUSTOMINI,@buf);
      end;
    end;
  end;
end;

function Service_ExtSearchUI(wParam:WPARAM;lParam:LPARAM):int_ptr;cdecl;
type
  PDLGTEMPLATEEX = ^TDLGTEMPLATEEX;
  TDLGTEMPLATEEX = packed record
    dlgVer:word;
    signature:word;
    helpID:dword;
    exStyle:dword;
    style:dword;
    cDlgItems:word;
    x:word;
    y:word;
    cx:word;
    cy:word;
{
    sz_Or_Ord menu;
    sz_Or_Ord windowClass;
    title:array [0..titleLen] of WideChar;
    pointsize:word;
    weight:word;
    italic:byte;
    charset:byte;
    typeface:array [0..stringLen] of WideChar;
}
  end;

var
  hr:HRSRC;
  pdte:PDLGTEMPLATEEX;
begin
  result:=0;
  if lParam<>0 then
  begin
    hr:=FindResource(hInstance,MAKEINTRESOURCE(IDD_SEARCH),RT_DIALOG);
    if hr<>0 then
    begin
      pdte:=PDLGTEMPLATEEX(LoadResource(hInstance,hr));
      if pdte<>nil then
      begin
        if (Service_GetCaps(PFLAGNUM_1,0) and PF1_EXTSEARCHUI)<>0 then
          pdte^.style:=(pdte^.style and not WS_CHILD) or WS_POPUP or WS_BORDER;
        result:=CreateDialogIndirect(hInstance,
            PDlgTemplate(pdte){$IFNDEF FPC}^{$ENDIF},lParam,@ExtSearchProc);
      end;
    end;
  end;
end;

function Service_AddToList(wParam:WPARAM;lParam:LPARAM):int_ptr;cdecl;
var
  hContact:TMCONTACT;
  p:PWideChar;
  lurl:pWideChar;
begin
  result:=0;
  if lParam<>0 then
  begin
    with PPROTOSEARCHRESULT(lParam)^ do
    begin
      if id.w<>nil then
        lurl:=id.w
      else
        lurl:=firstname.w;

      if lurl<>nil then
      begin
{
// find contact
        hContact:=db_find_first(PluginName);
        while hContact<>0 do
        begin
          p:=DBReadString(hContact,PluginName,optStationURL);
          l:=StrCmp(p,lurl)=0;
          mFreeMem(p);
          if l then
          begin
            DBDeleteSetting(hContact,strCList,'NotOnList');
            DBDeleteSetting(hContact,strCList,'Hidden');
            result:=hContact;
            exit;
          end;
          hContact:=db_find_next(hContact,PluginName);
        end;
// if not found
}
        hContact:=CallService(MS_DB_CONTACT_ADD,0,0);
        if hContact<>0 then
        begin
          CallService(MS_PROTO_ADDTOCONTACT,hContact,tlparam(PluginName));
          // URL
          DBWriteUnicode(hContact,PluginName,optStationURL,lurl);
          DBWriteUnicode(hContact,PluginName,optFirstName ,lurl);

          // Name
          if nick.w=nil then
            p:=lurl
          else
            p:=nick.w;
          DBWriteUnicode(hContact,strCList  ,optMyHandle,p);
          DBWriteUnicode(hContact,PluginName,optNick    ,p);

          // Bitrate
          if email.w<>nil then
          begin
            DBWriteWord   (hContact,PluginName,optAge    ,StrToInt(email.w));
            DBWriteUnicode(hContact,PluginName,optBitrate,email.w);
          end;

          // Genre
          if lastname.w<>nil then
          begin
            DBWriteUnicode(hContact,PluginName,optGenre   ,lastname.w);
            DBWriteUnicode(hContact,PluginName,optLastName,lastname.w);
          end;

          SetStatus(hContact,ID_STATUS_OFFLINE);

//          SetAvatar(hContact);

          CallService(MS_IGNORE_IGNORE,hContact,IGNOREEVENT_USERONLINE{IGNOREEVENT_ALL});
          result:=hContact;
        end;
      end;
    end;
  end;
end;
