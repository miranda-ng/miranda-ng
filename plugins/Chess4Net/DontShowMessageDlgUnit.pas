////////////////////////////////////////////////////////////////////////////////
// All code below is exclusively owned by author of Chess4Net - Pavel Perminov
// (packpaul@mail.ru, packpaul1@gmail.com).
// Any changes, modifications, borrowing and adaptation are a subject for
// explicit permition from the owner.

unit DontShowMessageDlgUnit;

interface

uses
  Forms, StdCtrls,
  //
  DialogUnit, ModalForm;

type
  TDontShowMessageDlg = class(TDialogForm)
  private
    m_DontShowCheckBox: TCheckBox;
    function FGetDontShow: boolean;
    procedure FSetDontShow(bValue: boolean);
  protected
    function GetModalID: TModalFormID; override;
  public
    class function Create(const ADialogs: TDialogs; const wstrMsg: WideString): TDontShowMessageDlg; overload;
    constructor Create(Owner: TForm; modHandler: TModalFormHandler = nil); overload; override;
    property DontShow: boolean read FGetDontShow write FSetDontShow;    
  end;

implementation

uses
  Dialogs, Controls;

////////////////////////////////////////////////////////////////////////////////
// TDontShowMessageDlg

var
  g_wstrMsg: WideString;

constructor TDontShowMessageDlg.Create(Owner: TForm; modHandler: TModalFormHandler = nil);
begin
  inherited Create(Owner, g_wstrMsg, mtInformation, [mbOK], GetModalID, modHandler);

  MsgDlg.Height := MsgDlg.Height + 10;

  m_DontShowCheckBox := TCheckBox.Create(MsgDlg);
  m_DontShowCheckBox.Parent := MsgDlg;
  m_DontShowCheckBox.Caption := 'Don''t Show'; // TODO: Localize

  m_DontShowCheckBox.Left := 10;
  m_DontShowCheckBox.Top := MsgDlg.ClientHeight - m_DontShowCheckBox.Height - 5;
end;


class function TDontShowMessageDlg.Create(const ADialogs: TDialogs;
  const wstrMsg: WideString): TDontShowMessageDlg;
begin
  g_wstrMsg := wstrMsg;
  Result := ADialogs.CreateDialog(TDontShowMessageDlg) as TDontShowMessageDlg;
  g_wstrMsg := '';
end;


function TDontShowMessageDlg.FGetDontShow: boolean;
begin
  Result := m_DontShowCheckBox.Checked;
end;


procedure TDontShowMessageDlg.FSetDontShow(bValue: boolean);
begin
  m_DontShowCheckBox.Checked := bValue;
end;


function TDontShowMessageDlg.GetModalID: TModalFormID;
begin
  Result := mfDontShowDlg;
end;

end.
