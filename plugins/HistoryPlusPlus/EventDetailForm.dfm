object EventDetailsFrm: TEventDetailsFrm
  Left = 269
  Top = 168
  Width = 466
  Height = 389
  BorderWidth = 4
  Caption = 'Event Details'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  KeyPreview = True
  OldCreateOrder = False
  ShowHint = True
  OnClose = FormClose
  OnCreate = FormCreate
  OnDestroy = FormDestroy
  OnKeyDown = FormKeyDown
  PixelsPerInch = 96
  TextHeight = 13
  object paBottom: TPanel
    Left = 0
    Top = 322
    Width = 450
    Height = 32
    Align = alBottom
    BevelOuter = bvNone
    TabOrder = 1
    object PrevBtn: TSpeedButton
      Left = 0
      Top = 4
      Width = 100
      Height = 25
      Hint = 'Previous message'
      Caption = 'Prev'
      OnClick = PrevBtnClick
    end
    object NextBtn: TSpeedButton
      Left = 110
      Top = 4
      Width = 100
      Height = 25
      Hint = 'Next message'
      Caption = 'Next'
      OnClick = NextBtnClick
    end
    object Panel3: TPanel
      Left = 250
      Top = 0
      Width = 200
      Height = 32
      Align = alRight
      BevelOuter = bvNone
      TabOrder = 0
      object bnReply: TButton
        Left = 4
        Top = 4
        Width = 116
        Height = 25
        Caption = '&Reply Quoted'
        TabOrder = 0
        OnClick = bnReplyClick
      end
      object CloseBtn: TButton
        Left = 125
        Top = 4
        Width = 75
        Height = 25
        Cancel = True
        Caption = '&Close'
        Default = True
        TabOrder = 1
        OnClick = CloseBtnClick
      end
    end
  end
  object paInfo: TPanel
    Left = 0
    Top = 0
    Width = 450
    Height = 101
    Align = alTop
    BevelOuter = bvNone
    TabOrder = 2
    object GroupBox: THppGroupBox
      Left = 0
      Top = 0
      Width = 450
      Height = 101
      Align = alClient
      Caption = 'Event Info'
      TabOrder = 0
      DesignSize = (
        450
        101)
      object laType: TLabel
        Left = 8
        Top = 16
        Width = 27
        Height = 13
        Caption = 'Type:'
        Transparent = True
      end
      object laDateTime: TLabel
        Left = 8
        Top = 36
        Width = 54
        Height = 13
        Caption = 'Date/Time:'
        Transparent = True
      end
      object laFrom: TLabel
        Left = 8
        Top = 56
        Width = 26
        Height = 13
        Caption = 'From:'
        Transparent = True
      end
      object laTo: TLabel
        Left = 8
        Top = 76
        Width = 16
        Height = 13
        Caption = 'To:'
        Transparent = True
      end
      object EFromMore: TSpeedButton
        Left = 420
        Top = 56
        Width = 20
        Height = 20
        Hint = 'Show sender information'
        Anchors = [akTop, akRight]
        Flat = True
        Layout = blGlyphTop
        OnClick = EFromMoreClick
      end
      object EToMore: TSpeedButton
        Left = 420
        Top = 76
        Width = 20
        Height = 20
        Hint = 'Show receiver information'
        Anchors = [akTop, akRight]
        Flat = True
        Layout = blGlyphTop
        OnClick = EToMoreClick
      end
      object imDirection: TImage
        Left = 422
        Top = 18
        Width = 16
        Height = 16
        Hint = 'Message direction'
        Anchors = [akTop, akRight]
        Center = True
        Transparent = True
      end
      object EMsgType: THppEdit
        Left = 80
        Top = 16
        Width = 337
        Height = 21
        Anchors = [akLeft, akTop, akRight]
        BorderStyle = bsNone
        Color = clBtnFace
        ReadOnly = True
        TabOrder = 0
      end
      object EFrom: THppEdit
        Left = 80
        Top = 56
        Width = 337
        Height = 21
        Anchors = [akLeft, akTop, akRight]
        BorderStyle = bsNone
        Color = clBtnFace
        ReadOnly = True
        TabOrder = 2
      end
      object ETo: THppEdit
        Left = 80
        Top = 76
        Width = 337
        Height = 21
        Anchors = [akLeft, akTop, akRight]
        BorderStyle = bsNone
        Color = clBtnFace
        ReadOnly = True
        TabOrder = 3
      end
      object EDateTime: THppEdit
        Left = 80
        Top = 36
        Width = 337
        Height = 21
        Anchors = [akLeft, akTop, akRight]
        BorderStyle = bsNone
        Color = clBtnFace
        ReadOnly = True
        TabOrder = 1
      end
    end
  end
  object paText: TPanel
    Left = 0
    Top = 101
    Width = 450
    Height = 6
    Align = alTop
    BevelOuter = bvNone
    TabOrder = 3
  end
  object EText: THPPRichEdit
    Left = 0
    Top = 107
    Width = 450
    Height = 215
    Align = alClient
    BevelInner = bvNone
    BevelOuter = bvNone
    BiDiMode = bdLeftToRight
    ParentBiDiMode = False
    PopupMenu = pmEText
    ReadOnly = True
    ScrollBars = ssVertical
    TabOrder = 0
    OnMouseMove = ETextMouseMove
    OnResizeRequest = ETextResizeRequest
    OnURLClick = ETextURLClick
  end
  object pmEText: TPopupMenu
    BiDiMode = bdLeftToRight
    ParentBiDiMode = False
    OnPopup = pmETextPopup
    Left = 68
    Top = 173
    object BrowseReceivedFiles: TMenuItem
      Caption = '&Browse Received Files'
      OnClick = BrowseReceivedFilesClick
    end
    object OpenFileFolder: TMenuItem
      Caption = '&Open file folder'
      OnClick = OpenFileFolderClick
    end
    object CopyFilename: TMenuItem
      Caption = 'Copy &Filename'
      OnClick = CopyLinkClick
    end
    object N3: TMenuItem
      Caption = '-'
    end
    object OpenLinkNW: TMenuItem
      Caption = 'Open in &new window'
      OnClick = OpenLinkNWClick
    end
    object OpenLink: TMenuItem
      Caption = 'Open in existing &window'
      OnClick = OpenLinkClick
    end
    object CopyLink: TMenuItem
      Caption = 'Copy &Link'
      OnClick = CopyLinkClick
    end
    object N4: TMenuItem
      Caption = '-'
    end
    object CopyText: TMenuItem
      Caption = 'Co&py'
      ShortCut = 16451
      OnClick = CopyTextClick
    end
    object CopyAll: TMenuItem
      Caption = 'Copy All'
      OnClick = CopyAllClick
    end
    object SelectAll: TMenuItem
      Caption = 'Select &All'
      ShortCut = 16449
      OnClick = SelectAllClick
    end
    object N1: TMenuItem
      Caption = '-'
    end
    object ToogleItemProcessing: TMenuItem
      Caption = 'Text Formatting'
      ShortCut = 16464
      OnClick = ToogleItemProcessingClick
    end
    object N2: TMenuItem
      Caption = '-'
    end
    object SendMessage1: TMenuItem
      Caption = 'Send &Message'
      ShortCut = 16461
      OnClick = SendMessage1Click
    end
    object ReplyQuoted1: TMenuItem
      Caption = '&Reply Quoted'
      ShortCut = 16466
      OnClick = ReplyQuoted1Click
    end
  end
end
