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
  GlobalSearch (historypp project)

  Version:   1.0
  Created:   05.08.2004
  Author:    Oxygen

  [ Description ]

  Here we have the form and UI for global searching. Curious
  can go to hpp_searchthread for internals of searching.

  [ History ]

  1.5 (05.08.2004)
  First version

  [ Modifications ]
  none

  [ Known Issues ]

  * When doing HotSearch, and then backspacing to empty search AnsiString
  grid doesn't return to the first item HotSearch started from
  unlike in HistoryForm. Probably shouldn't be done, because too much checking
  to reset LastHotIdx should be done, considering how much filtering &
  sorting is performed.

  Contributors: theMIROn, Art Fedorov
  ----------------------------------------------------------------------------- }

unit GlobalSearch;

interface

uses
  Windows, Messages, SysUtils, Classes, Graphics, Controls, Forms, Dialogs,
  StdCtrls, ExtCtrls, ComCtrls, Menus,
  HistoryGrid,
  m_api,
  hpp_global, hpp_searchthread,
  RichEdit,
  ImgList, HistoryControls, Buttons, Math, CommCtrl,
  Contnrs, ToolWin, ShellAPI;

const
  HM_SRCH_CONTACTICONCHANGED = HM_SRCH_BASE + 3;

type
  THPPContactInfo = class(TObject)
  public
    Proto: AnsiString;
    Codepage: Cardinal;
    RTLMode: TRTLMode;
    Name: String;
    ProfileName: String;
    Handle: THandle;
  end;

  TSearchItem = record
    hDBEvent: THandle;
    Contact: THPPContactInfo;
  end;

  TfmGlobalSearch = class(TForm)
    paClient: TPanel;
    paSearch: TPanel;
    laSearch: TLabel;
    edSearch: THppEdit;
    bnSearch: TButton;
    sb: TStatusBar;
    paProgress: TPanel;
    pb: TProgressBar;
    laProgress: TLabel;
    pmGrid: TPopupMenu;
    Open1: TMenuItem;
    Copy1: TMenuItem;
    CopyText1: TMenuItem;
    N1: TMenuItem;
    N2: TMenuItem;
    spContacts: TSplitter;
    ilContacts: TImageList;
    paContacts: TPanel;
    lvContacts: TListView;
    SendMessage1: TMenuItem;
    ReplyQuoted1: TMenuItem;
    SaveSelected1: TMenuItem;
    SaveDialog: TSaveDialog;
    tiFilter: TTimer;
    paHistory: TPanel;
    hg: THistoryGrid;
    paFilter: TPanel;
    sbClearFilter: TSpeedButton;
    edFilter: THppEdit;
    pbFilter: TPaintBox;
    Delete1: TMenuItem;
    N3: TMenuItem;
    Bookmark1: TMenuItem;
    ToolBar: THppToolBar;
    paAdvanced: TPanel;
    paRange: TPanel;
    rbAny: TRadioButton;
    rbAll: TRadioButton;
    rbExact: TRadioButton;
    laAdvancedHead: TLabel;
    sbAdvancedClose: TSpeedButton;
    sbRangeClose: TSpeedButton;
    dtRange1: TDateTimePicker;
    laRange1: TLabel;
    laRange2: TLabel;
    dtRange2: TDateTimePicker;
    laRangeHead: TLabel;
    tbEventsFilter: THppSpeedButton;
    tbAdvanced: THppToolButton;
    tbRange: THppToolButton;
    ToolButton2: THppToolButton;
    ilToolbar: TImageList;
    beRange: TBevel;
    beAdvanced: TBevel;
    ToolButton3: THppToolButton;
    tbSearch: THppToolButton;
    tbFilter: THppToolButton;
    pmEventsFilter: TPopupMenu;
    N4: TMenuItem;
    Customize1: TMenuItem;
    pmInline: TPopupMenu;
    InlineCopy: TMenuItem;
    InlineCopyAll: TMenuItem;
    InlineSelectAll: TMenuItem;
    MenuItem10: TMenuItem;
    InlineTextFormatting: TMenuItem;
    MenuItem6: TMenuItem;
    InlineSendMessage: TMenuItem;
    InlineReplyQuoted: TMenuItem;
    pmLink: TPopupMenu;
    OpenLink: TMenuItem;
    OpenLinkNW: TMenuItem;
    MenuItem2: TMenuItem;
    CopyLink: TMenuItem;
    mmAcc: TMainMenu;
    mmToolbar: TMenuItem;
    mmService: TMenuItem;
    mmHideMenu: TMenuItem;
    mmShortcuts: TMenuItem;
    mmBookmark: TMenuItem;
    tbBookmarks: THppToolButton;
    ToolButton1: THppToolButton;
    TopPanel: TPanel;
    N5: TMenuItem;
    SelectAll1: TMenuItem;
    pmFile: TPopupMenu;
    FileActions: TMenuItem;
    BrowseReceivedFiles: TMenuItem;
    OpenFileFolder: TMenuItem;
    N6: TMenuItem;
    CopyFilename: TMenuItem;
    N7: TMenuItem;
    paEvents: TPanel;
    laEvents: TLabel;
    beEvents: TBevel;
    laEventsHead: TLabel;
    sbEventsClose: TSpeedButton;
    tbEvents: THppToolButton;
    cbEvents: TComboBox;
    ToolButton4: THppToolButton;
    tbCopy: THppToolButton;
    tbDelete: THppToolButton;
    tbSave: THppToolButton;
    procedure pbFilterPaint(Sender: TObject);
    procedure edFilterKeyUp(Sender: TObject; var Key: Word; Shift: TShiftState);
    procedure tiFilterTimer(Sender: TObject);
    procedure sbClearFilterClick(Sender: TObject);
    procedure edPassKeyPress(Sender: TObject; var Key: Char);
    procedure edSearchKeyPress(Sender: TObject; var Key: Char);
    procedure hgItemDelete(Sender: TObject; Index: Integer);
    procedure OnCNChar(var Message: TWMChar); message WM_CHAR;
    procedure tbSaveClick(Sender: TObject);
    procedure hgPopup(Sender: TObject);
    procedure ReplyQuoted1Click(Sender: TObject);
    procedure SendMessage1Click(Sender: TObject);
    procedure edFilterKeyDown(Sender: TObject; var Key: Word; Shift: TShiftState);
    procedure hgItemFilter(Sender: TObject; Index: Integer; var Show: Boolean);
    procedure edFilterChange(Sender: TObject);
    procedure FormDestroy(Sender: TObject);
    procedure edSearchChange(Sender: TObject);
    procedure hgKeyDown(Sender: TObject; var Key: Word; Shift: TShiftState);
    procedure hgState(Sender: TObject; State: TGridState);
    procedure hgSearchFinished(Sender: TObject; const Text: String; Found: Boolean);
    procedure hgSearchItem(Sender: TObject; Item, ID: Integer; var Found: Boolean);
    // procedure FormHide(Sender: TObject);
    procedure FormMouseWheel(Sender: TObject; Shift: TShiftState; WheelDelta: Integer;
      MousePos: TPoint; var Handled: Boolean);
    procedure lvContactsSelectItem(Sender: TObject; Item: TListItem; Selected: Boolean);
    procedure hgNameData(Sender: TObject; Index: Integer; var Name: String);
    procedure hgTranslateTime(Sender: TObject; Time: Cardinal; var Text: String);
    procedure FormCreate(Sender: TObject);
    procedure FormClose(Sender: TObject; var Action: TCloseAction);
    procedure bnSearchClick(Sender: TObject);
    procedure FormCloseQuery(Sender: TObject; var CanClose: Boolean);
    procedure hgItemData(Sender: TObject; Index: Integer; var Item: THistoryItem);
    procedure hgDblClick(Sender: TObject);
    procedure edSearchEnter(Sender: TObject);
    procedure edSearchKeyUp(Sender: TObject; var Key: Word; Shift: TShiftState);
    procedure hgProcessRichText(Sender: TObject; Handle: Cardinal; Item: Integer);
    procedure FormShow(Sender: TObject);
    procedure hgKeyUp(Sender: TObject; var Key: Word; Shift: TShiftState);
    procedure hgUrlClick(Sender: TObject; Item: Integer; const URLText: String; Button: TMouseButton);
    procedure edPassKeyUp(Sender: TObject; var Key: Word; Shift: TShiftState);
    procedure FormKeyDown(Sender: TObject; var Key: Word; Shift: TShiftState);
    procedure hgSelect(Sender: TObject; Item, OldItem: Integer);
    procedure tbCopyClick(Sender: TObject);
    procedure CopyText1Click(Sender: TObject);
    procedure tbDeleteClick(Sender: TObject);
    procedure hgRTLEnabled(Sender: TObject; BiDiMode: TBiDiMode);
    procedure Bookmark1Click(Sender: TObject);
    procedure hgBookmarkClick(Sender: TObject; Item: Integer);
    procedure lvContactsContextPopup(Sender: TObject; MousePos: TPoint; var Handled: Boolean);
    procedure tbAdvancedClick(Sender: TObject);
    procedure tbRangeClick(Sender: TObject);
    procedure sbAdvancedCloseClick(Sender: TObject);
    procedure sbRangeCloseClick(Sender: TObject);
    procedure tbEventsFilterClick(Sender: TObject);
    procedure EventsFilterItemClick(Sender: TObject);
    procedure Customize1Click(Sender: TObject);
    procedure InlineCopyClick(Sender: TObject);
    procedure hgInlinePopup(Sender: TObject);
    procedure hgInlineKeyDown(Sender: TObject; var Key: Word; Shift: TShiftState);
    procedure InlineCopyAllClick(Sender: TObject);
    procedure InlineSelectAllClick(Sender: TObject);
    procedure InlineTextFormattingClick(Sender: TObject);
    procedure InlineReplyQuotedClick(Sender: TObject);
    procedure CopyLinkClick(Sender: TObject);
    procedure OpenLinkClick(Sender: TObject);
    procedure OpenLinkNWClick(Sender: TObject);
    procedure mmHideMenuClick(Sender: TObject);
    procedure mmToolbarClick(Sender: TObject);
    procedure pmEventsFilterPopup(Sender: TObject);
    procedure tbBookmarksClick(Sender: TObject);
    procedure SelectAll1Click(Sender: TObject);
    procedure OpenFileFolderClick(Sender: TObject);
    procedure BrowseReceivedFilesClick(Sender: TObject);
    procedure tbEventsClick(Sender: TObject);
    procedure sbEventsCloseClick(Sender: TObject);
    procedure lvContactsDblClick(Sender: TObject);
    procedure hgChar(Sender: TObject; var achar: WideChar; Shift: TShiftState);
    procedure edFilterKeyPress(Sender: TObject; var Key: Char);
  private
    UserMenu: hMenu;
    UserMenuContact: THandle;
    WasReturnPressed: Boolean;
    LastUpdateTime: Cardinal;
    HotString: String;
    hHookContactIconChanged: THandle;
    FContactFilter: THandle;
    FFiltered: Boolean;
    IsSearching: Boolean;
    IsBookmarksMode: Boolean;
    History: array of TSearchItem;
    FilterHistory: array of Integer;
    CurContact: THandle;
    SearchThread: TSearchThread;
    stime: DWord;
    ContactsFound: Integer;
    AllItems: Integer;
    AllContacts: Integer;
    HotFilterString: String;
    FormState: TGridState;
    SavedLinkUrl: String;
    SavedFileDir: String;

    procedure WMGetMinMaxInfo(var Message: TWMGetMinMaxInfo); message WM_GETMINMAXINFO;
    procedure WMSysColorChange(var Message: TMessage); message WM_SYSCOLORCHANGE;

    procedure SMPrepare(var M: TMessage); message HM_STRD_PREPARE;
    procedure SMProgress(var M: TMessage); message HM_STRD_PROGRESS;
    procedure SMItemsFound(var M: TMessage); message HM_STRD_ITEMSFOUND;
    procedure SMNextContact(var M: TMessage); message HM_STRD_NEXTCONTACT;
    procedure SMFinished(var M: TMessage); message HM_STRD_FINISHED;

    function FindHistoryItemByHandle(hDBEvent: THandle): Integer;
    procedure DeleteEventFromLists(Item: Integer);

    procedure HMEventDeleted(var M: TMessage); message HM_MIEV_EVENTDELETED;
    procedure HMPreShutdown(var M: TMessage); message HM_MIEV_PRESHUTDOWN;
    procedure HMContactDeleted(var M: TMessage); message HM_MIEV_CONTACTDELETED;
    // procedure HMMetaDefaultChanged(var M: TMessage); message HM_MIEV_METADEFCHANGED;

    procedure HMContactIconChanged(var M: TMessage); message HM_SRCH_CONTACTICONCHANGED;

    procedure HMIcons2Changed(var M: TMessage); message HM_NOTF_ICONS2CHANGED;
    procedure HMBookmarksChanged(var M: TMessage); message HM_NOTF_BOOKMARKCHANGED;
    procedure HMFiltersChanged(var M: TMessage); message HM_NOTF_FILTERSCHANGED;
    procedure HMAccChanged(var M: TMessage); message HM_NOTF_ACCCHANGED;
    procedure HMNickChanged(var M: TMessage); message HM_NOTF_NICKCHANGED;
    procedure TranslateForm;

    procedure HookEvents;
    procedure UnhookEvents;

    procedure ShowContacts(Show: Boolean);

    procedure SearchNext(Rev: Boolean; Warp: Boolean = True);
    procedure ReplyQuoted(Item: Integer);
    procedure StartHotFilterTimer;
    procedure EndHotFilterTimer;

    procedure StopSearching;
  private
    LastAddedContact: THPPContactInfo;
    ContactList: TObjectList;
    // function FindContact(hContact: Integer): THPPContactInfo;
    function AddContact(hContact: THandle): THPPContactInfo;
  protected
    procedure LoadPosition;
    procedure SavePosition;
    procedure WndProc(var Message: TMessage); override;

    function IsFileEvent(Index: Integer): Boolean;

    procedure ToggleAdvancedPanel(Show: Boolean);
    procedure ToggleRangePanel(Show: Boolean);
    procedure ToggleEventsPanel(Show: Boolean);
    procedure OrganizePanels;
    procedure ToggleMainMenu(Enabled: Boolean);

    procedure SetEventFilter(FilterIndex: Integer = -1);
    procedure CreateEventsFilterMenu;
  public
    CustomizeFiltersForm: TForm;
    procedure SetRecentEventsPosition(OnTop: Boolean);
  published
    // fix for splitter baug:
    procedure AlignControls(Control: TControl; var ARect: TRect); override;

    function GetSearchItem(GridIndex: Integer): TSearchItem;
    function GetContactInfo(hContact: THandle): THPPContactInfo;

    procedure DisableFilter;
    procedure FilterOnContact(hContact: THandle);

    procedure LoadButtonIcons;
    procedure LoadContactsIcons;
    procedure LoadToolbarIcons;

    procedure LoadAccMenu;
    procedure LoadEventFilterButton;
  public
    { Public declarations }
  end;

