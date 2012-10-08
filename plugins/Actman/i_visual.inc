{Dialog visual part, depends of Dialog window}
  procedure SHControl(ctrl,mode:dword);
  begin
    ShowWindow(GetDlgItem(Dialog,ctrl),mode);
  end;

  procedure SetupControls(group,mode:dword);
  begin
    case group of
      ACT_CONTACT: begin
        SHControl(IDC_STAT_CONTACT,mode);
        SHControl(IDC_CONTACTLIST ,mode);
        SHControl(IDC_CNT_KEEP    ,mode);
        SHControl(IDC_CNT_FILTER  ,mode);
        SHControl(IDC_STAT_FORMAT ,mode);
        SHControl(IDC_EDIT_FORMAT ,mode);
        SHControl(IDC_CNT_APPLY   ,mode);
        SHControl(IDC_STAT_FHELP  ,mode);
      end;
      ACT_SERVICE: begin
        SHControl(IDC_STAT_WPAR   ,mode);
        SHControl(IDC_STAT_LPAR   ,mode);
        SHControl(IDC_STAT_WPAR1  ,mode);
        SHControl(IDC_STAT_LPAR1  ,mode);
        SHControl(IDC_STAT_SERVICE,mode);
        SHControl(IDC_EDIT_SERVICE,mode);
        SHControl(IDC_SRV_SRVC    ,mode);
        SHControl(IDC_FLAG_WPAR   ,mode);
        SHControl(IDC_FLAG_LPAR   ,mode);

        SHControl(IDC_RES_GROUP   ,mode);
        SHControl(IDC_RES_POPUP   ,mode);
        SHControl(IDC_RES_MESSAGE ,mode);
        SHControl(IDC_RES_INSERT  ,mode);

        SHControl(IDC_SRV_RESULT  ,mode);
        SHControl(IDC_SRV_RESSTAT ,mode);

        if mode=SW_SHOW then
        begin
//!! Check parameter type
          if CB_GetData(GetDlgItem(Dialog,IDC_FLAG_WPAR))=ptStruct then
//          if SendDlgItemMessage(Dialog,IDC_FLAG_WPAR,CB_GETCURSEL,0,0)=ptStruct then
          begin
            SHControl(IDC_WSTRUCT  ,SW_SHOW);
            SHControl(IDC_EDIT_WPAR,SW_HIDE);
            SHControl(IDC_SRV_WPAR ,SW_HIDE);
          end
          else
          begin
            SHControl(IDC_WSTRUCT  ,SW_HIDE);
            SHControl(IDC_EDIT_WPAR,SW_SHOW);
            SHControl(IDC_SRV_WPAR ,SW_SHOW);
          end;
          if CB_GetData(GetDlgItem(Dialog,IDC_FLAG_LPAR))=ptStruct then
