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
 hpp_richedit(historypp project)

 Version:   1.0
 Created:   12.09.2006
 Author:    theMIROn

 [ Description ]


 [ History ]

 1.0 (12.09.2006)
   First version

 [ Modifications ]
 none

 [ Known Issues ]
 none

 Contributors: theMIROn
-----------------------------------------------------------------------------}

unit hpp_richedit;

interface

{.$DEFINE AllowMSFTEDIT}

uses
  Windows, Messages, Classes, RichEdit, ActiveX,
  Controls, StdCtrls, ComCtrls, Forms;

const
  IID_IOleObject: TGUID = '{00000112-0000-0000-C000-000000000046}';
  IID_IRichEditOle: TGUID = '{00020D00-0000-0000-C000-000000000046}';
  IID_IRichEditOleCallback: TGUID = '{00020D03-0000-0000-C000-000000000046}';
  IID_ITextDocument: TGUID = '{8CC497C0-A1DF-11CE-8098-00AA0047BE5D}';
  IID_ITextRange: TGUID = '{8CC497C2-A1DF-11CE-8098-00AA0047BE5D}';
  IID_ITextSelection: TGUID = '{8CC497C1-A1DF-11CE-8098-00AA0047BE5D}';
  IID_ITextFont: TGUID = '{8CC497C3-A1DF-11CE-8098-00AA0047BE5D}';
  IID_ITextPara: TGUID = '{8CC497C4-A1DF-11CE-8098-00AA0047BE5D}';
  IID_ITextStoryRanges: TGUID = '{8CC497C5-A1DF-11CE-8098-00AA0047BE5D}';

type
  TReObject = packed record
    cbStruct: DWORD;          // Size of structure
    cp: Integer;              // Character position of object
    clsid: TCLSID;            // Class ID of object
    poleobj: IOleObject;      // OLE object interface
    pstg: IStorage;           // Associated storage interface
    polesite: IOLEClientSite; // Associated client site interface
    sizel: TSize;             // Size of object (may be 0,0)
    dvaspect: DWORD;          // Display aspect to use
    dwFlags: DWORD;           // Object status flags
    dwUser: DWORD;            // Dword for user's use
  end;

const
  // Flags to specify which interfaces should be returned in the structure above
  REO_GETOBJ_NO_INTERFACES  = $00000000;
  REO_GETOBJ_POLEOBJ        = $00000001;
  REO_GETOBJ_PSTG           = $00000002;
  REO_GETOBJ_POLESITE       = $00000004;
  REO_GETOBJ_ALL_INTERFACES = $00000007;

  // Place object at selection
  REO_CP_SELECTION  = ULONG(-1);

  // Use character position to specify object instead of index
  REO_IOB_SELECTION = ULONG(-1);
  REO_IOB_USE_CP    = ULONG(-1);

  // Object flags
  REO_NULL            = $00000000; // No flags
  REO_READWRITEMASK   = $0000003F; // Mask out RO bits
  REO_DONTNEEDPALETTE = $00000020; // Object doesn't need palette
  REO_BLANK           = $00000010; // Object is blank
  REO_DYNAMICSIZE     = $00000008; // Object defines size always
  REO_INVERTEDSELECT  = $00000004; // Object drawn all inverted if sel
  REO_BELOWBASELINE   = $00000002; // Object sits below the baseline
  REO_RESIZABLE       = $00000001; // Object may be resized
  REO_LINK            = $80000000; // Object is a link (RO)
  REO_STATIC          = $40000000; // Object is static (RO)
  REO_SELECTED        = $08000000; // Object selected (RO)
  REO_OPEN            = $04000000; // Object open in its server (RO)
  REO_INPLACEACTIVE   = $02000000; // Object in place active (RO)
  REO_HILITED         = $01000000; // Object is to be hilited (RO)
  REO_LINKAVAILABLE   = $00800000; // Link believed available (RO)
  REO_GETMETAFILE     = $00400000; // Object requires metafile (RO)

  // flags for IRichEditOle::GetClipboardData(),
  // IRichEditOleCallback::GetClipboardData() and
  // IRichEditOleCallback::QueryAcceptData()
  RECO_PASTE  = $00000000; // paste from clipboard
  RECO_DROP   = $00000001; // drop
  RECO_COPY   = $00000002; // copy to the clipboard
  RECO_CUT    = $00000003; // cut to the clipboard
  RECO_DRAG   = $00000004; // drag

const
  tomFalse = $00000000;
  tomTrue = $FFFFFFFF;
  tomUndefined = $FF676981;
  tomToggle = $FF676982;
  tomAutoColor = $FF676983;
  tomDefault = $FF676984;
  tomSuspend = $FF676985;
  tomResume = $FF676986;
  tomApplyNow = $00000000;
  tomApplyLater = $00000001;
  tomTrackParms = $00000002;
  tomCacheParms = $00000003;
  tomApplyTmp = $00000004;
  tomBackward = $C0000001;
  tomForward = $3FFFFFFF;
  tomMove = $00000000;
  tomExtend = $00000001;
  tomNoSelection = $00000000;
  tomSelectionIP = $00000001;
  tomSelectionNormal = $00000002;
  tomSelectionFrame = $00000003;
  tomSelectionColumn = $00000004;
  tomSelectionRow = $00000005;
  tomSelectionBlock = $00000006;
  tomSelectionInlineShape = $00000007;
  tomSelectionShape = $00000008;
  tomSelStartActive = $00000001;
  tomSelAtEOL = $00000002;
  tomSelOvertype = $00000004;
  tomSelActive = $00000008;
  tomSelReplace = $00000010;
  tomEnd = $00000000;
  tomStart = $00000020;
  tomCollapseEnd = $00000000;
  tomCollapseStart = $00000001;
  tomClientCoord = $00000100;
  tomAllowOffClient = $00000200;
  tomNone = $00000000;
  tomSingle = $00000001;
  tomWords = $00000002;
  tomDouble = $00000003;
  tomDotted = $00000004;
  tomDash = $00000005;
  tomDashDot = $00000006;
  tomDashDotDot = $00000007;
  tomWave = $00000008;
  tomThick = $00000009;
  tomHair = $0000000A;
  tomDoubleWave = $0000000B;
  tomHeavyWave = $0000000C;
  tomLongDash = $0000000D;
  tomThickDash = $0000000E;
  tomThickDashDot = $0000000F;
  tomThickDashDotDot = $00000010;
  tomThickDotted = $00000011;
  tomThickLongDash = $00000012;
  tomLineSpaceSingle = $00000000;
  tomLineSpace1pt5 = $00000001;
  tomLineSpaceDouble = $00000002;
  tomLineSpaceAtLeast = $00000003;
  tomLineSpaceExactly = $00000004;
  tomLineSpaceMultiple = $00000005;
  tomAlignLeft = $00000000;
  tomAlignCenter = $00000001;
  tomAlignRight = $00000002;
  tomAlignJustify = $00000003;
  tomAlignDecimal = $00000003;
  tomAlignBar = $00000004;
  tomAlignInterWord = $00000003;
  tomAlignInterLetter = $00000004;
  tomAlignScaled = $00000005;
  tomAlignGlyphs = $00000006;
  tomAlignSnapGrid = $00000007;
  tomSpaces = $00000000;
  tomDots = $00000001;
  tomDashes = $00000002;
  tomLines = $00000003;
  tomThickLines = $00000004;
  tomEquals = $00000005;
  tomTabBack = $FFFFFFFD;
  tomTabNext = $FFFFFFFE;
  tomTabHere = $FFFFFFFF;
  tomListNone = $00000000;
  tomListBullet = $00000001;
  tomListNumberAsArabic = $00000002;
  tomListNumberAsLCLetter = $00000003;
  tomListNumberAsUCLetter = $00000004;
  tomListNumberAsLCRoman = $00000005;
  tomListNumberAsUCRoman = $00000006;
  tomListNumberAsSequence = $00000007;
  tomListParentheses = $00010000;
  tomListPeriod = $00020000;
  tomListPlain = $00030000;
  tomCharacter = $00000001;
  tomWord = $00000002;
  tomSentence = $00000003;
  tomParagraph = $00000004;
  tomLine = $00000005;
  tomStory = $00000006;
  tomScreen = $00000007;
  tomSection = $00000008;
  tomColumn = $00000009;
  tomRow = $0000000A;
  tomWindow = $0000000B;
  tomCell = $0000000C;
  tomCharFormat = $0000000D;
  tomParaFormat = $0000000E;
  tomTable = $0000000F;
  tomObject = $00000010;
  tomPage = $00000011;
  tomMatchWord = $00000002;
  tomMatchCase = $00000004;
  tomMatchPattern = $00000008;
  tomUnknownStory = $00000000;
  tomMainTextStory = $00000001;
  tomFootnotesStory = $00000002;
  tomEndnotesStory = $00000003;
  tomCommentsStory = $00000004;
  tomTextFrameStory = $00000005;
  tomEvenPagesHeaderStory = $00000006;
  tomPrimaryHeaderStory = $00000007;
  tomEvenPagesFooterStory = $00000008;
  tomPrimaryFooterStory = $00000009;
  tomFirstPageHeaderStory = $0000000A;
  tomFirstPageFooterStory = $0000000B;
  tomNoAnimation = $00000000;
  tomLasVegasLights = $00000001;
  tomBlinkingBackground = $00000002;
  tomSparkleText = $00000003;
  tomMarchingBlackAnts = $00000004;
  tomMarchingRedAnts = $00000005;
  tomShimmer = $00000006;
  tomWipeDown = $00000007;
  tomWipeRight = $00000008;
  tomAnimationMax = $00000008;
  tomLowerCase = $00000000;
  tomUpperCase = $00000001;
  tomTitleCase = $00000002;
  tomSentenceCase = $00000004;
  tomToggleCase = $00000005;
  tomReadOnly = $00000100;
  tomShareDenyRead = $00000200;
  tomShareDenyWrite = $00000400;
  tomPasteFile = $00001000;
  tomCreateNew = $00000010;
  tomCreateAlways = $00000020;
  tomOpenExisting = $00000030;
  tomOpenAlways = $00000040;
  tomTruncateExisting = $00000050;
  tomRTF = $00000001;
  tomText = $00000002;
  tomHTML = $00000003;
  tomWordDocument = $00000004;
  tomBold = $80000001;
  tomItalic = $80000002;
  tomUnderline = $80000004;
  tomStrikeout = $80000008;
  tomProtected = $80000010;
  tomLink = $80000020;
  tomSmallCaps = $80000040;
  tomAllCaps = $80000080;
  tomHidden = $80000100;
  tomOutline = $80000200;
  tomShadow = $80000400;
  tomEmboss = $80000800;
  tomImprint = $80001000;
  tomDisabled = $80002000;
  tomRevised = $80004000;
  tomNormalCaret = $00000000;
  tomKoreanBlockCaret = $00000001;
  tomIncludeInset = $00000001;
  tomIgnoreCurrentFont = $00000000;
  tomMatchFontCharset = $00000001;
  tomMatchFontSignature = $00000002;
  tomCharset = $80000000;
  tomRE10Mode = $00000001;
  tomUseAtFont = $00000002;
  tomTextFlowMask = $0000000C;
  tomTextFlowES = $00000000;
  tomTextFlowSW = $00000004;
  tomTextFlowWN = $00000008;
  tomTextFlowNE = $0000000C;
  tomNoIME = $00080000;
  tomSelfIME = $00040000;

