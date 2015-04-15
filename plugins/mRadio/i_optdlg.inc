{Options dialog}
procedure SetButtonIcons(Dialog:HWND);
var
  ti:TTOOLINFOW;
  hwndTooltip:HWND;
begin
  hwndTooltip:=CreateWindowW(TOOLTIPS_CLASS,nil,TTS_ALWAYSTIP,
      integer(CW_USEDEFAULT),integer(CW_USEDEFAULT),
      integer(CW_USEDEFAULT),integer(CW_USEDEFAULT),
      Dialog,0,hInstance,nil);

  FillChar(ti,SizeOf(ti),0);
  ti.cbSize  :=sizeof(TOOLINFO);
  ti.uFlags  :=TTF_IDISHWND or TTF_SUBCLASS;
  ti.hwnd    :=Dialog;
  ti.hinst   :=hInstance;

  ti.uId     :=GetDlgItem(Dialog,IDC_EQ_ADD);
  ti.lpszText:=TranslateW('Add');

  SetButtonIcon(ti.uId,IcoBtnAdd);

  SendMessageW(hwndTooltip,TTM_ADDTOOLW,0,lparam(@ti));

  ti.uId     :=GetDlgItem(Dialog,IDC_EQ_DEL);
  ti.lpszText:=TranslateW('Delete');
  SetButtonIcon(ti.uId,IcoBtnDel);
  SendMessageW(hwndTooltip,TTM_ADDTOOLW,0,lparam(@ti));
end;

function DlgProcOpt(Dialog:HWND;hMessage:uint;wParam:WPARAM;lParam:LPARAM):LRESULT; stdcall;
const
  DlgInit:integer=1;
var
  buf:array [0..MAX_PATH-1] of WideChar;
  psr:TPROTOSEARCHRESULT;
  dst:pWideChar;
  i:dword;
  vhi:TVARHELPINFO;
  wnd:HWND;
  p:pWideChar;