var
  fmGlobalSearch: TfmGlobalSearch;

const
  DEFAULT_SEARCH_TEXT = 'http: ftp: www. ftp.';

var
  GlobalSearchAllResultsIcon: Integer = -1;

implementation

uses
  hpp_options, hpp_itemprocess, hpp_messages, CustomizeFiltersForm,
  hpp_database, hpp_eventfilters, hpp_contacts, hpp_events, hpp_richedit,
  hpp_forms, hpp_services, hpp_bookmarks;

{$R *.DFM}

function TfmGlobalSearch.AddContact(hContact: THandle): THPPContactInfo;
var
  ci: THPPContactInfo;
  SubContact: TMCONTACT;
  SubProtocol: AnsiString;
begin
  ci := THPPContactInfo.Create;
  ci.Handle      := hContact;
  ci.Proto       := GetContactProto(CurContact, SubContact, SubProtocol);
  ci.Codepage    := GetContactCodePage(hContact, ci.Proto);
  ci.Name        := GetContactDisplayName(ci.Handle, ci.Proto, True);
  ci.ProfileName := GetContactDisplayName(0, SubProtocol);
  ci.RTLMode     := GetContactRTLModeTRTL(ci.Handle, ci.Proto);
  ContactList.Add(ci);
  Result := ci;
end;

// fix for infamous splitter bug!
// thanks to Greg Chapman
// http://groups.google.com/group/borland.public.delphi.objectpascal/browse_thread/thread/218a7511123851c3/5ada76e08038a75b%235ada76e08038a75b?sa=X&oi=groupsr&start=2&num=3
procedure TfmGlobalSearch.AlignControls(Control: TControl; var ARect: TRect);
begin
  inherited;
  if paContacts.Width = 0 then
    paContacts.Left := spContacts.Left;
end;

procedure TfmGlobalSearch.FormCreate(Sender: TObject);
// var
// NonClientMetrics: TNonClientMetrics;
begin
  // Setting different system font different way. For me works the same
  // but some said it produces better results than DesktopFont
  // Leave it here for possible future use.
  //
  // NonClientMetrics.cbSize := SizeOf(NonClientMetrics);
  // SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, @NonClientMetrics, 0);
  // Font.Handle := CreateFontIndirect(NonClientMetrics.lfMessageFont);
  // if Scaled then begin
  // Font.Height := NonClientMetrics.lfMessageFont.lfHeight;
  // end;
  Icon.ReleaseHandle;
  Icon.Handle := CopyIcon(hppIcons[HPP_ICON_GLOBALSEARCH].Handle);

  DesktopFont := True;
  MakeFontsParent(Self);

  DoubleBuffered := True;
  MakeDoubleBufferedParent(Self);
  TopPanel.DoubleBuffered := False;
  hg.DoubleBuffered := False;

  FormState := gsIdle;

  ContactList := TObjectList.Create;

  ilContacts.Handle := CallService(MS_CLIST_GETICONSIMAGELIST, 0, 0);
  // delphi 2006 doesn't save toolbar's flat property in dfm if it is True
  // delphi 2006 doesn't save toolbar's edgeborder property in dfm
  ToolBar.Flat := True;
  ToolBar.EdgeBorders := [];

  LoadToolbarIcons;
  LoadButtonIcons;
  LoadContactsIcons;

  TranslateForm;

  // File actions from context menu support
  AddMenuArray(pmGrid, [FileActions], -1);

  LoadAccMenu; // load accessability menu before LoadToolbar
  // put here because we want to translate everything
  // before copying to menu
  ToggleMainMenu(GetDBBool(hppDBName, 'Accessability', False));
end;

procedure TfmGlobalSearch.SMFinished(var M: TMessage);
// var
// sbt: WideString;
begin
  stime := GetTickCount - SearchThread.SearchStart;
  AllContacts := SearchThread.AllContacts;
  AllItems := SearchThread.AllEvents;
  IsSearching := False;
  bnSearch.Caption := TranslateW('Search');
  paProgress.Hide;
  // paFilter.Show;
  // if change, change also in hg.State:
  // sbt := WideFormat(TranslateW('%.0n items in %d contacts found. Searched for %.1f sec in %.0n items.'),[Length(History)/1, ContactsFound, stime/1000, AllItems/1]);
  // sb.SimpleText := sbt;
  hgState(Self, hg.State);
  if Length(History) = 0 then
    ShowContacts(False);
end;

procedure TfmGlobalSearch.SMItemsFound(var M: TMessage);
var
  li: TListItem;
  ci: THPPContactInfo;
  Buffer: PDBArray;
  FiltOldSize, OldSize, i, BufCount: Integer;
begin
  // wParam - array of hDBEvent, lParam - array size
  Buffer := PDBArray(M.wParam);
  BufCount := Integer(M.LParam);
  OldSize := Length(History);
  SetLength(History, OldSize + BufCount);

  if (LastAddedContact = nil) or (LastAddedContact.Handle <> CurContact) then
  begin
    ci := AddContact(CurContact);
    LastAddedContact := ci;
  end;

  for i := 0 to BufCount - 1 do
  begin
    History[OldSize + i].hDBEvent := Buffer^[i];
    History[OldSize + i].Contact := LastAddedContact;
    // History[OldSize + i].hContact := CurContact;
    // History[OldSize + i].ContactName := CurContactName;
    // History[OldSize + i].ProfileName := CurProfileName;
    // History[OldSize + i].Proto := CurProto;
  end;

  FreeMem(Buffer, SizeOf(Buffer^));

  if (lvContacts.Items.Count = 0) or
    (THandle(lvContacts.Items.Item[lvContacts.Items.Count - 1].Data) <> CurContact) then
  begin
    if lvContacts.Items.Count = 0 then
    begin
      li := lvContacts.Items.Add;
      li.Caption := TranslateW('All Results');
      li.ImageIndex := GlobalSearchAllResultsIcon;
      li.Selected := True;
    end;
    li := lvContacts.Items.Add;
    if CurContact = 0 then
      li.Caption := TranslateW('System History')
    else
    begin
      li.Caption := LastAddedContact.Name;
      // li.Caption := CurContactName;
      Inc(ContactsFound);
    end;
    li.ImageIndex := CallService(MS_CLIST_GETCONTACTICON, CurContact, 0);
    // meTest.Lines.Add(CurContactName+' icon is '+IntToStr(CallService(MS_CLIST_GETCONTACTICON,CurContact,0)));
    li.Data := Pointer(CurContact);
  end;

  if FFiltered then
  begin
    if CurContact = FContactFilter then
    begin
      FiltOldSize := Length(FilterHistory);
      for i := 0 to BufCount - 1 do
        FilterHistory[FiltOldSize + i] := OldSize + i;
      hg.Allocate(Length(FilterHistory));
    end;
  end
  else
    hg.Allocate(Length(History));

  if (hg.Count > 0) and (hg.Selected = -1) then
    hg.Selected := 0;

  paFilter.Visible := True;
  if not paContacts.Visible then
  begin
    ShowContacts(True);
    hg.Selected := 0;
    hg.SetFocus;
  end;

  tbEventsFilter.Enabled := True;
  // dirty hack: readjust scrollbars
  hg.Perform(WM_SIZE, SIZE_RESTORED, MakeLParam(hg.ClientWidth, hg.ClientHeight));
  // hg.Repaint;
  // Application.ProcessMessages;
end;

procedure TfmGlobalSearch.SMNextContact(var M: TMessage);
var
  CurProto: AnsiString;
begin
  // wParam - hContact, lParam - 0
  CurContact := M.wParam;
  if CurContact = 0 then
    CurProto := 'ICQ'
  else
    CurProto := GetContactProto(CurContact);
  laProgress.Caption := Format(TranslateW('Searching "%s"...'),
    [GetContactDisplayName(CurContact, CurProto, True)]);
