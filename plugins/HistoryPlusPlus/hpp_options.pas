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
 hpp_options (historypp project)

 Version:   1.0
 Created:   31.03.2003
 Author:    Oxygen

 [ Description ]

 Options module which has one global options variable and
 manages all options throu all history windows

 [ History ]
 1.0 (31.03.2003) - Initial version

 [ Modifications ]

 [ Knows Inssues ]
 None

 Contributors: theMIROn, Art Fedorov
-----------------------------------------------------------------------------}


unit hpp_options;

interface

uses
  Graphics, SysUtils, Windows, Dialogs,
  m_api,
  HistoryGrid,
  hpp_global;

type

  ThppIntIconsRec = record
    Handle: hIcon;
    case boolean of
      true:  (name: PAnsiChar);
      false: (id: SmallInt);
  end;

  ThppIconsRec = record
    name: PAnsiChar;
    desc: PChar;
    group: PChar;
    i: shortint;
  end;

  ThppFontType = set of (hppFont, hppColor);

  ThppFontsRec = record
    _type: ThppFontType;
    name: PAnsiChar;
    nameColor: PAnsiChar;
    Mes: TMessageTypes;
    style: byte;
    size: Integer;
    color: TColor;
    back: TColor;
  end;

  TSaveFilter = record
    Index: Integer;
    Filter: String;
    DefaultExt: String;
    Owned: TSaveFormats;
    OwnedIndex: Integer;
  end;