begin
  result:=0;
  case hMessage of

    WM_INITDIALOG: begin
      DlgInit:=1;
      TranslateDialogDefault(Dialog);

      wnd:=GetDlgItem(Dialog,IDC_HLP_VARS);
      if isVarsInstalled then
      begin
        SendMessage(wnd,BM_SETIMAGE,IMAGE_ICON,
            CallService(MS_VARS_GETSKINITEM,0,VSI_HELPICON));
      end
      else
        ShowWindow(wnd,SW_HIDE);

      if recpath<>nil then
        p:=recpath
      else
      begin
        buf[0]:=#0;
        p:=@buf;
      end;
      SetDlgItemTextW(Dialog,IDC_ED_RECPATH,p);

      CheckDlgButton(Dialog,IDC_LOOP     ,doLoop);
      CheckDlgButton(Dialog,IDC_SHUFFLE  ,doShuffle);
      CheckDlgButton(Dialog,IDC_CONTREC  ,doContRec);
      CheckDlgButton(Dialog,IDC_PLAYFIRST,PlayFirst);
      CheckDlgButton(Dialog,IDC_CONNECT  ,AuConnect);
      CheckDlgButton(Dialog,IDC_AUTOMUTE ,AuMute);
      CheckDlgButton(Dialog,IDC_OFFLINE  ,AsOffline);

      SetDlgItemTextW(Dialog,IDC_STATUS,StatusTmpl);

      DlgInit:=0;
    end;

    WM_COMMAND: begin
      case wParam shr 16 of
        EN_CHANGE: begin
          if DlgInit=0 then
            case loword(wParam) of
              IDC_STATION,IDC_STATIONURL,IDC_GENRE,IDC_BITRATE: ;
            else
              SendMessage(GetParent(Dialog),PSM_CHANGED,0,0);
            end;
        end;

        BN_CLICKED: begin
          case loword(wParam) of

            IDC_IMPORT: begin
              ImportAll(0,0);
            end;

            IDC_EXPORT: begin
              ExportAll(0,0);
            end;

            IDC_HLP_VARS: begin
              FillChar(vhi,SizeOf(vhi),0);
              with vhi do
              begin
                cbSize       :=SizeOf(vhi);
                flags        :=VHF_FULLDLG or VHF_SETLASTSUBJECT;
                hwndCtrl     :=GetDlgItem(Dialog,IDC_STATUS);
                szSubjectDesc:='test your variables';
              end;
              CallService(MS_VARS_SHOWHELPEX,Dialog,tlparam(@vhi));
            end;

            IDC_BN_URLPATH: begin

              dst:=ConstructFilter;
              if ShowDlgW(@buf,nil,dst) then
                SetDlgItemTextW(Dialog,IDC_STATIONURL,@buf);
              mFreeMem(dst);
            end;

            IDC_BN_RECPATH: begin
              dst:=nil;
              if SelectDirectory(pWideChar(nil),dst,Dialog) then
              begin
                PathToRelativeW(dst,buf);
                SetDlgItemTextW(Dialog,IDC_ED_RECPATH,pWideChar(@buf));
                mFreeMem(dst);
              end;
            end;

            IDC_LOOP,IDC_SHUFFLE,IDC_CONTREC,IDC_CONNECT,IDC_OFFLINE,IDC_AUTOMUTE:
              SendMessage(GetParent(Dialog),PSM_CHANGED,0,0);

            IDC_ADD_LIST,IDC_ADD_INI: begin
              if GetWindowTextLength(GetDlgItem(Dialog,IDC_STATIONURL))>0 then
              begin
                FillChar(psr,SizeOf(psr),0);
                with psr do
                begin
                  if loword(wParam)=IDC_ADD_LIST then
                  begin
                    GetDlgItemTextW(Dialog,IDC_STATIONURL,@buf,MAX_PATH);
                    StrDupW(firstname.w,@buf);
                    buf[0]:=#0;

                    GetDlgItemTextW(Dialog,IDC_STATION,@buf,MAX_PATH);
                    if buf[0]<>#0 then
                      StrDupW(nick.w,@buf)
                    else
                      StrDupW(nick.w,firstname.w);
                    buf[0]:=#0;

                    GetDlgItemTextW(Dialog,IDC_GENRE,@buf,MAX_PATH);
                    StrDupW(lastname.w,@buf);
                    buf[0]:=#0;

                    GetDlgItemTextW(Dialog,IDC_BITRATE,@buf,MAX_PATH);
                    StrDupW(email.w,@buf);

                    i:=Service_AddToList(0,tlparam(@psr));
{
                    GetDlgItemTextW(Dialog,IDC_STATION,@buf,SizeOf(buf));
                    DBWriteUnicode(i,strCList,optMyHandle,@buf);

                    GetDlgItemTextW(Dialog,IDC_STATIONURL,@buf,SizeOf(buf));
                    DBWriteUnicode(i,PluginName,optStationURL,@buf);
}
                    // "changing" station group
                    dst:=GetNewGroupName(Dialog);
                    if dst<>nil then
                      DBWriteUnicode(i,strCList,optGroup,dst)
                    else
                      DBDeleteSetting(i,strCList,optGroup);
                      
                  end
                  else if loword(wParam)=IDC_ADD_INI then
                  begin
                    GetDlgItemTextA(Dialog,IDC_STATIONURL,PAnsiChar(@buf),SizeOf(buf));
                    StrDup(firstname.a,PAnsiChar(@buf));
                    PAnsiChar(@buf)^:=#0;

                    GetDlgItemTextA(Dialog,IDC_STATION,PAnsiChar(@buf),SizeOf(buf));
                    if PAnsiChar(@buf)^<>#0 then
                      StrDup(nick.a,@buf)
                    else
                      StrDup(nick.a,firstname.a);
                    PAnsiChar(@buf)^:=#0;

                    GetDlgItemTextA(Dialog,IDC_GENRE,PAnsiChar(@buf),SizeOf(buf));
                    StrDup(lastname.a,@buf);
                    PAnsiChar(@buf)^:=#0;

                    GetDlgItemTextA(Dialog,IDC_BITRATE,PAnsiChar(@buf),SizeOf(buf));
                    StrDup(email.a,@buf);

                    if WritePrivateProfileStringA(firstname.a,'URL',firstname.a,storage) then
                    begin
                      WritePrivateProfileStringA(firstname.a,'Name'    ,nick.a     ,storage);
                      WritePrivateProfileStringA(firstname.a,optGenre  ,lastname.a ,storage);
                      WritePrivateProfileStringA(firstname.a,optBitrate,email.a    ,storage);
                    end
                    else
                    begin
                      WritePrivateProfileStringA(firstname.a,'URL'     ,firstname.a,storagep);
                      WritePrivateProfileStringA(firstname.a,'Name'    ,nick.a     ,storagep);
                      WritePrivateProfileStringA(firstname.a,optGenre  ,lastname.a ,storagep);
                      WritePrivateProfileStringA(firstname.a,optBitrate,email.a    ,storagep);
                    end;

                  end;
                  mFreeMem(nick);
                  mFreeMem(firstname);
                  mFreeMem(lastname);
                  mFreeMem(email);
                end;
              end;
            end;

          end;
        end;

      end;
    end;

    WM_NOTIFY: begin
      if integer(PNMHdr(lParam)^.code)=PSN_APPLY then
      begin

        doShuffle:=IsDlgButtonChecked(Dialog,IDC_SHUFFLE);
        doContRec:=IsDlgButtonChecked(Dialog,IDC_CONTREC);
        PlayFirst:=IsDlgButtonChecked(Dialog,IDC_PLAYFIRST);
        AuConnect:=IsDlgButtonChecked(Dialog,IDC_CONNECT);
        AuMute   :=IsDlgButtonChecked(Dialog,IDC_AUTOMUTE);
        AsOffline:=IsDlgButtonChecked(Dialog,IDC_OFFLINE);

        mFreeMem(StatusTmpl);
        StatusTmpl:=GetDlgText(Dialog,IDC_STATUS);

        doLoop:=IsDlgButtonChecked(Dialog,IDC_LOOP);
        if chan<>0 then
        begin
          BASS_ChannelFlags(chan,ord(doLoop<>BST_UNCHECKED),BASS_SAMPLE_LOOP);
        end;

        mFreeMem(recpath);
        buf[0]:=#0;
        GetDlgItemTextW(Dialog,IDC_ED_RECPATH,@buf,SizeOf(buf) div SizeOf(WideChar));
        if buf[0]<>#0 then
        begin
          mGetMem(recpath,MAX_PATH*SizeOf(WideChar));
          recpath^:=#0;
          PathToRelativeW(buf,recpath);
        end
        else
          recpath:=GetDefaultRecPath;

        SaveCommonSettings;
      end;
    end;

