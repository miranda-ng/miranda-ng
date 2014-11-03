unit Unit1;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls, CheckLst, XPMan, ImgList, ComCtrls, Utils;

type
  TForm1 = class(TForm)
    XPManifest1: TXPManifest;
    ImageList1: TImageList;
    CheckListBox1: TCheckListBox;
    Label1: TLabel;
    btnSelectAll: TButton;
    btnSelectNone: TButton;
    btnIceIt: TButton;
    btnMirandaPath: TButton;
    StatusBar1: TStatusBar;
    ProgressBar1: TProgressBar;
    Edit1: TEdit;
    Label2: TLabel;
    OpenDialog1: TOpenDialog;
    procedure CheckListBox1DrawItem(Control: TWinControl; Index: Integer;
      Rect: TRect; State: TOwnerDrawState);
    procedure FormCreate(Sender: TObject);
    procedure SelectAllClick(Sender: TObject);
    procedure SelectNoneClick(Sender: TObject);
    procedure IceItClick(Sender: TObject);
    procedure MirandaPathClick(Sender: TObject);
  private
    { Private declarations }
    function FillCombo(typ:string):integer;
  public
    { Public declarations }
    procedure Error(const Msg: String);
    procedure ErrorWithLastError(const Msg: String);
    procedure ClearIconGroups(const FileName: String; IconGroups: TStringList);
    function AddIcons(const FileName, IcoName: String; IconGroups: TStringList): Boolean;
    function GetIconGroups(const FileName: String; var IconGroups: TStringList): Boolean;
    function ProgressUpdate(const Filename, IcoName: String): Boolean;
  end;

  PIcoItemHeader = ^TIcoItemHeader;
  TIcoItemHeader = packed record
    Width: Byte;
    Height: Byte;
    Colors: Byte;
    Reserved: Byte;
    Planes: Word;
    BitCount: Word;
    ImageSize: DWORD;
  end;
  PIcoItem = ^TIcoItem;
  TIcoItem = packed record
    Header: TIcoItemHeader;
    Offset: DWORD;
  end;
  PIcoHeader = ^TIcoHeader;
  TIcoHeader = packed record
    Reserved: Word;
    Typ: Word;
    ItemCount: Word;
    Items: array [0..MaxInt shr 4 - 1] of TIcoItem;
  end;
  PGroupIconDirItem = ^TGroupIconDirItem;
  TGroupIconDirItem = packed record
    Header: TIcoItemHeader;
    Id: Word;
  end;
  PGroupIconDir = ^TGroupIconDir;
  TGroupIconDir = packed record
    Reserved: Word;
    Typ: Word;
    ItemCount: Word;
    Items: array [0..MaxInt shr 4 - 1] of TGroupIconDirItem;
  end;

  function EnumResNamesProc(module: HMODULE; ResType, ResName: PChar; List: TStrings): Integer; stdcall;
  function StockResourceType(ResType: PChar): string;
  function EnumResTypesProc(module: HMODULE; ResType: PChar; List: TStrings): Integer; stdcall;
  function EnumLangsFunc(hModule: Cardinal; lpType, lpName: PAnsiChar; wLanguage: Word; lParam: Integer): Boolean; stdcall;
  function GetResourceLanguage(hModule: Cardinal; lpType, lpName: PAnsiChar; var wLanguage: Word): Boolean;
  function IsValidIcon(P: Pointer; Size: Cardinal): Boolean;

var
  Form1: TForm1;
  IconsItem, PluginsItem, CoreItem: Integer;
  IcePath: String;
  MirandaPath: String;

implementation

{$R *.dfm}

var
  Log: TextFile;

procedure TForm1.Error(const Msg: String);
begin
  StatusBar1.Panels[0].Text := 'Error: ' + Msg;
  WriteLn(Log, 'Error: ' + Msg);
  // raise Exception.Create('Error: ' + Msg);
end;

procedure TForm1.ErrorWithLastError(const Msg: String);
begin
  Error(Msg + '(' + IntToStr(GetLastError) + ')');
end;

