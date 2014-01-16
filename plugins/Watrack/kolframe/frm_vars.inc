{Frame variables}
var
  sic,
  PlStatusHook:cardinal;

  HiddenByMe:bool;

  FrameHeight:dword;

type
  PWATFrameData = ^TWATFrameData;
  TWATFrameData = record
    BkDC        :HDC;       // "buffer" DC and associated bitmap
    BkBitmap    :HBITMAP;   // original bitmap
    BkFile      :PAnsiChar; // original bitmap filename
    BkDefFile   :PAnsiChar; // default background picture filename
    BkMode      :cardinal;
    BkTimer     :cardinal;  // timer to free original picture buffer
    BkColor     :TCOLORREF;
    padding     :TRect;

    ShowControls:dword;
    UseBkPicture:bool;
    UseCover    :bool;
    HideNoMusic :bool;
    HideNoPlayer:bool;

    WasHidden   :bool;
    // not sure what will keep here
    FrameId     :integer;
    FrmAlpha    :integer;
    UpdInterval :cardinal;
    UpdTimer    :cardinal;

    Template :PWideChar;

    Trackbar :PTrackbar;
    TextBlock:PTextBlock;

    // Designer section
    Designer :PDesigner;
    ManualPlacement:bool;
    Loaded   :dword;

    btnarray:array [0..numbuttons-1] of pIcoButton;
  end;

  PWATFrame = ^TWATFrame;
  TWATFrame = object(TControl)
    procedure Paint(Sender: PControl; DC: HDC);
    procedure FrameResize(Sender: PObj);
    procedure RefreshAllFrameIcons;
    procedure ResetFrame;

    procedure CreateDesigner(Sender:PControl;var Mouse:TMouseEventData);
    procedure DesignerSaveSettings;
    procedure DesignerLoadSettings;

    procedure PreparePicture(dc:HDC;rc:TRECT);
    function LoadBkPicture(CoverFName:PAnsiChar;check:boolean=false;
             BackFName:PAnsiChar=nil):integer;
    procedure SaveSettings;
    procedure LoadSettings;
    procedure CheckControls;
    procedure SetAlpha(value:integer);
    procedure RefreshPicture(cover:PAnsiChar=nil);
    procedure ClearBitmapData;
    procedure AdjustFrame;
    procedure MyDestroy(Sender:PObj);

    procedure AdjustButtons(atop:integer);
    procedure RefreshButtonIcons;
    procedure FreeButtonsDesigner;
    procedure MakeButtonsDesigner;
    procedure FreeButtons;
    procedure MakeNewButtonGroup;
 end;

var
  FrameCtrl:PWATFrame;
