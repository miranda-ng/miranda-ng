unit tlb_richedit;

interface

uses
  Windows, Messages, RichEdit, ActiveX;


const
  IID_ITextDocument   : TGUID = '{8CC497C0-A1DF-11CE-8098-00AA0047BE5D}';
  IID_ITextSelection  : TGUID = '{8CC497C1-A1DF-11CE-8098-00AA0047BE5D}';
  IID_ITextRange      : TGUID = '{8CC497C2-A1DF-11CE-8098-00AA0047BE5D}';
  IID_ITextFont       : TGUID = '{8CC497C3-A1DF-11CE-8098-00AA0047BE5D}';
  IID_ITextPara       : TGUID = '{8CC497C4-A1DF-11CE-8098-00AA0047BE5D}';
  IID_ITextStoryRanges: TGUID = '{8CC497C5-A1DF-11CE-8098-00AA0047BE5D}';
  IID_ITextDocument2  : TGUID = '{01C25500-4268-11D1-883A-3C8B00C10000}';
  IID_ITextMsgFilter  : TGUID = '{A3787420-4267-11D1-883A-3C8B00C10000}';

const
  tomFalse       = $00000000;
  tomTrue        = $FFFFFFFF;

  tomUndefined   = $FF676981;
  tomToggle      = $FF676982;
  tomAutoColor   = $FF676983;
  tomDefault     = $FF676984;
  tomSuspend     = $FF676985;
  tomResume      = $FF676986;

  tomApplyNow    = $00000000;
  tomApplyLater  = $00000001;
  tomTrackParms  = $00000002;
  tomCacheParms  = $00000003;
  tomApplyTmp    = $00000004;

  tomBackward    = $C0000001;
  tomForward     = $3FFFFFFF;

  tomMove        = $00000000;
  tomExtend      = $00000001;

  tomNoSelection          = $00000000;
  tomSelectionIP          = $00000001;
  tomSelectionNormal      = $00000002;
  tomSelectionFrame       = $00000003;
  tomSelectionColumn      = $00000004;
  tomSelectionRow         = $00000005;
  tomSelectionBlock       = $00000006;
  tomSelectionInlineShape = $00000007;
  tomSelectionShape       = $00000008;

  tomSelStartActive       = $00000001;
  tomSelAtEOL             = $00000002;
  tomSelOvertype          = $00000004;
  tomSelActive            = $00000008;
  tomSelReplace           = $00000010;

  tomEnd   = $00000000;
  tomStart = $00000020;

  tomCollapseEnd   = $00000000;
  tomCollapseStart = $00000001;

  tomClientCoord    = $00000100;
  tomAllowOffClient = $00000200;

  tomNone            = $00000000;
  tomSingle          = $00000001;
  tomWords           = $00000002;
  tomDouble          = $00000003;
  tomDotted          = $00000004;
  tomDash            = $00000005;
  tomDashDot         = $00000006;
  tomDashDotDot      = $00000007;
  tomWave            = $00000008;
  tomThick           = $00000009;
  tomHair            = $0000000A;
  tomDoubleWave      = $0000000B;
  tomHeavyWave       = $0000000C;
  tomLongDash        = $0000000D;
  tomThickDash       = $0000000E;
  tomThickDashDot    = $0000000F;
  tomThickDashDotDot = $00000010;
  tomThickDotted     = $00000011;
  tomThickLongDash   = $00000012;

  tomLineSpaceSingle   = $00000000;
  tomLineSpace1pt5     = $00000001;
  tomLineSpaceDouble   = $00000002;
  tomLineSpaceAtLeast  = $00000003;
  tomLineSpaceExactly  = $00000004;
  tomLineSpaceMultiple = $00000005;

  tomAlignLeft        = $00000000;
  tomAlignCenter      = $00000001;
  tomAlignRight       = $00000002;
  tomAlignJustify     = $00000003;
  tomAlignDecimal     = $00000003;
  tomAlignBar         = $00000004;
  tomAlignInterWord   = $00000003;
  tomAlignInterLetter = $00000004;
  tomAlignScaled      = $00000005;
  tomAlignGlyphs      = $00000006;
  tomAlignSnapGrid    = $00000007;

  tomSpaces     = $00000000;
  tomDots       = $00000001;
  tomDashes     = $00000002;
  tomLines      = $00000003;
  tomThickLines = $00000004;
  tomEquals     = $00000005;

  tomTabBack    = $FFFFFFFD;
  tomTabNext    = $FFFFFFFE;
  tomTabHere    = $FFFFFFFF;

  tomListNone             = $00000000;
  tomListBullet           = $00000001;
  tomListNumberAsArabic   = $00000002;
  tomListNumberAsLCLetter = $00000003;
  tomListNumberAsUCLetter = $00000004;
  tomListNumberAsLCRoman  = $00000005;
  tomListNumberAsUCRoman  = $00000006;
  tomListNumberAsSequence = $00000007;

  tomListParentheses      = $00010000;
  tomListPeriod           = $00020000;
  tomListPlain            = $00030000;

  tomCharacter  = $00000001;
  tomWord       = $00000002;
  tomSentence   = $00000003;
  tomParagraph  = $00000004;
  tomLine       = $00000005;
  tomStory      = $00000006;
  tomScreen     = $00000007;
  tomSection    = $00000008;
  tomColumn     = $00000009;
  tomRow        = $0000000A;
  tomWindow     = $0000000B;
  tomCell       = $0000000C;
  tomCharFormat = $0000000D;
  tomParaFormat = $0000000E;
  tomTable      = $0000000F;
  tomObject     = $00000010;
  tomPage       = $00000011;

  tomMatchWord    = $00000002;
  tomMatchCase    = $00000004;
  tomMatchPattern = $00000008;

  tomUnknownStory         = $00000000;
  tomMainTextStory        = $00000001;
  tomFootnotesStory       = $00000002;
  tomEndnotesStory        = $00000003;
  tomCommentsStory        = $00000004;
  tomTextFrameStory       = $00000005;
  tomEvenPagesHeaderStory = $00000006;
  tomPrimaryHeaderStory   = $00000007;
  tomEvenPagesFooterStory = $00000008;
  tomPrimaryFooterStory   = $00000009;
  tomFirstPageHeaderStory = $0000000A;
  tomFirstPageFooterStory = $0000000B;

  tomNoAnimation        = $00000000;
  tomLasVegasLights     = $00000001;
  tomBlinkingBackground = $00000002;
  tomSparkleText        = $00000003;
  tomMarchingBlackAnts  = $00000004;
  tomMarchingRedAnts    = $00000005;
  tomShimmer            = $00000006;
  tomWipeDown           = $00000007;
  tomWipeRight          = $00000008;
  tomAnimationMax       = $00000008;

  tomLowerCase    = $00000000;
  tomUpperCase    = $00000001;
  tomTitleCase    = $00000002;
  tomSentenceCase = $00000004;
  tomToggleCase   = $00000005;

  tomReadOnly       = $00000100;
  tomShareDenyRead  = $00000200;
  tomShareDenyWrite = $00000400;
  tomPasteFile      = $00001000;

  tomCreateNew        = $00000010;
  tomCreateAlways     = $00000020;
  tomOpenExisting     = $00000030;
  tomOpenAlways       = $00000040;
  tomTruncateExisting = $00000050;

  tomRTF          = $00000001;
  tomText         = $00000002;
  tomHTML         = $00000003;
  tomWordDocument = $00000004;

  tomBold      = $80000001;
  tomItalic    = $80000002;
  tomUnderline = $80000004;
  tomStrikeout = $80000008;
  tomProtected = $80000010;
  tomLink      = $80000020;
  tomSmallCaps = $80000040;
  tomAllCaps   = $80000080;
  tomHidden    = $80000100;
  tomOutline   = $80000200;
  tomShadow    = $80000400;
  tomEmboss    = $80000800;
  tomImprint   = $80001000;
  tomDisabled  = $80002000;
  tomRevised   = $80004000;

  tomNormalCaret        = $00000000;
  tomKoreanBlockCaret   = $00000001;
  tomIncludeInset       = $00000001;
  tomIgnoreCurrentFont  = $00000000;
  tomMatchFontCharset   = $00000001;
  tomMatchFontSignature = $00000002;
  tomCharset            = $80000000;
  tomRE10Mode           = $00000001;
  tomUseAtFont          = $00000002;

  tomTextFlowMask = $0000000C;
  tomTextFlowES   = $00000000;
  tomTextFlowSW   = $00000004;
  tomTextFlowWN   = $00000008;
  tomTextFlowNE   = $0000000C;

  tomUsePassword = $00000010;

  tomNoIME   = $00080000;
  tomSelfIME = $00040000;

