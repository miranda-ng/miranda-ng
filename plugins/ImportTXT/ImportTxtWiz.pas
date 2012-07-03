unit ImportTxtWiz;

interface

uses Windows, Messages, SysUtils,
  m_api,
  general, ImportTU, FileDlgs,
  ImportThrd;

{$R ImpTxtWiz.res}
{$I ImpWizRes.inc}

function WizardIntroPageProc(hdlg: HWND; hMessage:uint; wParam:WPARAM; lParam:LPARAM): LongBool; stdcall;
function ImportTypePageProc(hdlg: HWND; hMessage:uint; wParam:WPARAM; lParam:LPARAM): LongBool; stdcall;
function WizardDlgProc(hdlg: HWND; hMessage:uint; wParam:WPARAM; lParam:LPARAM): LongBool; stdcall;
function ImportFilesPageProc(hdlg: HWND; hMessage:uint; wParam:WPARAM; lParam:LPARAM): LongBool; stdcall;
function ImportDestinationPageProc(hdlg: HWND; hMessage:uint; wParam:WPARAM; lParam:LPARAM): LongBool; stdcall;
function ImportProgressPageProc(hdlg: HWND; hMessage:uint; wParam:WPARAM; lParam:LPARAM): LongBool; stdcall;

implementation

const
  WIZM_GOTOPAGE       = (WM_USER + 10); // wParam=resource id, lParam=dlgproc
  WIZM_DISABLEBUTTON  = (WM_USER + 11); // wParam=0:back, 1:next, 2:cancel
  WIZM_SETCANCELTEXT  = (WM_USER + 12); // lParam=(char*)newText
  WIZM_SETNEXTTEXT    = (WM_USER + 13); // lParam=(char*)newText
  WIZM_ENABLEBUTTON   = (WM_USER + 14); // wParam=0:back, 1:next, 2:cancel
  WIZM_ACCLISTCHANGED = (WM_USER + 15); // OnAccountListChanged

var
  hwndPage: HWND;
  IndexOfPattern: integer;
  ImportThrd: TImportThrd;
  ProtoSelect: integer;
  nFO: integer;
  FilePath: WideString;
  FileName: WideString;
  FileNames: array of WideString;
  StartTime, FinishTime: TDateTime;

function MakeFileList(hdlg: HWND; param: WORD): boolean;
var
  i: integer;
  s1: WideString;
