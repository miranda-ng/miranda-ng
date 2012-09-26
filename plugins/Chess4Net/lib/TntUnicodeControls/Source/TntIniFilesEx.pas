
{*****************************************************************************}
{                                                                             }
{    Tnt Delphi Unicode Controls                                              }
{      http://www.tntware.com/delphicontrols/unicode/                         }
{        Extended TTntMemIniFile  (compatible with all versions)              }
{                                                                             }
{    Copyright (c) 1999-2007 Stanley Xu                                       }
{      http://getgosurf.com/?go=supportFeedback&ln=en                         }
{                                                                             }
{*****************************************************************************}

{*****************************************************************************}
{                                                                             }
{  BACKGROUND:                                                                }
{    TTntMemIniFile buffers all changes to the INI file in memory. To write   }
{    the data from memory back to the associated INI file, call the           }
{    UpdateFile() method. However, the whole content of this INI file will    }
{    be overwritten. Even those sections that are not used. This will make    }
{    troubles, if two instances try to change the same file at the same       }
{    time, without some method of managing access the instances may well end  }
{    up overwriting each other's work.                                        }
{                                                                             }
{  IDEA:                                                                      }
{    TTntMemIniFileEx implementes a simple idea: To check the timestamp       }
{    before each operation. If the file is modified, TTntMemIniFileEx will    }
{    reload the file to keep the content updated.                             }
{                                                                             }
{  CONCLUSION:                                                                }
{    #  TTntMemIniFileEx and TTntMemIniFile are ideal for read-only access.   }
{       For instance: To read localization files, etc.                        }
{    #  To perform mass WriteString() operations, please use the following    }
{       code.                                                                 }
{             BeginUpdate();                                                  }
{             try                                                             }
{               for I := 0 to 10000 do                                        }
{                 WriteString(...);                                           }
{             finally;                                                        }
{               EndUpdate();                                                  }
{               UpdateFile;                                                   }
{             end;                                                            }
{                                                                             }
{*****************************************************************************}

unit TntIniFilesEx;

{$INCLUDE TntCompilers.inc}

interface

uses
  TntClasses, TntIniFiles;

type
  TTntMemIniFileEx = class(TTntMemIniFile)
  private
    FUpdateCount: Integer;
    FModified: Boolean;
    FLastAccessed: Integer;
    function FileRealLastAccessedTime: Integer;
    procedure GetLatestVersion;
  protected
    procedure LoadValues; // Extended
  public
    constructor Create(const FileName: WideString); override;
    procedure BeginUpdate; virtual;
    procedure EndUpdate; virtual;
    function ReadString(const Section, Ident, Default: WideString): WideString; override;
    procedure WriteString(const Section, Ident, Value: WideString); override;
    procedure ReadSection(const Section: WideString; Strings: TTntStrings); override;
    procedure ReadSections(Strings: TTntStrings); override;
    procedure ReadSectionValues(const Section: WideString; Strings: TTntStrings); override;
    procedure DeleteKey(const Section, Ident: WideString); override;
    procedure EraseSection(const Section: WideString); override;
    procedure UpdateFile; override;
  end;



implementation

uses
  SysUtils, TntSysUtils;


{ TTntMemIniFileEx }

function TTntMemIniFileEx.FileRealLastAccessedTime: Integer;
var
  H: Integer;   // file handle
begin
  Result := 0;
  H := WideFileOpen(FileName, fmOpenWrite); //fmOpenRead (?)
  if H <> -1 then
  try
    Result := FileGetDate(H);
  finally
    FileClose(H);
  end;
end;

procedure TTntMemIniFileEx.GetLatestVersion;
begin
  if FLastAccessed = FileRealLastAccessedTime then
    Exit;

  LoadValues;
  // FLastAccess will be updated in LoadValues(...)
end;

procedure TTntMemIniFileEx.LoadValues; // Copied from TntIniFiles.pas
var
  List: TTntStringList;
begin
  if (FileName <> '') and WideFileExists(FileName) then
  begin
    List := TTntStringList.Create;
    try
      List.LoadFromFile(FileName);
      FLastAccessed := FileRealLastAccessedTime;  // Extra
      FModified := False;                         //
      SetStrings(List);
    finally
      List.Free;
    end;
  end else
    Clear;
end;

constructor TTntMemIniFileEx.Create(const FileName: WideString);
begin
  inherited Create(FileName);
  FUpdateCount := 0;
end;

procedure TTntMemIniFileEx.BeginUpdate;
begin
  Inc(FUpdateCount);
end;

procedure TTntMemIniFileEx.EndUpdate;
begin
  Dec(FUpdateCount);
end;

function TTntMemIniFileEx.ReadString(const Section, Ident, Default: WideString): WideString;
begin
  GetLatestVersion;
  Result := inherited ReadString(Section, Ident, Default);
end;

procedure TTntMemIniFileEx.WriteString(const Section, Ident, Value: WideString);
begin
  GetLatestVersion;
  inherited WriteString(Section, Ident, Value);
  FModified := True;
  UpdateFile; // Flush changes to disk
end;

procedure TTntMemIniFileEx.ReadSection(const Section: WideString; Strings: TTntStrings);
begin
  GetLatestVersion;
  inherited ReadSection(Section, Strings);
end;

procedure TTntMemIniFileEx.ReadSections(Strings: TTntStrings);
begin
  GetLatestVersion;
  inherited ReadSections(Strings);
end;

procedure TTntMemIniFileEx.ReadSectionValues(const Section: WideString; Strings: TTntStrings);
begin
  GetLatestVersion;
  inherited ReadSectionValues(Section, Strings);
end;

procedure TTntMemIniFileEx.DeleteKey(const Section, Ident: WideString);
begin
  GetLatestVersion;
  inherited DeleteKey(Section, Ident);
  FModified := True;
  UpdateFile; // Flush changes to disk
end;

procedure TTntMemIniFileEx.EraseSection(const Section: WideString);
begin
  GetLatestVersion;
  inherited EraseSection(Section);
  FModified := True;
  UpdateFile; // Flush changes to disk
end;

procedure TTntMemIniFileEx.UpdateFile;
begin
  if not FModified or (FUpdateCount > 0) then
    Exit;
  inherited;
end;





end.
