unit iac_contact;

interface

implementation

uses
  windows, messages, commctrl,
  m_api, global, iac_global, common,
  mircontacts, dlgshare, syswin, inouttext, base64,
  wrapper, mirutils, dbsettings;

{$include i_cnst_contact.inc}
{$resource iac_contact.res}

const
  ACF_KEEPONLY  = $00000001; // keep contact handle in Last, don't show window
  ACF_GETACTIVE = $00000002; // try to get contact from active window

const
  ioKeepOnly = 'keeponly';
  ioWindow   = 'window';

type
  tContactAction = class(tBaseAction)
  private
    contact:TMCONTACT;
  public
    constructor Create(uid:dword);
//    function  Clone:tBaseAction; override;
    function  DoAction(var WorkData:tWorkData):LRESULT; override;
    procedure Save(node:pointer;fmt:integer); override;
    procedure Load(node:pointer;fmt:integer); override;
  end;

//----- Support functions -----

constructor tContactAction.Create(uid:dword);
begin
  inherited Create(uid);

  contact:=0;
end;
{
function tContactAction.Clone:tBaseAction;
begin
  result:=tContactAction.Create(0);
  Duplicate(result);

  tContactAction(result).contact:=contact;
end;
}

//----- Object realization -----

function tContactAction.DoAction(var WorkData:tWorkData):LRESULT;
var
  wnd:HWND;
begin
  ClearResult(WorkData);

  if (flags and ACF_GETACTIVE)<>0 then
  begin
    wnd:=WaitFocusedWndChild(GetForegroundWindow){GetFocus};
    if wnd<>0 then
      contact:=WndToContact(wnd)
    else
      contact:=0;
  end
  else if (flags and ACF_KEEPONLY)=0 then
    ShowContactDialog(contact);

  WorkData.LastResult:=contact;
  WorkData.ResultType:=rtInt;

  result:=0;
end;

procedure tContactAction.Load(node:pointer;fmt:integer);
begin
  inherited Load(node,fmt);
  case fmt of
    0: if (flags and ACF_GETACTIVE)=0 then
      contact:=LoadContact(DBBranch,node);

    1: begin
      with xmlparser do
      begin
        contact:=ImportContact(HXML(node));
        if StrToInt(getAttrValue(HXML(node),ioKeepOnly))=1 then
          flags:=flags or ACF_KEEPONLY;
        if StrToInt(getAttrValue(HXML(node),ioWindow))=1 then
          flags:=flags or ACF_GETACTIVE;
      end;
    end;
{
    2: begin
      contact:=ImportContactINI(node);

      if GetParamSectionInt(node,ioKeepOnly)=1 then
        flags:=flags or ACF_KEEPONLY;
      if GetParamSectionInt(node,ioWindow)=1 then
        flags:=flags or ACF_GETACTIVE;
    end;
}
  end;
end;

function ExportContactText(node:tTextExport;hContact:TMCONTACT):integer;
var
  proto,uid:pAnsiChar;
  cws:TDBVARIANT;
  p1:pAnsiChar;
  p:pWideChar;
  tmpbuf:array [0..63] of WideChar;
  is_chat:boolean;
begin
  result:=0;
  proto:=GetContactProtoAcc(hContact);
  if proto<>nil then
  begin
    is_chat:=IsChat(hContact);
    if is_chat then
    begin
      p:=DBReadUnicode(hContact,proto,'ChatRoomID');
      node.AddTextW('id',p);
      mFreeMem(p);
      result:=1;
    end
    else
    begin
      uid:=pAnsiChar(CallProtoService(proto,PS_GETCAPS,PFLAG_UNIQUEIDSETTING,0));
      if DBReadSetting(hContact,proto,uid,@cws)=0 then
      begin
        result:=1;
        node.AddDWord('ctype',cws._type);
        case cws._type of
          DBVT_BYTE  : node.AddDWord('id',cws.bVal);
          DBVT_WORD  : node.AddDWord('id',cws.wVal);
          DBVT_DWORD : node.AddDWord('id',cws.dVal);
          DBVT_ASCIIZ: begin
            node.AddText('id',cws.szVal.A); // ansi to utf
          end;
          DBVT_UTF8  : begin
            node.AddText('id',cws.szVal.A);
          end;
          DBVT_WCHAR : node.AddTextW('id',cws.szVal.W);
          DBVT_BLOB  : begin
            p1:=Base64Encode(cws.pbVal,cws.cpbVal);
            node.AddText('id',p1);
            mFreeMem(p1);
          end;
        end;
      end;
      DBFreeVariant(@cws);
    end;
    if result<>0 then
    begin
      node.AddText('protocol',proto);
      node.AddFlag('ischat'  ,is_chat);
    end;
  end;
end;

