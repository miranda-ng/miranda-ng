unit iac_messagebox;

interface

implementation

uses
  editwrapper,
  windows, messages, commctrl,
  m_api, global, iac_global,
  wrapper, mirutils, common, dbsettings;

{$include i_cnst_message.inc}
{$resource iac_messagebox.res}

const
  ACF_MSG_TTL  = $00000001;
  ACF_MSG_TXT  = $00000002;

const
  opt_msgtitle = 'msgtitle';
  opt_msgtext  = 'msgtext';
  opt_boxopts  = 'boxopts';
const
  ioTitle       = 'title';
  ioText        = 'text';
  ioType        = 'type';
  ioArgVariable = 'argvariables';
  ioVariables   = 'variables';
type
  tMessageAction = class(tBaseAction)
  private
    msgtitle:pWideChar;
    msgtext :pWideChar;
    boxopts :uint;
  public
    constructor Create(uid:dword);
    destructor Destroy; override;
//    function  Clone:tBaseAction; override;
    function  DoAction(var WorkData:tWorkData):LRESULT; override;
    procedure Save(node:pointer;fmt:integer); override;
    procedure Load(node:pointer;fmt:integer); override;
  end;

//----- Object realization -----

constructor tMessageAction.Create(uid:dword);
begin
  inherited Create(uid);

  msgtext :=nil;
  msgtitle:=nil;
  boxopts :=0;
end;

destructor tMessageAction.Destroy;
begin
  mFreeMem(msgtitle);
  mFreeMem(msgtext);

  inherited Destroy;
end;
{
function tMessageAction.Clone:tBaseAction;
begin
  result:=tMessageAction.Create(0);
  Duplicate(result);

  StrDupW(tMessageAction(result).msgtext ,msgtext);
  StrDupW(tMessageAction(result).msgtitle,msgtitle);
  tMessageAction(result).boxopts:=boxopts;
end;
}
function tMessageAction.DoAction(var WorkData:tWorkData):LRESULT;
var
  i:integer;
  buf:array [0..31] of WideChar;
  tmpc:pWideChar;          // LastResult value
  tmpc1,tmpc2:pWideChar;   // title/text after LastResult insertion
  tmpcv1,tmpcv2:pWideChar; // title/text after Variables processing 
begin
  result:=0;

  if WorkData.ResultType=rtWide then
    tmpc:=pWidechar(WorkData.LastResult)
  else
  begin
    IntToStr(buf,WorkData.LastResult);
    tmpc:=@buf;
  end;
  // LastResult
  if StrPosW(msgtitle,'<last>')<>nil then
  begin
    mGetMem(tmpc1,8192);
    StrCopyW(tmpc1,msgtitle);
    StrReplaceW(tmpc1,'<last>',tmpc);
  end
  else
    tmpc1:=msgtitle;
  if StrPosW(msgtext,'<last>')<>nil then
  begin
    mGetMem(tmpc2,8192);
    StrCopyW(tmpc2,msgtext);
    StrReplaceW(tmpc2,'<last>',tmpc);
  end
  else
    tmpc2:=msgtext;
  // Variables
  if (flags and ACF_MSG_TTL)<>0 then
    tmpcv1:=ParseVarString(tmpc1,WorkData.Parameter,tmpc)
  else
    tmpcv1:=tmpc1;
  if (flags and ACF_MSG_TXT)<>0 then
    tmpcv2:=ParseVarString(tmpc2,WorkData.Parameter,tmpc)
  else
    tmpcv2:=tmpc2;

  i:=MessageBoxW(0,tmpcv2,tmpcv1,boxopts);

  // Keep old result just if has single OK button
  if (boxopts and $0F)<>MB_OK then
  begin
    ClearResult(WorkData);

    WorkData.ResultType:=rtInt;
    WorkData.LastResult:=i;
  end;

  if tmpcv1<>tmpc1    then mFreeMem(tmpcv1);
  if tmpcv2<>tmpc2    then mFreeMem(tmpcv2);
  if tmpc1 <>msgtitle then mFreeMem(tmpc1);
  if tmpc2 <>msgtext  then mFreeMem(tmpc2);
end;

procedure tMessageAction.Load(node:pointer;fmt:integer);
var
  section: array [0..127] of AnsiChar;
  pc: pAnsiChar;
