{ ############################################################################ }
{ #                                                                          # }
{ #  MirandaNG HistoryToDB Plugin v2.5                                       # }
{ #                                                                          # }
{ #  License: GPLv3                                                          # }
{ #                                                                          # }
{ #  Author: Grigorev Michael (icq: 161867489, email: sleuthhound@gmail.com) # }
{ #                                                                          # }
{ ############################################################################ }

unit FSMonitor;

{$I jedi.inc}

interface

type
  // Структура с информацией об изменении в файловой системе (передается в callback процедуру)
  PInfoCallBack = ^TInfoCallBack;
  TInfoCallBack = record
    FAction      : Integer; // Тип изменения (константы FILE_ACTION_XXX)
    FDrive       : String;  // Диск, на котором было изменение
    FOldFileName : String;  // Имя файла до переименования
    FNewFileName : String;  // Имя файла после переименования
  end;

  // callback процедура, вызываемая при изменении в файловой системе
  TWatchFileSystemCallBack = procedure (pInfo: TInfoCallBack);

  { Запуск мониторинга файловой системы
  Праметры:
  pName    - имя папки для мониторинга
  pFilter  - комбинация констант FILE_NOTIFY_XXX
  pSubTree - мониторить ли все подпапки заданной папки
  pInfoCallBack - адрес callback процедуры, вызываемой при изменении в файловой системе }
  procedure StartWatch(pName: String; pFilter: Cardinal; pSubTree: Boolean; pInfoCallBack: TWatchFileSystemCallBack);
  // Остановка мониторинга
  procedure StopWatch;

implementation

uses
  Classes, Windows, SysUtils, IniFiles;

const
  FILE_LIST_DIRECTORY   = $0001;

type
  PFileNotifyInformation = ^TFileNotifyInformation;
  TFileNotifyInformation = record
    NextEntryOffset : DWORD;
    Action          : DWORD;
    FileNameLength  : DWORD;
    FileName        : Array[0..0] of WideChar;
  end;

  WFSError = class(Exception);

  TWFS = class(TThread)
  private
    FName           : String;
    FFilter         : Cardinal;
    FSubTree        : Boolean;
    FInfoCallBack   : TWatchFileSystemCallBack;
    FWatchHandle    : THandle;
    FWatchBuf       : Array[0..4096] of Byte;
    FOverLapp       : TOverlapped;
    FPOverLapp      : POverlapped;
    FBytesWritte    : DWORD;
    FCompletionPort : THandle;
    FNumBytes       : Cardinal;
    FOldFileName    : String;
    function CreateDirHandle(aDir: string): THandle;
    procedure WatchEvent;
    //procedure HandleEvent;
  protected
    procedure Execute; override;
  public
    constructor Create(pName: String; pFilter: Cardinal; pSubTree: Boolean; pInfoCallBack: TWatchFileSystemCallBack);
    destructor Destroy; override;
  end;


var
  WFS : TWFS;

procedure StartWatch(pName: String; pFilter: Cardinal; pSubTree: Boolean; pInfoCallBack: TWatchFileSystemCallBack);
begin
  WFS := TWFS.Create(pName, pFilter, pSubTree, pInfoCallBack);
end;

procedure StopWatch;
begin
  if Assigned(WFS) then
  begin
    PostQueuedCompletionStatus(WFS.FCompletionPort, 0, 0, nil);
    WFS.Terminate;
    WFS.WaitFor;
    WFS.Free;
    WFS := nil;
  end;
end;

constructor TWFS.Create(pName: String; pFilter: Cardinal; pSubTree: Boolean; pInfoCallBack: TWatchFileSystemCallBack);
begin
  inherited Create(True);
  FreeOnTerminate := False;
  FName := IncludeTrailingBackslash(pName);
  FFilter := pFilter;
  FSubTree := pSubTree;
  FOldFileName := EmptyStr;
  ZeroMemory(@FOverLapp, SizeOf(TOverLapped));
  FPOverLapp := @FOverLapp;
  ZeroMemory(@FWatchBuf, SizeOf(FWatchBuf));
  FInfoCallBack := pInfoCallBack;
  Resume;
end;


destructor TWFS.Destroy;
begin
  PostQueuedCompletionStatus(FCompletionPort, 0, 0, nil);
  CloseHandle(FWatchHandle);
  FWatchHandle := 0;
  CloseHandle(FCompletionPort);
  FCompletionPort := 0;
  inherited Destroy;
end;

function TWFS.CreateDirHandle(aDir: String): THandle;
begin
  Result := CreateFile(PChar(aDir), FILE_LIST_DIRECTORY, FILE_SHARE_READ+FILE_SHARE_DELETE+FILE_SHARE_WRITE,
                   nil,OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS or FILE_FLAG_OVERLAPPED, 0);
end;

procedure TWFS.Execute;
begin
  FWatchHandle := CreateDirHandle(FName);
  WatchEvent;
end;

{procedure TWFS.HandleEvent;
var
  FileNotifyInfo : PFileNotifyInformation;
  InfoCallBack   : TInfoCallBack;
  Offset         : Longint;
begin
  Pointer(FileNotifyInfo) := @FWatchBuf[0];
  repeat
    Offset := FileNotifyInfo^.NextEntryOffset;
    InfoCallBack.FAction := FileNotifyInfo^.Action;
    InfoCallBack.FDrive := FName;
    SetString(InfoCallBack.FNewFileName,FileNotifyInfo^.FileName,FileNotifyInfo^.FileNameLength );
    InfoCallBack.FNewFileName := Trim(InfoCallBack.FNewFileName);
    case FileNotifyInfo^.Action of
      FILE_ACTION_RENAMED_OLD_NAME: FOldFileName := Trim(WideCharToString(@(FileNotifyInfo^.FileName[0])));
      FILE_ACTION_RENAMED_NEW_NAME: InfoCallBack.FOldFileName := FOldFileName;
    end;
    FInfoCallBack(InfoCallBack);
    PChar(FileNotifyInfo) := PChar(FileNotifyInfo)+Offset;
  until (Offset=0) or Terminated;
end;}

procedure TWFS.WatchEvent;
var
  {$ifdef DELPHIXE_UP}
  CompletionKey: DWORD_PTR;
  {$ELSE}
    CompletionKey: Cardinal;
  {$endif}
  // Перенесено из HandleEvent
  FileNotifyInfo : PFileNotifyInformation;
  InfoCallBack   : TInfoCallBack;
  Offset         : Longint;
  // End
begin
  FCompletionPort := CreateIoCompletionPort(FWatchHandle, 0, Longint(pointer(self)), 0);
  ZeroMemory(@FWatchBuf, SizeOf(FWatchBuf));
  if not ReadDirectoryChanges(FWatchHandle, @FWatchBuf, SizeOf(FWatchBuf), FSubTree,
    FFilter, @FBytesWritte,  @FOverLapp, 0) then
  begin
    raise WFSError.Create(SysErrorMessage(GetLastError));
    Terminate;
  end
  else
  begin
    while not Terminated do
    begin
      GetQueuedCompletionStatus(FCompletionPort, FNumBytes, CompletionKey, FPOverLapp, INFINITE);
      if CompletionKey <> 0 then
      begin
        //Synchronize(HandleEvent);
        { Т.к. Synchronize(HandleEvent) не отрабатывает в
         плагине по непонятной причине, то его код перенесен сюда
         Подробнее тут -> http://forum.qip.ru/showthread.php?t=25200 }
        Pointer(FileNotifyInfo) := @FWatchBuf[0];
        repeat
          Offset := FileNotifyInfo^.NextEntryOffset;
          InfoCallBack.FAction := FileNotifyInfo^.Action;
          InfoCallBack.FDrive := FName;
          SetString(InfoCallBack.FNewFileName,FileNotifyInfo^.FileName,FileNotifyInfo^.FileNameLength );
          InfoCallBack.FNewFileName := Trim(InfoCallBack.FNewFileName);
          case FileNotifyInfo^.Action of
            FILE_ACTION_RENAMED_OLD_NAME: FOldFileName := Trim(WideCharToString(@(FileNotifyInfo^.FileName[0])));
            FILE_ACTION_RENAMED_NEW_NAME: InfoCallBack.FOldFileName := FOldFileName;
          end;
          FInfoCallBack(InfoCallBack);
          PChar(FileNotifyInfo) := PChar(FileNotifyInfo)+Offset;
        until (Offset=0) or Terminated;
        // End synchronize
        ZeroMemory(@FWatchBuf, SizeOf(FWatchBuf));
        FBytesWritte := 0;
        ReadDirectoryChanges(FWatchHandle, @FWatchBuf, SizeOf(FWatchBuf), FSubTree, FFilter,
                             @FBytesWritte, @FOverLapp, 0);
      end
      else
        Terminate;
    end
  end
end;

end.
