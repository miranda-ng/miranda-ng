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

{ -----------------------------------------------------------------------------
  EventDetailForm (historypp project)

  Version:   1.4
  Created:   31.03.2003
  Author:    Oxygen

  [ Description ]

  Form for details about event

  [ History ]

  1.4
  - Added horz scroll bar to memo

  1.0 (31.03.2003) - Initial version

  [ Modifications ]
  * (29.05.2003) Added scroll bar to memo

  [ Knows Inssues ]
  None

  Contributors: theMIROn, Art Fedorov, Christian Kastner
  ----------------------------------------------------------------------------- }

unit EventDetailForm;

interface

uses
  Windows, Messages, SysUtils, Classes, Graphics, Controls, Forms, Dialogs,
  StdCtrls, ExtCtrls,
  HistoryForm,
  m_api,
  hpp_global, hpp_richedit,
  ComCtrls,
  Menus, RichEdit, Buttons, HistoryControls, ShellAPI;

type

  TEventDetailsFrm = class(TForm)
    paBottom: TPanel;
    Panel3: TPanel;
    paInfo: TPanel;
    GroupBox: THppGroupBox;
    laType: TLabel;
    laDateTime: TLabel;
    EMsgType: THppEdit;
    bnReply: TButton;
    CloseBtn: TButton;
    laFrom: TLabel;
    laTo: TLabel;
    EFrom: THppEdit;
    ETo: THppEdit;
    EText: THPPRichEdit;
    pmEText: TPopupMenu;
    CopyText: TMenuItem;
    CopyAll: TMenuItem;
    SelectAll: TMenuItem;
    N1: TMenuItem;
    ReplyQuoted1: TMenuItem;
    SendMessage1: TMenuItem;
    paText: TPanel;
    N2: TMenuItem;
    ToogleItemProcessing: TMenuItem;
    EFromMore: TSpeedButton;
    EDateTime: THppEdit;
    EToMore: TSpeedButton;
    PrevBtn: TSpeedButton;
    NextBtn: TSpeedButton;
    OpenLinkNW: TMenuItem;
    OpenLink: TMenuItem;
    CopyLink: TMenuItem;
    N4: TMenuItem;
    imDirection: TImage;
    N3: TMenuItem;
    BrowseReceivedFiles: TMenuItem;
    OpenFileFolder: TMenuItem;
    CopyFilename: TMenuItem;
    procedure PrevBtnClick(Sender: TObject);
    procedure NextBtnClick(Sender: TObject);
    procedure EFromMoreClick(Sender: TObject);
    procedure EToMoreClick(Sender: TObject);
    procedure FormDestroy(Sender: TObject);
    procedure FormKeyDown(Sender: TObject; var Key: Word; Shift: TShiftState);
    procedure FormClose(Sender: TObject; var Action: TCloseAction);
    procedure CloseBtnClick(Sender: TObject);
    procedure FormCreate(Sender: TObject);
    procedure bnReplyClick(Sender: TObject);
    procedure pmETextPopup(Sender: TObject);
    procedure SelectAllClick(Sender: TObject);
    procedure CopyTextClick(Sender: TObject);
    procedure CopyAllClick(Sender: TObject);
    procedure SendMessage1Click(Sender: TObject);
    procedure ReplyQuoted1Click(Sender: TObject);
    procedure ToogleItemProcessingClick(Sender: TObject);
    procedure ETextResizeRequest(Sender: TObject; Rect: TRect);
    procedure OpenLinkNWClick(Sender: TObject);
    procedure OpenLinkClick(Sender: TObject);
    procedure CopyLinkClick(Sender: TObject);
    procedure ETextMouseMove(Sender: TObject; Shift: TShiftState; X, Y: Integer);
    procedure BrowseReceivedFilesClick(Sender: TObject);
    procedure OpenFileFolderClick(Sender: TObject);
    procedure ETextURLClick(Sender: TObject; const URLText: String; Button: TMouseButton);
  private
    FParentForm: THistoryFrm;
    FItem: Integer;
    FRichHeight: Integer;
    FOverURL: Boolean;
    SavedLinkUrl: String;
    FOverFile: Boolean;
    SavedFileDir: String;
    hSubContactFrom, hSubContactTo: THandle;
    FNameFrom, FNameTo: String;
    FProtocol: AnsiString;

    procedure OnCNChar(var Message: TWMChar); message WM_CHAR;
    procedure WMGetMinMaxInfo(var Message: TWMGetMinMaxInfo); message WM_GETMINMAXINFO;
    procedure WMSetCursor(var Message: TWMSetCursor); message WM_SETCURSOR;
    procedure WMSysColorChange(var Message: TMessage); message WM_SYSCOLORCHANGE;
    procedure LoadPosition;
    procedure SavePosition;
    procedure SetItem(const Value: Integer);
    procedure TranslateForm;
    procedure LoadButtonIcons;
    procedure LoadMessageIcons;
    { Private declarations }
    procedure HMIconsChanged(var M: TMessage); message HM_NOTF_ICONSCHANGED;
    procedure HMIcons2Changed(var M: TMessage); message HM_NOTF_ICONS2CHANGED;
    procedure HMEventDeleted(var Message: TMessage); message HM_MIEV_EVENTDELETED;
    function GetPrevItem: Integer;
    function GetNextItem: Integer;
    function IsFileEvent: Boolean;
  protected
    property PrevItem: Integer read GetPrevItem;
    property NextItem: Integer read GetNextItem;
  public
    hContactTo, hContactFrom: THandle;
    property ParentForm: THistoryFrm read FParentForm write FParentForm;
    property Item: Integer read FItem write SetItem;
    procedure ProcessRichEdit(const FItem: Integer);
    procedure ResetItem;
  end;