//          if SendDlgItemMessage(Dialog,IDC_FLAG_LPAR,CB_GETCURSEL,0,0)=ptStruct then
          begin
            SHControl(IDC_LSTRUCT  ,SW_SHOW);
            SHControl(IDC_EDIT_LPAR,SW_HIDE);
            SHControl(IDC_SRV_LPAR ,SW_HIDE);
          end
          else
          begin
            SHControl(IDC_LSTRUCT  ,SW_HIDE);
            SHControl(IDC_EDIT_LPAR,SW_SHOW);
            SHControl(IDC_SRV_LPAR ,SW_SHOW);
          end;

          case CB_GetData(GetDlgItem(Dialog,IDC_SRV_RESULT)) of
            sresHex: ;
            sresInt: SHControl(IDC_RES_SIGNED,SW_SHOW);
            sresString: begin
              SHControl(IDC_RES_UNICODE,SW_SHOW);
              SHControl(IDC_RES_FREEMEM,SW_SHOW);
            end;
            sresStruct: ;
          end;

        end
        else
        begin
          SHControl(IDC_WSTRUCT    ,SW_HIDE);
          SHControl(IDC_LSTRUCT    ,SW_HIDE);
          SHControl(IDC_SRV_WPAR   ,SW_HIDE);
          SHControl(IDC_SRV_LPAR   ,SW_HIDE);
          SHControl(IDC_EDIT_WPAR  ,SW_HIDE);
          SHControl(IDC_EDIT_LPAR  ,SW_HIDE);
          SHControl(IDC_RES_FREEMEM,SW_HIDE);
          SHControl(IDC_RES_UNICODE,SW_HIDE);
          SHControl(IDC_RES_SIGNED ,SW_HIDE);
        end;
      end;
      ACT_PROGRAM: begin
        SHControl(IDC_PROCESS_GROUP,mode);
        SHControl(IDC_PROGRAM      ,mode);
        SHControl(IDC_EDIT_PROCTIME,mode);
        SHControl(IDC_EDIT_PRGPATH ,mode);
        SHControl(IDC_PRG_PRG      ,mode);
        SHControl(IDC_EDIT_PRGARGS ,mode);
        SHControl(IDC_PRG_ARG      ,mode);
        SHControl(IDC_STAT_PROCTIME,mode);
        SHControl(IDC_STAT_PRGPATH ,mode);
        SHControl(IDC_STAT_PRGARGS ,mode);

        SHControl(IDC_PRSTART_GROUP,mode);
        SHControl(IDC_FLAG_NORMAL  ,mode);
        SHControl(IDC_FLAG_HIDDEN  ,mode);
        SHControl(IDC_FLAG_MAXIMIZE,mode);
        SHControl(IDC_FLAG_MINIMIZE,mode);

        SHControl(IDC_FLAG_CURPATH,mode);
        SHControl(IDC_FLAG_CONTINUE,mode);
        SHControl(IDC_FLAG_PARALLEL,mode);

        if mode=SW_SHOW then
        begin
          if isVarsInstalled then
             SHControl(IDC_HLP_FVARS,SW_SHOW);
        end
        else
          SHControl(IDC_HLP_FVARS,SW_HIDE);
      end;
      ACT_TEXT: begin
        SHControl(IDC_FLAG_CLIP   ,mode);
        SHControl(IDC_FLAG_MESSAGE,mode);
        SHControl(IDC_CLIP_COPYTO ,mode);
        SHControl(IDC_CLIP_PASTE  ,mode);
        SHControl(IDC_CLIP_GROUP  ,mode);
        SHControl(IDC_FILE_ENC    ,mode);

        SHControl(IDC_FLAG_FILE   ,mode);
        SHControl(IDC_FILE_PATH   ,mode);
        SHControl(IDC_TXT_FILE    ,mode);
        SHControl(IDC_FILE_FILEBTN,mode);
        SHControl(IDC_FILE_READ   ,mode);
        SHControl(IDC_FILE_WRITE  ,mode);
        SHControl(IDC_FILE_APPEND ,mode);
        SHControl(IDC_FILE_GROUP  ,mode);

        SHControl(IDC_TXT_TEXT     ,mode);
        SHControl(IDC_EDIT_INSERT  ,mode);
        SHControl(IDC_STAT_INSERT  ,mode);

        if mode=SW_SHOW then
        begin
          if isVarsInstalled then
             SHControl(IDC_HLP_VARS,SW_SHOW);
        end
        else
          SHControl(IDC_HLP_VARS,SW_HIDE);
      end;
      ACT_ADVANCE: begin
        SHControl(IDC_CONDITION,mode);
        SHControl(IDC_FLAG_GT  ,mode);
        SHControl(IDC_FLAG_LT  ,mode);
        SHControl(IDC_FLAG_EQ  ,mode);
        SHControl(IDC_FLAG_NOP ,mode);
        SHControl(IDC_FLAG_NOT ,mode);
        SHControl(IDC_ADV_VALUE,mode);
        SHControl(IDC_STAT_VAL ,mode);
        SHControl(IDC_ADV_ASINT,mode);

        SHControl(IDC_OPERATION ,mode);
        SHControl(IDC_FLAG_BREAK,mode);
        SHControl(IDC_FLAG_JUMP ,mode);
        SHControl(IDC_FLAG_ANOP ,mode);

        SHControl(IDC_FLAG_MATH,mode);
        SHControl(IDC_ADV_OPER ,mode);
        SHControl(IDC_ADV_VAL2 ,mode);
        SHControl(IDC_ADV_VAL1 ,mode);

        if mode=SW_SHOW then
        begin
          if isVarsInstalled then
             SHControl(IDC_ADV_HVARS,SW_SHOW);
        end
        else
          SHControl(IDC_ADV_HVARS,SW_HIDE);
        SHControl(IDC_FLAG_VARS,mode);
        SHControl(IDC_ADV_VARS ,mode);
      end;
      ACT_CHAIN: begin
        SHControl(IDC_STAT_GROUPS,mode);
        SHControl(IDC_GROUP_LIST ,mode);
      end;
      ACT_RW: begin
        SHControl(IDC_STAT_CONTACT,mode);
        SHControl(IDC_CONTACTLIST ,mode);

        SHControl(IDC_RW_OPER   ,mode);
        SHControl(IDC_RW_VAL    ,mode);
        SHControl(IDC_RW_READ   ,mode);
        SHControl(IDC_RW_WRITE  ,mode);
        SHControl(IDC_RW_DELETE ,mode);
        SHControl(IDC_RW_STATM  ,mode);
        SHControl(IDC_RW_MODULE ,mode);
        SHControl(IDC_RW_STATS  ,mode);
        SHControl(IDC_RW_SETTING,mode);
        SHControl(IDC_RW_TVAR   ,mode);

        if mode=SW_SHOW then
        begin
          if CB_GetData(GetDlgItem(Dialog,IDC_RW_DATATYPE))>2 then
          begin
            SHControl(IDC_RW_VALUE,SW_HIDE);
            SHControl(IDC_RW_TEXT ,SW_SHOW);
          end
          else
          begin
            SHControl(IDC_RW_VALUE,SW_SHOW);
            SHControl(IDC_RW_TEXT ,SW_HIDE);
          end
        end
        else
        begin
          SHControl(IDC_RW_VALUE,SW_HIDE);
          SHControl(IDC_RW_TEXT ,SW_HIDE);
        end;
        SHControl(IDC_RW_DATATYPE,mode);

        SHControl(IDC_RW_CURRENT,mode);
        SHControl(IDC_RW_PARAM  ,mode);
        SHControl(IDC_RW_RESULT ,mode);
        SHControl(IDC_RW_MANUAL ,mode);
        SHControl(IDC_RW_LAST   ,mode);

        SHControl(IDC_RW_MVAR   ,mode);
        SHControl(IDC_RW_SVAR   ,mode);
      end;
      ACT_MESSAGE: begin
        SHControl(IDC_MSG_KEEP  ,mode);
        SHControl(IDC_MSG_STAT1 ,mode);
        SHControl(IDC_MSG_STAT2 ,mode);
        SHControl(IDC_MSG_TITLE ,mode);
        SHControl(IDC_MSG_TEXT  ,mode);
        SHControl(IDC_MSG_BTNS  ,mode);
        SHControl(IDC_MSGB_ARI  ,mode);
        SHControl(IDC_MSGB_OK   ,mode);
        SHControl(IDC_MSGB_OC   ,mode);
        SHControl(IDC_MSGB_RC   ,mode);
        SHControl(IDC_MSGB_YN   ,mode);
        SHControl(IDC_MSGB_YNC  ,mode);
        SHControl(IDC_MSG_ICONS ,mode);
        SHControl(IDC_MSGI_NONE ,mode);
        SHControl(IDC_MSGI_WARN ,mode);
        SHControl(IDC_MSGI_INFO ,mode);
        SHControl(IDC_MSGI_QUEST,mode);
        SHControl(IDC_MSGI_ERROR,mode);

        SHControl(IDC_MSG_TTL   ,mode);
        SHControl(IDC_MSG_TXT   ,mode);
      end;
    end;
  end;

  procedure SetButtonOnOff(ctrl,state:dword);
  begin
    CheckDlgButton(Dialog,ctrl,state);
    case ctrl of
      IDC_SRV_WPAR,
      IDC_SRV_LPAR,
      IDC_SRV_SRVC,
      IDC_PRG_PRG ,
      IDC_PRG_ARG ,
      IDC_TXT_FILE,
      IDC_TXT_TEXT,
      IDC_RW_MVAR ,
      IDC_RW_SVAR ,
      IDC_RW_TVAR ,
      IDC_MSG_TTL ,
      IDC_MSG_TXT : SetButtonIcon(GetDlgItem(Dialog,ctrl),checknames[state]);
     end;
  end;

  procedure ButtonOff(ctrl:dword); {$IFDEF MSWINDOWS}inline;{$ENDIF}
  begin
    SetButtonOnOff(ctrl,BST_UNCHECKED);
  end;

  procedure ButtonOn(ctrl:dword); {$IFDEF MSWINDOWS}inline;{$ENDIF}
  begin
    SetButtonOnOff(ctrl,BST_CHECKED);
  end;

  procedure TextClear(ctrl:dword);
  begin
    SetDlgItemTextW(Dialog,ctrl,nil);
  end;

  function FixParam(buf:PAnsiChar;flag:integer):integer;
  begin
    if      lstrcmpia(buf,Translate('hContact' ))=0 then result:=ptCurrent
    else if lstrcmpia(buf,Translate('parameter'))=0 then result:=ptParam
    else if lstrcmpia(buf,Translate('result'   ))=0 then result:=ptResult
    else if lstrcmpia(buf,Translate('structure'))=0 then result:=ptStruct
    else
    begin
      if (buf[0] in ['0'..'9']) or ((buf[0]='-') and (buf[1] in ['0'..'9'])) or
        ((buf[0]='$') and (buf[1] in sHexNum)) then
        result:=ptNumber
      else
        result:=ptString;
    end;

    CB_SelectData(Dialog,flag,result);
