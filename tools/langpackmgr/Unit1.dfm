object Form1: TForm1
  Left = 0
  Top = 0
  BiDiMode = bdLeftToRight
  BorderIcons = [biSystemMenu, biMinimize]
  Caption = 'Miranda NG Langpack Manager'
  ClientHeight = 323
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
    Left = 8
    Top = 19
    Width = 20
    Height = 13
    Caption = 'File:'
  end
  object Label2: TLabel
    Left = 328
    Top = 19
    Width = 6
    Height = 13
    Caption = '1'
  end
  object Label3: TLabel
    Left = 356
    Top = 19
    Width = 3
    Height = 13
  end
  object ListBox1: TListBox
    Left = 478
    Top = 38
    Width = 193
    Height = 277
    ItemHeight = 13
    TabOrder = 0
    OnClick = ListBox1Click
  end
  object Edit1: TEdit
    Left = 37
    Top = 11
    Width = 204
    Height = 21
    Enabled = False
    TabOrder = 1
  end
  object Button1: TButton
    Left = 247
    Top = 7
    Width = 75
    Height = 25
    Caption = 'OPEN'
    TabOrder = 2
    OnClick = Button1Click
  end
  object Button2: TButton
    Left = 480
    Top = 7
    Width = 193
    Height = 25
    Caption = 'SAVE AND REFRESH'
    TabOrder = 3
    OnClick = Button2Click
  end
  object C: TBitBtn
    Left = 8
    Top = 290
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
    Left = 74
    Top = 290
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
    Left = 41
    Top = 290
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
    Left = 8
    Top = 38
    Width = 466
    Height = 120
    ScrollBars = ssBoth
    TabOrder = 7
  end
  object Memo2: TMemo
    Left = 8
    Top = 156
    Width = 466
    Height = 120
    ScrollBars = ssBoth
    TabOrder = 8
  end
  object T: TBitBtn
    Left = 447
    Top = 290
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
    Left = 448
    Top = 7
  end
end
