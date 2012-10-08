object ConnectionForm: TConnectionForm
  Left = 298
  Top = 145
  BorderIcons = [biSystemMenu]
  BorderStyle = bsDialog
  Caption = 'Connection Setup'
  ClientHeight = 171
  ClientWidth = 292
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  KeyPreview = True
  OldCreateOrder = False
  OnKeyPress = FormKeyPress
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object NickLabel: TLabel
    Left = 8
    Top = 8
    Width = 50
    Height = 13
    Caption = 'Your Nick:'
  end
  object IPLabel: TLabel
    Left = 8
    Top = 115
    Width = 95
    Height = 13
    Caption = 'IP or Domain Name:'
  end
  object PortLabel: TLabel
    Left = 152
    Top = 115
    Width = 32
    Height = 13
    Caption = 'Port #:'
  end
  object NickEdit: TEdit
    Left = 8
    Top = 24
    Width = 121
    Height = 21
    MaxLength = 15
    TabOrder = 0
    Text = 'NN'
    OnExit = NickEditExit
  end
  object ConnectionRadioGroup: TRadioGroup
    Left = 8
    Top = 56
    Width = 185
    Height = 49
    Caption = 'Connect as'
    TabOrder = 1
  end
  object ServerRadioButton: TRadioButton
    Left = 24
    Top = 72
    Width = 57
    Height = 17
    Caption = 'Server'
    Checked = True
    TabOrder = 2
    TabStop = True
    OnClick = ServerRadioButtonClick
  end
  object ClientRadioButton: TRadioButton
    Left = 120
    Top = 72
    Width = 49
    Height = 17
    Caption = 'Client'
    TabOrder = 3
    OnClick = ClientRadioButtonClick
  end
  object OKButton: TButton
    Left = 208
    Top = 16
    Width = 75
    Height = 25
    Caption = '&OK'
    ModalResult = 1
    TabOrder = 6
  end
  object CancelButton: TButton
    Left = 208
    Top = 56
    Width = 75
    Height = 25
    Caption = '&Cancel'
    ModalResult = 2
    TabOrder = 7
  end
  object IPEdit: TEdit
    Left = 8
    Top = 131
    Width = 133
    Height = 21
    Enabled = False
    TabOrder = 4
    OnChange = IPEditChange
  end
  object PortEdit: TMaskEdit
    Left = 152
    Top = 131
    Width = 42
    Height = 21
    EditMask = '09999;; '
    MaxLength = 5
    TabOrder = 5
    Text = '5555 '
    OnExit = PortEditExit
  end
end
