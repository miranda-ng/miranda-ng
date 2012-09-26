object PromotionForm: TPromotionForm
  Left = 1
  Top = 1
  Cursor = crHandPoint
  AutoSize = True
  BorderIcons = []
  BorderStyle = bsNone
  BorderWidth = 2
  Caption = 'Promoting Figures'
  ClientHeight = 40
  ClientWidth = 166
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  OnKeyPress = FormKeyPress
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object PromFigImage: TImage
    Left = 0
    Top = 0
    Width = 166
    Height = 40
    OnMouseUp = PromFigImageMouseUp
  end
end