const
  DEFFORMAT_CLIPCOPY        = '%nick%, %smart_datetime%:\n%mes%\n';
  DEFFORMAT_CLIPCOPYTEXT    = '%mes%\n';
  DEFFORMAT_REPLYQUOTED     = '%nick%, %smart_datetime%:\n%quot_mes%\n';
  DEFFORMAT_REPLYQUOTEDTEXT = '%quot_selmes%\n';
  DEFFORMAT_SELECTION       = '%selmes%\n';
  DEFFORMAT_DATETIME        = 'c'; // ShortDateFormat + LongTimeFormat

  hppIconsDefs : array[0..32] of ThppIconsRec = (
    (name:'historypp_01'; desc:'Contact history';           group: nil; i:HPP_ICON_CONTACTHISTORY),
    (name:'historypp_02'; desc:'History search';            group: nil; i:HPP_ICON_GLOBALSEARCH),
    (name:'historypp_03'; desc:'Conversation divider';      group: 'Conversations'; i:HPP_ICON_SESS_DIVIDER),
    (name:'historypp_04'; desc:'Conversation icon';         group: 'Conversations'; i:HPP_ICON_SESSION),
    (name:'historypp_05'; desc:'Conversation summer';       group: 'Conversations'; i:HPP_ICON_SESS_SUMMER),
    (name:'historypp_06'; desc:'Conversation autumn';       group: 'Conversations'; i:HPP_ICON_SESS_AUTUMN),
    (name:'historypp_07'; desc:'Conversation winter';       group: 'Conversations'; i:HPP_ICON_SESS_WINTER),
    (name:'historypp_08'; desc:'Conversation spring';       group: 'Conversations'; i:HPP_ICON_SESS_SPRING),
    (name:'historypp_09'; desc:'Conversation year';         group: 'Conversations'; i:HPP_ICON_SESS_YEAR),
    (name:'historypp_10'; desc:'Filter';                    group: 'Toolbar'; i:HPP_ICON_HOTFILTER),
    (name:'historypp_11'; desc:'In-place filter wait';      group: 'Search panel'; i:HPP_ICON_HOTFILTERWAIT),
    (name:'historypp_12'; desc:'Search All Results';        group: nil; i:HPP_ICON_SEARCH_ALLRESULTS),
    (name:'historypp_13'; desc:'Save All';                  group: 'Toolbar'; i:HPP_ICON_TOOL_SAVEALL),
    (name:'historypp_14'; desc:'Search';                    group: 'Toolbar'; i:HPP_ICON_HOTSEARCH),
    (name:'historypp_15'; desc:'Search Up';                 group: 'Search panel'; i:HPP_ICON_SEARCHUP),
    (name:'historypp_16'; desc:'Search Down';               group: 'Search panel'; i:HPP_ICON_SEARCHDOWN),
    (name:'historypp_17'; desc:'Delete All';                group: 'Toolbar'; i:HPP_ICON_TOOL_DELETEALL),
    (name:'historypp_18'; desc:'Delete';                    group: 'Toolbar'; i:HPP_ICON_TOOL_DELETE),
    (name:'historypp_19'; desc:'Conversations';             group: 'Toolbar'; i:HPP_ICON_TOOL_SESSIONS),
    (name:'historypp_20'; desc:'Save';                      group: 'Toolbar'; i:HPP_ICON_TOOL_SAVE),
    (name:'historypp_21'; desc:'Copy';                      group: 'Toolbar'; i:HPP_ICON_TOOL_COPY),
    (name:'historypp_22'; desc:'End of page';               group: 'Search panel'; i:HPP_ICON_SEARCH_ENDOFPAGE),
    (name:'historypp_23'; desc:'Phrase not found';          group: 'Search panel'; i:HPP_ICON_SEARCH_NOTFOUND),
    (name:'historypp_24'; desc:'Clear in-place filter';     group: 'Search panel'; i:HPP_ICON_HOTFILTERCLEAR),
    (name:'historypp_25'; desc:'Conversation hide';         group: 'Conversations'; i:HPP_ICON_SESS_HIDE),
    (name:'historypp_26'; desc:'Drop down arrow';           group: 'Toolbar'; i:HPP_ICON_DROPDOWNARROW),
    (name:'historypp_27'; desc:'User Details';              group: 'Toolbar'; i:HPP_ICON_CONTACDETAILS),
    (name:'historypp_28'; desc:'User Menu';                 group: 'Toolbar'; i:HPP_ICON_CONTACTMENU),
    (name:'historypp_29'; desc:'Bookmarks';                 group: 'Toolbar'; i:HPP_ICON_BOOKMARK),
    (name:'historypp_30'; desc:'Bookmark enabled';          group: nil; i:HPP_ICON_BOOKMARK_ON),
    (name:'historypp_31'; desc:'Bookmark disabled';         group: nil; i:HPP_ICON_BOOKMARK_OFF),
    (name:'historypp_32'; desc:'Advanced Search Options';   group: 'Toolbar'; i:HPP_ICON_SEARCHADVANCED),
    (name:'historypp_33'; desc:'Limit Search Range';        group: 'Toolbar'; i:HPP_ICON_SEARCHRANGE)
  );

  hppFontItems: array[0..29] of ThppFontsRec = (
    (_type: [hppFont,hppColor]; name: 'Incoming nick'; nameColor: 'Divider';
       Mes: []; style:DBFONTF_BOLD; size: -11; color: $6B3FC8; back: clGray),

    (_type: [hppFont,hppColor]; name: 'Outgoing nick'; nameColor: 'Selected text';
       Mes: []; style:DBFONTF_BOLD; size: -11; color: $BD6008; back: clHighlightText),

    (_type: [hppColor]; nameColor: 'Selected background';
       Mes: []; back: clHighlight),

    (_type: [hppFont,hppColor]; name: 'Incoming message';
       Mes: [mtMessage,mtIncoming]; style:0; size: -11; color: $000000; back: $DBDBDB),

    (_type: [hppFont,hppColor]; name: 'Outgoing message';
       Mes: [mtMessage,mtOutgoing]; style:0; size: -11; color: $000000; back: $EEEEEE),

    (_type: [hppFont,hppColor]; name: 'Incoming file';
       Mes: [mtFile,mtIncoming]; style:0; size: -11; color: $000000; back: $9BEEE3),

    (_type: [hppFont,hppColor]; name: 'Outgoing file';
       Mes: [mtFile,mtOutgoing]; style:0; size: -11; color: $000000; back: $9BEEE3),

    (_type: [hppFont,hppColor]; name: 'Incoming url';
       Mes: [mtUrl,mtIncoming]; style:0; size: -11; color: $000000; back: $F4D9CC),

    (_type: [hppFont,hppColor]; name: 'Outgoing url';
       Mes: [mtUrl,mtOutgoing]; style:0; size: -11; color: $000000; back: $F4D9CC),

    (_type: [hppFont,hppColor]; name: 'Incoming SMS Message';
       Mes: [mtSMS,mtIncoming]; style:0; size: -11; color: $000000; back: $CFF4FE),

    (_type: [hppFont,hppColor]; name: 'Outgoing SMS Message';
       Mes: [mtSMS,mtOutgoing]; style:0; size: -11; color: $000000; back: $CFF4FE),

    (_type: [hppFont,hppColor]; name: 'Incoming contacts';
       Mes: [mtContacts,mtIncoming]; style:0; size: -11; color: $000000; back: $FEF4CF),

    (_type: [hppFont,hppColor]; name: 'Outgoing contacts';
       Mes: [mtContacts,mtOutgoing]; style:0; size: -11; color: $000000; back: $FEF4CF),

    (_type: [hppFont,hppColor]; name: 'System message';
       Mes: [mtSystem,mtIncoming,mtOutgoing]; style:0; size: -11; color: $000000; back: $CFFEDC),

    (_type: [hppFont,hppColor]; name: 'Status changes';
       Mes: [mtStatus,mtIncoming,mtOutgoing]; style:0; size: -11; color: $000000; back: $F0F0F0),

    (_type: [hppFont,hppColor]; name: 'SMTP Simple Email';
       Mes: [mtSMTPSimple,mtIncoming,mtOutgoing]; style:0; size: -11; color: $000000; back: $FFFFFF),

    (_type: [hppFont,hppColor]; name: 'Other events (unknown)';
       Mes: [mtOther,mtIncoming,mtOutgoing]; style:0; size: -11; color: $000000; back: $FFFFFF),

    (_type: [hppFont,hppColor]; name: 'Conversation header';
       Mes: []; style:0; size: -11; color: $000000; back: $00D7FDFF),

    (_type: [hppFont,hppColor]; name: 'Nick changes';
       Mes: [mtNickChange,mtIncoming,mtOutgoing]; style:0; size: -11; color: $000000; back: $00D7FDFF),

    (_type: [hppFont,hppColor]; name: 'Avatar changes';
       Mes: [mtAvatarChange,mtIncoming,mtOutgoing]; style:0; size: -11; color: $000000; back: $00D7FDFF),

    (_type: [hppFont]; name: 'Incoming timestamp';
       Mes: []; style:0; size: -11; color: $000000),

    (_type: [hppFont]; name: 'Outgoing timestamp';
       Mes: []; style:0; size: -11; color: $000000),

    (_type: [hppFont,hppColor]; name: 'Grid messages'; nameColor: 'Grid background';
       Mes: []; style:0; size: -11; color: $000000; back: $E9EAEB),

    (_type: [hppFont,hppColor]; name: 'Incoming WATrack notify';
       Mes: [mtWATrack,mtIncoming]; style:0; size: -11; color: $C08000; back: $C8FFFF),

    (_type: [hppFont,hppColor]; name: 'Outgoing WATrack notify';
       Mes: [mtWATrack,mtOutgoing]; style:0; size: -11; color: $C08000; back: $C8FFFF),

    (_type: [hppFont,hppColor]; name: 'Status message changes';
       Mes: [mtStatusMessage,mtIncoming,mtOutgoing]; style:0; size: -11; color: $000000; back: $F0F0F0),

    (_type: [hppFont,hppColor]; name: 'Voice calls';
       Mes: [mtVoiceCall,mtIncoming,mtOutgoing]; style:0; size: -11; color: $000000; back: $E9DFAB),

    (_type: [hppFont,hppColor]; name: 'Webpager message';
       Mes: [mtWebPager,mtIncoming,mtOutgoing]; style:0; size: -11; color: $000000; back: $FFFFFF),

    (_type: [hppFont,hppColor]; name: 'EMail Express message';
       Mes: [mtEmailExpress,mtIncoming,mtOutgoing]; style:0; size: -11; color: $000000; back: $FFFFFF),

    (_type: [hppColor]; nameColor: 'Link';
       Mes: []; back: clBlue)
  );