type
  ITextDocument        = interface;
  ITextDocumentDisp    = dispinterface;
  ITextRange           = interface;
  ITextRangeDisp       = dispinterface;
  ITextSelection       = interface;
  ITextSelectionDisp   = dispinterface;
  ITextFont            = interface;
  ITextFontDisp        = dispinterface;
  ITextPara            = interface;
  ITextParaDisp        = dispinterface;
  ITextStoryRanges     = interface;
  ITextStoryRangesDisp = dispinterface;
  ITextDocument2       = interface;
  ITextDocument2Disp   = dispinterface;
  ITextMsgFilter       = interface;

// *********************************************************************//
// Interface: ITextDocument
// Flags:     (4544) Dual NonExtensible OleAutomation Dispatchable
// GUID:      {8CC497C0-A1DF-11CE-8098-00AA0047BE5D}
// *********************************************************************//
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

// *********************************************************************//
// DispIntf:  ITextDocumentDisp
// Flags:     (4544) Dual NonExtensible OleAutomation Dispatchable
// GUID:      {8CC497C0-A1DF-11CE-8098-00AA0047BE5D}
// *********************************************************************//
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

// *********************************************************************//
// Interface: ITextRange
// Flags:     (4544) Dual NonExtensible OleAutomation Dispatchable
// GUID:      {8CC497C2-A1DF-11CE-8098-00AA0047BE5D}
// *********************************************************************//
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

