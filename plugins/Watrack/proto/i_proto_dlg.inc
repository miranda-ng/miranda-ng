{Misc}

procedure SetAllContactStat(hwndList:HWND);
var
  hContact:TMCONTACT;
  hItem:THANDLE;
begin
  hContact:=db_find_first();
  while hContact<>0 do
  begin
    hItem:=SendMessage(hwndList,CLM_FINDCONTACT,hContact,0);
    if hItem<>0 then
    begin
      SendMessage(hwndList,CLM_SETCHECKMARK,hItem,
        DBReadByte(hContact,strCList,ShareOptText,0));
    end;
    hContact:=db_find_next(hContact);
  end;
end;

procedure SaveAllContactStat(hwndList:HWND);
var
  hContact:TMCONTACT;
  hItem:THANDLE;
begin
  hContact:=db_find_first();
  while hContact<>0 do
  begin
    hItem:=SendMessage(hwndList,CLM_FINDCONTACT,hContact,0);
    if hItem<>0 then
    begin
      if SendMessage(hwndList,CLM_GETCHECKMARK,hItem,0)<>0 then
        DBWriteByte(hContact,strCList,ShareOptText,1)
      else
        DBDeleteSetting(hContact,strCList,ShareOptText);
    end;
    hContact:=db_find_next(hContact);
  end;
end;

procedure SetHistMask(Dlg:HWND);
begin
  CheckDlgButton(Dlg,IDC_IN_REQUEST ,ORD((HistMask and hmInRequest )<>0));
  CheckDlgButton(Dlg,IDC_OUT_REQUEST,ORD((HistMask and hmOutRequest)<>0));
  CheckDlgButton(Dlg,IDC_IN_INFO    ,ORD((HistMask and hmInInfo    )<>0));
  CheckDlgButton(Dlg,IDC_OUT_INFO   ,ORD((HistMask and hmOutInfo   )<>0));
  CheckDlgButton(Dlg,IDC_IN_ERROR   ,ORD((HistMask and hmInError   )<>0));
  CheckDlgButton(Dlg,IDC_OUT_ERROR  ,ORD((HistMask and hmOutError  )<>0));
  CheckDlgButton(Dlg,IDC_IREQUEST   ,ORD((HistMask and hmIRequest  )<>0));
  CheckDlgButton(Dlg,IDC_ISEND      ,ORD((HistMask and hmISend     )<>0));
end;

procedure SaveHistMask(Dlg:HWND);
begin
  HistMask:=0;
  if IsDlgButtonChecked(Dlg,IDC_IN_REQUEST )<>BST_UNCHECKED then HistMask:=HistMask or hmInRequest;
  if IsDlgButtonChecked(Dlg,IDC_OUT_REQUEST)<>BST_UNCHECKED then HistMask:=HistMask or hmOutRequest;
  if IsDlgButtonChecked(Dlg,IDC_IN_INFO    )<>BST_UNCHECKED then HistMask:=HistMask or hmInInfo;
  if IsDlgButtonChecked(Dlg,IDC_OUT_INFO   )<>BST_UNCHECKED then HistMask:=HistMask or hmOutInfo;
  if IsDlgButtonChecked(Dlg,IDC_IN_ERROR   )<>BST_UNCHECKED then HistMask:=HistMask or hmInError;
  if IsDlgButtonChecked(Dlg,IDC_OUT_ERROR  )<>BST_UNCHECKED then HistMask:=HistMask or hmOutError;
  if IsDlgButtonChecked(Dlg,IDC_IREQUEST   )<>BST_UNCHECKED then HistMask:=HistMask or hmIRequest;
  if IsDlgButtonChecked(Dlg,IDC_ISEND      )<>BST_UNCHECKED then HistMask:=HistMask or hmISend;
end;

function DlgProcOptions(Dialog:HWND;hMessage:uint;wParam:WPARAM;lParam:LPARAM):LRESULT; stdcall;
const
  Changed:integer=0;
//  hItemAll:THANDLE=0;
var
//  cii:TCLCINFOITEM;
  hList:HWND;
begin
  result:=0;
  case hMessage of
    WM_INITDIALOG: begin
      TranslateDialogDefault(Dialog);
      Changed:=DLGED_INIT;

      hList:=GetDlgItem(Dialog,IDC_SHARE);
      SetWindowLongPtr(hList,GWL_STYLE,GetWindowLongPtr(hList,GWL_STYLE) or CLS_SHOWHIDDEN);
      SendMessage(hList,CLM_SETUSEGROUPS      ,1,0);
      SendMessage(hList,CLM_SETHIDEEMPTYGROUPS,1,0);

//      SendMessage(hList,CLM_SETEXTRACOLUMNS,2,0);
{
      FillChar(cii,SizeOf(cii),0);
      cii.cbSize   :=SizeOf(cii);
      cii.flags    :=CLCIIF_GROUPFONT or CLCIIF_CHECKBOX;
      cii.pszText.w:=TranslateW('** All contacts **');
      hItemAll:=SendMessage(hList,CLM_ADDINFOITEM,0,tlparam(@cii));
}
      SetAllContactStat(hList);
      SetHistMask(Dialog);

      SetDlgItemTextW(Dialog,IDC_PROTO_TEXT,ProtoText);

      Changed:=0;
    end;

    WM_COMMAND: begin
      if Changed<>DLGED_INIT then
      begin
        case wParam shr 16 of
          BN_CLICKED,EN_CHANGE: SendMessage(GetParent(Dialog),PSM_CHANGED,0,0);
        end;
      end;
    end;

    WM_NOTIFY: begin
      if Changed<>DLGED_INIT then
      begin
        if PNMHDR(lParam)^.idFrom=IDC_SHARE then
          if integer(PNMHdr(lParam)^.code)=CLN_CHECKCHANGED then
            SendMessage(GetParent(Dialog),PSM_CHANGED,0,0);

        if integer(PNMHdr(lParam)^.code)=PSN_APPLY then
        begin
          mFreeMem(ProtoText);
          ProtoText:=GetDlgText(Dialog,IDC_PROTO_TEXT);

          SaveAllContactStat(GetDlgItem(Dialog,IDC_SHARE));
          SaveHistMask(Dialog);

          WriteOptions;
        end;
      end;
    end;
  else
    {result:=}DefWindowProc(Dialog,hMessage,wParam,lParam);
  end;
end;