//  else
//    result:=DefWindowProc(Dialog,hMessage,wParam,lParam);
  end;
end;

//----- Tech tab -----

procedure FillDeviceCombo(wnd:HWND;choose:pAnsiChar);
var
  i,select,def:integer;
  info:BASS_DEVICEINFO;
  buf: array [0..255] of AnsiChar;
begin
  def:=0;
  SendMessage(wnd,CB_RESETCONTENT,0,0);
  if BassStatus<>rbs_null then
  begin
    i:=0;
    select:=-1;
    repeat
      if not BASS_GetDeviceInfo(i+1,info) then
        break;
      if (info.flags and BASS_DEVICE_ENABLED)<>0 then
      begin
        if (info.flags and BASS_DEVICE_INIT)<>0 then
        begin
          select:=i;
          buf[0]:='>';
        end
        else if (info.flags and BASS_DEVICE_DEFAULT)<>0 then
        begin
          def:=i;
          buf[0]:='*';
        end
        else buf[0]:=' ';
        if select<0 then
        begin
          if StrCmp(info.name,choose)=0 then
            select:=i;
        end;
        StrCopy(@buf[1],info.name);
        SendMessageA(wnd,CB_ADDSTRING,0,lparam(@buf));
      end;
      inc(i);
    until false;
    if select>=0 then
      def:=select;
  end;
  SendMessage(wnd,CB_SETCURSEL,def,0);
