{ ################################################################################### }
{ #                                                                                 # }
{ #  IMDownloader                                                                   # }
{ #                                                                                 # }
{ #  Base on Downloader www.webdelphi.ru                                            # }
{ #  http://www.webdelphi.ru/2009/10/mnogopotochnost-v-svoix-prilozheniyax-chast-2/ # }
{ #                                                                                 # }
{ #  License: GPLv3                                                                 # }
{ #                                                                                 # }
{ #  Author: Grigorev Michael (icq: 161867489, email: sleuthhound@gmail.com)        # }
{ #                                                                                 # }
{ #  Доработки по сравнению с Downloader:                                           # }
{ #  + Добавлен работа через прокси (свойства Proxy, ProxyBypass,                   # }
{ #    AuthUserName, AuthPassword, ProxyAuthUserName, ProxyAuthPassword)                                                  # }
{ #  + Добавлено свойство DirPath - Путь где идет поиск файла и проверка его MD5.   # }
{ #    (Имя файла и его MD5 узнаются из заголовка ответа сервера). Если MD5 файла   # }
{ #    на диске равно MD5 файла в заголовке ответа, то скачивание не начинается.    # }
{ #  + Событие OnHeaders - Получение заголовка ответа сервера.                      # }
{ #  + Событие OnMD5Checked - Подсчет MD5 суммы скаченного файла.                   # }
{ #                                                                                 # }
{ ################################################################################### }

unit uIMDownloader;

interface

uses Classes, WinInet, SysUtils, Dialogs, Windows, Forms, uMD5;

const
 Accept = 'Accept: */*' + sLineBreak;
 ProxyConnection = 'Proxy-Connection: Keep-Alive' + sLineBreak;
 Lang = 'Accept-Language: ru' + sLineBreak;
 Agent =
   'User-Agent: Mozilla/4.0 (compatible; MSIE 6.0; ' +
   'Windows NT 5.1; SV1; .NET CLR 2.0.50727)' + sLineBreak;

type
  PMemoryStream = ^TMemoryStream;
  TIMDownloadError = (deInternetOpen, deInternetOpenUrl, deDownloadingFile, deConnect, deRequest);
  TErrorEvent = procedure(Sender: TObject; E: TIMDownloadError) of object;
  TDownloadingEvent = procedure(Sender: TObject; AcceptedSize, MaxSize: Cardinal) of object;
  THeadersEvent = procedure(Sender: TObject; Headers: String) of object;
  TMD5Event = procedure(Sender: TObject; MD5Correct, SizeCorrect: Boolean; MD5Str: String) of object;

  TIMDownloadThread = class(TThread)
  private
    fURL: String;
    fProxy: String;
    fProxyBypass: String;
    fAuthUserName: String;
    fAuthPassword: String;
    fProxyAuthUserName: String;
    fProxyAuthPassword: String;
    fDirPath: String;
    MemoryStream: TMemoryStream;
    Err: TIMDownloadError;
    fError: TErrorEvent;
    fAccepted: TNotifyEvent;
    fBreak: TNotifyEvent;
    fDownloading: TDownloadingEvent;
    fHeaders: THeadersEvent;
    AcceptedSize: Cardinal;
    AllSize: Cardinal;
    Headers: String;
    MD5Str: String;
    MD5Correct: Boolean;
    SizeCorrect: Boolean;
    fMD5: TMD5Event;
    procedure toError;
    procedure toHeaders;
    procedure toDownloading;
    procedure toAccepted;
    procedure toBreak;
    procedure toMD5;
    procedure Complete;
    function ErrorResult(E: Boolean; eType: TIMDownloadError): Boolean;
    function GetQueryInfo(hRequest: Pointer; Flag: Integer): String;
  protected
    procedure Execute; override;
  public
    constructor Create(CreateSuspennded: Boolean; const URL, Proxy, ProxyBypass, AuthUserName, AuthPassword, ProxyAuthUserName, ProxyAuthPassword, DirPath: String; Stream: PMemoryStream);
    property URL: string read fURL;
    property Proxy: string read fProxy;			                          // Список прокси
    property ProxyBypass: string read fProxyBypass;                   // Дополниотельный список прокси
    property AuthUserName: string read fAuthUserName;                 // Логин для Authorization: Basic
    property AuthPassword: string read fAuthPassword;                 // Пароль для Authorization: Basic
    property ProxyAuthUserName: string read fProxyAuthUserName;       // Логин для прокси
    property ProxyAuthPassword: string read fProxyAuthPassword;       // Пароль для прокси
    property DirPath: string read fDirPath write fDirPath;            // Директория в которой будут проверяться MD5 файлов
    property OnError: TErrorEvent read fError write fError;
    property OnAccepted: TNotifyEvent read fAccepted write fAccepted;
    property OnBreak: TNotifyEvent read fBreak write fBreak;
    property OnDownloading: TDownloadingEvent read fDownloading write fDownloading;
    property OnHeaders: THeadersEvent read fHeaders write fHeaders;
    property OnMD5Checked: TMD5Event read fMD5 write fMD5;
  end;

  TIMDownloader = class(TComponent)
  private
    fOutStream: TMemoryStream;
    fURL: String;
    fProxy: String;
    fProxyBypass: String;
    fAuthUserName: String;
    fAuthPassword: String;
    fProxyAuthUserName: String;
    fProxyAuthPassword: String;
    fDirPath: String;
    Downloader: TIMDownloadThread;
    fOnError: TErrorEvent;
    fOnAccepted: TNotifyEvent;
    fOnBreak: TNotifyEvent;
    fOnStartDownload: TNotifyEvent;
    fInDowloading: Boolean;
    fAcceptedSize: Cardinal;
    fMyHeaders: String;
    fMyMD5Str: String;
    fMyMD5Correct: Boolean;
    fMySizeCorrect: Boolean;
    fHeaders: THeadersEvent;
    fDownloading: TDownloadingEvent;
    fMD5: TMD5Event;
    procedure AcceptDownload(Sender: TObject);
    procedure Break_Download(Sender: TObject);
    procedure Downloading(Sender: TObject; AcceptedSize, MaxSize: Cardinal);
    procedure GetHeaders(Sender: TObject; Headers: String);
    procedure GetMD5(Sender: TObject; MD5Correct, SizeCorrect: Boolean; MD5Str: String);
    procedure ErrorDownload(Sender: TObject; Error: TIMDownloadError);
  public
    procedure Download;
    procedure BreakDownload;
    property OutStream: TMemoryStream read fOutStream;
    property InDowloading: Boolean read fInDowloading;
    property AcceptedSize: Cardinal read fAcceptedSize;
    property MyHeaders: String read fMyHeaders;
    property MyMD5Str: String read fMyMD5Str;
    property MyMD5Correct: Boolean read fMyMD5Correct;
    property MySizeCorrect: Boolean read fMySizeCorrect;
  published
    property URL: string read fURL write fURL;
    property Proxy: string read fProxy write fProxy;			                               // Список прокси
    property ProxyBypass: string read fProxyBypass write fProxyBypass;                   // Дополниотельный список прокси
    property AuthUserName: string read fAuthUserName write fAuthUserName;                // Логин для Authorization: Basic
    property AuthPassword: string read fAuthPassword write fAuthPassword;                // Пароль для Authorization: Basic
    property ProxyAuthUserName: string read fProxyAuthUserName write fProxyAuthUserName; // Логин для прокси
    property ProxyAuthPassword: string read fProxyAuthPassword write fProxyAuthUserName; // Пароль для прокси
    property DirPath: string read fDirPath write fDirPath;  	                           // Директория в которой будут проверяться MD5 файлов
    property OnError: TErrorEvent read fOnError write fOnError;
    property OnAccepted: TNotifyEvent read fOnAccepted write fOnAccepted;
    property OnHeaders: THeadersEvent read fHeaders write fHeaders;
    property OnMD5Checked: TMD5Event read fMD5 write fMD5;
    property OnDownloading: TDownloadingEvent read fDownloading write fDownloading;
    property OnStartDownload: TNotifyEvent read fOnStartDownload write fOnStartDownload;
    property OnBreak: TNotifyEvent read fOnBreak write fOnBreak;
  end;

{$R IMDownloader.dcr}

procedure Register;

implementation

procedure Register;
begin
  RegisterComponents('IM-History', [TIMDownloader]);
end;

procedure TIMDownloadThread.toHeaders;
begin
  if Assigned(fHeaders) then
    fHeaders(Self, Headers);
end;

procedure TIMDownloadThread.toMD5;
begin
  if Assigned(fMD5) then
    fMD5(Self, MD5Correct, SizeCorrect, MD5Str);
end;

procedure TIMDownloadThread.toDownloading;
begin
  if Assigned(fDownloading) then
    fDownloading(Self, AcceptedSize, AllSize);
end;

procedure TIMDownloadThread.toAccepted;
begin
  if Assigned(fAccepted) then
    fAccepted(Self);
end;

procedure TIMDownloadThread.toBreak;
begin
  if Assigned(fBreak) then
    fBreak(Self);
end;

procedure TIMDownloadThread.Complete;
begin
  if Terminated then
    Synchronize(toBreak)
  else
    Synchronize(toAccepted);
end;

procedure TIMDownloadThread.toError;
begin
  if Assigned(fError) then
    OnError(Self, err);
end;

function TIMDownloadThread.ErrorResult(E: Boolean; eType: TIMDownloadError): Boolean;
begin
  Result := E;
  if E then
  begin
    err := eType;
    toError;
  end;
end;

function TIMDownloadThread.GetQueryInfo(hRequest: Pointer; Flag: Integer): String;
var
  Code: String;
  Size, Index: Cardinal;
begin
  SetLength(Code, 8); // Достаточная длина для чтения статус-кода
  Size := Length(Code);
  Index := 0;
  if HttpQueryInfo(hRequest, Flag ,PChar(Code), Size, Index) then
    Result := Code
  else
  if GetLastError = ERROR_INSUFFICIENT_BUFFER then // Увеличиваем буффер
    begin
      SetLength(Code, Size);
      Size := Length(Code);
      if HttpQueryInfo(hRequest, Flag, PChar(Code), Size, Index) then
        Result := Code;
    end
  else
  begin
    //FErrorCode := GetLastError;
    Result := '';
  end;
end;

procedure TIMDownloadThread.Execute;
var
  Buffer: Array [0 .. 1024] of Byte;
  BytesRead: Cardinal;
  FSession, FConnect, FRequest: hInternet;
  dwBuffer: array [0 .. 1024] of Byte;
  dwBufferLen, dwIndex: DWORD;
  FHost, FScript, SRequest, ARequest: String;
  ProxyReqRes, ProxyReqLen: Cardinal;
  TempHeaders, TempMD5, DownloadsFileName, DownloadsFileMD5: String;

 function DelHttp(sURL: String): String;
 var
   HttpPos: Integer;
 begin
   HttpPos := Pos('http://', sURL);
   if HttpPos > 0 then Delete(sURL, HttpPos, 7);
   Result := Copy(sURL, 1, Pos('/', sURL) - 1);
   if Result = '' then Result := sURL;
 end;

 function ParseHeadersMD5andSize(HeaderStr: String): String;
 var
   HeadersStrList: TStringList;
   I: Integer;
   Size: String;
   Ch: Char;
   ResultFilename, ResultMD5Sum, ResultHeaders: String;
   ResultFileSize: Integer;
 begin
   ResultFilename := 'Test';
   ResultMD5Sum := '00000000000000000000000000000000';
   ResultFileSize := 0;
   // Создаем TStringList
   HeadersStrList := TStringList.Create;
   HeadersStrList.Clear;
   HeadersStrList.Text := HeaderStr;
   HeadersStrList.Delete(HeadersStrList.Count-1); // Последний элемент содержит всегда CRLF
   if HeadersStrList.Count > 0 then
   begin
     for I := 0 to HeadersStrList.Count - 1 do
     begin
       // Парсим строку вида
       // Content-Disposition: attachment; filename="ИМЯ-ФАЙЛА"
       // Такую строку вставляет в заголовок HTTP-запроса
       // только мой скрипт get.php
       if pos('content-disposition', LowerCase(HeadersStrList[I])) > 0 then
       begin
         ResultFilename := HeadersStrList[I];
         Delete(ResultFilename, 1, Pos('"', HeadersStrList[I]));
         Delete(ResultFilename, Length(ResultFilename),1);
         //LogMemo.Lines.Add('Filename: '+ResultFilename);
       end;
       // Парсим строку вида
       // Content-MD5Sum: MD5
       // Такую строку вставляет в заголовок HTTP-запроса
       // только мой скрипт get.php
       if pos('content-md5sum', LowerCase(HeadersStrList[I])) > 0 then
       begin
         ResultMD5Sum := HeadersStrList[I];
         Delete(ResultMD5Sum, 1, Pos(':', HeadersStrList[I]));
         Delete(ResultMD5Sum, 1,1);
       end;
       // Парсим строку вида
       // Content-Length: РАЗМЕР
       if pos('content-length', LowerCase(HeadersStrList[i])) > 0 then
       begin
         Size := '';
         for Ch in HeadersStrList[I]do
           if Ch in ['0'..'9'] then
             Size := Size + Ch;
         ResultFileSize := StrToIntDef(Size,-1);
       end;
     end;
     Result := ResultFilename + '|' + LowerCase(ResultMD5Sum) + '|' + IntToStr(ResultFileSize) + '|';
   end;
   HeadersStrList.Free;
 end;

 { Функция разбивает строку S на слова, разделенные символами-разделителями,
 указанными в строке Sep. Функция возвращает первое найденное слово, при
 этом из строки S удаляется начальная часть до следующего слова }
 function Tok(Sep: String; var S: String): String;

   function isoneof(c, s: string): Boolean;
   var
     iTmp: integer;
   begin
     Result := False;
     for iTmp := 1 to Length(s) do
     begin
       if c = Copy(s, iTmp, 1) then
       begin
         Result := True;
         Exit;
       end;
     end;
   end;

 var
   c, t: String;
 begin
   if s = '' then
   begin
     Result := s;
     Exit;
   end;
   c := Copy(s, 1, 1);
   while isoneof(c, sep) do
   begin
     s := Copy(s, 2, Length(s) - 1);
     c := Copy(s, 1, 1);
   end;
   t := '';
   while (not isoneof(c, sep)) and (s <> '') do
   begin
     t := t + c;
     s := Copy(s, 2, length(s) - 1);
     c := Copy(s, 1, 1);
   end;
   Result := t;
 end;

begin
  // Инициализируем WinInet
  if fProxy = '' then
    FSession := InternetOpen('IM-History Download Master', INTERNET_OPEN_TYPE_PRECONFIG, nil, nil, 0)
  else
  begin
    FSession := InternetOpen('IM-History Download Master', INTERNET_OPEN_TYPE_PROXY, PChar(fProxy), PChar(fProxyBypass), 0);
    if fProxyAuthUserName <> '' then
    begin
      InternetSetOption(FSession, INTERNET_OPTION_PROXY_USERNAME, @fProxyAuthUserName, Length(fProxyAuthUserName));
      InternetSetOption(FSession, INTERNET_OPTION_PROXY_PASSWORD, @fProxyAuthPassword, Length(fProxyAuthPassword));
    end;
  end;
  if ErrorResult(FSession = nil, deInternetOpen) then
    Exit;
  if Assigned(FSession) then
  begin
    // Небольшой парсинг
    // Вытаскиваем имя хоста и параметры обращения к скрипту
    ARequest := fURL;
    FHost := DelHttp(ARequest);
    FScript := ARequest;
    Delete(FScript, 1, Pos(FHost, FScript) + Length(FHost));
    // Попытка соединения с сервером
    if fAuthUserName = '' then // Если нет Basic-авторизации
      FConnect := InternetOpenURL(FSession, PChar(fURL), nil, 0, INTERNET_FLAG_RELOAD, 0)
    else
      FConnect := InternetConnect(FSession, PChar(FHost), INTERNET_DEFAULT_HTTP_PORT, PChar(fAuthUserName),
                  PChar(fAuthPassword), INTERNET_SERVICE_HTTP, 0, 0);
    if ErrorResult(FConnect = nil, deInternetOpenUrl) then
      Exit;
    dwIndex := 0;
    dwBufferLen := Length(dwBuffer);
    if fProxy <> '' then
    begin
      // Подготавливаем запрос
      FRequest := HttpOpenRequest(FConnect, 'GET', PChar(FScript), nil, '', nil, 0, 0);
      // Добавляем необходимые заголовки к запросу
      HttpAddRequestHeaders(FRequest, Accept, Length(Accept), HTTP_ADDREQ_FLAG_ADD);
      HttpAddRequestHeaders(FRequest, ProxyConnection, Length(ProxyConnection), HTTP_ADDREQ_FLAG_ADD);
      HttpAddRequestHeaders(FRequest, Lang, Length(Lang), HTTP_ADDREQ_FLAG_ADD);
      HttpAddRequestHeaders(FRequest, Agent, Length(Agent), HTTP_ADDREQ_FLAG_ADD);
      // Проверяем запрос:
      ProxyReqLen := 0;
      ProxyReqRes := 0;
      SRequest := ' ';
      HttpQueryInfo(FRequest, HTTP_QUERY_RAW_HEADERS_CRLF or
        HTTP_QUERY_FLAG_REQUEST_HEADERS, @SRequest[1], ProxyReqLen, ProxyReqRes);
      if ProxyReqLen > 0 then
      begin
        SetLength(SRequest, ProxyReqLen);
        HttpQueryInfo(FRequest, HTTP_QUERY_RAW_HEADERS_CRLF or
          HTTP_QUERY_FLAG_REQUEST_HEADERS, @SRequest[1], ProxyReqLen, ProxyReqRes);
      end;
      // Отправляем запрос
      if ErrorResult(not HttpSendRequest(FRequest, nil, 0, nil, 0), deRequest) then Exit;
    end;
    if fProxy = '' then
    begin
      // Получаем заголовок ответа с сервера
      Headers := GetQueryInfo(FConnect, HTTP_QUERY_RAW_HEADERS_CRLF);
      Synchronize(toHeaders);
      // Запрос размера
      if HttpQueryInfo(FConnect, HTTP_QUERY_CONTENT_LENGTH, @dwBuffer, dwBufferLen, dwIndex) then
        AllSize := StrToInt('0' + PChar(@dwBuffer));
    end
    else
    begin
      // Получаем заголовок ответа с сервера
      Headers := GetQueryInfo(FRequest, HTTP_QUERY_RAW_HEADERS_CRLF);
      Synchronize(toHeaders);
      // Запрос размера
      if HttpQueryInfo(FRequest, HTTP_QUERY_CONTENT_LENGTH, @dwBuffer, dwBufferLen, dwIndex) then
        AllSize := StrToInt('0' + PChar(@dwBuffer));
    end;
    // Проверяем MD5 старого файла на диске
    TempHeaders := ParseHeadersMD5andSize(Headers);
    DownloadsFileName := Tok('|', TempHeaders);
    DownloadsFileMD5 := Tok('|', TempHeaders);
    if FileExists(fDirPath+DownloadsFileName) then
      TempMD5 := LowerCase(MD5DigestToStr(MD5File(fDirPath+DownloadsFileName)))
    else
      TempMD5 := '00000000000000000000000000000000';
    if LowerCase(DownloadsFileMD5) <> TempMD5 then
    begin
      repeat
        if Terminated then
          Break;
        FillChar(Buffer, SizeOf(Buffer), 0);
        if fProxy = '' then
        begin
          if ErrorResult(not InternetReadFile(FConnect, @Buffer, Length(Buffer), BytesRead), deDownloadingFile) then
            Exit
          else
            MemoryStream.Write(Buffer, BytesRead);
        end
        else
        begin
          if ErrorResult(not InternetReadFile(FRequest, @Buffer, Length(Buffer), BytesRead), deDownloadingFile) then
            Exit
          else
            MemoryStream.Write(Buffer, BytesRead);
        end;
        AcceptedSize := MemoryStream.Size;
        Synchronize(toDownloading);
      until (BytesRead = 0);
      MemoryStream.Position := 0;
      // Подсчет MD5 и размера файла
      MD5Str := LowerCase(MD5DigestToStr(MD5Stream(MemoryStream)));
      TempHeaders := ParseHeadersMD5andSize(Headers);
      DownloadsFileName := Tok('|', TempHeaders);
      if Tok('|', TempHeaders) = MD5Str then
        MD5Correct := True
      else
        MD5Correct := False;
      if Tok('|', TempHeaders) = IntToStr(MemoryStream.Size) then
        SizeCorrect := True
      else
        SizeCorrect := False;
      Synchronize(toMD5);
    end
    else
    begin
      AcceptedSize := 0;
      Synchronize(toDownloading);
      MD5Str := 'FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF';
      MD5Correct := True;
      SizeCorrect := True;
      Synchronize(toMD5);
    end;
    // Очищаем ресурсы
    if Assigned(FRequest) then
      InternetCloseHandle(FRequest);
    if Assigned(FConnect) then
      InternetCloseHandle(FConnect);
    InternetCloseHandle(FSession);
    Pointer(MemoryStream) := nil;
    Complete;
  end;
end;

constructor TIMDownloadThread.Create(CreateSuspennded: Boolean; const URL, Proxy, ProxyBypass, AuthUserName, AuthPassword, ProxyAuthUserName, ProxyAuthPassword, DirPath: String; Stream: PMemoryStream);
begin
  inherited Create(CreateSuspennded);
  FreeOnTerminate := True;
  Pointer(MemoryStream) := Stream;
  AcceptedSize := 0;
  Headers := '';
  MD5Str := '';
  MD5Correct := False;
  SizeCorrect := False;
  fURL := URL;
  fProxy := Proxy;
  fProxyBypass := ProxyBypass;
  fAuthUserName := AuthUserName;
  fAuthPassword := AuthPassword;
  fProxyAuthUserName := ProxyAuthUserName;
  fProxyAuthPassword := ProxyAuthPassword;
  fDirPath := DirPath;
end;

procedure TIMDownloader.Download;
begin
  fInDowloading := True;
  if Assigned(Downloader) then
    Downloader.Terminate;
  if Assigned(fOutStream) then
    FreeAndNil(fOutStream);
  fAcceptedSize := 0;
  fMyHeaders := '';
  fMyMD5Str := '';
  fMyMD5Correct := False;
  fMySizeCorrect := False;
  fOutStream := TMemoryStream.Create;
  Downloader := TIMDownloadThread.Create(True, fURL, fProxy, fProxyBypass, fAuthUserName, fAuthPassword, fProxyAuthUserName, fProxyAuthPassword, fDirPath, Pointer(fOutStream));
  Downloader.OnAccepted := AcceptDownload;
  Downloader.OnError := ErrorDownload;
  Downloader.OnHeaders := GetHeaders;
  Downloader.OnDownloading := Downloading;
  Downloader.OnBreak := Break_Download;
  Downloader.OnMD5Checked := GetMD5;
  Downloader.Resume;
  if Assigned(fOnStartDownload) then
    fOnStartDownload(Self);
end;

procedure TIMDownloader.BreakDownload;
begin
  if not InDowloading then
    Exit;
  if Assigned(Downloader) then
    Downloader.Terminate;
end;

procedure TIMDownloader.Break_Download(Sender: TObject);
begin
  fInDowloading := False;
  Downloader := nil;
  if Assigned(fOnBreak) then
    fOnBreak(Self);
end;

procedure TIMDownloader.AcceptDownload(Sender: TObject);
begin
  fInDowloading := False;
  Downloader := nil;
  if Assigned(fOnAccepted) then
    fOnAccepted(Self);
end;

procedure TIMDownloader.GetHeaders(Sender: TObject; Headers: String);
begin
  fMyHeaders := Headers;
  if Assigned(fHeaders) then
    fHeaders(Self, Headers);
end;

procedure TIMDownloader.GetMD5(Sender: TObject; MD5Correct, SizeCorrect: Boolean; MD5Str: String);
begin
  fMyMD5Str := MD5Str;
  fMyMD5Correct := MD5Correct;
  fMySizeCorrect := SizeCorrect;
  if Assigned(fMD5) then
    fMD5(Self, MD5Correct, SizeCorrect, MD5Str);
end;

procedure TIMDownloader.Downloading(Sender: TObject; AcceptedSize, MaxSize: Cardinal);
begin
  fAcceptedSize := AcceptedSize;
  if Assigned(fDownloading) then
    fDownloading(Self, AcceptedSize, MaxSize);
end;

procedure TIMDownloader.ErrorDownload(Sender: TObject; Error: TIMDownloadError);
begin
  fInDowloading := False;
  Downloader := nil;
  fOutStream := nil;
  if Assigned(fOnError) then
    fOnError(Self, Error);
end;

end.
