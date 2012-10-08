////////////////////////////////////////////////////////////////////////////////
// All code below is exclusively owned by author of Chess4Net - Pavel Perminov
// (packpaul@mail.ru, packpaul1@gmail.com).
// Any changes, modifications, borrowing and adaptation are a subject for
// explicit permition from the owner.

unit LocalizerUnit;

interface

uses
  SysUtils, TntSysUtils, Classes, TntClasses;

type
  // A singletone that is used to localize interface
  ILocalizable = interface
    procedure Localize;
  end;

  ELocalizer = class(Exception);
  TLocalizer = class
  private
    m_IniFileName: TFileName;
    m_iLangugesCount: integer;
    m_wstrarLanguageNames: array of WideString;
    m_wstrlLabels, m_wstrlMessages: TTntStringList;
    m_iActiveLanguage: integer;
    m_Subscribers: TList;

    constructor Create;
    procedure FReadLanguages;
    function FGetLangaugeName(iIndex: integer): WideString;
    procedure FSetActiveLanguage(iIndex: integer);
    function FReadLanguageData(iIndex: integer): boolean;
    procedure FNotifySubscribers;

  public
    destructor Destroy; override;
    class function Instance: TLocalizer;
    class procedure FreeInstance; reintroduce;
    property LanguagesCount: integer read m_iLangugesCount;
    // Returns a language name depending on its index
    function GetLabel(iIndex: integer): WideString;
    function GetMessage(iIndex: integer): WideString; overload;
    function GetMessage(iIndex: integer; wstrarParams: array of const): WideString; overload;
    procedure AddSubscriber(const Subscriber: ILocalizable);
    procedure DeleteSubscriber(const Subscriber: ILocalizable);
    property LanguageName[iIndex: integer]: WideString read FGetLangaugeName;
    property ActiveLanguage: integer read m_iActiveLanguage write FSetActiveLanguage;
  end;

implementation

uses
  StrUtils, Forms, TntIniFiles,
  //
  GlobalsUnit;

const
  LOCALIZER_INI_FILE = 'Lang.ini';

  // Labels are used to name interface controls s.a. TLabel and the like
  DEFAULT_LABELS: array[0..69] of WideString =
    ('Look & Feel Options', {0}
     'Animate Move:',
     'No',
     'Slowly',
     'Quickly',
     'Highlight Last Move',
     'Flash on incoming move',
     'Show Coordinates',
     'Stay Always on Top',
     'Extra Exit on ESC',
     'GUI Language:', {10}
     '&OK',
     '&Cancel',
     'White   ',
     'White ',
     'W ',
     'Black   ',
     'Black ',
     'B ',
     'Connecting...',
     'Wait until the connection is completed.', {20}
     '&Abort',
     'Press button to continue the game.',
     '&Continue',
     'Game Options',
     'Time Control',
     'Equal time for both players',
     'Your time',
     'Opponent''s time',
     'Unlimited',
     'Minutes per game:', {30}
     'Increment in seconds:',
     'Training Mode',
     'Enabled',
     'External base:',
     'Use user base',
     'Game can be paused',
     'Game can be adjourned',
     'Allow takebacks to your partner',
     'Auto Flag',
     'Yes', {40}
     'No',
     'OK',
     'Cancel',
     'Abort',
     'Retry',
     'Ignore',
     'All',
     'NoToAll',
     'YesToAll',
     'Help', {50}
     'Start Adjourned Game',
     'Start Standart Game',
     'Start PP Random Game',
     'Change Color',
     'Game Options...',
     'Look && Feel Options...',
     'About...',
     'Abort',
     'Draw',
     'Resign', {60}
     'Adjourn',
     'Pause',
     'Takeback',
     'If you liked plying Chess4Net give your credits at',
     '&Close',
     'Don''t show again',
     'Transmit Game',
     'Select Skype contact',
     'Broadcast...'
    );

  // Messages are used in message boxes
  DEFAULT_MESSAGES: array[0..37] of WideString =
    ('White is checkmated. You win.', {0}
     'White is checkmated. You loose.',
     'Black is checkmated. You win.',
     'Black is checkmated. You loose.',
     'It''s stalemate. No one wins.',
     'Your opponent leaves.',
     'Your opponent leaves. The game is aborted.',
     'An error during connection occured.',
     'Your opponent is using an older version of Chess4Net.' + sLineBreak +
       'Most of functionality will be not available.' + sLineBreak +
       'Please, ask him/her to update the client.',
     'The current version of Chess4Net is incompatible with the one of your partner.' + sLineBreak +
       'Please check the versions.',
     'Draw?', {10}
     'Can we abort the game?',
     'I resign. You win this game. Congratulations!',
     'The game is aborted.',
     'Sorry, but we have to finish this game.',
     'The game is drawn.',
     'No draw, sorry.',
     'You forfeited on time.',
     'Your opponent forfeited on time.',
     'Can we pause the game?',
     'No pause, sorry.', {20}
     'May I take back last move?',
     'Can we adjourn this game?',
     'No adjourns, sorry.',
     'Sorry, no takebacks!',
     'Do you really want to resign?',
     'Do you want to exit?',
     'The game is adjourned.',
     'You are currently playing some games. Do you want to start broadcasting?',
     'Black forfeits on time.',
     'White forfeits on time.', {30}
     'White resigns.',
     'Black resigns.',
     'Game transmition is not supported by this client!',
     'Broadcaster leaves. Transmition will be closed.',
     'Stalemate.',
     'White is checkmated.',
     'Black is checkmated.'
    );

