{format specific}

function DlgProcOptions3(Dialog:HWND;hMessage:uint;wParam:WPARAM;lParam:LPARAM):LRESULT; stdcall;
const
  dlginit:boolean=false;
var
  wnd:HWND;
  b:boolean;
begin
  result:=0;
  case hMessage of
    WM_INITDIALOG: begin
      dlginit:=false;
      TranslateDialogDefault(Dialog);

      MakeHint(Dialog,IDC_SIMPLEMODE,
        'If this option is "ON", one template will be used with all '+
        'protocols, protocol and player (media) statuses. Template option page will be '+
        'changed next time.');
      CheckDlgButton(Dialog,IDC_SIMPLEMODE,SimpleMode);
      MakeHint(Dialog,IDC_INDEPENDED,
        'If this option is "ON", xStatus doesn''t depend on protocol status.');
      CheckDlgButton(Dialog,IDC_INDEPENDED,XIndepended);
      MakeHint(Dialog,IDC_USESTATUS,
        'If this option is "ON", status text will be replaced by music info.');
      CheckDlgButton(Dialog,IDC_USESTATUS,UseStatus);
      MakeHint(Dialog,IDC_USEMSGS,
        'If this option is "ON", you can paste music info to your '+
        'message window pressing hotkey.');
      CheckDlgButton(Dialog,IDC_USEMSGS,UseMessages);
      MakeHint(Dialog,IDC_KEEPSTATUS,
        'If this option is "ON", xStatus won''t be changed when player is shut down.');
      CheckDlgButton(Dialog,IDC_KEEPSTATUS,KeepStatus);
      MakeHint(Dialog,IDC_CLEARXSTAT,
        'xStatus will be cleared before text changing and restored with new text later.');
      CheckDlgButton(Dialog,IDC_CLEARXSTAT,ClearXStat);
      MakeHint(Dialog,IDC_EXTSTATUS,
        'If this option is "ON", xStatus will be changed to "Music" and '+
        'status text will be replaced by music info.');
      CheckDlgButton(Dialog,IDC_EXTSTATUS ,UseExtStatus);
      MakeHint(Dialog,IDC_LISTENINGTO,
        'If this option is "ON", "Listening To" protocol property will be filled '+
        'by music info.');
      CheckDlgButton(Dialog,IDC_LISTENINGTO,UseListeningTo);

      wnd:=GetDlgItem(Dialog,IDC_SETXSTATUS);
//      SendMessage(wnd,CB_RESETCONTENT,0,0);
      CB_AddStrDataW(wnd,TranslateW('any xStatus is set'           ),0,0);
      CB_AddStrDataW(wnd,TranslateW('''Music'' status is set'      ),1,1);
      CB_AddStrDataW(wnd,TranslateW('xStatus is empty or ''Music'''),2,2);
      CB_SelectData(wnd,XStatusSet);

      SendMessage(Dialog,WM_COMMAND,(BN_CLICKED shl 16)+IDC_EXTSTATUS,
          GetDlgItem(Dialog,IDC_EXTSTATUS));

      dlginit:=true;
      result:=0;
    end;

    WM_COMMAND: begin
      if ((wParam shr 16)=BN_CLICKED) and (LoWord(wParam)=IDC_EXTSTATUS) then
      begin
        b:=IsDlgButtonchecked(Dialog,IDC_EXTSTATUS)<>BST_UNCHECKED;
        EnableWindow(GetDlgItem(Dialog,IDC_INDEPENDED),b);
        EnableWindow(GetDlgItem(Dialog,IDC_CLEARXSTAT),b);
//        EnableWindow(GetDlgItem(Dialog,IDC_OLDXSTATUS),b);
//        EnableWindow(GetDlgItem(Dialog,IDC_ONLYMUSIC ),b);
        EnableWindow(GetDlgItem(Dialog,IDC_SETXSTATUS),b);
        EnableWindow(GetDlgItem(Dialog,IDC_KEEPSTATUS),b);
      end;

      case wParam shr 16 of
        CBN_SELCHANGE,
        EN_CHANGE,
        BN_CLICKED: SendMessage(GetParent(Dialog),PSM_CHANGED,0,0);
      end;
      result:=1;
    end;

    WM_NOTIFY: begin
      if dlginit then
      begin
        case integer(PNMHdr(lParam)^.code) of
          LVN_ITEMCHANGED: begin
            SendMessage(GetParent(Dialog),PSM_CHANGED,0,0);
          end;

          PSN_APPLY: begin
            SimpleMode    :=IsDlgButtonChecked(Dialog,IDC_SIMPLEMODE);
            XIndepended   :=IsDlgButtonChecked(Dialog,IDC_INDEPENDED);
            UseMessages   :=IsDlgButtonChecked(Dialog,IDC_USEMSGS);
            UseStatus     :=IsDlgButtonChecked(Dialog,IDC_USESTATUS);
            UseExtStatus  :=IsDlgButtonChecked(Dialog,IDC_EXTSTATUS);
            KeepStatus    :=IsDlgButtonChecked(Dialog,IDC_KEEPSTATUS);
            ClearXStat    :=IsDlgButtonChecked(Dialog,IDC_CLEARXSTAT);
            UseListeningTo:=IsDlgButtonChecked(Dialog,IDC_LISTENINGTO);
            XStatusSet :=CB_GetData(GetDlgItem(Dialog,IDC_SETXSTATUS));
            SaveOpt;
          end;
        end;
      end;
    end;
  else
    {result:=}DefWindowProc(Dialog,hMessage,wParam,lParam);
  end;
end;
