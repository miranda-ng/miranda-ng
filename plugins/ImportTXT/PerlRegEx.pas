(**********************************************************
*                                                         *
*     Perl Regular Expressions                            *
*                                                         *
*     Delphi wrapper around PCRE  -  http://www.pcre.org  *
*                                                         *
*     Copyright (C) 1999-2006 Jan Goyvaerts               *
*                                                         *
*     Design & implementation Jan Goyvaerts 1999-2006     *
*                                                         *
*                                                         *
*     Shorted and added partial ability for PCRE 7.0      *
*                         by Abyss                        *
**********************************************************)

unit PerlRegEx;

interface



uses SysUtils;

type
  TPerlRegExOptions = set of (
    preCaseLess,       // /i -> Case insensitive
    preMultiLine,      // /m -> ^ and $ also match before/after a newline, not just at the beginning and the end of the string
    preSingleLine,     // /s -> Dot matches any character, including \n (newline). Otherwise, it matches anything except \n
    preExtended,       // /x -> Allow regex to contain extra whitespace, newlines and Perl-style comments, all of which will be filtered out
    preAnchored,       // /A -> Successful match can only occur at the start of the subject or right after the previous match
    preDollarEndOnly,  // /E
    preExtra,          // /X
    preUnGreedy,       // Repeat operators (+, *, ?) are not greedy by default
                       //   (i.e. they try to match the minimum number of characters instead of the maximum)
    preUTF8            // UTF8
      );
type
  TPerlRegExState = set of (
    preNotBOL,         // Not Beginning Of Line: ^ does not match at the start of Subject
    preNotEOL,         // Not End Of Line: $ does not match at the end of Subject
    preNotEmpty        // Empty matches not allowed
  );

const
  // Maximum number of subexpressions (backreferences)
  // Subexpressions are created by placing round brackets in the regex, and are referenced by \1, \2, ...
  // In Perl, they are available as $1, $2, ... after the regex matched; with TPerlRegEx, use the Subexpressions property
  // You can also insert \1, \2, ... in the Replacement string; \0 is the complete matched expression
  MAX_SUBEXPRESSIONS = 99;


type
  TPerlRegEx = class
  private    // *** Property storage, getters and setters
    FCompiled, FStudied: Boolean;
    FOptions: TPerlRegExOptions;
    FState: TPerlRegExState;
    FRegEx, FSubject: string;
    FStart, FStop: Integer;
    function GetMatchedExpression: string;
    function GetMatchedExpressionLength: Integer;
    function GetMatchedExpressionOffset: Integer;
    procedure SetOptions(Value: TPerlRegExOptions);
    procedure SetRegEx(const Value: string);
    function GetSubExpressionCount: Integer;
    function GetSubExpressions(Index: Integer): string;
    function GetSubExpressionLengths(Index: Integer): Integer;
    function GetSubExpressionOffsets(Index: Integer): Integer;
    procedure SetSubject(const Value: string);
    procedure SetStart(const Value: Integer);
    procedure SetStop(const Value: Integer);
    function GetFoundMatch: Boolean;
  private    // *** Variables used by pcrelib.dll
    Offsets: array[0..(MAX_SUBEXPRESSIONS+1)*3] of Integer;
    OffsetCount: Integer;
    pcreOptions: Integer;
    pattern, hints, chartable: Pointer;
    FSubjectPChar: PChar;
  protected
    procedure CleanUp;
        // Dispose off whatever we created, so we can start over. Called automatically when needed, so it is not made public
  public
    constructor Create; 
        // Come to life
    destructor Destroy; override;
        // Clean up after ourselves
    class function EscapeRegExChars(const S: string): string;
        // Escapes regex characters in S so that the regex engine can be used to match S as plain text
    procedure Compile;
        // Compile the regex. Called automatically by Match
    procedure Study;
        // Study the regex. Studying takes time, but will make the execution of the regex a lot faster.
        // Call study if you will be using the same regex many times
    function Match: Boolean;
        // Attempt to match the regex
    function MatchAgain: Boolean;
        // Attempt to match the regex to the remainder of the string after the previous match
        // To avoid problems (when using ^ in the regex), call MatchAgain only after a succesful Match()
    function NamedSubExpression(const SEName: string): Integer;
        // Returns the index of the named group SEName
    function Config (What: integer): Integer;
        //This  function makes it possible for a client program
        // to find out which optional features are  available  in  the
        //version  of  the PCRE library it is using.
    property Compiled: Boolean read FCompiled;
        // True if the RegEx has already been compiled.
    property FoundMatch: Boolean read GetFoundMatch;
        // Returns True when MatchedExpression* and SubExpression* indicate a match
    property Studied: Boolean read FStudied;
        // True if the RegEx has already been studied
    property MatchedExpression: string read GetMatchedExpression;
        // The matched string
    property MatchedExpressionLength: Integer read GetMatchedExpressionLength;
        // Length of the matched string
    property MatchedExpressionOffset: Integer read GetMatchedExpressionOffset;
        // Character offset in the Subject string at which the matched substring starts
    property Start: Integer read FStart write SetStart;
        // Starting position in Subject from which MatchAgain begins
    property Stop: Integer read FStop write SetStop;
        // Last character in Subject that Match and MatchAgain search through
    property State: TPerlRegExState read FState write FState;
        // State of Subject
    property SubExpressionCount: Integer read GetSubExpressionCount;
        // Number of matched subexpressions
    property SubExpressions[Index: Integer]: string read GetSubExpressions;
        // Matched subexpressions after a regex has been matched
    property SubExpressionLengths[Index: Integer]: Integer read GetSubExpressionLengths;
        // Lengths of the subexpressions
    property SubExpressionOffsets[Index: Integer]: Integer read GetSubExpressionOffsets;
        // Character offsets in the Subject string of the subexpressions
    property Subject: string read FSubject write SetSubject;
        // The string on which Match() will try to match RegEx
  published
    property Options: TPerlRegExOptions read FOptions write SetOptions;
        // Options
    property RegEx: string read FRegEx write SetRegEx;
        // The regular expression to be matched
  end;