begin
  result := false;
  case param of
    IDC_FILE:
      FileName := OpenDialogExecute(hdlg, $0200, nFO, TxtPatterns[IndexOfPattern].DefExtension);
      // OFN_ALLOWMULTISELECT = $00000200;
    IDC_DIR:
      FileName := FolderDialogExecute(hdlg, nFO, TxtPatterns[IndexOfPattern].DefExtension);
  end;
  if FileName <> '' then
  begin
    SendDlgItemMessageW(hdlg, IDC_LIST, LB_RESETCONTENT, 0, 0);
    SendDlgItemMessageW(hdlg, IDC_PATH, WM_SETTEXT, 0, 0);
    SetLength(FileNames, 0);
    FilePath := Copy(FileName, 1, nFO - 1);
    SendDlgItemMessageW(hdlg, IDC_PATH, WM_SETTEXT, 0, lparam(PWideChar(FilePath)));
    i := nFO;
    while FileName[i + 1] <> #0 do
    begin
      s1 := '';
      Inc(i);
      while FileName[i] <> #0 do
      begin
        s1 := s1 + FileName[i];
        Inc(i);
      end;
      if (s1 <> '') and (s1 <> #0) then
      begin
        SetLength(FileNames, Length(FileNames) + 1);
        FileNames[High(FileNames)] := s1;
        SendDlgItemMessageW(hdlg, IDC_LIST, LB_ADDSTRING, 0, lparam(PWideChar(s1)));
      end;
    end;
  end;
  if High(FileNames) > -1 then
    result := true;
end;

procedure AddStatusMessage(hdlg: HWND; msg: WideString);
begin
  SendDlgItemMessageW(hdlg, IDC_STATUS, LB_SETCURSEL,
    SendDlgItemMessageW(hdlg, IDC_STATUS, LB_ADDSTRING, 0, lparam(PWideChar(msg))), 0);
end;

procedure AccLstChngd;
begin
  SendMessageW(hwndPage, WIZM_ACCLISTCHANGED, 0, 0);
end;

function WizardIntroPageProc(hdlg: HWND; hMessage:uint; wParam:WPARAM; lParam:LPARAM): LongBool; stdcall;
begin
  result := false;
  case hMessage of
    WM_INITDIALOG:
      begin
        TranslateDialogDefault(hdlg);
        SendMessage(GetParent(hdlg), WIZM_DISABLEBUTTON, 0, 0);
        IndexOfPattern := 0;
        ProtoSelect := 0;
        SetLength(FileNames, 0);
        result := true;
      end;
    WM_COMMAND:
      case LOWORD(wParam) of
        IDOK:
          PostMessage(GetParent(hdlg), WIZM_GOTOPAGE, IDD_IMPTYPE, tlparam(@ImportTypePageProc));
        IDCANCEL:
          PostMessage(GetParent(hdlg), WM_CLOSE, 0, 0);
      end; // Case wParam
  end; // Case hMassege
end;

function ImportTypePageProc(hdlg: HWND; hMessage:uint; wParam:WPARAM; lParam:LPARAM): LongBool; stdcall;
var
  i: integer;
begin
  result := false;
  case hMessage of
    WM_INITDIALOG:
      begin
        TranslateDialogDefault(hdlg);
        ReadPatterns;
        if Length(TxtPatterns) > 0 then
        begin
          For i := 0 to High(TxtPatterns) do
            SendDlgItemMessage(hdlg, IDC_COMBO1, CB_ADDSTRING, 0, tlparam(PChar(TxtPatterns[i].Name)));
          SendDlgItemMessage(hdlg, IDC_COMBO1, CB_SETCURSEL, GetLastPattern, 0);
          IndexOfPattern := GetLastPattern;
        end
        else
          SendMessage(GetParent(hdlg), WIZM_DISABLEBUTTON, 1, 0);
        result := true;
      end;
    WM_COMMAND:
      begin
        if (HiWord(wParam) = CBN_SELCHANGE) and (LOWORD(wParam) = IDC_COMBO1)
        then
          IndexOfPattern := SendDlgItemMessage(hdlg, IDC_COMBO1,
            CB_GETCURSEL, 0, 0);
        Case LOWORD(wParam) of
          IDC_BACK:
            PostMessage(GetParent(hdlg), WIZM_GOTOPAGE, IDD_IMPWIZARDINTRO, tlparam(@WizardIntroPageProc));
          IDOK:
            PostMessage(GetParent(hdlg), WIZM_GOTOPAGE, IDD_IMPFILES, tlparam(@ImportFilesPageProc));
          IDCANCEL:
            PostMessage(GetParent(hdlg), WM_CLOSE, 0, 0);
        end; // Case wParam
      end; // WM_COMMAND
  end; // Case hMassage
end;

function ImportFilesPageProc(hdlg: HWND; hMessage:uint; wParam:WPARAM; lParam:LPARAM): LongBool; stdcall;
var
  i: integer;
begin
  result := false;
  case hMessage of
    WM_INITDIALOG:
      begin
        TranslateDialogDefault(hdlg);
        if High(FileNames) < 0 then
          SendMessage(GetParent(hdlg), WIZM_DISABLEBUTTON, 1, 0)
        else
        begin
          SendDlgItemMessageW(hdlg, IDC_PATH, WM_SETTEXT, 0, tlparam(PWideChar(FilePath)));
          for i := 0 to High(FileNames) do
            SendDlgItemMessageW(hdlg, IDC_LIST, LB_ADDSTRING, 0, tlparam(PWideChar(FileNames[i])));
        end;
        result := true;
      end;
    WM_COMMAND:
      Case LOWORD(wParam) of
        IDC_FILE, IDC_DIR:
          if MakeFileList(hdlg, LOWORD(wParam)) then
            SendMessage(GetParent(hdlg), WIZM_ENABLEBUTTON, 1, 0)
          else
            SendMessage(GetParent(hdlg), WIZM_DISABLEBUTTON, 1, 0);
        IDC_BACK:
          PostMessage(GetParent(hdlg), WIZM_GOTOPAGE, IDD_IMPTYPE, tlparam(@ImportTypePageProc));
        IDOK:
          PostMessage(GetParent(hdlg), WIZM_GOTOPAGE, IDD_IMPDEST, tlparam(@ImportDestinationPageProc));
        IDCANCEL:
          PostMessage(GetParent(hdlg), WM_CLOSE, 0, 0);
      end; // Case wParam
  end; // Case hMassage
end;

function ImportDestinationPageProc(hdlg: HWND; hMessage:uint; wParam:WPARAM; lParam:LPARAM): LongBool; stdcall;
var
  i: integer;
  tempws: WideString;
begin
  result := false;
  case hMessage of
    WM_INITDIALOG:
      begin
        TranslateDialogDefault(hdlg);
        EnumProtocols;
        for i := 0 to ProtoCount - 1 do
        begin
          tempws := Protocols[i].ProtoName + ' | ';
          tempws := tempws + Protocols[i].ProtoUID + ' | ' + Protocols[i].ProtoNick;
          SendDlgItemMessageW(hdlg, IDC_COMBO1, CB_ADDSTRING, 0, tlparam(PWideChar(tempws)));
        end;
        SendDlgItemMessage(hdlg, IDC_COMBO1, CB_SETCURSEL, 0, 0);
        OnAccountListChange := AccLstChngd;
        CheckDlgButton(hdlg, IDC_CHKDUPW, BST_CHECKED);
        CheckForDuplicates := true;
        CheckDlgButton(hdlg, IDC_SHOWDUPW, BST_UNCHECKED);
        CheckForDuplicates := false;
        result := true;
      end;
    WM_COMMAND:
      begin
        if (HiWord(wParam) = CBN_SELCHANGE) and (LOWORD(wParam) = IDC_COMBO1)
        then
          ProtoSelect := SendDlgItemMessage(hdlg, IDC_COMBO1, CB_GETCURSEL, 0, 0);

        if (HiWord(wParam) = BN_CLICKED) and (LOWORD(wParam) = IDC_CHKDUPW) then
          EnableWindow(GetDlgItem(hdlg, IDC_SHOWDUPW),
            IsDlgButtonChecked(hdlg, IDC_CHKDUPW)<>BST_UNCHECKED);

        Case LOWORD(wParam) of
          IDC_BACK:
            PostMessage(GetParent(hdlg), WIZM_GOTOPAGE, IDD_IMPFILES, tlparam(@ImportFilesPageProc));
          IDOK:
            begin
              CheckForDuplicates := LongBool(IsDlgButtonChecked(hdlg, IDC_CHKDUPW));
              ShowDuplicates := LongBool(IsDlgButtonChecked(hdlg, IDC_SHOWDUPW));
              PostMessage(GetParent(hdlg), WIZM_GOTOPAGE, IDD_PROGRESS, tlparam(@ImportProgressPageProc));
            end;
          IDCANCEL:
            PostMessage(GetParent(hdlg), WM_CLOSE, 0, 0);
        end; // Case wParam
      end; // WM_COMMAND
    WIZM_ACCLISTCHANGED:
      begin
        ProtoSelect := SendDlgItemMessage(hdlg, IDC_COMBO1, CB_GETCURSEL, 0, 0);
        SendDlgItemMessageW(hdlg, IDC_COMBO1, CB_RESETCONTENT, 0, 0);
        for i := 0 to ProtoCount - 1 do
        begin
          tempws := Protocols[i].ProtoName + ' | ';
          tempws := tempws + Protocols[i].ProtoUID + ' | ' + Protocols[i].ProtoNick;
          SendDlgItemMessageW(hdlg, IDC_COMBO1, CB_ADDSTRING, 0, tlparam(PWideChar(tempws)));
        end;
        if ProtoSelect < ProtoCount - 1 then
          SendDlgItemMessage(hdlg, IDC_COMBO1, CB_SETCURSEL, ProtoSelect, 0)
        else
          SendDlgItemMessage(hdlg, IDC_COMBO1, CB_SETCURSEL, ProtoCount - 1, 0);
      end; // WIZM_...
  end; // Case hMassage
end;

function ImportProgressPageProc(hdlg: HWND; hMessage:uint; wParam:WPARAM; lParam:LPARAM): LongBool; stdcall;
var
  H, Mi, sec, ms: WORD;
  s: WideString;
begin
  result := false;
  case hMessage of
    WM_INITDIALOG:
      begin
        TranslateDialogDefault(hdlg);
        SendMessage(GetParent(hdlg), WIZM_DISABLEBUTTON, 0, 0);
        SendMessage(GetParent(hdlg), WIZM_DISABLEBUTTON, 1, 0);
        SendMessage(GetParent(hdlg), WIZM_DISABLEBUTTON, 2, 0);
        SendMessage(GetParent(hdlg), WIZM_SETCANCELTEXT, 0, tlparam(TranslateW('Finish')));
        // START THREAD
        ImportThrd := TImportThrd.Create(true);
        ImportThrd.FreeOnTerminate := true;
        ImportThrd.DContact.hContact := 0;
        ImportThrd.Destination := Protocols[ProtoSelect];
        ImportThrd.FileNames := FileName + #0 + #0;
        ImportThrd.OffsetFileName := nFO;
        ImportThrd.WorkPattern := TxtPatterns[IndexOfPattern];
        ImportThrd.ParentHWND := hdlg;
        ImportThrd.Resume;
        result := true;
      end;
    WM_COMMAND:
      Case LOWORD(wParam) of
        IDCANCEL:
          PostMessage(GetParent(hdlg), WM_CLOSE, 0, 0);
      end; // Case wParam
    // Далее оброботка сообщений от потока импорта
    // Начали
    ITXT_THREAD_ALLSTARTED:
      StartTime := Time;
    ITXT_THREAD_START:
      begin
        AddStatusMessage(hdlg, TranslateWideString('Import started...'));
      end;
    // Известна длинна файла выставляем диапазон прогрессбара
    ITXT_THREAD_MAXPROGRESS:
      SendDlgItemMessage(hdlg, IDC_PROGRESS, PBM_SETRANGE, 0, MakeLParam(0, lParam));
    // Идет прогресс ...
    ITXT_THREAD_PROGRESS:
      SendDlgItemMessage(hdlg, IDC_PROGRESS, PBM_SETPOS, wParam, 0);
    // Возникла ошибка
    ITXT_THREAD_ERROR:
      AddStatusMessage(hdlg, TranslateWideString(PWideChar(wParam)));
    // Закончили
    ITXT_THREAD_FINISH:
      begin
        AddStatusMessage(hdlg, WideFormat(TranslateWideString('Added: %d messages'), [wParam]));
        AddStatusMessage(hdlg, WideFormat(TranslateWideString('Duplicates: %d messages'), [lParam]));
      end;
    // начали новый файл
    ITXT_THREAD_START_FILE:
      AddStatusMessage(hdlg, WideFormat(TranslateWideString('File: %s'), [PWideChar(wParam)]));
    // определили контакт
    ITXT_THREAD_DEST_CONTACT:
      begin
        s := GetContactID(wParam, '', true) + ' | ' + GetContactNick(wParam, '', true);
        AddStatusMessage(hdlg, WideFormat(TranslateWideString('To: %s'), [s]));
      end;
    ITXT_THREAD_ALLFINISHED:
      begin
        FinishTime := Time;
        DecodeTime(FinishTime - StartTime, H, Mi, sec, ms);
        AddStatusMessage(hdlg, WideFormat(TranslateWideString('In: %d:%d:%d'), [H, Mi, sec]));
        SendMessage(GetParent(hdlg), WIZM_ENABLEBUTTON, 2, 0);
      end;
  end; // Case hMessage
end;

function WizardDlgProc(hdlg: HWND; hMessage:uint; wParam:WPARAM; lParam:LPARAM): LongBool; stdcall;
begin
  result := false;
  case hMessage of
    WM_DESTROY:
      SetLastPattern(IndexOfPattern);
    WM_INITDIALOG:
      begin
        TranslateDialogDefault(hdlg);
        hwndPage := CreateDialog(hInstance, MAKEINTRESOURCE(IDD_IMPWIZARDINTRO),
          hdlg, @WizardIntroPageProc);
        SetWindowPos(hwndPage, 0, 0, 0, 0, 0, SWP_NOZORDER or SWP_NOSIZE);
        ShowWindow(hwndPage, SW_SHOW);
        ShowWindow(hdlg, SW_SHOW);
        SendMessage(hdlg, WM_SETICON, ICON_BIG, LoadIcon(hInstance, MAKEINTRESOURCE(110)));
        result := true;
      end;
    WIZM_GOTOPAGE:
      begin
        DestroyWindow(hwndPage);
        EnableWindow(GetDlgItem(hdlg, IDC_BACK), true);
        EnableWindow(GetDlgItem(hdlg, IDOK), true);
        EnableWindow(GetDlgItem(hdlg, IDCANCEL), true);
        SetDlgItemTextW(hdlg, IDCANCEL, TranslateW('Cancel'));
        hwndPage := CreateDialog(hInstance, MAKEINTRESOURCE(wParam), hdlg, Pointer(lParam));
        SetWindowPos(hwndPage, 0, 0, 0, 0, 0, SWP_NOZORDER or SWP_NOSIZE);
        ShowWindow(hwndPage, SW_SHOW);
      end;
    WIZM_DISABLEBUTTON:
      begin
        case wParam of
          0: EnableWindow(GetDlgItem(hdlg, IDC_BACK), false);
          1: EnableWindow(GetDlgItem(hdlg, IDOK), false);
          2: EnableWindow(GetDlgItem(hdlg, IDCANCEL), false);
        end; // Case wParam
      end;
    WIZM_ENABLEBUTTON:
      begin
        case wParam of
          0: EnableWindow(GetDlgItem(hdlg, IDC_BACK), true);
          1: EnableWindow(GetDlgItem(hdlg, IDOK), true);
          2: EnableWindow(GetDlgItem(hdlg, IDCANCEL), true);
        end; // Case wParam
      end;
    WIZM_SETCANCELTEXT:
      SetDlgItemTextW(hdlg, IDCANCEL, PWideChar(lParam));
    WIZM_SETNEXTTEXT:
      SetDlgItemTextW(hdlg, IDOK, PWideChar(lParam));
    WM_COMMAND:
      SendMessage(hwndPage, WM_COMMAND, wParam, lParam);
    WM_CLOSE:
      begin
        OnAccountListChange := nil;
        DestroyWindow(hwndPage);
        DestroyWindow(hdlg);
      end;
  end; // Case hMessage
end;

end.
