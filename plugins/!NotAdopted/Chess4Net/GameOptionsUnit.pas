////////////////////////////////////////////////////////////////////////////////
// All code below is exclusively owned by author of Chess4Net - Pavel Perminov
// (packpaul@mail.ru, packpaul1@gmail.com).
// Any changes, modifications, borrowing and adaptation are a subject for
// explicit permition from the owner.

unit GameOptionsUnit;

interface

uses
  Windows, Messages, SysUtils, Classes, Graphics, Controls,
  Dialogs, StdCtrls, TntStdCtrls, ExtCtrls, ComCtrls,
  ModalForm;

type
  TGameOptionsForm = class(TModalForm)
    OkButton: TTntButton;
    CancelButton: TTntButton;
    TimeControlGroupBox: TTntGroupBox;
    EqualTimeCheckBox: TTntCheckBox;
    YouGroupBox: TTntGroupBox;
    YouMinLabel: TTntLabel;
    YouIncLabel: TTntLabel;
    YouMinEdit: TEdit;
    YouIncEdit: TEdit;
    YouUnlimitedCheckBox: TTntCheckBox;
    OpponentGroupBox: TTntGroupBox;
    OpponentMinLabel: TTntLabel;
    OpponentIncLabel: TTntLabel;
    OpponentIncEdit: TEdit;
    OpponentMinEdit: TEdit;
    OpponentUnlimitedCheckBox: TTntCheckBox;
    Panel1: TPanel;
    AutoFlagCheckBox: TTntCheckBox;
    TakeBackCheckBox: TTntCheckBox;
    TrainingModeGroupBox: TTntGroupBox;
    TrainingEnabledCheckBox: TTntCheckBox;
    ExtBaseComboBox: TTntComboBox;
    UsrBaseCheckBox: TTntCheckBox;
    ExtBaseLabel: TTntLabel;
    GamePauseCheckBox: TTntCheckBox;
    YouMinUpDown: TUpDown;
    YouIncUpDown: TUpDown;
    OpponentMinUpDown: TUpDown;
    OpponentIncUpDown: TUpDown;
    GameAdjournCheckBox: TTntCheckBox;
    procedure YouEditChange(Sender: TObject);
    procedure OpponentEditChange(Sender: TObject);
    procedure EqualTimeCheckBoxClick(Sender: TObject);
    procedure UnlimitedCheckBoxClick(Sender: TObject);
    procedure TrainingEnabledCheckBoxClick(Sender: TObject);
    procedure ExtBaseComboBoxChange(Sender: TObject);
    procedure FormShow(Sender: TObject);
    procedure FormCreate(Sender: TObject);
  private
    procedure FLocalize;
  protected
    function GetModalID: TModalFormID; override;
  end;

implementation

{$R *.dfm}

uses
  LocalizerUnit;

procedure TGameOptionsForm.YouEditChange(Sender: TObject);
begin
  YouMinEdit.Text := IntToStr(YouMinUpDown.Position);
  YouIncEdit.Text := IntToStr(YouIncUpDown.Position);
  if EqualTimeCheckBox.Checked then
    begin
      OpponentMinEdit.Text := YouMinEdit.Text;
      OpponentIncEdit.Text := YouIncEdit.Text;
    end;
end;


procedure TGameOptionsForm.OpponentEditChange(Sender: TObject);
begin
  OpponentMinEdit.Text := IntToStr(OpponentMinUpDown.Position);
  OpponentIncEdit.Text := IntToStr(OpponentIncUpDown.Position);
  if EqualTimeCheckBox.Checked then
    begin
      YouMinEdit.Text := OpponentMinEdit.Text;
      YouIncEdit.Text := OpponentIncEdit.Text;
    end;
end;


