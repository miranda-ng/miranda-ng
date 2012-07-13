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

unit PassCheckForm;

interface

uses
  Windows, Messages, SysUtils, Classes, Graphics, Controls, Forms, Dialogs,
  StdCtrls, ExtCtrls, {Checksum, HistoryControls, }m_api;

type
  TfmPassCheck = class(TForm)
    Label1: TLabel;
    edPass: TEdit;
    bnOK: TButton;
    bnCancel: TButton;
    Image1: TImage;
    Label2: TLabel;
    Label3: TLabel;
    Bevel1: TBevel;
    procedure FormDestroy(Sender: TObject);
    procedure FormClose(Sender: TObject; var Action: TCloseAction);
    procedure bnOKClick(Sender: TObject);
    procedure bnCancelClick(Sender: TObject);
    procedure FormKeyDown(Sender: TObject; var Key: Word; Shift: TShiftState);
    procedure edPassKeyPress(Sender: TObject; var Key: Char);
    procedure FormCreate(Sender: TObject);
  private
    procedure TranslateForm;
    { Private declarations }
  public
    { Public declarations }
  end;

var
  fmPassCheck: TfmPassCheck;

implementation

uses hpp_options, hpp_services, hpp_forms, hpp_global, PassForm;

{$R *.DFM}

procedure TfmPassCheck.FormDestroy(Sender: TObject);
begin
  try
    PassCheckFm := nil;
  except
  end;
end;

procedure TfmPassCheck.FormClose(Sender: TObject; var Action: TCloseAction);
begin
  Action := caFree;
end;

procedure TfmPassCheck.bnOKClick(Sender: TObject);
begin
  if CheckPassword(AnsiString(edPass.Text)) then
  begin
    if not Assigned(PassFm) then
    begin
      PassFm := TfmPass.Create(nil);
    end;
    PassFm.Show;
    Close;
  end
  else
  begin
    { DONE: sHure }
    HppMessageBox(Handle, TranslateW('You have entered the wrong password.'),
      TranslateW('History++ Password Protection'), MB_OK or MB_DEFBUTTON1 or MB_ICONSTOP);
  end;
end;

procedure TfmPassCheck.bnCancelClick(Sender: TObject);
begin
  Close;
end;

procedure TfmPassCheck.FormKeyDown(Sender: TObject; var Key: Word; Shift: TShiftState);
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
      VK_RETURN, VK_EXECUTE, VK_ESCAPE, VK_CANCEL:
        Mask := DLGC_WANTALLKEYS;
    end;
    if (Mask <> 0) and (Perform(CM_WANTSPECIALKEY, Key, 0) = 0) and
      (Perform(WM_GETDLGCODE, 0, 0) and Mask = 0) and (Self.Perform(CM_DIALOGKEY, Key, 0) <> 0)
    then
      Exit;
  end;
end;

procedure TfmPassCheck.edPassKeyPress(Sender: TObject; var Key: Char);
begin
  if (Key = Chr(VK_RETURN)) or
     (Key = Chr(VK_TAB)) or
     (Key = Chr(VK_ESCAPE)) then
  Key := #0;
end;

procedure TfmPassCheck.TranslateForm;
begin
  Caption          := TranslateUnicodeString(Caption);
  Label3.Caption   := TranslateUnicodeString(Label3.Caption);
  Label2.Caption   := TranslateUnicodeString(Label2.Caption);
  Label1.Caption   := TranslateUnicodeString(Label1.Caption);
  bnOK.Caption     := TranslateUnicodeString(bnOK.Caption);
  bnCancel.Caption := TranslateUnicodeString(bnCancel.Caption);
end;

procedure TfmPassCheck.FormCreate(Sender: TObject);
begin
  DesktopFont := True;
  MakeFontsParent(Self);
  TranslateForm;
  Image1.Picture.Icon.Handle := CopyIcon(hppIntIcons[0].handle);
end;

end.