var
  EventDetailsFrm: TEventDetailsFrm;

implementation

uses
  HistoryGrid, hpp_messages, hpp_contacts, hpp_events, hpp_forms,
  {hpp_database,} hpp_options, hpp_services;

{$R *.DFM}
{ TForm1 }

procedure TEventDetailsFrm.WMGetMinMaxInfo(var Message: TWMGetMinMaxInfo);
begin
  inherited;
  with Message.MinMaxInfo^ do
  begin
    ptMinTrackSize.X := 376;
    ptMinTrackSize.Y := 240;
  end
end;

procedure TEventDetailsFrm.ProcessRichEdit(const FItem: Integer);
var
  ItemRenderDetails: TItemRenderDetails;
begin
  ZeroMemory(@ItemRenderDetails, SizeOf(ItemRenderDetails));
  ItemRenderDetails.cbSize := SizeOf(ItemRenderDetails);
  ItemRenderDetails.hContact := ParentForm.hContact;
  ItemRenderDetails.hDBEvent := ParentForm.History[ParentForm.GridIndexToHistory(FItem)];
  ItemRenderDetails.pProto := PAnsiChar(ParentForm.hg.Items[FItem].Proto);
  ItemRenderDetails.pModule := PAnsiChar(ParentForm.hg.Items[FItem].Module);
  ItemRenderDetails.pText := nil;
  ItemRenderDetails.pExtended := PAnsiChar(ParentForm.hg.Items[FItem].Extended);
  ItemRenderDetails.dwEventTime := ParentForm.hg.Items[FItem].Time;
  ItemRenderDetails.wEventType := ParentForm.hg.Items[FItem].EventType;
  ItemRenderDetails.IsEventSent := (mtOutgoing in ParentForm.hg.Items[FItem].MessageType);
  { TODO: Add flag for special event details form treatment? }
  ItemRenderDetails.dwFlags := ItemRenderDetails.dwFlags or IRDF_EVENT;
  if ParentForm.hContact = 0 then
    ItemRenderDetails.bHistoryWindow := IRDHW_GLOBALHISTORY
  else
    ItemRenderDetails.bHistoryWindow := IRDHW_CONTACTHISTORY;
  NotifyEventHooks(hHppRichEditItemProcess, EText.Handle, LPARAM(@ItemRenderDetails));
end;

procedure TEventDetailsFrm.EFromMoreClick(Sender: TObject);
begin
  CallService(MS_USERINFO_SHOWDIALOG, hContactFrom, 0);
end;

procedure TEventDetailsFrm.EToMoreClick(Sender: TObject);
begin
  CallService(MS_USERINFO_SHOWDIALOG, hContactTo, 0);
end;

procedure TEventDetailsFrm.FormDestroy(Sender: TObject);
begin
  try
    FParentForm.EventDetailForm := nil;
  except
  end;
end;