type
  THppRichEdit = class;

  IRichEditOle = interface(IUnknown)
    ['{00020d00-0000-0000-c000-000000000046}']
    function GetClientSite(out clientSite: IOleClientSite): HResult; stdcall;
    function GetObjectCount: HResult; stdcall;
    function GetLinkCount: HResult; stdcall;
    function GetObject(iob: Longint; out ReObject: TReObject; dwFlags: DWORD): HResult; stdcall;
    function InsertObject(var ReObject: TReObject): HResult; stdcall;
    function ConvertObject(iob: Longint; rclsidNew: TIID; lpstrUserTypeNew: LPCSTR): HResult; stdcall;
    function ActivateAs(rclsid: TIID; rclsidAs: TIID): HResult; stdcall;
    function SetHostNames(lpstrContainerApp: LPCSTR; lpstrContainerObj: LPCSTR): HResult; stdcall;
    function SetLinkAvailable(iob: Longint; fAvailable: BOOL): HResult; stdcall;
    function SetDvaspect(iob: Longint; dvaspect: DWORD): HResult; stdcall;
    function HandsOffStorage(iob: Longint): HResult; stdcall;
    function SaveCompleted(iob: Longint; const stg: IStorage): HResult; stdcall;
    function InPlaceDeactivate: HResult; stdcall;
    function ContextSensitiveHelp(fEnterMode: BOOL): HResult; stdcall;
    function GetClipboardData(var chrg: TCharRange; reco: DWORD; out dataobj: IDataObject): HResult; stdcall;
    function ImportDataObject(dataobj: IDataObject; cf: TClipFormat; hMetaPict: HGLOBAL): HResult; stdcall;
  end;

  IRichEditOleCallback = interface(IUnknown)
    ['{00020d03-0000-0000-c000-000000000046}']
    function GetNewStorage(out stg: IStorage): HResult; stdcall;
    function GetInPlaceContext(out Frame: IOleInPlaceFrame; out Doc: IOleInPlaceUIWindow; lpFrameInfo: POleInPlaceFrameInfo): HResult; stdcall;
    function ShowContainerUI(fShow: BOOL): HResult; stdcall;
    function QueryInsertObject(const clsid: TCLSID; const stg: IStorage; cp: Longint): HResult; stdcall;
    function DeleteObject(const oleobj: IOleObject): HResult; stdcall;
    function QueryAcceptData(const dataobj: IDataObject; var cfFormat: TClipFormat; reco: DWORD; fReally: BOOL; hMetaPict: HGLOBAL): HResult; stdcall;
    function ContextSensitiveHelp(fEnterMode: BOOL): HResult; stdcall;
    function GetClipboardData(const chrg: TCharRange; reco: DWORD; out dataobj: IDataObject): HResult; stdcall;
    function GetDragDropEffect(fDrag: BOOL; grfKeyState: DWORD; var dwEffect: DWORD): HResult; stdcall;
    function GetContextMenu(seltype: Word; const oleobj: IOleObject; const chrg: TCharRange; out menu: HMENU): HResult; stdcall;
  end;

  TRichEditOleCallback = class(TObject, IUnknown, IRichEditOleCallback)
    private
      FRefCount: Longint;
      FRichEdit: THppRichEdit;
    public
      constructor Create(RichEdit: THppRichEdit);
      destructor Destroy; override;
      function QueryInterface(const iid: TGUID; out Obj): HResult; stdcall;
      function _AddRef: Longint; stdcall;
      function _Release: Longint; stdcall;
      function GetNewStorage(out stg: IStorage): HResult; stdcall;
      function GetInPlaceContext(out Frame: IOleInPlaceFrame; out Doc: IOleInPlaceUIWindow; lpFrameInfo: POleInPlaceFrameInfo): HResult; stdcall;
      function GetClipboardData(const chrg: TCharRange; reco: DWORD; out dataobj: IDataObject): HResult; stdcall;
      function GetContextMenu(seltype: Word; const oleobj: IOleObject; const chrg: TCharRange; out menu: HMENU): HResult; stdcall;
      function ShowContainerUI(fShow: BOOL): HResult; stdcall;
      function QueryInsertObject(const clsid: TCLSID; const stg: IStorage; cp: Longint): HResult;  stdcall;
      function DeleteObject(const oleobj: IOleObject): HResult;  stdcall;
      function QueryAcceptData(const dataobj: IDataObject; var cfFormat: TClipFormat; reco: DWORD; fReally: BOOL; hMetaPict: HGLOBAL): HResult;  stdcall;
      function ContextSensitiveHelp(fEnterMode: BOOL): HResult;  stdcall;
      function GetDragDropEffect(fDrag: BOOL; grfKeyState: DWORD; var dwEffect: DWORD): HResult;  stdcall;
  end;

  ITextDocument = interface;
  ITextDocumentDisp = dispinterface;
  ITextRange = interface;
  ITextRangeDisp = dispinterface;
  ITextSelection = interface;
  ITextSelectionDisp = dispinterface;
  ITextFont = interface;
  ITextFontDisp = dispinterface;
  ITextPara = interface;
  ITextParaDisp = dispinterface;
  ITextStoryRanges = interface;
  ITextStoryRangesDisp = dispinterface;
  ITextDocument2 = interface;
  ITextDocument2Disp = dispinterface;

  ITextDocument = interface(IDispatch)
    ['{8CC497C0-A1DF-11CE-8098-00AA0047BE5D}']
    function Get_Name: WideString; safecall;
    function Get_Selection: ITextSelection; safecall;
    function Get_StoryCount: Integer; safecall;
    function Get_StoryRanges: ITextStoryRanges; safecall;
    function Get_Saved: Integer; safecall;
    procedure Set_Saved(pValue: Integer); safecall;
    function Get_DefaultTabStop: Single; safecall;
    procedure Set_DefaultTabStop(pValue: Single); safecall;
    procedure New; safecall;
    procedure Open(var pVar: OleVariant; Flags: Integer; CodePage: Integer); safecall;
    procedure Save(var pVar: OleVariant; Flags: Integer; CodePage: Integer); safecall;
    function Freeze: Integer; safecall;
    function Unfreeze: Integer; safecall;
    procedure BeginEditCollection; safecall;
    procedure EndEditCollection; safecall;
    function Undo(Count: Integer): Integer; safecall;
    function Redo(Count: Integer): Integer; safecall;
    function Range(cp1: Integer; cp2: Integer): ITextRange; safecall;
    function RangeFromPoint(x: Integer; y: Integer): ITextRange; safecall;
    property Name: WideString read Get_Name;
    property Selection: ITextSelection read Get_Selection;
    property StoryCount: Integer read Get_StoryCount;
    property StoryRanges: ITextStoryRanges read Get_StoryRanges;
    property Saved: Integer read Get_Saved write Set_Saved;
    property DefaultTabStop: Single read Get_DefaultTabStop write Set_DefaultTabStop;
  end;

  ITextDocumentDisp = dispinterface
    ['{8CC497C0-A1DF-11CE-8098-00AA0047BE5D}']
    property Name: WideString readonly dispid 0;
    property Selection: ITextSelection readonly dispid 1;
    property StoryCount: Integer readonly dispid 2;
    property StoryRanges: ITextStoryRanges readonly dispid 3;
    property Saved: Integer dispid 4;
    property DefaultTabStop: Single dispid 5;
    procedure New; dispid 6;
    procedure Open(var pVar: OleVariant; Flags: Integer; CodePage: Integer); dispid 7;
    procedure Save(var pVar: OleVariant; Flags: Integer; CodePage: Integer); dispid 8;
    function Freeze: Integer; dispid 9;
    function Unfreeze: Integer; dispid 10;
    procedure BeginEditCollection; dispid 11;
    procedure EndEditCollection; dispid 12;
    function Undo(Count: Integer): Integer; dispid 13;
    function Redo(Count: Integer): Integer; dispid 14;
    function Range(cp1: Integer; cp2: Integer): ITextRange; dispid 15;
    function RangeFromPoint(x: Integer; y: Integer): ITextRange; dispid 16;
  end;

  ITextRange = interface(IDispatch)
    ['{8CC497C2-A1DF-11CE-8098-00AA0047BE5D}']
    function Get_Text: WideString; safecall;
    procedure Set_Text(const pbstr: WideString); safecall;
    function Get_Char: Integer; safecall;
    procedure Set_Char(pch: Integer); safecall;
    function Get_Duplicate: ITextRange; safecall;
    function Get_FormattedText: ITextRange; safecall;
    procedure Set_FormattedText(const ppRange: ITextRange); safecall;
    function Get_Start: Integer; safecall;
    procedure Set_Start(pcpFirst: Integer); safecall;
    function Get_End_: Integer; safecall;
    procedure Set_End_(pcpLim: Integer); safecall;
    function Get_Font: ITextFont; safecall;
    procedure Set_Font(const pFont: ITextFont); safecall;
    function Get_Para: ITextPara; safecall;
    procedure Set_Para(const pPara: ITextPara); safecall;
    function Get_StoryLength: Integer; safecall;
    function Get_StoryType: Integer; safecall;
    procedure Collapse(bStart: Integer); safecall;
    function Expand(Unit_: Integer): Integer; safecall;
    function GetIndex(Unit_: Integer): Integer; safecall;
    procedure SetIndex(Unit_: Integer; Index: Integer; Extend: Integer); safecall;
    procedure SetRange(cpActive: Integer; cpOther: Integer); safecall;
    function InRange(const pRange: ITextRange): Integer; safecall;
    function InStory(const pRange: ITextRange): Integer; safecall;
    function IsEqual(const pRange: ITextRange): Integer; safecall;
    procedure Select; safecall;
    function StartOf(Unit_: Integer; Extend: Integer): Integer; safecall;
    function EndOf(Unit_: Integer; Extend: Integer): Integer; safecall;
    function Move(Unit_: Integer; Count: Integer): Integer; safecall;
    function MoveStart(Unit_: Integer; Count: Integer): Integer; safecall;
    function MoveEnd(Unit_: Integer; Count: Integer): Integer; safecall;
    function MoveWhile(var Cset: OleVariant; Count: Integer): Integer; safecall;
    function MoveStartWhile(var Cset: OleVariant; Count: Integer): Integer; safecall;
    function MoveEndWhile(var Cset: OleVariant; Count: Integer): Integer; safecall;
    function MoveUntil(var Cset: OleVariant; Count: Integer): Integer; safecall;
    function MoveStartUntil(var Cset: OleVariant; Count: Integer): Integer; safecall;
    function MoveEndUntil(var Cset: OleVariant; Count: Integer): Integer; safecall;
    function FindText(const bstr: WideString; cch: Integer; Flags: Integer): Integer; safecall;
    function FindTextStart(const bstr: WideString; cch: Integer; Flags: Integer): Integer; safecall;
    function FindTextEnd(const bstr: WideString; cch: Integer; Flags: Integer): Integer; safecall;
    function Delete(Unit_: Integer; Count: Integer): Integer; safecall;
    procedure Cut(out pVar: OleVariant); safecall;
    procedure Copy(out pVar: OleVariant); safecall;
    procedure Paste(var pVar: OleVariant; Format: Integer); safecall;
    function CanPaste(var pVar: OleVariant; Format: Integer): Integer; safecall;
    function CanEdit: Integer; safecall;
    procedure ChangeCase(Type_: Integer); safecall;
    procedure GetPoint(Type_: Integer; out px: Integer; out py: Integer); safecall;
    procedure SetPoint(x: Integer; y: Integer; Type_: Integer; Extend: Integer); safecall;
    procedure ScrollIntoView(Value: Integer); safecall;
    function GetEmbeddedObject: IUnknown; safecall;
    property Text: WideString read Get_Text write Set_Text;
    property Char: Integer read Get_Char write Set_Char;
    property Duplicate: ITextRange read Get_Duplicate;
    property FormattedText: ITextRange read Get_FormattedText write Set_FormattedText;
    property Start: Integer read Get_Start write Set_Start;
    property End_: Integer read Get_End_ write Set_End_;
    property Font: ITextFont read Get_Font write Set_Font;
    property Para: ITextPara read Get_Para write Set_Para;
    property StoryLength: Integer read Get_StoryLength;
    property StoryType: Integer read Get_StoryType;
  end;

  ITextRangeDisp = dispinterface
    ['{8CC497C2-A1DF-11CE-8098-00AA0047BE5D}']
    property Text: WideString dispid 0;
    property Char: Integer dispid 513;
    property Duplicate: ITextRange readonly dispid 514;
    property FormattedText: ITextRange dispid 515;
    property Start: Integer dispid 516;
    property End_: Integer dispid 517;
    property Font: ITextFont dispid 518;
    property Para: ITextPara dispid 519;
    property StoryLength: Integer readonly dispid 520;
    property StoryType: Integer readonly dispid 521;
    procedure Collapse(bStart: Integer); dispid 528;
    function Expand(Unit_: Integer): Integer; dispid 529;
    function GetIndex(Unit_: Integer): Integer; dispid 530;
    procedure SetIndex(Unit_: Integer; Index: Integer; Extend: Integer); dispid 531;
    procedure SetRange(cpActive: Integer; cpOther: Integer); dispid 532;
    function InRange(const pRange: ITextRange): Integer; dispid 533;
    function InStory(const pRange: ITextRange): Integer; dispid 534;
    function IsEqual(const pRange: ITextRange): Integer; dispid 535;
    procedure Select; dispid 536;
    function StartOf(Unit_: Integer; Extend: Integer): Integer; dispid 537;
    function EndOf(Unit_: Integer; Extend: Integer): Integer; dispid 544;
    function Move(Unit_: Integer; Count: Integer): Integer; dispid 545;
    function MoveStart(Unit_: Integer; Count: Integer): Integer; dispid 546;
    function MoveEnd(Unit_: Integer; Count: Integer): Integer; dispid 547;
    function MoveWhile(var Cset: OleVariant; Count: Integer): Integer; dispid 548;
    function MoveStartWhile(var Cset: OleVariant; Count: Integer): Integer; dispid 549;
    function MoveEndWhile(var Cset: OleVariant; Count: Integer): Integer; dispid 550;
    function MoveUntil(var Cset: OleVariant; Count: Integer): Integer; dispid 551;
    function MoveStartUntil(var Cset: OleVariant; Count: Integer): Integer; dispid 552;
    function MoveEndUntil(var Cset: OleVariant; Count: Integer): Integer; dispid 553;
    function FindText(const bstr: WideString; cch: Integer; Flags: Integer): Integer; dispid 560;
    function FindTextStart(const bstr: WideString; cch: Integer; Flags: Integer): Integer; dispid 561;
    function FindTextEnd(const bstr: WideString; cch: Integer; Flags: Integer): Integer; dispid 562;
    function Delete(Unit_: Integer; Count: Integer): Integer; dispid 563;
    procedure Cut(out pVar: OleVariant); dispid 564;
    procedure Copy(out pVar: OleVariant); dispid 565;
    procedure Paste(var pVar: OleVariant; Format: Integer); dispid 566;
    function CanPaste(var pVar: OleVariant; Format: Integer): Integer; dispid 567;
    function CanEdit: Integer; dispid 568;
    procedure ChangeCase(Type_: Integer); dispid 569;
    procedure GetPoint(Type_: Integer; out px: Integer; out py: Integer); dispid 576;
    procedure SetPoint(x: Integer; y: Integer; Type_: Integer; Extend: Integer); dispid 577;
    procedure ScrollIntoView(Value: Integer); dispid 578;
    function GetEmbeddedObject: IUnknown; dispid 579;
  end;

  ITextSelection = interface(ITextRange)
    ['{8CC497C1-A1DF-11CE-8098-00AA0047BE5D}']
    function Get_Flags: Integer; safecall;
    procedure Set_Flags(pFlags: Integer); safecall;
    function Get_type_: Integer; safecall;
    function MoveLeft(Unit_: Integer; Count: Integer; Extend: Integer): Integer; safecall;
    function MoveRight(Unit_: Integer; Count: Integer; Extend: Integer): Integer; safecall;
    function MoveUp(Unit_: Integer; Count: Integer; Extend: Integer): Integer; safecall;
    function MoveDown(Unit_: Integer; Count: Integer; Extend: Integer): Integer; safecall;
    function HomeKey(Unit_: Integer; Extend: Integer): Integer; safecall;
    function EndKey(Unit_: Integer; Extend: Integer): Integer; safecall;
    procedure TypeText(const bstr: WideString); safecall;
    property Flags: Integer read Get_Flags write Set_Flags;
    property type_: Integer read Get_type_;
  end;

  ITextSelectionDisp = dispinterface
    ['{8CC497C1-A1DF-11CE-8098-00AA0047BE5D}']
    property Flags: Integer dispid 257;
    property type_: Integer readonly dispid 258;
    function MoveLeft(Unit_: Integer; Count: Integer; Extend: Integer): Integer; dispid 259;
    function MoveRight(Unit_: Integer; Count: Integer; Extend: Integer): Integer; dispid 260;
    function MoveUp(Unit_: Integer; Count: Integer; Extend: Integer): Integer; dispid 261;
    function MoveDown(Unit_: Integer; Count: Integer; Extend: Integer): Integer; dispid 262;
    function HomeKey(Unit_: Integer; Extend: Integer): Integer; dispid 263;
    function EndKey(Unit_: Integer; Extend: Integer): Integer; dispid 264;
    procedure TypeText(const bstr: WideString); dispid 265;
    property Text: WideString dispid 0;
    property Char: Integer dispid 513;
    property Duplicate: ITextRange readonly dispid 514;
    property FormattedText: ITextRange dispid 515;
    property Start: Integer dispid 516;
    property End_: Integer dispid 517;
    property Font: ITextFont dispid 518;
    property Para: ITextPara dispid 519;
    property StoryLength: Integer readonly dispid 520;
    property StoryType: Integer readonly dispid 521;
    procedure Collapse(bStart: Integer); dispid 528;
    function Expand(Unit_: Integer): Integer; dispid 529;
    function GetIndex(Unit_: Integer): Integer; dispid 530;
    procedure SetIndex(Unit_: Integer; Index: Integer; Extend: Integer); dispid 531;
    procedure SetRange(cpActive: Integer; cpOther: Integer); dispid 532;
    function InRange(const pRange: ITextRange): Integer; dispid 533;
    function InStory(const pRange: ITextRange): Integer; dispid 534;
    function IsEqual(const pRange: ITextRange): Integer; dispid 535;
    procedure Select; dispid 536;
    function StartOf(Unit_: Integer; Extend: Integer): Integer; dispid 537;
    function EndOf(Unit_: Integer; Extend: Integer): Integer; dispid 544;
    function Move(Unit_: Integer; Count: Integer): Integer; dispid 545;
    function MoveStart(Unit_: Integer; Count: Integer): Integer; dispid 546;
    function MoveEnd(Unit_: Integer; Count: Integer): Integer; dispid 547;
    function MoveWhile(var Cset: OleVariant; Count: Integer): Integer; dispid 548;
    function MoveStartWhile(var Cset: OleVariant; Count: Integer): Integer; dispid 549;
    function MoveEndWhile(var Cset: OleVariant; Count: Integer): Integer; dispid 550;
    function MoveUntil(var Cset: OleVariant; Count: Integer): Integer; dispid 551;
    function MoveStartUntil(var Cset: OleVariant; Count: Integer): Integer; dispid 552;
    function MoveEndUntil(var Cset: OleVariant; Count: Integer): Integer; dispid 553;
    function FindText(const bstr: WideString; cch: Integer; Flags: Integer): Integer; dispid 560;
    function FindTextStart(const bstr: WideString; cch: Integer; Flags: Integer): Integer; dispid 561;
    function FindTextEnd(const bstr: WideString; cch: Integer; Flags: Integer): Integer; dispid 562;
    function Delete(Unit_: Integer; Count: Integer): Integer; dispid 563;
    procedure Cut(out pVar: OleVariant); dispid 564;
    procedure Copy(out pVar: OleVariant); dispid 565;
    procedure Paste(var pVar: OleVariant; Format: Integer); dispid 566;
    function CanPaste(var pVar: OleVariant; Format: Integer): Integer; dispid 567;
    function CanEdit: Integer; dispid 568;
    procedure ChangeCase(Type_: Integer); dispid 569;
    procedure GetPoint(Type_: Integer; out px: Integer; out py: Integer); dispid 576;
    procedure SetPoint(x: Integer; y: Integer; Type_: Integer; Extend: Integer); dispid 577;
    procedure ScrollIntoView(Value: Integer); dispid 578;
    function GetEmbeddedObject: IUnknown; dispid 579;
  end;

  ITextFont = interface(IDispatch)
    ['{8CC497C3-A1DF-11CE-8098-00AA0047BE5D}']
    function Get_Duplicate: ITextFont; safecall;
    procedure Set_Duplicate(const ppFont: ITextFont); safecall;
    function CanChange: Integer; safecall;
    function IsEqual(const pFont: ITextFont): Integer; safecall;
    procedure Reset(Value: Integer); safecall;
    function Get_Style: Integer; safecall;
    procedure Set_Style(pValue: Integer); safecall;
    function Get_AllCaps: Integer; safecall;
    procedure Set_AllCaps(pValue: Integer); safecall;
    function Get_Animation: Integer; safecall;
    procedure Set_Animation(pValue: Integer); safecall;
    function Get_BackColor: Integer; safecall;
    procedure Set_BackColor(pValue: Integer); safecall;
    function Get_Bold: Integer; safecall;
    procedure Set_Bold(pValue: Integer); safecall;
    function Get_Emboss: Integer; safecall;
    procedure Set_Emboss(pValue: Integer); safecall;
    function Get_ForeColor: Integer; safecall;
    procedure Set_ForeColor(pValue: Integer); safecall;
    function Get_Hidden: Integer; safecall;
    procedure Set_Hidden(pValue: Integer); safecall;
    function Get_Engrave: Integer; safecall;
    procedure Set_Engrave(pValue: Integer); safecall;
    function Get_Italic: Integer; safecall;
    procedure Set_Italic(pValue: Integer); safecall;
    function Get_Kerning: Single; safecall;
    procedure Set_Kerning(pValue: Single); safecall;
    function Get_LanguageID: Integer; safecall;
    procedure Set_LanguageID(pValue: Integer); safecall;
    function Get_Name: WideString; safecall;
    procedure Set_Name(const pbstr: WideString); safecall;
    function Get_Outline: Integer; safecall;
    procedure Set_Outline(pValue: Integer); safecall;
    function Get_Position: Single; safecall;
    procedure Set_Position(pValue: Single); safecall;
    function Get_Protected_: Integer; safecall;
    procedure Set_Protected_(pValue: Integer); safecall;
    function Get_Shadow: Integer; safecall;
    procedure Set_Shadow(pValue: Integer); safecall;
    function Get_Size: Single; safecall;
    procedure Set_Size(pValue: Single); safecall;
    function Get_SmallCaps: Integer; safecall;
    procedure Set_SmallCaps(pValue: Integer); safecall;
    function Get_Spacing: Single; safecall;
    procedure Set_Spacing(pValue: Single); safecall;
    function Get_StrikeThrough: Integer; safecall;
    procedure Set_StrikeThrough(pValue: Integer); safecall;
    function Get_Subscript: Integer; safecall;
    procedure Set_Subscript(pValue: Integer); safecall;
    function Get_Superscript: Integer; safecall;
    procedure Set_Superscript(pValue: Integer); safecall;
    function Get_Underline: Integer; safecall;
    procedure Set_Underline(pValue: Integer); safecall;
    function Get_Weight: Integer; safecall;
    procedure Set_Weight(pValue: Integer); safecall;
    property Duplicate: ITextFont read Get_Duplicate write Set_Duplicate;
    property Style: Integer read Get_Style write Set_Style;
    property AllCaps: Integer read Get_AllCaps write Set_AllCaps;
    property Animation: Integer read Get_Animation write Set_Animation;
    property BackColor: Integer read Get_BackColor write Set_BackColor;
    property Bold: Integer read Get_Bold write Set_Bold;
    property Emboss: Integer read Get_Emboss write Set_Emboss;
    property ForeColor: Integer read Get_ForeColor write Set_ForeColor;
    property Hidden: Integer read Get_Hidden write Set_Hidden;
    property Engrave: Integer read Get_Engrave write Set_Engrave;
    property Italic: Integer read Get_Italic write Set_Italic;
    property Kerning: Single read Get_Kerning write Set_Kerning;
    property LanguageID: Integer read Get_LanguageID write Set_LanguageID;
    property Name: WideString read Get_Name write Set_Name;
    property Outline: Integer read Get_Outline write Set_Outline;
    property Position: Single read Get_Position write Set_Position;
    property Protected_: Integer read Get_Protected_ write Set_Protected_;
    property Shadow: Integer read Get_Shadow write Set_Shadow;
    property Size: Single read Get_Size write Set_Size;
    property SmallCaps: Integer read Get_SmallCaps write Set_SmallCaps;
    property Spacing: Single read Get_Spacing write Set_Spacing;
    property StrikeThrough: Integer read Get_StrikeThrough write Set_StrikeThrough;
    property Subscript: Integer read Get_Subscript write Set_Subscript;
    property Superscript: Integer read Get_Superscript write Set_Superscript;
    property Underline: Integer read Get_Underline write Set_Underline;
    property Weight: Integer read Get_Weight write Set_Weight;
  end;

  ITextFontDisp = dispinterface
    ['{8CC497C3-A1DF-11CE-8098-00AA0047BE5D}']
    property Duplicate: ITextFont dispid 0;
    function CanChange: Integer; dispid 769;
    function IsEqual(const pFont: ITextFont): Integer; dispid 770;
    procedure Reset(Value: Integer); dispid 771;
    property Style: Integer dispid 772;
    property AllCaps: Integer dispid 773;
    property Animation: Integer dispid 774;
    property BackColor: Integer dispid 775;
    property Bold: Integer dispid 776;
    property Emboss: Integer dispid 777;
    property ForeColor: Integer dispid 784;
    property Hidden: Integer dispid 785;
    property Engrave: Integer dispid 786;
    property Italic: Integer dispid 787;
    property Kerning: Single dispid 788;
    property LanguageID: Integer dispid 789;
    property Name: WideString dispid 790;
    property Outline: Integer dispid 791;
    property Position: Single dispid 792;
    property Protected_: Integer dispid 793;
    property Shadow: Integer dispid 800;
    property Size: Single dispid 801;
    property SmallCaps: Integer dispid 802;
    property Spacing: Single dispid 803;
    property StrikeThrough: Integer dispid 804;
    property Subscript: Integer dispid 805;
    property Superscript: Integer dispid 806;
    property Underline: Integer dispid 807;
    property Weight: Integer dispid 808;
  end;

  ITextPara = interface(IDispatch)
    ['{8CC497C4-A1DF-11CE-8098-00AA0047BE5D}']
    function Get_Duplicate: ITextPara; safecall;
    procedure Set_Duplicate(const ppPara: ITextPara); safecall;
    function CanChange: Integer; safecall;
    function IsEqual(const pPara: ITextPara): Integer; safecall;
    procedure Reset(Value: Integer); safecall;
    function Get_Style: Integer; safecall;
    procedure Set_Style(pValue: Integer); safecall;
    function Get_Alignment: Integer; safecall;
    procedure Set_Alignment(pValue: Integer); safecall;
    function Get_Hyphenation: Integer; safecall;
    procedure Set_Hyphenation(pValue: Integer); safecall;
    function Get_FirstLineIndent: Single; safecall;
    function Get_KeepTogether: Integer; safecall;
    procedure Set_KeepTogether(pValue: Integer); safecall;
    function Get_KeepWithNext: Integer; safecall;
    procedure Set_KeepWithNext(pValue: Integer); safecall;
    function Get_LeftIndent: Single; safecall;
    function Get_LineSpacing: Single; safecall;
    function Get_LineSpacingRule: Integer; safecall;
    function Get_ListAlignment: Integer; safecall;
    procedure Set_ListAlignment(pValue: Integer); safecall;
    function Get_ListLevelIndex: Integer; safecall;
    procedure Set_ListLevelIndex(pValue: Integer); safecall;
    function Get_ListStart: Integer; safecall;
    procedure Set_ListStart(pValue: Integer); safecall;
    function Get_ListTab: Single; safecall;
    procedure Set_ListTab(pValue: Single); safecall;
    function Get_ListType: Integer; safecall;
    procedure Set_ListType(pValue: Integer); safecall;
    function Get_NoLineNumber: Integer; safecall;
    procedure Set_NoLineNumber(pValue: Integer); safecall;
    function Get_PageBreakBefore: Integer; safecall;
    procedure Set_PageBreakBefore(pValue: Integer); safecall;
    function Get_RightIndent: Single; safecall;
    procedure Set_RightIndent(pValue: Single); safecall;
    procedure SetIndents(StartIndent: Single; LeftIndent: Single; RightIndent: Single); safecall;
    procedure SetLineSpacing(LineSpacingRule: Integer; LineSpacing: Single); safecall;
    function Get_SpaceAfter: Single; safecall;
    procedure Set_SpaceAfter(pValue: Single); safecall;
    function Get_SpaceBefore: Single; safecall;
    procedure Set_SpaceBefore(pValue: Single); safecall;
    function Get_WidowControl: Integer; safecall;
    procedure Set_WidowControl(pValue: Integer); safecall;
    function Get_TabCount: Integer; safecall;
    procedure AddTab(tbPos: Single; tbAlign: Integer; tbLeader: Integer); safecall;
    procedure ClearAllTabs; safecall;
    procedure DeleteTab(tbPos: Single); safecall;
    procedure GetTab(iTab: Integer; out ptbPos: Single; out ptbAlign: Integer;
                     out ptbLeader: Integer); safecall;
    property Duplicate: ITextPara read Get_Duplicate write Set_Duplicate;
    property Style: Integer read Get_Style write Set_Style;
    property Alignment: Integer read Get_Alignment write Set_Alignment;
    property Hyphenation: Integer read Get_Hyphenation write Set_Hyphenation;
    property FirstLineIndent: Single read Get_FirstLineIndent;
    property KeepTogether: Integer read Get_KeepTogether write Set_KeepTogether;
    property KeepWithNext: Integer read Get_KeepWithNext write Set_KeepWithNext;
    property LeftIndent: Single read Get_LeftIndent;
    property LineSpacing: Single read Get_LineSpacing;
    property LineSpacingRule: Integer read Get_LineSpacingRule;
    property ListAlignment: Integer read Get_ListAlignment write Set_ListAlignment;
    property ListLevelIndex: Integer read Get_ListLevelIndex write Set_ListLevelIndex;
    property ListStart: Integer read Get_ListStart write Set_ListStart;
    property ListTab: Single read Get_ListTab write Set_ListTab;
    property ListType: Integer read Get_ListType write Set_ListType;
    property NoLineNumber: Integer read Get_NoLineNumber write Set_NoLineNumber;
    property PageBreakBefore: Integer read Get_PageBreakBefore write Set_PageBreakBefore;
    property RightIndent: Single read Get_RightIndent write Set_RightIndent;
    property SpaceAfter: Single read Get_SpaceAfter write Set_SpaceAfter;
    property SpaceBefore: Single read Get_SpaceBefore write Set_SpaceBefore;
    property WidowControl: Integer read Get_WidowControl write Set_WidowControl;
    property TabCount: Integer read Get_TabCount;
  end;

  ITextParaDisp = dispinterface
    ['{8CC497C4-A1DF-11CE-8098-00AA0047BE5D}']
    property Duplicate: ITextPara dispid 0;
    function CanChange: Integer; dispid 1025;
    function IsEqual(const pPara: ITextPara): Integer; dispid 1026;
    procedure Reset(Value: Integer); dispid 1027;
    property Style: Integer dispid 1028;
    property Alignment: Integer dispid 1029;
    property Hyphenation: Integer dispid 1030;
    property FirstLineIndent: Single readonly dispid 1031;
    property KeepTogether: Integer dispid 1032;
    property KeepWithNext: Integer dispid 1033;
    property LeftIndent: Single readonly dispid 1040;
    property LineSpacing: Single readonly dispid 1041;
    property LineSpacingRule: Integer readonly dispid 1042;
    property ListAlignment: Integer dispid 1043;
    property ListLevelIndex: Integer dispid 1044;
    property ListStart: Integer dispid 1045;
    property ListTab: Single dispid 1046;
    property ListType: Integer dispid 1047;
    property NoLineNumber: Integer dispid 1048;
    property PageBreakBefore: Integer dispid 1049;
    property RightIndent: Single dispid 1056;
    procedure SetIndents(StartIndent: Single; LeftIndent: Single; RightIndent: Single); dispid 1057;
    procedure SetLineSpacing(LineSpacingRule: Integer; LineSpacing: Single); dispid 1058;
    property SpaceAfter: Single dispid 1059;
    property SpaceBefore: Single dispid 1060;
    property WidowControl: Integer dispid 1061;
    property TabCount: Integer readonly dispid 1062;
    procedure AddTab(tbPos: Single; tbAlign: Integer; tbLeader: Integer); dispid 1063;
    procedure ClearAllTabs; dispid 1064;
    procedure DeleteTab(tbPos: Single); dispid 1065;
    procedure GetTab(iTab: Integer; out ptbPos: Single; out ptbAlign: Integer;
                     out ptbLeader: Integer); dispid 1072;
  end;

  ITextStoryRanges = interface(IDispatch)
    ['{8CC497C5-A1DF-11CE-8098-00AA0047BE5D}']
    function _NewEnum: IUnknown; safecall;
    function Item(Index: Integer): ITextRange; safecall;
    function Get_Count: Integer; safecall;
    property Count: Integer read Get_Count;
  end;

  ITextStoryRangesDisp = dispinterface
    ['{8CC497C5-A1DF-11CE-8098-00AA0047BE5D}']
    function _NewEnum: IUnknown; dispid -4;
    function Item(Index: Integer): ITextRange; dispid 0;
    property Count: Integer readonly dispid 2;
  end;

  ITextDocument2 = interface(ITextDocument)
    ['{01C25500-4268-11D1-883A-3C8B00C10000}']
    procedure AttachMsgFilter(const pFilter: IUnknown); safecall;
    procedure SetEffectColor(Index: Integer; cr: LongWord); safecall;
    procedure GetEffectColor(Index: Integer; out pcr: LongWord); safecall;
    function Get_CaretType: Integer; safecall;
    procedure Set_CaretType(pCaretType: Integer); safecall;
    function GetImmContext: Integer; safecall;
    procedure ReleaseImmContext(Context: Integer); safecall;
    procedure GetPreferredFont(cp: Integer; CodePage: Integer; Option: Integer;
                               curCodepage: Integer; curFontSize: Integer; out pbstr: WideString;
                               out pPitchAndFamily: Integer; out pNewFontSize: Integer); safecall;
    function Get_NotificationMode: Integer; safecall;
    procedure Set_NotificationMode(pMode: Integer); safecall;
    procedure GetClientRect(Type_: Integer; out pLeft: Integer; out pTop: Integer;
                            out pRight: Integer; out pBottom: Integer); safecall;
    function Get_SelectionEx: ITextSelection; safecall;
    procedure GetWindow(out phWnd: Integer); safecall;
    procedure GetFEFlags(out pFlags: Integer); safecall;
    procedure UpdateWindow; safecall;
    procedure CheckTextLimit(cch: Integer; var pcch: Integer); safecall;
    procedure IMEInProgress(Mode: Integer); safecall;
    procedure SysBeep; safecall;
    procedure Update(Mode: Integer); safecall;
    procedure Notify(Notify: Integer); safecall;
    function GetDocumentFont: ITextFont; safecall;
    function GetDocumentPara: ITextPara; safecall;
    function GetCallManager: IUnknown; safecall;
    procedure ReleaseCallManager(const pVoid: IUnknown); safecall;
    property CaretType: Integer read Get_CaretType write Set_CaretType;
    property NotificationMode: Integer read Get_NotificationMode write Set_NotificationMode;
    property SelectionEx: ITextSelection read Get_SelectionEx;
  end;

  ITextDocument2Disp = dispinterface
    ['{01C25500-4268-11D1-883A-3C8B00C10000}']
    procedure AttachMsgFilter(const pFilter: IUnknown); dispid 21;
    procedure SetEffectColor(Index: Integer; cr: LongWord); dispid 22;
    procedure GetEffectColor(Index: Integer; out pcr: LongWord); dispid 23;
    property CaretType: Integer dispid 24;
    function GetImmContext: Integer; dispid 25;
    procedure ReleaseImmContext(Context: Integer); dispid 26;
    procedure GetPreferredFont(cp: Integer; CodePage: Integer; Option: Integer;
                               curCodepage: Integer; curFontSize: Integer; out pbstr: WideString;
                               out pPitchAndFamily: Integer; out pNewFontSize: Integer); dispid 27;
    property NotificationMode: Integer dispid 28;
    procedure GetClientRect(Type_: Integer; out pLeft: Integer; out pTop: Integer;
                            out pRight: Integer; out pBottom: Integer); dispid 29;
    property SelectionEx: ITextSelection readonly dispid 30;
    procedure GetWindow(out phWnd: Integer); dispid 31;
    procedure GetFEFlags(out pFlags: Integer); dispid 32;
    procedure UpdateWindow; dispid 33;
    procedure CheckTextLimit(cch: Integer; var pcch: Integer); dispid 34;
    procedure IMEInProgress(Mode: Integer); dispid 35;
    procedure SysBeep; dispid 36;
    procedure Update(Mode: Integer); dispid 37;
    procedure Notify(Notify: Integer); dispid 38;
    function GetDocumentFont: ITextFont; dispid 39;
    function GetDocumentPara: ITextPara; dispid 40;
    function GetCallManager: IUnknown; dispid 41;
    procedure ReleaseCallManager(const pVoid: IUnknown); dispid 42;
    property Name: WideString readonly dispid 0;
    property Selection: ITextSelection readonly dispid 1;
    property StoryCount: Integer readonly dispid 2;
    property StoryRanges: ITextStoryRanges readonly dispid 3;
    property Saved: Integer dispid 4;
    property DefaultTabStop: Single dispid 5;
    procedure New; dispid 6;
    procedure Open(var pVar: OleVariant; Flags: Integer; CodePage: Integer); dispid 7;
    procedure Save(var pVar: OleVariant; Flags: Integer; CodePage: Integer); dispid 8;
    function Freeze: Integer; dispid 9;
    function Unfreeze: Integer; dispid 10;
    procedure BeginEditCollection; dispid 11;
    procedure EndEditCollection; dispid 12;
    function Undo(Count: Integer): Integer; dispid 13;
    function Redo(Count: Integer): Integer; dispid 14;
    function Range(cp1: Integer; cp2: Integer): ITextRange; dispid 15;
    function RangeFromPoint(x: Integer; y: Integer): ITextRange; dispid 16;
  end;

  TURLClickEvent = procedure(Sender: TObject; const URLText: String; Button: TMouseButton) of object;

  THppRichEdit = class(TCustomRichEdit)
  private
    FVersion: Integer;
    FCodepage: Cardinal;
    FClickRange: TCharRange;
    FClickBtn: TMouseButton;
    FOnURLClick: TURLClickEvent;
    FRichEditOleCallback: TRichEditOleCallback;
    FRichEditOle: IRichEditOle;
    procedure CNNotify(var Message: TWMNotify); message CN_NOTIFY;
    procedure WMDestroy(var Msg: TWMDestroy); message WM_DESTROY;
    procedure WMRButtonUp(var Message: TWMRButtonUp); message WM_RBUTTONUP;
    procedure WMSetFocus(var Message: TWMSetFocus); message WM_SETFOCUS;
    procedure WMLangChange(var Message: TMessage); message WM_INPUTLANGCHANGE;
    procedure WMCopy(var Message: TWMCopy); message WM_COPY;
    procedure WMKeyDown(var Message: TWMKey); message WM_KEYDOWN;
    procedure SetAutoKeyboard(Enabled: Boolean);
    procedure LinkNotify(Link: TENLink);
    procedure CloseObjects;
    function UpdateHostNames: Boolean;
  protected
    procedure CreateParams(var Params: TCreateParams); override;
    procedure CreateWindowHandle(const Params: TCreateParams); override;
    procedure CreateWnd; override;
    procedure URLClick(const URLText: String; Button: TMouseButton); dynamic;
  public
    constructor Create(AOwner: TComponent); override;
    destructor Destroy; override;
    procedure Clear; override;
    //function GetTextRangeA(cpMin,cpMax: Integer): AnsiString;
    function GetTextRange(cpMin,cpMax: Integer): String;
    function GetTextLength: Integer;
    procedure ReplaceCharFormatRange(const fromCF, toCF: CHARFORMAT2; idx, len: Integer);
    procedure ReplaceCharFormat(const fromCF, toCF: CHARFORMAT2);
    property Codepage: Cardinal read FCodepage write FCodepage default CP_ACP;
    property Version: Integer read FVersion;
    property RichEditOle: IRichEditOle read FRichEditOle;
  published
    published
    property Align;
    property Alignment;
    property Anchors;
    property BevelEdges;
    property BevelInner;
    property BevelOuter;
    property BevelKind default bkNone;
    property BevelWidth;
    property BiDiMode;
    property BorderStyle;
    property BorderWidth;
    property Color;
    property Ctl3D;
    property DragCursor;
    property DragKind;
    property DragMode;
    property Enabled;
    property Font;
    property HideSelection;
    property HideScrollBars;
    property ImeMode;
    property ImeName;
    property Constraints;
    property Lines;
    property MaxLength;
    property ParentBiDiMode;
    property ParentColor;
    property ParentCtl3D;
    property ParentFont;
    property ParentShowHint;
    property PlainText;
    property PopupMenu;
    property ReadOnly;
    property ScrollBars;
    property ShowHint;
    property TabOrder;
    property TabStop default True;
    property Visible;
    property WantTabs;
    property WantReturns;
    property WordWrap;
    property OnChange;
    property OnContextPopup;
    property OnDragDrop;
    property OnDragOver;
    property OnEndDock;
    property OnEndDrag;
    property OnEnter;
    property OnExit;
    property OnKeyDown;
    property OnKeyPress;
    property OnKeyUp;
    property OnMouseDown;
    property OnMouseMove;
    property OnMouseUp;
    property OnMouseWheel;
    property OnMouseWheelDown;
    property OnMouseWheelUp;
    property OnProtectChange;
    property OnResizeRequest;
    property OnSaveClipboard;
    property OnSelectionChange;
    property OnStartDock;
    property OnStartDrag;
    property OnURLClick: TURLClickEvent read FOnURLClick write FOnURLClick;
  end;

  TImageDataObject = class(TInterfacedObject,IDataObject)
  private
    FBmp:hBitmap;
    FMedium:TStgMedium;
    FFormatEtc: TFormatEtc;
    procedure SetBitmap(bmp:hBitmap);
    function GetOleObject(OleClientSite:IOleClientSite; Storage:IStorage):IOleObject;
    // IDataObject
    function GetData(const formatetcIn: TFormatEtc; out medium: TStgMedium): HResult; stdcall;
    function GetDataHere(const formatetc: TFormatEtc; out medium: TStgMedium): HResult; stdcall;
    function QueryGetData(const formatetc: TFormatEtc): HResult; stdcall;
    function GetCanonicalFormatEtc(const formatetc: TFormatEtc; out formatetcOut: TFormatEtc): HResult; stdcall;
    function SetData(const formatetc: TFormatEtc; var medium: TStgMedium; fRelease: BOOL): HResult; stdcall;
    function EnumFormatEtc(dwDirection: Longint; out enumFormatEtc: IEnumFormatEtc): HResult; stdcall;
    function DAdvise(const formatetc: TFormatEtc; advf: Longint; const advSink: IAdviseSink; out dwConnection: Longint): HResult; stdcall;
    function DUnadvise(dwConnection: Longint): HResult; stdcall;
    function EnumDAdvise(out enumAdvise: IEnumStatData): HResult; stdcall;
  public
    destructor Destroy; override;
    function InsertBitmap(Wnd: HWND; Bitmap: hBitmap; cp: Cardinal): Boolean;
  end;

  PTextStream = ^TTextStream;
  TTextStream = record
    Size: Integer;
    case Boolean of
      false: (Data:  PAnsiChar);
      true:  (DataW: PChar);
  end;