//    SendDlgItemMessage(Dialog,flag,CB_SETCURSEL,result,0);
    SendMessage(Dialog,WM_COMMAND,(CBN_SELCHANGE shl 16) or flag,GetDlgItem(Dialog,flag));
  end;

  procedure ReloadService;
  var
    pc:pAnsiChar;
    buf,buf1:array [0..MaxDescrLen] of AnsiChar;
    wnd:hwnd;
    i:integer;
//    bufw:array [0..MaxDescrLen] of WideChar;
  begin
    wnd:=GetDlgItem(Dialog,IDC_EDIT_SERVICE);
    SendMessageA(wnd,CB_GETLBTEXT,SendMessage(wnd,CB_GETCURSEL,0,0),tlparam(@buf));
    ApiCard.Service:=@buf;

    pc:=ApiCard.FillParams(GetDlgItem(Dialog,IDC_EDIT_WPAR),true);
    if pc<>nil then
    begin
      if GetDlgItemTextA(Dialog,IDC_EDIT_WPAR,buf1,SizeOf(buf1))>0 then
        case FixParam(@buf1,IDC_FLAG_WPAR) of
          ptStruct: begin
            mFreeMem(wstruct);
            StrDup(wstruct,StrScan(pc,'|')+1);
//            AnsiToWide(StrScan(pc,'|')+1,wstruct,MirandaCP);
          end;
        end;
      mFreeMem(pc);
    end;

    pc:=ApiCard.FillParams(GetDlgItem(Dialog,IDC_EDIT_LPAR),false);
    if pc<>nil then
    begin
      if GetDlgItemTextA(Dialog,IDC_EDIT_LPAR,buf1,SizeOf(buf1))>0 then
        case FixParam(@buf1,IDC_FLAG_LPAR) of
          ptStruct: begin
            mFreeMem(lstruct);
            StrDup(lstruct,StrScan(pc,'|')+1);
//            AnsiToWide(StrScan(pc,'|')+1,lstruct,MirandaCP);
          end;
        end;
      mFreeMem(pc);
    end;

    pc:=ApiCard.ResultType;
    i:=sresInt;
    if pc<>nil then
    begin
      if      lstrcmpia(pc,'struct')=0 then i:=sresStruct
      else if lstrcmpia(pc,'hex'   )=0 then i:=sresHex
      else if lstrcmpia(pc,'int')=0 then
      begin
        i:=sresInt;
        ButtonOff(IDC_RES_SIGNED);
      end
      else if lstrcmpia(pc,'signed')=0 then
      begin
        i:=sresInt;
        ButtonOn(IDC_RES_SIGNED);
      end
      else if lstrcmpia(pc,'str')=0 then
      begin
        i:=sresString;
        ButtonOff(IDC_RES_UNICODE);
      end
      else if lstrcmpia(pc,'wide')=0 then
      begin
        i:=sresString;
        ButtonOn(IDC_RES_UNICODE);
      end;
      mFreeMem(pc);
    end;
    CB_SelectData(Dialog,IDC_SRV_RESULT,i);

//    ApiCard.Show;
  end;

  procedure SelectActionType(group:dword);
  var
    i:integer;
  begin
    for i:=0 to ACT_MAXTYPE-1 do
      if ActIds[i].code=group then break;
    SendDlgItemMessage(Dialog,IDC_ACTION_TYPE,CB_SETCURSEL,i,0);
  end;

  procedure ClearControls(group:dword);
  begin
//!    SelectActionType(group);
    case group of
      ACT_CONTACT: begin
        EnableWindow(GetDlgItem(Dialog,IDC_CONTACTLIST),true);
        SendDlgItemMessage(Dialog,IDC_CONTACTLIST,CB_SETCURSEL,0,0);
        ButtonOff(IDC_CNT_KEEP);
        ButtonOff(IDC_CNT_FILTER);
        TextClear(IDC_EDIT_FORMAT);
      end;
      ACT_SERVICE: begin
        TextClear(IDC_EDIT_SERVICE);
        TextClear(IDC_EDIT_WPAR);
        TextClear(IDC_EDIT_LPAR);
        EnableWindow(GetDlgItem(Dialog,IDC_EDIT_WPAR),true);
        EnableWindow(GetDlgItem(Dialog,IDC_EDIT_LPAR),true);