function EnumResNamesProc(module: HMODULE; ResType, ResName: PChar;
  List: TStrings): Integer; stdcall;
begin
{
  if HiWord(Cardinal(ResName)) <> 0 then
    List.Add('  ' + ResName)
  else
    List.Add(Format('  #%d', [LoWord(Cardinal(ResName))]));
}
  List.Add(Format('%d', [LoWord(Cardinal(ResName))]));
  Result := 1;
end;

function StockResourceType(ResType: PChar): string;
const
  ResTypeNames: Array [1..22] of String =
    ('RT_CURSOR',       //  = MakeIntResource(1);
     'RT_BITMAP',       //  = MakeIntResource(2);
     'RT_ICON',         //  = MakeIntResource(3);
     'RT_MENU',         //  = MakeIntResource(4);
     'RT_DIALOG',       //  = MakeIntResource(5);
     'RT_STRING',       //  = MakeIntResource(6);
     'RT_FONTDIR',      //  = MakeIntResource(7);
     'RT_FONT',         //  = MakeIntResource(8);
     'RT_ACCELERATOR',  //  = MakeIntResource(9);
     'RT_RCDATA',       //  = MakeIntResource(10);
     'RT_MESSAGETABLE', //  = MakeIntResource(11);
                        //    DIFFERENCE = 11;
     'RT_GROUP_CURSOR', //  = MakeIntResource(DWORD(RT_CURSOR +7DIFFERENCE));
     'UNKNOWN',         //    13 not used
     'RT_GROUP_ICON',   //  = MakeIntResource(DWORD(RT_ICON +DIFFERENCE));
     'UNKNOWN',         //    15 not used
     'RT_VERSION',      //  = MakeIntResource(16);
     'RT_DLGINCLUDE',   //  = MakeIntResource(17);
     'UNKNOWN',
     'RT_PLUGPLAY',     //  = MakeIntResource(19);
     'RT_VXD',          //  = MakeIntResource(20);
     'RT_ANICURSOR',    //  = MakeIntResource(21);
     'RT_ANIICON'       //  = MakeIntResource(22);
   );
var
  ResId: Cardinal absolute ResType;
begin
  if ResId in [1..22] then
    Result := ResTypeNames[ResId]
  else
    Result := 'UNKNOWN';
end;

function EnumResTypesProc(module: HMODULE; ResType: PChar; List: TStrings): Integer; stdcall;
begin
{
  if HiWord(Cardinal(restype)) <> 0 then
    List.Add(restype)
  else
    List.Add(Format('Stock type %d: %s', [LoWord(Cardinal(ResType)),
      StockResourcetype(ResType)]));
  EnumResourceNames(module, restype, @EnumResNamesProc, Integer(List));
}
  if StockResourcetype(ResType) = 'RT_GROUP_ICON' then
    EnumResourceNames(module, restype, @EnumResNamesProc, Integer(List));
  Result := 1;
end;

function EnumLangsFunc(hModule: Cardinal; lpType, lpName: PAnsiChar; wLanguage: Word; lParam: Integer): Boolean; stdcall;
begin
  PWord(lParam)^ := wLanguage;
  Result := False;
end;

function GetResourceLanguage(hModule: Cardinal; lpType, lpName: PAnsiChar; var wLanguage: Word): Boolean;
begin
  wLanguage := 0;
  EnumResourceLanguages(hModule, lpType, lpName, @EnumLangsFunc, Integer(@wLanguage));
  Result := True;
end;

procedure TForm1.ClearIconGroups(const FileName: String; IconGroups: TStringList);
var
  H: THandle;
  M: HMODULE;
  R: HRSRC;
  Res: HGLOBAL;
  GroupIconDir: PGroupIconDir;
  n, I: Integer;
  wLanguage: Word;
  ResName: PAnsiChar;