var
  GridOptions: TGridOptions;
  SmileyAddEnabled: Boolean;
  ShowHistoryCount: Boolean;
  hppIcons: array of ThppIntIconsRec;
  skinIcons: array of ThppIntIconsRec;
  SaveFormats: array[TSaveFormat] of TSaveFilter;

procedure LoadGridOptions;
procedure SaveGridOptions;
procedure LoadIcons;
procedure LoadIcons2;
procedure OnShowIcons;
procedure OnTextFormatting(Value: Boolean);
procedure hppRegisterGridOptions;
procedure hppPrepareTranslation;
procedure PrepareSaveDialog(SaveDialog: TSaveDialog; SaveFormat: TSaveFormat; AllFormats: Boolean = False);

implementation

uses
  hpp_database, hpp_contacts, hpp_events, hpp_mescatcher,
  ShellAPI;

const
  SaveFormatsDef: array[TSaveFormat] of TSaveFilter = (
    (Index: -1; Filter:'All files';         DefaultExt:'*.*'; Owned:[]; OwnedIndex: -1),
    (Index: 1;  Filter:'HTML file';         DefaultExt:'*.html'; Owned:[]; OwnedIndex: -1),
    (Index: 2;  Filter:'XML file';          DefaultExt:'*.xml'; Owned:[]; OwnedIndex: -1),
    (Index: 3;  Filter:'RTF file';          DefaultExt:'*.rtf'; Owned:[]; OwnedIndex: -1),
    (Index: 4;  Filter:'mContacts files';   DefaultExt:'*.dat'; Owned:[]; OwnedIndex: -1),
    (Index: 5;  Filter:'Unicode text file'; DefaultExt:'*.txt'; Owned:[sfUnicode,sfText]; OwnedIndex: 1),
    (Index: 6;  Filter:'Text file';         DefaultExt:'*.txt'; Owned:[sfUnicode,sfText]; OwnedIndex: 2));