function InitRichEditLibrary: Integer;

function GetRichRTF(RichEditHandle: THandle; var RTFStream: String;
                    SelectionOnly, PlainText, NoObjects, PlainRTF: Boolean): Integer; overload;
function GetRichRTF(RichEditHandle: THandle; var RTFStream: AnsiString;
                    SelectionOnly, PlainText, NoObjects, PlainRTF: Boolean): Integer; overload;
function SetRichRTF(RichEditHandle: THandle; const RTFStream: String;
                    SelectionOnly, PlainText, PlainRTF: Boolean): Integer; overload;
function SetRichRTF(RichEditHandle: THandle; const RTFStream: AnsiString;
                    SelectionOnly, PlainText, PlainRTF: Boolean): Integer; overload;
function FormatString2RTF(Source:     String; Suffix: AnsiString = ''): AnsiString; overload;
function FormatString2RTF(Source: AnsiString; Suffix: AnsiString = ''): AnsiString; overload;
//function FormatRTF2String(RichEditHandle: THandle; RTFStream: WideString): WideString; overload;
//function FormatRTF2String(RichEditHandle: THandle; RTFStream: AnsiString): WideString; overload;
function GetRichString(RichEditHandle: THandle; SelectionOnly: Boolean = false): String;

function RichEdit_SetOleCallback(Wnd: HWND; const Intf: IRichEditOleCallback): Boolean;
function RichEdit_GetOleInterface(Wnd: HWND; out Intf: IRichEditOle): Boolean;
function RichEdit_InsertBitmap(Wnd: HWND; Bitmap: hBitmap; cp: Cardinal): Boolean;

