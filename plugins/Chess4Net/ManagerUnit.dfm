object Manager: TManager
  Left = 555
  Top = 124
  BorderIcons = []
  BorderStyle = bsToolWindow
  Caption = 'Manager'
  ClientHeight = 72
  ClientWidth = 204
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Microsoft Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  Position = poDefault
  OnClose = FormClose
  OnCreate = FormCreate
  OnDestroy = FormDestroy
  PixelsPerInch = 96
  TextHeight = 13
  object ActionList: TTntActionList
    OnUpdate = ActionListUpdate
    Left = 176
    Top = 8
    object LookFeelOptionsAction: TTntAction
      Caption = 'Look && Feel Options...'
      OnExecute = LookFeelOptionsActionExecute
    end
    object AboutAction: TTntAction
      Caption = 'About...'
      OnExecute = AboutActionExecute
    end
    object BroadcastAction: TTntAction
      Caption = 'Broadcast...'
      OnExecute = BroadcastActionExecute
    end
  end
  object ConnectedPopupMenu: TTntPopupMenu
    AutoPopup = False
    Left = 8
    Top = 8
    object StartAdjournedGameConnected: TTntMenuItem
      Caption = 'Start Adjourned Game'
      Visible = False
      OnClick = StartAdjournedGameConnectedClick
    end
    object StartStandartGameConnected: TTntMenuItem
      Caption = 'Start Standart Game'
      OnClick = StartStandartGameConnectedClick
    end
    object StartPPRandomGameConnected: TTntMenuItem
      Caption = 'Start PP Random Game'
      OnClick = StartPPRandomGameConnectedClick
    end
    object N5: TTntMenuItem
      Caption = '-'
    end
    object ChangeColorConnected: TTntMenuItem
      Caption = 'Change Color'
      OnClick = ChangeColorConnectedClick
    end
    object TTntMenuItem
      Caption = '-'
    end
    object GameOptionsConnected: TTntMenuItem
      Caption = 'Game Options...'
      OnClick = GameOptionsConnectedClick
    end
    object LookFeelOptionsConnected: TTntMenuItem
      Action = LookFeelOptionsAction
    end
    object N1: TTntMenuItem
      Caption = '-'
    end
    object BroadcastConnected: TTntMenuItem
      Action = BroadcastAction
    end
    object N3: TTntMenuItem
      Caption = '-'
    end
    object AboutConnected: TTntMenuItem
      Action = AboutAction
    end
  end
  object GamePopupMenu: TTntPopupMenu
    AutoPopup = False
    OnPopup = GamePopupMenuPopup
    Left = 40
    Top = 8
    object AbortGame: TTntMenuItem
      Caption = 'Abort'
      OnClick = AbortGameClick
    end
    object DrawGame: TTntMenuItem
      Caption = 'Draw'
      OnClick = DrawGameClick
    end
    object ResignGame: TTntMenuItem
      Caption = 'Resign'
      OnClick = ResignGameClick
    end
    object N6: TTntMenuItem
      Caption = '-'
    end
    object AdjournGame: TTntMenuItem
      Caption = 'Adjourn'
      Visible = False
      OnClick = AdjournGameClick
    end
    object GamePause: TTntMenuItem
      Caption = 'Pause'
      Visible = False
      OnClick = GamePauseClick
    end
    object TakebackGame: TTntMenuItem
      Caption = 'Takeback'
      Visible = False
      OnClick = TakebackGameClick
    end
    object N4: TTntMenuItem
      Caption = '-'
    end
    object LookFeelOptionsGame: TTntMenuItem
      Action = LookFeelOptionsAction
    end
    object N2: TTntMenuItem
      Caption = '-'
    end
    object Broadcast: TTntMenuItem
      Action = BroadcastAction
    end
    object N7: TTntMenuItem
      Caption = '-'
    end
    object AboutGame: TTntMenuItem
      Action = AboutAction
    end
  end
  object ConnectorTimer: TTimer
    Enabled = False
    Interval = 500
    OnTimer = ConnectorTimerTimer
    Left = 8
    Top = 40
  end
end