procedure RegisterFont(Name:PAnsiChar; Order:integer; const defFont:TFontSettings);
var
  fid: TFontID;
begin
  fid.cbSize := sizeof(fid);
  fid.group := hppName;
  fid.dbSettingsGroup := hppDBName;
  fid.flags := FIDF_DEFAULTVALID+FIDF_ALLOWEFFECTS;
  fid.order := Order;
  lstrcpya(fid.name,Name);
  lstrcpya(fid.prefix,PAnsiChar(AnsiString('Font')+AnsiString(intToStr(Order))));
  fid.deffontsettings := defFont;
  fid.deffontsettings.size   := hppFontItems[Order].size;
  fid.deffontsettings.style  := hppFontItems[Order].style;
  fid.deffontsettings.colour := ColorToRGB(hppFontItems[Order].color);
  FontRegister(@fid);
end;

procedure RegisterColor(Name:PAnsiChar; Order:integer; defColor:TColor);
var
  cid: TColourID;
begin
  cid.cbSize := sizeof(cid);
  cid.group := hppName;
  cid.dbSettingsGroup := hppDBName;
  cid.order := Order;
  lstrcpya(cid.name,Name);
  lstrcpya(cid.setting,PAnsiChar('Color'+AnsiString(intToStr(Order))));
  cid.defcolour := ColorToRGB(defColor);
  ColourRegister(@cid);
end;

