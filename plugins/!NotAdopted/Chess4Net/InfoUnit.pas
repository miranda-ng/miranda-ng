////////////////////////////////////////////////////////////////////////////////
// All code below is exclusively owned by author of Chess4Net - Pavel Perminov
// (packpaul@mail.ru, packpaul1@gmail.com).
// Any changes, modifications, borrowing and adaptation are a subject for
// explicit permition from the owner.

unit InfoUnit;

interface

uses
  Windows, Messages, SysUtils, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls, ShellAPI;

type
  TInfoForm = class(TForm)
    OkButton: TButton;
    PluginNameLabel: TLabel;
    PlayingViaLabel: TLabel;
    Label2: TLabel;
    Label3: TLabel;
    Label4: TLabel;
    Label5: TLabel;
    URLLabel: TLabel;
    EMailLabel: TLabel;
    procedure OkButtonClick(Sender: TObject);
    procedure FormCreate(Sender: TObject);
    procedure EMailLabelClick(Sender: TObject);
    procedure URLLabelClick(Sender: TObject);
    procedure FormClose(Sender: TObject; var Action: TCloseAction);
  end;

procedure ShowInfo;

implementation

{$R *.dfm}

uses
  GlobalsLocalUnit, ModalForm;

var
  infoForm: TInfoForm = nil;

procedure ShowInfo;
begin
  if (not Assigned(infoForm)) then
  begin
    infoForm := TInfoForm.Create(nil);
    if (TDialogs.HasStayOnTopOwners) then
      infoForm.FormStyle := fsStayOnTop;
{$IFDEF SKYPE}
    infoForm.Icon := Chess4NetIcon;
    infoForm.Caption := DIALOG_CAPTION;      
{$ELSE} // MI, TRILLIAN, AND_RQ, QIP
    infoForm.Icon := pluginIcon;
    infoForm.Caption := PLUGIN_NAME;
{$ENDIF}
  end;
  if not infoForm.Showing then
    infoForm.Show
  else
    infoForm.SetFocus;
end;

procedure TInfoForm.OkButtonClick(Sender: TObject);
begin
  Close;
end;

procedure TInfoForm.FormCreate(Sender: TObject);
begin
  PlayingViaLabel.Caption := PLUGIN_PLAYING_OVER; 
  PluginNameLabel.Caption := PLUGIN_INFO_NAME;
  URLLabel.Caption := PLUGIN_URL;
  EMailLabel.Caption := PLUGIN_EMAIL;
end;

procedure TInfoForm.URLLabelClick(Sender: TObject);
begin
  ShellExecute(Handle, nil, PChar(URLLabel.Caption), nil, nil, SW_SHOWNORMAL);
end;

procedure TInfoForm.EMailLabelClick(Sender: TObject);
var
  shellStr: string;
begin
  shellStr := 'mailto:' + EMailLabel.Caption;
  ShellExecute(Handle, nil, PChar(shellStr), nil, nil, SW_SHOWNORMAL);
end;

procedure TInfoForm.FormClose(Sender: TObject; var Action: TCloseAction);
begin
  infoForm := nil;
  Action := caFree;
end;

end.