procedure TGameOptionsForm.EqualTimeCheckBoxClick(Sender: TObject);
begin
  if EqualTimeCheckBox.Checked then
    begin
      OpponentMinEdit.Text := YouMinEdit.Text;
      OpponentIncEdit.Text := YouIncEdit.Text;
      OpponentUnlimitedCheckBox.Checked := YouUnlimitedCheckBox.Checked;
    end;
end;


procedure TGameOptionsForm.UnlimitedCheckBoxClick(Sender: TObject);
begin
  if EqualTimeCheckBox.Checked then
    begin
      YouUnlimitedCheckBox.Checked := TCheckBox(Sender).Checked;
      OpponentUnlimitedCheckBox.Checked := TCheckBox(Sender).Checked;
    end;

  YouMinEdit.Enabled := (not YouUnlimitedCheckBox.Checked);
  YouMinUpDown.Enabled := (not YouUnlimitedCheckBox.Checked);
  YouIncEdit.Enabled := (not YouUnlimitedCheckBox.Checked);
  YouIncUpDown.Enabled := (not YouUnlimitedCheckBox.Checked);

  OpponentMinEdit.Enabled := (not OpponentUnlimitedCheckBox.Checked);
  OpponentMinUpDown.Enabled := (not OpponentUnlimitedCheckBox.Checked);
  OpponentIncEdit.Enabled := (not OpponentUnlimitedCheckBox.Checked);
  OpponentIncUpDown.Enabled := (not OpponentUnlimitedCheckBox.Checked);
end;


procedure TGameOptionsForm.TrainingEnabledCheckBoxClick(Sender: TObject);
begin
  ExtBaseComboBox.Enabled := TrainingEnabledCheckBox.Checked;
  UsrBaseCheckBox.Enabled := TrainingEnabledCheckBox.Checked and (ExtBaseComboBox.ItemIndex <> 0);
  TakeBackCheckBox.Enabled := not TrainingEnabledCheckBox.Checked;
end;


procedure TGameOptionsForm.ExtBaseComboBoxChange(Sender: TObject);
begin
  UsrBaseCheckBox.Enabled := (ExtBaseComboBox.ItemIndex <> 0);
  if ExtBaseComboBox.ItemIndex = 0 then
    UsrBaseCheckBox.Checked := TRUE;
end;


procedure TGameOptionsForm.FormShow(Sender: TObject);
begin
  ExtBaseComboBoxChange(Sender);
end;


function TGameOptionsForm.GetModalID: TModalFormID;
begin
  Result := mfGameOptions;
end;


procedure TGameOptionsForm.FormCreate(Sender: TObject);
begin
  FLocalize;
end;


procedure TGameOptionsForm.FLocalize;
begin
  with TLocalizer.Instance do
  begin
    Caption := GetLabel(24);

    TimeControlGroupBox.Caption := GetLabel(25);
    EqualTimeCheckBox.Caption := GetLabel(26);
    YouGroupBox.Caption := GetLabel(27);
    OpponentGroupBox.Caption := GetLabel(28);
    YouUnlimitedCheckBox.Caption := GetLabel(29);
    OpponentUnlimitedCheckBox.Caption := GetLabel(29);
    YouMinLabel.Caption := GetLabel(30);
    OpponentMinLabel.Caption := GetLabel(30);
    YouIncLabel.Caption := GetLabel(31);
    OpponentIncLabel.Caption := GetLabel(31);

    TrainingModeGroupBox.Caption := GetLabel(32);
    TrainingEnabledCheckBox.Caption := GetLabel(33);
    ExtBaseLabel.Caption := GetLabel(34);
    UsrBaseCheckBox.Caption := GetLabel(35);

    GamePauseCheckBox.Caption := GetLabel(36);
    GameAdjournCheckBox.Caption := GetLabel(37);
    TakeBackCheckBox.Caption := GetLabel(38);
    AutoFlagCheckBox.Caption := GetLabel(39);

    OkButton.Caption := GetLabel(11);
    CancelButton.Caption := GetLabel(12);
  end;
end;


end.