procedure OnShowIcons;
begin
  if GridOptions.ShowIcons then LoadIcons;
end;

procedure OnTextFormatting(Value: Boolean);
begin
  WriteDBBool(hppDBName,'InlineTextFormatting',Value);
end;

{function LoadIconFromDB(ID: Integer; Icon: TIcon): Boolean;
var
  hic: HIcon;
begin
  Result := False;
  hic := LoadSkinnedIcon(ID);
  if (hic <> 0) then begin
    hic := CopyIcon(hic);
    Icon.Handle := hic;
    Result := True;
  end;
end;}

procedure LoadIcons;
var
  i: Integer;
  ic: hIcon;
  Changed: Boolean;
begin
  Changed := false;
  GridOptions.StartChange;
  try
    // LoadIconFromDB(SKINICON_EVENT_MESSAGE,GridOptions.IconMessage);
    // LoadIconFromDB(SKINICON_EVENT_URL,GridOptions.IconUrl);
    // LoadIconFromDB(SKINICON_EVENT_FILE,GridOptions.IconFile);
    // LoadIconFromDB(SKINICON_OTHER_MIRANDA,GridOptions.IconOther);
    for i := 0 to High(skinIcons) do
    begin
      ic := LoadSkinnedIcon(skinIcons[i].id);
      if skinIcons[i].handle <> ic then
      begin
        skinIcons[i].handle := ic;
        Changed := true;
      end;
    end;
  finally
    GridOptions.EndChange(Changed);
  end;
end;

procedure LoadIcons2;
var
  i: integer;
  ic: hIcon;
  Changed: Boolean;
begin
  Changed := false;
  GridOptions.StartChange;
  try
    for i := 0 to High(hppIcons) do
    begin
      ic := CallService(MS_SKIN2_GETICON, 0, LPARAM(hppIcons[i].name));
      if hppIcons[i].handle <> ic then
      begin
        hppIcons[i].handle := ic;
        Changed := true;
      end;
    end;
  finally
    GridOptions.EndChange(Changed);
  end;
end;

function LoadColorDB(Order: integer): TColor;
begin
  Result := GetDBInt(hppDBName,PAnsiChar(AnsiString('Color'+intToStr(Order))),
      ColorToRGB(hppFontItems[Order].back));
end;

function LoadFont(Order: Integer; F: TFont): TFont;
const
  size: Integer = -11;
var
  fid: TFontID;
  lf: TLogFontA;
  col: TColor;
begin
  fid.cbSize := sizeof(fid);
  fid.group := hppName;
  lstrcpya(fid.name, hppFontItems[Order].name { TRANSLATE-IGNORE } );
  col := CallService(MS_FONT_GETA, WPARAM(@fid), LPARAM(@lf));
  F.handle := CreateFontIndirectA(lf);
  F.color := col;
  Result := F;
end;

procedure LoadGridOptions;
var
  i,index: integer;