implementation

         { ********* pcrelib.dll imports ********* }

const
  PCRE_CASELESS        = $00000001;
  PCRE_MULTILINE       = $00000002;
  PCRE_SINGLELINE      = $00000004;
  PCRE_EXTENDED        = $00000008;
  PCRE_ANCHORED        = $00000010;
  PCRE_DOLLAR_ENDONLY  = $00000020;
  PCRE_EXTRA           = $00000040;
  PCRE_NOTBOL          = $00000080;
  PCRE_NOTEOL          = $00000100;
  PCRE_UNGREEDY        = $00000200;
  PCRE_NOTEMPTY        = $00000400;
  PCRE_UTF8            = $00000800;
  PCRE_NO_AUTO_CAPTURE = $00001000;
  PCRE_NO_UTF8_CHECK   = $00002000;
  PCRE_AUTO_CALLOUT    = $00004000;
  PCRE_PARTIAL         = $00008000;
  PCRE_DFA_SHORTEST    = $00010000;
  PCRE_DFA_RESTART     = $00020000;
  PCRE_FIRSTLINE       = $00040000;
  PCRE_DUPNAMES        = $00080000;
  PCRE_NEWLINE_CR      = $00100000;
  PCRE_NEWLINE_LF      = $00200000;
  PCRE_NEWLINE_CRLF    = $00300000;
  PCRE_NEWLINE_ANY     = $00400000;

  // Exec error codes
  PCRE_ERROR_NOMATCH        = -1;
  PCRE_ERROR_NULL           = -2;
  PCRE_ERROR_BADOPTION      = -3;
  PCRE_ERROR_BADMAGIC       = -4;
  PCRE_ERROR_UNKNOWN_OPCODE = -5;
  PCRE_ERROR_UNKNOWN_NODE   = -5;  // For backward compatibility
  PCRE_ERROR_NOMEMORY       = -6;
  PCRE_ERROR_NOSUBSTRING    = -7;
  PCRE_ERROR_MATCHLIMIT     = -8;
  PCRE_ERROR_CALLOUT        = -9;  // Never used by PCRE itself
  PCRE_ERROR_BADUTF8        =-10;
  PCRE_ERROR_BADUTF8_OFFSET =-11;
  PCRE_ERROR_PARTIAL        =-12;
  PCRE_ERROR_BADPARTIAL     =-13;
  PCRE_ERROR_INTERNAL       =-14;
  PCRE_ERROR_BADCOUNT       =-15;
  PCRE_ERROR_DFA_UITEM      =-16;
  PCRE_ERROR_DFA_UCOND      =-17;
  PCRE_ERROR_DFA_UMLIMIT    =-18;
  PCRE_ERROR_DFA_WSSIZE     =-19;
  PCRE_ERROR_DFA_RECURSE    =-20;
  PCRE_ERROR_RECURSIONLIMIT =-21;
  PCRE_ERROR_NULLWSLIMIT    =-22;
  PCRE_ERROR_BADNEWLINE     =-23;

(* Request types for pcre_config(). Do not re-arrange, in order to remain compatible. *)

  PCRE_CONFIG_UTF8                   = 0;
  PCRE_CONFIG_NEWLINE                = 1;
  PCRE_CONFIG_LINK_SIZE              = 2;
  PCRE_CONFIG_POSIX_MALLOC_THRESHOLD = 3;
  PCRE_CONFIG_MATCH_LIMIT            = 4;
  PCRE_CONFIG_STACKRECURSE           = 5;
  PCRE_CONFIG_UNICODE_PROPERTIES     = 6;
  PCRE_CONFIG_MATCH_LIMIT_RECURSION  = 7;