procedure OleCheck(OleResult: HResult);
procedure ReleaseObject(var Obj);

procedure Register;

implementation

uses
  Types, SysUtils,
  hpp_global;

type
  PClass = ^TClass;
  EOleError = class(Exception);

const
  SOleError        = 'OLE2 error occured. Error code: %.8xH';

  SF_UNICODE = 16;
  SF_USECODEPAGE = 32;

  RICHEDIT_CLASS20A = 'RICHEDIT20A';
  RICHEDIT_CLASS20W = 'RICHEDIT20W';
  MSFTEDIT_CLASS    = 'RICHEDIT50W';

var
  FRichEditModule:  THandle = 0;
  FRichEditVersion: Integer = 0;

procedure Register;
begin
  RegisterComponents(hppName, [THppRichedit]);
end;

function GetModuleVersionFile(hModule: THandle): Integer;
var
  dwVersion: Cardinal;
begin
  Result := -1;
  if hModule = 0 then exit;
  try
    dwVersion := GetFileVersion(GetModuleName(hModule));
    if dwVersion <> Cardinal(-1) then
      Result := LoWord(dwVersion);
  except
  end;
end;

function InitRichEditLibrary: Integer;
const
  RICHED20_DLL = 'RICHED20.DLL';
  {$IFDEF AllowMSFTEDIT}
  MSFTEDIT_DLL = 'MSFTEDIT.DLL';
  {$ENDIF}