//        SendDlgItemMessage(Dialog,IDC_FLAG_WPAR,CB_SETCURSEL,0,0);
//        SendDlgItemMessage(Dialog,IDC_FLAG_LPAR,CB_SETCURSEL,0,0);
        CB_SelectData(GetDlgItem(Dialog,IDC_FLAG_WPAR),ptNumber);
        CB_SelectData(GetDlgItem(Dialog,IDC_FLAG_LPAR),ptNumber);

        CB_SelectData(GetDlgItem(Dialog,IDC_SRV_RESULT),sresInt);

        SendMessage(GetDlgItem(Dialog,IDC_EDIT_WPAR),CB_RESETCONTENT,0,0);
        SendMessage(GetDlgItem(Dialog,IDC_EDIT_LPAR),CB_RESETCONTENT,0,0);

        ButtonOff(IDC_RES_POPUP);
        ButtonOff(IDC_RES_MESSAGE);
        ButtonOff(IDC_RES_INSERT);

        ButtonOff(IDC_RES_FREEMEM);
        ButtonOff(IDC_RES_UNICODE);
        ButtonOff(IDC_RES_SIGNED);

        ButtonOff(IDC_SRV_WPAR);
        ButtonOff(IDC_SRV_LPAR);
        ButtonOff(IDC_SRV_SRVC);
      end;
      ACT_PROGRAM: begin
        TextClear(IDC_EDIT_PROCTIME);
        TextClear(IDC_EDIT_PRGPATH);
        TextClear(IDC_EDIT_PRGARGS);

        ButtonOff(IDC_FLAG_NORMAL);
        ButtonOff(IDC_FLAG_HIDDEN);
        ButtonOff(IDC_FLAG_MINIMIZE);
        ButtonOff(IDC_FLAG_MAXIMIZE);

        ButtonOff(IDC_FLAG_CURPATH);
        ButtonOff(IDC_FLAG_CONTINUE);
        ButtonOff(IDC_FLAG_PARALLEL);

        ButtonOff(IDC_PRG_PRG);
        ButtonOff(IDC_PRG_ARG);
      end;
      ACT_TEXT: begin
        ButtonOff(IDC_FLAG_CLIP);
        ButtonOff(IDC_FLAG_MESSAGE);
        ButtonOff(IDC_CLIP_COPYTO);
        ButtonOff(IDC_CLIP_PASTE);

        ButtonOff(IDC_FLAG_FILE);
        TextClear(IDC_FILE_PATH);
        ButtonOff(IDC_FILE_READ);
        ButtonOff(IDC_FILE_WRITE);
        ButtonOff(IDC_FILE_APPEND);
        
        EnableWindow(GetDlgItem(Dialog,IDC_EDIT_INSERT ),false);
        EnableWindow(GetDlgItem(Dialog,IDC_CLIP_COPYTO ),false);
        EnableWindow(GetDlgItem(Dialog,IDC_CLIP_PASTE  ),false);

        EnableWindow(GetDlgItem(Dialog,IDC_FILE_ENC    ),false);
        EnableWindow(GetDlgItem(Dialog,IDC_FILE_PATH   ),false);
        EnableWindow(GetDlgItem(Dialog,IDC_FILE_FILEBTN),false);
        EnableWindow(GetDlgItem(Dialog,IDC_FILE_READ   ),false);
        EnableWindow(GetDlgItem(Dialog,IDC_FILE_WRITE  ),false);
        EnableWindow(GetDlgItem(Dialog,IDC_FILE_APPEND ),false);

        TextClear(IDC_EDIT_INSERT);

        ButtonOff(IDC_TXT_FILE);
        ButtonOff(IDC_TXT_TEXT);
      end;
      ACT_ADVANCE: begin
        ButtonOff(IDC_FLAG_GT);
        ButtonOff(IDC_FLAG_LT);
        ButtonOff(IDC_FLAG_EQ);
        ButtonOff(IDC_FLAG_NOP);
        ButtonOff(IDC_FLAG_NOT);
        TextClear(IDC_ADV_VALUE);

        ButtonOff(IDC_FLAG_BREAK);
        ButtonOff(IDC_FLAG_JUMP);
        ButtonOff(IDC_FLAG_ANOP);
        EnableWindow(GetDlgItem(Dialog,IDC_ADV_VAL2),false);

        ButtonOff(IDC_FLAG_MATH);
        SendDlgItemMessage(Dialog,IDC_ADV_OPER,CB_SETCURSEL,0,0);
        TextClear(IDC_ADV_VAL1);
        ButtonOff(IDC_ADV_ASINT);

        EnableWindow(GetDlgItem(Dialog,IDC_ADV_OPER),false);
        EnableWindow(GetDlgItem(Dialog,IDC_ADV_VAL1),false);
        EnableWindow(GetDlgItem(Dialog,IDC_ADV_VARS),false);

        ButtonOff(IDC_FLAG_VARS);
        TextClear(IDC_ADV_VARS);
      end;
      ACT_CHAIN: begin
