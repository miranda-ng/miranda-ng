////////////////////////////////////////////////////////////////////////////////
// All code below is exclusively owned by author of Chess4Net - Pavel Perminov
// (packpaul@mail.ru, packpaul1@gmail.com).
// Any changes, modifications, borrowing and adaptation are a subject for
// explicit permition from the owner.

unit DialogUnit;

interface

uses
  Forms, Dialogs, Controls, Classes, Windows,
  ModalForm;

type
  TDialogForm = class(TModalForm)
    procedure FormShow(Sender: TObject);
    procedure FormClose(Sender: TObject; var Action: TCloseAction);
    procedure ButtonClick(Sender: TObject);
  private
    m_ModID: TModalFormID;
    m_MsgDlg: TForm;
    function GetCaption: TCaption;
    procedure SetCaption(capt: TCaption);
  protected
    function GetHandle: hWnd; override;
    function GetEnabled_: boolean; override;
    procedure SetEnabled_(flag: boolean); override;
    function GetLeft_: integer; override;
    procedure SetLeft_(x: integer); override;
    function GetTop_: integer; override;
    procedure SetTop_(y: integer); override;
    function GetModalID: TModalFormID; override;
    function RGetModalResult: TModalResult; override;
    procedure RSetModalResult(Value: TModalResult); override;

    property MsgDlg: TForm read m_MsgDlg;
  public
    constructor Create(frmOwner: TForm; const wstrMsg: WideString;
      DlgType: TMsgDlgType; Buttons: TMsgDlgButtons; modID: TModalFormID = mfNone;
      msgDlgHandler: TModalFormHandler = nil; bStayOnTopIfNoOwner: boolean = FALSE); overload;
    constructor Create(dlgOwner: TDialogs; const wstrMsg: WideString;
      DlgType: TMsgDlgType; Buttons: TMsgDlgButtons; modID: TModalFormID;
      msgDlgHandler: TModalFormHandler; bStayOnTopIfNoOwner: boolean = FALSE); overload;
    destructor Destroy; override;

    procedure Show; override;
    procedure Close; override;
    function ShowModal: integer; reintroduce;
    procedure SetFocus; override;

    property Caption: TCaption read GetCaption write SetCaption;
  end;

implementation

uses
  StdCtrls, SysUtils, MessageDialogUnit;

constructor TDialogForm.Create(frmOwner: TForm; const wstrMsg: WideString;
  DlgType: TMsgDlgType; Buttons: TMsgDlgButtons; modID: TModalFormID = mfNone;
  msgDlgHandler: TModalFormHandler = nil; bStayOnTopIfNoOwner: boolean = FALSE);
var
  i: integer;
begin
  inherited CreateNew(frmOwner);

  m_ModID := modID;
  RHandler := msgDlgHandler;

  m_MsgDlg := MessageDialogUnit.CreateMessageDialog(frmOwner, wstrMsg, DlgType, Buttons,
    bStayOnTopIfNoOwner);
  // msgDlg.FormStyle := frmOwner.FormStyle;
  m_MsgDlg.OnShow := FormShow;
  m_MsgDlg.OnClose := FormClose;

  for i := 0 to (m_MsgDlg.ComponentCount - 1) do
  begin
    if (m_MsgDlg.Components[i] is TButton) then
      TButton(m_MsgDlg.Components[i]).OnClick := ButtonClick;
  end;
end;


constructor TDialogForm.Create(dlgOwner: TDialogs; const wstrMsg: WideString;
  DlgType: TMsgDlgType; Buttons: TMsgDlgButtons; modID: TModalFormID;
  msgDlgHandler: TModalFormHandler; bStayOnTopIfNoOwner: boolean = FALSE);
begin
  self.dlgOwner := dlgOwner;
  Create((dlgOwner.Owner as TForm), wstrMsg, DlgType, Buttons, modID, msgDlgHandler, bStayOnTopIfNoOwner);
end;


procedure TDialogForm.FormShow(Sender: TObject);
begin
  inherited FormShow(m_MsgDlg);
end;


procedure TDialogForm.FormClose(Sender: TObject; var Action: TCloseAction);
begin
  if Assigned(dlgOwner) then
    dlgOwner.UnsetShowing(self);
  if (fsModal in m_MsgDlg.FormState) then
    exit;
  if (Assigned(RHandler)) then
    RHandler(self, GetModalID);
//  Action := caFree;
  Release;
end;


procedure TDialogForm.Show;
begin
  m_MsgDlg.Show;
end;


procedure TDialogForm.Close;
begin
  m_MsgDlg.Close;
end;


function TDialogForm.ShowModal: integer;
begin
  Result := m_MsgDlg.ShowModal;
end;

procedure TDialogForm.ButtonClick(Sender: TObject);
begin
  if not (fsModal in m_MsgDlg.FormState) then
    m_MsgDlg.Close;
end;

destructor TDialogForm.Destroy;
begin
  m_MsgDlg.Release;
  inherited;
end;

function TDialogForm.GetCaption: TCaption;
begin
  Result := m_MsgDlg.Caption;
end;

procedure TDialogForm.SetCaption(capt: TCaption);
begin
  m_MsgDlg.Caption := capt;
end;

function TDialogForm.GetHandle: hWnd;
begin
  Result := m_MsgDlg.Handle;
end;


function TDialogForm.GetEnabled_: boolean;
begin
  Result := m_MsgDlg.Enabled;
end;


procedure TDialogForm.SetEnabled_(flag: boolean);
begin
  m_MsgDlg.Enabled := flag;
end;


procedure TDialogForm.SetFocus;
begin
  m_MsgDlg.SetFocus;
  m_MsgDlg.Show;
end;


function TDialogForm.GetLeft_: integer;
begin
  Result := m_MsgDlg.Left;
end;


procedure TDialogForm.SetLeft_(x: integer);
begin
  m_MsgDlg.Left := x;
end;


function TDialogForm.GetTop_: integer;
begin
  Result := m_MsgDlg.Top;
end;


procedure TDialogForm.SetTop_(y: integer);
begin
  m_MsgDlg.Top := y;
end;


function TDialogForm.GetModalID: TModalFormID;
begin
  Result := m_ModID;
end;

function TDialogForm.RGetModalResult: TModalResult;
begin
  Result := m_MsgDlg.ModalResult;
end;


procedure TDialogForm.RSetModalResult(Value: TModalResult);
begin
  m_MsgDlg.ModalResult := Value;
end;

end.