var
  {$IFDEF AllowMSFTEDIT}
  hModule : THandle;
  hVersion: Integer;
  {$ENDIF}
  emError : DWord;
begin
  if FRichEditModule = 0 then
  begin
    FRichEditVersion := -1;
    emError := SetErrorMode(SEM_NOOPENFILEERRORBOX);
    try
      FRichEditModule := LoadLibrary(RICHED20_DLL);
      if FRichEditModule <= HINSTANCE_ERROR then
        FRichEditModule := 0;
      if FRichEditModule <> 0 then
        FRichEditVersion := GetModuleVersionFile(FRichEditModule);
{$IFDEF AllowMSFTEDIT}
      repeat
        if FRichEditVersion > 40 then
          break;
        hModule := LoadLibrary(MSFTEDIT_DLL);
        if hModule <= HINSTANCE_ERROR then
          hModule := 0;
        if hModule <> 0 then
        begin
          hVersion := GetModuleVersionFile(hModule);
          if hVersion > FRichEditVersion then
          begin
            if FRichEditModule <> 0 then
              FreeLibrary(FRichEditModule);
            FRichEditModule := hModule;
            FRichEditVersion := hVersion;
            break;
          end;
          FreeLibrary(hModule);
        end;
      until True;
{$ENDIF}
      if (FRichEditModule <> 0) and (FRichEditVersion = 0) then
        FRichEditVersion := 20;
    finally
      SetErrorMode(emError);
    end;
  end;
  Result := FRichEditVersion;