begin
  { Update the resources }
  H := BeginUpdateResource(PChar(FileName), False);
  if H = 0 then
    ErrorWithLastError('BeginUpdateResource failed (' + FileName + ')');
  try
    M := LoadLibraryEx(PChar(FileName), 0, LOAD_LIBRARY_AS_DATAFILE);
    if M = 0 then
      ErrorWithLastError('LoadLibraryEx failed [1] (' + FileName + ')');
    try
      for n := 0 to IconGroups.Count-1 do
      begin
        { Load the 'MAINICON' group icon resource }
        ResName := MakeIntResource(StrToInt(IconGroups[n]));
        R := FindResource(M, ResName, RT_GROUP_ICON);
        if R = 0 then
          ErrorWithLastError('FindResource failed [1] ' + FileName + ')');
        Res := LoadResource(M, R);
        if Res = 0 then
          ErrorWithLastError('LoadResource failed [1] (' + FileName + ')');
        GroupIconDir := LockResource(Res);
        if GroupIconDir = nil then
          ErrorWithLastError('LockResource failed [1] (' + FileName + ')');
        { Delete 'RT_GROUP_ICON' }
        if not GetResourceLanguage(M, RT_GROUP_ICON, ResName, wLanguage) then
          Error('GetResourceLanguage failed (1)');
        if not UpdateResource(H, RT_GROUP_ICON, ResName, wLanguage, nil, 0) then
          ErrorWithLastError('UpdateResource failed [1] (' + FileName + ')');
        { Delete the RT_ICON icon resources that belonged to 'RT_GROUP_ICON' }
        for I := 0 to GroupIconDir.ItemCount-1 do begin
          if not GetResourceLanguage(M, RT_ICON, MakeIntResource(GroupIconDir.Items[I].Id), wLanguage) then
            Error('GetResourceLanguage failed [2] (' + FileName + ')');
          if not UpdateResource(H, RT_ICON, MakeIntResource(GroupIconDir.Items[I].Id), wLanguage, nil, 0) then
            ErrorWithLastError('UpdateResource failed [2] (' + FileName + ')');
        end;
      end;
    finally
      FreeLibrary(M);
  end;
  except
    EndUpdateResource(H, True);  { discard changes }
    raise;
  end;
  if not EndUpdateResource(H, False) then
    ErrorWithLastError('EndUpdateResource failed');
end;

function IsValidIcon(P: Pointer; Size: Cardinal): Boolean;
{
var
  ItemCount: Cardinal;
}
begin
  {
  Result := False;
  if Size < Cardinal(SizeOf(Word) * 3) then
    Exit;
  if (PChar(P)[0] = 'M') and (PChar(P)[1] = 'Z') then
    Exit;
  ItemCount := PIcoHeader(P).ItemCount;
  if Size < Cardinal((SizeOf(Word) * 3) + (ItemCount * SizeOf(TIcoItem))) then
    Exit;
  P := @PIcoHeader(P).Items;
  while ItemCount > Cardinal(0) do begin
    if (Cardinal(PIcoItem(P).Offset + PIcoItem(P).Header.ImageSize) < Cardinal(PIcoItem(P).Offset)) or
      (Cardinal(PIcoItem(P).Offset + PIcoItem(P).Header.ImageSize) > Cardinal(Size)) then
      Exit;
    Inc(PIcoItem(P));
    Dec(ItemCount);
  end;
  }
  Result := True;
end;

function TForm1.AddIcons(const FileName, IcoName: String; IconGroups: TStringList): Boolean;
var
  H: THandle;
  M: HMODULE;
  NewGroupIconDir: PGroupIconDir;
  n, I, e: Integer;
  F: TFileStream;
  Ico: PIcoHeader;
  NewGroupIconDirSize: LongInt;
  IcoFileName: String;
  ResName: PAnsiChar;
  Size: Cardinal;