end;

function DlgProcOptTech(Dialog:HWND;hMessage:uint;wParam:WPARAM;lParam:LPARAM):LRESULT; stdcall;
const
  DlgInit:integer=1;
var
  hEAXCombo:THANDLE;
  ltmp:longbool;
  wnd:HWND;
  buf:array [0..MAX_PATH-1] of WideChar;
  dst,old:pWideChar;
  device:pAnsiChar;
  i,level:integer;
begin
  result:=0;
  case hMessage of
    WM_DESTROY: begin
      hVolCtrl:=0;

      for i:=0 to 9 do
      begin
        eq[i].wnd:=0;
      end;
    end;

    WM_INITDIALOG: begin
      DlgInit:=1;
      TranslateDialogDefault(Dialog);

      SetButtonIcons(Dialog);
      hVolCtrl:=GetDlgItem(Dialog,IDC_VOLUME);
      SendMessage(hVolCtrl,TBM_SETRANGE,0,(100 shl 16)+0);
      SendMessage(hVolCtrl,TBM_SETPOS,1,ABS(gVolume));

      SetDlgItemInt(Dialog,IDC_BUFFER ,sBuffer ,false);
      SetDlgItemInt(Dialog,IDC_PREBUF ,sPreBuf ,false);
      SetDlgItemInt(Dialog,IDC_TIMEOUT,sTimeout,false);
      SetDlgItemInt(Dialog,IDC_TRIES  ,NumTries,false);

      CheckDlgButton(Dialog,IDC_MONO,ForcedMono);

      // Fill EAX combo
      hEAXCombo:=GetDlgItem(Dialog,IDC_EAXTYPE);
      for i:=0 to EAX_ENVIRONMENT_COUNT do
        SendMessageW(hEAXCombo,CB_ADDSTRING,0,tlparam(TranslateW(EAXItems[i].name)));
      SendMessage(hEAXCombo,CB_SETCURSEL,DBReadByte(0,PluginName,optEAXType,0),0);

      if BassStatus=rbs_null then
        MyLoadBass;
      // Fill combo with devices
      FillDeviceCombo(GetDlgItem(Dialog,IDC_DEVICE),usedevice);

      // Add preset to combo
      wnd:=GetDlgItem(Dialog,IDC_PRESET);

      for i:=0 to HIGH(Presets) do
        SendMessageW(wnd,CB_ADDSTRING,0,tlparam(Presets[i].name));
      SendMessage(wnd,CB_SETCURSEL,OldEQPreset,0);

      // Equalizer
      for i:=0 to 9 do
      begin
        eq[i].wnd:=GetDlgItem(Dialog,IDC_EQ00+i);
        SendMessage(eq[i].wnd,TBM_SETRANGE,1,(16 shl 16)-15);
        SendMessage(eq[i].wnd,TBM_SETTIC,0,0);
        SendMessage(eq[i].wnd,TBM_SETPOS,1,-trunc(eq[i].param.fGain));
        SendDlgItemMessageA(Dialog,IDC_0+i,WM_SETTEXT,0,tlparam(eq[i].text));
        EnableWindow(eq[i].wnd,isEQ_OFF=BST_UNCHECKED);
      end;
      
      CheckDlgButton(Dialog,IDC_EQOFF,isEQ_OFF);

      dst:=DBReadUnicode(0,PluginName,optBASSPath,nil);
      SetDlgItemTextW(Dialog,IDC_BASSPATH,dst);
      mFreeMem(dst);

      DlgInit:=0;
    end;

    WM_COMMAND: begin
      case wParam shr 16 of
        EN_CHANGE: begin
          if DlgInit=0 then
            SendMessage(GetParent(Dialog),PSM_CHANGED,0,0);
        end;

        BN_CLICKED: begin
          case loword(wParam) of
            IDC_EQOFF, IDC_ZERO:
              SendMessage(GetParent(Dialog),PSM_CHANGED,0,0);
          end;

          case loword(wParam) of

            IDC_BASSPTHBTN: begin
              dst:=nil;
              if SelectDirectory(pWideChar(nil),dst,Dialog) then
              begin
                PathToRelativeW(dst,buf);
                SetDlgItemTextW(Dialog,IDC_BASSPATH,pWideChar(@buf));
                mFreeMem(dst);
              end;
            end;

            IDC_EQOFF: begin
              isEQ_OFF:=IsDlgButtonChecked(Dialog,IDC_EQOFF);
              if isEQ_OFF=BST_UNCHECKED then
                EQ_ON
              else
                EQ_OFF;

              for i:=0 to 9 do
                EnableWindow(eq[i].wnd,isEQ_OFF=BST_UNCHECKED);
            end;

            IDC_ZERO: begin
              OldEQPreset:=-1;
              for i:=0 to 9 do
              begin
                eq[i].param.fGain:=0;
                SendMessage(eq[i].wnd,TBM_SETPOS,1,0);
                if (chan<>0) and (isEQ_OFF=BST_UNCHECKED) then
                  BASS_FXSetParameters(eq[i].fx,@eq[i].param);
              end;
            end;

            IDC_EQ_ADD: begin
              SetLength(Presets,Length(Presets)+1);

              for i:=0 to 9 do
                Presets[HIGH(Presets)].preset[i]:=-SendMessage(eq[i].wnd,TBM_GETPOS,0,0);

              wnd:=GetDlgItem(Dialog,IDC_PRESET);
              Presets[HIGH(Presets)].name:=GetDlgText(wnd,false);
              if Presets[HIGH(Presets)].name=nil then
                StrDupW(Presets[HIGH(Presets)].name,'New');
              OldEQPreset:=SendMessage(wnd,CB_SETCURSEL,
                SendMessageW(wnd,CB_ADDSTRING,0,tlparam(Presets[HIGH(Presets)].name)),0);
            end;

            IDC_EQ_DEL: begin
              wnd:=GetDlgItem(Dialog,IDC_PRESET);
              i:=SendMessage(wnd,CB_GETCURSEL,0,0);
              if (i>=0) and (i<=HIGH(Presets)) then
              begin
                SendMessage(wnd,CB_DELETESTRING,i,0);
                mFreeMem(Presets[i].name);
                move(Presets[i+1],Presets[i],(HIGH(Presets)-i)*SizeOf(tPreset));
                SetLength(Presets,Length(Presets)-1);
                OldEQPreset:=-1;
                SendMessage(wnd,CB_SETCURSEL,-1,0);
              end;
            end;

          end;
        end;

        CBN_SELCHANGE: begin
          SendMessage(GetParent(Dialog),PSM_CHANGED,0,0);
          case loword(wParam) of
            IDC_PRESET: begin
              OldEQPreset:=SendDlgItemMessage(Dialog,IDC_PRESET,CB_GETCURSEL,0,0);
              for i:=0 to 9 do
              begin
                SendMessage(eq[i].wnd,TBM_SETPOS,1,-Presets[OldEQPreset].preset[i]);
                eq[i].param.fGain:=Presets[OldEQPreset].preset[i];
                if (chan<>0) and (isEQ_OFF=BST_UNCHECKED) then
                  BASS_FXSetParameters(eq[i].fx,@eq[i].param);
              end;
            end;
            IDC_EAXTYPE: begin
{
              i:=SendDlgItemMessage(Dialog,IDC_EAXTYPE,CB_GETCURSEL,0,0);
              DBWriteByte(0,PluginName,optEAXType,i);
              if i=0 then
                BASS_SetEAXParameters(-1,0,-1,-1)
              else
                BASS_SetEAXPreset(EAXItems[i].code);
}
            end;
          end;
        end;
      end;
    end;

    WM_VSCROLL: begin
      for i:=0 to 9 do
      begin
        if HWND(lParam)=eq[i].wnd then
        begin
          eq[i].param.fGain:=-SendMessage(lParam,TBM_GETPOS,0,0);
          if (chan<>0) and (isEQ_OFF=BST_UNCHECKED) then
            BASS_FXSetParameters(eq[i].fx,@eq[i].param);
          OldEQPreset:=-1;
          break;
        end;
      end;
      SendMessage(GetParent(Dialog),PSM_CHANGED,0,0);
    end;

    WM_HSCROLL: begin
       Service_RadioSetVolume(SendMessage(lParam,TBM_GETPOS,0,0),1)
    end;

    WM_NOTIFY: begin
      if integer(PNMHdr(lParam)^.code)=PSN_APPLY then
      begin
        SavePresets;
        level:=0;

        device:=GetDlgText(Dialog,IDC_DEVICE,true);
        StrCopy(device,device+1);
        if StrCmp(usedevice,device)<>0 then
        begin
          level:=1;
          mFreeMem(usedevice);
          usedevice:=device;
        end
        else
          mFreeMem(device);