end;

function RichEditStreamLoad(dwCookie: Longint; pbBuff: PByte; cb: Longint; var pcb: Longint): Longint; stdcall;
var
  pBuff: PAnsiChar;
begin
  with PTextStream(dwCookie)^ do
  begin
    pBuff := Data;
    pcb := Size;
    if pcb > cb then
      pcb := cb;
    Move(pBuff^, pbBuff^, pcb);
    Inc(Data, pcb);
    Dec(Size, pcb);
  end;
  Result := 0;
end;

function RichEditStreamSave(dwCookie: Longint; pbBuff: PByte; cb: Longint; var pcb: Longint): Longint; stdcall;
var
  prevSize: Integer;
begin
  with PTextStream(dwCookie)^ do begin
    prevSize := Size;
    Inc(Size,cb);
    ReallocMem(Data,Size);
    Move(pbBuff^,(Data+prevSize)^,cb);
    pcb := cb;
  end;
  Result := 0;
end;

function _GetRichRTF(RichEditHandle: THandle; TextStream: PTextStream;
                    SelectionOnly, PlainText, NoObjects, PlainRTF, Unicode: Boolean): Integer;
var
  es: TEditStream;
  Format: Longint;
begin
  format := 0;
  if SelectionOnly then
    Format := Format or SFF_SELECTION;
  if PlainText then
  begin
    if NoObjects then
      Format := Format or SF_TEXT
    else
      Format := Format or SF_TEXTIZED;
    if Unicode then
      Format := Format or SF_UNICODE;
  end
  else
  begin
    if NoObjects then
      Format := Format or SF_RTFNOOBJS
    else
      Format := Format or SF_RTF;
    if PlainRTF then
      Format := Format or SFF_PLAINRTF;
    // if Unicode then   format := format or SF_USECODEPAGE or (CP_UTF16 shl 16);
  end;
  TextStream^.Size := 0;
  TextStream^.Data := nil;
  es.dwCookie := LPARAM(TextStream);
  es.dwError := 0;
  es.pfnCallback := @RichEditStreamSave;
  SendMessage(RichEditHandle, EM_STREAMOUT, format, LPARAM(@es));
  Result := es.dwError;
end;

function GetRichRTF(RichEditHandle: THandle; var RTFStream: String;
                    SelectionOnly, PlainText, NoObjects, PlainRTF: Boolean): Integer;
var
  Stream: TTextStream;
begin
  Result := _GetRichRTF(RichEditHandle, @Stream,
                        SelectionOnly, PlainText, NoObjects, PlainRTF, PlainText);
  if Assigned(Stream.DataW) then
  begin
    if PlainText then
      SetString(RTFStream, Stream.DataW, Stream.Size div SizeOf(Char))
    else
      RTFStream := AnsiToWideString(Stream.Data, CP_ACP);
    FreeMem(Stream.Data, Stream.Size);
  end;
end;

function GetRichRTF(RichEditHandle: THandle; var RTFStream: AnsiString;
                    SelectionOnly, PlainText, NoObjects, PlainRTF: Boolean): Integer;
var
  Stream: TTextStream;
begin
  Result := _GetRichRTF(RichEditHandle, @Stream,
                        SelectionOnly, PlainText, NoObjects, PlainRTF, False);
  if Assigned(Stream.Data) then
  begin
    SetString(RTFStream, Stream.Data, Stream.Size - 1);
    FreeMem(Stream.Data, Stream.Size);
  end;
end;

function _SetRichRTF(RichEditHandle: THandle; TextStream: PTextStream;
                    SelectionOnly, PlainText, PlainRTF, Unicode: Boolean): Integer;
var
  es: TEditStream;
  Format: Longint;
begin
  Format := 0;
  if SelectionOnly then
    Format := Format or SFF_SELECTION;
  if PlainText then
  begin
    Format := Format or SF_TEXT;
    if Unicode then
      Format := Format or SF_UNICODE;
  end
  else
  begin
    Format := Format or SF_RTF;
    if PlainRTF then
      Format := Format or SFF_PLAINRTF;
    // if Unicode then  format := format or SF_USECODEPAGE or (CP_UTF16 shl 16);
  end;
  es.dwCookie := LPARAM(TextStream);
  es.dwError := 0;
  es.pfnCallback := @RichEditStreamLoad;
  SendMessage(RichEditHandle, EM_STREAMIN, format, LPARAM(@es));
  Result := es.dwError;
end;

function SetRichRTF(RichEditHandle: THandle; const RTFStream: String;
                    SelectionOnly, PlainText, PlainRTF: Boolean): Integer;
var
  Stream: TTextStream;
  Buffer: AnsiString;
begin
  if PlainText then
  begin
    Stream.DataW := @RTFStream[1];
    Stream.Size := Length(RTFStream) * SizeOf(WideChar);
  end
  else
  begin
    Buffer := WideToAnsiString(RTFStream, CP_ACP);
    Stream.Data := @Buffer[1];
    Stream.Size := Length(Buffer);
  end;
  Result := _SetRichRTF(RichEditHandle, @Stream,
                        SelectionOnly, PlainText, PlainRTF, PlainText);
end;

function SetRichRTF(RichEditHandle: THandle; const RTFStream: AnsiString;
                    SelectionOnly, PlainText, PlainRTF: Boolean): Integer;
var
  Stream: TTextStream;
begin
  Stream.Data := @RTFStream[1];
  Stream.Size := Length(RTFStream);
  Result := _SetRichRTF(RichEditHandle, @Stream,
                        SelectionOnly, PlainText, PlainRTF, False);
end;

function FormatString2RTF(Source: String; Suffix: AnsiString = ''): AnsiString;
var
  Text: PChar;
