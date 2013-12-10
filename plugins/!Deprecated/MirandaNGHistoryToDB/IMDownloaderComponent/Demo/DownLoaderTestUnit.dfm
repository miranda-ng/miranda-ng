object MainForm: TMainForm
  Left = 0
  Top = 0
  Caption = 'TIMDownloadThread Test'
  ClientHeight = 252
  ClientWidth = 608
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  Position = poDesktopCenter
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object ToolBar1: TToolBar
    Left = 0
    Top = 0
    Width = 608
    Height = 25
    ButtonHeight = 27
    ButtonWidth = 79
    Caption = 'ToolBar1'
    List = True
    ShowCaptions = True
    TabOrder = 0
    object Edit1: TEdit
      Left = 0
      Top = 0
      Width = 385
      Height = 27
      TabOrder = 0
    end
    object TBDownload: TToolButton
      Left = 385
      Top = 0
      AutoSize = True
      Caption = #1057#1082#1072#1095#1072#1090#1100
      ImageIndex = 0
      OnClick = TBDownloadClick
    end
    object TBStopDownload: TToolButton
      Left = 444
      Top = 0
      AutoSize = True
      Caption = #1054#1089#1090#1072#1085#1086#1074#1080#1090#1100
      ImageIndex = 1
      Visible = False
      OnClick = TBStopDownloadClick
    end
    object TBView: TToolButton
      Left = 521
      Top = 0
      AutoSize = True
      Caption = #1042#1099#1074#1077#1089#1090#1080
      DropdownMenu = PopupMenu1
      ImageIndex = 2
      Style = tbsDropDown
      Visible = False
      OnClick = TBViewClick
    end
  end
  object StatusBar1: TStatusBar
    Left = 0
    Top = 233
    Width = 608
    Height = 19
    Panels = <>
    SimplePanel = True
  end
  object RichEdit1: TRichEdit
    Left = 0
    Top = 25
    Width = 608
    Height = 191
    Align = alClient
    Font.Charset = RUSSIAN_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
    ReadOnly = True
    ScrollBars = ssBoth
    TabOrder = 2
  end
  object ProgressBar1: TProgressBar
    Left = 0
    Top = 216
    Width = 608
    Height = 17
    Align = alBottom
    TabOrder = 3
  end
  object IMDownloader_Demo: TIMDownloader
    OnError = IMDownloader_DemoError
    OnAccepted = IMDownloader_DemoAccepted
    OnHeaders = IMDownloader_DemoHeaders
    OnMD5Checked = IMDownloader_DemoMD5Checked
    OnDownloading = IMDownloader_DemoDownloading
    OnStartDownload = IMDownloader_DemoStartDownload
    OnBreak = IMDownloader_DemoBreak
    Left = 72
    Top = 40
  end
  object SaveDialog1: TSaveDialog
    Left = 136
    Top = 96
  end
  object PopupMenu1: TPopupMenu
    Left = 224
    Top = 120
    object ViewAsTest: TMenuItem
      Caption = #1054#1090#1086#1073#1088#1072#1079#1080#1090#1100' '#1082#1072#1082' '#1090#1077#1082#1089#1090
      Default = True
      OnClick = ViewAsTestClick
    end
    object SaveToFile: TMenuItem
      Caption = #1057#1086#1093#1088#1072#1085#1080#1090#1100' '#1074' '#1092#1072#1081#1083
      OnClick = SaveToFileClick
    end
  end
end
