unit ImportTxtDlg;

interface

uses
  m_api,
  Windows, Messages, SysUtils, CommDlg, IniFiles,
  ImportThrd, general, ImportTU, FileDlgs;


{$I ImpDlgRes.inc}

function IDMainWndProc(Dialog:HWnd; hMessage:uint;wParam:WPARAM;lParam:LPARAM):lresult; stdcall;

implementation

{$R ImpTxtDlg.res}

type
  TImpTxtDlgData = record
    cbSize: integer;
    CanStart: boolean;
    ChoFile: boolean;
    ChoPatt: boolean;
    PattFError: boolean;
    ITStarted: boolean;
    ITFinished: boolean;
    StartTime: TDateTime;
    FinishTime: TDateTime;
    IndexOfPattern: integer;
    oFN: integer;
    hContact: THandle;
    ImportThrd: TImportThrd;
  end;

type PImpTxtDlgData = ^TImpTxtDlgData;

var ITDD:TImpTxtDlgData;

procedure AddStatusMessage(hDLG:HWND; msg:WideString);
begin
  SendDlgItemMessageW(hDLG,IDC_STATUSLIST,LB_SETCURSEL,
    SendDlgItemMessageW(hDLG,IDC_STATUSLIST,LB_ADDSTRING,0,lparam(PWideChar(msg))),0);
end;

procedure FillComboBox(hDlg:hWnd);  //Заполняем список именами шаблонов
var
  i:integer;
begin
  ITDD.PattFError:=true;
  ReadPatterns;
  SendDlgItemMessage(hDLG, IDC_TYPECOMBO, CB_RESETCONTENT, 0, 0);
  if Length(TxtPatterns) > 0 then
  begin
    For i := 0 to High(TxtPatterns) do
      SendDlgItemMessage(hDLG, IDC_TYPECOMBO, CB_ADDSTRING, 0, lparam(PChar(TxtPatterns[i].Name)));
    ITDD.PattFError := false;
  end
  else
  begin
    AddStatusMessage(hDLG, TranslateWideString('No Files Patterns Found'));
    EnableWindow(GetDlgItem(hDLG, IDC_TYPECOMBO), false);
    ITDD.ChoPatt := false;
  end;
end;

procedure MStart(hDLG: HWnd);    //проверяем всё ли сделано чтоб начать импорт
begin
  with ITDD do
  begin
    CanStart := ChoFile and ChoPatt and (not PattFError);
    EnableWindow(GetDlgItem(hDLG, IDSTART), CanStart);
  end;
end;

function IDMainWndProc(Dialog:HWnd; hMessage:uint;wParam:WPARAM;lParam:LPARAM):lresult; stdcall;
//Процедура окна диалога
var
  s:WideString;
  tempwstr:PWideChar;
  H,Mi,sec,ms:word;