begin
  Result := True;
  e := 1;
  for n := 0 to IconGroups.Count-1 do
  begin
    Application.ProcessMessages;
    Ico := nil;
    try
      { Load the icons }
      case n of
      0..8  : IcoFileName := IcoName + ' 00' + IntToStr(n+1) + '.ico';
      9..98 : IcoFileName := IcoName + ' 0' + IntToStr(n+1) + '.ico';
      99..998 : IcoFileName := IcoName + ' ' + IntToStr(n+1) + '.ico';
      end;
      if not FileExists(IcoFileName) then
      begin
        Error('Icon ' + IcoFileName + ' not found');
        Break;
      end;
      ResName := MakeIntResource(StrToInt(IconGroups[n]));
      { Load the icons }
      F := TFileStream.Create(IcoFileName, fmOpenRead);
      try
        Size := F.Size;
        if Cardinal(Size) > Cardinal($100000) then  { sanity check }
          Error('Icon file ' + IcoFileName + ' is too large');
        GetMem(Ico, Size);
        F.ReadBuffer(Ico^, Size);
      finally
        F.Free;
      end;

      { Ensure the icon is valid }
      if not IsValidIcon(Ico, N) then
      Error('Icon file ' + IcoFileName + ' is invalid');

      { Update the resources }
      H := BeginUpdateResource(PChar(FileName), False);
      if H = 0 then
        ErrorWithLastError('BeginUpdateResource failed (' + FileName + ')');
      try
        M := LoadLibraryEx(PChar(FileName), 0, LOAD_LIBRARY_AS_DATAFILE);
        if M = 0 then
          ErrorWithLastError('LoadLibraryEx failed (' + FileName + ')');
        try
          { Build the new group icon resource }
          NewGroupIconDirSize := 3*SizeOf(Word)+Ico.ItemCount*SizeOf(TGroupIconDirItem);
          GetMem(NewGroupIconDir, NewGroupIconDirSize);
          try
            { Build the new group icon resource }
            NewGroupIconDir.Reserved := 0;
            NewGroupIconDir.Typ := 1;
            NewGroupIconDir.ItemCount := Ico.ItemCount;
            for I := 0 to NewGroupIconDir.ItemCount-1 do begin
              NewGroupIconDir.Items[I].Header := Ico.Items[I].Header;
              NewGroupIconDir.Items[I].Id := e; //assumes that there aren't any icons left
              inc(e);
            end;

            { Update 'MAINICON' }
            for I := 0 to NewGroupIconDir.ItemCount-1 do
              if not UpdateResource(H, RT_ICON, MakeIntResource(NewGroupIconDir.Items[I].Id), 1033, Pointer(DWORD(Ico) + Ico.Items[I].Offset), Ico.Items[I].Header.ImageSize) then
                ErrorWithLastError('UpdateResource failed (' + FileName + ')');

            { Update the icons }
            if not UpdateResource(H, RT_GROUP_ICON, ResName, 1033, NewGroupIconDir, NewGroupIconDirSize) then
              ErrorWithLastError('UpdateResource failed (' + FileName + ')');
          finally
            FreeMem(NewGroupIconDir);
          end;
        finally
          FreeLibrary(M);
        end;
      except
        EndUpdateResource(H, True);  { discard changes }
        raise;
      end;
      if not EndUpdateResource(H, False) then
        ErrorWithLastError('EndUpdateResource failed');
      Result := False;
    finally
      FreeMem(Ico);
    end;
  end;
end;

function TForm1.GetIconGroups(const FileName: String; var IconGroups: TStringList): Boolean;
var
  M: HMODULE;
begin
  M := LoadLibraryEx(PChar(FileName), 0, LOAD_LIBRARY_AS_DATAFILE);
  try
    Result := EnumResourceTypes(M, @EnumResTypesProc, Integer(IconGroups));
  finally
    FreeLibrary(M);
  end;
end;

procedure TForm1.SelectAllClick(Sender: TObject);
var n: Integer;
begin
  for n := 0 to CheckListBox1.Items.Count - 1 do
  begin
    CheckListBox1.Items[n] := SetValue(CheckListBox1.Items[n], 1, 'NONE');
    CheckListBox1.Checked[n] := True;
  end;
end;

procedure TForm1.SelectNoneClick(Sender: TObject);
var n: Integer;
begin
  for n := 0 to CheckListBox1.Items.Count - 1 do
  begin
    CheckListBox1.Items[n] := SetValue(CheckListBox1.Items[n], 1, 'NONE');
    CheckListBox1.Checked[n] := False;
  end;
end;