type
  PPChar = ^PChar;
  PInt = ^Integer;



// Functions we import from the PCRE library DLL
// Leading underscores gratuitously added by Borland C++Builder 6.0
function  pcre_maketables: PAnsiChar; cdecl; external 'pcre3.dll';
function  pcre_compile(const pattern: PChar; options: Integer; errorptr: PPChar; erroroffset: PInt;
                        const tables: PChar): Pointer; cdecl; external 'pcre3.dll';
function  pcre_exec(const pattern: Pointer; const hints: Pointer; const subject: PChar; length, startoffset: Integer;
                     options: Integer; offsets: PInt; offsetcount: Integer): Integer; cdecl; external 'pcre3.dll';
function  pcre_get_stringnumber(const pattern: Pointer; const Name: PChar): Integer; cdecl; external 'pcre3.dll';
function  pcre_study(const pattern: Pointer; options: Integer; errorptr: PPChar): Pointer; cdecl; external 'pcre3.dll';
function  pcre_fullinfo(const pattern: Pointer; const hints: Pointer; what: Integer; where: Pointer): Integer; cdecl; external 'pcre3.dll';
function  pcre_version: pchar; cdecl; external 'pcre3.dll';
function  pcre_config (what:integer; where:pointer):integer; cdecl; external 'pcre3.dll'; 
//procedure pcre_free(ptr: Pointer); cdecl; external 'pcre3.dll';




         { ********* TPerlRegEx component ********* }

procedure TPerlRegEx.CleanUp;
begin
  FCompiled := False; FStudied := False;
  pattern := nil; hints := nil;
  OffsetCount := 0;
end;

procedure TPerlRegEx.Compile;
var
  Error: PChar;
  ErrorOffset: Integer;
begin
  if FRegEx = '' then raise Exception.Create('TPerlRegEx.Compile() - Please specify a regular expression in RegEx first');
  CleanUp;
  Pattern := pcre_compile(PChar(FRegEx), pcreOptions, @Error, @ErrorOffset, chartable);
  if Pattern = nil then
    raise Exception.Create(Format('TPerlRegEx.Compile() - Error in regex at offset %d: %s', [ErrorOffset, AnsiString(Error)]));
  FCompiled := True
end;


constructor TPerlRegEx.Create;
begin
  inherited Create;
  FState := [preNotEmpty];
  chartable := pcre_maketables;
end;

destructor TPerlRegEx.Destroy;
begin
  CleanUp;
  inherited Destroy;
end;

class function TPerlRegEx.EscapeRegExChars(const S: string): string;
var
  I: Integer;
