{Templates}

procedure SetScreenLite(Dialog:HWND);
var
  p:pWideChar;
begin
  Changed:=Changed or DLGED_INIT;

  p:=GetTemplateStr(tmpl_pm    ,0,0);
  SetDlgItemTextW(Dialog,IDC_EDIT_MSG     ,p);
  p:=GetTemplateStr(tmpl_xtitle,0,0);
  SetDlgItemTextW(Dialog,IDC_XSTATUS_TITLE,p);
  p:=GetTemplateStr(tmpl_stext ,0,0);
  SetDlgItemTextW(Dialog,IDC_STATUS_TEXT  ,p);
  p:=GetTemplateStr(tmpl_chat  ,0,0);
  SetDlgItemTextW(Dialog,IDC_EDIT_CHANNEL ,p);

  Changed:=Changed and not DLGED_INIT;
end;

procedure SetTemplateLite(Dialog:HWND;idc:integer;Tmpl:tTemplateType);
begin
  SetTemplateStr(GetDlgText(Dialog,idc),Tmpl,0,0);
end;

procedure SaveChangesLite(Dialog:HWND);
begin
  if (Changed and DLGED_BASE)<>0 then
  begin
    if (Changed and DLGED_MSG )<>0 then SetTemplateLite(Dialog,IDC_EDIT_MSG    ,tmpl_pm);
    if (Changed and DLGED_CHNL)<>0 then SetTemplateLite(Dialog,IDC_EDIT_CHANNEL,tmpl_chat);
    if (Changed and DLGED_XTTL)<>0 then
    begin
      SetTemplateLite(Dialog,IDC_XSTATUS_TITLE,tmpl_xtitle);
    end;
    if (Changed and DLGED_STTT)<>0 then
    begin
      SetTemplateLite(Dialog,IDC_STATUS_TEXT,tmpl_stext);
      SetTemplateLite(Dialog,IDC_STATUS_TEXT,tmpl_xtext);
      SetTemplateLite(Dialog,IDC_STATUS_TEXT,tmpl_tunes);
    end;

    Changed:=Changed and (not DLGED_BASE);
    SaveTemplates;
  end;
end;

function DlgProcOptions12(Dialog:HWND;hMessage:uint;wParam:WPARAM;lParam:LPARAM):LRESULT; stdcall;
begin
  result:=0;
  case hMessage of
    WM_INITDIALOG: begin
      TranslateDialogDefault(Dialog);
      SetScreenLite(Dialog);
      result:=0;
      Changed:=0;
    end;

    WM_COMMAND: begin
      if (Changed and DLGED_INIT)=0 then
      begin
        case wParam shr 16 of
          EN_CHANGE: begin
            Changed:=Changed or DLGED_CHGD or DLGED_PACK;
            case loword(wParam) of
              IDC_EDIT_MSG      : Changed:=Changed or DLGED_MSG;
              IDC_XSTATUS_TITLE : Changed:=Changed or DLGED_XTTL;
              IDC_STATUS_TEXT   : Changed:=Changed or DLGED_STTT;
              IDC_EDIT_CHANNEL  : Changed:=Changed or DLGED_CHNL;
            end;
          end;
          BN_CLICKED: begin
            case LoWord(wParam) of
              IDC_CMD_RESET: begin
                SetScreenLite(Dialog);
              end;
              IDC_HELP_COLOR: begin
                ShowColorHelpDlg(Dialog);
                exit;
              end;
              IDC_HELP_FORMAT: begin
                MessageBoxW(0,TranslateW(sFormatHelp),TranslateW('Format text Info'),0);
                exit;
              end;
              IDC_HELP_VARIABLES: begin
                CallService(MS_WAT_MACROHELP,Dialog,0);
                exit;
              end;
            end;
          end;
        else
          exit;
        end;
        SendMessage(GetParent(Dialog),PSM_CHANGED,0,0);
      end;
    end;

    WM_HELP: CallService(MS_WAT_MACROHELP,Dialog,0);

    WM_NOTIFY: begin
      if integer(PNMHdr(lParam)^.code)=PSN_APPLY then
        SaveChangesLite(Dialog);
    end;

  else
    {result:=}DefWindowProc(Dialog,hMessage,wParam,lParam);
  end;
end;
