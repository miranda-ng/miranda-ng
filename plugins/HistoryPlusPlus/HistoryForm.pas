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
  HistoryForm (historypp project)

  Version:   1.4
  Created:   xx.03.2003
  Author:    Oxygen

  [ Description ]

  Main window with history listing

  [ History ]

  1.4
  - Fixed bug on closing history window with FindDialog opened

  1.3 ()
  + Added XML export
  + URL & File highlight handling
  * "Reply Quoted" now is "Forward Message", and it forwards now,
  instead of sending
  - Fixed possible bug when opening hist. window and deleting contact
  now hist. window closes on contact deletion.
  1.2
  1.1
  1.0 (xx.02.03) First version.

  [ Modifications ]
  * (29.05.2003) Added FindDialog.CloseDialog to Form.OnClose so now
  closing history window without closing find dialog don't throws
  exception

  [ Known Issues ]

  * Not very good support of EmailExpress events (togeter
  with HistoryGrid.pas)

  Contributors: theMIROn, Art Fedorov
  ----------------------------------------------------------------------------- }

unit HistoryForm;

interface

uses
  Windows, Messages, SysUtils, Classes, RichEdit,
  Graphics, Controls, Forms, Dialogs, Buttons, StdCtrls, Menus, ComCtrls, ExtCtrls,
  m_api,
  hpp_global, hpp_sessionsthread,
  HistoryGrid, DateUtils,
  ImgList, HistoryControls, CommCtrl, ToolWin, ShellAPI, Themes;

type

  TLastSearch = (lsNone, lsHotSearch, lsSearch);
  TSearchMode = (smNone, smSearch, smFilter, smHotSearch);
  // smHotSearch for possible future use
  THistoryPanel = (hpSessions, hpBookmarks);
  THistoryPanels = set of THistoryPanel;

  THistoryFrm = class(TForm)
    SaveDialog: TSaveDialog;
    pmGrid: TPopupMenu;
    paClient: TPanel;
    paGrid: TPanel;
    hg: THistoryGrid;
    sb: TStatusBar;
    pmLink: TPopupMenu;
    paSess: TPanel;
    spHolder: TSplitter;
    ilSessions: TImageList;
    paSessInt: TPanel;
    laSess: TLabel;
    sbCloseSess: TSpeedButton;
    N13: TMenuItem;
    SaveSelected1: TMenuItem;
    N2: TMenuItem;
    Delete1: TMenuItem;
    CopyText1: TMenuItem;
    Copy1: TMenuItem;
    N12: TMenuItem;
    ReplyQuoted1: TMenuItem;
    SendMessage1: TMenuItem;
    N8: TMenuItem;
    Details1: TMenuItem;
    CopyLink: TMenuItem;
    N1: TMenuItem;
    OpenLinkNW: TMenuItem;
    OpenLink: TMenuItem;
    ContactRTLmode: TMenuItem;
    ANSICodepage: TMenuItem;
    RTLDisabled2: TMenuItem;
    RTLEnabled2: TMenuItem;
    RTLDefault2: TMenuItem;
    SystemCodepage: TMenuItem;
    sbClearFilter: TSpeedButton;
    pbFilter: TPaintBox;
    tiFilter: TTimer;
    ilToolbar: TImageList;
    Toolbar: THppToolBar;
    pmHistory: TPopupMenu;
    SaveasMContacts2: TMenuItem;
    SaveasRTF2: TMenuItem;
    SaveasXML2: TMenuItem;
    SaveasHTML2: TMenuItem;
    SaveasText2: TMenuItem;
    tbSearch: THppToolButton;
    ToolButton3: THppToolButton;
    paSearch: TPanel;
    tbFilter: THppToolButton;
    tbDelete: THppToolButton;
    tbSessions: THppToolButton;
    ToolButton2: THppToolButton;
    paSearchStatus: TPanel;
    laSearchState: TLabel;
    paSearchPanel: TPanel;
    sbSearchNext: TSpeedButton;
    sbSearchPrev: TSpeedButton;
    edSearch: THppEdit;
    pbSearch: TPaintBox;
    tvSess: TTreeView;
    tbSave: THppToolButton;
    tbCopy: THppToolButton;
    tbHistorySearch: THppToolButton;
    imSearchEndOfPage: TImage;
    imSearchNotFound: TImage;
    ToolButton4: THppToolButton;
    N4: TMenuItem;
    EmptyHistory1: TMenuItem;
    pmEventsFilter: TPopupMenu;
    ShowAll1: TMenuItem;
    Customize1: TMenuItem;
    N6: TMenuItem;
    TopPanel: TPanel;
    paSearchButtons: TPanel;
    pmSessions: TPopupMenu;
    SessCopy: TMenuItem;
    SessSelect: TMenuItem;
    SessDelete: TMenuItem;
    N7: TMenuItem;
    SessSave: TMenuItem;
    tbUserMenu: THppToolButton;
    tbUserDetails: THppToolButton;
    ToolButton1: THppToolButton;
    tbEventsFilter: THppSpeedButton;
    ToolButton5: THppToolButton;
    pmToolbar: TPopupMenu;
    Customize2: TMenuItem;
    Bookmark1: TMenuItem;
    paBook: TPanel;
    paBookInt: TPanel;
    laBook: TLabel;
    sbCloseBook: TSpeedButton;
    lvBook: TListView;
    ilBook: TImageList;
    tbBookmarks: THppToolButton;
    pmBook: TPopupMenu;
    DeleteBookmark1: TMenuItem;
    N3: TMenuItem;
    SaveSelected2: TMenuItem;
    N11: TMenuItem;
    RenameBookmark1: TMenuItem;
    pmInline: TPopupMenu;
    InlineReplyQuoted: TMenuItem;
    MenuItem6: TMenuItem;
    InlineCopy: TMenuItem;
    InlineCopyAll: TMenuItem;
    MenuItem10: TMenuItem;
    InlineSelectAll: TMenuItem;
    InlineTextFormatting: TMenuItem;
    InlineSendMessage: TMenuItem;
    N5: TMenuItem;
    mmAcc: TMainMenu;
    mmToolbar: TMenuItem;
    mmService: TMenuItem;
    mmHideMenu: TMenuItem;
    mmShortcuts: TMenuItem;
    mmBookmark: TMenuItem;
    SelectAll1: TMenuItem;
    tbHistory: THppToolButton;
    paHolder: TPanel;
    spBook: TSplitter;
    UnknownCodepage: TMenuItem;
    OpenFileFolder: TMenuItem;
    BrowseReceivedFiles: TMenuItem;
    N9: TMenuItem;
    CopyFilename: TMenuItem;
    FileActions: TMenuItem;
    N10: TMenuItem;
    pmFile: TPopupMenu;
    procedure tbHistoryClick(Sender: TObject);
    procedure SaveasText2Click(Sender: TObject);
    procedure SaveasMContacts2Click(Sender: TObject);
    procedure SaveasRTF2Click(Sender: TObject);
    procedure SaveasXML2Click(Sender: TObject);
    procedure SaveasHTML2Click(Sender: TObject);
    procedure tbSessionsClick(Sender: TObject);
    procedure pbSearchStatePaint(Sender: TObject);
    procedure tbDeleteClick(Sender: TObject);
    procedure sbSearchPrevClick(Sender: TObject);
    procedure sbSearchNextClick(Sender: TObject);
    procedure edSearchChange(Sender: TObject);
    procedure hgChar(Sender: TObject; var achar: WideChar; Shift: TShiftState);
    procedure edSearchKeyDown(Sender: TObject; var Key: Word; Shift: TShiftState);
    procedure edSearchKeyUp(Sender: TObject; var Key: Word; Shift: TShiftState);
    procedure tbSearchClick(Sender: TObject);
    procedure tbFilterClick(Sender: TObject);
    procedure pbSearchPaint(Sender: TObject);
    procedure tvSessMouseMove(Sender: TObject; Shift: TShiftState; X, Y: Integer);
    // procedure tvSessClick(Sender: TObject);
    procedure sbCloseSessClick(Sender: TObject);
    procedure hgItemFilter(Sender: TObject; Index: Integer; var Show: Boolean);
    procedure tvSessChange(Sender: TObject; Node: TTreeNode);
    // procedure bnConversationClick(Sender: TObject);

    procedure LoadHistory(Sender: TObject);
    procedure OnCNChar(var Message: TWMChar); message WM_CHAR;

    procedure FormCreate(Sender: TObject);
    procedure FormShow(Sender: TObject);
    procedure FormKeyDown(Sender: TObject; var Key: Word; Shift: TShiftState);
    procedure FormMouseWheel(Sender: TObject; Shift: TShiftState; WheelDelta: Integer;
      MousePos: TPoint; var Handled: Boolean);
    procedure FormClose(Sender: TObject; var Action: TCloseAction);
    procedure FormCloseQuery(Sender: TObject; var CanClose: Boolean);
    procedure FormDestroy(Sender: TObject);

    procedure hgItemData(Sender: TObject; Index: Integer; var Item: THistoryItem);
    procedure hgTranslateTime(Sender: TObject; Time: Cardinal; var Text: String);
    procedure hgPopup(Sender: TObject);

    procedure hgSearchFinished(Sender: TObject; const Text: String; Found: Boolean);
    procedure hgDblClick(Sender: TObject);
    procedure tbSaveClick(Sender: TObject);
    procedure hgItemDelete(Sender: TObject; Index: Integer);
    procedure tbCopyClick(Sender: TObject);
    procedure Details1Click(Sender: TObject);
    procedure hgKeyDown(Sender: TObject; var Key: Word; Shift: TShiftState);
    procedure hgState(Sender: TObject; State: TGridState);

    procedure hgSelect(Sender: TObject; Item, OldItem: Integer);
    procedure hgXMLData(Sender: TObject; Index: Integer; var Item: TXMLItem);
    procedure hgMCData(Sender: TObject; Index: Integer; var Item: TMCItem; Stage: TSaveStage);
    procedure OpenLinkClick(Sender: TObject);
    procedure OpenLinkNWClick(Sender: TObject);
    procedure CopyLinkClick(Sender: TObject);
    procedure CopyText1Click(Sender: TObject);
    procedure hgUrlClick(Sender: TObject; Item: Integer; const URLText: String;  Button: TMouseButton);
    procedure hgProcessRichText(Sender: TObject; Handle: THandle; Item: Integer);
    procedure hgSearchItem(Sender: TObject; Item, ID: Integer; var Found: Boolean);
    procedure hgKeyUp(Sender: TObject; var Key: Word; Shift: TShiftState);
    procedure ContactRTLmode1Click(Sender: TObject);
    procedure SendMessage1Click(Sender: TObject);
    procedure ReplyQuoted1Click(Sender: TObject);
    procedure CodepageChangeClick(Sender: TObject);
    procedure sbClearFilterClick(Sender: TObject);
    procedure pbFilterPaint(Sender: TObject);
    procedure StartHotFilterTimer;
    procedure EndHotFilterTimer(DoClearFilter: Boolean = False);
    procedure tiFilterTimer(Sender: TObject);
    procedure tbHistorySearchClick(Sender: TObject);
    procedure EmptyHistory1Click(Sender: TObject);
    procedure EventsFilterItemClick(Sender: TObject);
    procedure SessSelectClick(Sender: TObject);
    procedure pmGridPopup(Sender: TObject);
    procedure pmHistoryPopup(Sender: TObject);
    procedure tbUserMenuClick(Sender: TObject);
    procedure tvSessGetSelectedIndex(Sender: TObject; Node: TTreeNode);
    procedure Customize1Click(Sender: TObject);
    procedure tbEventsFilterClick(Sender: TObject);
    procedure hgRTLEnabled(Sender: TObject; BiDiMode: TBiDiMode);
    procedure ToolbarDblClick(Sender: TObject);
    procedure Customize2Click(Sender: TObject);
    procedure Bookmark1Click(Sender: TObject);
    procedure tbUserDetailsClick(Sender: TObject);
    procedure hgBookmarkClick(Sender: TObject; Item: Integer);
    procedure tbBookmarksClick(Sender: TObject);
    procedure sbCloseBookClick(Sender: TObject);
    procedure lvBookSelectItem(Sender: TObject; Item: TListItem; Selected: Boolean);
    procedure lvBookContextPopup(Sender: TObject; MousePos: TPoint; var Handled: Boolean);
    procedure lvBookEdited(Sender: TObject; Item: TListItem; var S: String);
    procedure RenameBookmark1Click(Sender: TObject);
    procedure hgProcessInlineChange(Sender: TObject; Enabled: Boolean);
    procedure hgInlinePopup(Sender: TObject);
    procedure InlineCopyClick(Sender: TObject);
    procedure InlineCopyAllClick(Sender: TObject);
    procedure InlineSelectAllClick(Sender: TObject);
    procedure InlineTextFormattingClick(Sender: TObject);
    procedure hgInlineKeyDown(Sender: TObject; var Key: Word; Shift: TShiftState);
    procedure InlineReplyQuotedClick(Sender: TObject);
    procedure pmEventsFilterPopup(Sender: TObject);
    procedure mmToolbarClick(Sender: TObject);
    procedure mmHideMenuClick(Sender: TObject);
    procedure SelectAll1Click(Sender: TObject);
    procedure lvBookKeyDown(Sender: TObject; var Key: Word; Shift: TShiftState);
    procedure tvSessKeyDown(Sender: TObject; var Key: Word; Shift: TShiftState);
    procedure paHolderResize(Sender: TObject);
    procedure spBookMoved(Sender: TObject);
    procedure pmToolbarPopup(Sender: TObject);
    procedure hgFilterChange(Sender: TObject);
    procedure OpenFileFolderClick(Sender: TObject);
    procedure BrowseReceivedFilesClick(Sender: TObject);
    procedure hgOptionsChange(Sender: TObject);
  private
    DelayedFilter: TMessageTypes;
    StartTimestamp: DWord;
    EndTimestamp: DWord;
    FhContact, FhSubContact: TMCONTACT;
    FProtocol, FSubProtocol: AnsiString;
    SavedLinkUrl: String;
    SavedFileDir: String;
    HotFilterString: String;
    FormState: TGridState;
    PreHotSearchMode: TSearchMode;
    FSearchMode: TSearchMode;
    UserMenu: hMenu;
    FPanel: THistoryPanels;
    IsLoadingSessions: Boolean;

    procedure WMGetMinMaxInfo(var Message: TWMGetMinMaxInfo); message WM_GETMINMAXINFO;
    procedure CMShowingChanged(var Message: TMessage); message CM_SHOWINGCHANGED;
    procedure WMSysColorChange(var Message: TMessage); message WM_SYSCOLORCHANGE;
    procedure LoadPosition;
    procedure SavePosition;

    procedure HMEventAdded(var Message: TMessage); message HM_MIEV_EVENTADDED;
    procedure HMEventDeleted(var Message: TMessage); message HM_MIEV_EVENTDELETED;
    procedure HMPreShutdown(var Message: TMessage); message HM_MIEV_PRESHUTDOWN;
    procedure HMContactDeleted(var Message: TMessage); message HM_MIEV_CONTACTDELETED;
    procedure HMMetaDefaultChanged(var M: TMessage); message HM_MIEV_METADEFCHANGED;

    procedure HMIcons2Changed(var M: TMessage); message HM_NOTF_ICONS2CHANGED;
    procedure HMAccChanged(var M: TMessage); message HM_NOTF_ACCCHANGED;
    procedure HMNickChanged(var M: TMessage); message HM_NOTF_NICKCHANGED;

    procedure OpenDetails(Item: Integer);
    procedure TranslateForm;

    procedure SethContact(const Value: TMCONTACT);
    procedure LoadInOptions();
    function IsFileEvent(Index: Integer): Boolean;

    procedure PreLoadHistory;
    procedure PostLoadHistory;
    procedure SetSearchMode(const Value: TSearchMode);
    procedure SetPanel(const Value: THistoryPanels);
    procedure ToggleMainMenu(Enabled: Boolean);

  protected
    procedure LoadPendingHeaders(rowidx: Integer; count: Integer);
    property SearchMode: TSearchMode read FSearchMode write SetSearchMode;
    property Panel: THistoryPanels read FPanel write SetPanel;
    procedure WndProc(var Message: TMessage); override;

  public
    UserCodepage: Cardinal;
    UseDefaultCP: Boolean;
    LastSearch: TLastSearch;
    HotString: String;
    LastHotIdx: Integer;
    EventDetailForm: TForm;
    CustomizeFiltersForm: TForm;
    CustomizeToolbarForm: TForm;
    WindowList: TList;
    History: array of THandle;
    HistoryLength: Integer;
    RecentFormat: TSaveFormat;
    SessThread: TSessionsThread;
    Sessions: TSessArray;
    SeparatorButtonWidth: Integer;

    procedure SearchNext(Rev: Boolean; Warp: Boolean = True);
    procedure DeleteHistoryItem(ItemIdx: Integer);
    procedure AddHistoryItem(hDBEvent: THandle);
    procedure Load;
    function GridIndexToHistory(Index: Integer): Integer;
    function HistoryIndexToGrid(Index: Integer): Integer;
    function GetItemData(Index: Integer): THistoryItem;

    procedure ReplyQuoted(Item: Integer);
    procedure EmptyHistory;

    procedure SMPrepare(var M: TMessage); message HM_SESS_PREPARE;
    procedure SMItemsFound(var M: TMessage); message HM_SESS_ITEMSFOUND;
    procedure SMFinished(var M: TMessage); message HM_SESS_FINISHED;
    procedure AddEventToSessions(hDBEvent: THandle);
    procedure DeleteEventFromSessions(ItemIdx: Integer);

    procedure LoadSessionIcons;
    procedure LoadBookIcons;
    procedure LoadToolbarIcons;
    procedure LoadEventFilterButton;
    procedure LoadButtonIcons;

    procedure CustomizeToolbar;
    procedure LoadToolbar;
    procedure LoadAccMenu;
    procedure HMToolbarChanged(var M: TMessage); message HM_NOTF_TOOLBARCHANGED;

    procedure SetRecentEventsPosition(OnTop: Boolean);
    procedure Search(Next: Boolean; FromNext: Boolean = False);

    procedure ShowAllEvents;
    procedure ShowItem(Value: Integer);
    procedure SetEventFilter(FilterIndex: Integer = -1; DelayApply: Boolean = False);
    procedure CreateEventsFilterMenu;
    procedure HMFiltersChanged(var M: TMessage); message HM_NOTF_FILTERSCHANGED;

    procedure FillBookmarks;
    procedure HMBookmarkChanged(var M: TMessage); message HM_NOTF_BOOKMARKCHANGED;

    property hContact: TMCONTACT read FhContact write SethContact;
    property Protocol: AnsiString read FProtocol;
    property hSubContact: TMCONTACT read FhSubContact;
    property SubProtocol: AnsiString read FSubProtocol;
  published
    procedure AlignControls(Control: TControl; var ARect: TRect); override;
  end;