procedure tContactAction.Save(node:pointer;fmt:integer);
begin
  inherited Save(node,fmt);
  case fmt of
    0: if (flags and ACF_GETACTIVE)=0 then
      SaveContact(contact,DBBranch,node);
{
    1: begin
        sub:=AddChild(actnode,ioContactWindow,nil);
        ExportContact(sub,contact);
//        AddAttrInt(sub,ioNumber,0); // contact
        if (flags and ACF_KEEPONLY)<>0 then AddAttrInt(sub,ioKeepOnly,1);
    end;
}
    13: begin
      tTextExport(node).AddFlag('keeponly' ,(flags or ACF_KEEPONLY )<>0);
      tTextExport(node).AddFlag('getactive',(flags or ACF_GETACTIVE)<>0);
      ExportContactText(tTextExport(node),contact);
    end;
  end;
end;

//----- Dialog realization -----

procedure ClearFields(Dialog:HWND);
begin
  CheckDlgButton(Dialog,IDC_CNT_KEEP,BST_UNCHECKED);
  CheckDlgButton(Dialog,IDC_CNT_GET ,BST_UNCHECKED);
end;

function DlgProc(Dialog:HWND;hMessage:uint;wParam:WPARAM;lParam:LPARAM):LRESULT; stdcall;
var
  wnd:HWND;
  bb:boolean;
begin
  result:=0;

  case hMessage of
    WM_INITDIALOG: begin
      TranslateDialogDefault(Dialog);

      wnd:=GetDlgItem(Dialog,IDC_CNT_REFRESH);
      OptSetButtonIcon(wnd,ACI_REFRESH);
      SendMessage(wnd,BUTTONADDTOOLTIP,TWPARAM(TranslateW('Refresh')),BATF_UNICODE);
      OptFillContactList(GetDlgItem(Dialog,IDC_CONTACTLIST));
    end;

    WM_ACT_SETVALUE: begin
      ClearFields(Dialog);

      with tContactAction(lParam) do
      begin
        if (flags and ACF_GETACTIVE)<>0 then
        begin
          bb:=false;
          CheckDlgButton(Dialog,IDC_CNT_GET,BST_CHECKED);
        end
        else
        begin
          bb:=true;
          if (flags and ACF_KEEPONLY)<>0 then
            CheckDlgButton(Dialog,IDC_CNT_KEEP,BST_CHECKED);

          SendDlgItemMessage(Dialog,IDC_CONTACTLIST,CB_SETCURSEL,
            FindContact(GetDlgItem(Dialog,IDC_CONTACTLIST),contact),0);
        end;
        EnableWindow(GetDlgItem(Dialog,IDC_CNT_REFRESH),bb);
        EnableWindow(GetDlgItem(Dialog,IDC_CONTACTLIST),bb);
        EnableWindow(GetDlgItem(Dialog,IDC_CNT_KEEP   ),bb);
      end;
    end;

    WM_ACT_RESET: begin
      ClearFields(Dialog);
      EnableWindow(GetDlgItem(Dialog,IDC_CNT_REFRESH),true);
      EnableWindow(GetDlgItem(Dialog,IDC_CONTACTLIST),true);
      EnableWindow(GetDlgItem(Dialog,IDC_CNT_KEEP   ),true);
    end;

    WM_ACT_SAVE: begin
      with tContactAction(lParam) do
      begin
        contact:=SendDlgItemMessage(Dialog,IDC_CONTACTLIST,CB_GETITEMDATA,
            SendDlgItemMessage(Dialog,IDC_CONTACTLIST,CB_GETCURSEL,0,0),0);
        if IsDlgButtonChecked(Dialog,IDC_CNT_KEEP)=BST_CHECKED then
          flags:=flags or ACF_KEEPONLY;
      end;
    end;

    WM_COMMAND: begin
      case wParam shr 16 of
        CBN_SELCHANGE: SendMessage(GetParent(GetParent(Dialog)),PSM_CHANGED,0,0);
        BN_CLICKED: begin
          case loword(wParam) of
            IDC_CNT_GET: begin
              bb:=IsDlgButtonChecked(Dialog,IDC_CNT_GET)=BST_UNCHECKED;
              EnableWindow(GetDlgItem(Dialog,IDC_CNT_REFRESH),bb);
              EnableWindow(GetDlgItem(Dialog,IDC_CONTACTLIST),bb);
              EnableWindow(GetDlgItem(Dialog,IDC_CNT_KEEP   ),bb);
            end;

            IDC_CNT_REFRESH: begin
              OptFillContactList(GetDlgItem(Dialog,IDC_CONTACTLIST));
              exit;
            end;
          end;
          SendMessage(GetParent(GetParent(Dialog)),PSM_CHANGED,0,0);
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
  result:=tContactAction.Create(vc.Hash);
end;

function CreateDialog(parent:HWND):HWND;
begin
  result:=CreateDialogW(hInstance,'IDD_ACTCONTACT',parent,@DlgProc);
end;

procedure Init;
begin
  vc.Next    :=ModuleLink;

  vc.Name    :='Contact';
  vc.Dialog  :=@CreateDialog;
  vc.Create  :=@CreateAction;
  vc.Icon    :='IDI_CONTACT';
  vc.Hash    :=0;

  ModuleLink :=@vc;
end;

begin
  Init;
end.
