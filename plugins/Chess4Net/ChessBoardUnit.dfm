object ChessBoard: TChessBoard
  Left = 715
  Top = 238
  Width = 364
  Height = 381
  Caption = 'ChessBoard'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  OnCanResize = FormCanResize
  OnCreate = FormCreate
  OnDestroy = FormDestroy
  OnResize = FormResize
  PixelsPerInch = 96
  TextHeight = 13
  object PBoxBoard: TPaintBox
    Left = 0
    Top = 0
    Width = 356
    Height = 354
    Align = alClient
    Color = clSilver
    DragCursor = crHandPoint
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentColor = False
    ParentFont = False
    OnDragDrop = PBoxBoardDragDrop
    OnDragOver = PBoxBoardDragOver
    OnEndDrag = PBoxBoardEndDrag
    OnMouseDown = PBoxBoardMouseDown
    OnMouseMove = PBoxBoardMouseMove
    OnMouseUp = PBoxBoardMouseUp
    OnPaint = PBoxBoardPaint
    OnStartDrag = PBoxBoardStartDrag
  end
  object AnimateTimer: TTimer
    Enabled = False
    Interval = 1
    OnTimer = AnimateTimerTimer
    Left = 8
    Top = 8
  end
end
