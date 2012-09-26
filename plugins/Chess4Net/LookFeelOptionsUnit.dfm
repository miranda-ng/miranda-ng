object LookFeelOptionsForm: TLookFeelOptionsForm
  Left = 702
  Top = 299
  BorderIcons = [biSystemMenu]
  BorderStyle = bsDialog
  Caption = 'Look & Feel Options'
  ClientHeight = 217
  ClientWidth = 321
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Microsoft Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  OnCreate = FormCreate
  OnDestroy = FormDestroy
  PixelsPerInch = 96
  TextHeight = 13
  object AnimateLabel: TTntLabel
    Left = 8
    Top = 11
    Width = 89
    Height = 13
    AutoSize = False
    Caption = 'Animate Move:'
  end
  object GUILangLabel: TTntLabel
    Left = 8
    Top = 187
    Width = 97
    Height = 13
    AutoSize = False
    Caption = 'GUI Language:'
  end
  object OkButton: TTntButton
    Left = 240
    Top = 8
    Width = 75
    Height = 25
    Caption = '&OK'
    Default = True
    ModalResult = 1
    TabOrder = 2
  end
  object CancelButton: TTntButton
    Left = 240
    Top = 40
    Width = 75
    Height = 25
    Cancel = True
    Caption = '&Cancel'
    ModalResult = 2
    TabOrder = 3
  end
  object AnimationComboBox: TTntComboBox
    Left = 104
    Top = 8
    Width = 81
    Height = 21
    Style = csDropDownList
    ItemHeight = 13
    ItemIndex = 0
    TabOrder = 0
    Text = 'No'
    Items.Strings = (
      'No'
      'Slowly'
      'Quickly')
  end
  object BoxPanel: TPanel
    Left = 8
    Top = 40
    Width = 217
    Height = 129
    BevelInner = bvRaised
    BevelOuter = bvLowered
    TabOrder = 1
    DesignSize = (
      217
      129)
    object HilightLastMoveBox: TTntCheckBox
      Left = 8
      Top = 8
      Width = 201
      Height = 17
      Anchors = [akLeft, akTop, akRight]
      Caption = 'Highlight Last Move'
      TabOrder = 0
    end
    object CoordinatesBox: TTntCheckBox
      Left = 8
      Top = 56
      Width = 201
      Height = 17
      Anchors = [akLeft, akTop, akRight]
      Caption = 'Show Coordinates'
      TabOrder = 1
    end
    object StayOnTopBox: TTntCheckBox
      Left = 8
      Top = 80
      Width = 201
      Height = 17
      Anchors = [akLeft, akTop, akRight]
      Caption = 'Stay Always on Top'
      TabOrder = 2
    end
    object ExtraExitBox: TTntCheckBox
      Left = 8
      Top = 104
      Width = 201
      Height = 17
      Anchors = [akLeft, akTop, akRight]
      Caption = 'Extra Exit on ESC'
      TabOrder = 3
    end
    object FlashIncomingMoveBox: TTntCheckBox
      Left = 8
      Top = 32
      Width = 201
      Height = 17
      Anchors = [akLeft, akTop, akRight]
      Caption = 'Flash on incoming move'
      TabOrder = 4
    end
  end
  object GUILangComboBox: TTntComboBox
    Left = 120
    Top = 184
    Width = 129
    Height = 21
    Style = csDropDownList
    ItemHeight = 13
    ItemIndex = 0
    TabOrder = 4
    Text = 'English'
    OnChange = GUILangComboBoxChange
    Items.Strings = (
      'English')
  end
end
