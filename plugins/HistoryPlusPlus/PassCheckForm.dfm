object fmPassCheck: TfmPassCheck
  Left = 398
  Top = 290
  ActiveControl = edPass
  BorderIcons = [biSystemMenu]
  BorderStyle = bsDialog
  Caption = 'Password Check'
  ClientHeight = 166
  ClientWidth = 280
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  KeyPreview = True
  OldCreateOrder = False
  Position = poScreenCenter
  OnClose = FormClose
  OnCreate = FormCreate
  OnDestroy = FormDestroy
  OnKeyDown = FormKeyDown
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 10
    Top = 92
    Width = 49
    Height = 13
    Caption = 'Password:'
  end
  object Image1: TImage
    Left = 10
    Top = 10
    Width = 32
    Height = 29
    AutoSize = True
    Transparent = True
  end
  object Label2: TLabel
    Left = 50
    Top = 10
    Width = 220
    Height = 31
    AutoSize = False
    Caption = 'Enter password'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
    WordWrap = True
  end
  object Label3: TLabel
    Left = 10
    Top = 46
    Width = 260
    Height = 37
    AutoSize = False
    Caption = 'To access Password Protection options you need to enter password'
    WordWrap = True
  end
  object Bevel1: TBevel
    Left = 10
    Top = 124
    Width = 260
    Height = 2
  end
  object edPass: TEdit
    Left = 77
    Top = 88
    Width = 193
    Height = 21
    MaxLength = 100
    TabOrder = 0
    PasswordChar = '*'
    OnKeyPress = edPassKeyPress
  end
  object bnOK: TButton
    Left = 118
    Top = 134
    Width = 75
    Height = 25
    Caption = 'OK'
    Default = True
    TabOrder = 1
    OnClick = bnOKClick
  end
  object bnCancel: TButton
    Left = 196
    Top = 134
    Width = 75
    Height = 25
    Cancel = True
    Caption = 'Cancel'
    TabOrder = 2
    OnClick = bnCancelClick
  end
end