procedure TEventDetailsFrm.FormKeyDown(Sender: TObject; var Key: Word; Shift: TShiftState);
var
  Mask: Integer;
begin
  if IsFormShortCut([pmEText], Key, Shift) then
    Key := 0;

  with Sender as TWinControl do
  begin
    if Perform(CM_CHILDKEY, Key, LPARAM(Sender)) <> 0 then
      Exit;
    Mask := 0;
    case Key of
      VK_TAB:
        Mask := DLGC_WANTTAB;
      VK_RETURN, VK_EXECUTE, VK_ESCAPE, VK_CANCEL:
        Mask := DLGC_WANTALLKEYS;
    end;
    if (Mask <> 0) and (Perform(CM_WANTSPECIALKEY, Key, 0) = 0) and
      (Perform(WM_GETDLGCODE, 0, 0) and Mask = 0) and (Self.Perform(CM_DIALOGKEY, Key, 0) <> 0)
    then
      Exit;
  end;
end;

procedure TEventDetailsFrm.OnCNChar(var Message: TWMChar);
// make tabs work!
begin
  if not(csDesigning in ComponentState) then
    with Message do
    begin
      Result := 1;
      if (Perform(WM_GETDLGCODE, 0, 0) and DLGC_WANTCHARS = 0) and
        (GetParentForm(Self).Perform(CM_DIALOGCHAR, CharCode, KeyData) <> 0) then
        Exit;
      Result := 0;
    end;
end;

procedure TEventDetailsFrm.LoadPosition;
begin
  Utils_RestoreFormPosition(Self, 0, hppDBName, 'EventDetail.');
end;

procedure TEventDetailsFrm.SavePosition;
begin
  Utils_SaveFormPosition(Self, 0, hppDBName, 'EventDetail.');
end;

procedure TEventDetailsFrm.FormClose(Sender: TObject; var Action: TCloseAction);
begin
  Action := caFree;
  SavePosition;
end;

procedure TEventDetailsFrm.CloseBtnClick(Sender: TObject);
begin
  SavePosition;
  Self.Release;
end;

procedure TEventDetailsFrm.FormCreate(Sender: TObject);
begin
  Icon.ReleaseHandle;

  DesktopFont := True;
  MakeFontsParent(Self);

  DoubleBuffered := True;
  MakeDoubleBufferedParent(Self);

  LoadButtonIcons;
  TranslateForm;

  LoadPosition;
end;

procedure TEventDetailsFrm.SetItem(const Value: Integer);
var
  FromContact, ToContact: Boolean;
begin
  Assert(Assigned(FParentForm));
  if Value = -1 then
    Exit;
  FItem := Value;
  EMsgType.Text := TranslateUnicodeString(GetEventRecord(FParentForm.hg.Items[FItem])
    .Name { TRANSLATE-IGNORE } );
  EMsgType.Text := Format('%s [%s/%u]', [EMsgType.Text, FParentForm.hg.Items[FItem].Module,
    FParentForm.hg.Items[FItem].EventType]);
  EDateTime.Text := TimestampToString(FParentForm.hg.Items[FItem].Time);
  if FParentForm.hContact = 0 then
    FProtocol := FParentForm.hg.Items[FItem].Proto
  else
    FProtocol := FParentForm.SubProtocol;
  FromContact := false;
  ToContact := false;
  if mtIncoming in FParentForm.hg.Items[FItem].MessageType then
  begin
    hContactFrom := FParentForm.hContact;
    hSubContactFrom := FParentForm.hSubContact;
    hContactTo := 0;
    hSubContactTo := 0;
    FNameFrom := FParentForm.hg.ContactName;
    FNameTo := GetContactDisplayName(0, FProtocol);
    FromContact := (hContactFrom = 0);
  end
  else
  begin
    hContactFrom := 0;
    hSubContactFrom := 0;
    hContactTo := FParentForm.hContact;
    hSubContactTo := FParentForm.hSubContact;
    FNameFrom := GetContactDisplayName(0, FProtocol);
    FNameTo := FParentForm.hg.ContactName;
    ToContact := (hContactTo = 0);
  end;

  LoadMessageIcons;

  EFromMore.Enabled := not FromContact;
  EFrom.Text := FNameFrom;
  if not FromContact then
    EFrom.Text := EFrom.Text + ' (' + AnsiToWideString
      (FProtocol + ': ' + GetContactID(hSubContactFrom, FProtocol, FromContact),
      ParentForm.UserCodepage) + ')';
  EToMore.Enabled := not ToContact;
  ETo.Text := FNameTo;
  if not ToContact then
    ETo.Text := ETo.Text + ' (' + AnsiToWideString
      (FProtocol + ': ' + GetContactID(hSubContactTo, FProtocol, ToContact),
      ParentForm.UserCodepage) + ')';

  EText.Lines.BeginUpdate;
  ParentForm.hg.ApplyItemToRich(FItem, EText, True);
  EText.Brush.Style := bsClear;
  EText.SelStart := 0;
  EText.SelLength := 0;

  SendMessage(EText.Handle, EM_REQUESTRESIZE, 0, 0);
  EText.Lines.EndUpdate;

  if FromContact or ToContact then
    bnReply.Enabled := false
  else
    bnReply.Enabled := True;

  // check forward and back buttons
  NextBtn.Enabled := (NextItem <> -1);
  PrevBtn.Enabled := (PrevItem <> -1);

  FOverFile := IsFileEvent;