//        FillChainList(Dialog);
        SendDlgItemMessage(Dialog,IDC_GROUP_LIST,CB_SETCURSEL,0,0);
      end;
      ACT_RW: begin
        ButtonOff(IDC_RW_READ);
        ButtonOff(IDC_RW_WRITE);
        ButtonOff(IDC_RW_DELETE);
        SendDlgItemMessage(Dialog,IDC_CONTACTLIST,CB_SETCURSEL,0,0);
        EnableWindow(GetDlgItem(Dialog,IDC_CONTACTLIST),true);
        TextClear(IDC_CONTACTLIST);
        TextClear(IDC_RW_MODULE);
        TextClear(IDC_RW_SETTING);
        TextClear(IDC_RW_VALUE);
        TextClear(IDC_RW_TEXT);

        ButtonOff(IDC_RW_CURRENT);
        ButtonOff(IDC_RW_PARAM);
        ButtonOff(IDC_RW_RESULT);
        ButtonOff(IDC_RW_MANUAL);
        ButtonOff(IDC_RW_LAST);

        ButtonOff(IDC_RW_MVAR);
        ButtonOff(IDC_RW_SVAR);
        ButtonOff(IDC_RW_TVAR);
      end;
      ACT_MESSAGE: begin
        TextClear(IDC_MSG_TITLE);
        TextClear(IDC_MSG_TEXT);
        ButtonOff(IDC_MSG_KEEP);
        ButtonOff(IDC_MSGB_OK);
        ButtonOff(IDC_MSGB_OC);
        ButtonOff(IDC_MSGB_ARI);
        ButtonOff(IDC_MSGB_YNC);
        ButtonOff(IDC_MSGB_YN);
        ButtonOff(IDC_MSGB_RC);
        ButtonOff(IDC_MSGI_NONE);
        ButtonOff(IDC_MSGI_ERROR);
        ButtonOff(IDC_MSGI_QUEST);
        ButtonOff(IDC_MSGI_WARN);
        ButtonOff(IDC_MSGI_INFO);

        ButtonOff(IDC_MSG_TTL);
        ButtonOff(IDC_MSG_TXT);
      end;
    end;
  end;

  procedure ClearDialogData;
  begin
    ClearControls(ACT_CONTACT);
    ClearControls(ACT_SERVICE);
    ClearControls(ACT_PROGRAM);
    ClearControls(ACT_TEXT);
    ClearControls(ACT_ADVANCE);
    ClearControls(ACT_CHAIN);
    ClearControls(ACT_RW);
    ClearControls(ACT_MESSAGE);
    mFreeMem(wstruct);
    mFreeMem(lstruct);
  end;

  procedure SHWindows(exclude:dword=0);
  begin
    SetupControls(ACT_CONTACT,SW_HIDE);
    SetupControls(ACT_SERVICE,SW_HIDE);
    SetupControls(ACT_PROGRAM,SW_HIDE);
    SetupControls(ACT_TEXT   ,SW_HIDE);
    SetupControls(ACT_ADVANCE,SW_HIDE);
    SetupControls(ACT_CHAIN  ,SW_HIDE);
    SetupControls(ACT_RW     ,SW_HIDE);
    SetupControls(ACT_MESSAGE,SW_HIDE);
{
    if exclude<>ACT_CONTACT then SetupControls(ACT_CONTACT,SW_HIDE);
    if exclude<>ACT_SERVICE then SetupControls(ACT_SERVICE,SW_HIDE);
    if exclude<>ACT_PROGRAM then SetupControls(ACT_PROGRAM,SW_HIDE);
    if exclude<>ACT_TEXT    then SetupControls(ACT_TEXT   ,SW_HIDE);
    if exclude<>ACT_ADVANCE then SetupControls(ACT_ADVANCE,SW_HIDE);
    if exclude<>ACT_CHAIN   then SetupControls(ACT_CHAIN  ,SW_HIDE);
    if exclude<>ACT_RW      then SetupControls(ACT_RW     ,SW_HIDE);
}
    case exclude of
      ACT_CONTACT,
      ACT_SERVICE,
      ACT_PROGRAM,
      ACT_TEXT   ,
      ACT_ADVANCE,
      ACT_CHAIN  ,
      ACT_RW     ,
      ACT_MESSAGE: begin
        SetupControls(exclude,SW_SHOW);
      end;
    end;
  end;

  procedure SHActButtons(mode:integer);
  begin
    ShowWindow(GetDlgItem(Dialog,IDC_STAT_ACTION),mode);
    ShowWindow(GetDlgItem(Dialog,IDC_ACTION_TYPE),mode);
  end;

  procedure InitDef(exclude:dword=0);
  begin
    if exclude<>ACT_CONTACT then
    begin
      ButtonOn(IDC_CNT_FILTER);
      // do nothing
    end;
    if exclude<>ACT_SERVICE then
    begin
      CB_SelectData(Dialog,IDC_SRV_RESULT,sresInt);
    end;
    if exclude<>ACT_PROGRAM then
    begin
      ButtonOn(IDC_FLAG_PARALLEL);
      ButtonOn(IDC_FLAG_NORMAL);
      SetDlgItemInt(Dialog,IDC_EDIT_PROCTIME,0,false);
    end;
    if exclude<>ACT_TEXT then
    begin
      ButtonOn(IDC_FLAG_CLIP);
      EnableWindow(GetDlgItem(Dialog,IDC_CLIP_COPYTO),true);
      EnableWindow(GetDlgItem(Dialog,IDC_CLIP_PASTE ),true);
      ButtonOn(IDC_CLIP_COPYTO);

      ButtonOn(IDC_FILE_READ);
      // do nothing
    end;
    if exclude<>ACT_ADVANCE then
    begin
      ButtonOn(IDC_FLAG_NOP);
      ButtonOn(IDC_FLAG_ANOP);
      SetDlgItemInt(Dialog,IDC_ADV_VALUE,0,false);
      SetDlgItemInt(Dialog,IDC_ADV_VAL1 ,0,false);
      EnableWindow(GetDlgItem(Dialog,IDC_ADV_VAL1),false);
      EnableWindow(GetDlgItem(Dialog,IDC_ADV_VAL2),false);
      EnableWindow(GetDlgItem(Dialog,IDC_ADV_OPER),false);
//      SendDlgItemMessage(Dialog,IDC_ADV_OPER,CB_SETCURSEL,0,0);
      CB_SelectData(Dialog,IDC_ADV_OPER,ORD(aeNot));
    end;
    if exclude<>ACT_CHAIN then
    begin
