(*
    History++ plugin for Miranda IM: the free IM client for Microsoft* Windows*

    Copyright (C) 2006-2009 theMIROn, 2003-2006 Art Fedorov.
    History+ parts (C) 2001 Christian Kastner

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*)

unit PassForm;

interface

uses
  Windows, Messages, SysUtils, Classes, Graphics, Controls, Forms, Dialogs,
  ComCtrls, Menus, ExtCtrls, StdCtrls,
  m_api;

type
  TfmPass = class(TForm)
    Image1: TImage;
    rbProtAll: TRadioButton;
    rbProtSel: TRadioButton;
    lvCList: TListView;
    bnPass: TButton;
    laPassState: TLabel;
    Bevel1: TBevel;
    bnCancel: TButton;
    bnOK: TButton;
    PopupMenu1: TPopupMenu;
    Refresh1: TMenuItem;
    Label1: TLabel;
    procedure bnCancelClick(Sender: TObject);
    procedure FormCreate(Sender: TObject);
    procedure rbProtSelClick(Sender: TObject);
    procedure bnPassClick(Sender: TObject);
    procedure bnOKClick(Sender: TObject);
    procedure Refresh1Click(Sender: TObject);
    procedure FormClose(Sender: TObject; var Action: TCloseAction);
    procedure FormDestroy(Sender: TObject);
    procedure FormKeyDown(Sender: TObject; var Key: Word; Shift: TShiftState);
  private
    PassMode: Byte;
    Password: AnsiString;
    FLastContact: THandle;
    procedure FillList;
    procedure UpdatePassword;
    procedure SetlastContact(const Value: THandle);
    procedure TranslateForm;
  public
    property LastContact: THandle read FLastContact write SetLastContact;
    { Public declarations }
  end;

var
  fmPass: TfmPass;

const
  PASSMODE_PROTNONE   = 0; // no protection, not used
  PASSMODE_PROTALL    = 1; // protect all contacts
  PASSMODE_PROTSEL    = 2; // protect ONLY selected contacts
  PASSMODE_PROTNOTSEL = 3; // protect ALL, except selected contacts (not used)

function ReadPassModeFromDB: Byte;
function GetPassMode: Byte;
function GetPassword: AnsiString;
function IsPasswordBlank(const Password: AnsiString): Boolean;
function IsUserProtected(hContact: THandle): Boolean;
function CheckPassword(const Pass: AnsiString): Boolean;

procedure RunPassForm;

implementation

uses
  PassNewForm, hpp_options, hpp_services, PassCheckForm,
  Checksum, hpp_global, hpp_contacts, hpp_database, hpp_forms;

{$R *.DFM}

procedure RunPassForm;
begin
  if Assigned(PassFm) then
  begin
    PassFm.Show;
    exit;
  end;
  if Assigned(PassCheckFm) then
  begin
    PassCheckFm.Show;
    exit;
  end;
  if IsPasswordBlank(GetPassword) then
  begin
    if not Assigned(PassFm) then
    begin
      PassFm := TfmPass.Create(nil);
    end;
    PassFm.Show;
  end
  else
  begin
    PassCheckFm := TfmPassCheck.Create(nil);
    PassCheckFm.Show;
  end;
end;

function CheckPassword(const Pass: AnsiString): Boolean;
begin
  Result := (DigToBase(HashString(Pass)) = GetPassword);
end;

function IsUserProtected(hContact: THandle): Boolean;
begin
  Result := False;
  case GetPassMode of
    PASSMODE_PROTNONE:   Result := False;
    PASSMODE_PROTALL:    Result := True;
    PASSMODE_PROTSEL:    Result := (DBGetContactSettingByte(hContact, hppDBName, 'PasswordProtect', 0) = 1);
    PASSMODE_PROTNOTSEL: Result := (DBGetContactSettingByte(hContact, hppDBName, 'PasswordProtect', 1) = 1);
  end;
  if IsPasswordBlank(GetPassword) then;
end;

function IsPasswordBlank(const Password: AnsiString): Boolean;
begin
  Result := (Password = DigToBase(HashString('')));
end;

function GetPassword: AnsiString;
begin
  Result := GetDBStr(hppDBName,'Password',DigToBase(HashString('')));
end;

function ReadPassModeFromDB: Byte;
begin
  Result := GetDBByte(hppDBName,'PasswordMode',PASSMODE_PROTALL);
end;

function GetPassMode: Byte;
begin
  Result := ReadPassModeFromDB;
  if IsPasswordBlank(GetPassword) then
    Result := PASSMODE_PROTNONE;
end;

procedure TfmPass.bnCancelClick(Sender: TObject);
begin
  close;
end;

procedure AddContact(var lvCList:TListView; Contact: THandle);
var
  li: TListItem;
  Capt: String;
begin
  li := lvCList.Items.Add;
  if Contact = 0 then
  begin
    Capt := TranslateW('System History') + ' (' + GetContactDisplayName(Contact, 'ICQ') + ')';
  end
  else
    Capt := GetContactDisplayName(Contact);
  li.Caption := Capt;
  li.Data := Pointer(Contact);
  li.Checked := DBGetContactSettingByte(Contact, hppDBName, 'PasswordProtect', 0) = 1;
end;

procedure TfmPass.FillList;
var
  hCont: THandle;