begin
  inherited Load(node,fmt);
  case fmt of
    0: begin
      pc:=StrCopyE(section,pAnsiChar(node));
      StrCopy(pc,opt_msgtitle); msgtitle:=DBReadUnicode(0,DBBranch,section);
      StrCopy(pc,opt_msgtext ); msgtext :=DBReadUnicode(0,DBBranch,section);
      StrCopy(pc,opt_boxopts ); boxopts :=DBReadDword  (0,DBBranch,section); // v2 = byte
    end;

    1: begin
      with xmlparser do
      begin
        StrDupW(msgtitle,getAttrValue(HXML(node),ioTitle));
        StrDupW(msgtext,getText(HXML(node)));
        boxopts:=StrToInt(getAttrValue(HXML(node),ioType));

        if StrToInt(getAttrValue(HXML(node),ioArgVariable))=1 then flags:=flags or ACF_MSG_TXT;
        if StrToInt(getAttrValue(HXML(node),ioVariables  ))=1 then flags:=flags or ACF_MSG_TTL;
      end;
    end;
{
    2: begin
      UTF8ToWide(GetParamSectionInt(node,ioTitle),msgtitle);
      UTF8ToWide(GetParamSectionInt(node,ioText ),msgtext);
      boxopts:=GetParamSectionInt(node,ioType);

      if GetParamSectionInt(node,ioArgVariable)=1 then flags:=flags or ACF_MSG_TXT;
      if GetParamSectionInt(node,ioVariables  )=1 then flags:=flags or ACF_MSG_TTL;
    end;
}
  end;
end;

procedure tMessageAction.Save(node:pointer;fmt:integer);
var
  section: array [0..127] of AnsiChar;
  pc: pAnsiChar;
begin
  inherited Save(node,fmt);
  case fmt of
    0:  begin
      pc:=StrCopyE(section,pAnsiChar(node));
      StrCopy(pc,opt_msgtitle); DBWriteUnicode(0,DBBranch,section,msgtitle);
      StrCopy(pc,opt_msgtext ); DBWriteUnicode(0,DBBranch,section,msgtext);
      StrCopy(pc,opt_boxopts ); DBWriteDWord  (0,DBBranch,section,boxopts);
    end;
{
    1: begin
    end;
}
    13: begin
    end;
  end;
end;

//----- Dialog realization -----

procedure SetMBRadioIcon(Dialog:HWND;h:THANDLE;id:dword;icon:uint_ptr);
begin
  SendDlgItemMessage(Dialog,id,BM_SETIMAGE,IMAGE_ICON,
    LoadImage(h,MAKEINTRESOURCE(icon),IMAGE_ICON,16,16,0{LR_SHARED}));
//  SendDlgItemMessage(Dialog,id,BM_SETIMAGE,IMAGE_ICON,LoadIcon(0,icon));
end;

procedure SetMBRadioIcons(Dialog:HWND);
var
  h:THANDLE;
begin
  h:=LoadLibrary('user32.dll');
//  SetMBRadioIcon(IDC_MSGI_NONE,IDI_); //?
  SetMBRadioIcon(Dialog,h,IDC_MSGI_ERROR,103{IDI_HAND});
  SetMBRadioIcon(Dialog,h,IDC_MSGI_QUEST,102{IDI_QUESTION});
  SetMBRadioIcon(Dialog,h,IDC_MSGI_WARN ,101{IDI_EXCLAMATION});
  SetMBRadioIcon(Dialog,h,IDC_MSGI_INFO ,104{IDI_ASTERISK});
  FreeLibrary(h);
end;

procedure ClearFields(Dialog:HWND);
begin
  CheckDlgButton(Dialog,IDC_MSG_RTL  ,BST_UNCHECKED);
  CheckDlgButton(Dialog,IDC_MSG_RIGHT,BST_UNCHECKED);

  CheckDlgButton(Dialog,IDC_MSGB_OK ,BST_UNCHECKED);
  CheckDlgButton(Dialog,IDC_MSGB_OC ,BST_UNCHECKED);
  CheckDlgButton(Dialog,IDC_MSGB_ARI,BST_UNCHECKED);
  CheckDlgButton(Dialog,IDC_MSGB_YNC,BST_UNCHECKED);
  CheckDlgButton(Dialog,IDC_MSGB_YN ,BST_UNCHECKED);
  CheckDlgButton(Dialog,IDC_MSGB_RC ,BST_UNCHECKED);

  CheckDlgButton(Dialog,IDC_MSGI_NONE ,BST_UNCHECKED);
  CheckDlgButton(Dialog,IDC_MSGI_ERROR,BST_UNCHECKED);
  CheckDlgButton(Dialog,IDC_MSGI_QUEST,BST_UNCHECKED);
  CheckDlgButton(Dialog,IDC_MSGI_WARN ,BST_UNCHECKED);
  CheckDlgButton(Dialog,IDC_MSGI_INFO ,BST_UNCHECKED);