end;

procedure TfmGlobalSearch.SMPrepare(var M: TMessage);
begin
  LastUpdateTime := 0;
  ContactsFound := 0;
  AllItems := 0;
  AllContacts := 0;
  FFiltered := False;

  // hg.Filter := GenerateEvents(FM_EXCLUDE,[]);
  hg.Selected := -1;
  hg.Allocate(0);

  SetLength(FilterHistory, 0);
  SetLength(History, 0);

  IsSearching := True;
  bnSearch.Caption := TranslateW('Stop');

  tbEventsFilter.Enabled := False;
  sb.SimpleText := TranslateW('Searching... Please wait.');
  laProgress.Caption := TranslateW('Preparing search...');
  pb.Position := 0;
  paProgress.Show;
  paFilter.Visible := False;
  // ShowContacts(False);
  lvContacts.Items.Clear;
  ContactList.Clear;
  LastAddedContact := nil;
end;

procedure TfmGlobalSearch.SMProgress(var M: TMessage);
begin
  // wParam - progress; lParam - max

  if (GetTickCount - LastUpdateTime) < 100 then
    exit;
  LastUpdateTime := GetTickCount;

  pb.Max := M.LParam;
  pb.Position := M.wParam;
  // Application.ProcessMessages;

  // if change, change also in hg.OnState
  sb.SimpleText := Format(TranslateW('Searching... %.0n items in %d contacts found'),
    [Length(History) / 1, ContactsFound]);
end;

procedure TfmGlobalSearch.StartHotFilterTimer;
begin
  if tiFilter.Interval = 0 then
    EndHotFilterTimer
  else
  begin
    tiFilter.Enabled := False;
    tiFilter.Enabled := True;
    if pbFilter.Tag <> 1 then
    begin // use Tag to not repaint every keystroke
      pbFilter.Tag := 1;
      pbFilter.Repaint;
    end;
  end;
end;

procedure TfmGlobalSearch.tbAdvancedClick(Sender: TObject);
begin
  // when called from menu item handler
  if Sender <> tbAdvanced then
    tbAdvanced.Down := not tbAdvanced.Down;
  ToggleAdvancedPanel(tbAdvanced.Down);
end;

procedure TfmGlobalSearch.tbEventsFilterClick(Sender: TObject);
var
  p: TPoint;
begin
  p := tbEventsFilter.ClientOrigin;
  tbEventsFilter.ClientToScreen(p);
  pmEventsFilter.Popup(p.X, p.Y + tbEventsFilter.Height);
end;

procedure TfmGlobalSearch.tbRangeClick(Sender: TObject);
begin
  if Sender <> tbRange then
    tbRange.Down := not tbRange.Down;
  ToggleRangePanel(tbRange.Down);
end;

procedure TfmGlobalSearch.tiFilterTimer(Sender: TObject);
begin
  EndHotFilterTimer;
end;

procedure TfmGlobalSearch.edFilterChange(Sender: TObject);
begin
  StartHotFilterTimer;
end;

procedure TfmGlobalSearch.edFilterKeyDown(Sender: TObject; var Key: Word; Shift: TShiftState);
begin
  if Key in [VK_UP, VK_DOWN, VK_NEXT, VK_PRIOR] then
  begin
    SendMessage(hg.Handle, WM_KEYDOWN, Key, 0);
    Key := 0;
  end;
end;

procedure TfmGlobalSearch.edFilterKeyUp(Sender: TObject; var Key: Word; Shift: TShiftState);
begin
  if Key = VK_RETURN then
  begin
    hg.SetFocus;
    Key := 0;
  end;
end;

procedure TfmGlobalSearch.FormDestroy(Sender: TObject);
begin
  fmGlobalSearch := nil;
  if Assigned(CustomizeFiltersForm) then
    CustomizeFiltersForm.Release;
  ContactList.Free;
end;

procedure TfmGlobalSearch.WMGetMinMaxInfo(var Message: TWMGetMinMaxInfo);
begin
  inherited;
  with Message.MinMaxInfo^ do
  begin
    ptMinTrackSize.X := 320;
    ptMinTrackSize.Y := 240;
  end
end;

procedure TfmGlobalSearch.FormMouseWheel(Sender: TObject; Shift: TShiftState;
  WheelDelta: Integer; MousePos: TPoint; var Handled: Boolean);
var
  Ctrl: TControl;
begin
  Handled := True;
  Ctrl := paClient.ControlAtPos(paClient.ScreenToClient(MousePos), False, True);
{$RANGECHECKS OFF}
  if Assigned(Ctrl) then
  begin
    if Ctrl.Name = 'paContacts' then
    begin
      Handled := not TListView(Ctrl).Focused;
      if Handled then
      begin
        // ??? what to do here?
        // how to tell listview to scroll?
      end;
    end
    else
    begin
      hg.Perform(WM_MOUSEWHEEL, MakeLong(MK_CONTROL, WheelDelta), 0);
    end;
  end;
{$RANGECHECKS ON}
end;

procedure TfmGlobalSearch.ToggleAdvancedPanel(Show: Boolean);
var
  Lock: Boolean;
begin
  Lock := Visible;
  if Lock then
    Lock := LockWindowUpdate(Handle);
  try
    tbAdvanced.Down := Show;
    paAdvanced.Visible := Show and tbAdvanced.Enabled;
    OrganizePanels;
  finally
    if Lock then
      LockWindowUpdate(0);
  end;
end;

procedure TfmGlobalSearch.ToggleRangePanel(Show: Boolean);
var
  Lock: Boolean;
begin
  Lock := Visible;
  if Lock then
    Lock := LockWindowUpdate(Handle);
  try
    tbRange.Down := Show;
    paRange.Visible := Show and tbRange.Enabled;
    edSearchChange(Self);
    OrganizePanels;
  finally
    if Lock then
      LockWindowUpdate(0);
  end;
end;

procedure TfmGlobalSearch.ToggleEventsPanel(Show: Boolean);
var
  Lock: Boolean;
begin
  Lock := Visible;
  if Lock then
    Lock := LockWindowUpdate(Handle);
  try
    tbEvents.Down := Show;
    paEvents.Visible := Show and tbEvents.Enabled;
    edSearchChange(Self);
    OrganizePanels;
  finally
    if Lock then
      LockWindowUpdate(0);
  end;
end;

procedure TfmGlobalSearch.mmToolbarClick(Sender: TObject);
var
  i, n: Integer;
  pm: TPopupMenu;
  mi: TMenuItem;
  flag: Boolean;
begin
  for i := 0 to mmToolbar.Count - 1 do
  begin
    if mmToolbar.Items[i].Owner is THppToolButton then
    begin
      flag := TToolButton(mmToolbar.Items[i].Owner).Enabled
    end
    else if mmToolbar.Items[i].Owner is TSpeedButton then
    begin
      TMenuItem(mmToolbar.Items[i]).Caption := TSpeedButton(mmToolbar.Items[i].Owner).Hint;
      flag := TSpeedButton(mmToolbar.Items[i].Owner).Enabled
    end
    else
      flag := True;
    mmToolbar.Items[i].Enabled := flag;
    if mmToolbar.Items[i].Tag = 0 then
      continue;
    pm := TPopupMenu(Pointer(mmToolbar.Items[i].Tag));
    for n := pm.Items.Count - 1 downto 0 do
    begin
      mi := TMenuItem(pm.Items[n]);
      pm.Items.Remove(mi);
      mmToolbar.Items[i].Insert(0, mi);
    end;
  end;
end;

procedure TfmGlobalSearch.sbAdvancedCloseClick(Sender: TObject);
begin
  ToggleAdvancedPanel(False);
end;

procedure TfmGlobalSearch.sbClearFilterClick(Sender: TObject);
begin
  edFilter.Text := '';
  EndHotFilterTimer;
  hg.SetFocus;
end;

procedure TfmGlobalSearch.sbRangeCloseClick(Sender: TObject);
begin
  ToggleRangePanel(False);
end;

procedure TfmGlobalSearch.TranslateForm;
begin
  Caption := TranslateUnicodeString(Caption);

  laSearch.Caption := TranslateUnicodeString(laSearch.Caption);
  bnSearch.Caption := TranslateUnicodeString(bnSearch.Caption);
  edSearch.Left := laSearch.Left + laSearch.Width + 5;
  edSearch.Width := bnSearch.Left - edSearch.Left - 5;

  laAdvancedHead.Caption := TranslateUnicodeString(laAdvancedHead.Caption);
  rbAny.Caption := TranslateUnicodeString(rbAny.Caption);
  rbAll.Caption := TranslateUnicodeString(rbAll.Caption);
  rbExact.Caption := TranslateUnicodeString(rbExact.Caption);

  laRangeHead.Caption := TranslateUnicodeString(laRangeHead.Caption);
  laRange1.Caption := TranslateUnicodeString(laRange1.Caption);
  laRange2.Caption := TranslateUnicodeString(laRange2.Caption);

  laEventsHead.Caption := TranslateUnicodeString(laEventsHead.Caption);
  laEvents.Caption := TranslateUnicodeString(laEvents.Caption);
  cbEvents.Left := laEvents.Left + laEvents.Width + 10;

  sbClearFilter.Hint := TranslateUnicodeString(sbClearFilter.Hint);

  SaveDialog.Title := TranslateUnicodeString(PWideChar(SaveDialog.Title));

  TranslateToolbar(ToolBar);

  TranslateMenu(pmGrid.Items);
  TranslateMenu(pmInline.Items);
  TranslateMenu(pmLink.Items);
  TranslateMenu(pmFile.Items);
  TranslateMenu(pmEventsFilter.Items);

  hg.TxtFullLog    := TranslateUnicodeString(hg.TxtFullLog);
  hg.TxtGenHist1   := TranslateUnicodeString(hg.TxtGenHist1);
  hg.TxtGenHist2   := TranslateUnicodeString(hg.TxtGenHist2);
  hg.TxtHistExport := TranslateUnicodeString(hg.TxtHistExport);
  hg.TxtNoItems    := TranslateUnicodeString(hg.TxtNoItems);
  hg.TxtNoSuch     := TranslateUnicodeString(hg.TxtNoSuch);
  hg.TxtPartLog    := TranslateUnicodeString(hg.TxtPartLog);
  hg.TxtStartUp    := TranslateUnicodeString(hg.TxtStartUp);
  hg.TxtSessions   := TranslateUnicodeString(hg.TxtSessions);
end;

procedure TfmGlobalSearch.FilterOnContact(hContact: THandle);
var
  i: Integer;
begin
  if FFiltered and (FContactFilter = hContact) then
    exit;
  FFiltered := True;
  FContactFilter := hContact;
  SetLength(FilterHistory, 0);
  for i := 0 to Length(History) - 1 do
  begin
    if History[i].Contact.Handle = hContact then
    begin
      SetLength(FilterHistory, Length(FilterHistory) + 1);
      FilterHistory[High(FilterHistory)] := i;
    end;
  end;
  hg.Allocate(0);
  if Length(FilterHistory) > 0 then
  begin
    hg.Allocate(Length(FilterHistory));
    hg.Selected := 0;
  end
  else
    hg.Selected := -1;
  // dirty hack: readjust scrollbars
  hg.Perform(WM_SIZE, SIZE_RESTORED, MakeLParam(hg.ClientWidth, hg.ClientHeight));