begin
  GridOptions.StartChange;
  try
    // load fonts
    LoadFont(0, GridOptions.FontContact);
    // GridOptions.FontSelected := LoadFont(2,GridOptions.FontSelected);
    GridOptions.FontProfile           := LoadFont(1, GridOptions.FontProfile);
    GridOptions.FontSessHeader        := LoadFont(17, GridOptions.FontSessHeader);
    GridOptions.FontIncomingTimestamp := LoadFont(20, GridOptions.FontIncomingTimestamp);
    GridOptions.FontOutgoingTimestamp := LoadFont(21, GridOptions.FontOutgoingTimestamp);
    GridOptions.FontMessage           := LoadFont(22, GridOptions.FontMessage);
    // load colors
    GridOptions.ColorDivider      := LoadColorDB(0);
    GridOptions.ColorSelectedText := LoadColorDB(1);
    GridOptions.ColorSelected     := LoadColorDB(2);
    GridOptions.ColorSessHeader   := LoadColorDB(17);
    GridOptions.ColorBackground   := LoadColorDB(22);
    GridOptions.ColorLink         := LoadColorDB(29);

    // load mestype-related
    index := 0;
    for i := 0 to High(hppFontItems) do
    begin
      if hppFontItems[i].Mes <> [] then
      begin
        if index > High(GridOptions.ItemOptions) then
          GridOptions.AddItemOptions;
        with GridOptions.ItemOptions[index] do
        begin
          MessageType := hppFontItems[i].Mes;
          textFont := LoadFont(i, GridOptions.ItemOptions[index].textFont);
          textColor := LoadColorDB(i);
        end;
        Inc(index);
      end;
    end;

    // for i :=  3 to High(hppFontItems)-1 do begin
    // if (i-3) > High(GridOptions.ItemOptions) then GridOptions.AddItemOptions;
    // GridOptions.ItemOptions[i-3].MessageType := hppFontItems[i].Mes;
    // LoadFont(i,GridOptions.ItemOptions[i-3].textFont);
    // GridOptions.ItemOptions[i-3].textColor := LoadColorDB(i);
    // end;

    // load others
    GridOptions.ShowIcons := GetDBBool(hppDBName, 'ShowIcons', true);
    GridOptions.RTLEnabled := GetContactRTLMode(0, '');
    // we have no per-proto rtl setup ui, use global instead
    // GridOptions.ShowAvatars := GetDBBool(hppDBName,'ShowAvatars',False);

    GridOptions.SmileysEnabled        := GetDBBool(hppDBName, 'Smileys', SmileyAddEnabled);
    GridOptions.BBCodesEnabled        := GetDBBool(hppDBName, 'BBCodes', true);
    GridOptions.RawRTFEnabled         := GetDBBool(hppDBName, 'RawRTF', true);
    GridOptions.AvatarsHistoryEnabled := GetDBBool(hppDBName, 'AvatarsHistory', true);

    GridOptions.OpenDetailsMode := GetDBBool(hppDBName, 'OpenDetailsMode', false);

    GridOptions.ProfileName := GetDBWideStr(hppDBName, 'ProfileName', '');

    GridOptions.ClipCopyFormat        := GetDBWideStr(hppDBName, 'FormatCopy', DEFFORMAT_CLIPCOPY);
    GridOptions.ClipCopyTextFormat    := GetDBWideStr(hppDBName, 'FormatCopyText', DEFFORMAT_CLIPCOPYTEXT);
    GridOptions.ReplyQuotedFormat     := GetDBWideStr(hppDBName, 'FormatReplyQuoted', DEFFORMAT_REPLYQUOTED);
    GridOptions.ReplyQuotedTextFormat := GetDBWideStr(hppDBName, 'FormatReplyQuotedText', DEFFORMAT_REPLYQUOTEDTEXT);
    GridOptions.SelectionFormat       := GetDBWideStr(hppDBName, 'FormatSelection', DEFFORMAT_SELECTION);
    GridOptions.DateTimeFormat        := GetDBWideStr(hppDBName, 'DateTimeFormat', DEFFORMAT_DATETIME);
    GridOptions.TextFormatting        := GetDBBool(hppDBName, 'InlineTextFormatting', true);

    ShowHistoryCount := GetDBBool(hppDBName, 'ShowHistoryCount', false);
  finally
    GridOptions.EndChange;
  end;
end;

procedure SaveGridOptions;
begin
  GridOptions.StartChange;
  try
    WriteDBBool(hppDBName, 'ShowIcons', GridOptions.ShowIcons);
    WriteDBBool(hppDBName, 'RTL', GridOptions.RTLEnabled);
    // WriteDBBool(hppDBName,'ShowAvatars',GridOptions.ShowAvatars);

    WriteDBBool(hppDBName, 'BBCodes', GridOptions.BBCodesEnabled);
    WriteDBBool(hppDBName, 'Smileys', GridOptions.SmileysEnabled);
    WriteDBBool(hppDBName, 'RawRTF', GridOptions.RawRTFEnabled);
    WriteDBBool(hppDBName, 'AvatarsHistory', GridOptions.AvatarsHistoryEnabled);

    WriteDBBool(hppDBName, 'OpenDetailsMode', GridOptions.OpenDetailsMode);

    // WriteDBWideStr(hppDBName,'FormatCopy',GridOptions.ClipCopyFormat);
    // WriteDBWideStr(hppDBName,'FormatCopyText',GridOptions.ClipCopyTextFormat);
  finally
    GridOptions.EndChange;
  end;