end;

function DlgProc(Dialog:HWND;hMessage:uint;wParam:WPARAM;lParam:LPARAM):LRESULT; stdcall;
const
  NoProcess:boolean=true;
begin
  result:=0;

  case hMessage of
    WM_INITDIALOG: begin
      TranslateDialogDefault(Dialog);

      MakeEditField(Dialog,IDC_MSG_TITLE);
      MakeEditField(Dialog,IDC_MSG_TEXT);

      SetMBRadioIcons(Dialog);
    end;

    WM_ACT_SETVALUE: begin
      NoProcess:=true;
      ClearFields(Dialog);
      with tMessageAction(lParam) do
      begin
        SetDlgItemTextW(Dialog,IDC_MSG_TITLE,msgtitle);
        SetDlgItemTextW(Dialog,IDC_MSG_TEXT ,msgtext);

        SetEditFlags(Dialog,IDC_MSG_TITLE,EF_SCRIPT,ord((flags and ACF_MSG_TTL)<>0));
        SetEditFlags(Dialog,IDC_MSG_TEXT ,EF_SCRIPT,ord((flags and ACF_MSG_TXT)<>0));

        if (boxopts and MB_RTLREADING)<>0 then CheckDlgButton(Dialog,IDC_MSG_RTL  ,BST_CHECKED);
        if (boxopts and MB_RIGHT     )<>0 then CheckDlgButton(Dialog,IDC_MSG_RIGHT,BST_CHECKED);

        case boxopts and $0F of
          MB_OKCANCEL         : CheckDlgButton(Dialog,IDC_MSGB_OC ,BST_CHECKED);
          MB_ABORTRETRYIGNORE : CheckDlgButton(Dialog,IDC_MSGB_ARI,BST_CHECKED);
          MB_YESNOCANCEL      : CheckDlgButton(Dialog,IDC_MSGB_YNC,BST_CHECKED);
          MB_YESNO            : CheckDlgButton(Dialog,IDC_MSGB_YN ,BST_CHECKED);
          MB_RETRYCANCEL      : CheckDlgButton(Dialog,IDC_MSGB_RC ,BST_CHECKED);
