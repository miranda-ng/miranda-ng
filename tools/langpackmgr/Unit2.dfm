object Form2: TForm2
  Left = 0
  Top = 0
  BorderIcons = []
  Caption = 'Select the language'
  ClientHeight = 151
  ClientWidth = 205
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  OnCreate = FormCreate
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 8
    Top = 8
    Width = 89
    Height = 13
    Caption = 'Editable language:'
  end
  object Label2: TLabel
    Left = 8
    Top = 62
    Width = 113
    Height = 13
    Caption = 'Languages to compare:'
  end
  object ComboBox1: TComboBox
    Left = 8
    Top = 27
    Width = 185
    Height = 22
    Style = csOwnerDrawFixed
    TabOrder = 0
    OnChange = ComboBox1Change
  end
  object CheckBox1: TCheckBox
    Left = 137
    Top = 61
    Width = 56
    Height = 17
    Caption = 'english'
    Checked = True
    Enabled = False
    State = cbChecked
    TabOrder = 1
  end
  object Button1: TButton
    Left = 103
    Top = 118
    Width = 89
    Height = 25
    Caption = 'Start'
    Enabled = False
    TabOrder = 2
    OnClick = Button1Click
  end
  object Button2: TButton
    Left = 8
    Top = 118
    Width = 89
    Height = 25
    Caption = 'Cancel'
    TabOrder = 3
    OnClick = Button2Click
  end
  object ComboBox2: TComboBox
    Left = 8
    Top = 84
    Width = 185
    Height = 22
    Style = csOwnerDrawFixed
    TabOrder = 4
  end
end
