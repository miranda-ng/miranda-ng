unit ImportTxtDlg;

interface

uses
  m_api,
  Windows, Messages, SysUtils, CommDlg, IniFiles,
  ImportThrd, general, ImportTU, FileDlgs;


{$I ImpDlgRes.inc}


function IDMainWndProc(Dialog:HWnd; hMessage,wParam,lParam:DWord):integer; stdcall;
function GetContactDisplayName(hContact:THandle):string;

implementation

{$R ImpTxtDlg.res}

type TImpTxtDlgData = record
    cbSize:integer;
    CanStart:boolean;
    ChoFile:boolean;
    ChoPatt:boolean;
    PattFError:boolean;
    ITStarted:boolean;
    ITFinished:boolean;
    StartTime:TDateTime;
    FinishTime:TDateTime;
    IndexOfPattern:integer;
    oFN:integer;
    hContact:THandle;
    ImportThrd:TImportThrd;
    end;

type PImpTxtDlgData = ^TImpTxtDlgData;

var ITDD:TImpTxtDlgData;

function GetContactDisplayName(hContact:THandle):string;
begin
 Result := PChar(pluginLink^.CallService(MS_CLIST_GETCONTACTDISPLAYNAME,hContact,0));
end;

procedure AddStatusMessage(hDLG:HWND; msg:WideString);
begin
  SendDlgItemMessageW(hDLG,IDC_STATUSLIST,LB_SETCURSEL,SendDlgItemMessageW(hDLG,IDC_STATUSLIST,LB_ADDSTRING,0,dword(PWideChar(msg))),0);
end;

procedure FillComboBox(hDlg:hWnd);  //Заполняем список именами шаблонов
var
  i:integer;
begin
 ITDD.PattFError:=true;
 ReadPatterns;
 SendDlgItemMessage(hDlg,IDC_TYPECOMBO,CB_RESETCONTENT,0,0);
 if PatternsCount>0 then
  begin
   For i:=0 to PatternsCount-1 do
    SendDlgItemMessage(hDlg,IDC_TYPECOMBO,CB_ADDSTRING,0,Integer(PatternNames[i]));
   ITDD.PattFError:=false;
  end
          else
  begin
   AddStatusMessage(hDlg,TranslateWideString('No Files Patterns Found'));
   EnableWindow(GetDlgItem(hDlg, IDC_TYPECOMBO), FALSE);
   ITDD.ChoPatt:=false;
  end;
end;

procedure MStart(hDlg:hWnd);    //проверяем всё ли сделано чтоб начать импорт
begin
with ITDD do
begin
if ChoFile and ChoPatt and (not PattFError)  then
           begin
          CanStart:=true;
          EnableWindow(GetDlgItem(hDlg, IDSTART), TRUE);
           end
                    else
           begin
          CanStart:=false;
          EnableWindow(GetDlgItem(hDlg, IDSTART), FALSE);
           end;
end;
end;


function IDMainWndProc(Dialog:HWnd; hMessage,wParam,lParam:DWord):integer; stdcall;
//Процедура окна диалога
var
    s:WideString;
    tempwstr:PWideChar;

var H,Mi,sec,ms:word;