end;

{ function TfmGlobalSearch.FindContact(hContact: Integer): THPPContactInfo;
  begin
  Result := nil;
  end; }

function TfmGlobalSearch.FindHistoryItemByHandle(hDBEvent: THandle): Integer;
var
  i: Integer;
begin
  Result := -1;
  for i := 0 to Length(History) - 1 do
  begin
    if History[i].hDBEvent = hDBEvent then
    begin
      Result := i;
      break;
    end;
  end;
end;

procedure TfmGlobalSearch.FormClose(Sender: TObject; var Action: TCloseAction);
begin
  try
    Action := caFree;
    SavePosition;
    UnhookEvents;
  except
  end;
end;

procedure TfmGlobalSearch.StopSearching;
begin
  bnSearch.Enabled := False;
  try
    SearchThread.Terminate(tpHigher);
    while IsSearching do
      Application.ProcessMessages;
  finally
    bnSearch.Enabled := True;
    if edSearch.CanFocus then
      edSearch.SetFocus;
  end;
end;

procedure TfmGlobalSearch.bnSearchClick(Sender: TObject);
var
  SearchProtected: Boolean;
begin
  if IsSearching then
  begin
    StopSearching;
    exit;
  end;
  // if edSearch.Text = '' then
  // raise Exception.Create('Enter text to search');

  SearchProtected := False;

  if Assigned(SearchThread) then
    FreeAndNil(SearchThread);
  SearchThread := TSearchThread.Create(True);

  if IsBookmarksMode then
    SearchThread.SearchMethod := [smBookmarks]
  else if edSearch.Text = '' then
    SearchThread.SearchMethod := []
  else if rbAny.Checked then
    SearchThread.SearchMethod := [smAnyWord]
  else if rbAll.Checked then
    SearchThread.SearchMethod := [smAllWords]
  else
    SearchThread.SearchMethod := [smExact];

  if paRange.Visible then
  begin
    SearchThread.SearchMethod := SearchThread.SearchMethod + [smRange];
    SearchThread.SearchRangeFrom := dtRange1.Date;
    SearchThread.SearchRangeTo := dtRange2.Date;
  end;

  if paEvents.Visible and (cbEvents.ItemIndex <> -1) then
  begin
    SearchThread.SearchMethod := SearchThread.SearchMethod + [smEvents];
    SearchThread.SearchEvents := hppEventFilters[cbEvents.ItemIndex].Events;
  end;

  SearchThread.Priority := tpLower;
  SearchThread.ParentHandle := Handle;
  SearchThread.SearchText := edSearch.Text;
  SearchThread.SearchProtectedContacts := SearchProtected;
  SearchThread.Resume;
end;

// takes index from *History* array as parameter
procedure TfmGlobalSearch.DeleteEventFromLists(Item: Integer);
var
  i: Integer;
  EventDeleted: Boolean;
begin
  if Item = -1 then
    exit;

  i := High(History);
  if Item <> i then
    Move(History[Item + 1], History[Item], (i - Item) * SizeOf(History[0]));
  SetLength(History, i);

  if not FFiltered then
    exit;

  EventDeleted := False;
  for i := 0 to Length(FilterHistory) - 1 do
  begin
    if EventDeleted then
    begin
      if i < Length(FilterHistory) - 1 then
        FilterHistory[i] := FilterHistory[i + 1];
      Dec(FilterHistory[i]);
    end
    else if FilterHistory[i] = Item then
      EventDeleted := True;
  end;
  if EventDeleted then
    SetLength(FilterHistory, Length(FilterHistory) - 1);
end;

procedure TfmGlobalSearch.DisableFilter;
begin
  if not FFiltered then
    exit;
  FFiltered := False;
  SetLength(FilterHistory, 0);
  hg.Allocate(0);
  if Length(History) > 0 then
  begin
    hg.Allocate(Length(History));
    hg.Selected := 0;
  end
  else
    hg.Selected := -1;
  // dirty hack: readjust scrollbars
  hg.Perform(WM_SIZE, SIZE_RESTORED, MakeLParam(hg.ClientWidth, hg.ClientHeight));
end;

procedure TfmGlobalSearch.FormCloseQuery(Sender: TObject; var CanClose: Boolean);
var
  flag: UINT;
  AppSysMenu: THandle;
begin
  CanClose := (hg.State in [gsIdle, gsInline]);
  if CanClose and IsSearching then
  begin
    // disable close button
    AppSysMenu := GetSystemMenu(Handle, False);
    flag := MF_GRAYED;
    EnableMenuItem(AppSysMenu, SC_CLOSE, MF_BYCOMMAND or flag);
    // laProgress.Caption := TranslateW('Please wait while closing the window...');
    // laProgress.Font.Style := [fsBold];
    // pb.Visible := False;
    if paProgress.Visible then
      paProgress.Hide;
    sb.SimpleText := TranslateW('Please wait while closing the window...');
    // terminate thread
    SearchThread.Terminate(tpHigher);
    repeat
      Application.ProcessMessages
    until not IsSearching;
  end;
  if CanClose and Assigned(SearchThread) then
    FreeAndNil(SearchThread);
end;

procedure TfmGlobalSearch.hgItemData(Sender: TObject; Index: Integer; var Item: THistoryItem);
begin
  Item := ReadEvent(GetSearchItem(Index).hDBEvent, GetSearchItem(Index).Contact.Codepage);
  Item.Proto := GetSearchItem(Index).Contact.Proto;
  Item.RTLMode := GetSearchItem(Index).Contact.RTLMode;
  Item.Bookmarked := BookmarkServer[GetSearchItem(Index).Contact.Handle].Bookmarked
    [GetSearchItem(Index).hDBEvent];
end;

procedure TfmGlobalSearch.hgItemDelete(Sender: TObject; Index: Integer);
var
  si: TSearchItem;
begin
  si := GetSearchItem(Index);
  if (FormState = gsDelete) and (si.hDBEvent <> 0) then
    db_event_delete(si.Contact.Handle, si.hDBEvent);
  if FFiltered then
    Index := FilterHistory[Index];
  DeleteEventFromLists(Index);
  hgState(hg, hg.State);
  Application.ProcessMessages;
end;

procedure TfmGlobalSearch.hgItemFilter(Sender: TObject; Index: Integer; var Show: Boolean);
begin
  if HotFilterString = '' then
    exit;
  if Pos(WideUpperCase(HotFilterString), WideUpperCase(hg.Items[Index].Text)) = 0 then
    Show := False;
end;

procedure TfmGlobalSearch.hgBookmarkClick(Sender: TObject; Item: Integer);
var
  val: Boolean;
  hContact, hDBEvent: THandle;
begin
  hContact := GetSearchItem(Item).Contact.Handle;
  hDBEvent := GetSearchItem(Item).hDBEvent;
  val := not BookmarkServer[hContact].Bookmarked[hDBEvent];
  BookmarkServer[hContact].Bookmarked[hDBEvent] := val;
end;

procedure TfmGlobalSearch.hgDblClick(Sender: TObject);
var
  oep: TOpenEventParams;
begin
  if hg.Selected = -1 then
    exit;
  oep.cbSize := SizeOf(oep);
  oep.hContact := GetSearchItem(hg.Selected).Contact.Handle;
  oep.hDBEvent := GetSearchItem(hg.Selected).hDBEvent;
  CallService(MS_HPP_OPENHISTORYEVENT, wParam(@oep), 0);
end;

procedure TfmGlobalSearch.edSearchChange(Sender: TObject);
begin
  bnSearch.Enabled := (edSearch.Text <> '') or paRange.Visible or paEvents.Visible;
end;

procedure TfmGlobalSearch.edSearchEnter(Sender: TObject);
begin
  // edSearch.SelectAll;
end;

procedure TfmGlobalSearch.LoadAccMenu;
var
  i: Integer;
  wstr: String;
  menuitem: TMenuItem;
  pm: TPopupMenu;
begin
  mmToolbar.Clear;
  for i := ToolBar.ButtonCount - 1 downto 0 do
  begin
    if ToolBar.Buttons[i].Style = tbsSeparator then
    begin
      menuitem := TMenuItem.Create(mmToolbar);
      menuitem.Caption := '-';
    end
    else
    begin
      menuitem := TMenuItem.Create(ToolBar.Buttons[i]);
      wstr := ToolBar.Buttons[i].Caption;
      if wstr = '' then
        wstr := ToolBar.Buttons[i].Hint;
      if wstr <> '' then
      begin
        pm := TPopupMenu(ToolBar.Buttons[i].PopupMenu);
        if pm = nil then
          menuitem.OnClick := ToolBar.Buttons[i].OnClick
        else
        begin
          menuitem.Tag := THandle(Pointer(pm));
        end;
        menuitem.Caption := wstr;
        menuitem.ShortCut := TextToShortCut(ToolBar.Buttons[i].HelpKeyword);
        menuitem.Enabled := ToolBar.Buttons[i].Enabled;
        menuitem.Visible := ToolBar.Buttons[i].Visible;
      end;
    end;
    mmToolbar.Insert(0, menuitem);
  end;
  mmToolbar.RethinkHotkeys;
end;

procedure LoadHPPIcons(var sb:TSpeedButton;idx:integer);
begin
  with sb.Glyph do
  begin
    Width := 16;
    Height := 16;
    Canvas.Brush.Color := clBtnFace;
    Canvas.FillRect(Canvas.ClipRect);
    DrawiconEx(Canvas.Handle, 0, 0, hppIcons[idx].Handle, 16, 16, 0,
      Canvas.Brush.Handle, DI_NORMAL);
  end;
end;

procedure TfmGlobalSearch.LoadButtonIcons;
begin
  LoadHPPIcons(sbClearFilter,HPP_ICON_HOTFILTERCLEAR);
  LoadHPPIcons(sbAdvancedClose,HPP_ICON_SESS_HIDE);
  LoadHPPIcons(sbRangeClose,HPP_ICON_SESS_HIDE);
  LoadHPPIcons(sbEventsClose,HPP_ICON_SESS_HIDE);
end;

procedure TfmGlobalSearch.LoadContactsIcons;
begin
  lvContacts.Items.BeginUpdate;

  if GlobalSearchAllResultsIcon = -1 then
    GlobalSearchAllResultsIcon := ImageList_AddIcon(ilContacts.Handle,
      hppIcons[HPP_ICON_SEARCH_ALLRESULTS].Handle)
  else
    ImageList_ReplaceIcon(ilContacts.Handle, GlobalSearchAllResultsIcon,
      hppIcons[HPP_ICON_SEARCH_ALLRESULTS].Handle);

  lvContacts.Items.EndUpdate;
end;

procedure TfmGlobalSearch.LoadEventFilterButton;
var
  pad: DWord;
  PadH, { PadV, } GlyphHeight: Integer;
  sz: TSize;
  FirstName, Name: String;
  PaintRect: TRect;
  DrawTextFlags: Cardinal;
  GlyphWidth: Integer;
