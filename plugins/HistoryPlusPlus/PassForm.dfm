object fmPass: TfmPass
  Left = 359
  Top = 180
  BorderIcons = [biSystemMenu]
  BorderStyle = bsDialog
  Caption = 'History++ Password Protection'
  ClientHeight = 329
  ClientWidth = 300
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  KeyPreview = True
  OldCreateOrder = False
  Position = poScreenCenter
  OnClose = FormClose
  OnCreate = FormCreate
  OnDestroy = FormDestroy
  OnKeyDown = FormKeyDown
  PixelsPerInch = 96
  TextHeight = 13
  object Image1: TImage
    Left = 10
    Top = 10
    Width = 32
    Height = 29
    AutoSize = True
    Transparent = True
  end
  object laPassState: TLabel
    Left = 106
    Top = 254
    Width = 174
    Height = 25
    AutoSize = False
    Caption = '-'
    Layout = tlCenter
    WordWrap = True
  end
  object Bevel1: TBevel
    Left = 10
    Top = 291
    Width = 280
    Height = 2
  end
  object Label1: TLabel
    Left = 50
    Top = 10
    Width = 102
    Height = 13
    Caption = 'Password Options'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
  end
  object rbProtAll: TRadioButton
    Left = 10
    Top = 53
    Width = 280
    Height = 17
    Caption = 'Protect all contacts'
    Checked = True
    TabOrder = 0
    TabStop = True
    OnClick = rbProtSelClick
  end
  object rbProtSel: TRadioButton
    Left = 10
    Top = 73
    Width = 280
    Height = 17
    Caption = 'Protect only selected contacts'
    TabOrder = 1
    TabStop = True
    OnClick = rbProtSelClick
  end
  object lvCList: TListView
    Left = 10
    Top = 93
    Width = 280
    Height = 150
    Checkboxes = True
    Columns = <
      item
        Width = 276
      end>
    ReadOnly = True
    RowSelect = True
    PopupMenu = PopupMenu1
    ShowColumnHeaders = False
    SortType = stText
    TabOrder = 2
    ViewStyle = vsReport
  end
  object bnPass: TButton
    Left = 10
    Top = 254
    Width = 89
    Height = 25
    Caption = 'Password...'
    TabOrder = 3
    OnClick = bnPassClick
  end
  object bnCancel: TButton
    Left = 215
    Top = 299
    Width = 75
    Height = 25
    Cancel = True
    Caption = 'Cancel'
    TabOrder = 5
    OnClick = bnCancelClick
  end
  object bnOK: TButton
    Left = 135
    Top = 299
    Width = 75
    Height = 25
    Caption = 'OK'
    TabOrder = 4
    OnClick = bnOKClick
  end
  object PopupMenu1: TPopupMenu
    Left = 186
    Top = 144
    object Refresh1: TMenuItem
      Caption = '&Refresh List'
      OnClick = Refresh1Click
    end
  end
end