begin
  lvCList.Items.BeginUpdate;
  try
    lvCList.Items.Clear;
    hCont := db_find_first();
    while hCont <> 0 do
    begin
      AddContact(lvCList,hCont);
      hCont := db_find_next(hCont);
    end;
    AddContact(lvCList,0);
    lvCList.SortType := stNone;
    lvCList.SortType := stText;
  finally
    lvCList.Items.EndUpdate;
  end;
end;

procedure TfmPass.FormCreate(Sender: TObject);
begin
  DesktopFont := True;
  MakeFontsParent(Self);
  TranslateForm;
  FillList;
  PassMode := ReadPassModeFromDB;
  if not(PassMode in [PASSMODE_PROTALL, PASSMODE_PROTSEL]) then
    PassMode := PASSMODE_PROTALL;
  Password := GetPassword;

  if PassMode = PASSMODE_PROTSEL then
    rbProtSel.Checked := True
  else
    rbProtAll.Checked := True;
  rbProtSelClick(Self);
  UpdatePassword;
  Image1.Picture.Icon.Handle := CopyIcon(hppIntIcons[0].Handle);
end;

procedure TfmPass.rbProtSelClick(Sender: TObject);
begin
  if rbProtSel.Checked then
    PassMode := PASSMODE_PROTSEL
  else if rbProtAll.Checked then
    PassMode := PASSMODE_PROTALL;

  if rbProtSel.Checked then
  begin
    lvCList.Enabled := True;
    lvCList.Color   := clWindow;
  end
  else
  begin
    lvCList.Enabled := False;
    lvCList.Color   := clInactiveBorder;
  end;
end;

procedure TfmPass.bnPassClick(Sender: TObject);
begin
  with TfmPassNew.Create(Self) do
  begin
    if ShowModal = mrOK then
    begin
      Password := DigToBase(HashString(AnsiString(edPass.Text)));
      UpdatePassword;
    end;
    Free;
  end;
end;

procedure TfmPass.UpdatePassword;
begin
  if Password = DigToBase(HashString('')) then
  begin
    // password not set
    laPassState.Font.Style := laPassState.Font.Style + [fsBold];
    laPassState.Caption    := TranslateW('Password not set');
  end
  else
  begin
    // password set
    laPassState.ParentFont := True;
    laPassState.Caption    := TranslateW('Password set');
  end;
end;

procedure TfmPass.bnOKClick(Sender: TObject);
var
  i: Integer;
  li: TListItem;
begin
  WriteDBByte(hppDBName,'PasswordMode',PassMode);
  WriteDBStr(hppDBName, 'Password', Password);
  if PassMode = PASSMODE_PROTSEL then
  begin
    for i := 0 to lvCList.Items.Count - 1 do
    begin
      li := lvCList.Items[i];
      if li.Checked then
        DBWriteContactSettingByte(THANDLE(li.Data), hppDBName, 'PasswordProtect', 1)
      else
        DBDeleteContactSetting(THANDLE(li.Data), hppDBName, 'PasswordProtect');
    end;
  end;

  close;
end;

procedure TfmPass.SetlastContact(const Value: THandle);
begin
  FLastContact := Value;
end;

procedure TfmPass.Refresh1Click(Sender: TObject);
begin
  FillList;
end;

procedure TfmPass.FormClose(Sender: TObject; var Action: TCloseAction);
begin
  Action := caFree;
end;

procedure TfmPass.FormDestroy(Sender: TObject);
begin
  try
    PassFm := nil;
  except
  end;
end;

procedure TfmPass.FormKeyDown(Sender: TObject; var Key: Word; Shift: TShiftState);
var
  Mask: Integer;
begin
  with Sender as TWinControl do
  begin
    if Perform(CM_CHILDKEY, Key, LPARAM(Sender)) <> 0 then
      Exit;
    Mask := 0;
    case Key of
      VK_TAB:
        Mask := DLGC_WANTTAB;
      VK_LEFT, VK_RIGHT, VK_UP, VK_DOWN:
        // added to change radio buttons from keyboard
        // however, we have to disable it when lvCList is focused
        if not lvCList.Focused then Mask := DLGC_WANTARROWS;
      VK_RETURN, VK_EXECUTE, VK_ESCAPE, VK_CANCEL:
        Mask := DLGC_WANTALLKEYS;
    end;
    if (Mask <> 0) and
       (Perform(CM_WANTSPECIALKEY, Key, 0) = 0) and
       (Perform(WM_GETDLGCODE, 0, 0) and Mask = 0) and
       (Perform(CM_DIALOGKEY, Key, 0) <> 0)  then
      exit;
  end;
end;

procedure TfmPass.TranslateForm;
begin
  Caption           := TranslateUnicodeString(Caption);
  Label1.Caption    := TranslateUnicodeString(Label1.Caption);
  rbProtAll.Caption := TranslateUnicodeString(rbProtAll.Caption);
  rbProtSel.Caption := TranslateUnicodeString(rbProtSel.Caption);
  bnPass.Caption    := TranslateUnicodeString(bnPass.Caption);
  bnOK.Caption      := TranslateUnicodeString(bnOK.Caption);
  bnCancel.Caption  := TranslateUnicodeString(bnCancel.Caption);
  Refresh1.Caption  := TranslateUnicodeString(Refresh1.Caption);
end;

end.