begin
  Result := S;
  I := Length(Result);
  while I > 0 do begin
    if Result[I] in ['.', '[', ']', '(', ')', '?', '*', '+', '{', '}', '^', '$', '|', '\'] then
      Insert('\', Result, I)
    else if Result[I] = #0 then begin
      Result[I] := '0';
      Insert('\', Result, I);
    end;
    Dec(I);
  end;
end;

function TPerlRegEx.GetFoundMatch: Boolean;
begin
  Result := OffsetCount > 0;
end;

function TPerlRegEx.GetMatchedExpression: string;
begin
  Assert(FoundMatch, 'REQUIRE: There must be a successful match first');
  Result := GetSubExpressions(0);
end;

function TPerlRegEx.GetMatchedExpressionLength: Integer;
begin
  Assert(FoundMatch, 'REQUIRE: There must be a successful match first');
  Result := GetSubExpressionLengths(0)
end;

function TPerlRegEx.GetMatchedExpressionOffset: Integer;
begin
  Assert(FoundMatch, 'REQUIRE: There must be a successful match first');
  Result := GetSubExpressionOffsets(0)
end;

function TPerlRegEx.GetSubExpressionCount: Integer;
begin
  Assert(FoundMatch, 'REQUIRE: There must be a successful match first');
  Result := OffsetCount-1
end;

function TPerlRegEx.GetSubExpressionLengths(Index: Integer): Integer;
begin
  Assert(FoundMatch, 'REQUIRE: There must be a successful match first');
  Assert((Index >= 0) and (Index <= SubExpressionCount), 'REQUIRE: Index <= SubExpressionCount');
  Result := Offsets[Index*2+1]-Offsets[Index*2]
end;

function TPerlRegEx.GetSubExpressionOffsets(Index: Integer): Integer;
begin
  Assert(FoundMatch, 'REQUIRE: There must be a successful match first');
  Assert((Index >= 0) and (Index <= SubExpressionCount), 'REQUIRE: Index <= SubExpressionCount');
  Result := Offsets[Index*2]
end;

function TPerlRegEx.GetSubExpressions(Index: Integer): string;
begin
  Assert(FoundMatch, 'REQUIRE: There must be a successful match first');
  if Index > SubExpressionCount then Result := ''
    else Result := Copy(FSubject, Offsets[Index*2], Offsets[Index*2+1]-Offsets[Index*2]);
end;

function TPerlRegEx.Match: Boolean;
var
  I, Opts: Integer;
begin
  if not Compiled then Compile;
  if preNotBOL in State then Opts := PCRE_NOTBOL else Opts := 0;
  if preNotEOL in State then Opts := Opts or PCRE_NOTEOL;
  if preNotEmpty in State then Opts := Opts or PCRE_NOTEMPTY;
  if FStart > FStop then OffsetCount := -1
    else OffsetCount := pcre_exec(Pattern, Hints, FSubjectPChar, FStop, 0, Opts, @Offsets[0], High(Offsets));
  Result := OffsetCount > 0;
  // Convert offsets into string indices
  if Result then begin
    for I := 0 to OffsetCount*2-1 do
      Inc(Offsets[I]);
    FStart := Offsets[1];
    if Offsets[0] = Offsets[1] then Inc(FStart); // Make sure we don't get stuck at the same position
  end;
end;

function TPerlRegEx.MatchAgain: Boolean;
var
  I, Opts: Integer;
begin
  if not Compiled then Compile;
  if preNotBOL in State then Opts := PCRE_NOTBOL else Opts := 0;
  if preNotEOL in State then Opts := Opts or PCRE_NOTEOL;
  if preNotEmpty in State then Opts := Opts or PCRE_NOTEMPTY;
  if FStart > FStop then OffsetCount := -1
    else OffsetCount := pcre_exec(Pattern, Hints, FSubjectPChar, FStop, FStart-1, Opts, @Offsets[0], High(Offsets));
  Result := OffsetCount > 0;
  // Convert offsets into string indices
  if Result then begin
    for I := 0 to OffsetCount*2-1 do
      Inc(Offsets[I]);
    FStart := Offsets[1];
    if Offsets[0] = Offsets[1] then Inc(FStart); // Make sure we don't get stuck at the same position
  end;
end;

function TPerlRegEx.NamedSubExpression(const SEName: string): Integer;
begin
  Result := pcre_get_stringnumber(Pattern, PChar(SEName));
end;

function TPerlRegEx.Config(What: integer):integer;
begin
 result:=-1;
 pcre_config(what, @result);
end;


procedure TPerlRegEx.SetOptions(Value: TPerlRegExOptions);
begin
  if (FOptions <> Value) then begin
    FOptions := Value;
    pcreOptions := 0;
    if (preCaseLess in Value) then pcreOptions := pcreOptions or PCRE_CASELESS;
    if (preMultiLine in Value) then pcreOptions := pcreOptions or PCRE_MULTILINE;
    if (preSingleLine in Value) then pcreOptions := pcreOptions or PCRE_SINGLELINE;
    if (preExtended in Value) then pcreOptions := pcreOptions or PCRE_EXTENDED;
    if (preAnchored in Value) then pcreOptions := pcreOptions or PCRE_ANCHORED;
    if (preDollarEndOnly in Value) then pcreOptions := pcreOptions or PCRE_DOLLAR_ENDONLY;
    if (preExtra in Value) then pcreOptions := pcreOptions or PCRE_EXTRA;
    if (preUnGreedy in Value) then pcreOptions := pcreOptions or PCRE_UNGREEDY;
    if (preUTF8 in Value) then pcreOptions := pcreOptions or PCRE_UTF8;
    CleanUp
  end
end;

procedure TPerlRegEx.SetRegEx(const Value: string);
begin
  if FRegEx <> Value then begin
    FRegEx := Value;
    CleanUp
  end
end;

procedure TPerlRegEx.SetStart(const Value: Integer);
begin
  if Value < 1 then FStart := 1
  else FStart := Value;
  // If FStart > Length(Subject), MatchAgain() will simply return False
end;

procedure TPerlRegEx.SetStop(const Value: Integer);
begin
  if Value > Length(Subject) then FStop := Length(Subject)
    else FStop := Value;
end;

procedure TPerlRegEx.SetSubject(const Value: string);
begin
  FSubject := Value;
  FSubjectPChar := PChar(Value);
  FStart := 1;
  FStop := Length(Subject);
  OffsetCount := 0;
end;



procedure TPerlRegEx.Study;
var
  Error: PChar;
begin
  if not FCompiled then Compile;
  Hints := pcre_study(Pattern, 0, @Error);
  if Error <> nil then raise Exception.Create('TPerlRegEx.Study() - Error studying the regex: ' + AnsiString(Error));
  FStudied := True
end;


end.