var
  LocalizerInstance: TLocalizer = nil;

////////////////////////////////////////////////////////////////////////////////
// TLocalizer

constructor TLocalizer.Create;
begin
  m_IniFileName := Chess4NetPath + LOCALIZER_INI_FILE;

  m_wstrlLabels := TTntStringList.Create;
  m_wstrlMessages := TTntStringList.Create;

  m_Subscribers := TList.Create;

  m_iActiveLanguage := -1;
  FReadLanguages;
  FReadLanguageData(-1);
end;


destructor TLocalizer.Destroy;
begin
  Finalize(m_wstrarLanguageNames);

  m_Subscribers.Free;

  m_wstrlMessages.Free;
  m_wstrlLabels.Free;

  inherited;
end;


class function TLocalizer.Instance: TLocalizer;
begin
  if (not Assigned(LocalizerInstance)) then
    LocalizerInstance := TLocalizer.Create;
  Result := LocalizerInstance;
end;


class procedure TLocalizer.FreeInstance;
begin
  FreeAndNil(LocalizerInstance);
end;


procedure TLocalizer.FReadLanguages;
var
  IniFile: TTntIniFile;
  i: integer;
  wstrSection: WideString;
begin
  if (not FileExists(m_IniFileName)) then
    exit;

  IniFile := TTntIniFile.Create(m_IniFileName);
  try
    // Count available languages
    Finalize(m_wstrarLanguageNames);
    i := 0;
    repeat
      wstrSection := 'Language' + IntToStr(i + 1);
      if (not IniFile.SectionExists(wstrSection)) then
        break;
      SetLength(m_wstrarLanguageNames, length(m_wstrarLanguageNames) + 1);
      m_wstrarLanguageNames[i] := IniFile.ReadString(wstrSection, 'Name', '<unknown>');
      inc(i);
    until FALSE;
    m_iLangugesCount := i;

  finally
    IniFile.Free;
  end;
end;


function TLocalizer.FGetLangaugeName(iIndex: integer): WideString;
begin
  Result := m_wstrarLanguageNames[iIndex];
end;


procedure TLocalizer.FSetActiveLanguage(iIndex: integer);
begin
  if (iIndex <> m_iActiveLanguage) then
  begin
    if (FReadLanguageData(iIndex)) then
    begin
      m_iActiveLanguage := iIndex;
      FNotifySubscribers;
    end
    else
      m_iActiveLanguage := -1;
  end;
end;