begin
 result:= 0;
  case hMessage of
    WM_DESTROY:
      begin
        if ITDD.ImportThrd <> Nil then
          ITDD.ImportThrd.Terminate;
        SetLastPattern(SendDlgItemMessage(Dialog, IDC_TYPECOMBO, CB_GETCURSEL, 0, 0));
      end;

    WM_INITDIALOG:
      begin
        TranslateDialogDefault(Dialog);
        tempwstr := PWideChar(CallService(MS_CLIST_GETCONTACTDISPLAYNAME,lParam,GCDNF_UNICODE));
        s := WideFormat(TranslateWideString('Import history to %s (%s)'), [tempwstr, GetContactID(lParam)]);
        SetWindowTextW(Dialog, PWideChar(s));
        SendMessage(Dialog, WM_SETICON, ICON_SMALL, LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DEFAULT)));
        FillChar(ITDD, SizeOf(TImpTxtDlgData), 0);
        with ITDD do
        begin
          cbSize := SizeOf(TImpTxtDlgData);
          hContact := lParam;
          CanStart := false;
          ChoFile := false;
          ChoPatt := true;
          PattFError := false;
          ITStarted := false;
          ITFinished := false;
        end;

        FillComboBox(Dialog);
        SendDlgItemMessage(Dialog, IDC_TYPECOMBO, CB_SETCURSEL, GetLastPattern, 0);
        ITDD.IndexOfPattern := GetLastPattern;
        MStart(Dialog);
        CheckDlgButton(Dialog, IDC_CHKDUP, BST_CHECKED);
        CheckForDuplicates := true;
        CheckDlgButton(Dialog, IDC_SHOWDUP, BST_UNCHECKED);
        CheckForDuplicates := false;
        ShowWindow(Dialog, SW_NORMAL);
      end;

    WM_COMMAND:
      begin
        if (HiWord(wParam) = CBN_SELCHANGE) and (LoWord(wParam) = IDC_TYPECOMBO) then // Сделали выбор шаблона
        begin
          ITDD.ChoPatt := true;
          ITDD.IndexOfPattern := SendDlgItemMessage(Dialog, IDC_TYPECOMBO, CB_GETCURSEL, 0, 0);
          MStart(Dialog);
        end;

        if (HiWord(wParam) = BN_CLICKED) and (LoWord(wParam) = IDC_CHKDUP) then
          EnableWindow(GetDlgItem(Dialog, IDC_SHOWDUP),
            IsDlgButtonChecked(Dialog, IDC_CHKDUP)<>BST_UNCHECKED);

        case LoWord(wParam) of
          IDCLOSE:
            DestroyWindow(Dialog);
          IDC_BRWSBTN:
            begin // Нажали кнопку "..."
              s := OpenDialogExecute(Dialog, 0, ITDD.oFN,
                TxtPatterns[ITDD.IndexOfPattern].DefExtension);
              ITDD.ChoFile := s <> '';
              SendDlgItemMessageW(Dialog, IDC_FILENAME, WM_SETTEXT, 0, tlparam(PWideChar(s)));
              MStart(Dialog);
            end;
          IDSTART:
            begin // Старт
              with ITDD do
                if CanStart then
                begin
                  EnableWindow(GetDlgItem(Dialog, IDSTART), false);
                  EnableWindow(GetDlgItem(Dialog, IDCLOSE), false);
                  EnableWindow(GetDlgItem(Dialog, IDC_TYPECOMBO), false);
                  EnableWindow(GetDlgItem(Dialog, IDC_BRWSBTN), false);
                  ImportThrd := TImportThrd.Create(true);
                  ImportThrd.FreeOnTerminate := true;
                  ImportThrd.DContact.hContact := hContact;
                  CheckForDuplicates := LongBool(IsDlgButtonChecked(Dialog, IDC_CHKDUP));
                  ShowDuplicates := LongBool(IsDlgButtonChecked(Dialog, IDC_SHOWDUP));
                  H := SendDlgItemMessageW(Dialog, IDC_FILENAME, WM_GETTEXTLENGTH, 0, 0) + 1;
                  SetLength(s, H);
                  SendDlgItemMessageW(Dialog, IDC_FILENAME, WM_GETTEXT, H, tlparam(PWideChar(s)));
                  ImportThrd.FileNames := s + #0 + #0;
                  ImportThrd.OffsetFileName := oFN;
                  ImportThrd.WorkPattern := TxtPatterns[IndexOfPattern];
                  ImportThrd.ParentHWND := Dialog;
                  ImportThrd.Resume;
                end;
            end;
        end;
      end;
    WM_CLOSE:
      begin
        DestroyWindow(Dialog);
      end;
    // Далее оброботка сообщений от потока импорта
    // Начали
    ITXT_THREAD_ALLSTARTED:
      begin
        ITDD.ITStarted := true;
        ITDD.StartTime := Time;
      end;
    ITXT_THREAD_START:
      begin
        AddStatusMessage(Dialog, TranslateWideString('Import started...'));
      end;
    // Известна длинна файла выставляем диапазон прогрессбара
    ITXT_THREAD_MAXPROGRESS:
      SendDlgItemMessage(Dialog, IDC_PROGRESS, PBM_SETRANGE, 0, MakeLParam(0, lParam));
    // Идет прогресс ...
    ITXT_THREAD_PROGRESS:
      SendDlgItemMessage(Dialog, IDC_PROGRESS, PBM_SETPOS, wParam, 0);
    // Возникла ошибка
    ITXT_THREAD_ERROR:
      AddStatusMessage(Dialog, TranslateWideString(PWideChar(wParam)));
    // Закончили
    ITXT_THREAD_FINISH:
      begin
        ITDD.ITFinished := true;
        ITDD.FinishTime := Time;
        DecodeTime(ITDD.FinishTime - ITDD.StartTime, H, Mi, sec, ms);
        AddStatusMessage(Dialog, WideFormat(TranslateWideString('Added: %d messages'), [wParam]));
        AddStatusMessage(Dialog, WideFormat(TranslateWideString('Duplicates: %d messages'), [lParam]));
        AddStatusMessage(Dialog, WideFormat(TranslateWideString('In: %d:%d:%d'), [H, Mi, sec]));
        AddStatusMessage(Dialog, '');
        SendDlgItemMessageW(Dialog, IDC_FILENAME, WM_SETTEXT, 0, 0);
        ITDD.ChoFile := false;
        EnableWindow(GetDlgItem(Dialog, IDC_TYPECOMBO), true);
        EnableWindow(GetDlgItem(Dialog, IDC_BRWSBTN), true);
        EnableWindow(GetDlgItem(Dialog, IDCLOSE), true);
      end;
    // начали новый файл
    ITXT_THREAD_START_FILE:
      AddStatusMessage(Dialog, WideFormat(TranslateWideString('File: %s'), [PWideChar(wParam)]));
    // определили контакт
    ITXT_THREAD_DEST_CONTACT:
      begin
        s := GetContactID(wParam, '', true) + ' | ' + GetContactNick(wParam, '', true);

        AddStatusMessage(Dialog, WideFormat(TranslateWideString('To: %s'), [s]));
      end;
  end;
end;

end.
