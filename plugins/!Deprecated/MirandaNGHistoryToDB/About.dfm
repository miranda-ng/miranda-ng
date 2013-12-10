object AboutForm: TAboutForm
  Left = 200
  Top = 108
  BorderIcons = [biMinimize, biMaximize]
  BorderStyle = bsSingle
  Caption = 'About'
  ClientHeight = 263
  ClientWidth = 346
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  Icon.Data = {
    0000010001001010000001000000680400001600000028000000100000002000
    0000010020000000000000040000000000000000000000000000000000000000
    0000000000000000000000000000FF1F2A000000000000000000000000000000
    00000000000000000000FF1F2A00000000000000000000000000000000000000
    00000000000000000000FF1F2A00FEEFA900FF1F2A00CDCDCD00CDCDCD00CDCD
    CD00CDCDCD00FF1F2A00FEEFA900FF1F2A000000000000000000000000000000
    000000000000FF1F2A00FEEFA900FFE55F00FF1F2A00FF1F2A00FF1F2A00FF1F
    2A00FF1F2A00FF1F2A00FFE55F00FEEFA900FF1F2A0000000000000000000000
    0000FF1F2A00FEEFA900FFE55F00FFE55F00FFE55F00FFD35000FECF4D00FFCC
    4A00FFD35000FFE55F00FFE55F00FFE55F00FEEFA900FF1F2A0000000000FF1F
    2A00FEEFA900FFE55F00FFE55F00FFE55F00FFE55F00FFD35000FFCA4800FFCC
    4A00FFD35000FFE55F00FFE55F00FFE55F00FFE55F00FEEFA900FF1F2A000000
    0000FF1F2A00FEEFA900FFE55F00FFE55F00FFE55F00FFD35000FFCA4800FFCC
    4A00FFD35000FFE55F00FFE55F00FFE55F00FEEFA900FF1F2A00000000000000
    000000000000FF1F2A00FEEFA900FFE55F00FF1F2A00FF1F2A00FF1F2A00FF1F
    2A00FF1F2A00FF1F2A00FFE55F00FEEFA900FF1F2A0000000000000000000000
    000000000000C1BDB600FF1F2A00FEEFA900FF1F2A00C1B7AF00D2BEB900D2BE
    B900D2BEB900FF1F2A00FEEFA900FF1F2A00C1BDB60000000000000000000000
    000000000000BAB7AF00A39A8F00FF1F2A00CAC1B800DFD5CC00FAE2DF00F7DB
    D900DFD5CC00CAC1B800FF1F2A00A39A8F00BAB7AF0000000000000000000000
    000000000000B1AB9F00C9C2B900CFC7BF00D5CCC400DCD3CA00E8DDD500E4D6
    CF00E4D6CF00D5CCC400CFC7BF00C9C2B900B1AB9F0000000000000000000000
    000000000000B0AB9E00C7C0B600CCC4BB00D3C9C100DAD0C700EBE1DA00E4D9
    D100DDD2CA00DBCDC500DDCAC400DBC4BF00C1ADA40000000000000000000000
    000000000000AFAA9D00C8C1B700E8E3DB00F5F0E800FDF8F200FBF7F000FCF7
    F100FDF8F200F7F0EA00ECE3DC00CDC3BB00B2AA9E0000000000000000000000
    000000000000AEA89C00FBF5EF00EEE7DE00E0D8CE00DDD5CC00DDD4CB00DDD4
    CB00DDD5CC00E0D7CE00EDE6DE00FBF6EF00AEA89C0000000000000000000000
    000000000000AFA99D00E1D8CE00E1D7CE00DFD6CC00DFD5CB00DFD5CB00DFD5
    CB00DFD5CB00DFD6CC00E1D7CE00E1D8CE00AFA99D0000000000000000000000
    000000000000D9D9D900B6B0A400D6CFC400E6DED500EFE7DD00EEE6DC00EEE6
    DC00EFE7DD00E6DED500D6CFC400B6B0A400D9D9D90000000000000000000000
    00000000000000000000D9D9D900C4C0B700AEA99C00AEA89C00AEA89C00AEA8
    9C00AEA89C00AEA99C00C4C0B700D9D9D900000000000000000000000000F7EF
    FFFFE007FFFFC003FFFF8001FFFF0000FFFF8001FFFFC003FFFFC003FFFFC003
    FFFFC003FFFFC003FFFFC003FFFFC003FFFFC003FFFFC003FFFFE007FFFF}
  OldCreateOrder = False
  Position = poScreenCenter
  OnClose = FormClose
  OnCreate = FormCreate
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object AboutImage: TImage
    Left = 8
    Top = 2
    Width = 328
    Height = 55
    Transparent = True
  end
  object CloseButton: TButton
    Left = 270
    Top = 233
    Width = 70
    Height = 25
    Caption = 'Close'
    TabOrder = 0
    OnClick = CloseButtonClick
  end
  object AboutPageControl: TPageControl
    Left = 8
    Top = 63
    Width = 332
    Height = 168
    ActivePage = VersionTabSheet
    TabOrder = 1
    object VersionTabSheet: TTabSheet
      Caption = #1054' '#1087#1088#1086#1075#1088#1072#1084#1084#1077
      DesignSize = (
        324
        140)
      object BAbout: TBevel
        Left = 3
        Top = 7
        Width = 318
        Height = 130
        Anchors = [akLeft, akTop, akRight, akBottom]
        ExplicitWidth = 363
        ExplicitHeight = 187
      end
      object LProgramName: TLabel
        Left = 16
        Top = 16
        Width = 72
        Height = 13
        Caption = 'HistoryToDB'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -11
        Font.Name = 'MS Sans Serif'
        Font.Style = [fsBold]
        ParentFont = False
      end
      object LCopyright: TLabel
        Left = 16
        Top = 32
        Width = 124
        Height = 13
        Caption = 'Copyright '#169' 2011-2013 by'
      end
      object LabelAuthor: TLabel
        Left = 143
        Top = 32
        Width = 80
        Height = 13
        Cursor = crHandPoint
        Caption = 'Michael Grigorev'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clBlue
        Font.Height = -11
        Font.Name = 'MS Sans Serif'
        Font.Style = []
        ParentFont = False
        OnClick = LabelAuthorClick
      end
      object LVersionNum: TLabel
        Left = 56
        Top = 48
        Width = 33
        Height = 13
        Caption = '1.0.0.0'
      end
      object LVersion: TLabel
        Left = 16
        Top = 48
        Width = 41
        Height = 13
        Caption = 'Version: '
      end
      object LLicense: TLabel
        Left = 16
        Top = 64
        Width = 43
        Height = 13
        Caption = 'License: '
      end
      object LLicenseType: TLabel
        Left = 59
        Top = 64
        Width = 33
        Height = 13
        Caption = 'GPLv3'
      end
      object LWeb: TLabel
        Left = 16
        Top = 80
        Width = 29
        Height = 13
        Caption = 'Web: '
      end
      object LabelWebSite: TLabel
        Left = 46
        Top = 80
        Width = 82
        Height = 13
        Cursor = crHandPoint
        Caption = 'www.im-history.ru'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clBlue
        Font.Height = -11
        Font.Name = 'MS Sans Serif'
        Font.Style = []
        ParentFont = False
        OnClick = LabelWebSiteClick
      end
    end
    object ThankYouTabSheet: TTabSheet
      Caption = #1041#1083#1072#1075#1086#1076#1072#1088#1085#1086#1089#1090#1080
      ImageIndex = 1
      DesignSize = (
        324
        140)
      object BThankYou: TBevel
        Left = 3
        Top = 3
        Width = 318
        Height = 134
        Anchors = [akLeft, akTop, akRight, akBottom]
        ExplicitWidth = 311
        ExplicitHeight = 115
      end
      object ThankYou: TLabel
        Left = 11
        Top = 11
        Width = 305
        Height = 118
        AutoSize = False
        Caption = #1057#1087#1072#1089#1080#1073#1086' '#1074#1089#1077#1084'!'
        WordWrap = True
      end
    end
  end
end