//!!  bass path saving here
        old:=DBReadUnicode(0,PluginName,optBASSPath,nil);
        dst:=GetDlgText(Dialog,IDC_BASSPATH);
        if lstrcmpiw(dst,old)<>0 then
        begin
          level:=2;
          DBWriteUnicode(0,PluginName,optBASSPath,dst);
        end;
        mFreeMem(dst);
        mFreeMem(old);

        i:=SendDlgItemMessage(Dialog,IDC_EAXTYPE,CB_GETCURSEL,0,0);
        DBWriteByte(0,PluginName,optEAXType,i);

{
  level>0 - need to change
  ActiveContact<>0 - need to restart
  BassStatus<>rbs_null - need to stop (free, unload)
  really,  if ActiveContact<>0 then BassStatus<>rbs_null
}
        //!!really, need to do it if only active atm
        // need to reload dll or change outpout device
        // don't change proxy
        if level>0 then
        begin
          ltmp:=ActiveContact<>0;
          if ltmp then
            CallService(MS_RADIO_COMMAND,MRC_STOP,1);
          // save current, stop, bass free

          MyFreeBass;
          if level=2 then
          begin
            MyUnloadBass;
            // load dll
            if ltmp then
              MyLoadBass;
          end;
          // init device, restore current
          if ltmp and (BassStatus=rbs_load) then
          begin
            MyInitBass;
          end;
          if ltmp and (BassStatus=rbs_init) then
          begin
            CallService(MS_RADIO_COMMAND,MRC_PAUSE,0);
          end;
        end
        // if device re-init, EAX settings applied there
        else if BassStatus=rbs_init then
        begin
          if i=0 then
            BASS_SetEAXParameters(-1,0,-1,-1)
          else
            BASS_SetEAXPreset(EAXItems[i].code);
        end;


        ForcedMono:=IsDlgButtonChecked(Dialog,IDC_MONO);

        NumTries:=GetDlgItemInt(Dialog,IDC_TRIES,ltmp,false);
        if NumTries<1 then NumTries:=1;

        // Bass config settings

        sPreBuf:=GetDlgItemInt(Dialog,IDC_PREBUF,ltmp,false);
        if sPreBuf>100 then sPreBuf:=100;

        sBuffer:=GetDlgItemInt(Dialog,IDC_BUFFER,ltmp,false);
        if sBuffer>20000 then sBuffer:=20000;

        sTimeout:=GetDlgItemInt(Dialog,IDC_TIMEOUT,ltmp,false);
        if sTimeout>30000 then sTimeout:=30000;

        SetBassConfig;

        SaveTechSettings;
      end;
    end;

