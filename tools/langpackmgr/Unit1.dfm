object Form1: TForm1
  Left = 0
  Top = 0
  Caption = 'LangPackMgr'
  ClientHeight = 342
  ClientWidth = 785
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  Menu = MainMenu1
  OldCreateOrder = False
  OnCreate = FormCreate
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 514
    Top = 320
    Width = 6
    Height = 13
    Caption = '0'
  end
  object Button1: TButton
    Left = 631
    Top = 308
    Width = 146
    Height = 25
    Caption = 'Save and refresh'
    TabOrder = 0
    TabStop = False
    OnClick = Button1Click
  end
  object Button2: TButton
    Left = 8
    Top = 308
    Width = 146
    Height = 25
    Caption = 'Custom Plugins'
    TabOrder = 1
    TabStop = False
    OnClick = Button2Click
  end
  object BitBtn1: TBitBtn
    Left = 160
    Top = 308
    Width = 41
    Height = 25
    Caption = 'Copy'
    TabOrder = 2
    TabStop = False
    OnClick = BitBtn1Click
  end
  object BitBtn2: TBitBtn
    Left = 207
    Top = 308
    Width = 42
    Height = 25
    Caption = 'Google:'
    TabOrder = 3
    TabStop = False
    OnClick = BitBtn2Click
  end
  object BitBtn3: TBitBtn
    Left = 279
    Top = 308
    Width = 50
    Height = 25
    Caption = 'Clear'
    TabOrder = 4
    TabStop = False
    OnClick = BitBtn3Click
  end
  object ListBox1: TListBox
    Left = 8
    Top = 8
    Width = 146
    Height = 294
    TabStop = False
    Enabled = False
    ItemHeight = 13
    TabOrder = 5
    OnClick = ListBox1Click
  end
  object ListBox2: TListBox
    Left = 631
    Top = 8
    Width = 146
    Height = 294
    ItemHeight = 13
    TabOrder = 6
    OnClick = ListBox2Click
  end
  object Edit1: TEdit
    Left = 255
    Top = 312
    Width = 18
    Height = 21
    TabOrder = 7
    Text = 'be'
  end
  object Memo1: TMemo
    Left = 160
    Top = 8
    Width = 465
    Height = 144
    ScrollBars = ssHorizontal
    TabOrder = 8
  end
  object Memo2: TMemo
    Left = 160
    Top = 158
    Width = 465
    Height = 144
    ScrollBars = ssHorizontal
    TabOrder = 9
  end
  object MainMenu1: TMainMenu
    Left = 512
    Top = 309
    object File1: TMenuItem
      Caption = 'File'
      object Open1: TMenuItem
        Caption = 'Open'
        OnClick = Open1Click
      end
    end
  end
  object SaveTextFileDialog1: TSaveTextFileDialog
    Left = 480
    Top = 309
  end
end