begin
 result:=0;
 case hMessage of
  WM_DESTROY: begin
     if ITDD.ImportThrd<>Nil then ITDD.ImportThrd.Terminate;
     SetLastPattern(SendDlgItemMessage(Dialog, IDC_TYPECOMBO, CB_GETCURSEL, 0, 0));
              end;
  WM_INITDIALOG: begin
    TranslateDialogDefault(Dialog);
    tempwstr:=ANSIToWide(PChar(GetContactDisplayName(lParam)),tempwstr,cp);
    s:=WideFormat(TranslateWideString('Import history to %s (%s)'),[tempwstr,GetContactID(lParam)]);
    SetWindowTextW(Dialog,PWideChar(s));
    SendMessage(Dialog,WM_SETICON,ICON_SMALL,LoadIcon(hInstance,MAKEINTRESOURCE(IDI_DEFAULT)));
    FillChar(ITDD,SizeOf(TImpTxtDlgData),0);
    with ITDD do begin
      cbSize:=SizeOf(TImpTxtDlgData);
      hContact:=lParam;
      CanStart:=false;
      ChoFile:=false;
      ChoPatt:=true;
      PattFError:=false;
      ITStarted:=false;
      ITFinished:=false;
     end;

    FillComboBox(Dialog);
    SendDlgItemMessage(Dialog, IDC_TYPECOMBO, CB_SETCURSEL, GetLastPattern, 0);
    ITDD.IndexOfPattern:=GetLastPattern;
    MStart(Dialog);
    CheckDlgButton(Dialog,IDC_CHKDUP,BST_CHECKED);
    CheckForDuplicates:=true;
    CheckDlgButton(Dialog,IDC_SHOWDUP,BST_UNCHECKED);
    CheckForDuplicates:=false;
    ShowWindow(Dialog,SW_NORMAL);
    end;
  WM_COMMAND: begin
    if (HiWord(wParam)=CBN_SELCHANGE) and (LoWord(wParam)=IDC_TYPECOMBO) then  //Сделали выбор шаблона
       begin
      ITDD.ChoPatt:=true;
      ITDD.IndexOfPattern:=SendDlgItemMessage(Dialog, IDC_TYPECOMBO, CB_GETCURSEL, 0, 0);
      MStart(Dialog);
       end;
    if (HiWord(wParam)=BN_CLICKED) and (LoWord(wParam)=IDC_CHKDUP) then
     if LongBool(IsDlgButtonChecked(Dialog,IDC_CHKDUP)) then EnableWindow(GetDlgItem(Dialog, IDC_SHOWDUP), TRUE)
                                              else EnableWindow(GetDlgItem(Dialog, IDC_SHOWDUP), FALSE);
    case loword(wParam) of
      IDCLOSE:     DestroyWindow(Dialog);
      IDC_BRWSBTN: begin    //Нажали кнопку "..."
          s:=OpenDialogExecute(Dialog,0,ITDD.oFN,TxtPatterns[ITDD.IndexOfPattern].DefExtension);
          if s<>'' then ITDD.ChoFile:=true
                   else ITDD.ChoFile:=false;
          SendDlgItemMessageW(Dialog, IDC_FILENAME,WM_SETTEXT,0,dword(PWideChar(s)));
          MStart(Dialog);
          end;
      IDSTART: begin      //Старт
          with ITDD do
          if CanStart then
           begin
          EnableWindow(GetDlgItem(Dialog, IDSTART), FALSE);
          EnableWindow(GetDlgItem(Dialog, IDCLOSE), FALSE);
          EnableWindow(GetDlgItem(Dialog, IDC_TYPECOMBO), FALSE);
          EnableWindow(GetDlgItem(Dialog, IDC_BRWSBTN), FALSE);
          ImportThrd:=TImportThrd.Create(TRUE);
          ImportThrd.FreeOnTerminate:=true;
          ImportThrd.DContact.hContact:=hContact;
          CheckForDuplicates:=LongBool(IsDlgButtonChecked(Dialog,IDC_CHKDUP));
          ShowDuplicates:=LongBool(IsDlgButtonChecked(Dialog,IDC_SHOWDUP));
          h:=SendDlgItemMessageW(Dialog, IDC_FILENAME,WM_GETTEXTLENGTH,0,0)+1;
          SetLength(s,h);
          SendDlgItemMessageW(Dialog, IDC_FILENAME,WM_GETTEXT,h,dword(PWideChar(s)));
          ImportThrd.FileNames:=s+#0+#0;
          ImportThrd.OffsetFileName:=oFN;
          ImportThrd.WorkPattern:=TxtPatterns[IndexOfPattern];
          ImportThrd.ParentHWND:=Dialog;
          ImportThrd.Resume;
           end;
          end;
    end;
             end;
  WM_CLOSE: begin
    DestroyWindow(Dialog);
            end;
  //Далее оброботка сообщений от потока импорта
  //Начали
  ITXT_THREAD_ALLSTARTED:begin
     ITDD.ITStarted:=true;
     ITDD.StartTime:=Time;
     end;
  ITXT_THREAD_START:begin
     AddStatusMessage(Dialog,TranslateWideString('Import started...'));
     end;
  //Известна длинна файла выставляем диапазон прогрессбара
  ITXT_THREAD_MAXPROGRESS:
     SendDlgItemMessage(Dialog,IDC_PROGRESS,PBM_SETRANGE,0,MakeLParam(0,lParam));
  //Идет прогресс ...
  ITXT_THREAD_PROGRESS:
     SendDlgItemMessage(Dialog,IDC_PROGRESS,PBM_SETPOS,wParam,0);
  //Возникла ошибка
  ITXT_THREAD_ERROR:
     AddStatusMessage(Dialog,TranslateWideString(PWideChar(wParam)));
  //Закончили
  ITXT_THREAD_FINISH:begin
     ITDD.ITFinished:=true;
     ITDD.FinishTime:=Time;
     DecodeTime(ITDD.FinishTime-ITDD.StartTime,h,mi,sec,ms);
     AddStatusMessage(Dialog,WideFormat(TranslateWideString('Added: %d messages'),[wParam]));
     AddStatusMessage(Dialog,WideFormat(TranslateWideString('Duplicates: %d messages'),[lParam]));
     AddStatusMessage(Dialog,WideFormat(TranslateWideString('In: %d:%d:%d'),[h,mi,sec]));
     AddStatusMessage(Dialog,'');
     SendDlgItemMessageW(Dialog, IDC_FILENAME,WM_SETTEXT,0,0);
     ITDD.ChoFile:=false;
     EnableWindow(GetDlgItem(Dialog, IDC_TYPECOMBO), TRUE);
     EnableWindow(GetDlgItem(Dialog, IDC_BRWSBTN), TRUE);
     EnableWindow(GetDlgItem(Dialog, IDCLOSE), TRUE);
     end;
  //начали новый файл
  ITXT_THREAD_START_FILE:
     AddStatusMessage(Dialog,WideFormat(TranslateWideString('File: %s'),[PWideChar(wParam)]));
  //определили контакт
  ITXT_THREAD_DEST_CONTACT:begin
     if IsMirandaUnicode then
       begin
      tempwstr:=UTF8toWide(PChar(GetContactID(wParam,'',true)),tempwstr);
      s:= tempwstr;
      tempwstr:=UTF8toWide(PChar(GetContactNick(wParam,'',true)),tempwstr);
      s:=s +  ' | ' + tempwstr;
       end
                         else
      s:=GetContactID(wParam,'',true)+' | '+GetContactNick(wParam,'',true);

     AddStatusMessage(Dialog,WideFormat(TranslateWideString('To: %s'),[s]));
     end;
  end;
end;

end.