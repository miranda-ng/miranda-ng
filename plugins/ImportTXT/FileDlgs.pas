unit FileDlgs;

interface

uses windows,m_api;

function OpenDialogExecute(hDlg:hWnd; flg:cardinal; var nFO:integer; const DefExt:string):WideString;  //Диалог открытия файла
function FolderDialogExecute(hdlg:HWND; var nFO:integer; const DefExt:string):WideString; //Диалог выбора папки

implementation

uses ActiveX, ShlObj, SysUtils, CommDlg;

function OpenDialogExecute(hDlg:hWnd; flg:cardinal; var nFO:integer; const DefExt:string):WideString;  //Диалог открытия файла
var
  OpenFilename:OpenFilenameW;
  TempFilename:WideString;
  Temp1,Temp2:WideString;
  res:bool;
begin
 nFO:=0;
 FillChar(OpenFileName, SizeOf(OpenFileName), 0);
 with OpenFilename do
  begin
   lStructSize:=SizeOf(OpenFilename);
   hWndOwner:=hDlg;
   Temp1:=
   TranslateWideString('Default extension')+' (*.'+DefExt+')'+#0+'*.'+DefExt+#0+
   TranslateWideString('All files (*.*)')+#0+'*.*'+#0#0;
   lpstrFilter:=PWideChar(Temp1);
   nMaxFile:=MAX_PATH*16;
   SetLength(TempFilename, nMaxFile + 2);
   TempFilename[1]:=#0;
   lpstrFile :=PWideChar(TempFilename);
   Temp2:=TranslateWideString('Choose a file for import...');
   lpstrTitle:=PWideChar(Temp2);
   Flags:=flg or OFN_EXPLORER or OFN_ENABLESIZING or OFN_PATHMUSTEXIST or OFN_FILEMUSTEXIST or OFN_NOCHANGEDIR;
  end;
  res:=GetOpenFileNameW(@OpenFileName);
  if res then
  begin
    Result:=TempFilename;
    nFO:=OpenFilename.nFileOffset;
  end
  else
    Result:='';
end;

//***Awkward - author of this procedures************

function StrDupW(var dst:PWideChar;src:PWideChar;len:integer=0):PWideChar;
begin
  if (src=nil) or (src^=#0) then
    dst:=nil
  else
  begin
    if len=0 then
      len:=lstrlenw(src);
    GetMem(dst,(len+1)*SizeOf(WideChar));
    lstrcpynw(dst,src,len+1);
  end;
  result:=dst;
end;

function SelectDirectory(Caption:PWideChar;var Directory:PWideChar;
         Parent:HWND=0;newstyle:bool=false):Boolean;
var
  BrowseInfo:BrowseInfoW;
  Buffer:array [0..MAX_PATH-1] of WideChar;
  ItemIDList:PItemIDList;
  ShellMalloc:IMalloc;
begin
  Result:=False;
  FillChar(BrowseInfo,SizeOf(BrowseInfo),0);
  if (ShGetMalloc(ShellMalloc)=S_OK) and (ShellMalloc<>nil) then
  begin
    with BrowseInfo do
    begin
      hwndOwner     :=Parent;
      pszDisplayName:=Buffer;
      lpszTitle     :=Caption;
      ulFlags       :=BIF_RETURNONLYFSDIRS;
    end;
    if newstyle then
      if CoInitializeEx(nil,COINIT_APARTMENTTHREADED)<>RPC_E_CHANGED_MODE then
        BrowseInfo.ulFlags:=BrowseInfo.ulFlags or BIF_NEWDIALOGSTYLE;
    try
      ItemIDList:=ShBrowseForFolderW(@BrowseInfo);
      Result:=ItemIDList<>nil;
      if Result then
      begin
        ShGetPathFromIDListW(ItemIDList,Buffer);
        StrDupW(Directory,Buffer);
        ShellMalloc.Free(ItemIDList);
      end;
    finally
      if newstyle then CoUninitialize;
    end;
  end;
end;

//*******************************************************************

function FolderDialogExecute(hdlg:HWND; var nFO:integer; const DefExt:string):WideString; //Диалог выбора папки
var
  SR: _WIN32_FIND_DATAW;
  hFFF:THandle;
  tpwc:PWideChar;
  tws:WideString;
begin
 if SelectDirectory(TranslateW('Select folder for import...'),tpwc,hdlg,true) then
  begin
   result:=tpwc;
   result:=result+#0;
   nFO:=length(result);
   tws:=tpwc;
   tws:=tws+'\*.'+DefExt;
   SR.dwFileAttributes:=faAnyFile;
   hFFF:=FindFirstFileW(PWideChar(tws),SR);
   if hFFF<>INVALID_HANDLE_VALUE then
    begin
     repeat
      result:=result+SR.cFileName;
      result:=result+#0;
     until not FindNextFileW(hFFF,SR);
     windows.FindClose(hFFF);
     result:=result+#0;
    end;
  end
   else result:='';
end;

end.
