(*
    History++ plugin for Miranda IM: the free IM client for Microsoft* Windows*

    Copyright (C) 2006-2009 theMIROn, 2003-2006 Art Fedorov.
    History+ parts (C) 2001 Christian Kastner

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*)

{-----------------------------------------------------------------------------
 EmptyHistoryForm (historypp project)

 Version:   1.0
 Created:   15.03.2008
 Author:    theMIROn

 [ Description ]

  Empty history dialog

 [ History ]

 1.0 (15.04.08) First version.

 [ Modifications ]

 [ Known Issues ]

 Contributors: theMIROn, Art Fedorov
-----------------------------------------------------------------------------}

unit EmptyHistoryForm;

interface

uses Windows, Classes, Controls, Graphics,
  Forms, Buttons, StdCtrls, ExtCtrls,
//  HistoryControls,
  m_api;

type
  TEmptyHistoryFrm = class(TForm)
    btYes: TButton;
    btNo: TButton;
    paButtons: TPanel;
    Image: TImage;
    Text: TLabel;
    procedure FormCreate(Sender: TObject);
    procedure FormKeyDown(Sender: TObject; var Key: Word; Shift: TShiftState);
    procedure FormShow(Sender: TObject);
    procedure btYesClick(Sender: TObject);
  private
    FContact: TMCONTACT;
    procedure TranslateForm;
    procedure PrepareForm;
    procedure SetContact(const Value: TMCONTACT);
    procedure EmptyHistory(hContact: TMCONTACT);
  protected
    function GetFormText: String;
  public
    property Contact: TMCONTACT read FContact write SetContact;
  end;

implementation

uses
  Math, SysUtils, HistoryForm,
  hpp_global, hpp_forms, hpp_contacts, hpp_database, hpp_bookmarks;

{$R *.dfm}

function GetAveCharSize(Canvas: TCanvas): TPoint;
var
  I: Integer;
  Buffer: array[0..51] of WideChar;
  tm: TTextMetric;
begin
  for I := 0 to 25 do Buffer[I] := WideChar(I + Ord('A'));
  for I := 0 to 25 do Buffer[I + 26] := WideChar(I + Ord('a'));
  GetTextMetrics(Canvas.Handle, tm);
  GetTextExtentPointW(Canvas.Handle, Buffer, 52, TSize(Result));
  Result.X := (Result.X div 26 + 1) div 2;
  Result.Y := tm.tmHeight;
end;

function TEmptyHistoryFrm.GetFormText: String;
var
  DividerLine, ButtonCaptions: String;
  I: integer;
begin
  DividerLine := StringOfChar('-', 27) + sLineBreak;
  for I := 0 to ComponentCount - 1 do
    if Components[I] is TButton then
      ButtonCaptions := ButtonCaptions +
                        TButton(Components[I]).Caption + StringOfChar(' ', 3);
  ButtonCaptions := StringReplace(ButtonCaptions,'&','', [rfReplaceAll]);
  Result := DividerLine + Caption + sLineBreak +
            DividerLine + Text.Caption + sLineBreak +
            DividerLine + ButtonCaptions + sLineBreak +
            DividerLine;
end;

procedure TEmptyHistoryFrm.TranslateForm;
begin
  Caption           := TranslateUnicodeString(Caption);
  btYes.Caption     := TranslateUnicodeString(btYes.Caption);
  btNo.Caption      := TranslateUnicodeString(btNo.Caption);
end;

procedure TEmptyHistoryFrm.PrepareForm;
const
  mcSpacing = 8;
  mcButtonWidth = 50;
  mcButtonHeight = 14;
  mcButtonSpacing = 4;
var
  DialogUnits: TPoint;
  HorzSpacing, VertSpacing,
  ButtonWidth, ButtonHeight, ButtonSpacing, ButtonGroupWidth,
  IconTextWidth, IconTextHeight: Integer;
  TextRect,ContRect: TRect;
