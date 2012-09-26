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
    msgDlg: TForm;
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

  msgDlg := MessageDialogUnit.CreateMessageDialog(frmOwner, wstrMsg, DlgType, Buttons,
    bStayOnTopIfNoOwner);
  // msgDlg.FormStyle := frmOwner.FormStyle;
  msgDlg.OnShow := FormShow;
  msgDlg.OnClose := FormClose;

  for i := 0 to (msgDlg.ComponentCount - 1) do
  begin
    if (msgDlg.Components[i] is TButton) then
      TButton(msgDlg.Components[i]).OnClick := ButtonClick;
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
  inherited FormShow(msgDlg);
end;


procedure TDialogForm.FormClose(Sender: TObject; var Action: TCloseAction);
begin
  if Assigned(dlgOwner) then
    dlgOwner.UnsetShowing(self);
  if (fsModal in msgDlg.FormState) then
    exit;
  if (Assigned(RHandler)) then
    RHandler(TModalForm(msgDlg), GetModalID);
//  Action := caFree;
  Release;
end;


procedure TDialogForm.Show;
begin
  msgDlg.Show;
end;


procedure TDialogForm.Close;
begin
  msgDlg.Close;
end;


function TDialogForm.ShowModal: integer;
begin
  Result := msgDlg.ShowModal;
end;

procedure TDialogForm.ButtonClick(Sender: TObject);
begin
  if not (fsModal in msgDlg.FormState) then
    msgDlg.Close;
end;

destructor TDialogForm.Destroy;
begin
  msgDlg.Release;
  inherited;
end;

function TDialogForm.GetCaption: TCaption;
begin
  Result := msgDlg.Caption;
end;

procedure TDialogForm.SetCaption(capt: TCaption);
begin
  msgDlg.Caption := capt;
end;

function TDialogForm.GetHandle: hWnd;
begin
  Result := msgDlg.Handle;
end;


function TDialogForm.GetEnabled_: boolean;
begin
  Result := msgDlg.Enabled;
end;


procedure TDialogForm.SetEnabled_(flag: boolean);
begin
  msgDlg.Enabled := flag;
end;


procedure TDialogForm.SetFocus;
begin
  msgDlg.SetFocus;
  msgDlg.Show;
end;


function TDialogForm.GetLeft_: integer;
begin
  Result := msgDlg.Left;
end;


procedure TDialogForm.SetLeft_(x: integer);
begin
  msgDlg.Left := x;
end;


function TDialogForm.GetTop_: integer;
begin
  Result := msgDlg.Top;
end;


procedure TDialogForm.SetTop_(y: integer);
begin
  msgDlg.Top := y;
end;


function TDialogForm.GetModalID: TModalFormID;
begin
  Result := m_ModID;
end;

end.
