object InfoForm: TInfoForm
  Left = 489
  Top = 506
  BorderStyle = bsDialog
  ClientHeight = 166
  ClientWidth = 233
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Microsoft Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  Position = poScreenCenter
  OnClose = FormClose
  OnCreate = FormCreate
  PixelsPerInch = 96
  TextHeight = 13
  object PluginNameLabel: TLabel
    Left = 0
    Top = 8
    Width = 233
    Height = 17
    Alignment = taCenter
    AutoSize = False
    Caption = 'Chess4Net <version>'
    WordWrap = True
  end
  object PlayingViaLabel: TLabel
    Left = 0
    Top = 27
    Width = 233
    Height = 13
    Alignment = taCenter
    AutoSize = False
    Caption = 'Plugin for playing chess via <Messenger name>'
  end
  object Label2: TLabel
    Left = 0
    Top = 46
    Width = 233
    Height = 13
    Alignment = taCenter
    AutoSize = False
    Caption = 'Written by Pavel Perminov'
  end
  object Label3: TLabel
    Left = 0
    Top = 65
    Width = 233
    Height = 13
    Alignment = taCenter
    AutoSize = False
    Caption = #169' 2007-2011 no rights reserved'
  end
  object Label4: TLabel
    Left = 38
    Top = 84
    Width = 25
    Height = 13
    AutoSize = False
    Caption = 'URL:'
  end
  object Label5: TLabel
    Left = 38
    Top = 104
    Width = 32
    Height = 13
    AutoSize = False
    Caption = 'E-Mail:'
  end
  object URLLabel: TLabel
    Left = 73
    Top = 84
    Width = 65
    Height = 13
    Cursor = crHandPoint
    Caption = 'http://<URL>'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clBlue
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsUnderline]
    ParentFont = False
    OnClick = URLLabelClick
  end
  object EMailLabel: TLabel
    Left = 73
    Top = 104
    Width = 41
    Height = 13
    Cursor = crHandPoint
    Caption = '<E-Mail>'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clBlue
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsUnderline]
    ParentFont = False
    OnClick = EMailLabelClick
  end
  object OkButton: TButton
    Left = 80
    Top = 128
    Width = 75
    Height = 25
    Cancel = True
    Caption = '&OK'
    Default = True
    ModalResult = 1
    TabOrder = 0
    OnClick = OkButtonClick
  end
end