//  else
//    result:=DefWindowProc(Dialog,hMessage,wParam,lParam);
  end;
end;

function OnOptInitialise(wParam:WPARAM;lParam:LPARAM):int;cdecl;
var
  odp:TOPTIONSDIALOGPAGE;
begin
  FillChar(odp,SizeOf(odp),0);
  odp.flags      :=ODPF_BOLDGROUPS;
  odp.Position   :=900003000;
  odp.hInstance  :=hInstance;
  odp.szGroup.a  :='Network';
  odp.szTitle.a  :=PluginName;

  odp.pszTemplate:=MAKEINTRESOURCEA(IDD_SETTING);
  odp.pfnDlgProc :=@DlgProcOpt;
  odp.szTab.a    :='Common';
  CallService(MS_OPT_ADDPAGE,wParam,tlparam(@odp));

  odp.pszTemplate:=MAKEINTRESOURCEA(IDD_SETTING_TECH);
  odp.pfnDlgProc :=@DlgProcOptTech;//!!
  odp.szTab.a    :=Translate('Advanced');
  CallService(MS_OPT_ADDPAGE,wParam,tlparam(@odp));

  result:=0;
end;

// checking proto in several places for speed, not size economy
function OnSettingsChanged(wParam:WPARAM;lParam:LPARAM):int;cdecl;
var
  buf:array [0..MAX_PATH-1] of AnsiChar;
  i:integer;
  pc:PAnsiChar;
