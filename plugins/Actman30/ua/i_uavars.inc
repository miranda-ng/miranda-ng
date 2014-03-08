{}
const
  MODULE_NAME = 'Actions';
const
  opt_groups:PAnsiChar = 'Group';
  opt_ua    :PAnsiChar = 'UA';
  opt_id    :PAnsiChar = 'id';
  opt_flags :PAnsiChar = 'Flags';

const
  ICOLIB_ACTSECTION = MODULE_NAME+'/Registered actions';
  ICOLIB_MNUSECTION = MODULE_NAME+'/Menu icons';

  SERVICE_WITH_LPARAM_NAME = MODULE_NAME+'/CallAction';
  TTB_SERVICE_NAME         = MODULE_NAME+'/TTBAction';

type
  tMenuType = (main_menu,contact_menu,tray_menu,proto_menu,status_menu);
  pUAMenuItem = ^tUAMenuItem;
  tUAMenuItem = record
    hMenuItem         :THANDLE;
    szMenuPopup       :pWideChar;
    szMenuNameVars    :pWideChar;
    szMenuShowWhenVars:pWideChar;
    hMenuRoot         :THANDLE;
    menu_opt          :dword;
    changed           :boolean;
  end;
type
  pMyActionItem = ^tMyActionItem;
  tMyActionItem = record
    // UseActions/Action_ID
    szNameID            :pAnsiChar; // uaction ID
    szActDescr          :pWideChar; // action name

    hIcolibIcon,
    hIcolibIconPressed  :THANDLE;

    hTTBButton          :THANDLE;   // TopToolbar button
    szTTBTooltip        :PAnsiChar;
    szTTBTooltipPressed :PAnsiChar;
    szTTBShowWhenVars   :pWideChar;

    szTabBTooltip       :PWideChar; // TabSRMM toolbar button
    szTabBTooltipPressed:PWideChar;

    lastContact         :THANDLE;   // for contact menu
    hMenuService        :THANDLE;   // common menu service
    UAMenuItem          :array [tMenuType] of tUAMenuItem;
    // moved to the end for better align
    flags               :dword;
    dwActID             :dword;     // action ID
    wSortIndex          :word;      // list/menu/toolbar order
  end;

const
  UAF_NONE      = 0;

  UAF_REGHOTKEY = 1 shl  0; // hotkey
  UAF_REGTTBB   = 1 shl  1; // modern toolbar

  UAF_REGTABB   = 1 shl  5; // TabSRMM toolbar
  UAF_USING     = UAF_REGHOTKEY or UAF_REGTTBB or UAF_REGTABB;

  UAF_2STATE    = 1 shl 11; // Buttons/menu items are 2-state
  UAF_PRESSED   = 1 shl 12; // Button pressed/menu item selected
  UAF_SAVESTATE = 1 shl 13; // Save or not "pressed" state
  UAF_GLOBAL    = 1 shl 14; // not contact related even if in contact menu only

  // realtime, no save
  UAF_HKREGGED  = 1 shl 16; // hotkey registered
  UAF_TBREGGED  = 1 shl 17; // TabSRMM button registered
  UAF_DISABLED  = 1 shl 30; // action disabled atm
  UAF_REALTIME  = UAF_HKREGGED or UAF_TBREGGED or UAF_DISABLED;

  UAF_SPECIAL   = 1 shl 31; // for settings read

  // menu options
  UAF_MENUSEP = 1 shl 1; // menu item separated
  UAF_MENUUSE = 1 shl 8; // use this menu
  UAF_NOTRANS = 1 shl 9; // do not translate menus

type
  tNameRec = record
    name   :PAnsiChar;
    service:PAnsiChar;
    mask   :dword;
    atype  :word;
    enable :boolean;
  end;

const
  NumTypes  = 8;
const
  uaTTB     = 0;
  uaTAB     = 1;
  uaHotkey  = 2;
  uaMenu    = 3;

const
  NamesArray: array [0..NumTypes-1] of tNameRec = (
    (name:'TopToolbar'; service:'TopToolBar/AddButton';
     mask:UAF_REGTTBB  ; atype:uaTTB; enable:false),
    (name:'TabSRMM toolbar'     ; service:'TabSRMM/ButtonsBar/AddButton';
     mask:UAF_REGTABB  ; atype:uaTAB; enable:false),
    (name:'Core Hotkey'         ; service:nil{MS_HOTKEY_REGISTER};
     mask:UAF_REGHOTKEY; atype:uaHotkey; enable:false),
    (name:'Main menu'           ; service:nil;
     mask:0; atype:uaMenu+(ORD(main_menu   ) shl 8); enable:false),
    (name:'Contact menu'        ; service:nil;
     mask:0; atype:uaMenu+(ORD(contact_menu) shl 8); enable:false),
    (name:'Tray menu'           ; service:'CList/AddTrayMenuItem';
     mask:0; atype:uaMenu+(ORD(tray_menu   ) shl 8); enable:false),
    (name:'Protocol menus'      ; service:'CList/AddProtoMenuItem';
     mask:0; atype:uaMenu+(ORD(proto_menu  ) shl 8); enable:false),
    (name:'Status menu'         ; service:'CList/AddStatusMenuItem';
     mask:0; atype:uaMenu+(ORD(status_menu ) shl 8); enable:false)
    );

var
  UActionList:array of tMyActionItem;
var
  szMyPath:array [0..MAX_PATH] of WideChar;