begin
  DialogUnits := GetAveCharSize(Canvas);
  HorzSpacing   := MulDiv(mcSpacing, DialogUnits.X, 8);
  VertSpacing   := MulDiv(mcSpacing, DialogUnits.X, 4);
  ButtonWidth   := MulDiv(mcButtonWidth, DialogUnits.X, 4);
  ButtonHeight  := MulDiv(mcButtonHeight, DialogUnits.Y, 8);
  ButtonSpacing := MulDiv(mcButtonSpacing, DialogUnits.X, 4);

  SetRect(TextRect, 0, 0, Screen.Width div 2, 0);
    DrawTextW(Canvas.Handle, PChar(Text.Caption),
      Length(Text.Caption)+1, TextRect,
      DT_EXPANDTABS or DT_CALCRECT or DT_WORDBREAK or
      DrawTextBiDiModeFlagsReadingOnly);

  IconTextWidth := Image.Width + HorzSpacing + TextRect.Right;
  IconTextHeight := Max(Image.Height,TextRect.Bottom);

  ButtonGroupWidth := ButtonWidth*2 + ButtonSpacing;

  BorderWidth := VertSpacing;
  ClientWidth := Max(IconTextWidth, ButtonGroupWidth);
  if paContacts.Visible then
  begin
    ContRect := Rect(0,0,0,0);
    DrawTextW(Canvas.Handle,
      PChar(cbInclude.Caption), -1,
      ContRect, DT_CALCRECT or DT_LEFT or DT_SINGLELINE or
      DrawTextBiDiModeFlagsReadingOnly);
    Inc(ContRect.Right, HorzSpacing*4);
    cbInclude.SetBounds((ClientWidth - ContRect.Right) div 2,0,
      ContRect.Right, ContRect.Bottom);
    paContacts.Height := cbInclude.Height + VertSpacing;
    ClientHeight := IconTextHeight + VertSpacing + paContacts.Height + paButtons.Height;
  end
  else
    ClientHeight := IconTextHeight + VertSpacing + paButtons.Height;
  Text.SetBounds(Image.Width + HorzSpacing, 0,
    TextRect.Right, TextRect.Bottom);

  btYes.SetBounds((ClientWidth - ButtonGroupWidth) div 2,0, ButtonWidth, ButtonHeight);
  btNo.SetBounds(btYes.Left + btYes.Width + ButtonSpacing,0, ButtonWidth, ButtonHeight);
end;

procedure TEmptyHistoryFrm.FormShow(Sender: TObject);
begin
  TranslateForm;
  PrepareForm;
end;

procedure TEmptyHistoryFrm.FormCreate(Sender: TObject);
var
  NonClientMetrics: TNonClientMetrics;
begin
  NonClientMetrics.cbSize := sizeof(NonClientMetrics);
  if SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, @NonClientMetrics, 0) then
    Font.Handle := CreateFontIndirect(NonClientMetrics.lfMessageFont);
  MakeFontsParent(Self);
  Canvas.Font := Font;
  DoubleBuffered := True;
  MakeDoubleBufferedParent(Self);
end;

procedure TEmptyHistoryFrm.FormKeyDown(Sender: TObject; var Key: Word; Shift: TShiftState);
begin
  if (Shift = [ssCtrl]) and (Key = Word('C')) then
  begin
    CopyToClip(GetFormText,CP_ACP);
    Key := 0;
  end;
end;

procedure TEmptyHistoryFrm.SetContact(const Value: TMCONTACT);
var
  hContact: TMCONTACT;
  Proto: AnsiString;
  i,num: Integer;
begin
  FContact := Value;
  SetLength(FContacts,0);
  GetContactProto(FContact,hContact,Proto);
  if Value <> hContact then
  begin
    num := db_mc_getSubCount(FContact);
    for i := 0 to num-1 do
    begin
      hContact := db_mc_getSub(FContact,i);
      if hContact <> THandle(-1) then
      begin
        SetLength(FContacts,Length(FContacts)+1);
        FContacts[High(FContacts)] := hContact;
      end;
    end;
  end;
  paContacts.Visible := (Length(FContacts) > 0);
  Image.Picture.Icon.Handle := LoadIcon(0, IDI_QUESTION);
  Text.Caption :=
    TranslateW('Do you really want to delete ALL items for this contact?')+#10#13+
    #10#13+
    TranslateW('Note: It can take several minutes for large histories');
  btYes.Default := true;
end;

procedure TEmptyHistoryFrm.EmptyHistory(hContact: TMCONTACT);
var
  hDBEvent,prevhDbEvent: THandle;
begin
  BookmarkServer.Contacts[hContact].Clear;
  hDBEvent := db_event_last(hContact);
  SetSafetyMode(False);
  while hDBEvent <> 0 do
  begin
    prevhDbEvent := db_event_prev(hContact,hDBEvent);
    if db_event_delete(hContact,hDBEvent) = 0 then
      hDBEvent := prevhDbEvent
    else
      hDBEvent := 0;
  end;
  SetSafetyMode(True);
end;

procedure TEmptyHistoryFrm.btYesClick(Sender: TObject);
var
  i: Integer;
begin
  if Assigned(Owner) and (Owner is THistoryFrm) then
    THistoryFrm(Owner).EmptyHistory
  else
    EmptyHistory(FContact);
  if paContacts.Visible and cbInclude.Checked then
    for i := 0 to High(FContacts) do
      EmptyHistory(FContacts[i]);
end;

end.
