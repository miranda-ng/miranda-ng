object Form1: TForm1
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu]
  Caption = 'Miranda NG Langpack Manager'
  ClientHeight = 219
  ClientWidth = 684
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  OnCreate = FormCreate
  OnKeyDown = enter
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 8
    Top = 8
    Width = 20
    Height = 13
    Caption = 'File:'
  end
  object Label2: TLabel
    Left = 455
    Top = 8
    Width = 6
    Height = 13
    Caption = '1'
  end
  object Label3: TLabel
    Left = 8
    Top = 188
    Width = 3
    Height = 13
  end
  object ListBox1: TListBox
    Left = 488
    Top = 3
    Width = 193
    Height = 214
    ItemHeight = 13
    TabOrder = 0
    OnClick = ListBox1Click
  end
  object Edit1: TEdit
    Left = 45
    Top = 5
    Width = 316
    Height = 21
    Enabled = False
    TabOrder = 1
  end
  object Button1: TButton
    Left = 374
    Top = 3
    Width = 75
    Height = 25
    Caption = 'OPEN'
    TabOrder = 2
    OnClick = Button1Click
  end
  object Button2: TButton
    Left = 336
    Top = 176
    Width = 146
    Height = 25
    Caption = 'SAVE AND REFRESH'
    TabOrder = 3
    OnClick = Button2Click
  end
  object C: TBitBtn
    Left = 455
    Top = 56
    Width = 27
    Height = 25
    Caption = 'C'
    TabOrder = 4
    OnClick = CClick
  end
  object X: TBitBtn
    Left = 455
    Top = 145
    Width = 27
    Height = 25
    Caption = 'X'
    TabOrder = 5
    OnClick = XClick
  end
  object Edit2: TEdit
    Left = 8
    Top = 58
    Width = 441
    Height = 21
    TabOrder = 6
  end
  object Edit3: TEdit
    Left = 8
    Top = 111
    Width = 441
    Height = 21
    TabOrder = 7
    OnEnter = FormCreate
  end
  object Button3: TButton
    Left = 240
    Top = 176
    Width = 75
    Height = 25
    Caption = 'DUPES ON'
    TabOrder = 8
    OnClick = Button3Click
  end
  object B: TBitBtn
    Left = 455
    Top = 85
    Width = 27
    Height = 25
    Caption = 'B'
    TabOrder = 9
    OnClick = BClick
  end
  object G: TBitBtn
    Left = 455
    Top = 114
    Width = 27
    Height = 25
    Caption = 'G'
    TabOrder = 10
  end
  object OpenDialog1: TOpenDialog
    Left = 192
    Top = 176
  end
end
