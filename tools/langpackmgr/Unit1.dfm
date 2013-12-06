object Form1: TForm1
  Left = 0
  Top = 0
  AutoSize = True
  BorderIcons = [biSystemMenu]
  Caption = 'Miranda NG Langpack Manager'
  ClientHeight = 343
  ClientWidth = 643
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = 15
  Font.Name = 'Verdana'
  Font.Style = []
  OldCreateOrder = False
  Position = poMainFormCenter
  OnCreate = FormCreate
  PixelsPerInch = 96
  TextHeight = 14
  object ListBox1: TListBox
    Left = 0
    Top = 0
    Width = 137
    Height = 343
    ItemHeight = 14
    TabOrder = 0
    OnClick = ListBox1Click
    OnDblClick = ListBox12Click
  end
  object Memo1: TMemo
    Left = 143
    Top = 0
    Width = 500
    Height = 343
    ScrollBars = ssBoth
    TabOrder = 1
  end
  object Memo2: TMemo
    Left = 143
    Top = 153
    Width = 500
    Height = 150
    ScrollBars = ssBoth
    TabOrder = 2
    Visible = False
  end
  object Button1: TButton
    Left = 142
    Top = 318
    Width = 75
    Height = 25
    Caption = 'Copy'
    TabOrder = 3
    Visible = False
    OnClick = Button1Click
  end
  object Button2: TButton
    Left = 223
    Top = 318
    Width = 75
    Height = 25
    Caption = 'Translate'
    TabOrder = 4
    Visible = False
    OnClick = Button2Click
  end
  object Button3: TButton
    Left = 566
    Top = 318
    Width = 75
    Height = 25
    Caption = 'Save'
    TabOrder = 5
    Visible = False
    OnClick = Button3Click
  end
  object ComboBox1: TComboBox
    Left = 0
    Top = 318
    Width = 136
    Height = 22
    Style = csOwnerDrawFixed
    Enabled = False
    ItemIndex = 1
    TabOrder = 6
    Text = 'Untranslated items'
    Visible = False
    Items.Strings = (
      'All items'
      'Untranslated items')
  end
end
