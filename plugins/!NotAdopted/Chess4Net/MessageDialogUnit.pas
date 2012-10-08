////////////////////////////////////////////////////////////////////////////////
// All code below is exclusively owned by author of Chess4Net - Pavel Perminov
// (packpaul@mail.ru, packpaul1@gmail.com).
// Any changes, modifications, borrowing and adaptation are a subject for
// explicit permition from the owner.

unit MessageDialogUnit;

interface

uses
  Forms, TntForms, Dialogs, Classes;

function CreateMessageDialog(AOwner: TComponent; const Msg: WideString; DlgType: TMsgDlgType;
  Buttons: TMsgDlgButtons; bStayOnTopIfNoOwner: boolean = FALSE): TTntForm; overload;
function CreateMessageDialog(AOwner: TComponent; const Msg: WideString;
  DlgType: TMsgDlgType; Buttons: TMsgDlgButtons; DefaultButton: TMsgDlgBtn;
  bStayOnTopIfNoOwner: boolean = FALSE): TTntForm; overload;

implementation

uses
  Types, StdCtrls, TntStdCtrls, Graphics, Windows, TntWindows, Consts, Math,
  ExtCtrls, TntExtCtrls, Controls, SysUtils,
  // Chess4Net units
  LocalizerUnit;

type
  TMessageForm = class(TTntForm)
  private
    Message: TTntLabel;
  public
    constructor CreateNew(AOwner: TComponent; bStayOnTop: boolean); reintroduce;
  end;

////////////////////////////////////////////////////////////////////////////////
// Globals

var
//  ButtonWidths : array[TMsgDlgBtn] of integer;  // initialized to zero
{
  ButtonCaptions: array[TMsgDlgBtn] of Pointer = (
    @SMsgDlgYes, @SMsgDlgNo, @SMsgDlgOK, @SMsgDlgCancel, @SMsgDlgAbort,
    @SMsgDlgRetry, @SMsgDlgIgnore, @SMsgDlgAll, @SMsgDlgNoToAll, @SMsgDlgYesToAll,
    @SMsgDlgHelp);
}    
  IconIDs: array[TMsgDlgType] of PChar = (IDI_EXCLAMATION, IDI_HAND,
    IDI_ASTERISK, IDI_QUESTION, nil);
{
  Captions: array[TMsgDlgType] of Pointer = (@SMsgDlgWarning, @SMsgDlgError,
    @SMsgDlgInformation, @SMsgDlgConfirm, nil);
}
  ButtonNames: array[TMsgDlgBtn] of string = (
    'Yes', 'No', 'OK', 'Cancel', 'Abort', 'Retry', 'Ignore', 'All', 'NoToAll',
    'YesToAll', 'Help');
  ModalResults: array[TMsgDlgBtn] of Integer = (
    mrYes, mrNo, mrOk, mrCancel, mrAbort, mrRetry, mrIgnore, mrAll, mrNoToAll,
    mrYesToAll, 0);

function CreateMessageDialog(AOwner: TComponent; const Msg: WideString;
  DlgType: TMsgDlgType; Buttons: TMsgDlgButtons; DefaultButton: TMsgDlgBtn;
  bStayOnTopIfNoOwner: boolean = FALSE): TTntForm; overload;

  function NGetAveCharSize(Canvas: TCanvas): TPoint;
  var
    I: Integer;
    Buffer: array[0..51] of Char;
  begin
    for I := 0 to 25 do Buffer[I] := Chr(I + Ord('A'));
    for I := 0 to 25 do Buffer[I + 26] := Chr(I + Ord('a'));
    GetTextExtentPoint(Canvas.Handle, Buffer, 52, TSize(Result));
    Result.X := Result.X div 52;
  end;

  function NGetButtonCaption(MsgDlgBtn: TMsgDlgBtn): WideString;
  begin
    with TLocalizer.Instance do
    case MsgDlgBtn of
      mbYes:         Result := GetLabel(40);
      mbNo:          Result := GetLabel(41);
      mbOK:          Result := GetLabel(42);
      mbCancel:      Result := GetLabel(43);
      mbAbort:       Result := GetLabel(44);
      mbRetry:       Result := GetLabel(45);
      mbIgnore:      Result := GetLabel(46);
      mbAll:         Result := GetLabel(47);
      mbNoToAll:     Result := GetLabel(48);
      mbYesToAll:    Result := GetLabel(49);
      mbHelp:        Result := GetLabel(50);
    else
      raise Exception.Create('Unexpected MsgDlgBtn in CreateMessageDialog\NGetButtonCaption.');
    end;
  end;

  function NGetMessageCaption(MsgType: TMsgDlgType): WideString;
  begin
    case MsgType of
      mtWarning:      Result := SMsgDlgWarning;
      mtError:        Result := SMsgDlgError;
      mtInformation:  Result := SMsgDlgInformation;
      mtConfirmation: Result := SMsgDlgConfirm;
      mtCustom:       Result := '';
    else
      raise Exception.Create('Unexpected MsgType in CreateMessageDialog\NGetMessageCaption.');
    end;
  end;