//      FillChainList(Dialog);
      SendDlgItemMessage(Dialog,IDC_GROUP_LIST,CB_SETCURSEL,0,0);
    end;
    if exclude<>ACT_RW then
    begin
      ButtonOn(IDC_RW_READ);
      SetDlgItemInt(Dialog,IDC_RW_VALUE,0,false);
      ButtonOn(IDC_RW_MANUAL);
      EnableWindow(GetDlgItem(Dialog,IDC_RW_VALUE),true);
      EnableWindow(GetDlgItem(Dialog,IDC_RW_TEXT ),true);
    end;
    if exclude<>ACT_MESSAGE then
    begin
      ButtonOn(IDC_MSGB_OK);
      ButtonOn(IDC_MSGI_NONE);
    end;
  end;

  // set buttons by options
  procedure FillAction(CurAction:integer);
  var
    i:integer;
    bb:boolean;
    wnd:HWND;
  begin
    with NewActionList^[CurAction] do
    begin
      if (flags and ACF_ASSIGNED)=0 then
        exit;
      if actionType=0 then
        actionType:=ACT_CONTACT;

      InitDef(actionType);
      SelectActionType(actionType);
      case actionType of

        ACT_CONTACT: begin
          if (flags and ACF_KEEPONLY)<>0 then
            ButtonOn(IDC_CNT_KEEP);
          if fCLfilter then
            ButtonOn(IDC_CNT_FILTER);
          SetDlgItemTextW(Dialog,IDC_EDIT_FORMAT,fCLformat);

          SendDlgItemMessage(Dialog,IDC_CONTACTLIST,CB_SETCURSEL,
            FindContact(GetDlgItem(Dialog,IDC_CONTACTLIST),contact),0);
        end;

        ACT_SERVICE: begin
          if SendDlgItemMessageA(Dialog,IDC_EDIT_SERVICE,CB_SELECTSTRING,twparam(-1),tlparam(service))<>CB_ERR then
            ReloadService
          else
            SetDlgItemTextA(Dialog,IDC_EDIT_SERVICE,service);

          if (flags2 and ACF2_SRV_WPAR)<>0 then ButtonOn(IDC_SRV_WPAR);
          if (flags2 and ACF2_SRV_LPAR)<>0 then ButtonOn(IDC_SRV_LPAR);
          if (flags2 and ACF2_SRV_SRVC)<>0 then ButtonOn(IDC_SRV_SRVC);

          if (flags and ACF_MESSAGE)<>0 then ButtonOn(IDC_RES_MESSAGE);
          if (flags and ACF_POPUP  )<>0 then ButtonOn(IDC_RES_POPUP);
          if (flags and ACF_INSERT )<>0 then ButtonOn(IDC_RES_INSERT);

          if (flags and ACF_HEX)<>0 then
            i:=sresHex
          else if (flags and ACF_STRUCT)<>0 then
            i:=sresStruct
          else if (flags and ACF_STRING)<>0 then
          begin
            i:=sresString;
            if (flags  and ACF_UNICODE )<>0 then ButtonOn(IDC_RES_UNICODE);
            if (flags2 and ACF2_FREEMEM)<>0 then ButtonOn(IDC_RES_FREEMEM);
          end
          else
          begin
            i:=sresInt;
            if (flags and ACF_SIGNED)<>0 then
              ButtonOn(IDC_RES_SIGNED);
          end;
          CB_SelectData(Dialog,IDC_SRV_RESULT,i);

          if (flags and ACF_WPARAM)<>0 then
          begin
            EnableWindow(GetDlgItem(Dialog,IDC_EDIT_WPAR),false);
            i:=ptParam;
          end
          else if (flags and ACF_WRESULT)<>0 then
          begin
            EnableWindow(GetDlgItem(Dialog,IDC_EDIT_WPAR),false);
            i:=ptResult;
          end
          else if (flags and ACF_WPARNUM)<>0 then
          begin
            if (flags and ACF_WCURRENT)<>0 then
            begin
              EnableWindow(GetDlgItem(Dialog,IDC_EDIT_WPAR),false);
              i:=ptCurrent
            end
            else
            begin
              i:=ptNumber;
              SetNumValue(GetDlgItem(Dialog,IDC_EDIT_WPAR),wparam,
                  (flags2 and ACF2_SRV_WPAR)<>0,
                  (flags2 and ACF2_SRV_WHEX)<>0);
//              SetDlgItemInt(Dialog,IDC_EDIT_WPAR,wparam,true)
            end;
          end
          else if (flags and ACF_WSTRUCT)<>0 then
          begin
            i:=ptStruct;
            SHControl(IDC_EDIT_WPAR,SW_HIDE);
            SHControl(IDC_WSTRUCT  ,SW_SHOW);
            mFreeMem(wstruct);
            StrDup(wstruct,PAnsiChar(wparam));
          end
          else if (flags and ACF_WUNICODE)<>0 then
          begin
            i:=ptUnicode;
            SetDlgItemTextW(Dialog,IDC_EDIT_WPAR,pWideChar(wparam));
          end
          else
          begin
            i:=ptString;
            SetDlgItemTextA(Dialog,IDC_EDIT_WPAR,PAnsiChar(wparam));
          end;
          CB_SelectData(GetDlgItem(Dialog,IDC_FLAG_WPAR),i);
          SendDlgItemMessage(Dialog,IDC_FLAG_WPAR,CB_SETCURSEL,i,0);

          if (flags and ACF_LPARAM)<>0 then
          begin
            EnableWindow(GetDlgItem(Dialog,IDC_EDIT_LPAR),false);
            i:=ptParam;
          end
          else if (flags and ACF_LRESULT)<>0 then
          begin
            EnableWindow(GetDlgItem(Dialog,IDC_EDIT_LPAR),false);
            i:=ptResult;
          end
          else if (flags and ACF_LPARNUM)<>0 then
          begin
            if (flags and ACF_LCURRENT)<>0 then
            begin
              EnableWindow(GetDlgItem(Dialog,IDC_EDIT_LPAR),false);
              i:=ptCurrent;
            end
            else
            begin
              i:=ptNumber;
              SetNumValue(GetDlgItem(Dialog,IDC_EDIT_LPAR),lparam,
                  (flags2 and ACF2_SRV_LPAR)<>0,
                  (flags2 and ACF2_SRV_LHEX)<>0);
