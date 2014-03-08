{Statistic Dialog}

{$include stat_rc.inc}

procedure SetReportMask(Dlg:HWND);
begin
  ReportMask:=0;
  if IsDlgButtonChecked(Dlg,IDC_FREQART)=BST_CHECKED then
    ReportMask:=ReportMask or stArtist;
  if IsDlgButtonChecked(Dlg,IDC_FREQSONG)=BST_CHECKED then
    ReportMask:=ReportMask or stCount;
  if IsDlgButtonChecked(Dlg,IDC_FREQPATH)=BST_CHECKED then
    ReportMask:=ReportMask or stPath;
  if IsDlgButtonChecked(Dlg,IDC_LASTSONG)=BST_CHECKED then
    ReportMask:=ReportMask or stDate;
  if IsDlgButtonChecked(Dlg,IDC_SONGTIME)=BST_CHECKED then
    ReportMask:=ReportMask or stLength;
  if IsDlgButtonChecked(Dlg,IDC_FREQALBUM)=BST_CHECKED then
    ReportMask:=ReportMask or stAlbum;
end;

procedure EnableItems(Dlg:HWND;enable:boolean);
begin
  EnableWindow(GetDlgItem(Dlg,IDC_STATNAME) ,enable);
  EnableWindow(GetDlgItem(Dlg,IDC_SNBUTTON) ,enable);
  EnableWindow(GetDlgItem(Dlg,IDC_SORTFILE) ,enable);
  EnableWindow(GetDlgItem(Dlg,IDC_REPORT)   ,enable);
  EnableWindow(GetDlgItem(Dlg,IDC_CLEAR)    ,enable);
  EnableWindow(GetDlgItem(Dlg,IDC_BYTITLE)  ,enable);
  EnableWindow(GetDlgItem(Dlg,IDC_BYDATE)   ,enable);
  EnableWindow(GetDlgItem(Dlg,IDC_BYCOUNT)  ,enable);
  EnableWindow(GetDlgItem(Dlg,IDC_BYPATH)   ,enable);
  EnableWindow(GetDlgItem(Dlg,IDC_BYLENGTH) ,enable);
  EnableWindow(GetDlgItem(Dlg,IDC_DIRECTION),enable);
  EnableWindow(GetDlgItem(Dlg,IDC_REPNAME)  ,enable);
  EnableWindow(GetDlgItem(Dlg,IDC_RNBUTTON) ,enable);
  EnableWindow(GetDlgItem(Dlg,IDC_ITEMS)    ,enable);
  EnableWindow(GetDlgItem(Dlg,IDC_FREQART)  ,enable);
  EnableWindow(GetDlgItem(Dlg,IDC_FREQSONG) ,enable);
  EnableWindow(GetDlgItem(Dlg,IDC_FREQALBUM),enable);
  EnableWindow(GetDlgItem(Dlg,IDC_FREQPATH) ,enable);
  EnableWindow(GetDlgItem(Dlg,IDC_LASTSONG) ,enable);
  EnableWindow(GetDlgItem(Dlg,IDC_SONGTIME) ,enable);
  EnableWindow(GetDlgItem(Dlg,IDC_RUNREPORT),enable);
  EnableWindow(GetDlgItem(Dlg,IDC_ADDEXT)   ,enable);
  EnableWindow(GetDlgItem(Dlg,IDC_AUTOSORT) ,enable);
end;

function DlgProcOptions(Dialog:HWND; hMessage:uint;wParam:WPARAM;lParam:LPARAM):LRESULT; stdcall;
const
  changed:boolean=false;
var
  buf,buf1:array [0..511] of AnsiChar;
  tmp:longbool;
  p:PAnsiChar;
  f:THANDLE;