function TLocalizer.FReadLanguageData(iIndex: integer): boolean;

  function NInsertLineFeeds(const wstrSource: WideString): WideString;
  var
    iPos, iOffset: integer;
  begin
    Result := '';
    iOffset := 1;
    repeat
      iPos := PosEx('/n', wstrSource, iOffset);
      if (iPos = 0) then
        break;
      if ((iPos = 1) or (wstrSource[iPos - 1] <> '/')) then
        Result := Result + Copy(wstrSource, iOffset, iPos - iOffset) + sLineBreak
      else
        Result := Result + Copy(wstrSource, iOffset, iPos - iOffset) + 'n'; // '//n' -> '/n'
      iOffset := iPos + 2;
    until FALSE;

    Result := Result + Copy(wstrSource, iOffset, MaxInt);
  end;

var
  IniFile: TTntIniFile;
  i: integer;
  wstrSection: WideString;
  wstrlValues: TTntStringList;
  wstrValue: WideString;
begin
  Result := FALSE;

  // Copy default values
  m_wstrlLabels.Clear;
  for i := Low(DEFAULT_LABELS) to High(DEFAULT_LABELS) do
    m_wstrlLabels.Add(DEFAULT_LABELS[i]);

  m_wstrlMessages.Clear;
  for i := Low(DEFAULT_MESSAGES) to High(DEFAULT_MESSAGES) do
    m_wstrlMessages.Add(DEFAULT_MESSAGES[i]);

  if (not FileExists(m_IniFileName)) then
    exit;    

  wstrlValues := nil;
  IniFile := TTntIniFile.Create(m_IniFileName);
  try
    wstrSection := 'Language' + IntToStr(iIndex + 1);
    if (not IniFile.SectionExists(wstrSection)) then
      exit;
      
    wstrlValues := TTntStringList.Create;
    IniFile.ReadSectionValues(wstrSection, wstrlValues);

    for i := 0 to m_wstrlLabels.Count - 1 do
    begin
      wstrValue := wstrlValues.Values['Label' + IntToStr(i)];
      if (wstrValue <> '') then
      begin
        if (wstrValue[length(wstrValue)] = '|') then // labels with a width
          wstrValue := Copy(wstrValue, 1, length(wstrValue) - 1);
        m_wstrlLabels[i] := wstrValue;
      end;
    end;
    for i := 0 to m_wstrlMessages.Count - 1 do
    begin
      wstrValue := wstrlValues.Values['Msg' + IntToStr(i)];
      if (wstrValue <> '') then
        m_wstrlMessages[i] := NInsertLineFeeds(wstrValue);
    end;

  finally
    wstrlValues.Free;
    IniFile.Free;
  end;

  Result := TRUE;
end;


function TLocalizer.GetLabel(iIndex: integer): WideString;
begin
  if (iIndex in [0..m_wstrlLabels.Count - 1]) then
    Result := m_wstrlLabels[iIndex]
  else
    ELocalizer.Create('Wrong label index!');
end;


function TLocalizer.GetMessage(iIndex: integer): WideString;
begin
  if (iIndex in [0..m_wstrlMessages.Count - 1]) then
    Result := m_wstrlMessages[iIndex]
  else
    ELocalizer.Create('Wrong message index!');
end;


function TLocalizer.GetMessage(iIndex: integer; wstrarParams: array of const): WideString;
begin
  Result := WideFormat(GetMessage(iIndex), wstrarParams)
end;


procedure TLocalizer.AddSubscriber(const Subscriber: ILocalizable);
begin
  if (m_Subscribers.IndexOf(Pointer(Subscriber)) < 0) then
    m_Subscribers.Add(Pointer(Subscriber));
end;


procedure TLocalizer.DeleteSubscriber(const Subscriber: ILocalizable);
var
  iIndex: integer;
begin
  iIndex := m_Subscribers.IndexOf(Pointer(Subscriber));
  if (iIndex >= 0) then
    m_Subscribers.Delete(iIndex);
end;


procedure TLocalizer.FNotifySubscribers;
var
  i: integer;
begin
  for i := 0 to m_Subscribers.Count - 1 do
    ILocalizable(m_Subscribers[i]).Localize;
end;

initialization

finalization
  TLocalizer.FreeInstance;

end.
