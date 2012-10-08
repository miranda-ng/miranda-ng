unit pngextra;

interface

uses
  Windows, Graphics, Messages, SysUtils, Classes, Controls, pngimage, Buttons,
  ExtCtrls;

type
  TPNGButtonStyle = (pbsDefault, pbsFlat, pbsNoFrame);
  TPNGButtonLayout = (pbsImageAbove, pbsImageBellow, pbsImageLeft,
    pbsImageRight);
  TPNGButtonState = (pbsNormal, pbsDown, pbsDisabled);

  TPNGButton = class(TGraphicControl)
  private
    {Holds the property values}
    fButtonStyle: TPNGButtonStyle;
    fMouseOverControl: Boolean;
    FCaption: String;
    FButtonLayout: TPNGButtonLayout;
    FButtonState: TPNGButtonState;
    FImageDown: TPNGObject;
    fImageNormal: TPNGObject;
    fImageDisabled: TPNGObject;
    fImageOver: TPNGObject;
    fOnMouseEnter, fOnMouseExit: TNotifyEvent;
    {Procedures for setting the property values}
    procedure SetButtonStyle(const Value: TPNGButtonStyle);
    procedure SetCaption(const Value: String);
    procedure SetButtonLayout(const Value: TPNGButtonLayout);
    procedure SetButtonState(const Value: TPNGButtonState);
    procedure SetImageNormal(const Value: TPNGObject);
    procedure SetImageDown(const Value: TPNGObject);
    procedure SetImageOver(const Value: TPNGObject);
  published
    {Published properties}
    property Font;
    property Visible;
    property ButtonLayout: TPNGButtonLayout read FButtonLayout write SetButtonLayout;
    property Caption: String read FCaption write SetCaption;
    property ImageNormal: TPNGObject read fImageNormal write SetImageNormal;
    property ImageDown: TPNGObject read FImageDown write SetImageDown;
    property ImageOver: TPNGObject read FImageOver write SetImageOver;
    property ButtonStyle: TPNGButtonStyle read fButtonStyle
      write SetButtonStyle;
    property Enabled;
    property ParentShowHint;
    property ShowHint;
    {Default events}
    property OnMouseDown;
    property OnClick;
    property OnMouseUp;
    property OnMouseMove;
    property OnDblClick;
    property OnMouseEnter: TNotifyEvent read fOnMouseEnter write fOnMouseEnter;
    property OnMouseExit:  TNotifyEvent read fOnMouseExit  write fOnMouseExit;
  public
    {Public properties}
    property ButtonState: TPNGButtonState read FButtonState write SetButtonState;
  protected
    {Being painted}
    procedure Paint; override;
    {Clicked}
    procedure Click; override;
    {Mouse pressed}
    procedure MouseDown(Button: TMouseButton; Shift: TShiftState;
      X, Y: Integer); override;
    procedure MouseUp(Button: TMouseButton; Shift: TShiftState;
      X, Y: Integer); override;
    procedure MouseMove(Shift: TShiftState; X, Y: Integer); override;
    {Mouse entering or leaving}
    procedure CMMouseEnter(var Message: TMessage); message CM_MOUSEENTER;
    procedure CMMouseLeave(var Message: TMessage); message CM_MOUSELEAVE;
    {Being enabled or disabled}
    procedure CMEnabledChanged(var Message: TMessage);
      message CM_ENABLEDCHANGED;
  public
    {Returns if the mouse is over the control}
    property IsMouseOver: Boolean read fMouseOverControl;
    {Constructor and destructor}
    constructor Create(AOwner: TComponent); override;
    destructor Destroy; override;
  end;

procedure Register;
procedure MakeImageHalfTransparent(Source, Dest: TPNGObject);

implementation

procedure Register;
begin
  RegisterComponents('Samples', [TPNGButton]);
end;

procedure MakeImageHalfTransparent(Source, Dest: TPNGObject);
var
  i, j: Integer;
