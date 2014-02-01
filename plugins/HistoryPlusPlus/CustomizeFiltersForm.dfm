object fmCustomizeFilters: TfmCustomizeFilters
  Left = 227
  Top = 70
  BorderStyle = bsDialog
  Caption = 'Customize Filters'
  ClientHeight = 466
  ClientWidth = 370
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  KeyPreview = True
  OldCreateOrder = False
  Position = poOwnerFormCenter
  OnClose = FormClose
  OnCreate = FormCreate
  OnDestroy = FormDestroy
  OnKeyDown = FormKeyDown
  PixelsPerInch = 96
  TextHeight = 13
  object paClient: TPanel
    Left = 0
    Top = 0
    Width = 370
    Height = 466
    Align = alClient
    BevelOuter = bvNone
    BorderWidth = 4
    TabOrder = 0
    DesignSize = (
      370
      466)
    object bnCancel: TButton
      Left = 89
      Top = 433
      Width = 75
      Height = 25
      Anchors = [akLeft, akBottom]
      Cancel = True
      Caption = '&Cancel'
      TabOrder = 3
      OnClick = bnCancelClick
    end
    object bnOK: TButton
      Left = 8
      Top = 433
      Width = 75
      Height = 25
      Anchors = [akLeft, akBottom]
      Caption = '&OK'
      Default = True
      TabOrder = 2
      OnClick = bnOKClick
    end
    object bnReset: TButton
      Left = 231
      Top = 433
      Width = 131
      Height = 25
      Anchors = [akRight, akBottom]
      Caption = '&Reset to Default'
      TabOrder = 4
      OnClick = bnResetClick
    end
    object gbFilter: THppGroupBox
      Left = 4
      Top = 145
      Width = 362
      Height = 281
      Align = alCustom
      Anchors = [akLeft, akTop, akRight, akBottom]
      Caption = 'Filter Properties'
      Color = clBtnFace
      ParentBackground = False
      ParentColor = False
      TabOrder = 1
      DesignSize = (
        362
        281)
      object laFilterName: TLabel
        Left = 12
        Top = 23
        Width = 31
        Height = 13
        Caption = '&Name:'
        FocusControl = edFilterName
      end
      object edFilterName: THppEdit
        Left = 60
        Top = 20
        Width = 292
        Height = 21
        Anchors = [akLeft, akTop, akRight]
        MaxLength = 63
        TabOrder = 0
        OnChange = edFilterNameChange
        OnKeyPress = edEditKeyPress
      end
      object clEvents: TCheckListBox
        Left = 12
        Top = 88
        Width = 340
        Height = 153
        OnClickCheck = clEventsClickCheck
        Anchors = [akLeft, akTop, akRight, akBottom]
        ItemHeight = 13
        Style = lbOwnerDrawFixed
        TabOrder = 3
        OnDrawItem = clEventsDrawItem
      end
      object cbCustomEvent: TCheckBox
        Left = 12
        Top = 248
        Width = 249
        Height = 21
        Anchors = [akLeft, akRight, akBottom]
        Caption = 'Include custom event type (0-65535)'
        TabOrder = 4
        OnClick = cbCustomEventClick
      end
      object edCustomEvent: THppEdit
        Left = 267
        Top = 248
        Width = 85
        Height = 21
        Anchors = [akRight, akBottom]
        MaxLength = 5
        TabOrder = 5
        OnChange = edCustomEventChange
        OnKeyPress = edEditKeyPress
      end
      object rbInclude: TRadioButton
        Left = 12
        Top = 50
        Width = 340
        Height = 17
        Anchors = [akLeft, akTop, akRight]
        Caption = 'Show only selected events'
        TabOrder = 1
        OnClick = rbPropertyClick
      end
      object rbExclude: TRadioButton
        Left = 12
        Top = 69
        Width = 340
        Height = 17
        Anchors = [akLeft, akTop, akRight]
        Caption = 'Show all except selected events'
        TabOrder = 2
        OnClick = rbPropertyClick
      end
    end
    object gbFilters: THppGroupBox
      Left = 4
      Top = 4
      Width = 362
      Height = 137
      Align = alTop
      Caption = 'Filters'
      Color = clBtnFace
      ParentBackground = False
      ParentColor = False
      TabOrder = 0
      DesignSize = (
        362
        137)
      object lbFilters: TListBox
        Left = 12
        Top = 20
        Width = 249
        Height = 105
        Style = lbOwnerDrawFixed
        Anchors = [akLeft, akTop, akRight, akBottom]
        DragMode = dmAutomatic
        ItemHeight = 13
        TabOrder = 0
        OnClick = lbFiltersClick
        OnDragDrop = lbFiltersDragDrop
        OnDragOver = lbFiltersDragOver
        OnDrawItem = lbFiltersDrawItem
      end
      object bnDown: TButton
        Left = 267
        Top = 102
        Width = 85
        Height = 23
        Anchors = [akTop, akRight]
        Caption = '&Down'
        TabOrder = 4
        OnClick = bnDownClick
      end
      object bnUp: TButton
        Left = 267
        Top = 74
        Width = 85
        Height = 23
        Anchors = [akTop, akRight]
        Caption = '&Up'
        TabOrder = 3
        OnClick = bnUpClick
      end
      object bnDelete: TButton
        Left = 267
        Top = 47
        Width = 85
        Height = 23
        Anchors = [akTop, akRight]
        Caption = 'D&elete'
        TabOrder = 2
        OnClick = bnDeleteClick
      end
      object bnAdd: TButton
        Left = 267
        Top = 20
        Width = 85
        Height = 23
        Anchors = [akTop, akRight]
        Caption = '&Add'
        TabOrder = 1
        OnClick = bnAddClick
      end
    end
  end
end
