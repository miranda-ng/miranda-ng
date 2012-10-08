object TransmitGameSelectionForm: TTransmitGameSelectionForm
  Left = 487
  Top = 197
  BorderIcons = [biSystemMenu]
  BorderStyle = bsDialog
  Caption = 'Transmit Game'
  ClientHeight = 99
  ClientWidth = 381
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Microsoft Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  OnCreate = FormCreate
  PixelsPerInch = 96
  TextHeight = 13
  object OkButton: TTntButton
    Left = 296
    Top = 8
    Width = 75
    Height = 25
    Caption = '&OK'
    Default = True
    ModalResult = 1
    TabOrder = 1
  end
  object CancelButton: TTntButton
    Left = 296
    Top = 40
    Width = 75
    Height = 25
    Cancel = True
    Caption = '&Cancel'
    ModalResult = 2
    TabOrder = 2
  end
  object TransmitGameListBox: TTntListBox
    Left = 8
    Top = 8
    Width = 281
    Height = 81
    ItemHeight = 13
    TabOrder = 0
  end
end