begin
  FirstName := hppEventFilters[0].Name;
  Name := hppEventFilters[tbEventsFilter.Tag].Name;
  tbEventsFilter.Hint := Name; // show hint because the whole name may not fit in button

  pad := SendMessage(ToolBar.Handle, TB_GETPADDING, 0, 0);
  // PadV := HiWord(pad);
  PadH := LoWord(pad);

  tbEventsFilter.Glyph.Canvas.Font := tbEventsFilter.Font;
  sz := tbEventsFilter.Glyph.Canvas.TextExtent(FirstName);
  GlyphHeight := Max(sz.cy, 16);
  GlyphWidth := 16 + sz.cx + tbEventsFilter.Spacing;

  tbEventsFilter.Glyph.Height := GlyphHeight;
  tbEventsFilter.Glyph.Width := GlyphWidth * 2;
  tbEventsFilter.Glyph.Canvas.Brush.Color := clBtnFace;
  tbEventsFilter.Glyph.Canvas.FillRect(tbEventsFilter.Glyph.Canvas.ClipRect);
  DrawiconEx(tbEventsFilter.Glyph.Canvas.Handle, sz.cx + tbEventsFilter.Spacing,
    ((GlyphHeight - 16) div 2), hppIcons[HPP_ICON_DROPDOWNARROW].Handle, 16, 16, 0,
    tbEventsFilter.Glyph.Canvas.Brush.Handle, DI_NORMAL);
  DrawState(tbEventsFilter.Glyph.Canvas.Handle, 0, nil,
    hppIcons[HPP_ICON_DROPDOWNARROW].Handle, 0, sz.cx + tbEventsFilter.Spacing +
    GlyphWidth, ((GlyphHeight - 16) div 2), 0, 0, DST_ICON or DSS_DISABLED);

  PaintRect := Rect(0, ((GlyphHeight - sz.cy) div 2), GlyphWidth - 16 - tbEventsFilter.Spacing,
    tbEventsFilter.Glyph.Height);
  DrawTextFlags := DT_END_ELLIPSIS or DT_NOPREFIX or DT_CENTER;
  tbEventsFilter.Glyph.Canvas.Font.Color := clWindowText;
  DrawTextW(tbEventsFilter.Glyph.Canvas.Handle, @Name[1], Length(Name), PaintRect,
    DrawTextFlags);
  OffsetRect(PaintRect, GlyphWidth, 0);
  tbEventsFilter.Glyph.Canvas.Font.Color := clGrayText;
  DrawTextW(tbEventsFilter.Glyph.Canvas.Handle, @Name[1], Length(Name), PaintRect,
    DrawTextFlags);
  tbEventsFilter.Width := GlyphWidth + 2 * PadH;
  tbEventsFilter.NumGlyphs := 2;
end;

procedure TfmGlobalSearch.LoadPosition;
var
  n: Integer;
begin
  // if Utils_RestoreWindowPosition(Self.Handle,0,0,hppDBName,'GlobalSearchWindow.') <> 0 then begin
  // Self.Left := (Screen.Width-Self.Width) div 2;
  // Self.Top := (Screen.Height - Self.Height) div 2;
  // end;
  Utils_RestoreFormPosition(Self, 0, hppDBName, 'GlobalSearchWindow.');

  n := GetDBInt(hppDBName, 'GlobalSearchWindow.ContactListWidth', -1);
  if n <> -1 then
  begin
    paContacts.Width := n;
  end;
  spContacts.Left := paContacts.Left + paContacts.Width + 1;
  edFilter.Width := paFilter.Width - edFilter.Left - 2;

  SetRecentEventsPosition(GetDBInt(hppDBName, 'SortOrder', 0) <> 0);

  ToggleAdvancedPanel(GetDBBool(hppDBName, 'GlobalSearchWindow.ShowAdvanced', False));
  case GetDBInt(hppDBName, 'GlobalSearchWindow.AdvancedOptions', 0) of
    0:
      rbAny.Checked := True;
    1:
      rbAll.Checked := True;
    2:
      rbExact.Checked := True
  else
    rbAny.Checked := True;
  end;
  ToggleRangePanel(GetDBBool(hppDBName, 'GlobalSearchWindow.ShowRange', False));
  ToggleEventsPanel(GetDBBool(hppDBName, 'GlobalSearchWindow.ShowEvents', False));
  dtRange1.Date := Trunc(GetDBDateTime(hppDBName, 'GlobalSearchWindow.RangeFrom', Now));
  dtRange2.Date := Trunc(GetDBDateTime(hppDBName, 'GlobalSearchWindow.RangeTo', Now));
  edSearch.Text := GetDBWideStr(hppDBName, 'GlobalSearchWindow.LastSearch',
    DEFAULT_SEARCH_TEXT);
end;

procedure TfmGlobalSearch.LoadToolbarIcons;
var
  il: HIMAGELIST;
  ii: Integer;
begin
  ImageList_Remove(ilToolbar.Handle, -1); // clears image list
  il := ImageList_Create(16, 16, ILC_COLOR32 or ILC_MASK, 10, 2);
  if il <> 0 then
    ilToolbar.Handle := il
  else
    il := ilToolbar.Handle;
  ToolBar.Images := ilToolbar;

  ii := ImageList_AddIcon(il, hppIcons[HPP_ICON_SEARCHADVANCED].Handle);
  tbAdvanced.ImageIndex := ii;
  ii := ImageList_AddIcon(il, hppIcons[HPP_ICON_SEARCHRANGE].Handle);
  tbRange.ImageIndex := ii;
  ii := ImageList_AddIcon(il, hppIcons[HPP_ICON_HOTFILTER].Handle);
  tbEvents.ImageIndex := ii;
  ii := ImageList_AddIcon(il, hppIcons[HPP_ICON_BOOKMARK].Handle);
  tbBookmarks.ImageIndex := ii;

  ii := ImageList_AddIcon(il, hppIcons[HPP_ICON_HOTFILTER].Handle);
  tbFilter.ImageIndex := ii;
  ii := ImageList_AddIcon(il, hppIcons[HPP_ICON_HOTSEARCH].Handle);
  tbSearch.ImageIndex := ii;

  ii := ImageList_AddIcon(il, hppIcons[HPP_ICON_TOOL_COPY].Handle);
  tbCopy.ImageIndex := ii;
  ii := ImageList_AddIcon(il, hppIcons[HPP_ICON_TOOL_DELETE].Handle);
  tbDelete.ImageIndex := ii;
  ii := ImageList_AddIcon(il, hppIcons[HPP_ICON_TOOL_SAVE].Handle);
  tbSave.ImageIndex := ii;

  LoadEventFilterButton;
end;

procedure TfmGlobalSearch.lvContactsContextPopup(Sender: TObject; MousePos: TPoint;
  var Handled: Boolean);
var
  Item: TListItem;
  hContact: THandle;
begin
  Handled := True;
  Item := TListItem(lvContacts.GetItemAt(MousePos.X, MousePos.Y));
  if Item = nil then
    exit;
  hContact := THandle(Item.Data);
  if hContact = 0 then
    exit;
  UserMenu := CallService(MS_CLIST_MENUBUILDCONTACT, hContact, 0);
  if UserMenu <> 0 then
  begin
    UserMenuContact := hContact;
    MousePos := lvContacts.ClientToScreen(MousePos);
    Application.CancelHint;
    TrackPopupMenu(UserMenu, TPM_TOPALIGN or TPM_LEFTALIGN or TPM_LEFTBUTTON, MousePos.X,
      MousePos.Y, 0, Handle, nil);
    DestroyMenu(UserMenu);
    UserMenu := 0;
    // UserMenuContact := 0;
  end;
end;

procedure TfmGlobalSearch.lvContactsSelectItem(Sender: TObject; Item: TListItem;
  Selected: Boolean);
var
  hCont: THandle;
  // i,Index: Integer;
begin
  if not Selected then
    exit;

  { Index := -1;
    hCont := THANDLE(Item.Data);
    for i := 0 to Length(History) - 1 do
    if History[i].hContact = hCont then begin
    Index := i;
    break;
    end;
    if Index = -1 then exit;
    hg.Selected := Index; }

  // OXY: try to make selected item the topmost
  // while hg.GetFirstVisible <> Index do begin
  // if hg.VertScrollBar.Position = hg.VertScrollBar.Range then break;
  // hg.VertScrollBar.Position := hg.VertScrollBar.Position + 1;
  // end;

  if Item.Index = 0 then
    DisableFilter
  else
  begin
    hCont := THANDLE(Item.Data);
    FilterOnContact(hCont);
  end;
end;

procedure TfmGlobalSearch.OnCNChar(var Message: TWMChar);
// make tabs work!
begin
  if not(csDesigning in ComponentState) then
    with Message do
    begin
      Result := 1;
      if (Perform(WM_GETDLGCODE, 0, 0) and DLGC_WANTCHARS = 0) and
        (GetParentForm(Self).Perform(CM_DIALOGCHAR, CharCode, KeyData) <> 0) then
        exit;
      Result := 0;
    end;
end;

procedure TfmGlobalSearch.OrganizePanels;
var
  PrevPanel: TPanel;
begin
  PrevPanel := paSearch;
  if paAdvanced.Visible then
  begin
    paAdvanced.Top := PrevPanel.Top + PrevPanel.Width;
    PrevPanel := paAdvanced;
  end;
  if paRange.Visible then
  begin
    paRange.Top := PrevPanel.Top + PrevPanel.Width;
    PrevPanel := paRange;
  end;
  if paEvents.Visible then
  begin
    paEvents.Top := PrevPanel.Top + PrevPanel.Width;
  end;
end;

procedure TfmGlobalSearch.pbFilterPaint(Sender: TObject);
var
  ic: hIcon;
begin
  if tiFilter.Enabled then
    ic := hppIcons[HPP_ICON_HOTFILTERWAIT].Handle
  else
    ic := hppIcons[HPP_ICON_HOTFILTER].Handle;

  DrawiconEx(pbFilter.Canvas.Handle, 0, 0, ic, 16, 16, 0, pbFilter.Canvas.Brush.Handle,
    DI_NORMAL);
end;

procedure TfmGlobalSearch.pmEventsFilterPopup(Sender: TObject);
var
  i: Integer;
  pmi, mi: TMenuItem;
begin
  if Customize1.Parent <> pmEventsFilter.Items then
  begin
    pmi := TMenuItem(Customize1.Parent);
    for i := pmi.Count - 1 downto 0 do
    begin
      mi := TMenuItem(pmi.Items[i]);
      pmi.Remove(mi);
      pmEventsFilter.Items.Insert(0, mi);
    end;
  end;
  Application.CancelHint;
end;

procedure TfmGlobalSearch.ReplyQuoted(Item: Integer);
begin
  if (GetSearchItem(Item).Contact.Handle = 0) or (hg.SelCount = 0) then
    exit;
  SendMessageTo(GetSearchItem(Item).Contact.Handle,
    hg.FormatSelected(GridOptions.ReplyQuotedFormat));
end;

procedure TfmGlobalSearch.ReplyQuoted1Click(Sender: TObject);
begin
  if hg.Selected <> -1 then
  begin
    if GetSearchItem(hg.Selected).Contact.Handle = 0 then
      exit;
    ReplyQuoted(hg.Selected);
  end;
end;

procedure TfmGlobalSearch.tbSaveClick(Sender: TObject);
var
  t, t1: String;
  SaveFormat: TSaveFormat;
  RecentFormat: TSaveFormat;