const
  mcHorzMargin = 8;
  mcVertMargin = 8;
  mcHorzSpacing = 10;
  mcVertSpacing = 10;
  mcButtonWidth = 50;
  mcButtonHeight = 14;
  mcButtonSpacing = 4;
var
  bStayOnTop: boolean;
  DialogUnits: TPoint;
  HorzMargin, VertMargin, HorzSpacing, VertSpacing, ButtonWidth,
  ButtonHeight, ButtonSpacing, ButtonCount, ButtonGroupWidth,
  IconTextWidth, IconTextHeight, X, ALeft: Integer;
  B, CancelButton: TMsgDlgBtn;
  IconID: PAnsiChar;
  ATextRect: TRect;
  ThisButtonWidth: integer;
  LButton: TTntButton;
begin { CreateMessageDialog }
  if (Assigned(AOwner)) then
    bStayOnTop := (AOwner.InheritsFrom(TForm) and (TForm(AOwner).FormStyle = fsStayOnTOp))
  else
    bStayOnTop := bStayOnTopIfNoOwner;

  Result := TMessageForm.CreateNew(AOwner, bStayOnTop);
  with Result do
  begin
    BorderStyle := bsDialog; // By doing this first, it will work on WINE.
    BiDiMode := Application.BiDiMode;
    Canvas.Font := Font;
    KeyPreview := True;
    Position := poDesigned;
    DialogUnits := NGetAveCharSize(Canvas);
    HorzMargin := MulDiv(mcHorzMargin, DialogUnits.X, 4);
    VertMargin := MulDiv(mcVertMargin, DialogUnits.Y, 8);
    HorzSpacing := MulDiv(mcHorzSpacing, DialogUnits.X, 4);
    VertSpacing := MulDiv(mcVertSpacing, DialogUnits.Y, 8);
    ButtonWidth := MulDiv(mcButtonWidth, DialogUnits.X, 4);
    for B := Low(TMsgDlgBtn) to High(TMsgDlgBtn) do
    begin
      if B in Buttons then
      begin
        ATextRect := Rect(0,0,0,0);
        Tnt_DrawTextW(Canvas.Handle,
          PWideChar(NGetButtonCaption(B)), -1,
          ATextRect, DT_CALCRECT or DT_LEFT or DT_SINGLELINE or
          DrawTextBiDiModeFlagsReadingOnly);
        with ATextRect do ThisButtonWidth := Right - Left + 8;
        if ThisButtonWidth > ButtonWidth then
          ButtonWidth := ThisButtonWidth;
      end;
    end;
    ButtonHeight := MulDiv(mcButtonHeight, DialogUnits.Y, 8);
    ButtonSpacing := MulDiv(mcButtonSpacing, DialogUnits.X, 4);
    SetRect(ATextRect, 0, 0, Screen.Width div 2, 0);
    Tnt_DrawTextW(Canvas.Handle, PWideChar(Msg), Length(Msg) + 1, ATextRect,
      DT_EXPANDTABS or DT_CALCRECT or DT_WORDBREAK or
      DrawTextBiDiModeFlagsReadingOnly);
    IconID := IconIDs[DlgType];
    IconTextWidth := ATextRect.Right;
    IconTextHeight := ATextRect.Bottom;
    if IconID <> nil then
    begin
      Inc(IconTextWidth, 32 + HorzSpacing);
      if IconTextHeight < 32 then IconTextHeight := 32;
    end;
    ButtonCount := 0;
    for B := Low(TMsgDlgBtn) to High(TMsgDlgBtn) do
      if B in Buttons then Inc(ButtonCount);
    ButtonGroupWidth := 0;
    if ButtonCount <> 0 then
      ButtonGroupWidth := ButtonWidth * ButtonCount +
        ButtonSpacing * (ButtonCount - 1);
    ClientWidth := Max(IconTextWidth, ButtonGroupWidth) + HorzMargin * 2;
    ClientHeight := IconTextHeight + ButtonHeight + VertSpacing +
      VertMargin * 2;
    Left := (Screen.Width div 2) - (Width div 2);
    Top := (Screen.Height div 2) - (Height div 2);
    if DlgType <> mtCustom then
      Caption := NGetMessageCaption(DlgType)
    else
      Caption := TntApplication.Title;
    if IconID <> nil then
      with TTntImage.Create(Result) do
      begin
        Name := 'Image';
        Parent := Result;
        Picture.Icon.Handle := LoadIcon(0, IconID);
        SetBounds(HorzMargin, VertMargin, 32, 32);
      end;
    TMessageForm(Result).Message := TTntLabel.Create(Result);
    with TMessageForm(Result).Message do
    begin
      Name := 'Message';
      Parent := Result;
      WordWrap := True;
      Caption := Msg;
      BoundsRect := ATextRect;
      BiDiMode := Result.BiDiMode;
      ALeft := IconTextWidth - ATextRect.Right + HorzMargin;
      if UseRightToLeftAlignment then
        ALeft := Result.ClientWidth - ALeft - Width;
      SetBounds(ALeft, VertMargin,
        ATextRect.Right, ATextRect.Bottom);
    end;
    if mbCancel in Buttons then CancelButton := mbCancel else
      if mbNo in Buttons then CancelButton := mbNo else
        CancelButton := mbOk;
    X := (ClientWidth - ButtonGroupWidth) div 2;
    for B := Low(TMsgDlgBtn) to High(TMsgDlgBtn) do
      if B in Buttons then
      begin
        LButton := TTntButton.Create(Result);
        with LButton do
        begin
          Name := ButtonNames[B];
          Parent := Result;
          Caption := NGetButtonCaption(B);
          ModalResult := ModalResults[B];
          if B = DefaultButton then
          begin
            Default := True;
            ActiveControl := LButton;
          end;
          if B = CancelButton then
            Cancel := True;
          SetBounds(X, IconTextHeight + VertMargin + VertSpacing,
            ButtonWidth, ButtonHeight);
          Inc(X, ButtonWidth + ButtonSpacing);
        end;
      end;
  end;
end;


function CreateMessageDialog(AOwner: TComponent; const Msg: WideString; DlgType: TMsgDlgType;
  Buttons: TMsgDlgButtons; bStayOnTopIfNoOwner: boolean = FALSE): TTntForm;
var
  DefaultButton: TMsgDlgBtn;
begin
  if mbOk in Buttons then
    DefaultButton := mbOk
  else if mbYes in Buttons then
    DefaultButton := mbYes
  else
    DefaultButton := mbRetry;
  Result := CreateMessageDialog(AOwner, Msg, DlgType, Buttons, DefaultButton, bStayOnTopIfNoOwner);
end;

////////////////////////////////////////////////////////////////////////////////
// TMessageForm

constructor TMessageForm.CreateNew(AOwner: TComponent; bStayOnTop: boolean);
var
  NonClientMetrics: TNonClientMetrics;
begin
  if (bStayOnTop) then
    FormStyle := fsStayOnTop;
  inherited CreateNew(AOwner);
  NonClientMetrics.cbSize := sizeof(NonClientMetrics);
  if SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, @NonClientMetrics, 0) then
    Font.Handle := CreateFontIndirect(NonClientMetrics.lfMessageFont);
end;

end.
