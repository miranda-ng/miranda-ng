////////////////////////////////////////////////////////////////////////////////
// All code below is exclusively owned by author of Chess4Net - Pavel Perminov
// (packpaul@mail.ru, packpaul1@gmail.com).
// Any changes, modifications, borrowing and adaptation are a subject for
// explicit permition from the owner.

unit URLVersionQueryUnit;

interface

uses
  Classes;

type
  TApplicationID = (aidAnalyzer = 1, aidSkype = 2);
  TOperatingSystemID = (osidWindows = 1, osidLinux = 2);

  TURLVersionQuery = class;

  TQueryReadyEvent = procedure(Sender: TURLVersionQuery) of object;

  TURLVersionQuery = class(TDataModule)
  private
    m_iLastVersion: integer;
    m_wstrInfo: WideString;
    FQueryReadyEvent: TQueryReadyEvent;
    procedure FDoQueryReady;
    function FQuery(const strURL: string): string;
    function FGetURL(ApplicationID: TApplicationID; iVersion: integer;
      OperatingSystemID: TOperatingSystemID): string;
    procedure FParseResponse(const strResponse: string);
  public
    constructor Create; reintroduce;
    procedure Query(ApplicationID: TApplicationID; iVersion: integer;
      OperatingSystemID: TOperatingSystemID);
    property LastVersion: integer read m_iLastVersion;
    property Info: WideString read m_wstrInfo;
    property OnQueryReady: TQueryReadyEvent read FQueryReadyEvent write FQueryReadyEvent;
  end;

implementation

{$R *.dfm}

uses
  Forms, SysUtils, StrUtils,
  //
  XIE;

type
  TQueryThread = class(TThread)
  private
    m_URLVersionQuery: TURLVersionQuery;
    m_strURL: string;
    m_strResponse: string;
    procedure FNotifyOnResponse;
  protected
    procedure Execute; override;
  public
    constructor Create(AURLVersionQuery: TURLVersionQuery; const strURL: string);
  end;

////////////////////////////////////////////////////////////////////////////////
// TURLVersionQuery

constructor TURLVersionQuery.Create;
begin
  inherited Create(Application);
end;

procedure TURLVersionQuery.FDoQueryReady;
begin
  if (Assigned(FQueryReadyEvent)) then
    FQueryReadyEvent(self);
end;


procedure TURLVersionQuery.Query(ApplicationID: TApplicationID; iVersion: integer;
  OperatingSystemID: TOperatingSystemID);
begin
  TQueryThread.Create(self, FGetURL(ApplicationID, iVersion, OperatingSystemID));
end;


function TURLVersionQuery.FQuery(const strURL: string): string;
begin
  with TIEWrapper.Create do
  try
    Result := OpenRequest(strURL);
  finally
    Free;
  end;
end;


function TURLVersionQuery.FGetURL(ApplicationID: TApplicationID; iVersion: integer;
  OperatingSystemID: TOperatingSystemID): string;
begin
  Result := Format('http://chess4net.ru/stat.php?app=%d&ver=%d&os=%d',
    [Ord(ApplicationID), iVersion, Ord(OperatingSystemID)]);
end;


procedure TURLVersionQuery.FParseResponse(const strResponse: string);

  procedure NSplit(const str: string; out strlList: TStringList);
  var
    iPosPrev, iPosNext: integer;
    strSub: string;
  begin
    strlList := TStringList.Create;

    strSub := '';
    iPosPrev := 1;

    while (iPosPrev <= Length(str)) do
    begin
      iPosNext := iPosPrev;

      iPosNext := PosEx(';', str, iPosNext);

      if (iPosNext = 0) then
        iPosNext := MaxInt - 1;

      strSub := strSub + Copy(str, iPosPrev, iPosNext - iPosPrev);
      if ((iPosNext < Length(str)) and (str[iPosNext + 1] = ';')) then
      begin
        strSub := strSub + ';';
        iPosPrev := iPosNext + 2;
        continue;
      end;

      strlList.Append(strSub);
      strSub := '';

      iPosPrev := iPosNext + 1;
    end;

  end;

var
  strl: TStringList;
begin // .FParseResponse
  NSplit(strResponse, strl);
  try
    m_iLastVersion := StrToIntDef(strl.Values['Last version'], 0);
    m_wstrInfo := strl.Values['Info'];
  finally
    strl.Free;
  end;
end;

////////////////////////////////////////////////////////////////////////////////
// TQueryThread

constructor TQueryThread.Create(AURLVersionQuery: TURLVersionQuery; const strURL: string);
begin
  m_URLVersionQuery := AURLVersionQuery;
  m_strURL := strURL;

  inherited Create(TRUE);
  FreeOnTerminate := TRUE;

  Resume;
end;


procedure TQueryThread.Execute;
begin
{$IFNDEF TESTING}
  m_strResponse := m_URLVersionQuery.FQuery(m_strURL);
{$ELSE}
  m_strResponse := 'Last version=201102;Info=Version 2011.2 is available'#10'TEST> You can download it from http://chess4net.ru <TEST';
{$ENDIF}
  Synchronize(FNotifyOnResponse);
end;


procedure TQueryThread.FNotifyOnResponse;
begin
  m_URLVersionQuery.FParseResponse(m_strResponse);
  m_URLVersionQuery.FDoQueryReady;
end;

end.