begin
  if hg.Selected = -1 then
    exit;
  RecentFormat := TSaveFormat(GetDBInt(hppDBName, 'ExportFormat', 0));
  SaveFormat := RecentFormat;
  PrepareSaveDialog(SaveDialog, SaveFormat, True);
  t1 := TranslateW('Partial History [%s] - [%s]');
  t1 := Format(t1, [hg.ProfileName, hg.ContactName]);
  t := MakeFileName(t1);
  SaveDialog.FileName := t;
  if not SaveDialog.Execute then
    exit;
  for SaveFormat := High(SaveFormats) downto Low(SaveFormats) do
    if SaveDialog.FilterIndex = SaveFormats[SaveFormat].Index then
      break;
  if SaveFormat <> sfAll then
    RecentFormat := SaveFormat;
  hg.SaveSelected(SaveDialog.Files[0], RecentFormat);
  WriteDBInt(hppDBName, 'ExportFormat', Integer(RecentFormat));
end;

procedure TfmGlobalSearch.SavePosition;
begin
  Utils_SaveFormPosition(Self, 0, hppDBName, 'GlobalSearchWindow.');

  WriteDBInt(hppDBName, 'GlobalSearchWindow.ContactListWidth', paContacts.Width);

  WriteDBBool(hppDBName, 'GlobalSearchWindow.ShowAdvanced', paAdvanced.Visible);
  if rbAny.Checked then
    WriteDBInt(hppDBName, 'GlobalSearchWindow.AdvancedOptions', 0)
  else if rbAll.Checked then
    WriteDBInt(hppDBName, 'GlobalSearchWindow.AdvancedOptions', 1)
  else
    WriteDBInt(hppDBName, 'GlobalSearchWindow.AdvancedOptions', 2);

  WriteDBBool(hppDBName, 'GlobalSearchWindow.ShowRange', paRange.Visible);
  WriteDBBool(hppDBName, 'GlobalSearchWindow.ShowEvents', paEvents.Visible);

  if Trunc(dtRange1.Date) = Trunc(Now) then
    DBDelete(hppDBName, 'GlobalSearchWindow.RangeFrom')
  else
    WriteDBDateTime(hppDBName, 'GlobalSearchWindow.RangeFrom', Trunc(dtRange1.Date));
  if Trunc(dtRange2.Date) = Trunc(Now) then
    DBDelete(hppDBName, 'GlobalSearchWindow.RangeTo')
  else
    WriteDBDateTime(hppDBName, 'GlobalSearchWindow.RangeTo', Trunc(dtRange2.Date));

  WriteDBWideStr(hppDBName, 'GlobalSearchWindow.LastSearch', edSearch.Text);
end;

procedure TfmGlobalSearch.edSearchKeyPress(Sender: TObject; var Key: Char);
begin
  // to prevent ** BLING ** when press Enter
  // to prevent ** BLING ** when press Tab
  // to prevent ** BLING ** when press Esc
  if Ord(Key) in [VK_RETURN, VK_TAB, VK_ESCAPE] then
    Key := #0;
end;

procedure TfmGlobalSearch.edSearchKeyUp(Sender: TObject; var Key: Word; Shift: TShiftState);
begin
  if (Key = VK_RETURN) and bnSearch.Enabled then
    bnSearch.Click;
end;

procedure TfmGlobalSearch.EndHotFilterTimer;
begin
  tiFilter.Enabled := False;
  HotFilterString := edFilter.Text;
  hg.GridUpdate([guFilter]);
  if pbFilter.Tag <> 0 then
  begin
    pbFilter.Tag := 0;
    pbFilter.Repaint;
  end;
end;

procedure TfmGlobalSearch.EventsFilterItemClick(Sender: TObject);
begin
  SetEventFilter(TMenuItem(Sender).Tag);
end;

function TfmGlobalSearch.IsFileEvent(Index: Integer): Boolean;
begin
  Result := (Index <> -1) and (mtFile in hg.Items[Index].MessageType);
  if Result then
  begin
    SavedLinkUrl := ExtractFileName(String(hg.Items[Index].Extended));
    SavedFileDir := ExtractFileDir(String(hg.Items[Index].Extended));
  end;
end;

procedure TfmGlobalSearch.hgPopup(Sender: TObject);
begin
  Delete1.Visible := False;
  SaveSelected1.Visible := False;
  if hg.Selected <> -1 then
  begin
    Delete1.Visible := True;
    SaveSelected1.Visible := (hg.SelCount > 1);
    if GetSearchItem(hg.Selected).Contact.Handle = 0 then
    begin
      SendMessage1.Visible := False;
      ReplyQuoted1.Visible := False;
    end;
    if hg.Items[hg.Selected].Bookmarked then
      Bookmark1.Caption := TranslateW('Remove &Bookmark')
    else
      Bookmark1.Caption := TranslateW('Set &Bookmark');
    FileActions.Visible := IsFileEvent(hg.Selected);
    if FileActions.Visible then
      OpenFileFolder.Visible := (SavedFileDir <> '');
    pmGrid.Popup(Mouse.CursorPos.X, Mouse.CursorPos.Y);
  end;
end;

procedure TfmGlobalSearch.hgProcessRichText(Sender: TObject; Handle: Cardinal; Item: Integer);
var
  ItemRenderDetails: TItemRenderDetails;
begin
  ZeroMemory(@ItemRenderDetails, SizeOf(ItemRenderDetails));
  ItemRenderDetails.cbSize := SizeOf(ItemRenderDetails);
  ItemRenderDetails.hContact := GetSearchItem(Item).Contact.Handle;
  ItemRenderDetails.hDBEvent := GetSearchItem(Item).hDBEvent;
  ItemRenderDetails.pProto := PAnsiChar(hg.Items[Item].Proto);
  ItemRenderDetails.pModule := PAnsiChar(hg.Items[Item].Module);
  ItemRenderDetails.pText := nil;
  ItemRenderDetails.pExtended := PAnsiChar(hg.Items[Item].Extended);
  ItemRenderDetails.dwEventTime := hg.Items[Item].Time;
  ItemRenderDetails.wEventType := hg.Items[Item].EventType;
  ItemRenderDetails.IsEventSent := (mtOutgoing in hg.Items[Item].MessageType);

  if Handle = hg.InlineRichEdit.Handle then
    ItemRenderDetails.dwFlags := ItemRenderDetails.dwFlags or IRDF_INLINE;
  if hg.IsSelected(Item) then
    ItemRenderDetails.dwFlags := ItemRenderDetails.dwFlags or IRDF_SELECTED;

  ItemRenderDetails.bHistoryWindow := IRDHW_GLOBALSEARCH;

  NotifyEventHooks(hHppRichEditItemProcess, Handle, LPARAM(@ItemRenderDetails));
end;

procedure TfmGlobalSearch.hgTranslateTime(Sender: TObject; Time: Cardinal; var Text: String);
begin
  Text := TimestampToString(Time);
end;

procedure TfmGlobalSearch.HookEvents;
begin
  hHookContactIconChanged := HookEventMessage(ME_CLIST_CONTACTICONCHANGED,
    Self.Handle, HM_SRCH_CONTACTICONCHANGED);
end;

procedure TfmGlobalSearch.UnhookEvents;
begin
  UnhookEvent(hHookContactIconChanged);
end;

procedure TfmGlobalSearch.WndProc(var Message: TMessage);
begin
  case Message.Msg of
    WM_COMMAND:
      begin
        if mmAcc.DispatchCommand(Message.wParam) then
          exit;
        inherited;
        if Message.Result <> 0 then
          exit;
        Message.Result := CallService(MS_CLIST_MENUPROCESSCOMMAND,
          MAKEWPARAM(Message.WParamLo, MPCF_CONTACTMENU), UserMenuContact);
        exit;
      end;
    WM_MEASUREITEM:
      if Self.UserMenu <> 0 then
      begin
        Message.Result := CallService(MS_CLIST_MENUMEASUREITEM, Message.wParam,
          Message.LParam);
        if Message.Result <> 0 then
          exit;
      end;
    WM_DRAWITEM:
      if Self.UserMenu <> 0 then
      begin
        Message.Result := CallService(MS_CLIST_MENUDRAWITEM, Message.wParam,
          Message.LParam);
        if Message.Result <> 0 then
          exit;
      end;
  end;
  inherited;
end;

procedure TfmGlobalSearch.FormShow(Sender: TObject);
begin
  paFilter.Visible := False;
  ToggleAdvancedPanel(False);
  ShowContacts(False);

  IsSearching := False;
  SearchThread := nil;

  hg.Codepage := hppCodepage;
  hg.RTLMode := hppRTLDefault;
  hg.TxtStartUp := TranslateW('Ready to search') + #10#13#10#13 +
    TranslateW('Click Search button to start');
  hg.Allocate(0);

  LoadPosition;

  HookEvents;

  edSearch.SetFocus;
  edSearch.SelectAll;
  edSearchChange(Self);

  CreateEventsFilterMenu;
  // SetEventFilter(0);
  SetEventFilter(GetShowAllEventsIndex);
end;

function TfmGlobalSearch.GetSearchItem(GridIndex: Integer): TSearchItem;
begin
  if not FFiltered then
    Result := History[GridIndex]
  else
    Result := History[FilterHistory[GridIndex]];
end;

function TfmGlobalSearch.GetContactInfo(hContact: THandle): THPPContactInfo;
var
  i: Integer;
begin
  Result := nil;
  for i := 0 to ContactList.Count - 1 do
    if hContact = THPPContactInfo(ContactList.Items[i]).Handle then
    begin
      Result := THPPContactInfo(ContactList.Items[i]);
      break;
    end;
end;

procedure TfmGlobalSearch.HMContactDeleted(var M: TMessage);
// var
// ci: THPPContactInfo;
// i: Integer;
begin
  { wParam - hContact; lParam - zero }
  // do here something because the contact is deleted
  if IsSearching then
    exit;
  // need to remove contact
  // ci := GetContactInfo(M.WParam);
  // if ci = nil then exit;
  // for i := 1 to lvContacts.Items.Count - 1 do
  // if ci.Handle = THandle(lvContacts.Items[i].Data) then begin
  // lvContacts.Items.Delete(i);
  // break;
  // end;
  // ContactList.Remove(ci);
end;

procedure TfmGlobalSearch.HMNickChanged(var M: TMessage);
var
  ci: THPPContactInfo;
  i: Integer;
  SubContact: TMCONTACT;
  SubProtocol: AnsiString;
begin
  { wParam - hContact; lParam - zero }
  if IsSearching then
    exit;
  ci := GetContactInfo(M.wParam);
  if ci = nil then
    exit;
  GetContactProto(CurContact, SubContact, SubProtocol);
  ci.ProfileName := GetContactDisplayName(0, SubProtocol);
  ci.Name := GetContactDisplayName(ci.Handle, ci.Proto, True);
  for i := 1 to lvContacts.Items.Count - 1 do
    if M.wParam = THandle(lvContacts.Items[i].Data) then
    begin
      lvContacts.Items[i].Caption := ci.Name;
      break;
    end;
  hg.Invalidate;
end;

procedure TfmGlobalSearch.HMContactIconChanged(var M: TMessage);
var
  i: Integer;
begin
  { wParam - hContact; lParam - IconID }
  // contact icon has changed
  // meTest.Lines.Add(GetContactDisplayName(M.wParam)+' changed icon to '+IntToStr(m.LParam));
  if not paContacts.Visible then
    exit;
  for i := 0 to lvContacts.Items.Count - 1 do
  begin
    if THandle(M.wParam) = THandle(lvContacts.Items[i].Data) then
    begin
      lvContacts.Items[i].ImageIndex := Integer(M.LParam);
      break;
    end;
  end;