// *********************************************************************//
// DispIntf:  ITextRangeDisp
// Flags:     (4544) Dual NonExtensible OleAutomation Dispatchable
// GUID:      {8CC497C2-A1DF-11CE-8098-00AA0047BE5D}
// *********************************************************************//
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

// *********************************************************************//
// Interface: ITextSelection
// Flags:     (4544) Dual NonExtensible OleAutomation Dispatchable
// GUID:      {8CC497C1-A1DF-11CE-8098-00AA0047BE5D}
// *********************************************************************//
  ITextSelection = interface(ITextRange)
    ['{8CC497C1-A1DF-11CE-8098-00AA0047BE5D}']
    function Get_Flags: Integer; safecall;
    procedure Set_Flags(pFlags: Integer); safecall;
    function  Get_Type_: Integer; safecall;
    function MoveLeft(Unit_: Integer; Count: Integer; Extend: Integer): Integer; safecall;
    function MoveRight(Unit_: Integer; Count: Integer; Extend: Integer): Integer; safecall;
    function MoveUp(Unit_: Integer; Count: Integer; Extend: Integer): Integer; safecall;
    function MoveDown(Unit_: Integer; Count: Integer; Extend: Integer): Integer; safecall;
    function HomeKey(Unit_: Integer; Extend: Integer): Integer; safecall;
    function EndKey(Unit_: Integer; Extend: Integer): Integer; safecall;
    procedure TypeText(const bstr: WideString); safecall;
    property Flags: Integer read Get_Flags write Set_Flags;
    property Type_: Integer read Get_Type_;
  end;

