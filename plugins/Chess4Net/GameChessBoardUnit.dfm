object GameChessBoard: TGameChessBoard
  Left = 429
  Top = 209
  Width = 372
  Height = 421
  BorderIcons = [biSystemMenu]
  Color = clBtnFace
  TransparentColorValue = clBackground
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -13
  Font.Name = 'MS Sans Serif'
  Font.Style = [fsBold]
  KeyPreview = True
  OldCreateOrder = False
  OnActivate = FormActivate
  OnCanResize = FormCanResize
  OnClose = FormClose
  OnCreate = FormCreate
  OnDestroy = FormDestroy
  OnKeyDown = FormKeyDown
  DesignSize = (
    356
    385)
  PixelsPerInch = 96
  TextHeight = 16
  object ChessBoardPanel: TPanel
    Left = 0
    Top = 32
    Width = 354
    Height = 352
    Anchors = [akLeft, akTop, akRight, akBottom]
    BevelOuter = bvNone
    TabOrder = 1
  end
  object TimePanel: TPanel
    Left = 0
    Top = 0
    Width = 356
    Height = 33
    Align = alTop
    BevelInner = bvLowered
    TabOrder = 0
    OnResize = TimePanelResize
    object WhitePanel: TPanel
      Left = 8
      Top = 4
      Width = 145
      Height = 25
      BevelOuter = bvNone
      TabOrder = 0
      DesignSize = (
        145
        25)
      object WhiteTimeLabel: TLabel
        Left = 71
        Top = 0
        Width = 68
        Height = 25
        Align = alLeft
        AutoSize = False
        Caption = '0:00:00'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -19
        Font.Name = 'MS Sans Serif'
        Font.Style = [fsBold]
        ParentFont = False
        Layout = tlCenter
      end
      object WhiteFlagButton: TSpeedButton
        Left = 115
        Top = 2
        Width = 23
        Height = 22
        Anchors = [akTop, akRight]
        Glyph.Data = {
          66010000424D6601000000000000760000002800000014000000140000000100
          040000000000F000000000000000000000001000000000000000000000000000
          8000008000000080800080000000800080008080000080808000C0C0C0000000
          FF0000FF000000FFFF00FF000000FF00FF00FFFF0000FFFFFF00FFFFFFFFFFFF
          FFFFFFFF0000FFFFFFFFFFFFFFFFFFFF0000FFFFFFFFFFFFFFFFFFFF0000FFFF
          FFFFFFFFFFFFFFFF0000FFFFFFFFFFFFFFFFFFFF0000FFFFFFFFFFFFFFF83FFF
          0000FFFF99FFFFFFFFF33FFF0000FFFFF99FFFFFFF83FFFF0000FFFFF999FFFF
          FF33FFFF0000F99999999FFFF33FFFFF0000FF99999999FF83FFFFFF0000FFF9
          9999999F33FFFFFF0000FFFF999999983FFFFFFF0000FFFFFF9999993FFFFFFF
          0000FFFFFF999999FFFFFFFF0000FFFFFFF9999FFFFFFFFF0000FFFFFFF9999F
          FFFFFFFF0000FFFFFFFF99FFFFFFFFFF0000FFFFFFFF9FFFFFFFFFFF0000FFFF
          FFFFFFFFFFFFFFFF0000}
        Visible = False
        OnClick = FlagButtonClick
      end
      object WhiteLabel: TTntLabel
        Left = 0
        Top = 0
        Width = 71
        Height = 25
        Align = alLeft
        Caption = 'White   '
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -19
        Font.Name = 'Microsoft Sans Serif'
        Font.Style = [fsBold]
        ParentFont = False
      end
    end
    object BlackPanel: TPanel
      Left = 184
      Top = 4
      Width = 145
      Height = 25
      BevelOuter = bvNone
      TabOrder = 1
      DesignSize = (
        145
        25)
      object BlackTimeLabel: TLabel
        Left = 68
        Top = 0
        Width = 68
        Height = 25
        Align = alLeft
        AutoSize = False
        Caption = '0:00:00'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -19
        Font.Name = 'MS Sans Serif'
        Font.Style = [fsBold]
        ParentFont = False
        Layout = tlCenter
      end
      object BlackLabel: TTntLabel
        Left = 0
        Top = 0
        Width = 68
        Height = 25
        Align = alLeft
        Caption = 'Black   '
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -19
        Font.Name = 'Microsoft Sans Serif'
        Font.Style = [fsBold]
        ParentFont = False
      end
      object BlackFlagButton: TSpeedButton
        Left = 115
        Top = 2
        Width = 23
        Height = 22
        Anchors = [akTop, akRight]
        Visible = False
        OnClick = FlagButtonClick
      end
    end
  end
  object GameTimer: TTimer
    Enabled = False
    Interval = 100
    OnTimer = GameTimerTimer
    Left = 8
    Top = 40
  end
end
