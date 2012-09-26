////////////////////////////////////////////////////////////////////////////////
// All code below is exclusively owned by author of Chess4Net - Pavel Perminov
// (packpaul@mail.ru, packpaul1@gmail.com).
// Any changes, modifications, borrowing and adaptation are a subject for
// explicit permition from the owner.

unit TransmitGameSelectionUnit;

interface

uses
  Forms, Controls, StdCtrls, TntStdCtrls, Classes,
  //
  ModalForm;

type
  TTransmitGameSelectionForm = class(TModalForm)
    OkButton: TTntButton;
    CancelButton: TTntButton;
    TransmitGameListBox: TTntListBox;
    procedure FormCreate(Sender: TObject);
  private
    procedure FLocalize;
  protected
    function GetModalID: TModalFormID; override;
  public
    procedure SetGames(Games: TStrings);
    function GetSelected: TObject;
  end;

implementation

{$R *.dfm}

uses
  LocalizerUnit;

////////////////////////////////////////////////////////////////////////////////
// TTransmitGameSelectionForm

procedure TTransmitGameSelectionForm.FormCreate(Sender: TObject);
begin
  FLocalize;
end;


procedure TTransmitGameSelectionForm.FLocalize;
begin
  with TLocalizer.Instance do
  begin
    Caption := GetLabel(67);
    OkButton.Caption := GetLabel(11);
    CancelButton.Caption := GetLabel(12);
  end;
end;


function TTransmitGameSelectionForm.GetModalID: TModalFormID;
begin
  Result := mfTransmitGame;
end;


procedure TTransmitGameSelectionForm.SetGames(Games: TStrings);
begin
  TransmitGameListBox.Items.Assign(Games);
  if (TransmitGameListBox.Count > 0) then
    TransmitGameListBox.ItemIndex := 0;
end;


function TTransmitGameSelectionForm.GetSelected: TObject;
var
  iIndex: integer;
begin
  iIndex := TransmitGameListBox.ItemIndex;
  if (iIndex >= 0) then
    Result := TransmitGameListBox.Items.Objects[iIndex]
  else
    Result := nil;
end;

end.