end;

procedure TEventDetailsFrm.PrevBtnClick(Sender: TObject);
begin
  SetItem(PrevItem);
  Assert(Assigned(FParentForm));
  if FParentForm.hg.Selected <> FItem then
    FParentForm.hg.Selected := FItem;
end;

procedure TEventDetailsFrm.NextBtnClick(Sender: TObject);
begin
  SetItem(NextItem);
  Assert(Assigned(FParentForm));
  if FParentForm.hg.Selected <> FItem then
    FParentForm.hg.Selected := FItem;
end;

procedure TEventDetailsFrm.ResetItem;
begin
  SetItem(FItem);
end;

procedure TEventDetailsFrm.bnReplyClick(Sender: TObject);
begin
  FParentForm.ReplyQuoted(FItem);
end;

procedure TEventDetailsFrm.TranslateForm;
begin
  Caption            := TranslateUnicodeString(Caption);
  GroupBox.Caption   := TranslateUnicodeString(GroupBox.Caption);
  laType.Caption     := TranslateUnicodeString(laType.Caption);
  laDateTime.Caption := TranslateUnicodeString(laDateTime.Caption);
  laFrom.Caption     := TranslateUnicodeString(laFrom.Caption);
  laTo.Caption       := TranslateUnicodeString(laTo.Caption);
  EFromMore.Hint     := TranslateUnicodeString(EFromMore.Hint);
  EToMore.Hint       := TranslateUnicodeString(EToMore.Hint);
  PrevBtn.Caption    := TranslateUnicodeString(PrevBtn.Caption);
  NextBtn.Caption    := TranslateUnicodeString(NextBtn.Caption);
  CloseBtn.Caption   := TranslateUnicodeString(CloseBtn.Caption);
  bnReply.Caption    := TranslateUnicodeString(bnReply.Caption);
  imDirection.Hint   := TranslateUnicodeString(imDirection.Hint);
  TranslateMenu(pmEText.Items);
end;

procedure TEventDetailsFrm.pmETextPopup(Sender: TObject);
begin
  CopyText.Enabled := (EText.SelLength > 0);
  SendMessage1.Enabled := (ParentForm.hContact <> 0);
  ReplyQuoted1.Enabled := (ParentForm.hContact <> 0);
  ToogleItemProcessing.Checked := GridOptions.TextFormatting;
  OpenLinkNW.Visible := FOverURL;
  OpenLink.Visible := FOverURL;
  CopyLink.Visible := FOverURL;
  BrowseReceivedFiles.Visible := FOverFile and not FOverURL;
  OpenFileFolder.Visible := FOverFile and not FOverURL and (SavedFileDir <> '');
  CopyFilename.Visible := FOverFile and not FOverURL;
end;

procedure TEventDetailsFrm.SelectAllClick(Sender: TObject);
begin
  EText.SelectAll;
end;

procedure TEventDetailsFrm.CopyTextClick(Sender: TObject);
begin
  EText.CopyToClipboard;
end;

procedure TEventDetailsFrm.CopyAllClick(Sender: TObject);
var
  ss, sl: Integer;
