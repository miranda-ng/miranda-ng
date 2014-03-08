unit iac_settings;

interface

implementation

uses
  windows, messages, commctrl,
  global,iac_global, dlgshare, lowlevelc,
  m_api, mirutils, dbsettings, common, wrapper;

{$include i_cnst_settings.inc}
{$resource iac_settings.res}


//----- Support functions -----


//----- Dialog realization -----

procedure FillServiceModeList(wnd:HWND);
begin
  SendMessage(wnd,CB_RESETCONTENT,0,0);
  InsertString(wnd,0 ,'value');
  InsertString(wnd,1 ,'name');
  InsertString(wnd,2 ,'value (name)');
  InsertString(wnd,3 ,'name ''value''');
  SendMessage(wnd,CB_SETCURSEL,0,0);
end;

procedure ClearFields(Dialog:HWND);
begin
  CheckDlgButton(Dialog,IDC_CNT_FILTER,BST_UNCHECKED);
  SetDlgItemTextW(Dialog,IDC_EDIT_FORMAT,'');

  CheckDlgButton(Dialog,IDC_FR_FLAG,BST_UNCHECKED);
end;

function DlgProc(Dialog:HWND;hMessage:uint;wParam:WPARAM;lParam:LPARAM):LRESULT; stdcall;
var
  fCLformat:pWideChar;
  lp:TLPARAM;
  stat:integer;
begin
  result:=0;

  case hMessage of
    WM_INITDIALOG: begin
      TranslateDialogDefault(Dialog);

      OptSetButtonIcon(GetDlgItem(Dialog,IDC_CNT_APPLY),ACI_APPLY);

      OptSetButtonIcon(GetDlgItem(Dialog,IDC_SRV_APPLY),ACI_APPLY);
      FillServiceModeList(GetDlgItem(Dialog,IDC_SERVICELIST));
    end;

    WM_ACT_SETVALUE: begin
      ClearFields(Dialog);
    end;

    WM_ACT_RESET: begin
      ClearFields(Dialog);
      // Contact list settings
      CheckDlgButton (Dialog,IDC_CNT_FILTER,DBReadByte(0,DBBranch,'CLfilter',BST_UNCHECKED));
      fCLformat:=DBReadUnicode(0,DBBranch,'CLformat');
      SetDlgItemTextW(Dialog,IDC_EDIT_FORMAT,fCLformat);
      mFreeMem(fCLformat);

      // Servicelist mode settings
      CB_SelectData(Dialog,IDC_SERVICELIST,DBReadByte(0,DBBranch,'SrvListMode'));
    end;

    WM_SHOWWINDOW: begin
      // Show window by ShowWindow function
      if (lParam=0) and (wParam=1) then
      begin
        lp:=LV_GetLParam(MacroListWindow);
        if (lp and ACF_FIRSTRUN)<>0 then
          stat:=BST_CHECKED
        else
          stat:=BST_UNCHECKED;
          CheckDlgButton(Dialog,IDC_FR_FLAG,stat);
      end;
    end;

    WM_COMMAND: begin
      case wParam shr 16 of
        BN_CLICKED: begin
          case loword(wParam) of
            IDC_SRV_APPLY: begin
              DBWriteByte(0,DBBranch,'SrvListMode',
                  CB_GetData(GetDlgItem(Dialog,IDC_SERVICELIST)));
            end;

            IDC_CNT_APPLY: begin
              fCLformat:=GetDlgText(Dialog,IDC_EDIT_FORMAT);
              DBWriteUnicode(0,DBBranch,'CLformat',fCLformat);
              mFreeMem(fCLformat);
            end;

            IDC_CNT_FILTER: begin
              DBWriteByte(0,DBBranch,'CLfilter',IsDlgButtonChecked(Dialog,IDC_CNT_FILTER));
            end;

            IDC_FR_FLAG: begin
              lp:=LV_GetLParam(MacroListWindow);
              if IsDlgButtonChecked(Dialog,IDC_FR_FLAG)=BST_UNCHECKED then
                lp:=lp and not ACF_FIRSTRUN
              else
                lp:=lp or ACF_FIRSTRUN;
              LV_SetLParam(MacroListWindow,lp);

              SendMessage(GetParent(GetParent(Dialog)),PSM_CHANGED,0,0);
            end;
          end;
        end;
      end;
    end;

    WM_HELP: begin
      result:=1;
    end;

  end;
end;

//----- Export/interface functions -----

var
  vc:tActModule;

function CreateAction:tBaseAction;
begin
  result:=nil;
end;

function CreateDialog(parent:HWND):HWND;
begin
  result:=CreateDialogW(hInstance,'IDD_SETTINGS',parent,@DlgProc);
end;

procedure Init;
begin
  vc.Next    :=ModuleLink;

  vc.Name    :='Settings';
  vc.Dialog  :=@CreateDialog;
  vc.Create  :=@CreateAction;
  vc.Icon    :='IDI_SETTINGS';
  vc.Hash    :=1;

  ModuleLink :=@vc;
end;

begin
  Init;
end.