begin
  Dest.Assign(Source);
  Dest.CreateAlpha;
  if (Dest.Header.ColorType <> COLOR_PALETTE) then
    for j := 0 to Source.Height - 1 do
      for i := 0 to Source.Width - 1 do
        Dest.AlphaScanline[j]^[i] := Dest.AlphaScanline[j]^[i] div 3;
end;

{TPNGButton implementation}

{Being created}
constructor TPNGButton.Create(AOwner: TComponent);
begin
  {Calls ancestor}
  inherited Create(AOwner);
  {Creates the TPNGObjects}
  fImageNormal := TPNGObject.Create;
  fImageDown := TPNGObject.Create;
  fImageDisabled := TPNGObject.Create;
  fImageOver := TPNGObject.Create;
  {Initial properties}
  ControlStyle := ControlStyle + [csCaptureMouse];
  SetBounds(Left, Top, 23, 23);
  fMouseOverControl := False;
  fButtonLayout := pbsImageAbove;
  fButtonState := pbsNormal
end;

destructor TPNGButton.Destroy;
begin
  {Frees the TPNGObject}
  fImageNormal.Free;
  fImageDown.Free;
  fImageDisabled.Free;
  fImageOver.Free;

  {Calls ancestor}
  inherited Destroy;
end;

{Being enabled or disabled}
procedure TPNGButton.CMEnabledChanged(var Message: TMessage);
begin
  if not Enabled then MakeImageHalfTransparent(fImageNormal, fImageDisabled);
  if Enabled then ButtonState := pbsNormal else ButtonState := pbsDisabled
end;

{Returns the largest number}
function Max(A, B: Integer): Integer;
begin
  if A > B then Result := A else Result := B
end;

{Button being painted}
procedure TPNGButton.Paint;
const
  Slide: Array[false..true] of Integer = (0, 2);
var
  Area: TRect;
  TextSize, ImageSize: TSize;
  TextPos, ImagePos: TPoint;
  Image: TPNGObject;
  Pushed: Boolean;
