unit DraggedFigureUnit;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, ExtCtrls;

type
  TDraggedFigure = class(TForm)
    Image1: TImage;
    procedure FormCreate(Sender: TObject);
    procedure Image1MouseDown(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  DraggedFigure: TDraggedFigure;

implementation

{$R *.dfm}

procedure TDraggedFigure.FormCreate(Sender: TObject);
begin
  ClientWidth:= 40;
end;



procedure TDraggedFigure.Image1MouseDown(Sender: TObject; Button: TMouseButton;
  Shift: TShiftState; X, Y: Integer);
const SC_DRAGMOVE = $F012;
begin ReleaseCapture;
  Perform(WM_SYSCOMMAND, SC_DRAGMOVE, 0 );
end;

end.
