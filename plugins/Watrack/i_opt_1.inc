{common options}
const
  ChkFmtStr:array [0..3] of pWideChar = ('None','Audio','Video','All');

type
  TCPTABLE = packed record
    cpId  :cardinal;
    cpName:PAnsiChar;
  end;
const
  cpNum = 15;
  cpTable:array [0..cpNum-1] of TCPTABLE = (
    (cpId: 874;cpName:'Thai'),
    (cpId: 932;cpName:'Japanese'),
    (cpId: 936;cpName:'Simplified Chinese'),
    (cpId: 949;cpName:'Korean'),
    (cpId: 950;cpName:'Traditional Chinese'),
    (cpId:1250;cpName:'Central European'),
    (cpId:1251;cpName:'Cyrillic'),
    (cpId:1252;cpName:'Latin I'),
    (cpId:1253;cpName:'Greek'),
    (cpId:1254;cpName:'Turkish'),
    (cpId:1255;cpName:'Hebrew'),
    (cpId:1256;cpName:'Arabic'),
    (cpId:1257;cpName:'Baltic'),
    (cpId:1258;cpName:'Vietnamese'),
    (cpId:1361;cpName:'Korean (Johab)'));

var
  hCpCombo:HWND;

function FillCpCombo(astr:PAnsiChar):boolean; stdcall;
var
  i:integer;
  cp:cardinal;
  iIndex:integer;
  buf:array [0..63] of WideChar;
begin
  result:=true; // MUST be at start
  cp:=StrToInt(astr);
  i:=0;
  while i<cpNum do
  begin
    if cpTable[i].cpId=cp then
    begin
      iIndex:=SendMessageW(hCpCombo,CB_ADDSTRING,0,
        lparam(TranslateW(FastAnsiToWideBuf(cpTable[i].cpName,buf))));
      SendMessage(hCpCombo,CB_SETITEMDATA,iIndex,cpTable[i].cpId);
      break;
    end;
    inc(i);
  end;
end;

function DlgProcOptions1(Dialog:HWND;hMessage:uint;wParam:WPARAM;lParam:LPARAM):LRESULT; stdcall;
const
  btnChkPlayer:bool=false;
  btnChkFormat:integer=0;
var
  tmp:longbool;
  i,j:cardinal;
  wnd:HWND;
  pldescr:array [0..27] of AnsiChar;
  pldescw:array [0..27] of WideChar;
  p:pWideChar;
begin
  result:=0;
  case hMessage of
    WM_INITDIALOG: begin
      SetWindowTextW(GetDlgItem(Dialog,IDC_CHK_PLAYER),'None');
      SetWindowTextW(GetDlgItem(Dialog,IDC_CHK_FORMAT),ChkFmtStr[0]);
      TranslateDialogDefault(Dialog);
      DefFillPlayerList(GetDlgItem(Dialog,IDC_PLAYERLIST));
      DefFillFormatList(GetDlgItem(Dialog,IDC_FORMATLIST));

      i:=mTimer;
      if (i>=1000) and (i mod 1000=0) then
        i:=i div 1000;

      MakeHint(Dialog,IDC_TIMER,
        'Refresh time (sec) is time to refresh music info, statistic and'+
        ' status messages. If zero, automatic refresh is disabled. If '+
        'value greater than 499, time signify as milliseconds.');
      SetDlgItemInt(Dialog,IDC_TIMER,i,false);

      MakeHint(Dialog,IDC_CHECKTIME,
        'Check file date and time to tag updates while playing.');
      CheckDlgButton(Dialog,IDC_CHECKTIME,CheckTime);

      MakeHint(Dialog,IDC_IMPLANTANT,
        'Use player process injection to obtain info easier. Can provoke antivirus '+
        'or firewall alarm.');
      CheckDlgButton(Dialog,IDC_IMPLANTANT,UseImplant);

      MakeHint(Dialog,IDC_KEEPOLD,
        'Keep opened file as active, not newly founded.');
      CheckDlgButton(Dialog,IDC_KEEPOLD,KeepOld);

      MakeHint(Dialog,IDC_CHECKALL,
        'Check all marked players for active (started and playing) or stop at first founded');
      CheckDlgButton(Dialog,IDC_CHECKALL,CheckAll);

      MakeHint(Dialog,IDC_CHECKUNKN,
        'Check unknown file formats if player gives info.');
      CheckDlgButton(Dialog,IDC_CHECKUNKN,CheckUnknown);

