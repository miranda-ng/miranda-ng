{$include compilers.inc}
unit wrapdlgs;

interface

uses Windows;

function SelectDirectory(Caption:PAnsiChar;var Directory:PAnsiChar;
         Parent:HWND=0):Boolean; overload;
function SelectDirectory(Caption:PWideChar;var Directory:PWideChar;
         Parent:HWND=0):Boolean; overload;

implementation

uses common, messages;

type
  PSHItemID = ^TSHItemID;
  TSHItemID = packed record
    cb: word;                         { Size of the ID (including cb itself) }
    abID: array[0..0] of byte;        { The item ID (variable length) }
  end;

  PItemIDList = ^TItemIDList;
  TItemIDList = record
     mkid: TSHItemID;
  end;

  TBrowseInfoA = record
    hwndOwner     : HWND;
    pidlRoot      : PItemIDList;
    pszDisplayName: PAnsiChar;   { Return display name of item selected. }
    lpszTitle     : PAnsiChar;   { text to go in the banner over the tree. }
    ulFlags       : uint;        { Flags that control the return stuff }
    lpfn          : pointer; //TFNBFFCallBack;
    lParam        : LPARAM;      { extra info that's passed back in callbacks }
    iImage        : integer;     { output var: where to return the Image index. }
  end;
  TBrowseInfoW = record
    hwndOwner     : HWND;
    pidlRoot      : PItemIDList;
    pszDisplayName: PWideChar;   { Return display name of item selected. }
    lpszTitle     : PWideChar;   { text to go in the banner over the tree. }
    ulFlags       : uint;        { Flags that control the return stuff }
    lpfn          : pointer; //TFNBFFCallBack;
    lParam        : LPARAM;      { extra info that's passed back in callbacks }
    iImage        : integer;     { output var: where to return the Image index. }
  end;

function SHBrowseForFolderA(var lpbi: TBrowseInfoA): PItemIDList; stdcall;
  external 'shell32.dll' name 'SHBrowseForFolderA';
function SHBrowseForFolderW(var lpbi: TBrowseInfoW): PItemIDList; stdcall;
  external 'shell32.dll' name 'SHBrowseForFolderW';
function SHGetPathFromIDListA(pidl: PItemIDList; pszPath: PAnsiChar): bool; stdcall;
  external 'shell32.dll' name 'SHGetPathFromIDListA';
function SHGetPathFromIDListW(pidl: PItemIDList; pszPath: PWideChar): bool; stdcall;
  external 'shell32.dll' name 'SHGetPathFromIDListW';
procedure CoTaskMemFree(pv: pointer); stdcall; external 'ole32.dll'
  name 'CoTaskMemFree';

const
  BIF_RETURNONLYFSDIRS   = $0001;  { For finding a folder to start document searching }
  BIF_DONTGOBELOWDOMAIN  = $0002;  { For starting the Find Computer }
  BIF_STATUSTEXT         = $0004;
  BIF_RETURNFSANCESTORS  = $0008;
  BIF_EDITBOX            = $0010;
  BIF_VALIDATE           = $0020;  { insist on valid result (or CANCEL) }
  BIF_NEWDIALOGSTYLE     = $0040;  { Use the new dialog layout with the ability to resize }
                                   { Caller needs to call OleInitialize() before using this API (c) JVCL }
  BIF_BROWSEFORCOMPUTER  = $1000;  { Browsing for Computers. }
  BIF_BROWSEFORPRINTER   = $2000;  { Browsing for Printers }
  BIF_BROWSEINCLUDEFILES = $4000;  { Browsing for Everything }

  BFFM_INITIALIZED       = 1;
  BFFM_SELCHANGED        = 2;

  BFFM_SETSTATUSTEXT     = WM_USER + 100;
  BFFM_ENABLEOK          = WM_USER + 101;
  BFFM_SETSELECTION      = WM_USER + 102;
  BFFM_SETSELECTIONW     = WM_USER + 103;

function SelectDirectory(Caption:PAnsiChar;var Directory:PAnsiChar;Parent:HWND=0):Boolean;
var
  BrowseInfo:TBrowseInfoA;
  Buffer:array [0..MAX_PATH-1] of AnsiChar;
  ItemIDList:PItemIDList;
begin
  Result:=False;
  FillChar(BrowseInfo,SizeOf(BrowseInfo),0);

  BrowseInfo.hwndOwner     :=Parent;
  BrowseInfo.pszDisplayName:=@Buffer;
  BrowseInfo.lpszTitle     :=Caption;
  BrowseInfo.ulFlags       :=BIF_RETURNONLYFSDIRS or BIF_NEWDIALOGSTYLE;

  ItemIDList:=ShBrowseForFolderA(BrowseInfo);
  if ItemIDList<>nil then
  begin
    ShGetPathFromIDListA(ItemIDList,Buffer);
    StrDup(Directory,Buffer);
    CoTaskMemFree(ItemIDList);
    result:=true;
  end;
end;

function SelectDirectory(Caption:PWideChar;var Directory:PWideChar;Parent:HWND=0):Boolean;
var
  BrowseInfo:TBrowseInfoW;
  Buffer:array [0..MAX_PATH-1] of WideChar;
  ItemIDList:PItemIDList;
begin
  Result:=False;
  FillChar(BrowseInfo,SizeOf(BrowseInfo),0);

  BrowseInfo.hwndOwner     :=Parent;
  BrowseInfo.pszDisplayName:=@Buffer;
  BrowseInfo.lpszTitle     :=Caption;
  BrowseInfo.ulFlags       :=BIF_RETURNONLYFSDIRS or BIF_NEWDIALOGSTYLE;

  ItemIDList:=ShBrowseForFolderW(BrowseInfo);
  if ItemIDList<>nil then
  begin
    ShGetPathFromIDListW(ItemIDList,Buffer);
    StrDupW(Directory,Buffer);
    CoTaskMemFree(ItemIDList);
    result:=true;
  end;
end;

end.
