////////////////////////////////////////////////////////////////////////////////
// All code below is exclusively owned by author of Chess4Net - Pavel Perminov
// (packpaul@mail.ru, packpaul1@gmail.com).
// Any changes, modifications, borrowing and adaptation are a subject for
// explicit permition from the owner.

unit LookFeelOptionsUnit;

interface

uses
  Windows, Messages, SysUtils, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls, ExtCtrls,
  ModalForm, TntStdCtrls,
  // Chess4Net units
  LocalizerUnit;

type
  TLookFeelOptionsForm = class(TModalForm, ILocalizable)
    OkButton: TTntButton;
    CancelButton: TTntButton;
    AnimationComboBox: TTntComboBox;
    AnimateLabel: TTntLabel;
    BoxPanel: TPanel;
    HilightLastMoveBox: TTntCheckBox;
    FlashIncomingMoveBox: TTntCheckBox;
    CoordinatesBox: TTntCheckBox;
    StayOnTopBox: TTntCheckBox;
    ExtraExitBox: TTntCheckBox;
    GUILangLabel: TTntLabel;
    GUILangComboBox: TTntComboBox;
    procedure FormCreate(Sender: TObject);
    procedure GUILangComboBoxChange(Sender: TObject);
    procedure FormDestroy(Sender: TObject);
  private
    procedure ILocalizable.Localize = FLocalize;
    procedure FLocalize;
  protected
    function GetModalID: TModalFormID; override;  
  end;

implementation

{$R *.dfm}

function TLookFeelOptionsForm. GetModalID: TModalFormID;
begin
  Result := mfLookFeel;
end;


procedure TLookFeelOptionsForm.FormCreate(Sender: TObject);
var
  i: integer;
begin
  // Fill GUI Languages combo box
  GUILangComboBox.Clear;
  with TLocalizer.Instance do
  begin
    for i := 0 to LanguagesCount - 1 do
      GUILangComboBox.Items.Add(LanguageName[i]);
    GUILangComboBox.ItemIndex := ActiveLanguage;
  end;

  TLocalizer.Instance.AddSubscriber(self);
  FLocalize;
end;


procedure TLookFeelOptionsForm.FLocalize;
var
  iSavedAnimation: integer;
begin
  with TLocalizer.Instance do
  begin
    Caption := GetLabel(0);
    AnimateLabel.Caption := GetLabel(1);
    with AnimationComboBox do
    begin
      iSavedAnimation := ItemIndex;
      Items[0] := GetLabel(2);
      Items[1] := GetLabel(3);
      Items[2] := GetLabel(4);
      ItemIndex := iSavedAnimation;
    end;
    HilightLastMoveBox.Caption := GetLabel(5);
    FlashIncomingMoveBox.Caption := GetLabel(6);
    CoordinatesBox.Caption := GetLabel(7);
    StayOnTopBox.Caption := GetLabel(8);
    ExtraExitBox.Caption := GetLabel(9);
    GUILangLabel.Caption := GetLabel(10);

    OkButton.Caption := GetLabel(11);
    CancelButton.Caption := GetLabel(12);
  end;
end;


procedure TLookFeelOptionsForm.GUILangComboBoxChange(Sender: TObject);
begin
  TLocalizer.Instance.ActiveLanguage := GUILangComboBox.ItemIndex;
end;


procedure TLookFeelOptionsForm.FormDestroy(Sender: TObject);
begin
  TLocalizer.Instance.DeleteSubscriber(self);
end;

end.