begin
  result:=0;
  case hMessage of
    WM_INITDIALOG: begin
      TranslateDialogDefault(Dialog);
      SetDlgItemInt(Dialog,IDC_ITEMS,ReportItems,false);
      if ReportName=nil then
        p:=''
      else
        p:=ReportName;
      SetDlgItemTextA(Dialog,IDC_REPNAME,p);
      if StatName=nil then
        p:=''
      else
        p:=StatName;
      SetDlgItemTextA(Dialog,IDC_STATNAME,p);
      if TmplName=nil then
        p:=''
      else
        p:=TmplName;
      SetDlgItemTextA(Dialog,IDC_TMPLNAME,p);
      SetDlgItemInt(Dialog,IDC_AUTOSORT,AutoSort,false);
      CheckDlgButton(Dialog,IDC_RUNREPORT,RunReport);
      CheckDlgButton(Dialog,IDC_ADDEXT,DoAddExt);

      CheckDlgButton(Dialog,IDC_BYTITLE ,ord(SortMode=stArtist));
      CheckDlgButton(Dialog,IDC_BYDATE  ,ord(SortMode=stDate));
      CheckDlgButton(Dialog,IDC_BYCOUNT ,ord(SortMode=stCount));
      CheckDlgButton(Dialog,IDC_BYPATH  ,ord(SortMode=stPath));
      CheckDlgButton(Dialog,IDC_BYLENGTH,ord(SortMode=stLength));

      if Direction=smReverse then
        CheckDlgButton(Dialog,IDC_DIRECTION,BST_CHECKED);

      if (ReportMask and stArtist)<>0 then
        CheckDlgButton(Dialog,IDC_FREQART,BST_CHECKED);
      if (ReportMask and stAlbum)<>0 then
        CheckDlgButton(Dialog,IDC_FREQALBUM,BST_CHECKED);
      if (ReportMask and stCount)<>0 then
        CheckDlgButton(Dialog,IDC_FREQSONG,BST_CHECKED);
      if (ReportMask and stPath)<>0 then
        CheckDlgButton(Dialog,IDC_FREQPATH,BST_CHECKED);
      if (ReportMask and stDate)<>0 then
        CheckDlgButton(Dialog,IDC_LASTSONG,BST_CHECKED);
      if (ReportMask and stLength)<>0 then
        CheckDlgButton(Dialog,IDC_SONGTIME,BST_CHECKED);
      result:=0;
      changed:=false;
    end;

    WM_COMMAND: begin
      if (wParam shr 16)=BN_CLICKED then
      begin
        case loword(wParam) of
          IDC_BYTITLE : SortMode:=stArtist;
          IDC_BYDATE  : SortMode:=stDate;
          IDC_BYCOUNT : SortMode:=stCount;
          IDC_BYPATH  : SortMode:=stPath;
          IDC_BYLENGTH: SortMode:=stLength;
          IDC_RUNREPORT: RunReport :=IsDlgButtonChecked(Dialog,IDC_RUNREPORT);
          IDC_ADDEXT:    DoAddExt  :=IsDlgButtonChecked(Dialog,IDC_ADDEXT);
          IDC_DIRECTION: begin
            if IsDlgButtonChecked(Dialog,IDC_DIRECTION)=BST_CHECKED then
              Direction:=smReverse
            else
              Direction:=smDirect;
          end;

          IDC_CLEAR: begin
            DeleteFileA(StatName);
            exit;
          end;
          IDC_SNBUTTON: begin
            if ShowDlg(buf,StatName) then
              SetDlgItemTextA(Dialog,IDC_STATNAME,buf);
          end;
          IDC_TNBUTTON: begin
            if ShowDlg(buf,TmplName) then
              SetDlgItemTextA(Dialog,IDC_TMPLNAME,buf);
          end;
          IDC_RNBUTTON: begin
            if ShowDlg(buf,ReportName) then
              SetDlgItemTextA(Dialog,IDC_REPNAME,buf);
          end;
          IDC_SORTFILE: begin
            GetDlgItemTextA(Dialog,IDC_STATNAME,buf,511);
            if buf[0]<>#0 then
              SortFile(buf,SortMode,Direction);
            exit;
          end;
          IDC_EXPORTDEF: begin
            if ShowDlg(buf,TmplName) then
            begin
              f:=Rewrite(buf);
              if f=THANDLE(INVALID_HANDLE_VALUE) then
                err('Can''t create file')
              else
              begin
                BlockWrite(f,IntTmpl^,StrLen(IntTmpl));
                CloseHandle(f);
              end;
            end;
            exit;
          end;
          IDC_REPORT: begin
            ReportItems:=GetDlgItemInt(Dialog,IDC_ITEMS,tmp,false);
            if ReportItems=0 then
              ReportItems:=1;
            GetDlgItemTextA(Dialog,IDC_REPNAME,buf1,511);
            GetDlgItemTextA(Dialog,IDC_TMPLNAME,buf,511);
            SetReportMask(Dialog);
            CallService(MS_WAT_MAKEREPORT,TWPARAM(@buf),TLPARAM(@buf1));
          end;
        end;
      end;
      if ((wParam shr 16)=EN_CHANGE) or ((wParam shr 16)=BN_CLICKED) then
      begin
        SendMessage(GetParent(Dialog),PSM_CHANGED,0,0);
        changed:=true;
      end;
      result:=1;
    end;

    WM_NOTIFY: begin
      if (integer(PNMHdr(lParam)^.code)=PSN_APPLY) and changed then
      begin
        GetDlgItemTextA(Dialog,IDC_STATNAME,buf,511);
        mFreeMem(StatName);
        if buf[0]<>#0 then
        begin
          buf1[0]:=#0;
          PathToRelative(buf,buf1);
//          CallService(MS_UTILS_PATHTORELATIVE,TWPARAM(@buf),TLPARAM(@buf1));
          StrDup(StatName,buf1);
        end;

        GetDlgItemTextA(Dialog,IDC_REPNAME,buf,511);
        mFreeMem(ReportName);
        if buf[0]<>#0 then
        begin
          buf1[0]:=#0;
          PathToRelative(buf,buf1);
//          CallService(MS_UTILS_PATHTORELATIVE,TWPARAM(@buf),TLPARAM(@buf1));
          StrDup(ReportName,buf1);
        end;

        GetDlgItemTextA(Dialog,IDC_TMPLNAME,buf,511);
        mFreeMem(TmplName);
        if buf[0]<>#0 then
        begin
          buf1[0]:=#0;
          PathToRelative(buf,buf1);
//          CallService(MS_UTILS_PATHTORELATIVE,TWPARAM(@buf),TLPARAM(@buf1));
          StrDup(TmplName,buf1);
        end;

        AutoSort:=GetDlgItemInt(Dialog,IDC_AUTOSORT,tmp,false);
        ReportItems:=GetDlgItemInt(Dialog,IDC_ITEMS,tmp,false);
        if ReportItems=0 then
          ReportItems:=1;
        SetReportMask(Dialog);
        result:=1;
        savestat;
        changed:=false;
      end;
    end;
  else
    {result:=}DefWindowProc(Dialog,hMessage,wParam,lParam);
  end;
end;
