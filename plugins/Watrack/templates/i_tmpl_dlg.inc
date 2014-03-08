{}
function DlgProcOptions(Dialog:HWND;hMessage:uint;wParam:WPARAM;lParam:LPARAM):LRESULT; stdcall;
begin
  result:=0;
  case hMessage of
    WM_INITDIALOG: begin
      TranslateDialogDefault(Dialog);

      if not isVarsInstalled then
        ShowWindow(GetDlgItem(Dialog,IDC_VAR_HELP),SW_HIDE)
      else
        SendDlgItemMessage(Dialog,IDC_VAR_HELP,BM_SETIMAGE,IMAGE_ICON,
          CallService(MS_VARS_GETSKINITEM,0,VSI_HELPICON));

      SendDlgItemMessage(Dialog,IDC_MACRO_HELP,BM_SETIMAGE,IMAGE_ICON,
          CallService(MS_SKIN_LOADICON,SKINICON_OTHER_HELP,0));

      MakeHint(Dialog,IDC_REPLACESPC,
        'Replaces "_" (underscores) globally in pasted os status text,'+
        ' sometimes may be useful');
      CheckDlgButton(Dialog,IDC_REPLACESPC,ReplaceSpc);

      CheckDlgButton(Dialog,IDC_LOCASE,LoCaseType);
      
      CheckDlgButton(Dialog,IDC_FSIZEBYTE,ord(FSizeMode=1));
      CheckDlgButton(Dialog,IDC_FSIZEKILO,ord(FSizeMode=1024));
      CheckDlgButton(Dialog,IDC_FSIZEMEGA,ord(FSizeMode=1024*1024));
      
      SetDlgItemInt (Dialog,IDC_PRECISION,FSPrecision,false);
      CheckDlgButton(Dialog,IDC_POSTNONE ,ord(FSizePost=0));
      CheckDlgButton(Dialog,IDC_POSTSMALL,ord(FSizePost=1));
      CheckDlgButton(Dialog,IDC_POSTMIX  ,ord(FSizePost=2));
      CheckDlgButton(Dialog,IDC_POSTLARGE,ord(FSizePost=3));

      CheckDlgButton(Dialog,IDC_ALLCAP  ,ord(PlayerCaps=2));
      CheckDlgButton(Dialog,IDC_SMALLCAP,ord(PlayerCaps=1));
      CheckDlgButton(Dialog,IDC_MIXCAP  ,ord(PlayerCaps=0));

      CheckDlgButton(Dialog,IDC_WRITECBR1,ord(WriteCBR=0));
      CheckDlgButton(Dialog,IDC_WRITECBR2,ord(WriteCBR<>0));

      SetDlgItemTextW(Dialog,IDC_EXPORT_TEXT,ExportText);
    end;

    WM_COMMAND: begin
      if (wParam shr 16)=BN_CLICKED then
      begin
        case loword(wParam) of
          IDC_VAR_HELP  : ShowVarHelp     (Dialog);
          IDC_MACRO_HELP: CallService(MS_WAT_MACROHELP,Dialog,0);
          IDC_ALLCAP: begin
            CheckDlgButton(Dialog,IDC_ALLCAP  ,BST_CHECKED);
            CheckDlgButton(Dialog,IDC_SMALLCAP,BST_UNCHECKED);
            CheckDlgButton(Dialog,IDC_MIXCAP  ,BST_UNCHECKED);
          end;
          IDC_SMALLCAP: begin
            CheckDlgButton(Dialog,IDC_ALLCAP  ,BST_UNCHECKED);
            CheckDlgButton(Dialog,IDC_SMALLCAP,BST_CHECKED);
            CheckDlgButton(Dialog,IDC_MIXCAP  ,BST_UNCHECKED);
          end;
          IDC_MIXCAP: begin
            CheckDlgButton(Dialog,IDC_ALLCAP  ,BST_UNCHECKED);
            CheckDlgButton(Dialog,IDC_SMALLCAP,BST_UNCHECKED);
            CheckDlgButton(Dialog,IDC_MIXCAP  ,BST_CHECKED);
          end;
        end;
      end;
      if ((wParam shr 16)=EN_CHANGE) or ((wParam shr 16)=BN_CLICKED) then
        SendMessage(GetParent(Dialog),PSM_CHANGED,0,0);
      result:=1;
    end;

    WM_NOTIFY: begin
      if integer(PNMHdr(lParam)^.code)=PSN_APPLY then
      begin
        ReplaceSpc:=IsDlgButtonChecked(Dialog,IDC_REPLACESPC);
        LoCaseType:=IsDlgButtonChecked(Dialog,IDC_LOCASE);
        if IsDlgButtonChecked(Dialog,IDC_WRITECBR1)=BST_CHECKED then
          WriteCBR:=0
        else //if IsDlgButtonChecked(Dialog,IDC_WRITECBR2)=BST_CHECKED then
          WriteCBR:=1;
        if IsDlgButtonChecked(Dialog,IDC_FSIZEBYTE)=BST_CHECKED then
          FSizeMode:=1
        else if IsDlgButtonChecked(Dialog,IDC_FSIZEKILO)=BST_CHECKED then
          FSizeMode:=1024
        else// if IsDlgButtonChecked(Dialog,IDC_FSIZEMEGA)=BST_CHECKED then
          FSizeMode:=1024*1024;

        if IsDlgButtonChecked(Dialog,IDC_MIXCAP)=BST_CHECKED then
          PlayerCaps:=0
        else if IsDlgButtonChecked(Dialog,IDC_SMALLCAP)=BST_CHECKED then
          PlayerCaps:=1
        else// if IsDlgButtonChecked(Dialog,IDC_ALLCAP)=BST_CHECKED then
          PlayerCaps:=2;

        if IsDlgButtonChecked(Dialog,IDC_POSTNONE)=BST_CHECKED then
          FSizePost:=0
        else if IsDlgButtonChecked(Dialog,IDC_POSTSMALL)=BST_CHECKED then
          FSizePost:=1
        else if IsDlgButtonChecked(Dialog,IDC_POSTMIX)=BST_CHECKED then
          FSizePost:=2
        else// if IsDlgButtonChecked(Dialog,IDC_POSTLARGE)=BST_CHECKED then
          FSizePost:=3;
        FSPrecision:=GetDlgItemInt(Dialog,IDC_PRECISION,pbool(nil)^,false);
        if FSPrecision>3 then
          FSPrecision:=3;

        mFreeMem(ExportText);
        ExportText:=GetDlgText(Dialog,IDC_EXPORT_TEXT);

        SaveOpt;
      end;
    end;
  else
    {result:=}DefWindowProc(Dialog,hMessage,wParam,lParam);
  end;
end;
