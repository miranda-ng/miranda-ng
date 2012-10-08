object GameOptionsForm: TGameOptionsForm
  Left = 565
  Top = 197
  BorderIcons = [biSystemMenu]
  BorderStyle = bsDialog
  Caption = 'Game Options'
  ClientHeight = 503
  ClientWidth = 412
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Microsoft Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  OnCreate = FormCreate
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object OkButton: TTntButton
    Left = 328
    Top = 16
    Width = 75
    Height = 25
    Caption = '&OK'
    Default = True
    ModalResult = 1
    TabOrder = 3
  end
  object CancelButton: TTntButton
    Left = 328
    Top = 48
    Width = 75
    Height = 25
    Cancel = True
    Caption = '&Cancel'
    ModalResult = 2
    TabOrder = 4
  end
  object TimeControlGroupBox: TTntGroupBox
    Left = 8
    Top = 8
    Width = 305
    Height = 265
    Caption = 'Time Control'
    TabOrder = 0
    object EqualTimeCheckBox: TTntCheckBox
      Left = 16
      Top = 24
      Width = 233
      Height = 17
      Caption = 'Equal time for both players'
      Checked = True
      State = cbChecked
      TabOrder = 0
      OnClick = EqualTimeCheckBoxClick
    end
    object YouGroupBox: TTntGroupBox
      Left = 32
      Top = 48
      Width = 249
      Height = 97
      Caption = 'Your time'
      TabOrder = 1
      DesignSize = (
        249
        97)
      object YouMinLabel: TTntLabel
        Left = 16
        Top = 42
        Width = 145
        Height = 14
        AutoSize = False
        Caption = 'Minutes per game:'
      end
      object YouIncLabel: TTntLabel
        Left = 16
        Top = 66
        Width = 145
        Height = 14
        AutoSize = False
        Caption = 'Increment in seconds:'
      end
      object YouMinEdit: TEdit
        Left = 168
        Top = 40
        Width = 41
        Height = 21
        BiDiMode = bdLeftToRight
        MaxLength = 3
        ParentBiDiMode = False
        TabOrder = 1
        Text = '5'
        OnChange = YouEditChange
      end
      object YouIncEdit: TEdit
        Left = 168
        Top = 64
        Width = 41
        Height = 21
        TabOrder = 2
        Text = '0'
        OnChange = YouEditChange
      end
      object YouUnlimitedCheckBox: TTntCheckBox
        Left = 16
        Top = 16
        Width = 217
        Height = 17
        Anchors = [akLeft, akTop, akRight]
        Caption = 'Unlimited'
        TabOrder = 0
        OnClick = UnlimitedCheckBoxClick
      end
      object YouMinUpDown: TUpDown
        Left = 209
        Top = 40
        Width = 15
        Height = 21
        Associate = YouMinEdit
        Min = 1
        Max = 999
        Position = 5
        TabOrder = 3
      end
      object YouIncUpDown: TUpDown
        Left = 209
        Top = 64
        Width = 15
        Height = 21
        Associate = YouIncEdit
        Max = 999
        TabOrder = 4
      end
    end
    object OpponentGroupBox: TTntGroupBox
      Left = 32
      Top = 152
      Width = 249
      Height = 97
      Caption = 'Opponent'#39's time'
      TabOrder = 2
      DesignSize = (
        249
        97)
      object OpponentMinLabel: TTntLabel
        Left = 16
        Top = 42
        Width = 145
        Height = 13
        AutoSize = False
        Caption = 'Minutes per game:'
      end
      object OpponentIncLabel: TTntLabel
        Left = 16
        Top = 66
        Width = 145
        Height = 13
        AutoSize = False
        Caption = 'Increment in seconds:'
      end
      object OpponentIncEdit: TEdit
        Left = 168
        Top = 64
        Width = 41
        Height = 21
        TabOrder = 2
        Text = '0'
        OnChange = OpponentEditChange
      end
      object OpponentMinEdit: TEdit
        Left = 168
        Top = 40
        Width = 41
        Height = 21
        BiDiMode = bdLeftToRight
        MaxLength = 3
        ParentBiDiMode = False
        TabOrder = 1
        Text = '5'
        OnChange = OpponentEditChange
      end
      object OpponentUnlimitedCheckBox: TTntCheckBox
        Left = 16
        Top = 16
        Width = 217
        Height = 17
        Anchors = [akLeft, akTop, akRight]
        Caption = 'Unlimited'
        TabOrder = 0
        OnClick = UnlimitedCheckBoxClick
      end
      object OpponentMinUpDown: TUpDown
        Left = 209
        Top = 40
        Width = 15
        Height = 21
        Associate = OpponentMinEdit
        Min = 1
        Max = 999
        Position = 5
        TabOrder = 3
      end
      object OpponentIncUpDown: TUpDown
        Left = 209
        Top = 64
        Width = 15
        Height = 21
        Associate = OpponentIncEdit
        Max = 999
        TabOrder = 4
      end
    end
  end
  object Panel1: TPanel
    Left = 8
    Top = 390
    Width = 305
    Height = 105
    BevelInner = bvRaised
    BevelOuter = bvLowered
    TabOrder = 2
    DesignSize = (
      305
      105)
    object AutoFlagCheckBox: TTntCheckBox
      Left = 8
      Top = 80
      Width = 289
      Height = 17
      Anchors = [akLeft, akTop, akRight]
      Caption = 'Auto Flag'
      Checked = True
      State = cbChecked
      TabOrder = 3
    end
    object TakeBackCheckBox: TTntCheckBox
      Left = 8
      Top = 56
      Width = 289
      Height = 17
      Anchors = [akLeft, akTop, akRight]
      Caption = 'Allow takebacks to your partner'
      TabOrder = 2
    end
    object GamePauseCheckBox: TTntCheckBox
      Left = 8
      Top = 8
      Width = 289
      Height = 17
      Anchors = [akLeft, akTop, akRight]
      Caption = 'Game can be paused'
      TabOrder = 0
    end
    object GameAdjournCheckBox: TTntCheckBox
      Left = 8
      Top = 32
      Width = 289
      Height = 17
      Anchors = [akLeft, akTop, akRight]
      Caption = 'Game can be adjourned'
      TabOrder = 1
    end
  end
  object TrainingModeGroupBox: TTntGroupBox
    Left = 8
    Top = 280
    Width = 305
    Height = 97
    Caption = 'Training Mode'
    TabOrder = 1
    DesignSize = (
      305
      97)
    object ExtBaseLabel: TTntLabel
      Left = 16
      Top = 38
      Width = 81
      Height = 13
      AutoSize = False
      Caption = 'External base:'
    end
    object TrainingEnabledCheckBox: TTntCheckBox
      Left = 16
      Top = 16
      Width = 257
      Height = 17
      Caption = 'Enabled'
      TabOrder = 0
      OnClick = TrainingEnabledCheckBoxClick
    end
    object ExtBaseComboBox: TTntComboBox
      Left = 104
      Top = 36
      Width = 169
      Height = 21
      Enabled = False
      ItemHeight = 13
      ItemIndex = 0
      TabOrder = 1
      Text = '<No>'
      OnChange = ExtBaseComboBoxChange
      Items.Strings = (
        '<No>')
    end
    object UsrBaseCheckBox: TTntCheckBox
      Left = 40
      Top = 64
      Width = 257
      Height = 17
      Anchors = [akLeft, akTop, akRight]
      Caption = 'Use user base'
      Enabled = False
      TabOrder = 2
    end
  end
end