begin
  {Prepares the canvas}
  Canvas.Font.Assign(Font);

  {Determines if the button is pushed}
  Pushed := (ButtonState = pbsDown) and IsMouseOver;

  {Determines the image to use}
  if (Pushed) and not fImageDown.Empty then
    Image := fImageDown
  else if IsMouseOver and not fImageOver.Empty and Enabled then
    Image := fImageOver
  else if (ButtonState = pbsDisabled) and not fImageDisabled.Empty then
    Image := fImageDisabled
  else
    Image := fImageNormal;

  {Get the elements size}
  ImageSize.cx := Image.Width;
  ImageSize.cy := Image.Height;
  Area := ClientRect;
  if Caption <> '' then
  begin
    TextSize := Canvas.TextExtent(Caption);
    ImageSize.cy := ImageSize.Cy + 4;
  end else FillChar(TextSize, SizeOf(TextSize), #0);

  {Set the elements position}
  ImagePos.X := (Width - ImageSize.cx) div 2 + Slide[Pushed];
  TextPos.X := (Width - TextSize.cx) div 2 + Slide[Pushed];
  TextPos.Y := (Height - TextSize.cy) div 2;
  ImagePos.Y := (Height - ImageSize.cy) div 2;
  case ButtonLayout of
    pbsImageAbove: begin
      ImagePos.Y := (Height - ImageSize.cy - TextSize.cy) div 2;
      TextPos.Y := ImagePos.Y + ImageSize.cy;
      end;
    pbsImageBellow: begin
      TextPos.Y := (Height - ImageSize.cy - TextSize.cy) div 2;
      ImagePos.Y := TextPos.Y + TextSize.cy;
      end;
    pbsImageLeft: begin
      ImagePos.X := (Width - ImageSize.cx - TextSize.cx) div 2;
      TextPos.X := ImagePos.X + ImageSize.cx + 5;
      end;
    pbsImageRight: begin
      TextPos.X := (Width - ImageSize.cx - TextSize.cx) div 2;;
      ImagePos.X := TextPos.X + TextSize.cx + 5;
    end
  end;
  ImagePos.Y := ImagePos.Y + Slide[Pushed];
  TextPos.Y := TextPos.Y + Slide[Pushed];

  {Draws the border}
  if ButtonStyle = pbsFlat then
  begin
    if ButtonState <> pbsDisabled then
      if (Pushed) then
        Frame3D(Canvas, Area, clBtnShadow, clBtnHighlight, 1)
      else if IsMouseOver or (ButtonState = pbsDown) then
        Frame3D(Canvas, Area, clBtnHighlight, clBtnShadow, 1)
  end
  else if ButtonStyle = pbsDefault then
    DrawButtonFace(Canvas, Area, 1, bsNew, TRUE, Pushed, FALSE);

  {Draws the elements}
  Canvas.Brush.Style := bsClear;
  Canvas.Draw(ImagePos.X, ImagePos.Y, Image);
  if ButtonState = pbsDisabled then Canvas.Font.Color := clGrayText;
  Canvas.TextRect(Area, TextPos.X, TextPos.Y, Caption)
end;

{Changing the button Layout property}
procedure TPNGButton.SetButtonLayout(const Value: TPNGButtonLayout);
begin
  FButtonLayout := Value;
  Repaint
end;

{Changing the button state property}
procedure TPNGButton.SetButtonState(const Value: TPNGButtonState);
begin
  FButtonState := Value;
  Repaint
end;

{Changing the button style property}
procedure TPNGButton.SetButtonStyle(const Value: TPNGButtonStyle);
begin
  fButtonStyle := Value;
  Repaint
end;

{Changing the caption property}
procedure TPNGButton.SetCaption(const Value: String);
begin
  FCaption := Value;
  Repaint
end;

{Changing the image property}
procedure TPNGButton.SetImageNormal(const Value: TPNGObject);
begin
  fImageNormal.Assign(Value);
  MakeImageHalfTransparent(fImageNormal, fImageDisabled);
  Repaint
end;

{Setting the down image}
procedure TPNGButton.SetImageDown(const Value: TPNGObject);
begin
  FImageDown.Assign(Value);
  Repaint
end;

{Setting the over image}
procedure TPNGButton.SetImageOver(const Value: TPNGObject);
begin
  fImageOver.Assign(Value);
  Repaint
end;

{Mouse pressed}
procedure TPNGButton.MouseDown(Button: TMouseButton; Shift: TShiftState; X,
  Y: Integer);
begin
  {Changes the state and repaints}
  if (ButtonState = pbsNormal) and (Button = mbLeft) then
    ButtonState := pbsDown;
  {Calls ancestor}
  inherited
end;

{Being clicked}
procedure TPNGButton.Click;
begin
  if ButtonState = pbsDown then ButtonState := pbsNormal;
  inherited Click;
end;

{Mouse released}
procedure TPNGButton.MouseUp(Button: TMouseButton; Shift: TShiftState; X,
  Y: Integer);
begin
  {Changes the state and repaints}
  if ButtonState = pbsDown then ButtonState := pbsNormal;
  {Calls ancestor}
  inherited
end;

{Mouse moving over the control}
procedure TPNGButton.MouseMove(Shift: TShiftState; X, Y: Integer);
begin
  {In case cursor is over the button}
  if (X >= 0) and (X < ClientWidth) and (Y >= 0) and (Y <= ClientHeight) and
    (fMouseOverControl = False) and (ButtonState <> pbsDown)  then
  begin
    fMouseOverControl := True;
    Repaint;
  end;

  {Calls ancestor}
  inherited;

end;

{Mouse is now over the control}
procedure TPNGButton.CMMouseEnter(var Message: TMessage);
begin
  if Enabled then
  begin
    if Assigned(fOnMouseEnter) then fOnMouseEnter(Self);
    fMouseOverControl := True;
    Repaint
  end
end;

{Mouse has left the control}
procedure TPNGButton.CMMouseLeave(var Message: TMessage);
begin
  if Enabled then
  begin
    if Assigned(fOnMouseExit) then FOnMouseExit(Self);
    fMouseOverControl := False;
    Repaint
  end
end;



end.