//          MB_CANCELTRYCONTINUE:
        else
          CheckDlgButton(Dialog,IDC_MSGB_OK,BST_CHECKED);
        end;
        case boxopts and $F0 of
          MB_ICONERROR      : CheckDlgButton(Dialog,IDC_MSGI_ERROR,BST_CHECKED);
          MB_ICONQUESTION   : CheckDlgButton(Dialog,IDC_MSGI_QUEST,BST_CHECKED);
          MB_ICONWARNING    : CheckDlgButton(Dialog,IDC_MSGI_WARN ,BST_CHECKED);
          MB_ICONINFORMATION: CheckDlgButton(Dialog,IDC_MSGI_INFO ,BST_CHECKED);
        else
          CheckDlgButton(Dialog,IDC_MSGI_NONE,BST_CHECKED);
        end;
      end;
      NoProcess:=false;
    end;

    WM_ACT_RESET: begin
      NoProcess:=true;
      ClearFields(Dialog);

      SetDlgItemTextW(Dialog,IDC_MSG_TITLE,nil);
      SetDlgItemTextW(Dialog,IDC_MSG_TEXT ,nil);
      SetEditFlags(Dialog,IDC_MSG_TITLE,EF_ALL,0);
      SetEditFlags(Dialog,IDC_MSG_TEXT ,EF_ALL,0);

      CheckDlgButton(Dialog,IDC_MSGB_OK  ,BST_CHECKED);
      CheckDlgButton(Dialog,IDC_MSGI_NONE,BST_CHECKED);
      NoProcess:=false;
    end;

    WM_ACT_SAVE: begin
      with tMessageAction(lParam) do
      begin
        {mFreeMem(msgtitle); }msgtitle:=GetDlgText(Dialog,IDC_MSG_TITLE);
        {mFreeMem(msgtext ); }msgtext :=GetDlgText(Dialog,IDC_MSG_TEXT);

        if (GetEditFlags(Dialog,IDC_MSG_TITLE) and EF_SCRIPT)<>0 then flags:=flags or ACF_MSG_TTL;
        if (GetEditFlags(Dialog,IDC_MSG_TEXT ) and EF_SCRIPT)<>0 then flags:=flags or ACF_MSG_TXT;

        if IsDlgButtonChecked(Dialog,IDC_MSG_RTL  )=BST_CHECKED then boxopts:=boxopts or MB_RTLREADING;
        if IsDlgButtonChecked(Dialog,IDC_MSG_RIGHT)=BST_CHECKED then boxopts:=boxopts or MB_RIGHT;
        
        if      IsDlgButtonChecked(Dialog,IDC_MSGB_OC )=BST_CHECKED then boxopts:=boxopts or MB_OKCANCEL
        else if IsDlgButtonChecked(Dialog,IDC_MSGB_ARI)=BST_CHECKED then boxopts:=boxopts or MB_ABORTRETRYIGNORE
        else if IsDlgButtonChecked(Dialog,IDC_MSGB_YNC)=BST_CHECKED then boxopts:=boxopts or MB_YESNOCANCEL
        else if IsDlgButtonChecked(Dialog,IDC_MSGB_YN )=BST_CHECKED then boxopts:=boxopts or MB_YESNO
        else if IsDlgButtonChecked(Dialog,IDC_MSGB_RC )=BST_CHECKED then boxopts:=boxopts or MB_RETRYCANCEL
        else{if IsDlgButtonChecked(Dialog,IDC_MSGB_OK )=BST_CHECKED then}boxopts:=boxopts or MB_OK;

        if      IsDlgButtonChecked(Dialog,IDC_MSGI_ERROR)=BST_CHECKED then boxopts:=boxopts or MB_ICONHAND
        else if IsDlgButtonChecked(Dialog,IDC_MSGI_QUEST)=BST_CHECKED then boxopts:=boxopts or MB_ICONQUESTION
        else if IsDlgButtonChecked(Dialog,IDC_MSGI_WARN )=BST_CHECKED then boxopts:=boxopts or MB_ICONWARNING
        else if IsDlgButtonChecked(Dialog,IDC_MSGI_INFO )=BST_CHECKED then boxopts:=boxopts or MB_ICONINFORMATION
        ;//else if IsDlgButtonChecked(Dialog,IDC_MSGI_NONE)=BST_CHECKED then ;
      end;
    end;

    WM_COMMAND: begin
      case wParam shr 16 of
        EN_CHANGE,
        BN_CLICKED: if not NoProcess then
            SendMessage(GetParent(GetParent(Dialog)),PSM_CHANGED,0,0);
      end;
    end;

    WM_HELP: begin
      MessageBoxW(0,
        TranslateW(
          'Text <last> replacing'#13#10+
          'by last result'#13#10#13#10+
          'Returns:'#13#10+
          '--------'#13#10+
          'OK'#9'= 1'#13#10+
          'CANCEL'#9'= 2'#13#10+
          'ABORT'#9'= 3'#13#10+
          'RETRY'#9'= 4'#13#10+
          'IGNORE'#9'= 5'#13#10+
          'YES'#9'= 6'#13#10+
          'NO'#9'= 7'#13#10+
          'CLOSE'#9'= 8'),
        TranslateW('MessageBox'),0);
      result:=1;
    end;
  end;
end;

//----- Export/interface functions -----

var
  vc:tActModule;

function CreateAction:tBaseAction;
begin
  result:=tMessageAction.Create(vc.Hash);
end;

function CreateDialog(parent:HWND):HWND;
begin
  result:=CreateDialogW(hInstance,'IDD_ACTMESSAGEBOX',parent,@DlgProc);
end;

procedure Init;
begin
  vc.Next    :=ModuleLink;

  vc.Name    :='MessageBox';
  vc.Dialog  :=@CreateDialog;
  vc.Create  :=@CreateAction;
  vc.Icon    :='IDI_MESSAGE';
  vc.Hash    :=0;

  ModuleLink :=@vc;
end;

begin
  Init;
end.