end;

function FindIconsDll(ForceCheck: boolean): String;
var
  hppIconsDir: String;
  hppMessage: WideString;
  CountIconsDll: Integer;
  DoCheck: boolean;
  hppDllName,hppPluginsDir:string;
begin
  // Get plugins dir
  SetLength(hppPluginsDir, MAX_PATH);
  SetLength(hppPluginsDir, GetModuleFileNameW(hInstance, @hppPluginsDir[1], MAX_PATH));
  hppDllName := ExtractFileName(hppPluginsDir);
  hppPluginsDir := ExtractFilePath(hppPluginsDir);

  DoCheck := ForceCheck or GetDBBool(hppDBName, 'CheckIconPack', true);
  hppIconsDir := ExpandFileName(hppPluginsDir + '..\Icons\');
  if FileExists(hppIconsDir + hppIPName) then
    Result := hppIconsDir + hppIPName
  else if FileExists(hppPluginsDir + hppIPName) then
    Result := hppPluginsDir + hppIPName
  else
  begin
    Result := hppPluginsDir + hppDllName;
    if DoCheck then
    begin
      DoCheck := false;
      hppMessage :=
        WideFormat
        (FormatCString
        (TranslateW
        ('Cannot load icon pack (%s) from:\r\n%s\r\nThis can cause no icons will be shown.')),
        [hppIPName, hppIconsDir + #13#10 + hppPluginsDir]);
      HppMessageBox(hppMainWindow, hppMessage, hppName + ' Error', MB_ICONERROR or MB_OK);
    end;
  end;
  if DoCheck then
  begin
    CountIconsDll := ExtractIconExW(PWideChar(Result), -1, hIcon(nil^), hIcon(nil^), 0);
    if CountIconsDll < HppIconsCount then
    begin
      hppMessage :=
        WideFormat
        (FormatCString
        (TranslateW
        ('You are using old icon pack from:\r\n%s\r\nThis can cause missing icons, so update the icon pack.')),
        [Result]);
      HppMessageBox(hppMainWindow, hppMessage, hppName + ' Warning', MB_ICONWARNING or MB_OK);
    end;
  end;
end;

procedure hppRegisterGridOptions;
var
  sid: TSKINICONDESC;
  defFont: TFontSettings;
  // sarc: SMADD_REGCAT;
  i: Integer;
  mt: TMessageType;
  hppIconPack: String;
begin
  // Register in IcoLib
  hppIconPack := FindIconsDll(false);
  ZeroMemory(@sid, sizeof(sid));
  sid.cbSize := sizeof(sid);
  sid.Flags:=SIDF_ALL_UNICODE;

  sid.szDefaultFile.w := PChar(hppIconPack);
  for i := 0 to High(hppIconsDefs) do
  begin
    hppIcons[hppIconsDefs[i].i].name := hppIconsDefs[i].name;
    sid.pszName := hppIconsDefs[i].name;
    sid.szDescription.w := hppIconsDefs[i].desc;
    if hppIconsDefs[i].group = nil then
      sid.szSection.w := hppName
    else
      sid.szSection.w := PChar(hppName + '/' + hppIconsDefs[i].group);
    sid.iDefaultIndex := hppIconsDefs[i].i;
    Skin_AddIcon(@sid);
  end;

  for mt := Low(EventRecords) to High(EventRecords) do
  begin
    if EventRecords[mt].i = -1 then
      continue;
    if EventRecords[mt].iSkin = -1 then
    begin
      hppIcons[EventRecords[mt].i].name := EventRecords[mt].iName;
      sid.pszName := hppIcons[EventRecords[mt].i].name;
      sid.szDescription.w := PChar(EventRecords[mt].name);
      sid.szSection.w := PChar(hppName + '/' +'Events');
      sid.iDefaultIndex := EventRecords[mt].i;
      Skin_AddIcon(@sid);
    end
    else
      skinIcons[EventRecords[mt].i].id := EventRecords[mt].iSkin;
  end;

  // Register in FontService
  defFont.szFace := 'Tahoma';
  defFont.charset := DEFAULT_CHARSET;
  for i := 0 to High(hppFontItems) do
  begin
    if hppFontItems[i].Mes <> [] then
      GridOptions.AddItemOptions;
    if hppFont in hppFontItems[i]._type then
    begin
      RegisterFont(hppFontItems[i].name, i, defFont { TRANSLATE-IGNORE } );
    end;
    if hppColor in hppFontItems[i]._type then
    begin
      if hppFontItems[i].nameColor = '' then
        RegisterColor(hppFontItems[i].name, i,
          hppFontItems[i].back { TRANSLATE-IGNORE } )
      else
        RegisterColor(hppFontItems[i].nameColor, i,
          hppFontItems[i].back { TRANSLATE-IGNORE } );
    end;
  end;

  // Register in SmileyAdd
  SmileyAddEnabled := boolean(ServiceExists(MS_SMILEYADD_REPLACESMILEYS));
end;

procedure PrepareSaveDialog(SaveDialog: TSaveDialog; SaveFormat: TSaveFormat; AllFormats: boolean = false);
var
  sf: TSaveFormat;
begin
  SaveDialog.Filter := '';
  if SaveFormat = sfAll then
    SaveFormat := Succ(SaveFormat);
  if AllFormats then
  begin
    for sf := Low(SaveFormats) to High(SaveFormats) do
      if sf <> sfAll then
        SaveDialog.Filter := SaveDialog.Filter + SaveFormats[sf].Filter + '|';
    SaveDialog.FilterIndex := SaveFormats[SaveFormat].Index;
  end
  else
  begin
    if SaveFormats[SaveFormat].Owned = [] then
    begin
      SaveDialog.Filter := SaveFormats[SaveFormat].Filter + '|';
      SaveDialog.Filter := SaveDialog.Filter + SaveFormats[sfAll].Filter;
      SaveDialog.FilterIndex := 1;
    end
    else
    begin
      for sf := Low(SaveFormats) to High(SaveFormats) do
        if sf in SaveFormats[SaveFormat].Owned then
          SaveDialog.Filter := SaveDialog.Filter + SaveFormats[sf].Filter + '|';
      SaveDialog.FilterIndex := SaveFormats[SaveFormat].OwnedIndex;
    end;
  end;
  SaveDialog.DefaultExt := SaveFormats[SaveFormat].DefaultExt;
end;

procedure hppPrepareTranslation;
var
  sf: TSaveFormat;
begin
  for sf := Low(SaveFormatsDef) to High(SaveFormatsDef) do
  begin
    SaveFormats[sf] := SaveFormatsDef[sf];
    SaveFormats[sf].Filter := Format('%s (%s)|%s',
      [TranslateWideString(SaveFormatsDef[sf].Filter { TRANSLATE-IGNORE } ),
      SaveFormatsDef[sf].DefaultExt, SaveFormatsDef[sf].DefaultExt]);
  end;
end;

initialization

  GridOptions := TGridOptions.Create;
  GridOptions.OnShowIcons := OnShowIcons;
  GridOptions.OnTextFormatting := OnTextFormatting;
  SetLength(hppIcons, HppIconsCount);
  SetLength(skinIcons, SkinIconsCount);

finalization

  Finalize(hppIcons);
  Finalize(skinIcons);

  GridOptions.Free;

end.
