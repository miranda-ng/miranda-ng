object EmptyHistoryFrm: TEmptyHistoryFrm
  Left = 346
  Top = 283
  BorderStyle = bsDialog
  BorderWidth = 8
  Caption = 'Empty History'
  ClientHeight = 65
  ClientWidth = 274
  Color = clBtnFace
  DefaultMonitor = dmDesktop
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  FormStyle = fsStayOnTop
  KeyPreview = True
  OldCreateOrder = True
  Position = poScreenCenter
  OnCreate = FormCreate
  OnKeyDown = FormKeyDown
  OnShow = FormShow
  DesignSize = (
    274
    65)
  PixelsPerInch = 96
  TextHeight = 13
  object Image: TImage
    Left = 0
    Top = 0
    Width = 32
    Height = 32
  end
  object Text: TLabel
    Left = 42
    Top = 0
    Width = 232
    Height = 32
    Anchors = [akLeft, akTop, akRight]
    AutoSize = False
    Layout = tlCenter
    WordWrap = True
  end
  object paButtons: TPanel
    Left = 0
    Top = 40
    Width = 274
    Height = 25
    Align = alBottom
    AutoSize = True
    BevelOuter = bvNone
    TabOrder = 0
    ExplicitTop = 54
    object btYes: TButton
      Left = 0
      Top = 0
      Width = 75
      Height = 25
      Caption = 'Yes'
      ModalResult = 6
      TabOrder = 0
      OnClick = btYesClick
    end
    object btNo: TButton
      Left = 85
      Top = 0
      Width = 75
      Height = 25
      Cancel = True
      Caption = 'No'
      ModalResult = 7
      TabOrder = 1
    end
  end
end
