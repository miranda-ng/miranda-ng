{Macro help dialog}

procedure SaveAliases;
var
  buf:array [0..31] of AnsiChar;
  i:integer;
  p:PAnsiChar;
begin
  p:=StrCopyE(buf,'alias/');
  for i:=0 to numvars-1 do
  begin
//    if vars[i].alias<>nil then
      DBWriteUnicode(0,PluginShort,IntToStr(p,i),vars[i].alias);
  end;
end;

procedure LoadAliases;
var
  buf:array [0..31] of AnsiChar;
  i:integer;
  p:PAnsiChar;
begin
  p:=StrCopyE(buf,'alias/');
  for i:=0 to numvars-1 do
    vars[i].alias:=DBReadUnicode(0,PluginShort,IntToStr(p,i),nil);
end;

procedure FreeAliases;
var
  i:integer;
begin
  for i:=0 to numvars-1 do
    mFreeMem(vars[i].alias);
end;

function MacroHelpDlg(Dialog:HWND;hMessage:uint;wParam:WPARAM;lParam:LPARAM):LRESULT; stdcall;
const
  Changed:bool=false;
var
  i:integer;
  itemw:LV_ITEMW;
  lvc:LV_COLUMN;
  wnd:HWND;
  ws:PWideChar;
  s:pAnsiChar;
  rc:TRECT;
begin
  result:=0;
  case hMessage of

    WM_INITDIALOG: begin
      FillChar(itemw,SizeOf(itemw),0);
      FillChar(lvc  ,SizeOf(lvc)  ,0);
      wnd:=GetDlgItem(Dialog,IDC_MACROHELP);
      SendMessage(wnd,LVM_SETUNICODEFORMAT,1,0);
      lvc.mask:=LVCF_FMT;
      lvc.fmt :=LVCFMT_LEFT;
      ListView_InsertColumn(wnd,0,lvc);
      itemw.mask:=LVIF_TEXT;
      for i:=0 to numvars-1 do
      begin
        itemw.iItem:=i;
        ws:=vars[i].alias;
        if ws=nil then
          ws:=vars[i].name;
        itemw.pszText:=ws;
        SendMessageW(wnd,LVM_INSERTITEMW,0,tlparam(@itemw));
      end;
      ListView_SetColumnWidth(wnd,0,LVSCW_AUTOSIZE);
      ListView_InsertColumn(wnd,1,lvc);
      itemw.iSubItem:=1;
      s:=nil;
      for i:=0 to numvars-1 do
      begin
        itemw.iItem:=i;
        if vars[i].help<>nil then
          s:=vars[i].help;
        itemw.pszText:=TranslateA2W(s);
        SendMessageW(wnd,LVM_SETITEMTEXTW,i,tlparam(@itemw));
        mFreeMem(itemw.pszText);
      end;
      ListView_SetColumnWidth(wnd,1,LVSCW_AUTOSIZE);
      result:=1;
      Changed:=false;
      TranslateDialogDefault(Dialog);
    end;

    WM_SIZE: begin
      GetClientRect(Dialog,rc);
      InflateRect(rc,-8,-8);
      MoveWindow(GetDlgItem(Dialog,IDC_MACROHELP),
          rc.left,rc.top,rc.right-rc.left,rc.bottom-rc.top,true);
    end;

    WM_COMMAND: begin
      if (wParam shr 16)=BN_CLICKED then
        case loword(wParam) of
          IDOK, IDCANCEL: DestroyWindow(Dialog);
        end;
    end;

    WM_DESTROY: begin
      if Changed then
      begin
        SaveAliases;
        Changed:=false;
        RegisterVariables;
      end;
    end;

    WM_NOTIFY: begin
      if wParam=IDC_MACROHELP then
      begin
        case integer(PNMHdr(lParam)^.code) of
          LVN_ENDLABELEDITW: begin
            with PLVDISPINFO(lParam)^ do
            begin
              if item.pszText<>nil then
              begin
                item.mask:=LVIF_TEXT;
                if pWideChar(item.pszText)^=#0 then
                  pWideChar(item.pszText):=vars[item.iItem].name;
                SendMessageW(hdr.hWndFrom,LVM_SETITEMW,0,tlparam(@item));
                mFreeMem(vars[item.iItem].alias);
                StrDupW(vars[item.iItem].alias,pWideChar(item.pszText));
                result:=1;
              end;
              Changed:=true;
            end;
          end;

          NM_DBLCLK: begin
            if PNMListView(lParam)^.iItem>=0 then
            begin
              SendMessage(PNMHdr(lParam)^.hWndFrom,LVM_EDITLABEL,
                          PNMListView(lParam)^.iItem,0);
            end;
          end;
        end;
      end;
    end;

  end;
end;

function WATMacroHelp(wParam:WPARAM;lParam:LPARAM):int;cdecl;
begin
  result:=CreateDialogParamW(hInstance,'MACRO',wParam,@MacroHelpDlg,0);
end;
