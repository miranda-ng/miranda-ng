{Popup Option Dialog}

// Popup options
const
  DLGPOPUP = 'POPUP';

function DlgPopupOpt(Dialog:HWND;hMessage:Uint;wParam:WPARAM;lParam:LPARAM):LRESULT; stdcall;
const
  dlginit:boolean=false;
var
  tmp:longbool;
  ppd:PPOPUPDATAW;
  fore,back:HWND;
begin
  result:=0;
  case hMessage of
    WM_INITDIALOG: begin
      TranslateDialogDefault(Dialog);
      dlginit:=false;

      SetDlgItemTextW(Dialog,IDC_POPUP_TITLE,PopTitle);
      SetDlgItemTextW(Dialog,IDC_POPUP_TEXT ,PopText);
      
      CheckDlgButton(Dialog,IDC_SHOWFILE ,PopupFile);
      CheckDlgButton(Dialog,IDC_REQUEST  ,PopRequest);

      CheckDlgButton(Dialog,IDC_ACTLEFTCLOSE  ,ord(LoByte(PopupAction)=0));
      CheckDlgButton(Dialog,IDC_ACTLEFTINFO   ,ord(LoByte(PopupAction)=1));
      CheckDlgButton(Dialog,IDC_ACTLEFTPLAYER ,ord(LoByte(PopupAction)=2));
      CheckDlgButton(Dialog,IDC_ACTLEFTNEXT   ,ord(LoByte(PopupAction)=3));
      CheckDlgButton(Dialog,IDC_ACTRIGHTCLOSE ,ord(HiByte(PopupAction)=0));
      CheckDlgButton(Dialog,IDC_ACTRIGHTINFO  ,ord(HiByte(PopupAction)=1));
      CheckDlgButton(Dialog,IDC_ACTRIGHTPLAYER,ord(HiByte(PopupAction)=2));
      CheckDlgButton(Dialog,IDC_ACTRIGHTNEXT  ,ord(HiByte(PopupAction)=3));

      CheckDlgButton(Dialog,IDC_USEBUTTONS,PopupButtons);

      SetDlgItemInt (Dialog,IDC_DELAY,PopupPause,false);

      if PopupDelay<0 then
        CheckDlgButton(Dialog,IDC_DELAYPERM,BST_CHECKED)
      else if PopupDelay=0 then
        CheckDlgButton(Dialog,IDC_DELAYDEF,BST_CHECKED)
      else
        CheckDlgButton(Dialog,IDC_DELAYCUST,BST_CHECKED);
      if PopupDelay<=0 then
        EnableWindow(GetDlgItem(Dialog,IDC_DELAY),false);

      SendDlgItemMessage(Dialog,IDC_MACRO_HELP,BM_SETIMAGE,IMAGE_ICON,
          CallService(MS_SKIN_LOADICON,SKINICON_OTHER_HELP,0));

      fore:=GetDlgItem(Dialog,IDC_FORE);
      back:=GetDlgItem(Dialog,IDC_BACK);
      SendMessage(fore,CPM_SETCOLOUR,0,PopupFore);
      SendMessage(fore,CPM_SETDEFAULTCOLOUR,0,GetSysColor(COLOR_BTNTEXT));
      SendMessage(back,CPM_SETCOLOUR,0,PopupBack);
      SendMessage(back,CPM_SETDEFAULTCOLOUR,0,GetSysColor(COLOR_BTNFACE));
      SetDlgItemInt(Dialog,IDC_FORE,PopupFore,false);
      SetDlgItemInt(Dialog,IDC_BACK,PopupBack,false);
      if PopupColor<2 then
      begin
        EnableWindow(fore,false);
        EnableWindow(back,false);
      end;
      case PopupColor of
        0: CheckDlgButton(Dialog,IDC_COLORDEF ,BST_CHECKED);
        1: CheckDlgButton(Dialog,IDC_COLORWIN ,BST_CHECKED);
        2: CheckDlgButton(Dialog,IDC_COLORCUST,BST_CHECKED);
      end;
      dlginit:=true;
    end;

    WM_COMMAND: begin
      if (wParam shr 16)=BN_CLICKED then
      begin
        fore:=GetDlgItem(Dialog,IDC_FORE);
        back:=GetDlgItem(Dialog,IDC_BACK);
        case LoWord(wParam) of
          IDC_MACRO_HELP: CallService(MS_WAT_MACROHELP,Dialog,0);
          IDC_TEST: begin
            mGetMem(ppd,SizeOf(TPOPUPDATAW));
            FillChar(ppd^,SizeOf(ppd^),0);
            ppd^.lchIcon:=LoadSkinnedIcon(SKINICON_OTHER_MIRANDA);
            StrCopyW(ppd^.lpwzContactName,TranslateW('popup test'));
            StrCopyW(ppd^.lpwzText,TranslateW('second line'));
            if IsDlgButtonChecked(Dialog,IDC_COLORDEF)=BST_CHECKED then
            begin
              ppd^.colorBack:=0;
              ppd^.colorText:=0;
            end
            else if IsDlgButtonChecked(Dialog,IDC_COLORWIN)=BST_CHECKED then
            begin
              ppd^.colorBack:=GetSysColor(COLOR_BTNFACE);
              ppd^.colorText:=GetSysColor(COLOR_BTNTEXT);
            end
            else
            begin
              ppd^.colorBack:=SendMessage(back,CPM_GETCOLOUR,0,0);
              ppd^.colorText:=SendMessage(fore,CPM_GETCOLOUR,0,0);
            end;
            CallService(MS_POPUP_ADDPOPUPW,twparam(ppd),0);
            mFreeMem(ppd);
          end;
          IDC_DELAYCUST:
            EnableWindow(GetDlgItem(Dialog,IDC_DELAY),true);
          IDC_DELAYDEF,IDC_DELAYPERM:
            EnableWindow(GetDlgItem(Dialog,IDC_DELAY),false);
          IDC_COLORCUST: begin
            EnableWindow(fore,true);
            EnableWindow(back,true);
          end;
          IDC_COLORDEF,IDC_COLORWIN: begin
            EnableWindow(fore,false);
            EnableWindow(back,false);
          end;
        end;
      end;
      SendMessage(GetParent(Dialog),PSM_CHANGED,0,0);
      result:=0;
    end;

    WM_NOTIFY: begin
      if dlginit then
      begin
        if integer(PNMHdr(lParam)^.code)=PSN_APPLY then
        begin

          mFreeMem(PopTitle);
          mFreeMem(PopText);
          PopTitle:=GetDlgText(Dialog,IDC_POPUP_TITLE);
          PopText :=GetDlgText(Dialog,IDC_POPUP_TEXT);
          
          PopupButtons:=IsDlgButtonChecked(Dialog,IDC_USEBUTTONS);

          PopupFile :=IsDlgButtonChecked(Dialog,IDC_SHOWFILE);
          PopRequest:=IsDlgButtonChecked(Dialog,IDC_REQUEST);