begin
  // CopyToClip(EText.Lines.Text,Handle,ParentForm.UserCodepage);
  EText.Lines.BeginUpdate;
  ss := EText.SelStart;
  sl := EText.SelLength;
  EText.SelectAll;
  EText.CopyToClipboard;
  EText.SelStart := ss;
  EText.SelLength := sl;
  EText.Lines.EndUpdate;
end;

procedure TEventDetailsFrm.SendMessage1Click(Sender: TObject);
begin
  if ParentForm.hContact = 0 then
    Exit;
  SendMessageTo(ParentForm.hContact);
end;

procedure TEventDetailsFrm.ReplyQuoted1Click(Sender: TObject);
begin
  if ParentForm.hContact = 0 then
    Exit;
  FParentForm.ReplyQuoted(FItem);
end;

procedure TEventDetailsFrm.WMSetCursor(var Message: TWMSetCursor);
var
  p: TPoint;
begin
  if (FRichHeight > 0) and (Message.CursorWnd = EText.Handle) and (Message.HitTest = HTCLIENT)
  then
  begin
    p := EText.ScreenToClient(Mouse.CursorPos);
    if p.Y > FRichHeight then
    begin
      if Windows.GetCursor <> Screen.Cursors[crIBeam] then
        Windows.SetCursor(Screen.Cursors[crIBeam]);
      Message.Result := 1;
      Exit;
    end;
  end;
  inherited;
end;

procedure TEventDetailsFrm.ToogleItemProcessingClick(Sender: TObject);
begin
  GridOptions.TextFormatting := not GridOptions.TextFormatting;
end;

procedure TEventDetailsFrm.LoadButtonIcons;
begin
  with EFromMore.Glyph do
  begin
    Width := 16;
    Height := 16;
    Canvas.Brush.Color := clBtnFace;
    Canvas.FillRect(Canvas.ClipRect);
    DrawIconEx(Canvas.Handle, 0, 0, hppIcons[HPP_ICON_CONTACDETAILS].Handle, 16, 16, 0,
      Canvas.Brush.Handle, DI_NORMAL);
  end;
  with EToMore.Glyph do
  begin
    Width := 16;
    Height := 16;
    Canvas.Brush.Color := clBtnFace;
    Canvas.FillRect(Canvas.ClipRect);
    DrawIconEx(Canvas.Handle, 0, 0, hppIcons[HPP_ICON_CONTACDETAILS].Handle, 16, 16, 0,
      Canvas.Brush.Handle, DI_NORMAL);
  end;
  with PrevBtn.Glyph do
  begin
    PrevBtn.NumGlyphs := 2;
    Width := 16 * 2;
    Height := 16;
    Canvas.Brush.Color := clBtnFace;
    Canvas.FillRect(Canvas.ClipRect);
    DrawIconEx(Canvas.Handle, 0, 0, hppIcons[HPP_ICON_SEARCHUP].Handle, 16, 16, 0,
      Canvas.Brush.Handle, DI_NORMAL);
    DrawState(Canvas.Handle, 0, nil, hppIcons[HPP_ICON_SEARCHUP].Handle, 0, 16, 0, 16, 16,
      DST_ICON or DSS_DISABLED);
  end;
  with NextBtn.Glyph do
  begin
    NextBtn.NumGlyphs := 2;
    Width := 16 * 2;
    Height := 16;
    Canvas.Brush.Color := clBtnFace;
    Canvas.FillRect(Canvas.ClipRect);
    DrawIconEx(Canvas.Handle, 0, 0, hppIcons[HPP_ICON_SEARCHDOWN].Handle, 16, 16, 0,
      Canvas.Brush.Handle, DI_NORMAL);
    DrawState(Canvas.Handle, 0, nil, hppIcons[HPP_ICON_SEARCHDOWN].Handle, 0, 16, 0, 16, 16,
      DST_ICON or DSS_DISABLED);
  end;
end;

procedure TEventDetailsFrm.LoadMessageIcons;
var
  ic: hIcon;
  er: PEventRecord;