//              SetDlgItemInt(Dialog,IDC_EDIT_LPAR,lparam,true)
            end;
          end
          else if (flags and ACF_LSTRUCT)<>0 then
          begin
            i:=ptStruct;
            SHControl(IDC_EDIT_LPAR,SW_HIDE);
            SHControl(IDC_LSTRUCT  ,SW_SHOW);
            mFreeMem(lstruct);
            StrDup(lstruct,PAnsiChar(lparam));
          end
          else if (flags and ACF_LUNICODE)<>0 then
          begin
            i:=ptUnicode;
            SetDlgItemTextW(Dialog,IDC_EDIT_LPAR,pWideChar(lparam));
          end
          else
          begin
            i:=ptString;
            SetDlgItemTextA(Dialog,IDC_EDIT_LPAR,PAnsiChar(lparam));
          end;
          CB_SelectData(GetDlgItem(Dialog,IDC_FLAG_LPAR),i);

        end;

        ACT_PROGRAM: begin
          if (flags2 and ACF2_PRG_PRG)<>0 then
            ButtonOn(IDC_PRG_PRG);
          if (flags2 and ACF2_PRG_ARG)<>0 then
            ButtonOn(IDC_PRG_ARG);

          SetDlgItemTextW(Dialog,IDC_EDIT_PRGPATH ,prgname);
          SetDlgItemTextW(Dialog,IDC_EDIT_PRGARGS ,args);
          SetDlgItemInt  (Dialog,IDC_EDIT_PROCTIME,time,false);
          case show of
            SW_HIDE         : ButtonOn(IDC_FLAG_HIDDEN);
            SW_SHOWMINIMIZED: ButtonOn(IDC_FLAG_MINIMIZE);
            SW_SHOWMAXIMIZED: ButtonOn(IDC_FLAG_MAXIMIZE);
          else
            {SW_SHOWNORMAL   :} ButtonOn(IDC_FLAG_NORMAL);
          end;
          if (flags and ACF_CURPATH)<>0 then
            ButtonOn(IDC_FLAG_CURPATH);
          if (flags and ACF_PRTHREAD)<>0 then
            ButtonOn(IDC_FLAG_PARALLEL)
          else
            ButtonOn(IDC_FLAG_CONTINUE);

        end;

        ACT_TEXT: begin
          if (flags and ACF_CLIPBRD)<>0 then
          begin
            ButtonOn(IDC_FLAG_CLIP);
            EnableWindow(GetDlgItem(Dialog,IDC_CLIP_COPYTO),true);
            EnableWindow(GetDlgItem(Dialog,IDC_CLIP_PASTE ),true);
            if (flags and ACF_COPYTO)<>0 then
              ButtonOn(IDC_CLIP_COPYTO)
            else
              ButtonOn(IDC_CLIP_PASTE);
// for switches
            ButtonOn(IDC_FILE_READ);
          end

          else
          begin
            if (flags and (ACF_FILE or ACF_FAPPEND or ACF_FWRITE))<>ACF_FILE then
              EnableWindow(GetDlgItem(Dialog,IDC_EDIT_INSERT),true);
            SetDlgItemTextW(Dialog,IDC_EDIT_INSERT,text);
// for switches
            ButtonOn(IDC_CLIP_COPYTO);

            if (flags2 and ACF2_TXT_TEXT)<>0 then
              ButtonOn(IDC_TXT_TEXT);

            if (flags and ACF_FILE)<>0 then
            begin
              if (flags2 and ACF2_TXT_FILE)<>0 then
                ButtonOn(IDC_TXT_FILE);
              ButtonOn(IDC_FLAG_FILE);
              EnableWindow(GetDlgItem(Dialog,IDC_FILE_PATH   ),true);
              EnableWindow(GetDlgItem(Dialog,IDC_FILE_FILEBTN),true);
              EnableWindow(GetDlgItem(Dialog,IDC_FILE_READ   ),true);
              EnableWindow(GetDlgItem(Dialog,IDC_FILE_WRITE  ),true);
              EnableWindow(GetDlgItem(Dialog,IDC_FILE_APPEND ),true);

              wnd:=GetDlgItem(Dialog,IDC_FILE_ENC);
              EnableWindow(wnd,true);
              if (flags and ACF_ANSI)<>0 then
                CB_SelectData(wnd,0)
              else if (flags and ACF_UTF8)<>0 then
              begin
                if (flags and ACF_SIGN)<>0 then
                  CB_SelectData(wnd,2)
                else
                  CB_SelectData(wnd,1);
              end
              else if (flags and ACF_SIGN)<>0 then
                CB_SelectData(wnd,4)
              else
                CB_SelectData(wnd,3);

              if      (flags and ACF_FAPPEND)<>0 then ButtonOn(IDC_FILE_APPEND)
              else if (flags and ACF_FWRITE )<>0 then ButtonOn(IDC_FILE_WRITE)
              else ButtonOn(IDC_FILE_READ);
              SetDlgItemTextW(Dialog,IDC_FILE_PATH,tfile);
            end
            else
            begin
              ButtonOn(IDC_FLAG_MESSAGE);
// for switches
              ButtonOn(IDC_FILE_READ);
            end;
          end;

        end;

        ACT_ADVANCE: begin
          FillSubList(Dialog);
//          SendDlgItemMessage(Dialog,IDC_ADV_VAL2,CB_SETCURSEL,0,0);

          case condition and not ADV_COND_NOT of
            ADV_COND_GT: ButtonOn(IDC_FLAG_GT);
            ADV_COND_LT: ButtonOn(IDC_FLAG_LT);
            ADV_COND_EQ: ButtonOn(IDC_FLAG_EQ);
          else // ADV_COND_NOP
            ButtonOn(IDC_FLAG_NOP);
          end;
          SetDlgItemInt(Dialog,IDC_ADV_VALUE,value,false);
          if (condition and ADV_COND_NOT)<>0 then
            ButtonOn(IDC_FLAG_NOT);

          SetDlgItemInt(Dialog,IDC_ADV_VAL1,mathval,true);
          case action and ADV_ACTION of
            ADV_ACT_MATH: begin
              ButtonOn(IDC_FLAG_MATH);
              CB_SelectData(GetDlgItem(Dialog,IDC_ADV_OPER),oper);
//              SendDlgItemMessage(Dialog,IDC_ADV_OPER,CB_SETCURSEL,oper,0);
              EnableWindow(GetDlgItem(Dialog,IDC_ADV_VAL1),true);
              EnableWindow(GetDlgItem(Dialog,IDC_ADV_OPER),true);
            end;
            ADV_ACT_VARS: begin
              ButtonOn(IDC_FLAG_VARS);
              if (flags and ACF_VARASINT)<>0 then
                ButtonOn(IDC_ADV_ASINT);
              EnableWindow(GetDlgItem(Dialog,IDC_ADV_VARS),true);
              SetDlgItemTextW(Dialog,IDC_ADV_VARS,varval);
            end;
          end;
          bb:=false;
          case action and ADV_ACT_POST of
            ADV_ACT_BREAK: ButtonOn(IDC_FLAG_BREAK);
            ADV_ACT_JUMP : begin
              SendDlgItemMessageW(Dialog,IDC_ADV_VAL2,CB_SELECTSTRING,twparam(-1),tlparam(operval));
              ButtonOn(IDC_FLAG_JUMP);
              bb:=true;
            end;
          else // ADV_ACT_NOP
            ButtonOn(IDC_FLAG_ANOP);
          end;
          EnableWindow(GetDlgItem(Dialog,IDC_ADV_VAL2),bb);
        end;

        ACT_CHAIN: begin