//color
          if IsDlgButtonChecked(Dialog,IDC_COLORDEF)=BST_CHECKED then
            PopupColor:=0
          else if IsDlgButtonChecked(Dialog,IDC_COLORWIN)=BST_CHECKED then
            PopupColor:=1
          else
          begin
            PopupColor:=2;
            PopupFore:=SendDlgItemMessage(Dialog,IDC_FORE,CPM_GETCOLOUR,0,0);
            PopupBack:=SendDlgItemMessage(Dialog,IDC_BACK,CPM_GETCOLOUR,0,0);
          end;
//pause
          if IsDlgButtonChecked(Dialog,IDC_DELAYDEF)=BST_CHECKED then
            PopupDelay:=0
          else if IsDlgButtonChecked(Dialog,IDC_DELAYPERM)=BST_CHECKED then
            PopupDelay:=-1
          else
          begin
            PopupDelay:=1;
            PopupPause:=GetDlgItemInt(Dialog,IDC_DELAY,tmp,false);
          end;
//action
          if IsDlgButtonChecked(Dialog,IDC_ACTLEFTINFO)=BST_CHECKED then
            PopupAction:=1
          else if IsDlgButtonChecked(Dialog,IDC_ACTLEFTPLAYER)=BST_CHECKED then
            PopupAction:=2
          else if IsDlgButtonChecked(Dialog,IDC_ACTLEFTNEXT)=BST_CHECKED then
            PopupAction:=3
          else
            PopupAction:=0;
          if IsDlgButtonChecked(Dialog,IDC_ACTRIGHTINFO)=BST_CHECKED then
            inc(PopupAction,$100)
          else if IsDlgButtonChecked(Dialog,IDC_ACTRIGHTPLAYER)=BST_CHECKED then
            inc(PopupAction,$200)
          else if IsDlgButtonChecked(Dialog,IDC_ACTRIGHTNEXT)=BST_CHECKED then
            inc(PopupAction,$300);

          savepopup;
        end;
      end;
    end;
  end;
end;