begin
  Text := PChar(Source);
  Result := '{\uc1 ';
  while Text[0] <> #0 do
  begin
    if (Text[0] = #13) and (Text[1] = #10) then
    begin
      Result := Result + '\par ';
      Inc(Text);
    end
    else
      case Text[0] of
        #10:
          Result := Result + '\par ';
        #09:
          Result := Result + '\tab ';
        '\', '{', '}':
          Result := Result + '\' + AnsiChar(Text[0]);
      else
        if Word(Text[0]) < 128 then
          Result := Result + AnsiChar(Word(Text[0]))
        else
          Result := Result + AnsiString(Format('\u%d?', [Word(Text[0])]));
      end;
    Inc(Text);
  end;
  Result := Result + Suffix + '}';
end;

function FormatString2RTF(Source: AnsiString; Suffix: AnsiString = ''): AnsiString;
var
  Text: PAnsiChar;
begin
  Text := PAnsiChar(Source);
  Result := '{';
  while Text[0] <> #0 do
  begin
    if (Text[0] = #13) and (Text[1] = #10) then
    begin
      Result := Result + '\line ';
      Inc(Text);
    end
    else
      case Text[0] of
        #10:
          Result := Result + '\line ';
        #09:
          Result := Result + '\tab ';
        '\', '{', '}':
          Result := Result + '\' + Text[0];
      else
        Result := Result + Text[0];
      end;
    Inc(Text);
  end;
  Result := Result + Suffix + '}';
end;

{function FormatRTF2String(RichEditHandle: THandle; RTFStream: WideString): WideString;
begin
  SetRichRTF(RichEditHandle,RTFStream,False,False,True);
  GetRichRTF(RichEditHandle,Result,False,True,True,True);
end;

function FormatRTF2String(RichEditHandle: THandle; RTFStream: AnsiString): WideString;
begin
  SetRichRTF(RichEditHandle,RTFStream,False,False,True);
  GetRichRTF(RichEditHandle,Result,False,True,True,True);
end;}

function GetRichString(RichEditHandle: THandle; SelectionOnly: Boolean = false): String;
begin
  GetRichRTF(RichEditHandle,Result,SelectionOnly,True,True,False);
end;

{ OLE Specific }

function FailedHR(hr: HResult): Boolean;
begin
  Result := Failed(hr);
end;

function OleErrorMsg(ErrorCode: HResult): String;
begin
  FmtStr(Result, SOleError, [Longint(ErrorCode)]);
end;

procedure OleError(ErrorCode: HResult);
begin
  raise EOleError.Create(OleErrorMsg(ErrorCode));
end;

procedure OleCheck(OleResult: HResult);
begin
  if FailedHR(OleResult) then OleError(OleResult);
end;

procedure ReleaseObject(var Obj);
begin
  if IUnknown(Obj) <> nil then IUnknown(Obj) := nil;
end;

procedure CreateStorage(var Storage: IStorage);
var
  LockBytes: ILockBytes;
begin
  OleCheck(CreateILockBytesOnHGlobal(0, True, LockBytes));
  try
    OleCheck(StgCreateDocfileOnILockBytes(LockBytes,
      STGM_READWRITE or STGM_SHARE_EXCLUSIVE or STGM_CREATE, 0, Storage));
  finally
    ReleaseObject(LockBytes);
  end;
end;

{ THPPRichEdit }

constructor THppRichedit.Create(AOwner: TComponent);
begin
  FClickRange.cpMin := -1;
  FClickRange.cpMax := -1;
  FRichEditOleCallback := TRichEditOleCallback.Create(Self);
  inherited;
end;

destructor THppRichedit.Destroy;
begin
  inherited Destroy;
  FRichEditOleCallback.Free;
end;

procedure THppRichedit.CloseObjects;
var
  i: Integer;
  ReObject: TReObject;
begin
  if Assigned(FRichEditOle) then
  begin
    ZeroMemory(@ReObject, SizeOf(ReObject));
    ReObject.cbStruct := SizeOf(ReObject);
    with IRichEditOle(FRichEditOle) do
    begin
      for i := GetObjectCount - 1 downto 0 do
        if Succeeded(GetObject(i, ReObject, REO_GETOBJ_POLEOBJ)) then
        begin
          if ReObject.dwFlags and REO_INPLACEACTIVE <> 0 then
            IRichEditOle(FRichEditOle).InPlaceDeactivate;
          ReObject.poleobj.Close(OLECLOSE_NOSAVE);
          ReleaseObject(ReObject.poleobj);
        end;
    end;
  end;
end;

procedure THppRichedit.Clear;
begin
  CloseObjects;
  inherited;
end;

function THppRichedit.UpdateHostNames: Boolean;
var
  AppName: String;
  AnsiAppName:AnsiString;
begin
  Result := True;
  if HandleAllocated and Assigned(FRichEditOle) then
  begin
    AppName := Application.Title;
    if Trim(AppName) = '' then
      AppName := ExtractFileName(Application.ExeName);
    AnsiAppName:=AnsiString(AppName);
    try
      FRichEditOle.SetHostNames(PAnsiChar(AnsiAppName), PAnsiChar(AnsiAppName));
    except
      Result := false;
    end;
  end;
end;

type
  TAccessCustomMemo = class(TCustomMemo);
  InheritedCreateParams = procedure(var Params: TCreateParams) of object;

  procedure THppRichedit.CreateParams(var Params: TCreateParams);
const
  aHideScrollBars: array[Boolean] of DWORD = (ES_DISABLENOSCROLL, 0);
  aHideSelections: array[Boolean] of DWORD = (ES_NOHIDESEL, 0);
  aWordWrap:       array[Boolean] of DWORD = (WS_HSCROLL, 0);
var
  Method: TMethod;
begin
  FVersion := InitRichEditLibrary;
  Method.Code := @TAccessCustomMemo.CreateParams;
  Method.Data := Self;
  InheritedCreateParams(Method)(Params);
  if FVersion >= 20 then
  begin
{$IFDEF AllowMSFTEDIT}
    if FVersion = 41 then
      CreateSubClass(Params, MSFTEDIT_CLASS)
    else
{$ENDIF}
      CreateSubClass(Params, RICHEDIT_CLASS20W);
  end;
  with Params do
  begin
    Style := Style or aHideScrollBars[HideScrollBars] or aHideSelections[HideSelection] and
      not aWordWrap[WordWrap]; // more compatible with RichEdit 1.0
    // Fix for repaint richedit in event details form
    // used if class inherits from TCustomRichEdit
    // WindowClass.style := WindowClass.style or (CS_HREDRAW or CS_VREDRAW);
  end;
end;

procedure THppRichedit.CreateWindowHandle(const Params: TCreateParams);
begin
(*
  {$IFDEF AllowMSFTEDIT}
  if FVersion = 41 then
    CreateUnicodeHandle(Self, Params, MSFTEDIT_CLASS) else
  {$ENDIF}
    CreateUnicodeHandle(Self, Params, RICHEDIT_CLASS20W);
*)
inherited;
end;

procedure THppRichedit.CreateWnd;
const
  EM_SETEDITSTYLE         = WM_USER + 204;
  SES_EXTENDBACKCOLOR     = 4;
begin
  inherited;
  //SendMessage(Handle,EM_SETMARGINS,EC_LEFTMARGIN or EC_RIGHTMARGIN,0);
  Perform(EM_SETMARGINS,EC_LEFTMARGIN or EC_RIGHTMARGIN,0);
  //SendMessage(Handle,EM_SETEDITSTYLE,SES_EXTENDBACKCOLOR,SES_EXTENDBACKCOLOR);
  Perform(EM_SETEDITSTYLE,SES_EXTENDBACKCOLOR,SES_EXTENDBACKCOLOR);
  //SendMessage(Handle,EM_SETOPTIONS,ECOOP_OR,ECO_AUTOWORDSELECTION);
  Perform(EM_SETOPTIONS,ECOOP_OR,ECO_AUTOWORDSELECTION);
  //SendMessage(Handle,EM_AUTOURLDETECT,1,0);
  Perform(EM_AUTOURLDETECT,1,0);
  //SendMessage(Handle,EM_SETEVENTMASK,0,SendMessage(Handle,EM_GETEVENTMASK,0,0) or ENM_LINK);
  Perform(EM_SETEVENTMASK,0,Perform(EM_GETEVENTMASK,0,0) or ENM_LINK);
  RichEdit_SetOleCallback(Handle, FRichEditOleCallback as IRichEditOleCallback);
  if RichEdit_GetOleInterface(Handle, FRichEditOle) then UpdateHostNames;
end;

procedure THppRichedit.SetAutoKeyboard(Enabled: Boolean);
var
  re_options,new_options: DWord;
begin
  // re_options := SendMessage(Handle,EM_GETLANGOPTIONS,0,0);
  re_options := Perform(EM_GETLANGOPTIONS, 0, 0);
  if Enabled then
    new_options := re_options or IMF_AUTOKEYBOARD
  else
    new_options := re_options and not IMF_AUTOKEYBOARD;
  if re_options <> new_options then
    // SendMessage(Handle,EM_SETLANGOPTIONS,0,new_options);
    Perform(EM_SETLANGOPTIONS,0,new_options);
end;

procedure THppRichedit.ReplaceCharFormatRange(const fromCF, toCF: CHARFORMAT2; idx, len: Integer);
var
  cr: CHARRANGE;
  cf: CHARFORMAT2;
  loglen: Integer;
  res: DWord;
begin
  if len = 0 then
    exit;
  cr.cpMin := idx;
  cr.cpMax := idx + len;
  Perform(EM_EXSETSEL, 0, LPARAM(@cr));
  ZeroMemory(@cf, SizeOf(cf));
  cf.cbSize := SizeOf(cf);
  cf.dwMask := fromCF.dwMask;
  res := Perform(EM_GETCHARFORMAT, SCF_SELECTION, LPARAM(@cf));
  if (res and fromCF.dwMask) = 0 then
  begin
    if len = 2 then
    begin
      // wtf, msdn tells that cf will get the format of the first AnsiChar,
      // and then we have to select it, if format match or second, if not
      // instead we got format of the last AnsiChar... weired
      if (cf.dwEffects and fromCF.dwEffects) = fromCF.dwEffects then
        Inc(cr.cpMin)
      else
        Dec(cr.cpMax);
      Perform(EM_EXSETSEL, 0, LPARAM(@cr));
      Perform(EM_SETCHARFORMAT, SCF_SELECTION, LPARAM(@toCF));
    end
    else
    begin
      loglen := len div 2;
      ReplaceCharFormatRange(fromCF, toCF, idx, loglen);
      ReplaceCharFormatRange(fromCF, toCF, idx + loglen, len - loglen);
    end;
  end
  else if (cf.dwEffects and fromCF.dwEffects) = fromCF.dwEffects then
    Perform(EM_SETCHARFORMAT, SCF_SELECTION, LPARAM(@toCF));
end;

procedure THppRichedit.ReplaceCharFormat(const fromCF, toCF: CHARFORMAT2);
begin
  ReplaceCharFormatRange(fromCF,toCF,0,GetTextLength);
end;

(*
function THppRichedit.GetTextRangeA(cpMin,cpMax: Integer): AnsiString;
var
  WideText: WideString;
  tr: TextRange;
begin
  tr.chrg.cpMin := cpMin;
  tr.chrg.cpMax := cpMax;
  SetLength(WideText,cpMax-cpMin);
  tr.lpstrText := @WideText[1];
  Perform(EM_GETTEXTRANGE,0,LPARAM(@tr));
  Result := WideToAnsiString(WideText,Codepage);
end;
*)

function THppRichedit.GetTextRange(cpMin,cpMax: Integer): String;
var
  tr: TextRange;
begin
  tr.chrg.cpMin := cpMin;
  tr.chrg.cpMax := cpMax;
  SetLength(Result,cpMax-cpMin);
  tr.lpstrText := @Result[1];

  Perform(EM_GETTEXTRANGE,0,LPARAM(@tr));
end;

function THppRichedit.GetTextLength: Integer;
var
  gtxl: GETTEXTLENGTHEX;
begin
  gtxl.flags := GTL_DEFAULT or GTL_PRECISE;
  gtxl.codepage := 1200;
  gtxl.flags := gtxl.flags or GTL_NUMCHARS;
  Result := Perform(EM_GETTEXTLENGTHEX, WPARAM(@gtxl), 0);
end;

procedure THppRichedit.URLClick(const URLText: String; Button: TMouseButton);
begin
  if Assigned(OnURLClick) then
    OnURLClick(Self, URLText, Button);
end;

procedure THppRichedit.LinkNotify(Link: TENLink);
begin
  case Link.msg of
    WM_RBUTTONDOWN: begin
      FClickRange := Link.chrg;
      FClickBtn := mbRight;
    end;
    WM_RBUTTONUP: begin
      if (FClickBtn = mbRight) and
         (FClickRange.cpMin = Link.chrg.cpMin) and (FClickRange.cpMax = Link.chrg.cpMax) then
        URLClick(GetTextRange(Link.chrg.cpMin, Link.chrg.cpMax), mbRight);
      FClickRange.cpMin := -1;
      FClickRange.cpMax := -1;
    end;
    WM_LBUTTONDOWN: begin
      FClickRange := Link.chrg;
      FClickBtn := mbLeft;
    end;
    WM_LBUTTONUP: begin
      if (FClickBtn = mbLeft) and
         (FClickRange.cpMin = Link.chrg.cpMin) and (FClickRange.cpMax = Link.chrg.cpMax) then
        URLClick(GetTextRange(Link.chrg.cpMin, Link.chrg.cpMax), mbLeft);
      FClickRange.cpMin := -1;
      FClickRange.cpMax := -1;
    end;
  end;
end;

procedure THppRichedit.CNNotify(var Message: TWMNotify);
begin
  case Message.NMHdr^.code of
    EN_LINK: LinkNotify(TENLINK(Pointer(Message.NMHdr)^));
  else
    inherited;
  end;
end;

procedure THppRichedit.WMDestroy(var Msg: TWMDestroy);
begin
  CloseObjects;
  ReleaseObject(FRichEditOle);
  inherited;
end;

procedure THppRichedit.WMRButtonUp(var Message: TWMRButtonUp);
var
  ClassOld: TClass;
begin
  ClassOld := PClass(Self)^;
  PClass(Self)^ := TCustomMemo;
  with TMessage(Message) do
    Perform(WM_RBUTTONUP, WParam, LParam);
  PClass(Self)^ := ClassOld;
end;

procedure THppRichedit.WMSetFocus(var Message: TWMSetFocus);
begin
  SetAutoKeyboard(False);
  inherited;
end;

procedure THppRichedit.WMLangChange(var Message: TMessage);
begin
  SetAutoKeyboard(False);
  Message.Result:=1;
end;

procedure THppRichedit.WMCopy(var Message: TWMCopy);
var
  Text: String;
begin
  inherited;
  // do not empty clip to not to loose rtf data
  //EmptyClipboard();
  Text := GetRichString(Handle,True);
  CopyToClip(Text,Handle,FCodepage,False);
end;

procedure THppRichedit.WMKeyDown(var Message: TWMKey);
begin
  if (KeyDataToShiftState(Message.KeyData) = [ssCtrl]) then
    case Message.CharCode of
      Ord('E'), Ord('J'):
        Message.Result := 1;
      Ord('C'), VK_INSERT:
        begin
          PostMessage(Handle, WM_COPY, 0, 0);
          Message.Result := 1;
        end;
    end;
  if Message.Result = 1 then
    exit;
  inherited;
end;

{ TRichEditOleCallback }

constructor TRichEditOleCallback.Create(RichEdit: THppRichEdit);
begin
  inherited Create;
  FRichEdit := RichEdit;
end;

destructor TRichEditOleCallback.Destroy;
begin
  inherited Destroy;
end;

function TRichEditOleCallback.QueryInterface(const iid: TGUID; out Obj): HResult;
begin
  if GetInterface(iid, Obj) then
    Result := S_OK
  else
    Result := E_NOINTERFACE;
end;

function TRichEditOleCallback._AddRef: Longint;
begin
  Inc(FRefCount);
  Result := FRefCount;
end;

function TRichEditOleCallback._Release: Longint;
begin
  Dec(FRefCount);
  Result := FRefCount;
end;

function TRichEditOleCallback.GetNewStorage(out stg: IStorage): HResult;
begin
  try
    CreateStorage(stg);
    Result := S_OK;
  except
    Result:= E_OUTOFMEMORY;
  end;
end;

function TRichEditOleCallback.GetInPlaceContext(out Frame: IOleInPlaceFrame; out Doc: IOleInPlaceUIWindow; lpFrameInfo: POleInPlaceFrameInfo): HResult;
begin
  Result := E_NOTIMPL;
end;

function TRichEditOleCallback.QueryInsertObject(const clsid: TCLSID; const stg: IStorage; cp: Longint): HResult;
begin
  Result := NOERROR;
end;

function TRichEditOleCallback.DeleteObject(const oleobj: IOleObject): HResult;
begin
  if Assigned(oleobj) then oleobj.Close(OLECLOSE_NOSAVE);
  Result := NOERROR;
end;

function TRichEditOleCallback.QueryAcceptData(const dataobj: IDataObject; var cfFormat: TClipFormat; reco: DWORD; fReally: BOOL; hMetaPict: HGLOBAL): HResult;
begin
  Result := S_OK;
end;

function TRichEditOleCallback.ContextSensitiveHelp(fEnterMode: BOOL): HResult;
begin
  Result := E_NOTIMPL;
end;

function TRichEditOleCallback.GetClipboardData(const chrg: TCharRange; reco: DWORD; out dataobj: IDataObject): HResult;
begin
  Result := E_NOTIMPL;
end;

function TRichEditOleCallback.GetDragDropEffect(fDrag: BOOL; grfKeyState: DWORD; var dwEffect: DWORD): HResult;
begin
  Result := E_NOTIMPL;
end;

function TRichEditOleCallback.GetContextMenu(seltype: Word; const oleobj: IOleObject; const chrg: TCharRange; out menu: HMENU): HResult;
begin
  Result := E_NOTIMPL;
end;

function TRichEditOleCallback.ShowContainerUI(fShow: BOOL): HResult;
begin
  Result := E_NOTIMPL;
end;

function RichEdit_SetOleCallback(Wnd: HWND; const Intf: IRichEditOleCallback): Boolean;
begin
  Result := SendMessage(Wnd, EM_SETOLECALLBACK, 0, LPARAM(Intf)) <> 0;
end;

function RichEdit_GetOleInterface(Wnd: HWND; out Intf: IRichEditOle): Boolean;
begin
  Result := SendMessage(Wnd, EM_GETOLEINTERFACE, 0, LPARAM(@Intf)) <> 0;
end;

{ TImageDataObject }

function TImageDataObject.DAdvise(const formatetc: TFormatEtc; advf: Integer; const advSink: IAdviseSink; out dwConnection: Integer): HResult;
begin
  Result := E_NOTIMPL;
end;

function TImageDataObject.DUnadvise(dwConnection: Integer): HResult;
begin
  Result := E_NOTIMPL;
end;

function TImageDataObject.EnumDAdvise(out enumAdvise: IEnumStatData): HResult;
begin
  Result := E_NOTIMPL;
end;

function TImageDataObject.EnumFormatEtc(dwDirection: Integer; out enumFormatEtc: IEnumFormatEtc): HResult;
begin
  Result := E_NOTIMPL;
end;

function TImageDataObject.GetCanonicalFormatEtc(const formatetc: TFormatEtc; out formatetcOut: TFormatEtc): HResult;
begin
  Result := E_NOTIMPL;
end;

function TImageDataObject.GetDataHere(const formatetc: TFormatEtc; out medium: TStgMedium): HResult;
begin
  Result := E_NOTIMPL;
end;

function TImageDataObject.QueryGetData(const formatetc: TFormatEtc): HResult;
begin
  Result := E_NOTIMPL;
end;

destructor TImageDataObject.Destroy;
begin
  ReleaseStgMedium(FMedium);
end;

function TImageDataObject.GetData(const formatetcIn: TFormatEtc; out medium: TStgMedium): HResult;
begin
  medium.tymed := TYMED_GDI;
  medium.hBitmap :=  FMedium.hBitmap;
  medium.unkForRelease := nil;
  Result:=S_OK;
end;

function TImageDataObject.SetData(const formatetc: TFormatEtc; var medium: TStgMedium; fRelease: BOOL): HResult;
begin
  FFormatEtc := formatetc;
  FMedium := medium;
  Result:= S_OK;
end;

procedure TImageDataObject.SetBitmap(bmp: hBitmap);
var
  stgm: TStgMedium;
  fm: TFormatEtc;
begin
  stgm.tymed := TYMED_GDI;
  stgm.hBitmap := bmp;
  stgm.UnkForRelease := nil;
  fm.cfFormat := CF_BITMAP;
  fm.ptd := nil;
  fm.dwAspect := DVASPECT_CONTENT;
  fm.lindex := -1;
  fm.tymed := TYMED_GDI;
  SetData(fm, stgm, FALSE);
end;

function TImageDataObject.GetOleObject(OleClientSite: IOleClientSite; Storage: IStorage):IOleObject;
begin
  if (FMedium.hBitmap = 0) then
    Result := nil
  else
    OleCreateStaticFromData(Self, IID_IOleObject, OLERENDER_FORMAT, @FFormatEtc, OleClientSite,
      Storage, Result);
end;

function TImageDataObject.InsertBitmap(Wnd: HWND; Bitmap: hBitmap; cp: Cardinal): Boolean;
var
  RichEditOLE: IRichEditOLE;
  OleClientSite: IOleClientSite;
  Storage: IStorage;
  OleObject: IOleObject;
  ReObject: TReObject;
  clsid: TGUID;
begin
  Result := false;
  if Bitmap = 0 then
    exit;
  if not RichEdit_GetOleInterface(Wnd, RichEditOle) then
    exit;
  FBmp := CopyImage(Bitmap, IMAGE_BITMAP, 0, 0, 0);
  try
    SetBitmap(FBmp);
    RichEditOle.GetClientSite(OleClientSite);
    Storage := nil;
    try
      CreateStorage(Storage);
      if not(Assigned(OleClientSite) and Assigned(Storage)) then
        exit;
      try
        OleObject := GetOleObject(OleClientSite, Storage);
        if OleObject = nil then
          exit;
        OleSetContainedObject(OleObject, True);
        OleObject.GetUserClassID(clsid);
        ZeroMemory(@ReObject, SizeOf(ReObject));
        ReObject.cbStruct := SizeOf(ReObject);
        ReObject.clsid := clsid;
        ReObject.cp := cp;
        ReObject.dvaspect := DVASPECT_CONTENT;
        ReObject.poleobj := OleObject;
        ReObject.polesite := OleClientSite;
        ReObject.pstg := Storage;
        Result := (RichEditOle.InsertObject(ReObject) = NOERROR);
      finally
        ReleaseObject(OleObject);
      end;
    finally
      ReleaseObject(OleClientSite);
      ReleaseObject(Storage);
    end;
  finally
    DeleteObject(FBmp);
    ReleaseObject(RichEditOLE);
  end;
end;

function RichEdit_InsertBitmap(Wnd: HWND; Bitmap: hBitmap; cp: Cardinal): Boolean;
begin
  with TImageDataObject.Create do
  try
    Result := InsertBitmap(Wnd,Bitmap,cp);
  finally
    Free;
  end
end;

initialization

finalization
  if FRichEditModule <> 0 then FreeLibrary(FRichEditModule);

end.