var
  HistoryFrm: THistoryFrm;

const
  DEF_HISTORY_TOOLBAR = '[SESS][BOOK] [SEARCH][FILTER] [EVENTS] [COPY][DELETE] [HISTORY]';

  // function ParseUrlItem(Item: THistoryItem; out Url,Mes: WideString): Boolean;
  // function ParseFileItem(Item: THistoryItem; out FileName,Mes: WideString): Boolean;

implementation

uses
  EventDetailForm, hpp_options, hpp_services, hpp_eventfilters,
  hpp_database, hpp_contacts, hpp_itemprocess, hpp_events, hpp_forms, hpp_richedit,
  hpp_messages, hpp_bookmarks, Checksum, CustomizeFiltersForm, CustomizeToolbar;

{$R *.DFM}

const
  HPP_SESS_YEARFORMAT = 'yyyy';
  HPP_SESS_MONTHFORMAT = 'mmmm';
  HPP_SESS_DAYFORMAT = 'd (h:nn)';

function Max(a, b: Integer): Integer;
begin
  if b > a then
    Result := b
  else
    Result := a
end;

function NotZero(X: THandle): THandle;
// used that array doesn't store 0 for already loaded data
begin
  if X = 0 then
    Result := 1
  else
    Result := X
end;

{ function ParseUrlItem(Item: THistoryItem; out Url,Mes: WideString): Boolean;
  var
  tmp1,tmp2: WideString;
  n: Integer;
  begin
  Url := '';
  Mes := '';
  Result := False;
  if not (mtUrl in Item.MessageType) then exit;
  tmp1 := Item.Text;
  if tmp1 = '' then exit;
  Result := True;

  n := Pos(#10,tmp1);
  if n <> 0 then begin
  tmp2 := Copy(tmp1,1,n-2);
  Delete(tmp1,1,n);
  end else begin
  tmp2 := tmp1;
  tmp1 := '';
  end;

  Mes := tmp1;

  n := Pos(':',tmp2);
  if n <> 0 then begin
  tmp2 := Copy(tmp2,n+2,Length(tmp2));
  end else begin
  Result := False;
  tmp2 := '';
  end;

  url := tmp2;
  end; }

{ function ParseFileItem(Item: THistoryItem; out FileName,Mes: WideString): Boolean;
  var
  tmp1,tmp2: AnsiString;
  n: Integer;
  begin
  Result := False;
  FileName := '';
  Mes := '';
  if not (mtFile in Item.MessageType) then exit;
  tmp1 := Item.Text;

  n := Pos(#10,tmp1);
  if n <> 0 then begin
  Delete(tmp1,1,n)
  end else
  exit;

  Result := True;

  n := Pos(#10,tmp1);
  if n <> 0 then begin
  tmp2 := tmp1;
  tmp1 := Copy(tmp2,1,n-2);
  Delete(tmp2,1,n);
  end;

  Mes := tmp2;
  FileName := tmp1;
  end; }

{ function GetEventInfo(hDBEvent: DWord): TDBEVENTINFO;
  var
  BlobSize:Integer;
  begin
  ZeroMemory(@Result,SizeOf(Result));
  Result.cbSize:=SizeOf(Result);
  Result.pBlob:=nil;
  BlobSize:=CallService(MS_DB_EVENT_GETBLOBSIZE,hDBEvent,0);

  GetMem(Result.pBlob,BlobSize);
  Result.cbBlob:=BlobSize;

  CallService(MS_DB_EVENT_GET,hDBEvent,LPARAM(@Result));
  end; }

(*
  This function gets only name of the file
  and tries to make it FAT-happy, so we trim out and
  ":"-s, "\"-s and so on...
*)

procedure THistoryFrm.LoadHistory(Sender: TObject);
// Load the History from the Database and Display it in the grid
  procedure FastLoadHandles;
  var
    hDBEvent: THandle;
    LineIdx: Integer;
    ToRead: Integer;
  begin
    HistoryLength := db_event_count(hContact);
    if HistoryLength = -1 then
    begin
      // contact is missing
      // or other error ?
      HistoryLength := 0;
    end;
    SetLength(History, HistoryLength);
    if HistoryLength = 0 then
      Exit;
    hDBEvent := db_event_last(hContact);
    History[HistoryLength - 1] := NotZero(hDBEvent);
    { if NeedhDBEvent = 0 then toRead := Max(0,HistoryLength-hppLoadBlock-1)
      else toRead := 0; }
    ToRead := Max(0, HistoryLength - hppFirstLoadBlock - 1);
    LineIdx := HistoryLength - 2;
    repeat
      hDBEvent := db_event_prev(hContact,hDBEvent);
      History[LineIdx] := NotZero(hDBEvent);
      { if NeedhDBEvent = hDbEvent then begin
        Result := HistoryLength-LineIdx-1;
        toRead := Max(0,Result-hppLoadBlock shr 1);
        end; }
      dec(LineIdx);
    until LineIdx < ToRead;
  end;

begin
  FastLoadHandles;

  hg.Contact := hContact;
  hg.Protocol := Protocol;
  // hContact,hSubContact,Protocol,SubProtocol should be
  // already filled by calling hContact := Value;
  hg.ProfileName := GetContactDisplayName(0, SubProtocol);
  hg.ContactName := GetContactDisplayName(hContact, Protocol, True);
  UserCodepage := GetContactCodePage(hContact, Protocol, UseDefaultCP);
  hg.Codepage := UserCodepage;
  hg.RTLMode := GetContactRTLModeTRTL(hContact, Protocol);
  UnknownCodepage.Tag := Integer(UserCodepage);
  UnknownCodepage.Caption := Format(TranslateW('Unknown codepage %u'), [UserCodepage]);
  if hContact = 0 then
    Caption := TranslateW('System History')
  else
    Caption := Format(TranslateW('%s - History++'), [hg.ContactName]);
  hg.Allocate(HistoryLength);
end;

procedure THistoryFrm.FormCreate(Sender: TObject);
var
  i: Integer;
  mi: TMenuItem;
begin
  hg.BeginUpdate;

  Icon.ReleaseHandle;
  Icon.Handle := CopyIcon(hppIcons[HPP_ICON_CONTACTHISTORY].Handle);

  // delphi 2006 doesn't save toolbar's flat property in dfm if it is True
  // delphi 2006 doesn't save toolbar's edgeborder property in dfm
  Toolbar.Flat := True;
  Toolbar.EdgeBorders := [];

  LoadToolbarIcons;
  LoadButtonIcons;
  LoadSessionIcons;
  LoadBookIcons;

  DesktopFont := True;
  MakeFontsParent(Self);

  DoubleBuffered := True;
  MakeDoubleBufferedParent(Self);
  TopPanel.DoubleBuffered := False;
  hg.DoubleBuffered := False;

  IsLoadingSessions := False;
  SessThread := nil;

  FormState := gsIdle;

  DelayedFilter := [];
  // if we do so, we'll never get selected if filters match
  // hg.Filter := GenerateEvents(FM_EXCLUDE,[]);

  for i := 0 to High(cpTable) do
  begin
    mi := NewItem(cpTable[i].name, 0, False, True, nil, 0, 'cp' + intToStr(i));
    mi.Tag := cpTable[i].cp;
    mi.OnClick := CodepageChangeClick;
    mi.AutoCheck := True;
    mi.RadioItem := True;
    ANSICodepage.Add(mi);
  end;

  TranslateForm;

  // File actions from context menu support
  AddMenuArray(pmGrid, [FileActions], -1);

  LoadAccMenu; // load accessability menu before LoadToolbar
  // put here because we want to translate everything
  // before copying to menu

  // cbFilter.ItemIndex := 0;
  RecentFormat := sfHtml;
  // hg.InlineRichEdit.PopupMenu := pmGridInline;
  // for i := 0 to pmOptions.Items.Count-1 do
  // pmOptions.Items.Remove(pmOptions.Items[0]);
end;

procedure THistoryFrm.LoadPosition;
// load last position and filter setting
// var
// filt: Integer;
// w,h,l,t: Integer;
begin
  // removed Utils_RestoreWindowPosition because it shows window sooner than we expect
  Utils_RestoreFormPosition(Self, 0, hppDBName, 'HistoryWindow.');
  SearchMode := TSearchMode(GetDBByte(hppDBName, 'SearchMode', 0));
end;

procedure THistoryFrm.LoadSessionIcons;
var
  il: THandle;
begin
  tvSess.Items.BeginUpdate;
  try
    ImageList_Remove(ilSessions.Handle, -1); // clears image list
    il := ImageList_Create(16, 16, ILC_COLOR32 or ILC_MASK, 8, 2);
    if il <> 0 then
      ilSessions.Handle := il
    else
      il := ilSessions.Handle;

    ImageList_AddIcon(il, hppIcons[HPP_ICON_SESSION].Handle);
    ImageList_AddIcon(il, hppIcons[HPP_ICON_SESS_SUMMER].Handle);
    ImageList_AddIcon(il, hppIcons[HPP_ICON_SESS_AUTUMN].Handle);
    ImageList_AddIcon(il, hppIcons[HPP_ICON_SESS_WINTER].Handle);
    ImageList_AddIcon(il, hppIcons[HPP_ICON_SESS_SPRING].Handle);
    ImageList_AddIcon(il, hppIcons[HPP_ICON_SESS_YEAR].Handle);
  finally
    tvSess.Items.EndUpdate;
    // tvSess.Update;
  end;

  // simple hack to avoid dark icons
  ilSessions.BkColor := tvSess.Color;

end;

// to do:
// SAVEALL (???)
// DELETEALL
// SENDMES (???)
// REPLQUOTED (???)
// COPYTEXT (???)
procedure THistoryFrm.LoadToolbar;
var
  tool: array of TControl;
  i, n: Integer;
  tb_butt: THppToolButton;
  butt: TControl;
  butt_str, tb_str, str: String;
begin
  tb_str := String(GetDBStr(hppDBName, 'HistoryToolbar', DEF_HISTORY_TOOLBAR));

  if (tb_str = '') then
  begin // toolbar is disabled
    Toolbar.Visible := False;
    // should add "else T.Visible := True" to make it dynamic in run-time, but I will ignore it
    // you can never know which Toolbar bugs & quirks you'll trigger with it :)
  end;

  if hContact = 0 then
  begin
    tb_str := StringReplace(tb_str, '[SESS]', '', [rfReplaceAll]);
    // tb_str := StringReplace(tb_str,'[BOOK]','',[rfReplaceAll]);
    // tb_str := StringReplace(tb_str,'[EVENTS]','',[rfReplaceAll]);
  end;
  str := tb_str;

  i := 0;
  while True do
  begin
    if i = Toolbar.ControlCount then
      break;
    if Toolbar.Controls[i] is THppToolButton then
    begin
      tb_butt := THppToolButton(Toolbar.Controls[i]);
      if (tb_butt.Style = tbsSeparator) or (tb_butt.Style = tbsDivider) then
      begin
        // adding separator in runtime results in too wide separators
        // we'll remeber the currect width and later re-apply it
        SeparatorButtonWidth := tb_butt.Width;
        tb_butt.Free;
        dec(i);
      end
      else
        tb_butt.Visible := False;
    end
    else if Toolbar.Controls[i] is TSpeedButton then
      TSpeedButton(Toolbar.Controls[i]).Visible := False;
    Inc(i);
  end;

  try
    while True do
    begin
      if str = '' then
        break;
      if (str[1] = ' ') or (str[1] = '|') then
      begin
        if (Length(tool) > 0) and (tool[High(tool)] is THppToolButton) then
        begin
          // don't add separator if previous button is separator
          tb_butt := THppToolButton(tool[High(tool)]);
          if (tb_butt.Style = tbsDivider) or (tb_butt.Style = tbsSeparator) then
          begin
            Delete(str, 1, 1);
            continue;
          end;
        end
        else if (Length(tool) = 0) then
        begin
          // don't add separator as first button
          Delete(str, 1, 1);
          continue;
        end;
        SetLength(tool, Length(tool) + 1);
        tb_butt := THppToolButton.Create(Toolbar);
        tb_butt.Visible := False;
        if str[1] = ' ' then
          tb_butt.Style := tbsSeparator
        else
          tb_butt.Style := tbsDivider;
        Delete(str, 1, 1);
        tb_butt.Parent := Toolbar;
        tb_butt.Width := SeparatorButtonWidth;
        tool[High(tool)] := tb_butt;
      end
      else if str[1] = '[' then
      begin
        n := Pos(']', str);
        if n = -1 then
          raise EAbort.Create('Wrong toolbar string format');
        butt_str := Copy(str, 2, n - 2);
        Delete(str, 1, n);
        if      butt_str = 'SESS'        then butt := tbSessions
        else if butt_str = 'BOOK'        then butt := tbBookmarks
        else if butt_str = 'SEARCH'      then butt := tbSearch
        else if butt_str = 'FILTER'      then butt := tbFilter
        else if butt_str = 'COPY'        then butt := tbCopy
        else if butt_str = 'DELETE'      then butt := tbDelete
        else if butt_str = 'SAVE'        then butt := tbSave
        else if butt_str = 'HISTORY'     then butt := tbHistory
        else if butt_str = 'GLOBSEARCH'  then butt := tbHistorySearch
        else if butt_str = 'EVENTS'      then butt := tbEventsFilter
        else if butt_str = 'USERMENU'    then butt := tbUserMenu
        else if butt_str = 'USERDETAILS' then butt := tbUserDetails
        else
          butt := nil;

        if butt <> nil then
        begin
          SetLength(tool, Length(tool) + 1);
          tool[High(tool)] := butt;
        end;
      end
      else
        raise EAbort.Create('Wrong toolbar string format');
    end;
  except
    // if we have error, try loading default toolbar config or
    // show error if it doesn't work
    if tb_str = DEF_HISTORY_TOOLBAR then
    begin
      // don't think it should be translated:
      HppMessageBox(Handle, 'Can not apply default toolbar configuration.' + #10#13 +
        'Looks like it is an internal problem.' + #10#13 + #10#13 +
        'Download new History++ version or report the error to the authors' + #10#13 +
        '(include plugin version number and file date in the report).' + #10#13 + #10#13 +
        'You can find authors'' emails and plugin website in the Options->Plugins page.',
        TranslateW('Error'), MB_OK or MB_ICONERROR);
      Exit;
    end
    else
    begin
      DBDeleteContactSetting(0, hppDBName, 'HistoryToolbar');
      LoadToolbar;
      Exit;
    end;
  end;

  // move buttons in reverse order and set parent afterwards
  // thanks Luu Tran for this tip
  // http://groups.google.com/group/borland.public.delphi.vcl.components.using/browse_thread/thread/da4e4da814baa745/c1ce8b671c1dac20
  for i := High(tool) downto 0 do
  begin
    if not(tool[i] is TSpeedButton) then
      tool[i].Parent := nil;
    tool[i].Left := -3;
    tool[i].Visible := True;
    if not(tool[i] is TSpeedButton) then
      tool[i].Parent := Toolbar;
  end;

  // Thanks Primoz Gabrijeleie for this trick!
  // http://groups.google.com/group/alt.comp.lang.borland-delphi/browse_thread/thread/da77e8db6d8f418a/dc4fd87eee6b1d54
  // This f***ing toolbar has almost got me!
  // A bit of explanation: without the following line loading toolbar when
  // window is show results in unpredictable buttons placed on toolbar
  Toolbar.Perform(CM_RECREATEWND, 0, 0);
end;

procedure THistoryFrm.LoadToolbarIcons;
var
  il: HIMAGELIST;
begin
  try
    ImageList_Remove(ilToolbar.Handle, -1); // clears image list
    il := ImageList_Create(16, 16, ILC_COLOR32 or ILC_MASK, 10, 2);
    if il <> 0 then
      ilToolbar.Handle := il
    else
      il := ilToolbar.Handle;
    Toolbar.Images := ilToolbar;

    // add other icons without processing
    tbUserDetails.ImageIndex   := ImageList_AddIcon(il, hppIcons[HPP_ICON_CONTACDETAILS].Handle);
    tbUserMenu.ImageIndex      := ImageList_AddIcon(il, hppIcons[HPP_ICON_CONTACTMENU].Handle);
    tbFilter.ImageIndex        := ImageList_AddIcon(il, hppIcons[HPP_ICON_HOTFILTER].Handle);
    tbSearch.ImageIndex        := ImageList_AddIcon(il, hppIcons[HPP_ICON_HOTSEARCH].Handle);
    tbDelete.ImageIndex        := ImageList_AddIcon(il, hppIcons[HPP_ICON_TOOL_DELETE].Handle);
    tbSessions.ImageIndex      := ImageList_AddIcon(il, hppIcons[HPP_ICON_TOOL_SESSIONS].Handle);
    tbSave.ImageIndex          := ImageList_AddIcon(il, hppIcons[HPP_ICON_TOOL_SAVE].Handle);
    tbCopy.ImageIndex          := ImageList_AddIcon(il, hppIcons[HPP_ICON_TOOL_COPY].Handle);
    tbHistorySearch.ImageIndex := ImageList_AddIcon(il, hppIcons[HPP_ICON_GLOBALSEARCH].Handle);
    tbBookmarks.ImageIndex     := ImageList_AddIcon(il, hppIcons[HPP_ICON_BOOKMARK].Handle);
    tbHistory.ImageIndex       := ImageList_AddIcon(il, hppIcons[HPP_ICON_CONTACTHISTORY].Handle);

    LoadEventFilterButton;
  finally
  end;
end;

procedure THistoryFrm.SavePosition;
// save position and filter setting
var
  SearchModeForSave: TSearchMode;
begin
  Utils_SaveFormPosition(Self, 0, hppDBName, 'HistoryWindow.');

  if (HistoryLength > 0) then
  begin
    if hContact = 0 then
    begin
      WriteDBBool(hppDBName, 'ShowBookmarksSystem', paBook.Visible);
      if paBook.Visible then
        WriteDBInt(hppDBName, 'PanelWidth', paBook.Width);
    end
    else
    begin
      WriteDBBool(hppDBName, 'ShowSessions', paSess.Visible);
      WriteDBBool(hppDBName, 'ShowBookmarks', paBook.Visible);
      if paHolder.Visible then
        WriteDBInt(hppDBName, 'PanelWidth', paHolder.Width);
      if spBook.Visible then
        WriteDBByte(hppDBName, 'PanelSplit', spBook.Tag);
    end;
  end;

  if hContact <> 0 then
    WriteDBBool(hppDBName, 'ExpandHeaders', hg.ExpandHeaders);
  if SearchMode = smHotSearch then
    SearchModeForSave := PreHotSearchMode
  else
    SearchModeForSave := SearchMode;
  WriteDBByte(hppDBName, 'SearchMode', Byte(SearchModeForSave));
end;

procedure THistoryFrm.HMEventAdded(var Message: TMessage);
// new message added to history (wparam=hcontact, lparam=hdbevent)
begin
  // if for this contact
  if THandle(message.wParam) = hContact then
  begin
    // receive message from database
    AddHistoryItem(message.lParam);
    hgState(hg, hg.State);
  end;
end;

procedure THistoryFrm.HMEventDeleted(var Message: TMessage);
var
  i: Integer;
begin
  { wParam - hContact; lParam - hDBEvent }
  if hg.State = gsDelete then
    Exit;
  if THandle(message.wParam) <> hContact then
    Exit;
  for i := 0 to hg.count - 1 do
    if (History[GridIndexToHistory(i)] = THandle(Message.lParam)) then
    begin
      hg.Delete(i);
      hgState(hg, hg.State);
      Exit;
    end;
end;

procedure THistoryFrm.HMFiltersChanged(var M: TMessage);
begin
  CreateEventsFilterMenu;
  SetEventFilter(0);
end;

procedure THistoryFrm.HMIcons2Changed(var M: TMessage);
begin
  Icon.Handle := CopyIcon(hppIcons[HPP_ICON_CONTACTHISTORY].Handle);
  LoadToolbarIcons;
  LoadButtonIcons;
  LoadSessionIcons;
  LoadBookIcons;
  pbFilter.Repaint;
  // hg.Repaint;
end;

procedure THistoryFrm.HMAccChanged(var M: TMessage);
begin
  ToggleMainMenu(Boolean(M.wParam));
end;

procedure THistoryFrm.HMBookmarkChanged(var M: TMessage);
var
  i: Integer;
begin
  if THandle(M.wParam) <> hContact then
    Exit;
  for i := 0 to hg.count - 1 do
    if History[GridIndexToHistory(i)] = THandle(M.lParam) then
    begin
      hg.Bookmarked[i] := BookmarkServer[hContact].Bookmarked[M.lParam];
      break;
    end;
  FillBookmarks;
end;

procedure THistoryFrm.HMPreShutdown(var Message: TMessage);
begin
  Close;
end;

procedure THistoryFrm.HMContactDeleted(var Message: TMessage);
begin
  if THandle(Message.wParam) <> hContact then
    Exit;
  Close;
end;

procedure THistoryFrm.HMToolbarChanged(var M: TMessage);
begin
  LoadToolbar;
end;

procedure THistoryFrm.HMNickChanged(var M: TMessage);
begin
  if SubProtocol = '' then
    Exit;
  hg.BeginUpdate;
  if M.wParam = 0 then
    hg.ProfileName := GetContactDisplayName(0, SubProtocol)
  else if THandle(M.wParam) = hContact then
  begin
    hg.ProfileName := GetContactDisplayName(0, SubProtocol);
    hg.ContactName := GetContactDisplayName(hContact, Protocol, True);
    Caption := Format(TranslateW('%s - History++'), [hg.ContactName]);
  end;
  hg.EndUpdate;
  hg.Invalidate;
  if Assigned(EventDetailForm) then
    TEventDetailsFrm(EventDetailForm).ResetItem;
end;

procedure THistoryFrm.HMMetaDefaultChanged(var M: TMessage);
var
  newSubContact: TMCONTACT;
  newSubProtocol: AnsiString;
begin
  if THandle(M.wParam) <> hContact then
    Exit;
  GetContactProto(hContact, newSubContact, newSubProtocol);
  if (hSubContact <> newSubContact) or (SubProtocol <> newSubProtocol) then
  begin
    hg.BeginUpdate;
    FhSubContact := newSubContact;
    FSubProtocol := newSubProtocol;
    hg.ProfileName := GetContactDisplayName(0, newSubProtocol);
    hg.ContactName := GetContactDisplayName(hContact, Protocol, True);
    Caption := Format(TranslateW('%s - History++'), [hg.ContactName]);
    hg.EndUpdate;
    hg.Invalidate;
    if Assigned(EventDetailForm) then
      TEventDetailsFrm(EventDetailForm).ResetItem;
  end;
end;

{ Unfortunatly when you make a form from a dll this form won't become the
  normal messages specified by the VCL but only the basic windows messages.
  Therefore neither tabs nor button shortcuts work on this form. As a workaround
  i've make some functions: }

procedure THistoryFrm.OnCNChar(var Message: TWMChar);
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

procedure THistoryFrm.FormKeyDown(Sender: TObject; var Key: Word; Shift: TShiftState);
var
  Mask: Integer;
begin
  if (Key = VK_ESCAPE) or ((Key = VK_F4) and (ssAlt in Shift)) then
  begin
    if (Key = VK_ESCAPE) and edSearch.Focused then
      SearchMode := smNone
    else
      Close;
    Key := 0;
    Exit;
  end;

  if (Key = VK_F10) and (Shift = []) then
  begin
    WriteDBBool(hppDBName, 'Accessability', True);
    NotifyAllForms(HM_NOTF_ACCCHANGED, ORD(True), 0);
    Key := 0;
    Exit;
  end;

  if (Key = VK_F3) and ((Shift = []) or (Shift = [ssShift])) and
    (SearchMode in [smSearch, smHotSearch]) then
  begin
    if ssShift in Shift then
      sbSearchPrev.Click
    else
      sbSearchNext.Click;
    Key := 0;
  end;

  // let only search keys be accepted if inline
  if hg.State = gsInline then
    Exit;

  if IsFormShortCut([mmAcc], Key, Shift) then
  begin
    Key := 0;
    Exit;
  end;

  with Sender as TWinControl do
  begin
    if Perform(CM_CHILDKEY, Key, lParam(Sender)) <> 0 then
      Exit;
    Mask := 0;
    case Key of
      VK_TAB:
        Mask := DLGC_WANTTAB;
      VK_RETURN, VK_EXECUTE, VK_ESCAPE, VK_CANCEL:
        Mask := DLGC_WANTALLKEYS;
    end;
    if (Mask <> 0) and (Perform(CM_WANTSPECIALKEY, Key, 0) = 0) and
      (Perform(WM_GETDLGCODE, 0, 0) and Mask = 0) and (Perform(CM_DIALOGKEY, Key, 0) <> 0) then
      Exit;
  end;
end;

procedure THistoryFrm.FillBookmarks;
var
  li: TListItem;
  cb: TContactBookmarks;
  i: Integer;
  hi: THistoryItem;
  hDBEvent: THandle;
  txt: String;
begin
  lvBook.Items.BeginUpdate;
  try
    lvBook.Items.Clear;
    // prefetch contact bookmarks object so we don't seek for it every time
    cb := BookmarkServer[hContact];
    for i := 0 to cb.count - 1 do
    begin
      li := lvBook.Items.Add;
      hDBEvent := cb.Items[i];
      txt := cb.Names[i];
      if txt = '' then
      begin
        hi := ReadEvent(hDBEvent, UserCodepage);
        txt := Copy(hi.Text, 1, 100);
        txt := StringReplace(txt, #13#10, ' ', [rfReplaceAll]);
        // without freeing Module AnsiString mem manager complains about memory leak! WTF???
        Finalize(hi);
        // hi.Module := '';
        // hi.Proto := '';
        // hi.Text := '';
      end;
      // compress spaces here!
      li.Caption := txt;
      li.Data := Pointer(hDBEvent);
      li.ImageIndex := 0;
    end;
  finally
    lvBook.Items.EndUpdate;
  end;
end;

procedure THistoryFrm.FormClose(Sender: TObject; var Action: TCloseAction);
begin
  try
    Action := caFree;
    if Assigned(WindowList) then
    begin
      WindowList.Delete(WindowList.IndexOf(Self));
    end;
    SavePosition;
  except
  end;
end;

procedure THistoryFrm.FormCloseQuery(Sender: TObject; var CanClose: Boolean);
var
  Flag: UINT;
  AppSysMenu: THandle;
begin
  CanClose := (hg.State in [gsIdle, gsInline]);
  if CanClose and IsLoadingSessions then
  begin
    // disable close button
    AppSysMenu := GetSystemMenu(Handle, False);
    Flag := MF_GRAYED;
    EnableMenuItem(AppSysMenu, SC_CLOSE, MF_BYCOMMAND or Flag);
    sb.SimpleText := TranslateW('Please wait while closing the window...');
    // terminate thread
    SessThread.Terminate(tpHigher);
    repeat
      Application.ProcessMessages
    until not IsLoadingSessions;
  end;
  if CanClose and Assigned(SessThread) then
    FreeAndNil(SessThread);
end;

procedure THistoryFrm.Load;
begin
  PreLoadHistory;
  LoadHistory(Self);
  PostLoadHistory;
end;

procedure THistoryFrm.LoadAccMenu;
var
  i: Integer;
  wstr: String;
  menuitem: TMenuItem;
  pm: TPopupMenu;
begin
  mmToolbar.Clear;
  for i := Toolbar.ButtonCount - 1 downto 0 do
  begin
    if Toolbar.Buttons[i].Style = tbsSeparator then
    begin
      menuitem := TMenuItem.Create(mmToolbar);
      menuitem.Caption := '-';
    end
    else
    begin
      menuitem := TMenuItem.Create(Toolbar.Buttons[i]);
      wstr := Toolbar.Buttons[i].Caption;
      if wstr = '' then
        wstr := Toolbar.Buttons[i].Hint;
      if wstr <> '' then
      begin
        pm := TPopupMenu(Toolbar.Buttons[i].PopupMenu);
        if pm = nil then
          menuitem.OnClick := Toolbar.Buttons[i].OnClick
        else
        begin
          menuitem.Tag := uint_ptr(Pointer(pm));
        end;
        menuitem.Caption := wstr;
        menuitem.ShortCut := TextToShortCut(Toolbar.Buttons[i].HelpKeyword);
        menuitem.Enabled := Toolbar.Buttons[i].Enabled;
        menuitem.Visible := Toolbar.Buttons[i].Visible;
      end;
    end;
    mmToolbar.Insert(0, menuitem);
  end;
  mmToolbar.RethinkHotkeys;
end;

var
  SearchUpHint: String = 'Search Up (Ctrl+Up)';
  SearchDownHint: String = 'Search Down (Ctrl+Down)';

procedure THistoryFrm.LoadBookIcons;
var
  il: THandle;
begin
  lvBook.Items.BeginUpdate;
  try
    ImageList_Remove(ilBook.Handle, -1); // clears image list
    il := ImageList_Create(16, 16, ILC_COLOR32 or ILC_MASK, 8, 2);
    if il <> 0 then
      ilBook.Handle := il
    else
      il := ilBook.Handle;

    ImageList_AddIcon(il, hppIcons[HPP_ICON_BOOKMARK_ON].Handle);
  finally
    lvBook.Items.EndUpdate;
  end;
  // simple hack to avoid dark icons
  ilBook.BkColor := lvBook.Color;
end;

procedure sButtonIcon(var sb: TSpeedButton; Icon: HICON);
begin
  with sb.Glyph do
  begin
    Width := 16;
    Height := 16;
    Canvas.Brush.Color := clBtnFace;
    Canvas.FillRect(Canvas.ClipRect);
    DrawiconEx(Canvas.Handle, 0, 0, Icon, 16, 16, 0, Canvas.Brush.Handle, DI_NORMAL);
  end;
end;

procedure THistoryFrm.LoadButtonIcons;
var
  previc: HICON;
  nextic: HICON;
  // prev_hint, next_hint: WideString;
begin
  if hg.Reversed then
  begin
    nextic := hppIcons[HPP_ICON_SEARCHUP].Handle;
    previc := hppIcons[HPP_ICON_SEARCHDOWN].Handle;
    sbSearchNext.Hint := SearchUpHint;
    sbSearchPrev.Hint := SearchDownHint;
  end
  else
  begin
    nextic := hppIcons[HPP_ICON_SEARCHDOWN].Handle;
    previc := hppIcons[HPP_ICON_SEARCHUP].Handle;
    sbSearchNext.Hint := SearchDownHint;
    sbSearchPrev.Hint := SearchUpHint;
  end;

  sButtonIcon(sbSearchPrev, previc);
  sButtonIcon(sbSearchNext, nextic);
  sButtonIcon(sbClearFilter, hppIcons[HPP_ICON_HOTFILTERCLEAR].Handle);
  sButtonIcon(sbCloseSess, hppIcons[HPP_ICON_SESS_HIDE].Handle);
  sButtonIcon(sbCloseBook, hppIcons[HPP_ICON_SESS_HIDE].Handle);
  {
    with sbSearchPrev.Glyph do begin
    Width := 16;
    Height := 16;
    Canvas.Brush.Color := clBtnFace;
    Canvas.FillRect(Canvas.ClipRect);
    DrawiconEx(Canvas.Handle,0,0,
    previc,16,16,0,Canvas.Brush.Handle,DI_NORMAL);
    end;
    with sbSearchNext.Glyph do begin
    Width := 16;
    Height := 16;
    Canvas.Brush.Color := clBtnFace;
    Canvas.FillRect(Canvas.ClipRect);
    DrawiconEx(Canvas.Handle,0,0,
    nextic,16,16,0,Canvas.Brush.Handle,DI_NORMAL);
    end;
    with sbClearFilter.Glyph do begin
    Width := 16;
    Height := 16;
    Canvas.Brush.Color := clBtnFace;
    Canvas.FillRect(Canvas.ClipRect);
    DrawiconEx(Canvas.Handle,0,0,
    hppIcons[HPP_ICON_HOTFILTERCLEAR].Handle,16,16,0,Canvas.Brush.Handle,DI_NORMAL);
    end;
    with sbCloseSess.Glyph do begin
    Width := 16;
    Height := 16;
    Canvas.Brush.Color := clBtnFace;
    Canvas.FillRect(Canvas.ClipRect);
    DrawiconEx(Canvas.Handle,0,0,
    hppIcons[HPP_ICON_SESS_HIDE].Handle,16,16,0,Canvas.Brush.Handle,DI_NORMAL);
    end;
    with sbCloseBook.Glyph do begin
    Width := 16;
    Height := 16;
    Canvas.Brush.Color := clBtnFace;
    Canvas.FillRect(Canvas.ClipRect);
    DrawiconEx(Canvas.Handle,0,0,
    hppIcons[HPP_ICON_SESS_HIDE].Handle,16,16,0,Canvas.Brush.Handle,DI_NORMAL);
    end;
  }
  imSearchNotFound.Picture.Icon.Handle := CopyIcon(hppIcons[HPP_ICON_SEARCH_NOTFOUND].Handle);
  imSearchEndOfPage.Picture.Icon.Handle := CopyIcon(hppIcons[HPP_ICON_SEARCH_ENDOFPAGE].Handle);
end;

procedure THistoryFrm.LoadEventFilterButton;
var
  pad: DWord;
  { PadV, } PadH, GlyphHeight: Integer;
  sz: TSize;
  FirstName, name: String;
  PaintRect: TRect;
  DrawTextFlags: Cardinal;
  GlyphWidth: Integer;
begin
  FirstName := hppEventFilters[0].name;
  Name := hppEventFilters[tbEventsFilter.Tag].name;
  tbEventsFilter.Hint := Name; // show hint because the whole name may not fit in button

  pad := SendMessage(Toolbar.Handle, TB_GETPADDING, 0, 0);
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

  tbEventsFilter.Glyph.Canvas.Brush.Style := bsClear;
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

procedure THistoryFrm.LoadPendingHeaders(rowidx: Integer; count: Integer);
// reads hDBEvents from the database until this row (begin from end which was loaded at the startup)
// 2006.02.13 reads a windows with rowidx at center. Prefeching
var
  // startrowidx: integer;
  fromRow, tillRow: Integer;
  fromRowIdx, tillRowIdx: Integer;
  ridx: Integer;
  hDBEvent: THandle;
begin
  if History[rowidx] <> 0 then
    Exit;
{$IFDEF DEBUG}
  OutPutDebugString(PWideChar('Loading pending headers = ' + intToStr(rowidx)));
{$ENDIF}
  Screen.Cursor := crHourGlass;
  try
    fromRow := rowidx + hppLoadBlock shr 1;
    if fromRow > HistoryLength - 1 then
      fromRow := HistoryLength - 1;
    fromRowIdx := rowidx;
    repeat
      Inc(fromRowIdx)
    until (fromRowIdx > HistoryLength - 1) or (History[fromRowIdx] <> 0);

    tillRow := rowidx - hppLoadBlock shr 1;
    if tillRow < 0 then
      tillRow := 0;
    tillRowIdx := rowidx;
    repeat
      dec(tillRowIdx)
    until (tillRowIdx < 0) or (History[tillRowIdx] <> 0);

    if fromRowIdx - rowidx < rowidx - tillRowIdx then
    begin
      if fromRowIdx > HistoryLength - 1 then
      begin
        fromRowIdx := HistoryLength - 1;
        hDBEvent := db_event_last(hContact);
        History[fromRowIdx] := hDBEvent;
      end
      else
        hDBEvent := History[fromRowIdx];
      for ridx := fromRowIdx - 1 downto tillRow do
      begin
        if History[ridx] <> 0 then
          break;
        hDBEvent := db_event_prev(hContact,hDBEvent);
        History[ridx] := NotZero(hDBEvent);
      end;
    end
    else
    begin
      if tillRowIdx < 0 then
      begin
        tillRowIdx := 0;
        hDBEvent := db_event_first(hContact);
        History[tillRowIdx] := hDBEvent;
      end
      else
        hDBEvent := History[tillRowIdx];
      for ridx := tillRowIdx + 1 to fromRow do
      begin
        if History[ridx] <> 0 then
          break;
        hDBEvent := db_event_next(hContact,hDBEvent);
        History[ridx] := NotZero(hDBEvent);
      end;
    end;
{$IFDEF DEBUG}
    OutPutDebugString(PWideChar('... pending headers from ' + intToStr(fromRow) + ' to ' +
      intToStr(tillRow)));
{$ENDIF}
  finally
    Screen.Cursor := crDefault;
  end;
end;

procedure THistoryFrm.FormDestroy(Sender: TObject);
begin
  // this is the only event fired when history is open
  // and miranda is closed
  // (added: except now I added ME_SYSTEM_PRESHUTDOWN hook, which should work)
  if Assigned(CustomizeToolbarForm) then
    CustomizeToolbarForm.Release;
  if Assigned(CustomizeFiltersForm) then
    CustomizeFiltersForm.Release;
  if Assigned(EventDetailForm) then
    EventDetailForm.Release;
end;

procedure THistoryFrm.DeleteHistoryItem(ItemIdx: Integer);
// history[itemidx] löschen (also row-1)
// var
// p: integer;
begin
  // for p:=ItemIdx to HistoryLength-2 do
  // History[p]:=history[p+1];
  dec(HistoryLength);
  if ItemIdx <> HistoryLength then
  begin
    Move(History[ItemIdx + 1], History[ItemIdx], (HistoryLength - ItemIdx) *
      SizeOf(History[0]));
    // reset has_header and linked_to_pervous_messages fields
    hg.ResetItem(HistoryIndexToGrid(ItemIdx));
  end;
  SetLength(History, HistoryLength);
end;

procedure THistoryFrm.AddEventToSessions(hDBEvent: THandle);
var
  ts: DWord;
  dt: TDateTime;
  idx: Integer;
  year, month, day: TTreeNode;
  AddNewSession: Boolean;
begin
  ts := GetEventTimestamp(hDBEvent);
  AddNewSession := True;
  if Length(Sessions) > 0 then
  begin
    idx := High(Sessions);
    if (ts - Sessions[idx].TimestampLast) <= SESSION_TIMEDIFF then
    begin
      Sessions[idx].hDBEventLast := hDBEvent;
      Sessions[idx].TimestampLast := ts;
      Inc(Sessions[idx].ItemsCount);
      AddNewSession := False;
    end;
  end;
  if AddNewSession then
  begin
    idx := Length(Sessions);
    SetLength(Sessions, idx + 1);
    Sessions[idx].hDBEventFirst := hDBEvent;
    Sessions[idx].TimestampFirst := ts;
    Sessions[idx].hDBEventLast := Sessions[idx].hDBEventFirst;
    Sessions[idx].TimestampLast := Sessions[idx].TimestampFirst;
    Sessions[idx].ItemsCount := 1;

    dt := TimestampToDateTime(ts);
    year := nil;
    if tvSess.Items.GetFirstNode <> nil then
    begin
      year := tvSess.Items.GetFirstNode;
      while year.getNextSibling <> nil do
        year := year.getNextSibling;
      if int_ptr(year.Data) <> YearOf(dt) then
        year := nil;
    end;
    if year = nil then
    begin
      year := tvSess.Items.AddChild(nil, FormatDateTime(HPP_SESS_YEARFORMAT, dt));
      year.Data := Pointer(YearOf(dt));
      year.ImageIndex := 5;
      // year.SelectedIndex := year.ImageIndex;
    end;
    month := nil;
    if year.GetLastChild <> nil then
    begin
      month := year.GetLastChild;
      if int_ptr(month.Data) <> MonthOf(dt) then
        month := nil;
    end;
    if month = nil then
    begin
      month := tvSess.Items.AddChild(year, FormatDateTime(HPP_SESS_MONTHFORMAT, dt));
      month.Data := Pointer(MonthOf(dt));
      case MonthOf(dt) of
        12, 1 .. 2: month.ImageIndex := 3;
        3 .. 5:     month.ImageIndex := 4;
        6 .. 8:     month.ImageIndex := 1;
        9 .. 11:    month.ImageIndex := 2;
      end;
      // month.SelectedIndex := month.ImageIndex;
    end;
    day := tvSess.Items.AddChild(month, FormatDateTime(HPP_SESS_DAYFORMAT, dt));
    day.Data := Pointer(idx);
    day.ImageIndex := 0;
    // day.SelectedIndex := day.ImageIndex;
  end;
end;

procedure THistoryFrm.AddHistoryItem(hDBEvent: THandle);
// only add single lines, not whole histories, because this routine is pretty
// slow
begin
  Inc(HistoryLength);
  SetLength(History, HistoryLength);
  History[HistoryLength - 1] := hDBEvent;
  hg.AddItem;
  if HistoryLength = 1 then
    if GetDBBool(hppDBName, 'ShowSessions', False) and not(hpSessions in Panel) then
      Panel := Panel + [hpSessions];
end;

procedure THistoryFrm.hgItemData(Sender: TObject; Index: Integer; var Item: THistoryItem);
var
  PrevTimestamp: DWord;
  PrevMessageType: TMessageTypes;
  HistoryIndex: Integer;
begin
  HistoryIndex := GridIndexToHistory(Index);
  Item := GetItemData(HistoryIndex);
  if hContact = 0 then
    Item.Proto := Item.Module
  else
    Item.Proto := Protocol;
  Item.Bookmarked := BookmarkServer[hContact].Bookmarked[History[HistoryIndex]];
  if HistoryIndex = 0 then
    Item.HasHeader := IsEventInSession(Item.EventType)
  else
  begin
    if History[HistoryIndex - 1] = 0 then
      LoadPendingHeaders(HistoryIndex - 1, HistoryLength);
    PrevTimestamp := GetEventTimestamp(History[HistoryIndex - 1]);
    if IsEventInSession(Item.EventType) then
      Item.HasHeader := ((DWord(Item.Time) - PrevTimestamp) > SESSION_TIMEDIFF);
    if not Item.Bookmarked then
    begin
      PrevMessageType := GetEventMessageType(History[HistoryIndex - 1]);
      if Item.MessageType = PrevMessageType then
        Item.LinkedToPrev := ((DWord(Item.Time) - PrevTimestamp) < 60);
    end;
  end;
end;

procedure THistoryFrm.hgTranslateTime(Sender: TObject; Time: Cardinal; var Text: String);
begin
  Text := TimestampToString(Time);
end;

procedure THistoryFrm.hgPopup(Sender: TObject);
begin
  Delete1.Visible := False;
  SaveSelected1.Visible := False;
  if hContact = 0 then
  begin
    SendMessage1.Visible := False;
    ReplyQuoted1.Visible := False;
  end;
  if hg.Selected <> -1 then
  begin
    Delete1.Visible := True;
    if GridOptions.OpenDetailsMode then
      Details1.Caption := TranslateW('&Pseudo-edit')
    else
      Details1.Caption := TranslateW('&Open');
    SaveSelected1.Visible := (hg.SelCount > 1);
    FileActions.Visible := IsFileEvent(hg.Selected);
    if FileActions.Visible then
      OpenFileFolder.Visible := (SavedFileDir <> '');
    pmGrid.Popup(Mouse.CursorPos.X, Mouse.CursorPos.Y);
  end;
end;

procedure THistoryFrm.hgSearchFinished(Sender: TObject; const Text: String; Found: Boolean);
var
  t: String;
begin
  if LastSearch <> lsHotSearch then
    LastHotIdx := hg.Selected;
  LastSearch := lsHotSearch;
  if Text = '' then
  begin
    if (LastHotIdx <> -1) and (HotString <> '') then
      hg.Selected := LastHotIdx;
    LastSearch := lsNone;
    HotString := Text;
    hgState(Self, gsIdle);
    Exit;
  end;
  HotString := Text;
  {
    if Found then t := 'Search: "'+Text+'" (Ctrl+Enter to search again)'
    else t := 'Search: "'+Text+'" (not found)';
    sb.SimpleText := t;
  }

  if not Found then
    t := HotString
  else
    t := Text;
  sb.SimpleText := Format(TranslateW('HotSearch: %s (F3 to find next)'), [t]);
  // if Found then HotString := Text;
end;

procedure THistoryFrm.hgBookmarkClick(Sender: TObject; Item: Integer);
var
  val: Boolean;
  hDBEvent: THandle;
begin
  hDBEvent := History[GridIndexToHistory(Item)];
  val := not BookmarkServer[hContact].Bookmarked[hDBEvent];
  BookmarkServer[hContact].Bookmarked[hDBEvent] := val;
end;

procedure THistoryFrm.hgChar(Sender: TObject; var achar: WideChar; Shift: TShiftState);
var
  Mes: TWMChar;
begin
  if SearchMode = smNone then
    SearchMode := smSearch;
  edSearch.SetFocus;
  edSearch.SelStart := Length(edSearch.Text);
  edSearch.SelLength := 0;
  // edSearch.Text := AnsiChar;
  ZeroMemory(@Mes, SizeOf(Mes));
  Mes.Msg := WM_CHAR;
  Mes.CharCode := Word(achar);
  Mes.KeyData := ShiftStateToKeyData(Shift);
  edSearch.Perform(WM_CHAR, TMessage(Mes).wParam, TMessage(Mes).lParam);
  achar := #0;
end;

procedure THistoryFrm.hgDblClick(Sender: TObject);
begin
  if hg.Selected = -1 then
    Exit;
  if GridOptions.OpenDetailsMode then
    OpenDetails(hg.Selected)
  else
    hg.EditInline(hg.Selected);
end;

procedure THistoryFrm.tbSaveClick(Sender: TObject);
var
  t: String;
  SaveFormat: TSaveFormat;
begin
  if hg.Selected = -1 then
    Exit;
  RecentFormat := TSaveFormat(GetDBInt(hppDBName, 'ExportFormat', 0));
  SaveFormat := RecentFormat;
  PrepareSaveDialog(SaveDialog, SaveFormat, True);
  t := TranslateW('Partial History [%s] - [%s]');
  t := Format(t, [hg.ProfileName, hg.ContactName]);
  t := MakeFileName(t);
  // t := t + SaveDialog.DefaultExt;
  SaveDialog.FileName := t;
  if not SaveDialog.Execute then
    Exit;
  // why SaveDialog.FileName shows '' here???
  // who knows? In debugger FFileName shows right file, but
  // FileName property returns ''
  for SaveFormat := High(SaveFormats) downto Low(SaveFormats) do
    if SaveDialog.FilterIndex = SaveFormats[SaveFormat].Index then
      break;
  if SaveFormat <> sfAll then
    RecentFormat := SaveFormat;
  // hg.SaveSelected(SaveDialog.FileName,RecentFormat);
  hg.SaveSelected(SaveDialog.Files[0], RecentFormat);
  WriteDBInt(hppDBName, 'ExportFormat', Integer(RecentFormat));
end;

procedure THistoryFrm.sbCloseBookClick(Sender: TObject);
begin
  Panel := Panel - [hpBookmarks]
end;

procedure THistoryFrm.sbCloseSessClick(Sender: TObject);
begin
  Panel := Panel - [hpSessions]
end;

procedure THistoryFrm.sbSearchNextClick(Sender: TObject);
begin
  Search(True, True);
end;

procedure THistoryFrm.sbSearchPrevClick(Sender: TObject);
begin
  Search(False, True);
end;

procedure THistoryFrm.hgItemDelete(Sender: TObject; Index: Integer);
var
  idx: Integer;
  hDBEvent: THANDLE;
begin
  if Index = -1 then
  begin // routine is called from DeleteAll
    if FormState = gsDelete then
    begin // probably unnecessary considering prev check
      hDBEvent := db_event_first(hContact);
      db_event_delete(hContact, hDBEvent);
    end;
  end
  else
  begin
    idx := GridIndexToHistory(Index);
    if (FormState = gsDelete) and (History[idx] <> 0) then
      db_event_delete(hContact, History[idx]);
    DeleteEventFromSessions(idx);
    DeleteHistoryItem(idx);
  end;
  hgState(hg, hg.State);
  Application.ProcessMessages;
end;

procedure THistoryFrm.hgItemFilter(Sender: TObject; Index: Integer; var Show: Boolean);
begin

  // if we have AnsiString filter
  if HotFilterString <> '' then
  begin
    if Pos(WideUpperCase(HotFilterString), WideUpperCase(hg.Items[Index].Text)) = 0 then
      Show := False;
    Exit;
  end;

  // if filter by sessions disabled, then exit
  if StartTimestamp <> 0 then
  begin
    // Show := False;
    if hg.Items[Index].Time >= StartTimestamp then
    begin
      if EndTimestamp = 0 then
        Exit
      else
      begin
        if hg.Items[Index].Time < EndTimestamp then
          Exit
        else
          Show := False;
      end;
    end
    else
      Show := False;
  end;
end;

procedure THistoryFrm.tbDeleteClick(Sender: TObject);
begin
  if hg.SelCount = 0 then
    Exit;
  if hg.SelCount > 1 then
  begin
    if HppMessageBox(Handle,
      WideFormat(TranslateW('Do you really want to delete selected items (%.0f)?'),
      [hg.SelCount / 1]), TranslateW('Delete Selected'), MB_YESNOCANCEL or MB_DEFBUTTON1 or
      MB_ICONQUESTION) <> IDYES then
      Exit;
  end
  else
  begin
    if HppMessageBox(Handle, TranslateW('Do you really want to delete selected item?'),
      TranslateW('Delete'), MB_YESNOCANCEL or MB_DEFBUTTON1 or MB_ICONQUESTION) <> IDYES then
      Exit;
  end;

  if hg.SelCount = hg.count then
    EmptyHistory
  else
  begin
    SetSafetyMode(False);
    try
      FormState := gsDelete;
      hg.DeleteSelected;
    finally
      FormState := gsIdle;
      SetSafetyMode(True);
    end;
  end;
end;

function THistoryFrm.GridIndexToHistory(Index: Integer): Integer;
begin
  Result := HistoryLength - 1 - Index;
end;

function THistoryFrm.HistoryIndexToGrid(Index: Integer): Integer;
begin
  Result := HistoryLength - 1 - Index;
end;

procedure THistoryFrm.mmHideMenuClick(Sender: TObject);
begin
  WriteDBBool(hppDBName, 'Accessability', False);
  NotifyAllForms(HM_NOTF_ACCCHANGED, WPARAM(False), 0);
end;

procedure THistoryFrm.tbCopyClick(Sender: TObject);
begin
  if hg.Selected = -1 then
    Exit;
  CopyToClip(hg.FormatSelected(GridOptions.ClipCopyFormat), Handle, UserCodepage);
end;

procedure THistoryFrm.Details1Click(Sender: TObject);
begin
  if hg.Selected = -1 then
    Exit;
  if GridOptions.OpenDetailsMode then
    hg.EditInline(hg.Selected)
  else
    OpenDetails(hg.Selected);
end;

procedure THistoryFrm.OpenDetails(Item: Integer);
begin
  if not Assigned(EventDetailForm) then
  begin
    EventDetailForm := TEventDetailsFrm.Create(Self);
    TEventDetailsFrm(EventDetailForm).ParentForm := Self;
    TEventDetailsFrm(EventDetailForm).Item := Item;
    TEventDetailsFrm(EventDetailForm).Show;
  end
  else
  begin
    TEventDetailsFrm(EventDetailForm).Item := Item;
    TEventDetailsFrm(EventDetailForm).Show;
  end;
end;

function THistoryFrm.GetItemData(Index: Integer): THistoryItem;
var
  hDBEvent: THandle;
begin
  hDBEvent := History[Index];
  if hDBEvent = 0 then
  begin
    LoadPendingHeaders(Index, HistoryLength);
    hDBEvent := History[Index];
    if hDBEvent = 0 then
      raise EAbort.Create('can''t load event');
  end;
  Result := ReadEvent(hDBEvent, UserCodepage);
{$IFDEF DEBUG}
  OutPutDebugString(PWideChar('Get item data from DB ' + intToStr(Index) + ' #' + intToStr(hDBEvent)));
{$ENDIF}
end;

var
  WasReturnPressed: Boolean = False;

procedure THistoryFrm.hgKeyDown(Sender: TObject; var Key: Word; Shift: TShiftState);
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
    Exit;
  end;

  WasReturnPressed := (Key = VK_RETURN);
end;

procedure THistoryFrm.hgState(Sender: TObject; State: TGridState);
var
  t: String;
begin
  if csDestroying in ComponentState then
    Exit;
  case State of
    gsIdle:
      t := Format(TranslateW('%.0n items in history'), [HistoryLength / 1]);
    gsLoad:
      t := TranslateW('Loading...');
    gsSave:
      t := TranslateW('Saving...');
    gsSearch:
      t := TranslateW('Searching...');
    gsDelete:
      t := TranslateW('Deleting...');
    gsInline:
      t := TranslateW('Pseudo-edit mode...');
  end;
  sb.SimpleText := t;
end;

procedure THistoryFrm.DeleteEventFromSessions(ItemIdx: Integer);
var
  ts: DWord;
  dt: TDateTime;
  year, month, day: TTreeNode;
  i, idx: Integer;
  hDBEvent: THandle;
begin
  hDBEvent := History[ItemIdx];
  ts := GetEventTimestamp(hDBEvent);

  // find idx in sessions array
  idx := -1;
  for i := Length(Sessions) - 1 downto 0 do
    if (ts >= Sessions[i].TimestampFirst) and (ts <= Sessions[i].TimestampLast) then
    begin
      idx := i;
      break;
    end;
  if idx = -1 then
    Exit;

  dec(Sessions[idx].ItemsCount);

  // if the event is not first, we can do it faster
  if Sessions[idx].hDBEventFirst <> hDBEvent then
  begin
    if Sessions[idx].hDBEventLast = hDBEvent then
    begin
      hDBEvent := db_event_prev(hContact,hDBEvent);
      if hDBEvent <> 0 then
      begin
        Sessions[idx].hDBEventLast := hDBEvent;
        Sessions[idx].TimestampLast := GetEventTimestamp(hDBEvent);
      end
      else
      begin // ????
        Sessions[idx].hDBEventLast := Sessions[idx].hDBEventFirst;
        Sessions[idx].TimestampLast := Sessions[idx].TimestampFirst;
      end;
    end;
    Exit;
  end;

  // now, the even is the first, probably the last in session
  dt := TimestampToDateTime(ts);
  year := tvSess.Items.GetFirstNode;
  while year <> nil do
  begin
    if int_ptr(year.Data) = YearOf(dt) then
      break;
    year := year.getNextSibling;
  end;
  if year = nil then
    Exit; // ???
  month := year.getFirstChild;
  while month <> nil do
  begin
    if int_ptr(month.Data) = MonthOf(dt) then
      break;
    month := month.getNextSibling;
  end;
  if month = nil then
    Exit; // ???
  day := month.getFirstChild;
  while day <> nil do
  begin
    if int_ptr(day.Data) = idx then
      break;
    day := day.getNextSibling;
  end;
  if day = nil then
    Exit; // ???
  if Sessions[idx].ItemsCount = 0 then
  begin
    day.Delete;
    if month.count = 0 then
      month.Delete;
    if year.count = 0 then
      year.Delete;
    // hmm... should we delete record in sessions array?
    // I'll not do it for the speed, I don't think there would be problems
    Sessions[idx].hDBEventFirst := 0;
    Sessions[idx].TimestampFirst := 0;
    Sessions[idx].hDBEventLast := 0;
    Sessions[idx].TimestampLast := 0;
    Exit;
  end;
  hDBEvent := db_event_next(hContact,hDBEvent);
  if hDBEvent <> 0 then
  begin
    Sessions[idx].hDBEventFirst := hDBEvent;
    Sessions[idx].TimestampFirst := GetEventTimestamp(hDBEvent);
  end
  else
  begin // ????
    Sessions[idx].hDBEventFirst := Sessions[idx].hDBEventLast;
    Sessions[idx].TimestampFirst := Sessions[idx].TimestampLast;
  end;
  ts := Sessions[idx].TimestampFirst;
  dt := TimestampToDateTime(ts);
  day.Text := FormatDateTime(HPP_SESS_DAYFORMAT, dt);
  // next item
  // Inc(ItemIdx);
  // if ItemIdx >= HistoryLength then exit;
  // hg.ResetItem(HistoryIndexToGrid(ItemIdx));
end;

procedure THistoryFrm.SaveasHTML2Click(Sender: TObject);
var
  t: String;
begin
  PrepareSaveDialog(SaveDialog, sfHtml);
  t := Format(TranslateW('Full History [%s] - [%s]'), [hg.ProfileName, hg.ContactName]);
  t := MakeFileName(t);
  SaveDialog.FileName := t;
  if not SaveDialog.Execute then
    Exit;
  RecentFormat := sfHtml;
  hg.SaveAll(SaveDialog.Files[0], sfHtml);
  WriteDBInt(hppDBName, 'ExportFormat', Integer(RecentFormat));
end;

procedure THistoryFrm.WMGetMinMaxInfo(var Message: TWMGetMinMaxInfo);
begin
  inherited;
  with Message.MinMaxInfo^ do
  begin
    ptMinTrackSize.X := 320;
    ptMinTrackSize.Y := 240;
  end
end;

procedure THistoryFrm.FormMouseWheel(Sender: TObject; Shift: TShiftState; WheelDelta: Integer;
  MousePos: TPoint; var Handled: Boolean);
begin
  Handled := True;
  (* we can get range check error (???) here
    it looks that without range check it works ok
    so turn it off *)
{$RANGECHECKS OFF}
  hg.Perform(WM_MOUSEWHEEL, MakeLong(MK_CONTROL, WheelDelta), 0);
{$RANGECHECKS ON}
end;

procedure THistoryFrm.hgSelect(Sender: TObject; Item, OldItem: Integer);
begin
  tbCopy.Enabled := (Item <> -1);
  tbDelete.Enabled := (Item <> -1);
  tbSave.Enabled := (hg.SelCount > 1);

  if hg.HotString = '' then
  begin
    LastHotIdx := -1;
    // redraw status bar
    hgState(hg, gsIdle);
  end;
end;

procedure THistoryFrm.Search(Next, FromNext: Boolean);
var
  Down: Boolean;
  Item: Integer;
  ShowEndOfPage: Boolean;
  ShowNotFound: Boolean;
begin
  if edSearch.Text = '' then
  begin
    paSearchStatus.Visible := False;
    edSearch.Color := clWindow;
    Exit;
  end;
  if Next then
    Down := not hg.Reversed
  else
    Down := hg.Reversed;
  Item := hg.Search(edSearch.Text, False, False, False, FromNext, Down);
  ShowEndOfPage := (Item = -1);
  if Item = -1 then
    Item := hg.Search(edSearch.Text, False, True, False, FromNext, Down);
  if Item <> -1 then
  begin
    hg.Selected := Item;
    edSearch.Color := clWindow;
    ShowNotFound := False;
  end
  else
  begin
    edSearch.Color := $008080FF;
    ShowEndOfPage := False;
    ShowNotFound := True;
  end;
  if ShowNotFound or ShowEndOfPage then
  begin
    imSearchNotFound.Visible := ShowNotFound;
    imSearchEndOfPage.Visible := ShowEndOfPage;
    if ShowNotFound then
      laSearchState.Caption := TranslateW('Phrase not found')
    else if ShowEndOfPage then
    begin
      if Down then
        laSearchState.Caption := TranslateW('Continued from the top')
      else
        laSearchState.Caption := TranslateW('Continued from the bottom');
    end;
    paSearchStatus.Width := 22 + laSearchState.Width + 3;
    paSearchStatus.Left := paSearchButtons.Left - paSearchStatus.Width;
    paSearchStatus.Visible := True;
  end
  else
  begin
    paSearchStatus.Visible := False;
    // paSearchStatus.Width := 0;
  end;
  // paSearch2.Width := paSearchButtons.Left + paSearchButtons.Width;
end;

procedure THistoryFrm.SearchNext(Rev: Boolean; Warp: Boolean = True);
// var
// stext,t,tCap: WideString;
// res: Integer;
// mcase,down: Boolean;
// WndHandle: HWND;
begin
  { if LastSearch = lsNone then exit;
    if LastSearch = lsHotSearch then begin
    stext := HotString;
    mcase := False;
    end else begin
    stext := FindDialog.FindText;
    mcase := (frMatchCase in FindDialog.Options);
    end;
    if stext = '' then exit;
    down := not hg.reversed;
    if Rev then Down := not Down;
    res := hg.Search(stext, mcase, not Warp, False, Warp, Down);
    if res <> -1 then begin
    // found
    hg.Selected := res;
    if LastSearch = lsSearch then
    t := TranslateW('Search: %s (F3 to find next)')
    else
    t := TranslateW('HotSearch: %s (F3 to find next)');
    sb.SimpleText := WideFormat(t,[stext]);
    end else begin
    if (LastSearch = lsSearch) and (FindDialog.Handle <> 0) then
    WndHandle := FindDialog.Handle
    else
    WndHandle := Handle;
    tCap := TranslateW('History++ Search');
    // not found
    if Warp and (down = not hg.Reversed) then begin
    // do warp?
    if HppMessageBox(WndHandle,
    TranslateW('You have reached the end of the history.')+#10#13+
    TranslateW('Do you want to continue searching at the beginning?'),
    tCap, MB_YESNOCANCEL or MB_DEFBUTTON1 or MB_ICONQUESTION) = ID_YES then
    SearchNext(Rev,False);
    end else begin
    // not warped
    hgState(Self,gsIdle);
    // 25.03.03 OXY: FindDialog looses focus when
    // calling ShowMessage, using MessageBox instead
    t := TranslateW('"%s" not found');
    HppMessageBox(WndHandle, WideFormat(t,[stext]),tCap, MB_OK or MB_DEFBUTTON1 or 0);
    end;
    end; }
end;

procedure THistoryFrm.ReplyQuoted(Item: Integer);
begin
  if (hContact = 0) or (hg.SelCount = 0) then
    Exit;
  SendMessageTo(hContact, hg.FormatSelected(GridOptions.ReplyQuotedFormat));
end;

procedure THistoryFrm.SaveasXML2Click(Sender: TObject);
var
  t: String;
begin
  PrepareSaveDialog(SaveDialog, sfXML);
  t := Format(TranslateW('Full History [%s] - [%s]'), [hg.ProfileName, hg.ContactName]);
  t := MakeFileName(t);
  SaveDialog.FileName := t;
  if not SaveDialog.Execute then
    Exit;
  hg.SaveAll(SaveDialog.Files[0], sfXML);
  RecentFormat := sfXML;
  WriteDBInt(hppDBName, 'ExportFormat', Integer(RecentFormat));
end;

procedure THistoryFrm.SaveasText2Click(Sender: TObject);
var
  t: String;
  SaveFormat: TSaveFormat;
begin
  SaveFormat := sfUnicode;
  PrepareSaveDialog(SaveDialog, SaveFormat);
  t := Format(TranslateW('Full History [%s] - [%s]'), [hg.ProfileName, hg.ContactName]);
  t := MakeFileName(t);
  SaveDialog.FileName := t;
  if not SaveDialog.Execute then
    Exit;
  case SaveDialog.FilterIndex of
    1: SaveFormat := sfUnicode;
    2: SaveFormat := sfText;
  end;
  RecentFormat := SaveFormat;
  hg.SaveAll(SaveDialog.Files[0], SaveFormat);
  // hg.SaveAll(SaveDialog.FileName,SaveFormat);
  WriteDBInt(hppDBName, 'ExportFormat', Integer(RecentFormat));
end;

procedure THistoryFrm.hgXMLData(Sender: TObject; Index: Integer; var Item: TXMLItem);
var
  tmp: AnsiString;
  dt: TDateTime;
  Mes: String;
begin
  dt := TimestampToDateTime(hg.Items[Index].Time);
  Item.Time := MakeTextXMLedA(AnsiString(FormatDateTime('hh:mm:ss', dt)));
  Item.Date := MakeTextXMLedA(AnsiString(FormatDateTime('yyyy-mm-dd', dt)));

  Item.Contact := UTF8Encode(MakeTextXMLedW(hg.ContactName));
  if mtIncoming in hg.Items[Index].MessageType then
    Item.From := Item.Contact
  else
    Item.From := '&ME;';

  Item.EventType := '&' + GetEventRecord(hg.Items[Index]).XML + ';';

  Mes := hg.Items[Index].Text;
  if GridOptions.RawRTFEnabled and IsRTF(Mes) then
  begin
    hg.ApplyItemToRich(Index);
    Mes := GetRichString(hg.RichEdit.Handle, False);
  end;
  if GridOptions.BBCodesEnabled then
    Mes := DoStripBBCodes(Mes);
  Item.Mes := UTF8Encode(MakeTextXMLedW(Mes));

  if mtFile in hg.Items[Index].MessageType then
  begin
    tmp := hg.Items[Index].Extended;
    if tmp = '' then
      Item.FileName := '&UNK;'
    else
      Item.FileName := UTF8Encode(MakeTextXMLedA(tmp));
  end
  else if mtUrl in hg.Items[Index].MessageType then
  begin
    tmp := hg.Items[Index].Extended;
    if tmp = '' then
      Item.Url := '&UNK;'
    else
      Item.Url := UTF8Encode(MakeTextXMLedA(tmp));
  end
  else if mtAvatarChange in hg.Items[Index].MessageType then
  begin
    tmp := hg.Items[Index].Extended;
    if tmp = '' then
      Item.FileName := '&UNK;'
    else
      Item.FileName := UTF8Encode(MakeTextXMLedA(tmp));
  end;

  { 2.8.2004 OXY: Change protocol guessing order. Now
    first use protocol name, then, if missing, use module }

  Item.Protocol := hg.Items[Index].Proto;
  if Item.Protocol = '' then
    Item.Protocol := MakeTextXMLedA(hg.Items[Index].Module);
  if Item.Protocol = '' then
    Item.Protocol := '&UNK;';

  if mtIncoming in hg.Items[Index].MessageType then
    Item.ID := GetContactID(hContact, Protocol, True)
  else
    Item.ID := GetContactID(0, SubProtocol);
  if Item.ID = '' then
    Item.ID := '&UNK;'
  else
    Item.ID := MakeTextXMLedA(Item.ID);
end;

procedure THistoryFrm.OpenLinkClick(Sender: TObject);
begin
  if SavedLinkUrl = '' then
    Exit;
  OpenUrl(SavedLinkUrl, False);
  SavedLinkUrl := '';
end;

procedure THistoryFrm.OpenLinkNWClick(Sender: TObject);
begin
  if SavedLinkUrl = '' then
    Exit;
  OpenUrl(SavedLinkUrl, True);
  SavedLinkUrl := '';
end;

procedure THistoryFrm.CopyLinkClick(Sender: TObject);
begin
  if SavedLinkUrl = '' then
    Exit;
  CopyToClip(SavedLinkUrl, Handle, CP_ACP);
  SavedLinkUrl := '';
end;

procedure THistoryFrm.SetPanel(const Value: THistoryPanels);
var
  Lock: Boolean;
begin
  FPanel := Value;
  if (HistoryLength = 0) or ((hContact = 0) and (hpSessions in FPanel)) then
    exclude(FPanel, hpSessions);
  tbSessions.Down := (hpSessions in Panel);
  tbBookmarks.Down := (hpBookmarks in Panel);
  hg.BeginUpdate;
  Lock := Visible;
  if Lock then
    Lock := LockWindowUpdate(Handle);
  try
    // if (FPanel = hpBookmarks) and paSess.Visible then
    // paBook.Width := paSess.Width;
    // if (FPanel = hpSessions) and paBook.Visible then
    // paSess.Width := paBook.Width;

    paSess.Visible := (hpSessions in Panel);
    paBook.Visible := (hpBookmarks in Panel);

    paHolder.Visible := paBook.Visible or paSess.Visible;
    spHolder.Visible := paHolder.Visible;
    spHolder.Left := paHolder.Left + paHolder.Width + 1;

    spBook.Visible := paBook.Visible and paSess.Visible;
    paHolderResize(Self);
    spBook.Top := paSess.Top + paSess.Height + 1;

  finally
    if Lock then
      LockWindowUpdate(0);
    hg.EndUpdate;
  end;
end;

procedure THistoryFrm.SetRecentEventsPosition(OnTop: Boolean);
begin
  hg.Reversed := not OnTop;
  LoadButtonIcons;
end;

procedure THistoryFrm.SetSearchMode(const Value: TSearchMode);
var
  SaveStr: String;
  NotFound, Lock: Boolean;
begin
  if FSearchMode = Value then
    Exit;

  if Value = smHotSearch then
    PreHotSearchMode := FSearchMode;
  if FSearchMode = smFilter then
    EndHotFilterTimer(True);

  FSearchMode := Value;

  Lock := Visible;
  if Lock then
    Lock := LockWindowUpdate(Handle);
  try
    tbFilter.Down := (FSearchMode = smFilter);
    tbSearch.Down := (FSearchMode = smSearch);
    paSearch.Visible := not(SearchMode = smNone);
    if SearchMode = smNone then
    begin
      edSearch.Text := '';
      edSearch.Color := clWindow;
      if Self.Visible then
        hg.SetFocus
      else
        Self.ActiveControl := hg;
      Exit;
    end;
    SaveStr := edSearch.Text;
    hg.BeginUpdate;
    try
      pbSearch.Visible := (FSearchMode in [smSearch, smHotSearch]);
      pbFilter.Visible := (FSearchMode = smFilter);
      if (FSearchMode = smFilter) then
        paSearchStatus.Visible := False;
      paSearchButtons.Visible := not(FSearchMode = smFilter);
      NotFound := not(edSearch.Color = clWindow);
      edSearch.Text := '';
      edSearch.Color := clWindow;
    finally
      hg.EndUpdate;
    end;
    // don't search or filter if the AnsiString is not found
    if not NotFound then
      edSearch.Text := SaveStr;
  finally
    if Lock then
      LockWindowUpdate(0);
  end;
end;

procedure THistoryFrm.EventsFilterItemClick(Sender: TObject);
begin
  // tbEventsFilter.Caption := TMenuItem(Sender).Caption;
  SetEventFilter(TMenuItem(Sender).Tag);
end;

procedure THistoryFrm.ShowAllEvents;
begin
  // TODO
  // we run UpdateFilter two times here, one when set
  // Filter property in SetEventFilter, one when reset hot filter
  // make Begin/EndUpdate support batch UpdateFilter requests
  // so we can make it run only one time on EndUpdate
  hg.BeginUpdate;
  SetEventFilter(GetShowAllEventsIndex);
  edSearch.Text := '';
  EndHotFilterTimer(True);
  hg.EndUpdate;
end;

procedure THistoryFrm.ShowItem(Value: Integer);
begin
  hg.MakeTopmost(Value);
  hg.Selected := Value;
end;

procedure THistoryFrm.edSearchChange(Sender: TObject);
begin
  if SearchMode = smFilter then
    StartHotFilterTimer
  else
    Search(True, False);
end;

procedure THistoryFrm.edSearchKeyDown(Sender: TObject; var Key: Word; Shift: TShiftState);
begin
  if SearchMode = smFilter then
  begin
    if Key in [VK_UP, VK_DOWN, VK_NEXT, VK_PRIOR, VK_END, VK_HOME] then
    begin
      SendMessage(hg.Handle, WM_KEYDOWN, Key, 0);
      Key := 0;
    end;
  end
  else
  begin
    if (Shift = []) and (Key in [VK_UP, VK_DOWN, VK_NEXT, VK_PRIOR, VK_END, VK_HOME]) then
    begin
      SendMessage(hg.Handle, WM_KEYDOWN, Key, 0);
      Key := 0;
      Exit;
    end;
    if (Shift = [ssCtrl]) and (Key in [VK_UP, VK_DOWN]) then
    begin
      if (Key = VK_UP) xor hg.Reversed then
        sbSearchNext.Click
      else
        sbSearchPrev.Click;
      Key := 0;
      Exit;
    end;
  end;
end;

procedure THistoryFrm.edSearchKeyUp(Sender: TObject; var Key: Word; Shift: TShiftState);
begin
  // to prevent ** BLING ** when press Enter
  if (Key = VK_RETURN) then
  begin
    if hg.State in [gsIdle, gsInline] then
      hg.SetFocus;
    Key := 0;
  end;
end;

procedure THistoryFrm.PostLoadHistory;
var
  tPanel: THistoryPanels;
begin
  LoadPosition;
  ToggleMainMenu(GetDBBool(hppDBName, 'Accessability', False));

  // if hContact = 0 then paTop.Visible := False;
  // set reversed here, after Allocate, because of some scrollbar
  // "features", we'll load end of the list if put before Allocate
  SetRecentEventsPosition(GetDBBool(hppDBName, 'SortOrder', False));
  // set ShowSessions here because we check for empty history
  paHolder.Width := GetDBInt(hppDBName, 'PanelWidth', 150);
  spBook.Tag := GetDBByte(hppDBName, 'PanelSplit', 127);
  if hContact = 0 then
  begin
    if GetDBBool(hppDBName, 'ShowBookmarksSystem', False) then
      Panel := [hpBookmarks];
  end
  else
  begin
    if GetDBBool(hppDBName, 'ShowSessions', False) then
      include(tPanel, hpSessions);
    if GetDBBool(hppDBName, 'ShowBookmarks', False) then
      include(tPanel, hpBookmarks);
    Panel := tPanel;
  end;

  CreateEventsFilterMenu;
  // delay event filter applying till showing form
  if hContact = 0 then
    SetEventFilter(GetShowAllEventsIndex, True)
  else
    SetEventFilter(0, True);
end;

procedure THistoryFrm.PreLoadHistory;
begin
  // LoadPosition;
  hg.ShowHeaders := (hContact <> 0);
  hg.ExpandHeaders := GetDBBool(hppDBName, 'ExpandHeaders', False);
  hg.GroupLinked := GetDBBool(hppDBName, 'GroupHistoryItems', False);
  if hContact = 0 then
  begin
    tbUserDetails.Enabled := False;
    tbUserMenu.Enabled := False;
    // tbEventsFilter.Enabled := False;
    tbSessions.Enabled := False;
    // hg.ShowBookmarks := False;
    Customize2.Enabled := False; // disable toolbar customization
  end;

  if tbSessions.Enabled then
  begin
    SessThread := TSessionsThread.Create(True);
    SessThread.ParentHandle := Handle;
    SessThread.Contact := hContact;
    SessThread.Priority := tpLower;
    SessThread.Resume;
  end;

end;

procedure THistoryFrm.FormShow(Sender: TObject);
begin
  // EndUpdate is better here, not in PostHistoryLoad, because it's faster
  // when called from OnShow. Don't know why.
  // Other form-modifying routines are better be kept at PostHistoryLoad for
  // speed too.
  hg.EndUpdate;
  LoadToolbar;
  FillBookmarks;
end;

procedure THistoryFrm.mmToolbarClick(Sender: TObject);
var
  i, n: Integer;
  pm: TPopupMenu;
  mi: TMenuItem;
  Flag: Boolean;
begin
  for i := 0 to mmToolbar.count - 1 do
  begin
    if mmToolbar.Items[i].Owner is THppToolButton then
    begin
      Flag := TToolButton(mmToolbar.Items[i].Owner).Enabled
    end
    else if mmToolbar.Items[i].Owner is TSpeedButton then
    begin
      TMenuItem(mmToolbar.Items[i]).Caption := TSpeedButton(mmToolbar.Items[i].Owner).Hint;
      Flag := TSpeedButton(mmToolbar.Items[i].Owner).Enabled
    end
    else
      Flag := True;
    mmToolbar.Items[i].Enabled := Flag;
    if mmToolbar.Items[i].Tag = 0 then
      continue;
    pm := TPopupMenu(Pointer(mmToolbar.Items[i].Tag));
    for n := pm.Items.count - 1 downto 0 do
    begin
      mi := TMenuItem(pm.Items[n]);
      pm.Items.Remove(mi);
      mmToolbar.Items[i].Insert(0, mi);
    end;
  end;
end;

procedure THistoryFrm.ToolbarDblClick(Sender: TObject);
begin
  CustomizeToolbar;
end;

procedure THistoryFrm.TranslateForm;
begin
  Caption := TranslateUnicodeString(Caption);

  hg.TxtFullLog    := TranslateUnicodeString(hg.TxtFullLog);
  hg.TxtGenHist1   := TranslateUnicodeString(hg.TxtGenHist1);
  hg.TxtGenHist2   := TranslateUnicodeString(hg.TxtGenHist2);
  hg.TxtHistExport := TranslateUnicodeString(hg.TxtHistExport);
  hg.TxtNoItems    := TranslateUnicodeString(hg.TxtNoItems);
  hg.TxtNoSuch     := TranslateUnicodeString(hg.TxtNoSuch);
  hg.TxtPartLog    := TranslateUnicodeString(hg.TxtPartLog);
  hg.TxtStartUp    := TranslateUnicodeString(hg.TxtStartUp);
  hg.TxtSessions   := TranslateUnicodeString(hg.TxtSessions);

  SearchUpHint := TranslateUnicodeString(SearchUpHint);
  SearchDownHint := TranslateUnicodeString(SearchDownHint);

  sbClearFilter.Hint := TranslateUnicodeString(sbClearFilter.Hint);

  laSess.Caption  := TranslateUnicodeString(laSess.Caption);
  laBook.Caption  := TranslateUnicodeString(laBook.Caption);

  SaveDialog.Title := TranslateUnicodeString(PWideChar(SaveDialog.Title));

  TranslateToolbar(Toolbar);

  TranslateMenu(pmGrid.Items);
  TranslateMenu(pmInline.Items);

  TranslateMenu(pmLink.Items);
  TranslateMenu(pmFile.Items);
  TranslateMenu(pmHistory.Items);
  TranslateMenu(pmEventsFilter.Items);
  TranslateMenu(pmSessions.Items);
  TranslateMenu(pmToolbar.Items);
  TranslateMenu(pmBook.Items);
end;

procedure THistoryFrm.tvSessChange(Sender: TObject; Node: TTreeNode);
var
  Index, i: Integer;
  Event: THandle;
begin
  if IsLoadingSessions then
    Exit;
  if Node = nil then
    Exit;
  if Node.Level <> 2 then
  begin
    Node := Node.getFirstChild;
    if (Node <> nil) and (Node.Level <> 2) then
      Node := Node.getFirstChild;
    if Node = nil then
      Exit;
  end;

  Event := Sessions[uint_ptr(Node.Data)].hDBEventFirst;
  Index := -1;
  // looks like history starts to load from end?
  // well, of course, we load from the last event!
  for i := HistoryLength - 1 downto 0 do
  begin
    if History[i] = 0 then
      LoadPendingHeaders(i, HistoryLength);
    if History[i] = Event then
    begin
      Index := i;
      break;
    end;
  end;
  if Index = -1 then
    Exit;
  if hg.State = gsInline then
    hg.CancelInline;
  Index := HistoryIndexToGrid(Index);
  ShowItem(Index);

  // exit;
  // OXY: try to make selected item the topmost
  // while hg.GetFirstVisible <> Index do begin
  // if hg.VertScrollBar.Position = hg.VertScrollBar.Range then break;
  // hg.VertScrollBar.Position := hg.VertScrollBar.Position + 1;
  // end;

  { if Node = nil then begin
    StartTimestamp := 0;
    EndTimestamp := 0;
    hg.GridUpdate([guFilter]);
    exit;
    end;

    if Node.Level <> 2 then exit;

    StartTimestamp := Sessions[uint_ptr(Node.Data)][1];
    EndTimestamp := 0;
    if uint_ptr(Node.Data) <= Length(Sessions)-2 then begin
    EndTimestamp := Sessions[uint_ptr(Node.Data)+1][1];
    end;
    hg.GridUpdate([guFilter]); }
end;

{ procedure THistoryFrm.tvSessClick(Sender: TObject);
  var
  Node: TTreeNode;
  begin
  Node := tvSess.Selected;
  if Node = nil then exit;
  //tvSessChange(Self,Node);
  end; }

procedure THistoryFrm.tvSessMouseMove(Sender: TObject; Shift: TShiftState; X, Y: Integer);
var
  Node: TTreeNode;
  count, Time: DWord;
  t: String;
  dt: TDateTime;
  timestr: String;
begin
  Node := tvSess.GetNodeAt(X, Y);
  if (Node = nil) or (Node.Level <> 2) then
  begin
    Application.CancelHint;
    tvSess.ShowHint := False;
    Exit;
  end;
  if uint_ptr(tvSess.Tag) <> (uint_ptr(Node.Data) + 1) then
  begin
    Application.CancelHint;
    tvSess.ShowHint := False;
    tvSess.Tag := uint_ptr(Node.Data) + 1;
    // +1 because we have tag = 0 by default, and it will not catch first session then
  end;
  // else
  // exit; // we are already showing the hint for this node

  with Sessions[uint_ptr(Node.Data)] do
  begin
    Time  := TimestampLast - TimestampFirst;
    count := ItemsCount;
    dt    := TimestampToDateTime(TimestampFirst);
  end;

  t := FormatDateTime('[yyyy, mmmm, d]', dt) + #13#10;
  if Time / 60 > 60 then
    timestr := Format(TranslateW('%0.1n h'), [Time / (60 * 60)])
  else
    timestr := Format(TranslateW('%d min'), [Time div 60]);

  if count = 1 then
    tvSess.Hint := t + Format('' + TranslateW('%d event'), [count])
  else
    tvSess.Hint := t + Format('' + TranslateW('%0.n events (%s)'), [count / 1, timestr]);
  tvSess.ShowHint := True;
end;

procedure THistoryFrm.CopyText1Click(Sender: TObject);
begin
  if hg.Selected = -1 then
    Exit;
  CopyToClip(hg.FormatSelected(GridOptions.ClipCopyTextFormat), Handle, UserCodepage);
  // rtf copy works only if not more then one selected
  // hg.ApplyItemToRich(hg.Selected,hg.RichEdit,False);
  // hg.RichEdit.SelectAll;
  // hg.RichEdit.CopyToClipboard;
end;

procedure THistoryFrm.CreateEventsFilterMenu;
var
  i: Integer;
  mi: TMenuItem;
  ShowAllEventsIndex: Integer;
begin
  for i := pmEventsFilter.Items.count - 1 downto 0 do
    if pmEventsFilter.Items[i].RadioItem then
      pmEventsFilter.Items.Delete(i);

  ShowAllEventsIndex := GetShowAllEventsIndex;
  for i := 0 to Length(hppEventFilters) - 1 do
  begin
    mi := TMenuItem.Create(pmEventsFilter);
    mi.Caption := StringReplace(hppEventFilters[i].name, '&', '&&', [rfReplaceAll]);
    mi.GroupIndex := 1;
    mi.RadioItem := True;
    mi.Tag := i;
    mi.OnClick := EventsFilterItemClick;
    if i = ShowAllEventsIndex then
      mi.Default := True;
    pmEventsFilter.Items.Insert(i, mi);
  end;
end;

procedure THistoryFrm.Customize1Click(Sender: TObject);
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

procedure THistoryFrm.Customize2Click(Sender: TObject);
begin
  CustomizeToolbar;
end;

procedure THistoryFrm.CustomizeToolbar;
begin
  if hContact = 0 then
    Exit;

  if not Assigned(fmCustomizeToolbar) then
  begin
    CustomizeToolbarForm := TfmCustomizeToolbar.Create(Self);
    CustomizeToolbarForm.Show;
  end
  else
  begin
    BringFormToFront(fmCustomizeToolbar);
  end;
end;

procedure THistoryFrm.hgUrlClick(Sender: TObject; Item: Integer; const URLText: String; Button: TMouseButton);
begin
  if URLText = '' then
    Exit;
  if (Button = mbLeft) or (Button = mbMiddle) then
    OpenUrl(URLText, True)
  else
  begin
    SavedLinkUrl := URLText;
    pmLink.Popup(Mouse.CursorPos.X, Mouse.CursorPos.Y);
  end;
end;

procedure THistoryFrm.hgProcessRichText(Sender: TObject; Handle: THandle; Item: Integer);
var
  ItemRenderDetails: TItemRenderDetails;
begin
  if Assigned(EventDetailForm) then
    if Handle = TEventDetailsFrm(EventDetailForm).EText.Handle then
    begin
      TEventDetailsFrm(EventDetailForm).ProcessRichEdit(Item);
      Exit;
    end;
  ZeroMemory(@ItemRenderDetails, SizeOf(ItemRenderDetails));
  ItemRenderDetails.cbSize := SizeOf(ItemRenderDetails);
  // use meta's subcontact info, if available
  // ItemRenderDetails.hContact := hContact;
  ItemRenderDetails.hContact := FhSubContact;
  ItemRenderDetails.hDBEvent := History[GridIndexToHistory(Item)];
  // use meta's subcontact info, if available
  if hContact = 0 then
    ItemRenderDetails.pProto := PAnsiChar(hg.Items[Item].Proto)
  else
    ItemRenderDetails.pProto := PAnsiChar(FSubProtocol);
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
  if hContact = 0 then
    ItemRenderDetails.bHistoryWindow := IRDHW_GLOBALHISTORY
  else
    ItemRenderDetails.bHistoryWindow := IRDHW_CONTACTHISTORY;
  NotifyEventHooks(hHppRichEditItemProcess, wParam(Handle), lParam(@ItemRenderDetails));
end;

procedure THistoryFrm.hgSearchItem(Sender: TObject; Item, ID: Integer; var Found: Boolean);
begin
  Found := (Cardinal(ID) = History[GridIndexToHistory(Item)]);
end;

procedure THistoryFrm.hgKeyUp(Sender: TObject; var Key: Word; Shift: TShiftState);
begin
  if not WasReturnPressed then
    Exit;
  WasReturnPressed := False;

  if (Key = VK_RETURN) and (Shift = []) then
  begin
    hgDblClick(hg);
    Key := 0;
  end;
  if (Key = VK_RETURN) and (Shift = [ssCtrl]) then
  begin
    Details1.Click;
    Key := 0;
  end;
end;

function THistoryFrm.IsFileEvent(Index: Integer): Boolean;
begin
  Result := (Index <> -1) and (mtFile in hg.Items[Index].MessageType);
  if Result then
  begin
    // Auto CP_ACP usage
    SavedLinkUrl := ExtractFileName(String(hg.Items[Index].Extended));
    SavedFileDir := ExtractFileDir(String(hg.Items[Index].Extended));
  end;
end;

procedure THistoryFrm.LoadInOptions();
var
  i: Integer;
begin
  if hContact = 0 then
  begin
    ContactRTLmode.Visible := False;
    ANSICodepage.Visible := False;
  end
  else
  begin
    case hg.RTLMode of
      hppRTLDefault: Self.RTLDefault2.Checked := True;
      hppRTLEnable:  Self.RTLEnabled2.Checked := True;
      hppRTLDisable: Self.RTLDisabled2.Checked := True;
    end;
    if UseDefaultCP then
      SystemCodepage.Checked := True
    else
      for i := 1 to ANSICodepage.count - 1 do
        if ANSICodepage.Items[i].Tag = Integer(UserCodepage) then
        begin
          ANSICodepage.Items[i].Checked := True;
          if i > 1 then
            break;
        end;
    // no need to make it invisible if it was turned on
    if UnknownCodepage.Checked then
      UnknownCodepage.Visible := True;
  end;
end;

// use that to delay events filtering until window will be visible
procedure THistoryFrm.CMShowingChanged(var Message: TMessage);
begin
  inherited;
  if Visible and (DelayedFilter <> []) then
  begin
    hg.ShowBottomAligned := True;
    hg.Filter := DelayedFilter;
    DelayedFilter := [];
  end;
end;

procedure THistoryFrm.SetEventFilter(FilterIndex: Integer = -1; DelayApply: Boolean = False);
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
  mi := TMenuItem(Customize1.Parent);
  for i := 0 to mi.count - 1 do
    if mi[i].RadioItem then
      mi[i].Checked := (mi[i].Tag = fi);
  hg.ShowHeaders := (tbSessions.Enabled) and (mtMessage in hppEventFilters[fi].Events);

  if DelayApply then
    DelayedFilter := hppEventFilters[fi].Events
  else
  begin
    DelayedFilter := [];
    hg.Filter := hppEventFilters[fi].Events;
  end;
end;

procedure THistoryFrm.SethContact(const Value: TMCONTACT);
begin
  // if FhContact = Value then exit;
  FhContact := Value;
  if FhContact = 0 then
  begin
    FhSubContact := 0;
    FProtocol := 'ICQ';
    FSubProtocol := FProtocol;
  end
  else
  begin
    FProtocol := GetContactProto(hContact, FhSubContact, FSubProtocol);
  end;
end;

// fix for infamous splitter bug!
// thanks to Greg Chapman
// http://groups.google.com/group/borland.public.delphi.objectpascal/browse_thread/thread/218a7511123851c3/5ada76e08038a75b%235ada76e08038a75b?sa=X&oi=groupsr&start=2&num=3
procedure THistoryFrm.AlignControls(Control: TControl; var ARect: TRect);
begin
  inherited;
  if paHolder.Width = 0 then
    paHolder.Left := spHolder.Left;
  if paSess.Height = 0 then
    paSess.Top := spBook.Top;
end;

procedure THistoryFrm.ContactRTLmode1Click(Sender: TObject);
begin
  if RTLDefault2.Checked then
    hg.RTLMode := hppRTLDefault
  else
  begin
    if RTLEnabled2.Checked then
      hg.RTLMode := hppRTLEnable
    else
      hg.RTLMode := hppRTLDisable;
  end;
  WriteContactRTLMode(hContact, hg.RTLMode, Protocol);
end;

procedure THistoryFrm.SMPrepare(var M: TMessage);
begin
  if tvSess.Visible then
    tvSess.Enabled := False;
  IsLoadingSessions := True;
end;

procedure THistoryFrm.SMItemsFound(var M: TMessage);
var
  ti: TTreeNode;
  i: Integer;
  dt: TDateTime;
  ts: DWord;
  PrevYearNode, PrevMonthNode: TTreeNode;
begin
{$RANGECHECKS OFF}
  // wParam - array of hDBEvent, lParam - array size
  PrevYearNode := nil;
  PrevMonthNode := nil;
  ti := nil;
  Sessions := PSessArray(M.wParam)^;
  FreeMem(PSessArray(M.wParam));
  tvSess.Items.BeginUpdate;
  try
    for i := 0 to Length(Sessions) - 1 do
    begin
      ts := Sessions[i].TimestampFirst;
      dt := TimestampToDateTime(ts);
      if (PrevYearNode = nil) or (uint_ptr(PrevYearNode.Data) <> YearOf(dt)) then
      begin
        PrevYearNode := tvSess.Items.AddChild(nil, FormatDateTime(HPP_SESS_YEARFORMAT, dt));
        PrevYearNode.Data := Pointer(YearOf(dt));
        PrevYearNode.ImageIndex := 5;
        // PrevYearNode.SelectedIndex := PrevYearNode.ImageIndex;
        PrevMonthNode := nil;
      end;
      if (PrevMonthNode = nil) or (uint_ptr(PrevMonthNode.Data) <> MonthOf(dt)) then
      begin
        PrevMonthNode := tvSess.Items.AddChild(PrevYearNode,
          FormatDateTime(HPP_SESS_MONTHFORMAT, dt));
        PrevMonthNode.Data := Pointer(MonthOf(dt));
        case MonthOf(dt) of
          12, 1 .. 2: PrevMonthNode.ImageIndex := 3;
          3 .. 5:     PrevMonthNode.ImageIndex := 4;
          6 .. 8:     PrevMonthNode.ImageIndex := 1;
          9 .. 11:    PrevMonthNode.ImageIndex := 2;
        end;
        // PrevMonthNode.SelectedIndex := PrevMonthNode.ImageIndex;
      end;
      ti := tvSess.Items.AddChild(PrevMonthNode, FormatDateTime(HPP_SESS_DAYFORMAT, dt));
      ti.Data := Pointer(i);
      ti.ImageIndex := 0;
      // ti.SelectedIndex := ti.ImageIndex;
    end;
    if PrevYearNode <> nil then
    begin
      PrevYearNode.Expand(False);
      PrevMonthNode.Expand(True);
    end;
    if ti <> nil then
      ti.Selected := True;
  finally
    tvSess.Items.EndUpdate;
  end;
{$RANGECHECKS ON}
end;

procedure THistoryFrm.SMFinished(var M: TMessage);
begin
  if not tvSess.Enabled then
    tvSess.Enabled := True;
  IsLoadingSessions := False;
end;

procedure THistoryFrm.SendMessage1Click(Sender: TObject);
begin
  if hContact <> 0 then
    SendMessageTo(hContact);
end;

procedure THistoryFrm.ReplyQuoted1Click(Sender: TObject);
begin
  if hContact = 0 then
    Exit;
  if hg.Selected <> -1 then
    ReplyQuoted(hg.Selected);
end;

procedure THistoryFrm.CodepageChangeClick(Sender: TObject);
var
  val: Cardinal;
begin
  val := (Sender as TMenuItem).Tag;
  WriteContactCodePage(hContact, val, Protocol);
  // UserCodepage := val;
  UserCodepage := GetContactCodePage(hContact, Protocol, UseDefaultCP);
  hg.Codepage := UserCodepage;
end;

procedure THistoryFrm.sbClearFilterClick(Sender: TObject);
begin
  if SearchMode = smFilter then
    EndHotFilterTimer;
  edSearch.Text := '';
  edSearch.Color := clWindow;
  if Self.Visible then
    hg.SetFocus
  else
    Self.ActiveControl := hg;
end;

procedure THistoryFrm.pbFilterPaint(Sender: TObject);
var
  ic: HICON;
begin
  if tiFilter.Enabled then
    ic := hppIcons[HPP_ICON_HOTFILTERWAIT].Handle
  else
    ic := hppIcons[HPP_ICON_HOTFILTER].Handle;

  DrawiconEx(pbFilter.Canvas.Handle, 0, 0, ic, 16, 16, 0, pbFilter.Canvas.Brush.Handle,
    DI_NORMAL);
end;

procedure THistoryFrm.pbSearchPaint(Sender: TObject);
begin
  DrawiconEx(pbSearch.Canvas.Handle, 0, 0, hppIcons[HPP_ICON_HOTSEARCH].Handle, 16, 16, 0,
    pbSearch.Canvas.Brush.Handle, DI_NORMAL);
end;

procedure THistoryFrm.pbSearchStatePaint(Sender: TObject);
begin
  { case laSearchState.Tag of
    1: DrawIconEx(pbSearchState.Canvas.Handle,0,0,hppIcons[HPP_ICON_HOTSEARCH].Handle,
    16,16,0,pbSearchState.Canvas.Brush.Handle,DI_NORMAL);
    2: DrawIconEx(pbSearchState.Canvas.Handle,0,0,hppIcons[HPP_ICON_HOTSEARCH].Handle,
    16,16,0,pbSearchState.Canvas.Brush.Handle,DI_NORMAL)
    else
    pbSearchState.Canvas.FillRect(pbSearchState.Canvas.ClipRect);
    end; }
end;

procedure THistoryFrm.StartHotFilterTimer;
// var
// RepaintIcon: Boolean;
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

procedure THistoryFrm.EmptyHistory;
begin
  if Assigned(EventDetailForm) then
    EventDetailForm.Release;

  HistoryLength := 0;
  SetLength(History, HistoryLength);

  SetLength(Sessions, 0);
  BookmarkServer.Contacts[hContact].Clear;
  tvSess.Items.Clear;
  lvBook.Items.Clear;

  SetSafetyMode(False);
  try
    FormState := gsDelete;
    hg.DeleteAll;
  finally
    FormState := gsIdle;
    SetSafetyMode(True);
  end;
end;

procedure THistoryFrm.EmptyHistory1Click(Sender: TObject);
begin
  CallService(MS_HPP_EMPTYHISTORY, hContact, 0);
end;

procedure THistoryFrm.EndHotFilterTimer(DoClearFilter: Boolean = False);
begin
  tiFilter.Enabled := False;
  if DoClearFilter then
    HotFilterString := ''
  else
    HotFilterString := edSearch.Text;
  hg.GridUpdate([guFilter]);
  if pbFilter.Tag <> 0 then
  begin
    pbFilter.Tag := 0;
    pbFilter.Repaint;
  end;
  if (not DoClearFilter) and (hg.Selected = -1) then
    edSearch.Color := $008080FF
  else
    edSearch.Color := clWindow;
end;

procedure THistoryFrm.tbBookmarksClick(Sender: TObject);
begin
  // when called from menu item handler
  if Sender <> tbBookmarks then
    tbBookmarks.Down := not tbBookmarks.Down;

  if tbBookmarks.Down then
    Panel := Panel + [hpBookmarks]
  else
    Panel := Panel - [hpBookmarks];
end;

procedure THistoryFrm.tbEventsFilterClick(Sender: TObject);
var
  p: TPoint;
begin
  p := tbEventsFilter.ClientOrigin;
  tbEventsFilter.ClientToScreen(p);
  pmEventsFilter.Popup(p.X, p.Y + tbEventsFilter.Height);
end;

procedure THistoryFrm.tbSearchClick(Sender: TObject);
begin
  // when called from menu item handler
  if Sender <> tbSearch then
    tbSearch.Down := not tbSearch.Down;

  if tbSearch.Down then
    SearchMode := smSearch
  else if tbFilter.Down then
    SearchMode := smFilter
  else
    SearchMode := smNone;

  if paSearch.Visible then
    edSearch.SetFocus;
end;

procedure THistoryFrm.tbFilterClick(Sender: TObject);
begin
  // when called from menu item handler
  if Sender <> tbFilter then
    tbFilter.Down := not tbFilter.Down;

  if tbSearch.Down then
    SearchMode := smSearch
  else if tbFilter.Down then
    SearchMode := smFilter
  else
    SearchMode := smNone;

  if paSearch.Visible then
    edSearch.SetFocus;
end;

procedure THistoryFrm.tbHistoryClick(Sender: TObject);
begin
  tbHistory.Down := True;
  tbHistory.CheckMenuDropdown;
  tbHistory.Down := False;
  { if hg.SelCount > 1 then begin
    SaveSelected1.Click
    exit;
    end;
    RecentFormat := TSaveFormat(GetDBInt(hppDBName,'ExportFormat',0));
    SaveFormat := RecentFormat;
    PrepareSaveDialog(SaveDialog,SaveFormat,True);
    t := Translate('Full History [%s] - [%s]');
    t := Format(t,[WideToAnsiString(hg.ProfileName,CP_ACP),WideToAnsiString(hg.ContactName,CP_ACP)]);
    t := MakeFileName(t);
    SaveDialog.FileName := t;
    if not SaveDialog.Execute then exit;
    case SaveDialog.FilterIndex of
    1: SaveFormat := sfHtml;
    2: SaveFormat := sfXml;
    3: SaveFormat := sfRTF;
    4: SaveFormat := sfUnicode;
    5: SaveFormat := sfText;
    end;
    RecentFormat := SaveFormat;
    hg.SaveAll(SaveDialog.Files[0],sfXML);
    WriteDBInt(hppDBName,'ExportFormat',Integer(RecentFormat)); }
end;

procedure THistoryFrm.tbSessionsClick(Sender: TObject);
begin
  // when called from menu item handler
  if Sender <> tbSessions then
    tbSessions.Down := not tbSessions.Down;

  if tbSessions.Down then
    Panel := Panel + [hpSessions]
  else
    Panel := Panel - [hpSessions];

end;

procedure THistoryFrm.tiFilterTimer(Sender: TObject);
begin
  EndHotFilterTimer;
end;

procedure THistoryFrm.SaveasRTF2Click(Sender: TObject);
var
  t: String;
begin
  PrepareSaveDialog(SaveDialog, sfRTF);
  t := Format(TranslateW('Full History [%s] - [%s]'), [hg.ProfileName, hg.ContactName]);
  t := MakeFileName(t);
  SaveDialog.FileName := t;
  if not SaveDialog.Execute then
    Exit;
  hg.SaveAll(SaveDialog.Files[0], sfRTF);
  RecentFormat := sfRTF;
  WriteDBInt(hppDBName, 'ExportFormat', Integer(RecentFormat));
end;

procedure THistoryFrm.SaveasMContacts2Click(Sender: TObject);
var
  t: String;
begin
  PrepareSaveDialog(SaveDialog, sfMContacts);
  t := Format(TranslateW('Full History [%s] - [%s]'), [hg.ProfileName, hg.ContactName]);
  t := MakeFileName(t);
  SaveDialog.FileName := t;
  if not SaveDialog.Execute then
    Exit;
  hg.SaveAll(SaveDialog.Files[0], sfMContacts);
  RecentFormat := sfMContacts;
  WriteDBInt(hppDBName, 'ExportFormat', Integer(RecentFormat));
end;

procedure THistoryFrm.tbHistorySearchClick(Sender: TObject);
begin
  CallService(MS_HPP_SHOWGLOBALSEARCH, 0, 0);
end;

procedure THistoryFrm.SessSelectClick(Sender: TObject);
var
  Items: Array of Integer;

  function BuildIndexesFromSession(const Node: TTreeNode): Boolean;
  var
    First, Last: THandle;
    fFirst, fLast: Integer;
    a, b, i, cnt: Integer;
  begin
    Result := False;
    if Node = nil then
      Exit;
    if Node.Level = 2 then
    begin
      First := Sessions[uint_ptr(Node.Data)].hDBEventFirst;
      Last  := Sessions[uint_ptr(Node.Data)].hDBEventLast;
      fFirst := -1;
      fLast := -1;
      for i := HistoryLength - 1 downto 0 do
      begin
        if History[i] = 0 then
          LoadPendingHeaders(i, HistoryLength);
        if History[i] = First then
          fFirst := i;
        if History[i] = Last then
          fLast := i;
        if (fLast >= 0) and (fFirst >= 0) then
          break;
      end;
      if (fLast >= 0) and (fFirst >= 0) then
      begin
        if fFirst > fLast then
        begin
          a := fLast;
          b := fFirst;
        end
        else
        begin
          a := fFirst;
          b := fLast;
        end;
        cnt := Length(Items);
        SetLength(Items, cnt + b - a + 1);
        for i := b downto a do
          Items[cnt + b - i] := HistoryIndexToGrid(i);
        Result := True;
      end;
    end
    else
      for i := 0 to Node.count - 1 do
        Result := BuildIndexesFromSession(Node.Item[i]) or Result;
  end;

begin
  if IsLoadingSessions then
    Exit;
  BuildIndexesFromSession(tvSess.Selected);
  hg.SelectRange(Items[0], Items[High(Items)]);
  // w := w + hg.Items[i].Text+#13#10+'--------------'+#13#10;
  // CopyToClip(w,Handle,UserCodepage);
  SetLength(Items, 0);
  // Index := HistoryIndexToGrid(Index);
  // ShowItem(Index);
  // exit;
  // Events := MakeSessionEvents();
end;

{ procedure THistoryFrm.tvSessMouseUp(Sender: TObject; Button: TMouseButton; Shift: TShiftState; X, Y: Integer);
  var
  Node: TTreeNode;
  begin
  exit;
  if (Button = mbRight) then begin
  Node := tvSess.GetNodeAt(X,Y);
  if Node <> nil then begin
  if not Node.Selected then
  tvSess.Select(Node);
  tvSessChange(tvSess,Node);
  if not Node.Focused then
  Node.Focused := True;
  tvSess.Invalidate;
  end;
  end;
  end; }

procedure THistoryFrm.pmEventsFilterPopup(Sender: TObject);
var
  i: Integer;
  pmi, mi: TMenuItem;
begin
  if Customize1.Parent <> pmEventsFilter.Items then
  begin
    pmi := TMenuItem(Customize1.Parent);
    for i := pmi.count - 1 downto 0 do
    begin
      mi := TMenuItem(pmi.Items[i]);
      pmi.Remove(mi);
      pmEventsFilter.Items.Insert(0, mi);
    end;
  end;
  Application.CancelHint;
end;

procedure THistoryFrm.pmGridPopup(Sender: TObject);
begin
  LoadInOptions();
  if hg.Items[hg.Selected].Bookmarked then
    Bookmark1.Caption := TranslateW('Remove &Bookmark')
  else
    Bookmark1.Caption := TranslateW('Set &Bookmark');
  AddMenuArray(pmGrid, [ContactRTLmode, ANSICodepage], -1);
end;

procedure THistoryFrm.pmHistoryPopup(Sender: TObject);
var
  pmi, mi: TMenuItem;
  i: Integer;
begin
  if SaveSelected2.Parent <> pmHistory.Items then
  begin
    pmi := TMenuItem(SaveSelected2.Parent);
    for i := pmi.count - 1 downto 0 do
    begin
      mi := TMenuItem(pmi.Items[i]);
      pmi.Remove(mi);
      pmHistory.Items.Insert(0, mi);
    end;
  end;
  LoadInOptions();
  SaveSelected2.Visible := (hg.SelCount > 1);
  AddMenuArray(pmHistory, [ContactRTLmode, ANSICodepage], 7);
  Application.CancelHint;
end;

procedure THistoryFrm.WndProc(var Message: TMessage);
begin
  case Message.Msg of
    WM_COMMAND:
      begin
        if mmAcc.DispatchCommand(Message.wParam) then
          Exit;
        inherited;
        if Message.Result <> 0 then
          Exit;
        Message.Result := CallService(MS_CLIST_MENUPROCESSCOMMAND,
          MAKEWPARAM(Message.WParamLo, MPCF_CONTACTMENU), hContact);
        Exit;
      end;
    WM_MEASUREITEM:
      if Self.UserMenu <> 0 then
      begin
        Message.Result := CallService(MS_CLIST_MENUMEASUREITEM, Message.wParam,
          Message.lParam);
        if Message.Result <> 0 then
          Exit;
      end;
    WM_DRAWITEM:
      if Self.UserMenu <> 0 then
      begin
        Message.Result := CallService(MS_CLIST_MENUDRAWITEM, Message.wParam,
          Message.lParam);
        if Message.Result <> 0 then
          Exit;
      end;
  end;
  inherited;
end;

procedure THistoryFrm.tbUserMenuClick(Sender: TObject);
var
  p: TPoint;
begin
  UserMenu := CallService(MS_CLIST_MENUBUILDCONTACT, hContact, 0);
  if UserMenu <> 0 then
  begin
    p := tbUserMenu.ClientToScreen(Point(0, tbUserMenu.Height));
    Application.CancelHint;
    TrackPopupMenu(UserMenu, TPM_TOPALIGN or TPM_LEFTALIGN or TPM_LEFTBUTTON, p.X, p.Y, 0,
      Handle, nil);
    DestroyMenu(UserMenu);
    UserMenu := 0;
  end;
end;

procedure THistoryFrm.tvSessGetSelectedIndex(Sender: TObject; Node: TTreeNode);
begin
  // and we don't need to set SelectedIndex manually anymore
  Node.SelectedIndex := Node.ImageIndex;
end;

procedure THistoryFrm.tvSessKeyDown(Sender: TObject; var Key: Word; Shift: TShiftState);
begin
  if IsFormShortCut([pmBook], Key, Shift) then
    Key := 0;
end;

procedure THistoryFrm.hgRTLEnabled(Sender: TObject; BiDiMode: TBiDiMode);
begin
  edSearch.BiDiMode := BiDiMode;
  // tvSess.BiDiMode := BiDiMode;
  if Assigned(EventDetailForm) then
    TEventDetailsFrm(EventDetailForm).ResetItem;
end;

procedure THistoryFrm.Bookmark1Click(Sender: TObject);
var
  val: Boolean;
  hDBEvent: THandle;
begin
  hDBEvent := History[GridIndexToHistory(hg.Selected)];
  val := not BookmarkServer[hContact].Bookmarked[hDBEvent];
  BookmarkServer[hContact].Bookmarked[hDBEvent] := val;
end;

procedure THistoryFrm.tbUserDetailsClick(Sender: TObject);
begin
  if hContact = 0 then
    Exit;
  CallService(MS_USERINFO_SHOWDIALOG, hContact, 0);
end;

procedure THistoryFrm.lvBookSelectItem(Sender: TObject; Item: TListItem; Selected: Boolean);
var
  Index, i: Integer;
  Event: THandle;
begin
  if not Selected then
    Exit;
  Event := THandle(Item.Data);
  Index := -1;
  // looks like history starts to load from end?
  // well, of course, we load from the last event!
  for i := HistoryLength - 1 downto 0 do
  begin
    if History[i] = 0 then
      LoadPendingHeaders(i, HistoryLength);
    if History[i] = Event then
    begin
      Index := i;
      break;
    end;
  end;
  if Index = -1 then
    Exit;
  if hg.State = gsInline then
    hg.CancelInline;
  Index := HistoryIndexToGrid(Index);
  hg.BeginUpdate;
  ShowAllEvents;
  ShowItem(Index);
  hg.EndUpdate;
end;

procedure THistoryFrm.SelectAll1Click(Sender: TObject);
begin
  hg.SelectAll;
end;

procedure THistoryFrm.lvBookContextPopup(Sender: TObject; MousePos: TPoint;
  var Handled: Boolean);
var
  Item: TListItem;
begin
  Handled := True;
  Item := TListItem(lvBook.GetItemAt(MousePos.X, MousePos.Y));
  if Item = nil then
    Exit;
  lvBook.Selected := Item;
  if BookmarkServer[hContact].Bookmarked[THandle(Item.Data)] then
  begin
    MousePos := lvBook.ClientToScreen(MousePos);
    pmBook.Popup(MousePos.X, MousePos.Y);
  end;
end;

procedure THistoryFrm.lvBookEdited(Sender: TObject; Item: TListItem; var S: String);
begin
  BookmarkServer[hContact].BookmarkName[THandle(Item.Data)] := S;
end;

procedure THistoryFrm.lvBookKeyDown(Sender: TObject; var Key: Word; Shift: TShiftState);
begin
  if IsFormShortCut([pmBook], Key, Shift) then
    Key := 0;
end;

procedure THistoryFrm.RenameBookmark1Click(Sender: TObject);
begin
  lvBook.Selected.EditCaption;
end;

procedure THistoryFrm.hgProcessInlineChange(Sender: TObject; Enabled: Boolean);
begin
  if Assigned(EventDetailForm) then
    TEventDetailsFrm(EventDetailForm).ResetItem;
end;

procedure THistoryFrm.hgInlinePopup(Sender: TObject);
begin
  InlineCopy.Enabled := hg.InlineRichEdit.SelLength > 0;
  InlineReplyQuoted.Enabled := InlineCopy.Enabled;
  InlineTextFormatting.Checked := GridOptions.TextFormatting;
  InlineSendMessage.Visible := (hContact <> 0);
  InlineReplyQuoted.Visible := (hContact <> 0);
  pmInline.Popup(Mouse.CursorPos.X, Mouse.CursorPos.Y);
end;

procedure THistoryFrm.InlineCopyClick(Sender: TObject);
begin
  if hg.InlineRichEdit.SelLength = 0 then
    Exit;
  hg.InlineRichEdit.CopyToClipboard;
end;

procedure THistoryFrm.InlineCopyAllClick(Sender: TObject);
var
  cr: TCharRange;
begin
  hg.InlineRichEdit.Lines.BeginUpdate;
  hg.InlineRichEdit.Perform(EM_EXGETSEL, 0, lParam(@cr));
  hg.InlineRichEdit.SelectAll;
  hg.InlineRichEdit.CopyToClipboard;
  hg.InlineRichEdit.Perform(EM_EXSETSEL, 0, lParam(@cr));
  hg.InlineRichEdit.Lines.EndUpdate;
end;

procedure THistoryFrm.InlineSelectAllClick(Sender: TObject);
begin
  hg.InlineRichEdit.SelectAll;
end;

procedure THistoryFrm.InlineTextFormattingClick(Sender: TObject);
begin
  GridOptions.TextFormatting := not GridOptions.TextFormatting;
end;

procedure THistoryFrm.InlineReplyQuotedClick(Sender: TObject);
begin
  if (hg.Selected = -1) or (hContact = 0) then
    Exit;
  if hg.InlineRichEdit.SelLength = 0 then
    Exit;
  SendMessageTo(hContact, hg.FormatSelected(GridOptions.ReplyQuotedTextFormat));
end;

procedure THistoryFrm.hgInlineKeyDown(Sender: TObject; var Key: Word; Shift: TShiftState);
begin
  if IsFormShortCut([mmAcc, pmInline], Key, Shift) then
  begin
    Key := 0;
    Exit;
  end;
end;

procedure THistoryFrm.ToggleMainMenu(Enabled: Boolean);
begin
  if Enabled then
  begin
    Toolbar.EdgeBorders := [ebTop];
    Menu := mmAcc
  end
  else
  begin
    Toolbar.EdgeBorders := [];
    Menu := nil;
  end;
end;

procedure THistoryFrm.WMSysColorChange(var Message: TMessage);
begin
  inherited;
  LoadToolbarIcons;
  LoadButtonIcons;
  LoadSessionIcons;
  LoadBookIcons;
  Repaint;
end;

procedure THistoryFrm.spBookMoved(Sender: TObject);
begin
  spBook.Tag := MulDiv(paSess.Height, 255, paHolder.ClientHeight);
end;

procedure THistoryFrm.paHolderResize(Sender: TObject);
begin
  if spBook.Visible then
    paSess.Height := Max(spBook.MinSize, MulDiv(paHolder.ClientHeight, spBook.Tag, 255))
  else if paSess.Visible then
    paSess.Height := paHolder.ClientHeight;
end;

procedure THistoryFrm.pmToolbarPopup(Sender: TObject);
begin
  Application.CancelHint;
end;

procedure THistoryFrm.hgFilterChange(Sender: TObject);
begin
  if Assigned(EventDetailForm) then
    TEventDetailsFrm(EventDetailForm).ResetItem;
end;

procedure THistoryFrm.OpenFileFolderClick(Sender: TObject);
begin
  if SavedFileDir = '' then
    Exit;
  ShellExecuteW(0, 'open', PWideChar(SavedFileDir), nil, nil, SW_SHOW);
  SavedFileDir := '';
end;

procedure THistoryFrm.BrowseReceivedFilesClick(Sender: TObject);
var
  Path: Array [0 .. MAX_PATH] of AnsiChar;
begin
  CallService(MS_FILE_GETRECEIVEDFILESFOLDER, hContact, lParam(@Path));
  ShellExecuteA(0, 'open', Path, nil, nil, SW_SHOW);
end;

procedure THistoryFrm.hgOptionsChange(Sender: TObject);
begin
  if Assigned(EventDetailForm) then
    TEventDetailsFrm(EventDetailForm).ResetItem;
end;

procedure THistoryFrm.hgMCData(Sender: TObject; Index: Integer; var Item: TMCItem; Stage: TSaveStage);
var
  DBEventInfo: TDBEventInfo;
  hDBEvent: THandle;
  DataOffset: PAnsiChar;
begin
  if Stage = ssInit then
  begin
    Item.Size := 0;
    hDBEvent := History[GridIndexToHistory(Index)];
    if hDBEvent <> 0 then
    begin
      DBEventInfo := GetEventInfo(hDBEvent);
      DBEventInfo.szModule := nil;
      Item.Size := Cardinal(DBEventInfo.cbSize) + Cardinal(DBEventInfo.cbBlob);
    end;
    if Item.Size > 0 then
    begin
      GetMem(Item.Buffer, Item.Size);
      DataOffset := PAnsiChar(Item.Buffer) + DBEventInfo.cbSize;
      Move(DBEventInfo, Item.Buffer^, DBEventInfo.cbSize);
      Move(DBEventInfo.pBlob^, DataOffset^, DBEventInfo.cbBlob);
    end;
  end
  else if Stage = ssDone then
  begin
    if Item.Size > 0 then
      FreeMem(Item.Buffer, Item.Size);
  end;
end;

end.
