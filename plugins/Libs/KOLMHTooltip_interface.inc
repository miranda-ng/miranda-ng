// part of KOLMHToolTip -- interface_part.
// Moved to separate inc-file still Delphi20XX does not allow compile 
// in DEBUG mode.

  TFE = (eTextColor, eBkColor, eAPDelay, eRDelay, eIDelay);

  TFI = record
    FE: set of TFE;
    Colors: array[0..1] of TColor;
    Delays: array[0..3] of Integer;
  end;

  PMHToolTipManager = ^TMHToolTipManager;
  TKOLMHToolTipManager = PMHToolTipManager;

  PMHToolTip = ^TMHToolTip;
  TKOLMHToolTip = PMHToolTip;

  TMHToolTipManager = object(TObj)
  protected
    destructor Destroy; virtual;
  public
    TTT: array of PMHToolTip;
    function AddTip: Integer;
    function FindNeed(FI: TFI): PMHToolTip;
    function CreateNeed(FI: TFI): PMHToolTip;
  end;

  //P_MHHint = ^TMHHint;
  TMHHint = object(TObj)
  private
    function GetManager:PMHToolTipManager;
    // Spec
    procedure ProcBegin(var TI: TToolInfo);
    procedure ProcEnd(var TI: TToolInfo);
    procedure ReConnect(FI: TFI);
    procedure MoveTool(T1: PMHToolTip);
    procedure CreateToolTip;
    function GetFI: TFI;

    // Group
    function GetDelay(const Index: Integer): Integer;
    procedure SetDelay(const Index: Integer; const Value: Integer);
    function GetColor(const Index: Integer): TColor;
    procedure SetColor(const Index: Integer; const Value: TColor);

    // Local
    procedure SetText(Value: KOLString);
    function GetText: KOLString;
  public
    ToolTip: PMHToolTip;
    HasTool: Boolean;
    Parent: PControl;
    destructor Destroy; virtual;
    procedure Pop;
    procedure Popup;

    property AutoPopDelay: Integer index 2 read GetDelay write SetDelay;
    property InitialDelay: Integer index 3 read GetDelay write SetDelay;
    property ReshowDelay: Integer index 1 read GetDelay write SetDelay;

    property TextColor: TColor index 1 read GetColor write SetColor;
    property BkColor: TColor index 0 read GetColor write SetColor;
    property Text: KOLString read GetText write SetText;
  end;

  TMHToolTip = object(TObj)
  private
    fHandle: THandle;
    Count: Integer;
    function GetDelay(const Index: Integer): Integer;
    procedure SetDelay(const Index: Integer; const Value: Integer);
    function GetColor(const Index: Integer): TColor;
    procedure SetColor(const Index: Integer; const Value: TColor);
    function GetMaxWidth: Integer;
    procedure SetMaxWidth(const Value: Integer);
    function GetMargin: TRect;
    procedure SetMargin(const Value: TRect);
    function GetActivate: Boolean;
    procedure SetActivate(const Value: Boolean);
  public
    destructor Destroy; virtual;
    procedure Pop;
    procedure Popup;
    procedure Update;
    property AutoPopDelay: Integer index 2 read GetDelay write SetDelay;
    property InitialDelay: Integer index 3 read GetDelay write SetDelay;
    property ReshowDelay: Integer index 1 read GetDelay write SetDelay;
    property TextColor: TColor index 1 read GetColor write SetColor;
    property BkColor: TColor index 0 read GetColor write SetColor;
    property MaxWidth: Integer read GetMaxWidth write SetMaxWidth;
    property Margin: TRect read GetMargin write SetMargin;
    property Activate: Boolean read GetActivate write SetActivate;
    property Handle: THandle read fHandle;
  end;