begin
  result:=0;

  with PDBCONTACTWRITESETTING(lParam)^ do
  begin

    if AuMute<>BST_UNCHECKED then
    begin
      if StrCmp(szModule,'Skin')=0 then
      begin
        if StrCmp(szSetting,'UseSound')=0 then
        begin
          // Mute
          if value.bVal=0 then
          begin
            if gVolume>=0 then
              Service_RadioMute(0,0);
          end
          // Unmute
          else
          begin
            if gVolume<0 then
              Service_RadioMute(0,0);
          end;
        end;

        exit;
      end
    end;

    // works only if called AFTER changes
    if StrCmp(szModule,strCList)=0 then
    begin
      if StrCmp(szSetting,optMyHandle)=0 then
      begin
        if value._type=DBVT_DELETED then
        begin
          if StrCmp(PAnsiChar(CallService(MS_PROTO_GETCONTACTBASEPROTO,wParam,0)),
             PluginName)<>0 then exit;

          pc:=DBReadString(wParam,PluginName,optNick);
          DBWriteString(wParam,strCList,optMyHandle,pc);
          mFreeMem(pc);
        end;
      end;
      exit;
    end;

    if StrCmp(szModule,'UserInfo')<>0 then exit;

    if StrCmp(szSetting,optAge)=0 then
    begin
      if StrCmp(PAnsiChar(CallService(MS_PROTO_GETCONTACTBASEPROTO,wParam,0)),
         PluginName)<>0 then exit;
      if value._type=DBVT_DELETED then
        i:=DBReadWord(wParam,PluginName,optAge)
      else
        i:=value.wVal;
      DBWriteString(wParam,PluginName,optBitrate,IntToStr(buf,i));
      exit;
    end;

    case value._type of
      DBVT_DELETED,
      DBVT_ASCIIZ ,
      DBVT_WCHAR  ,
      DBVT_UTF8   :
      if StrCmp(PAnsiChar(CallService(MS_PROTO_GETCONTACTBASEPROTO,wParam,0)),
         PluginName)<>0 then exit;
    else
      exit;
    end;

    case value._type of
      DBVT_DELETED: pc:=DBReadString(wParam,PluginName,szSetting);
      DBVT_ASCIIZ : pc:=value.szVal.a;
      DBVT_WCHAR  : WideToAnsi(value.szVal.w,pc,MirandaCP);
      DBVT_UTF8   : UTF8ToAnsi(value.szVal.a,pc,MirandaCP);
    end;

    if      StrCmp(szSetting,optFirstName)=0 then DBWriteString(wParam,PluginName,optStationURL,pc)
    else if StrCmp(szSetting,optNick     )=0 then DBWriteString(wParam,strCList,optMyHandle,pc)
    else if StrCmp(szSetting,optLastName )=0 then DBWriteString(wParam,PluginName,optGenre,pc);

    if value._type<>DBVT_ASCIIZ then
      mFreeMem(pc);
  end;
end;

function OnContactDeleted(wParam:WPARAM;lParam:LPARAM):int;cdecl;
begin
  result:=0;
  if ActiveContact<>THANDLE(wParam) then exit;
  ControlCenter(MRC_STOP,wParam);

{  if StrCmp(PAnsiChar(CallService(MS_PROTO_GETCONTACTBASEPROTO,wParam,0)),
             PluginName)<>0 then exit;
}
end;