procedure TForm1.IceItClick(Sender: TObject);
var
  n: Integer;
  S: String;
  F: String;
  xFilePath: String;
  xIcoPath: String;
  xTemp: String;
  MaxCount: Integer;
  res:String;
  size:Integer;
begin
  if FileExists(Edit1.Text) then
  begin
    AssignFile(Log, 'iceit.log');
    Rewrite(Log);

    MaxCount := 0;
    for n := 0 to CheckListBox1.Items.Count - 1 do
    begin
      if CheckListBox1.Checked[n] then inc(MaxCount);
      CheckListBox1.Items[n] := SetValue(CheckListBox1.Items[n], 1, 'NONE');
    end;
    MirandaPath := ExtractFileDir(Edit1.Text);
    ProgressBar1.Min := 0;
    ProgressBar1.Max := MaxCount;
    for n := 0 to CheckListBox1.Items.Count - 1 do
    begin
      if (CheckListBox1.Checked[n]) and not (CheckListBox1.Header[n]) then
      begin
        ProgressBar1.Position := n;
        S := GetValue(CheckListBox1.Items[n], 0);
        S := LowerCase(S);
        // keep the order as for FillCombo function (atm Core<Plugins<Icons)
        if      n < PluginsItem then F := '\core'
        else if n < IconsItem   then F := '\icons'
        else                         F := '\plugins';

        StatusBar1.Panels[0].Text := ' Processing: ' + S;
        if (S = 'miranda32') or (S = 'miranda64') then
        begin
          xFilePath := MirandaPath + '\' + S + '.exe';
           //in order not to duplicate existing packages of icons (miranda32) for miranda 64
          // will not work if there are two files  (miranda32.exe and miranda64.exe)
          if not FileExists(MirandaPath + '\miranda32.exe') then
              xFilePath := MirandaPath + '\miranda64.exe';
          xIcoPath := IcePath + F + '\' + S + '\' + S;
        end
        else
        begin
          xFilePath := MirandaPath + F + '\' + S + '.dll';
          xIcoPath := IcePath + F + '\' + S + '\' + S;
        end;
        CheckListBox1.Items[n] := SetValue(CheckListBox1.Items[n], 1, 'PROGRESS');
        CheckListBox1.ItemIndex := n;
        if not FileExists(xFilePath) then
          CheckListBox1.Items[n] := SetValue(CheckListBox1.Items[n], 1, 'PASS')
        else
        begin
          xTemp := xFilePath + '.temp';
          CopyFile(PChar(xFilePath), Pchar(xTemp), False);

          if ProgressUpdate(xTemp, xIcoPath) then
            res := 'ERROR'
          else
          begin
            res := 'SUCCESS';
            DeleteFile(xFilePath);
            MoveFile(PChar(xTemp), PChar(xFilePath));
          end;
          CheckListBox1.Items[n] := SetValue(CheckListBox1.Items[n], 1, res);
        end;
        if FileExists(xTemp) then DeleteFile(xTemp);
      end;
    end; // for

    size:=FileSize(Log);
    CloseFile(Log);
    if size = 0 then DeleteFile('iceit.log');
  end
  else
  begin
    Edit1.Text := 'Enter path to Miranda*.exe here or click this button ------>';
    Edit1.SelectAll;
    Edit1.SetFocus;
  end;
  ProgressBar1.Position := 0;
  StatusBar1.Panels[0].Text := ' Ready';
end;

procedure TForm1.MirandaPathClick(Sender: TObject);
begin
  if OpenDialog1.Execute then Edit1.Text := OpenDialog1.FileName;
end;

procedure TForm1.CheckListBox1DrawItem(Control: TWinControl; Index: Integer;
  Rect: TRect; State: TOwnerDrawState);
var
  S1, S2: String;
  n: Integer;
