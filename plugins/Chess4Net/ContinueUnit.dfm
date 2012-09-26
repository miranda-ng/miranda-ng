object ContinueForm: TContinueForm
  Left = 649
  Top = 305
  BorderStyle = bsDialog
  ClientHeight = 75
  ClientWidth = 210
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Microsoft Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  OnClose = FormClose
  OnCreate = FormCreate
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object ContinueLabel: TTntLabel
    Left = 0
    Top = 12
    Width = 209
    Height = 13
    Alignment = taCenter
    AutoSize = False
    Caption = 'Press button to continue the game.'
  end
  object ContinueButton: TTntButton
    Left = 67
    Top = 38
    Width = 75
    Height = 25
    Cancel = True
    Caption = '&Continue'
    Default = True
    ModalResult = 1
    TabOrder = 0
    OnClick = ContinueButtonClick
  end
end
