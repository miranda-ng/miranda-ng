////////////////////////////////////////////////////////////////////////////////
// All code below is exclusively owned by author of Chess4Net - Pavel Perminov
// (packpaul@mail.ru, packpaul1@gmail.com).
// Any changes, modifications, borrowing and adaptation are a subject for
// explicit permition from the owner.

unit NonMainFormStayOnTopUnit;

// Inclusion of this unit enables all non-main forms with FormStyle = fsStayOnTop
// to stay on top even if application is deactivated

interface

implementation

uses
  Forms, SysUtils, Classes, Messages, Windows;
  
type
  TApplicationObjSubclasser = class
  private
    m_NewObj, m_OldObj: pointer;
    procedure FWndProc(var Message: TMessage);
  public
    constructor Create;
    destructor Destroy; override;
  end;

var
  g_ApplicationObjSubclasserInstance: TApplicationObjSubclasser = nil;

////////////////////////////////////////////////////////////////////////////////
// TApplicationObjSubclasser

constructor TApplicationObjSubclasser.Create;
begin
  inherited Create;
  
  m_NewObj := Classes.MakeObjectInstance(FWndProc);
  m_OldObj := Pointer (SetWindowLong(Application.Handle, GWL_WNDPROC,
    Cardinal(m_NewObj)));
end;


destructor TApplicationObjSubclasser.Destroy;
begin
  SetWindowLong(Application.Handle, GWL_WNDPROC, Cardinal(m_OldObj));
  Classes.FreeObjectInstance(m_NewObj);

  inherited;
end;


procedure TApplicationObjSubclasser.FWndProc(var Message: TMessage);
begin
  Message.Result := CallWindowProc (m_OldObj, Application.Handle,
    Message.Msg, Message.wParam, Message.lParam);

    case Message.Msg of
      WM_ACTIVATEAPP:
      begin
        if (not TWMActivateApp(Message).Active) then
          Application.RestoreTopMosts;
      end;
    end;
end;

initialization
  g_ApplicationObjSubclasserInstance := TApplicationObjSubclasser.Create;

finalization
  FreeAndNil(g_ApplicationObjSubclasserInstance);

end.