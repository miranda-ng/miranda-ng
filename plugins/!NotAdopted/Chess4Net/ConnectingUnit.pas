////////////////////////////////////////////////////////////////////////////////
// All code below is exclusively owned by author of Chess4Net - Pavel Perminov
// (packpaul@mail.ru, packpaul1@gmail.com).
// Any changes, modifications, borrowing and adaptation are a subject for
// explicit permition from the owner.

unit ConnectingUnit;

interface

uses
  Windows, Messages, SysUtils, Classes, Graphics, Controls, TntStdCtrls,
  Forms, {Dialogs, }ExtCtrls, StdCtrls,
  DialogUnit, ModalForm;

type
  TConnectingHandler = procedure of object;

  TConnectingForm = class(TModalForm)
    AbortButton: TTntButton;
    ConnectingLabel: TTntLabel;
    ConnectingImage: TImage;
    procedure AbortButtonClick(Sender: TObject);
    procedure FormClose(Sender: TObject; var Action: TCloseAction);
    procedure FormShow(Sender: TObject);
    procedure FormCreate(Sender: TObject);
  private
//  dlgOwner: TDialogs;
    ConnectingHandler: TConnectingHandler;
    shuted: boolean;
    procedure FLocalize;
  protected
    function GetModalID : TModalFormID; override;
  public
    procedure Shut;
    constructor Create(Owner: TForm; h: TConnectingHandler = nil); reintroduce; overload;
//    constructor Create(dlgOwner: TDialogs; h: TConnectingHandler); reintroduce; overload;
  end;

implementation

{$R *.dfm}

uses
  LocalizerUnit;

////////////////////////////////////////////////////////////////////////////////
// TConnectiongForm

procedure TConnectingForm.AbortButtonClick(Sender: TObject);
begin
  Close;
end;


procedure TConnectingForm.FormShow(Sender: TObject);
var
  frmOwner: TForm;
begin
  frmOwner := (Owner as TForm);
  Left:= frmOwner.Left + (frmOwner.Width - Width) div 2;
  Top:= frmOwner.Top + (frmOwner.Height - Height) div 2;
end;

constructor TConnectingForm.Create(Owner: TForm; h: TConnectingHandler = nil);
begin
  self.FormStyle := Owner.FormStyle;
  inherited Create(Owner);
  shuted := FALSE;
  ConnectingHandler := h;
end;


procedure TConnectingForm.FormClose(Sender: TObject;
  var Action: TCloseAction);
begin
  if not shuted then
    begin
      ModalResult := AbortButton.ModalResult;
      if Assigned(ConnectingHandler) then
        ConnectingHandler;
    end
  else
    ModalResult := mrNone;
end;


procedure TConnectingForm.Shut;
begin
  shuted:= TRUE;
  Close;
end;

function TConnectingForm.GetModalID: TModalFormID;
begin
  Result := mfConnecting;
end;


procedure TConnectingForm.FLocalize;
begin
  with TLocalizer.Instance do
  begin
    Caption := GetLabel(19);
    ConnectingLabel.Caption := GetLabel(20);
    AbortButton.Caption := GetLabel(21);
  end;
end;


procedure TConnectingForm.FormCreate(Sender: TObject);
begin
  FLocalize;
end;

end.