begin
  er := GetEventRecord(FParentForm.hg.Items[FItem]);
  if er.i = -1 then
    ic := 0
  else if er.iSkin = -1 then
    ic := hppIcons[er.i].Handle
  else
    ic := skinIcons[er.i].Handle;
  if ic = 0 then
    ic := hppIcons[HPP_ICON_CONTACTHISTORY].Handle;
  Icon.Handle := CopyIcon(ic);
  with imDirection.Picture.Bitmap do
  begin
    Width := 16;
    Height := 16;
    Canvas.Brush.Color := clBtnFace;
    Canvas.FillRect(Canvas.ClipRect);
    if mtIncoming in FParentForm.hg.Items[FItem].MessageType then
      ic := hppIcons[EventRecords[mtIncoming].i].Handle
    else if mtOutgoing in FParentForm.hg.Items[FItem].MessageType then
      ic := hppIcons[EventRecords[mtOutgoing].i].Handle
    else
      Exit;
    DrawIconEx(Canvas.Handle, 0, 0, ic, 16, 16, 0, Canvas.Brush.Handle, DI_NORMAL);
  end;
end;

procedure TEventDetailsFrm.HMIconsChanged(var M: TMessage);
begin
  LoadMessageIcons;
end;

procedure TEventDetailsFrm.HMIcons2Changed(var M: TMessage);
begin
  LoadMessageIcons;
  LoadButtonIcons;
end;

procedure TEventDetailsFrm.ETextResizeRequest(Sender: TObject; Rect: TRect);
begin
  FRichHeight := Rect.Bottom - Rect.Top;
end;

procedure TEventDetailsFrm.HMEventDeleted(var Message: TMessage);
begin
  if uint_ptr(Message.WParam) = ParentForm.History[ParentForm.GridIndexToHistory(FItem)] then
    Close;
end;

procedure TEventDetailsFrm.WMSysColorChange(var Message: TMessage);
begin
  inherited;
  LoadMessageIcons;
  LoadButtonIcons;
  Repaint;
end;

procedure TEventDetailsFrm.OpenLinkNWClick(Sender: TObject);
begin
  if SavedLinkUrl = '' then
    Exit;
  OpenUrl(SavedLinkUrl, True);
  SavedLinkUrl := '';
end;

procedure TEventDetailsFrm.OpenLinkClick(Sender: TObject);
begin
  if SavedLinkUrl = '' then
    Exit;
  OpenUrl(SavedLinkUrl, false);
  SavedLinkUrl := '';
end;

procedure TEventDetailsFrm.CopyLinkClick(Sender: TObject);
begin
  if SavedLinkUrl = '' then
    Exit;
  CopyToClip(SavedLinkUrl, CP_ACP);
  SavedLinkUrl := '';
end;

procedure TEventDetailsFrm.ETextMouseMove(Sender: TObject; Shift: TShiftState; X, Y: Integer);
begin
  FOverURL := false;
end;

function TEventDetailsFrm.GetPrevItem: Integer;
begin
  if Assigned(FParentForm) then
    Result := FParentForm.hg.GetPrev(FItem)
  else
    Result := -1;
end;

function TEventDetailsFrm.GetNextItem: Integer;
begin
  if Assigned(FParentForm) then
    Result := FParentForm.hg.GetNext(FItem)
  else
    Result := -1;
end;

function TEventDetailsFrm.IsFileEvent: Boolean;
begin
  Result := Assigned(FParentForm) and (mtFile in FParentForm.hg.Items[FItem].MessageType);
  if Result then
  begin
    // Auto CP_ACP usage
    SavedLinkUrl := ExtractFileName(String(FParentForm.hg.Items[FItem].Extended));
    SavedFileDir := ExtractFileDir(String(FParentForm.hg.Items[FItem].Extended));
  end;
end;

procedure TEventDetailsFrm.OpenFileFolderClick(Sender: TObject);
begin
  if SavedFileDir = '' then
    Exit;
  ShellExecuteW(0, 'open', PChar(SavedFileDir), nil, nil, SW_SHOW);
  SavedFileDir := '';
end;

procedure TEventDetailsFrm.BrowseReceivedFilesClick(Sender: TObject);
var
  Path: Array [0 .. MAX_PATH] of AnsiChar;
begin
  CallService(MS_FILE_GETRECEIVEDFILESFOLDER, FParentForm.hContact, LPARAM(@Path));
  ShellExecuteA(0, 'open', Path, nil, nil, SW_SHOW);
end;

procedure TEventDetailsFrm.ETextURLClick(Sender: TObject; const URLText: String; Button: TMouseButton);
begin
  SavedLinkUrl := URLText;
  case Button of
    mbLeft : OpenLinkNW.Click;
    mbRight: FOverURL := True;
  end;
end;

end.
