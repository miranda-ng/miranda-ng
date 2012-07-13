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

unit PassNewForm;

interface

uses
  Windows, Messages, SysUtils, Classes, Graphics, Controls, Forms, Dialogs,
  StdCtrls, ExtCtrls, m_api{, HistoryControls};

type
  TfmPassNew = class(TForm)
    Label1: TLabel;
    Label2: TLabel;
    edPass: TEdit;
    edConf: TEdit;
    Label3: TLabel;
    bnCancel: TButton;
    bnOK: TButton;
    Label4: TLabel;
    Image1: TImage;
    Label5: TLabel;
    Bevel1: TBevel;
    procedure bnCancelClick(Sender: TObject);
    procedure bnOKClick(Sender: TObject);
    procedure FormCreate(Sender: TObject);
  private
    procedure TranslateForm;
    { Private declarations }
  public
    { Public declarations }
  end;

var
  fmPassNew: TfmPassNew;

implementation

uses
  {hpp_global, }hpp_forms, hpp_options;

{$R *.DFM}

procedure TfmPassNew.bnCancelClick(Sender: TObject);
begin
  ModalResult := mrCancel;
end;

procedure TfmPassNew.bnOKClick(Sender: TObject);
begin
  if edPass.Text <> edConf.Text then
  begin
    MessageBox(Handle, TranslateW('Password and Confirm fields should be similar'),
      TranslateW('Error'), MB_OK or MB_DEFBUTTON1 or MB_ICONEXCLAMATION);
  exit;
  end;
  ModalResult := mrOK;
end;

procedure TfmPassNew.TranslateForm;
begin
  Caption          := TranslateUnicodeString(Caption);
  Label1.Caption   := TranslateUnicodeString(Label1.Caption);
  Label5.Caption   := TranslateUnicodeString(Label5.Caption);
  Label2.Caption   := TranslateUnicodeString(Label2.Caption);
  Label3.Caption   := TranslateUnicodeString(Label3.Caption);
  Label4.Caption   := TranslateUnicodeString(Label4.Caption);
  bnOK.Caption     := TranslateUnicodeString(bnOK.Caption);
  bnCancel.Caption := TranslateUnicodeString(bnCancel.Caption);
end;

procedure TfmPassNew.FormCreate(Sender: TObject);
begin
  TranslateForm;
  DesktopFont := True;
  MakeFontsParent(Self);
  Image1.Picture.Icon.Handle := CopyIcon(hppIntIcons[0].handle);
end;

end.