end;

procedure TfmGlobalSearch.HMEventDeleted(var M: TMessage);
var
  i: Integer;
begin
  { wParam - hContact; lParam - hDBEvent }
  if hg.State = gsDelete then
    exit;
  // if WPARAM(message.wParam) <> hContact then exit;
  for i := 0 to hg.Count - 1 do
    if GetSearchItem(i).hDBEvent = THandle(M.LParam) then
    begin
      hg.Delete(i);
      hgState(hg, hg.State);
      exit;
    end;
  // if event is not in filter, we must search the overall array
  if FFiltered then
  begin
    i := FindHistoryItemByHandle(M.LParam);
    if i <> -1 then
      DeleteEventFromLists(i);
  end;
end;

procedure TfmGlobalSearch.HMFiltersChanged(var M: TMessage);
begin
  CreateEventsFilterMenu;
  SetEventFilter(0);
end;

procedure TfmGlobalSearch.HMIcons2Changed(var M: TMessage);
begin
  Icon.Handle := CopyIcon(hppIcons[HPP_ICON_GLOBALSEARCH].Handle);
  LoadToolbarIcons;
  LoadButtonIcons;
  LoadContactsIcons;
  pbFilter.Repaint;
  // hg.Repaint;
end;

procedure TfmGlobalSearch.mmHideMenuClick(Sender: TObject);
begin
  WriteDBBool(hppDBName, 'Accessability', False);
  NotifyAllForms(HM_NOTF_ACCCHANGED, WPARAM(False), 0);
end;

procedure TfmGlobalSearch.HMAccChanged(var M: TMessage);
begin
  ToggleMainMenu(Boolean(M.wParam));
end;

procedure TfmGlobalSearch.HMBookmarksChanged(var M: TMessage);
var
  i: Integer;
  Found: Boolean;
begin
  Found := False;
  for i := 0 to hg.Count - 1 do
    if GetSearchItem(i).hDBEvent = THandle(M.LParam) then
    begin
      hg.ResetItem(i);
      Found := True;
      break;
    end;
  if Found then
    hg.Repaint;
end;

procedure TfmGlobalSearch.HMPreShutdown(var M: TMessage);
begin
  Close;
end;

procedure TfmGlobalSearch.hgKeyDown(Sender: TObject; var Key: Word; Shift: TShiftState);
var
  pm: TPopupMenu;
begin
  if hg.State = gsInline then
    pm := pmInline
  else
    pm := pmGrid;

  if IsFormShortCut([pm], Key, Shift) then
  begin
    Key := 0;
    exit;
  end;

  WasReturnPressed := (Key = VK_RETURN);
end;

procedure TfmGlobalSearch.hgKeyUp(Sender: TObject; var Key: Word; Shift: TShiftState);
begin
  if not WasReturnPressed then
    exit;

  if (Key = VK_RETURN) and (Shift = []) then
  begin
    if hg.Selected <> -1 then
      hg.EditInline(hg.Selected);
  end;
  if (Key = VK_RETURN) and (Shift = [ssCtrl]) then
  begin
    hgDblClick(hg);
  end;
end;

procedure TfmGlobalSearch.hgNameData(Sender: TObject; Index: Integer; var Name: String);
var
  si: TSearchItem;
begin
  si := GetSearchItem(Index);
  if FFiltered then
  begin
    if mtIncoming in hg.Items[Index].MessageType then
      Name := si.Contact.Name
    else if not GridOptions.ForceProfileName then
      Name := si.Contact.ProfileName;
  end
  else
  begin
    if mtIncoming in hg.Items[Index].MessageType then
      Name := WideFormat(TranslateW('From %s'), [si.Contact.Name])
    else
      Name := WideFormat(TranslateW('To %s'), [si.Contact.Name]);
  end;
  // there should be anoter way to use bookmarks names
  // if IsBookmarksMode then
  // Name := Name + ' [' + BookmarkServer[si.Contact.Handle].BookmarkName[si.hDBEvent] + ']';
end;

procedure TfmGlobalSearch.hgUrlClick(Sender: TObject; Item: Integer; const URLText: String;
  Button: TMouseButton);
begin
  if URLText = '' then
    exit;
  if (Button = mbLeft) or (Button = mbMiddle) then
    OpenUrl(URLText, True)
  else
  begin
    SavedLinkUrl := URLText;
    pmLink.Popup(Mouse.CursorPos.X, Mouse.CursorPos.Y);
  end;
end;

procedure TfmGlobalSearch.edPassKeyPress(Sender: TObject; var Key: Char);
begin
  // to prevent ** BLING ** when press Enter
  // to prevent ** BLING ** when press Tab
  // to prevent ** BLING ** when press Esc
  if Ord(Key) in [VK_RETURN, VK_TAB, VK_ESCAPE] then
    Key := #0;
end;

procedure TfmGlobalSearch.edPassKeyUp(Sender: TObject; var Key: Word; Shift: TShiftState);
begin
  if Key = VK_RETURN then
  begin
    bnSearch.Click;
    Key := 0;
  end;
end;

procedure TfmGlobalSearch.ShowContacts(Show: Boolean);
begin
  paContacts.Visible := Show;
  spContacts.Visible := Show;
  if (Show) and (paContacts.Width > 0) then
    spContacts.Left := paContacts.Width + paContacts.Left + 1;
end;

procedure TfmGlobalSearch.SearchNext(Rev: Boolean; Warp: Boolean = True);
var
  stext: String;
  res: Integer;
  mcase, Down: Boolean;
  WndHandle: HWND;
begin
  stext := HotString;
  mcase := False;
  if stext = '' then
    exit;
  Down := not hg.Reversed;
  if Rev then
    Down := not Down;
  res := hg.Search(stext, mcase, not Warp, False, Warp, Down);
  if res <> -1 then
  begin
    // found
    hg.Selected := res;
    sb.SimpleText := Format(TranslateW('HotSearch: %s (F3 to find next)'), [stext]);
  end
  else
  begin
    WndHandle := Handle;
    // not found
    if Warp and (Down = not hg.Reversed) then
    begin
      // do warp?
      if HppMessageBox(WndHandle, TranslateW('You have reached the end of the history.') +
        #10#13 + TranslateW('Do you want to continue searching at the beginning?'),
        TranslateW('History++ Search'), MB_YESNOCANCEL or MB_DEFBUTTON1 or MB_ICONQUESTION) = ID_YES
      then
        SearchNext(Rev, False);
    end
    else
    begin
      // not warped
      hgState(Self, gsIdle);
      HppMessageBox(WndHandle, WideFormat('"%s" not found', [stext]),
        TranslateW('History++ Search'), MB_OK or MB_DEFBUTTON1 or 0);
    end;
  end;
end;

procedure TfmGlobalSearch.SendMessage1Click(Sender: TObject);
begin
  if hg.Selected <> -1 then
  begin
    if GetSearchItem(hg.Selected).Contact.Handle = 0 then
      exit;
    SendMessageTo(GetSearchItem(hg.Selected).Contact.Handle);
  end;
end;

procedure TfmGlobalSearch.SetEventFilter(FilterIndex: Integer);
var
  i, fi: Integer;
  mi: TMenuItem;
begin
  if FilterIndex = -1 then
  begin
    fi := tbEventsFilter.Tag + 1;
    if fi > High(hppEventFilters) then
      fi := 0;
  end
  else
    fi := FilterIndex;

  tbEventsFilter.Tag := fi;
  LoadEventFilterButton;
  // tbEventsFilter.Repaint;
  mi := TMenuItem(Customize1.Parent);
  for i := 0 to mi.Count - 1 do
    if mi[i].RadioItem then
      mi[i].Checked := (mi[i].Tag = fi);

  hg.Filter := hppEventFilters[fi].Events;
end;

procedure TfmGlobalSearch.SetRecentEventsPosition(OnTop: Boolean);
begin
  hg.Reversed := not OnTop;
end;

procedure TfmGlobalSearch.FormKeyDown(Sender: TObject; var Key: Word; Shift: TShiftState);
var
  Mask: Integer;
begin
  if (Key = VK_ESCAPE) or ((Key = VK_F4) and (ssAlt in Shift)) then
  begin
    if (Key = VK_ESCAPE) and IsSearching then
      StopSearching
    else
      Close;
    Key := 0;
    exit;
  end;

  if (Key = VK_F10) and (Shift = []) then
  begin
    WriteDBBool(hppDBName, 'Accessability', True);
    NotifyAllForms(HM_NOTF_ACCCHANGED, WPARAM(True), 0);
    Key := 0;
    exit;
  end;

  if (Key = VK_F3) and ((Shift = []) or (Shift = [ssShift])) and (Length(History) > 0) then
  begin
    SearchNext(ssShift in Shift, True);
    Key := 0;
  end;

  if hg.State = gsInline then
    exit;

  if IsFormShortCut([mmAcc], Key, Shift) then
  begin
    Key := 0;
    exit;
  end;

  with Sender as TWinControl do
  begin
    if Perform(CM_CHILDKEY, Key, LParam(Sender)) <> 0 then
      exit;
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
      exit;
  end;
end;

procedure TfmGlobalSearch.hgSearchFinished(Sender: TObject; const Text: String; Found: Boolean);
var
  t: String;
begin
  if Text = '' then
  begin
    HotString := Text;
    hgState(Self, gsIdle);
    exit;
  end;
  HotString := Text;

  if not Found then
    t := HotString
  else
    t := Text;
  sb.SimpleText := Format(TranslateW('HotSearch: %s (F3 to find next)'), [t]);
end;

procedure TfmGlobalSearch.hgSearchItem(Sender: TObject; Item, ID: Integer; var Found: Boolean);
begin
  Found := (THandle(ID) = GetSearchItem(Item).hDBEvent);
end;

procedure TfmGlobalSearch.hgSelect(Sender: TObject; Item, OldItem: Integer);
{ var
  i,hCont,Index: Integer; }
begin
  tbCopy.Enabled := (Item <> -1);
  tbDelete.Enabled := (Item <> -1);
  tbSave.Enabled := (hg.SelCount > 1);

  if hg.HotString = '' then
  begin
    // redraw status bar
    hgState(hg, gsIdle);
  end;

  { if Item = -1 then exit;
    index := -1;
    hCont := History[Item].hContact;
    for i := 0 to lvContacts.Items.Count-1 do
    if THandle(lvContacts.Items.Item[i].Data) = hCont then begin
    Index := i;
    break;
    end;
    if Index = -1 then exit;
    lvContacts.OnSelectItem := nil;
    lvContacts.Items.Item[index].MakeVisible(false);
    lvContacts.Items.Item[index].Selected := true;
    lvContacts.OnSelectItem := self.lvContactsSelectItem; }
end;

procedure TfmGlobalSearch.hgState(Sender: TObject; State: TGridState);
var
  t: String;
begin
  if csDestroying in ComponentState then
    exit;
  if IsSearching then
    t := Format(TranslateW('Searching... %.0n items in %d contacts found'),
      [Length(History) / 1, ContactsFound])
  else
    case State of
      gsIdle:
        t := Format
          (TranslateW('%.0n items in %d contacts found. Searched for %.1f sec in %.0n items.'),
          [Length(History) / 1, ContactsFound, stime / 1000, AllItems / 1]);
      gsLoad:
        t := TranslateW('Loading...');
      gsSave:
        t := TranslateW('Saving...');
      gsSearch:
        t := TranslateW('Searching...');
      gsDelete:
        t := TranslateW('Deleting...');
    end;
  sb.SimpleText := t;