//          FillChainList(Dialog);
          if (flags and ACF_BYNAME)<>0 then
            SendDlgItemMessageW(Dialog,IDC_GROUP_LIST,CB_SELECTSTRING,twparam(-1),tlparam(actname))
          else
            SendDlgItemMessageW(Dialog,IDC_GROUP_LIST,CB_SELECTSTRING,twparam(-1),tlparam(GetGroupName(id)));
        end;

        ACT_RW: begin
          if      (flags and ACF_DBDELETE)<>0 then ButtonOn(IDC_RW_DELETE)
          else if (flags and ACF_DBWRITE )= 0 then ButtonOn(IDC_RW_READ)
          else                                     ButtonOn(IDC_RW_WRITE);

          bb:=false;
          if (flags and ACF_CURRENT)<>0 then
          begin
            ButtonOn(IDC_RW_CURRENT);
            EnableWindow(GetDlgItem(Dialog,IDC_CONTACTLIST),false);
          end
          else if (flags and ACF_PARAM)<>0 then
          begin
            ButtonOn(IDC_RW_PARAM);
            EnableWindow(GetDlgItem(Dialog,IDC_CONTACTLIST),false);
          end
          else if (flags and ACF_RESULT)<>0 then
          begin
            ButtonOn(IDC_RW_RESULT);
            EnableWindow(GetDlgItem(Dialog,IDC_CONTACTLIST),false);
          end
          else
          begin
            ButtonOn(IDC_RW_MANUAL);
            bb:=true;
            EnableWindow(GetDlgItem(Dialog,IDC_CONTACTLIST),true);
            SendDlgItemMessage(Dialog,IDC_CONTACTLIST,CB_SETCURSEL,
              FindContact(GetDlgItem(Dialog,IDC_CONTACTLIST),dbcontact),0);
          end;
          EnableWindow(GetDlgItem(Dialog,IDC_CONTACTLIST),bb);

          if (flags and ACF_LAST)<>0 then
          begin
            ButtonOn(IDC_RW_LAST);
            bb:=false;
          end
          else
            bb:=true;
          if (flags and ACF_DBDELETE)<>0 then
            bb:=false;

          EnableWindow(GetDlgItem(Dialog,IDC_RW_VALUE),bb);
          EnableWindow(GetDlgItem(Dialog,IDC_RW_TEXT ),bb);

          SetDlgItemTextA(Dialog,IDC_RW_MODULE ,dbmodule);
          SetDlgItemTextA(Dialog,IDC_RW_SETTING,dbsetting);
          if (flags2 and ACF2_RW_MVAR)<>0 then ButtonOn(IDC_RW_MVAR);
          if (flags2 and ACF2_RW_SVAR)<>0 then ButtonOn(IDC_RW_SVAR);
          if (flags2 and ACF2_RW_TVAR)<>0 then ButtonOn(IDC_RW_TVAR);


          wnd:=GetDlgItem(Dialog,IDC_RW_DATATYPE);
          if ((flags and ACF_DBANSI )=ACF_DBANSI) or
             ((flags and ACF_DBUTEXT)=ACF_DBUTEXT) then
          begin
            SHControl(IDC_RW_TEXT ,SW_SHOW);
            SHControl(IDC_RW_VALUE,SW_HIDE);
            if (flags and ACF_DBANSI)=ACF_DBANSI then
              CB_SelectData(wnd,3)
            else
              CB_SelectData(wnd,4);
            SetDlgItemTextW(Dialog,IDC_RW_TEXT,pWideChar(dbvalue));
          end
          else
          begin
            if (flags and ACF_DBBYTE)=ACF_DBBYTE then
              CB_SelectData(wnd,0)
            else if (flags and ACF_DBWORD)=ACF_DBWORD then
              CB_SelectData(wnd,1)
            else
              CB_SelectData(wnd,2);
            SHControl(IDC_RW_TEXT ,SW_HIDE);
            SHControl(IDC_RW_VALUE,SW_SHOW);

            SetNumValue(GetDlgItem(Dialog,IDC_RW_VALUE),dbvalue,
                (flags2 and ACF2_RW_TVAR)<>0,
                (flags2 and ACF2_RW_HEX )<>0);
          end;

        end;

        ACT_MESSAGE: begin
          SetDlgItemTextW(Dialog,IDC_MSG_TITLE,msgtitle);
          SetDlgItemTextW(Dialog,IDC_MSG_TEXT ,msgtext);
          if (flags2 and ACF2_MSG_TTL)<>0 then ButtonOn(IDC_MSG_TTL);
          if (flags2 and ACF2_MSG_TXT)<>0 then ButtonOn(IDC_MSG_TXT);
          if (flags  and ACF_MSG_KEEP)<>0 then ButtonOn(IDC_MSG_KEEP);

          case boxopts and $0F of
            1: ButtonOn(IDC_MSGB_OC);
            2: ButtonOn(IDC_MSGB_ARI);
            3: ButtonOn(IDC_MSGB_YNC);
            4: ButtonOn(IDC_MSGB_YN);
            5: ButtonOn(IDC_MSGB_RC);
          else
            ButtonOn(IDC_MSGB_OK);
          end;
          case boxopts and $F0 of
            $10: ButtonOn(IDC_MSGI_ERROR);
            $20: ButtonOn(IDC_MSGI_QUEST);
            $30: ButtonOn(IDC_MSGI_WARN);
            $40: ButtonOn(IDC_MSGI_INFO);
          else
            ButtonOn(IDC_MSGI_NONE);
          end;
        end;
      end;
      SHWindows(actionType);
    end;
  end;
