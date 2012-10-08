////////////////////////////////////////////////////////////////////////////////
// All code below is exclusively owned by author of Chess4Net - Pavel Perminov
// (packpaul@mail.ru, packpaul1@gmail.com).
// Any changes, modifications, borrowing and adaptation are a subject for
// explicit permition from the owner.

unit ContinueUnit;

interface

uses
  Windows, Messages, SysUtils, Classes, Graphics, Controls, Forms,
  {Dialogs, }ExtCtrls, StdCtrls, TntStdCtrls,
  DialogUnit, ModalForm;

type
  TContinueHandler = procedure of object;

  TContinueForm = class(TModalForm)
    ContinueButton: TTntButton;
    ContinueLabel: TTntLabel;
    procedure ContinueButtonClick(Sender: TObject);
    procedure FormClose(Sender: TObject; var Action: TCloseAction);
    procedure FormShow(Sender: TObject);
    procedure FormCreate(Sender: TObject);
  private
//  dlgOwner: TDialogs;
    ContinueHandler: TContinueHandler;
    shuted: boolean;
    procedure FLocalize;
  protected
    function GetModalID: TModalFormID; override;  
  public
    procedure Shut;
    constructor Create(Owner: TForm; h: TContinueHandler = nil); reintroduce; overload;
//    constructor Create(dlgOwner: TDialogs; h: TContinueHandler); reintroduce; overload;
  end;

implementation

{$R *.dfm}

uses
  GlobalsUnit, LocalizerUnit;

////////////////////////////////////////////////////////////////////////////////
// TContinueForm

procedure TContinueForm.ContinueButtonClick(Sender: TObject);
begin
  Close;
end;


procedure TContinueForm.FormShow(Sender: TObject);
var
  frmOwner: TForm;
begin
  frmOwner := (Owner as TForm);
  Left:= frmOwner.Left + (frmOwner.Width - Width) div 2;
  Top:= frmOwner.Top + (frmOwner.Height - Height) div 2;
end;


constructor TContinueForm.Create(Owner: TForm; h: TContinueHandler = nil);
begin
  self.FormStyle := Owner.FormStyle;
  inherited Create(Owner);
  shuted := FALSE;
  ContinueHandler := h;
end;


procedure TContinueForm.FormClose(Sender: TObject;
  var Action: TCloseAction);
begin
  if not shuted then
    begin
      ModalResult := ContinueButton.ModalResult;
      if Assigned(ContinueHandler) then
        ContinueHandler;
    end
  else
    ModalResult := mrNone;
end;


procedure TContinueForm.Shut;
begin
  shuted:= TRUE;
  Close;
end;


function TContinueForm.GetModalID: TModalFormID;
begin
  Result := mfContinue;
end;


procedure TContinueForm.FormCreate(Sender: TObject);
begin
  Caption := DIALOG_CAPTION;
  FLocalize;
end;


procedure TContinueForm.FLocalize;
begin
  with TLocalizer.Instance do
  begin
    ContinueLabel.Caption := GetLabel(22);
    ContinueButton.Caption := GetLabel(23);
  end;
end;

end.