//      MakeHint(Dialog,IDC_COVERFN,
//        'Cover filename searching templates');
      SetDlgItemTextW(Dialog,IDC_COVERFN,CoverPaths);

      MakeHint(Dialog,IDC_APPCOMMAND,
        'Emulate multimedia keys presses to control palyer');
      CheckDlgButton(Dialog,IDC_APPCOMMAND,mmkeyemu);

      hCpCombo:=GetDlgItem(Dialog,IDC_CODEPAGE);
      EnumSystemCodePages(@FillCpCombo,CP_INSTALLED);
      SendDlgItemMessageW(Dialog,IDC_CODEPAGE,CB_INSERTSTRING,0,
        tlparam(TranslateW('System default codepage')));

      CB_SelectData(Dialog,IDC_CODEPAGE,UserCP);

      if UserCP=0 then
        i:=0
      else
      begin
        i:=SendDlgItemMessage(Dialog,IDC_CODEPAGE,CB_GETCOUNT,0,0)-1;
        while i>0 do
        begin
          if dword(SendDlgItemMessage(Dialog,IDC_CODEPAGE,CB_GETITEMDATA,twparam(i),0))=UserCP then
            break;
          dec(i);
        end
      end;
      SendDlgItemMessage(Dialog,IDC_CODEPAGE,CB_SETCURSEL,i,0);

      result:=0;
    end;

    WM_HELP: begin
      with pHelpInfo(lParam)^ do
      begin
        if (iContextType=HELPINFO_WINDOW) and (iCtrlId=IDC_PLAYERLIST) then
        begin
          ListView_GetItemTextA(hItemHandle,
              SendMessage(hItemHandle,LVM_GETNEXTITEM,-1,LVNI_FOCUSED),0,
              @pldescr,HIGH(pldescr));
          p:=GetPlayerNote(pldescr);
          if p=nil then
            p:='No any special notes for this player';
          MessageBoxW(0,TranslateW(p),FastAnsiToWideBuf(pldescr,pldescw),0);
        end;
      end;
    end;

    WM_COMMAND: begin
      case wParam shr 16 of
        EN_CHANGE,
        BN_CLICKED,
        CBN_SELCHANGE: begin
          SendMessage(GetParent(Dialog),PSM_CHANGED,0,0);
          result:=1;
        end;
      end;
      if (wParam shr 16)=BN_CLICKED then
      begin
        case loword(wParam) of
          IDC_CHK_PLAYER: begin
            btnChkPlayer:=not btnChkPlayer;
            if btnChkPlayer then
            begin
              SetWindowTextW(GetDlgItem(Dialog,IDC_CHK_PLAYER),TranslateW('All'));
              tmp:=false;
            end
            else
            begin
              SetWindowTextW(GetDlgItem(Dialog,IDC_CHK_PLAYER),TranslateW('None'));
              tmp:=true;
            end;
            wnd:=GetDlgItem(Dialog,IDC_PLAYERLIST);
            j:=ListView_GetItemCount(wnd)-1;
            for i:=0 to j do
              ListView_SetCheckState(wnd,i,tmp);
          end;

          IDC_CHK_FORMAT: begin
            wnd:=GetDlgItem(Dialog,IDC_FORMATLIST);
            j:=ListView_GetItemCount(wnd)-1;

            tmp:=btnChkFormat=3;
            for i:=0 to j do
            begin
              if (btnChkFormat=1) or (btnChkFormat=2) then
              begin
                if (LV_GetLParam(wnd,i) and WAT_OPT_VIDEO)<>0 then
                  tmp:=btnChkFormat=2
                else
                  tmp:=btnChkFormat=1;
              end;
              ListView_SetCheckState(wnd,i,tmp);
            end;
            inc(btnChkFormat);
            if btnChkFormat=4 then btnChkFormat:=0;
            SetWindowTextW(GetDlgItem(Dialog,IDC_CHK_FORMAT),TranslateW(ChkFmtStr[btnChkFormat]));
          end;
        end;
      end;
    end;

    WM_NOTIFY: begin
      case integer(PNMHdr(lParam)^.code) of
        LVN_ITEMCHANGED: begin
          SendMessage(GetParent(Dialog),PSM_CHANGED,0,0);
        end;

        PSN_APPLY: begin
          UserCP:=CB_GetData(GetDlgItem(Dialog,IDC_CODEPAGE));
          i:=GetDlgItemInt(Dialog,IDC_TIMER,tmp,false);
          if i<500 then
            i:=i*1000;
          if i<>mTimer then
          begin
            mTimer:=i;
            StopTimer;
            StartTimer;
          end;

{
          TimeoutForThread:=GetDlgItemInt(Dialog,IDC_TIMEOUT,tmp,false);
          if TimeoutForThread>=100 then
            TimeoutForThread:=SysWin.ThreadTimeout;
}
          CheckTime   :=IsDlgButtonChecked(Dialog,IDC_CHECKTIME);
          UseImplant  :=IsDlgButtonChecked(Dialog,IDC_IMPLANTANT);
          KeepOld     :=IsDlgButtonChecked(Dialog,IDC_KEEPOLD);
          mmkeyemu    :=IsDlgButtonChecked(Dialog,IDC_APPCOMMAND);
          CheckAll    :=IsDlgButtonChecked(Dialog,IDC_CHECKALL);
          CheckUnknown:=IsDlgButtonChecked(Dialog,IDC_CHECKUNKN);

          mFreeMem(CoverPaths);
          CoverPaths:=GetDlgText(Dialog,IDC_COVERFN);

          DefCheckPlayerList(GetDlgItem(Dialog,IDC_PLAYERLIST));
          DefCheckFormatList(GetDlgItem(Dialog,IDC_FORMATLIST));
          saveopt;

          result:=1;
        end;
      end;
    end;
  else
    {result:=}DefWindowProc(Dialog,hMessage,wParam,lParam);
  end;
end;