// *********************************************************************//
// DispIntf:  ITextSelectionDisp
// Flags:     (4544) Dual NonExtensible OleAutomation Dispatchable
// GUID:      {8CC497C1-A1DF-11CE-8098-00AA0047BE5D}
// *********************************************************************//
  ITextSelectionDisp = dispinterface
    ['{8CC497C1-A1DF-11CE-8098-00AA0047BE5D}']
    property Flags: Integer dispid 257;
    property Type_: Integer readonly dispid 258;
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

// *********************************************************************//
// Interface: ITextFont
// Flags:     (4544) Dual NonExtensible OleAutomation Dispatchable
// GUID:      {8CC497C3-A1DF-11CE-8098-00AA0047BE5D}
// *********************************************************************//
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

// *********************************************************************//
// DispIntf:  ITextFontDisp
// Flags:     (4544) Dual NonExtensible OleAutomation Dispatchable
// GUID:      {8CC497C3-A1DF-11CE-8098-00AA0047BE5D}
// *********************************************************************//
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

// *********************************************************************//
// Interface: ITextPara
// Flags:     (4544) Dual NonExtensible OleAutomation Dispatchable
// GUID:      {8CC497C4-A1DF-11CE-8098-00AA0047BE5D}
// *********************************************************************//
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

// *********************************************************************//
// DispIntf:  ITextParaDisp
// Flags:     (4544) Dual NonExtensible OleAutomation Dispatchable
// GUID:      {8CC497C4-A1DF-11CE-8098-00AA0047BE5D}
// *********************************************************************//
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

// *********************************************************************//
// Interface: ITextStoryRanges
// Flags:     (4544) Dual NonExtensible OleAutomation Dispatchable
// GUID:      {8CC497C5-A1DF-11CE-8098-00AA0047BE5D}
// *********************************************************************//
  ITextStoryRanges = interface(IDispatch)
    ['{8CC497C5-A1DF-11CE-8098-00AA0047BE5D}']
    function _NewEnum: IUnknown; safecall;
    function Item(Index: Integer): ITextRange; safecall;
    function Get_Count: Integer; safecall;
    property Count: Integer read Get_Count;
  end;

// *********************************************************************//
// DispIntf:  ITextStoryRangesDisp
// Flags:     (4544) Dual NonExtensible OleAutomation Dispatchable
// GUID:      {8CC497C5-A1DF-11CE-8098-00AA0047BE5D}
// *********************************************************************//
  ITextStoryRangesDisp = dispinterface
    ['{8CC497C5-A1DF-11CE-8098-00AA0047BE5D}']
    function _NewEnum: IUnknown; dispid -4;
    function Item(Index: Integer): ITextRange; dispid 0;
    property Count: Integer readonly dispid 2;
  end;

// *********************************************************************//
// Interface: ITextDocument2
// Flags:     (4544) Dual NonExtensible OleAutomation Dispatchable
// GUID:      {01C25500-4268-11D1-883A-3C8B00C10000}
// *********************************************************************//
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

// *********************************************************************//
// DispIntf:  ITextDocument2Disp
// Flags:     (4544) Dual NonExtensible OleAutomation Dispatchable
// GUID:      {01C25500-4268-11D1-883A-3C8B00C10000}
// *********************************************************************//
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

// *********************************************************************//
// Interface: ITextMsgFilter
// Flags:     (128) NonExtensible
// GUID:      {A3787420-4267-11D1-883A-3C8B00C10000}
// *********************************************************************//
  ITextMsgFilter = interface(IUnknown)
    ['{A3787420-4267-11D1-883A-3C8B00C10000}']
    function  AttachDocument(var hwnd: TGUID; const pTextDoc: ITextDocument2; const punk: IUnknown): HResult; stdcall;
    function  HandleMessage(var pmsg: SYSUINT; var pwparam: UINT_PTR; var plparam: LONG_PTR; 
                            out plres: LONG_PTR): HResult; stdcall;
    function  AttachMsgFilter(const pMsgFilter: ITextMsgFilter): HResult; stdcall;
  end;


implementation

end.