end;

procedure TfmGlobalSearch.tbCopyClick(Sender: TObject);
begin
  if hg.Selected = -1 then
    exit;
  CopyToClip(hg.FormatSelected(GridOptions.ClipCopyFormat), Handle,
    GetSearchItem(hg.Selected).Contact.Codepage);
end;

procedure TfmGlobalSearch.CopyText1Click(Sender: TObject);
begin
  if hg.Selected = -1 then
    exit;
  CopyToClip(hg.FormatSelected(GridOptions.ClipCopyTextFormat), Handle,
    GetSearchItem(hg.Selected).Contact.Codepage);
end;

procedure TfmGlobalSearch.CreateEventsFilterMenu;
var
  i: Integer;
  mi: TMenuItem;
  ShowAllEventsIndex: Integer;
begin
  for i := pmEventsFilter.Items.Count - 1 downto 0 do
    if pmEventsFilter.Items[i].RadioItem then
      pmEventsFilter.Items.Delete(i);
  cbEvents.Items.Clear;

  ShowAllEventsIndex := GetShowAllEventsIndex;
  for i := 0 to High(hppEventFilters) do
  begin
    mi := TMenuItem.Create(pmEventsFilter);
    mi.Caption := StringReplace(hppEventFilters[i].Name, '&', '&&', [rfReplaceAll]);
    mi.GroupIndex := 1;
    mi.RadioItem := True;
    mi.Tag := i;
    mi.OnClick := EventsFilterItemClick;
    if i = ShowAllEventsIndex then
      mi.Default := True;
    pmEventsFilter.Items.Insert(i, mi);
    cbEvents.Items.Insert(i, mi.Caption);
  end;

  cbEvents.DropDownCount := Length(hppEventFilters);
  cbEvents.ItemIndex := ShowAllEventsIndex;
end;

procedure TfmGlobalSearch.Customize1Click(Sender: TObject);
begin
  if not Assigned(fmCustomizeFilters) then
  begin
    CustomizeFiltersForm := TfmCustomizeFilters.Create(Self);
    CustomizeFiltersForm.Show;
  end
  else
  begin
    BringFormToFront(fmCustomizeFilters);
  end;
end;

procedure TfmGlobalSearch.tbDeleteClick(Sender: TObject);
begin
  if hg.SelCount = 0 then
    exit;
  if hg.SelCount > 1 then
  begin
    if HppMessageBox(Handle,
      WideFormat(TranslateW('Do you really want to delete selected items (%.0f)?'),
      [hg.SelCount / 1]), TranslateW('Delete Selected'), MB_YESNOCANCEL or MB_DEFBUTTON1 or
      MB_ICONQUESTION) <> IDYES then
      exit;
  end
  else
  begin
    if HppMessageBox(Handle, TranslateW('Do you really want to delete selected item?'),
      TranslateW('Delete'), MB_YESNOCANCEL or MB_DEFBUTTON1 or MB_ICONQUESTION) <> IDYES then
      exit;
  end;
  SetSafetyMode(False);
  try
    FormState := gsDelete;
    hg.DeleteSelected;
    FormState := gsIdle;
  finally
    SetSafetyMode(True);
  end;
end;

procedure TfmGlobalSearch.hgRTLEnabled(Sender: TObject; BiDiMode: TBiDiMode);
begin
  edSearch.BiDiMode := BiDiMode;
  edFilter.BiDiMode := BiDiMode;
  dtRange1.BiDiMode := BiDiMode;
  dtRange2.BiDiMode := BiDiMode;
  // lvContacts.BiDiMode := BiDiMode;
end;

procedure TfmGlobalSearch.Bookmark1Click(Sender: TObject);
var
  val: Boolean;
  hDBEvent: THandle;
begin
  hDBEvent := GetSearchItem(hg.Selected).hDBEvent;
  val := not BookmarkServer[GetSearchItem(hg.Selected).Contact.Handle].Bookmarked[hDBEvent];
  BookmarkServer[GetSearchItem(hg.Selected).Contact.Handle].Bookmarked[hDBEvent] := val;
end;

procedure TfmGlobalSearch.hgInlinePopup(Sender: TObject);
begin
  InlineCopy.Enabled := hg.InlineRichEdit.SelLength > 0;
  InlineReplyQuoted.Enabled := InlineCopy.Enabled;
  InlineTextFormatting.Checked := GridOptions.TextFormatting;
  if hg.Selected <> -1 then
  begin
    InlineSendMessage.Visible := (GetSearchItem(hg.Selected).Contact.Handle <> 0);
    InlineReplyQuoted.Visible := (GetSearchItem(hg.Selected).Contact.Handle <> 0);
  end;
  pmInline.Popup(Mouse.CursorPos.X, Mouse.CursorPos.Y);
end;

procedure TfmGlobalSearch.InlineCopyClick(Sender: TObject);
begin
  if hg.InlineRichEdit.SelLength = 0 then
    exit;
  hg.InlineRichEdit.CopyToClipboard;
end;

procedure TfmGlobalSearch.InlineCopyAllClick(Sender: TObject);
var
  cr: TCharRange;
begin
  hg.InlineRichEdit.Lines.BeginUpdate;
  hg.InlineRichEdit.Perform(EM_EXGETSEL, 0, LParam(@cr));
  hg.InlineRichEdit.SelectAll;
  hg.InlineRichEdit.CopyToClipboard;
  hg.InlineRichEdit.Perform(EM_EXSETSEL, 0, LParam(@cr));
  hg.InlineRichEdit.Lines.EndUpdate;
end;

procedure TfmGlobalSearch.InlineSelectAllClick(Sender: TObject);
begin
  hg.InlineRichEdit.SelectAll;
end;

procedure TfmGlobalSearch.InlineTextFormattingClick(Sender: TObject);
begin
  GridOptions.TextFormatting := not GridOptions.TextFormatting;
end;

procedure TfmGlobalSearch.InlineReplyQuotedClick(Sender: TObject);
begin
  if hg.Selected <> -1 then
  begin
    if GetSearchItem(hg.Selected).Contact.Handle = 0 then
      exit;
    if hg.InlineRichEdit.SelLength = 0 then
      exit;
    SendMessageTo(GetSearchItem(hg.Selected).Contact.Handle,
      hg.FormatSelected(GridOptions.ReplyQuotedTextFormat));
  end;
end;

procedure TfmGlobalSearch.hgInlineKeyDown(Sender: TObject; var Key: Word; Shift: TShiftState);
begin
  if IsFormShortCut([mmAcc, pmInline], Key, Shift) then
  begin
    Key := 0;
    exit;
  end;
end;

procedure TfmGlobalSearch.OpenLinkClick(Sender: TObject);
begin
  if SavedLinkUrl = '' then
    exit;
  OpenUrl(SavedLinkUrl, False);
  SavedLinkUrl := '';
end;

procedure TfmGlobalSearch.OpenLinkNWClick(Sender: TObject);
begin
  if SavedLinkUrl = '' then
    exit;
  OpenUrl(SavedLinkUrl, True);
  SavedLinkUrl := '';
end;

procedure TfmGlobalSearch.CopyLinkClick(Sender: TObject);
begin
  if SavedLinkUrl = '' then
    exit;
  CopyToClip(SavedLinkUrl, Handle, CP_ACP);
  SavedLinkUrl := '';
end;

procedure TfmGlobalSearch.ToggleMainMenu(Enabled: Boolean);
begin
  if Enabled then
  begin
    ToolBar.EdgeBorders := [ebTop];
    Menu := mmAcc
  end
  else
  begin
    ToolBar.EdgeBorders := [];
    Menu := nil;
  end;
end;

procedure TfmGlobalSearch.WMSysColorChange(var Message: TMessage);
begin
  inherited;
  LoadToolbarIcons;
  LoadButtonIcons;
  Repaint;
end;

procedure TfmGlobalSearch.tbBookmarksClick(Sender: TObject);
begin
  if Sender <> tbBookmarks then
    tbBookmarks.Down := not tbBookmarks.Down;
  IsBookmarksMode := tbBookmarks.Down;

  if IsSearching then
    StopSearching;

  paSearch.Visible := not IsBookmarksMode;
  tbAdvanced.Enabled := not IsBookmarksMode;
  ToggleAdvancedPanel(tbAdvanced.Down);
  tbRange.Enabled := not IsBookmarksMode;
  ToggleRangePanel(tbRange.Down);
  tbEvents.Enabled := not IsBookmarksMode;
  ToggleEventsPanel(tbEvents.Down);

  if IsBookmarksMode then
    bnSearch.Click
  else if edSearch.CanFocus then
    edSearch.SetFocus;
end;

procedure TfmGlobalSearch.SelectAll1Click(Sender: TObject);
begin
  hg.SelectAll;
end;

procedure TfmGlobalSearch.OpenFileFolderClick(Sender: TObject);
begin
  if SavedFileDir = '' then
    exit;
  ShellExecuteW(0, 'open', PWideChar(SavedFileDir), nil, nil, SW_SHOW);
  SavedFileDir := '';
end;

procedure TfmGlobalSearch.BrowseReceivedFilesClick(Sender: TObject);
var
  Path: Array [0 .. MAX_PATH] of AnsiChar;
  hContact: THandle;
begin
  hContact := GetSearchItem(hg.Selected).Contact.Handle;
  CallService(MS_FILE_GETRECEIVEDFILESFOLDER, hContact, LParam(@Path));
  ShellExecuteA(0, 'open', Path, nil, nil, SW_SHOW);
end;

procedure TfmGlobalSearch.tbEventsClick(Sender: TObject);
begin
  if Sender <> tbEvents then
    tbEvents.Down := not tbEvents.Down;
  ToggleEventsPanel(tbEvents.Down);
end;

procedure TfmGlobalSearch.sbEventsCloseClick(Sender: TObject);
begin
  ToggleEventsPanel(False);
end;

procedure TfmGlobalSearch.lvContactsDblClick(Sender: TObject);
var
  hContact: THandle;
begin
  if lvContacts.Selected = nil then
    exit;
  hContact := THandle(lvContacts.Selected.Data);
  if hContact = 0 then
    exit;
  SendMessageTo(hContact);
end;

procedure TfmGlobalSearch.hgChar(Sender: TObject; var achar: WideChar; Shift: TShiftState);
var
  Mes: TWMChar;
begin
  edFilter.SetFocus;
  edFilter.SelStart := Length(edFilter.Text);
  edFilter.SelLength := 0;
  // edFilter.Text := AnsiChar;
  ZeroMemory(@Mes, SizeOf(Mes));
  Mes.Msg := WM_CHAR;
  Mes.CharCode := Word(achar);
  Mes.KeyData := ShiftStateToKeyData(Shift);
  edFilter.Perform(WM_CHAR, TMessage(Mes).wParam, TMessage(Mes).LParam);
  achar := #0;
end;

procedure TfmGlobalSearch.edFilterKeyPress(Sender: TObject; var Key: Char);
begin
  // to prevent ** BLING ** when press Enter
  // to prevent ** BLING ** when press Tab
  // to prevent ** BLING ** when press Esc
  if Ord(Key) in [VK_RETURN, VK_TAB, VK_ESCAPE] then
    Key := #0;
end;

initialization

  fmGlobalSearch := nil;

end.