begin
  Rect.Left := Rect.Left-15;
  CheckListBox1.Canvas.Brush.Style := bsSolid;
  CheckListBox1.Canvas.Brush.Color := clWhite;
  CheckListBox1.Canvas.FillRect(Rect);
  CheckListBox1.Canvas.Font.Name := 'Verdana';
  if CheckListBox1.Header[Index] then
  begin
    ImageList1.Draw(CheckListBox1.Canvas, Rect.Left+17, Rect.Top+1, 3);
    CheckListBox1.Canvas.Font.Style := [];
    CheckListBox1.Canvas.Font.Size := 8;
  end
  else
  begin
    ImageList1.Draw(CheckListBox1.Canvas, Rect.Left+17, Rect.Top+1, 0);
    CheckListBox1.Canvas.Font.Style := [];
    CheckListBox1.Canvas.Font.Size := 7;
  end;
  if CheckListBox1.Checked[Index] then
  begin
    ImageList1.Draw(CheckListBox1.Canvas, Rect.Left+1, Rect.Top+1, 1);
  end
  else
  begin
    ImageList1.Draw(CheckListBox1.Canvas, Rect.Left+1, Rect.Top+1, 2);
  end;
  CheckListBox1.Canvas.Brush.Style := bsClear;
  CheckListBox1.Canvas.Font.Color := clBlack;
  S1 := GetValue(CheckListBox1.Items[Index], 0);
  S2 := GetValue(CheckListBox1.Items[Index], 1);
  n := CheckListBox1.Canvas.TextWidth(S2);
  CheckListBox1.Canvas.TextOut(Rect.Left+36, Rect.Top+2, S1);
  if      S2 = 'NONE'     then CheckListBox1.Canvas.Font.Color := clWindow
  else if S2 = 'PROGRESS' then CheckListBox1.Canvas.Font.Color := $007E7E7E
  else if S2 = 'SUCCESS'  then CheckListBox1.Canvas.Font.Color := $00129D02
  else if S2 = 'PASS'     then CheckListBox1.Canvas.Font.Color := $00168FAD
  else if S2 = 'ERROR'    then CheckListBox1.Canvas.Font.Color := $001232C7;
  CheckListBox1.Canvas.TextOut(Rect.Right-(n+4), Rect.Top+2, S2);
end;

function TForm1.FillCombo(typ:String):integer;
var
  FindHandle: THandle;
  FindData: TWin32FindData;
  FileName: String;
  FilePath: String;
begin
  CheckListBox1.Items.Add(typ+'|NONE|');
  result := CheckListBox1.Items.Count-1;
  CheckListBox1.Header[result] := True;
  FindData.dwFileAttributes := FILE_ATTRIBUTE_NORMAL;
  FilePath := IcePath + '\' + typ + '\*.*';
  FindHandle := FindFirstFile(PChar(FilePath), FindData);
  if FindHandle <> INVALID_HANDLE_VALUE then
  begin
    repeat
      FileName := FindData.cFileName;
      if (FileName <> '..') and (FileName <> '.') then
        if (FindData.dwFileAttributes and FILE_ATTRIBUTE_DIRECTORY) <> 0 then
         CheckListBox1.Items.Add(LowerCase(FileName)+'|NONE|');
    until not FindNextFile(FindHandle, FindData);
  end;
  Windows.FindClose(FindHandle);
end;

procedure TForm1.FormCreate(Sender: TObject);
begin
  IcePath := ExtractFilePath(ParamStr(0));

  CoreItem    := FillCombo('core');
  PluginsItem := FillCombo('icons');
  IconsItem   := FillCombo('plugins');
end;

function TForm1.ProgressUpdate(const FileName, IcoName: String): Boolean;
var
  IconGroups: TStringList;
begin
  Result := False;
  if Win32Platform <> VER_PLATFORM_WIN32_NT then
    Error('Only supported on Windows NT and above');

  IconGroups := TStringList.Create;
  if not GetIconGroups(FileName, IconGroups) then
     ErrorWithLastError('Cannot Get Icon Groups in ' + FileName)
  else begin
    if IconGroups.Count = 0 then
    begin
      Error('Icon Group in ' + FileName + ' Is Empty');
    end;
    ClearIconGroups(Filename, IconGroups);
    if AddIcons(Filename, IcoName, IconGroups) then
      Result := True;
  end;
  IconGroups.Free;
end;

end.
