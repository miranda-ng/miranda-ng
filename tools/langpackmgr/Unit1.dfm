object Form1: TForm1
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu]
  Caption = 'Miranda NG Langpack Manager'
  ClientHeight = 293
  ClientWidth = 710
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
    Top = 276
    Width = 30
    Height = 13
  end
  object ListBox1: TListBox
    Left = 544
    Top = 8
    Width = 161
    Height = 281
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
    Left = 392
    Top = 264
    Width = 146
    Height = 25
    Caption = 'SAVE AND REFRESH'
    TabOrder = 3
    OnClick = Button2Click
  end
  object X: TBitBtn
    Left = 511
    Top = 127
    Width = 27
    Height = 25
    Caption = 'X'
    TabOrder = 4
    OnClick = XClick
  end
  object Memo1: TMemo
    Left = 8
    Top = 32
    Width = 537
    Height = 89
    ScrollBars = ssBoth
    TabOrder = 5
  end
  object Memo2: TMemo
    Left = 8
    Top = 160
    Width = 537
    Height = 89
    ScrollBars = ssBoth
    TabOrder = 6
  end
  object OpenDialog1: TOpenDialog
    Left = 352
    Top = 265
  end
end
