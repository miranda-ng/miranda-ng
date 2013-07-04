object Form1: TForm1
  Left = 0
  Top = 0
  BiDiMode = bdLeftToRight
  BorderIcons = [biSystemMenu, biMinimize]
  Caption = 'Miranda NG Langpack Manager'
  ClientHeight = 310
  ClientWidth = 679
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  ParentBiDiMode = False
  OnCreate = FormCreate
  OnKeyDown = enter
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 0
    Top = 7
    Width = 20
    Height = 13
    Caption = 'File:'
  end
  object Label2: TLabel
    Left = 296
    Top = 7
    Width = 6
    Height = 13
    Caption = '1'
  end
  object Label3: TLabel
    Left = 323
    Top = 7
    Width = 3
    Height = 13
  end
  object ListBox1: TListBox
    Left = 480
    Top = 33
    Width = 193
    Height = 275
    ItemHeight = 13
    TabOrder = 0
    OnClick = ListBox1Click
  end
  object Edit1: TEdit
    Left = 37
    Top = 4
    Width = 156
    Height = 21
    Enabled = False
    TabOrder = 1
  end
  object Button1: TButton
    Left = 215
    Top = 0
    Width = 75
    Height = 25
    Caption = 'OPEN'
    TabOrder = 2
    OnClick = Button1Click
  end
  object Button2: TButton
    Left = 480
    Top = 2
    Width = 193
    Height = 25
    Caption = 'SAVE AND REFRESH'
    TabOrder = 3
    OnClick = Button2Click
  end
  object C: TBitBtn
    Left = 0
    Top = 283
    Width = 27
    Height = 25
    Hint = 'Copy Original'
    Caption = 'C'
    ParentShowHint = False
    ShowHint = True
    TabOrder = 4
    OnClick = CClick
  end
  object X: TBitBtn
    Left = 66
    Top = 283
    Width = 27
    Height = 25
    Hint = 'Clear Translate'
    Caption = 'X'
    ParentShowHint = False
    ShowHint = True
    TabOrder = 5
    OnClick = XClick
  end
  object G: TBitBtn
    Left = 33
    Top = 283
    Width = 27
    Height = 25
    Hint = 'Google Translator'
    Caption = 'G'
    ParentShowHint = False
    ShowHint = True
    TabOrder = 6
    OnClick = GClick
  end
  object Memo1: TMemo
    Left = 0
    Top = 31
    Width = 474
    Height = 120
    ScrollBars = ssBoth
    TabOrder = 7
  end
  object Memo2: TMemo
    Left = 0
    Top = 157
    Width = 474
    Height = 120
    ScrollBars = ssBoth
    TabOrder = 8
  end
  object T: TBitBtn
    Left = 447
    Top = 283
    Width = 27
    Height = 25
    Hint = 'Enable On Top'
    Caption = 'T'
    ParentShowHint = False
    ShowHint = True
    TabOrder = 9
    OnClick = TClick
  end
  object OpenDialog1: TOpenDialog
    Left = 456
  end
end
