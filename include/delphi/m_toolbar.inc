{$IFNDEF M_TOOLBAR}
{$DEFINE M_TOOLBAR}

// Modern contact list toolbar

const
  TOOLBARBUTTON_ICONIDPREFIX          = 'MTB_';
  TOOLBARBUTTON_ICONIDPRIMARYSUFFIX   = '_Primary';
  TOOLBARBUTTON_ICONIDSECONDARYSUFFIX = '_Secondary';
  TOOLBARBUTTON_ICONNAMEPRESSEDSUFFIX = 'Pressed';

//button flags
  TBBF_DISABLED    = 1 shl 0;
  TBBF_VISIBLE     = 1 shl 1;
  TBBF_PUSHED      = 1 shl 2;
  TBBF_SHOWTOOLTIP = 1 shl 3;
  TBBF_ISSEPARATOR = 1 shl 5;
  TBBF_ISLBUTTON   = 1 shl 6;
  TBBF_FLEXSIZESEPARATOR = TBBF_ISSEPARATOR or TBBF_PUSHED;

type
  _tagTBButton = record
    cbSize              :int;       // size of structure
    pszButtonID         :pAnsiChar; // char id of button used to store button info in DB and know about icon
    pszButtonName       :pAnsiChar; // name of button (not translated)
    pszServiceName      :pAnsiChar; // service name to be executed
    lParam              :LPARAM;    // param of service to be called
    pszTooltipUp        :pAnsiChar;
    pszTooltipDn        :pAnsiChar;
    defPos              :dword;     // default order pos of button (less values are nearer to
                                    // edge)..please use values greater that 100. the default
                                    // buttons has pos: 10,20..90
    tbbFlags            :dword;     // combine of TBBF_ flags above
    ParamDestructor     :procedure(var param); //will be called on parameters deletion
    hPrimaryIconHandle  :THANDLE;
    hSecondaryIconHandle:THANDLE;
  end;
  TBButton = _tagTBButton;

const
//////////////////////////////////////////////////////////////////////////
// Events
// Only after this event module subscribers should register their buttons
// wparam=lparam=0
// don't forget to return 0 to continue processing
  ME_TB_MODULELOADED:PAnsiChar = 'ToolBar/ModuleLoaded';

//////////////////////////////////////////////////////////////////////////
// Services
//
//////////////////////////////////////////////////////////////////////////
// Adding a button
// WPARAM  = 0
// LPARAM  = (TBButton *) &description
// LRESULT = (HANDLE) hButton
// in order to correctly process default icons via iconlib it should be
// registered icolib icon with id named:
// 'TBButton_'+pszButtonID+ 'Up' or +'Down' for Push (2-state) buttons
  MS_TB_ADDBUTTON:PAnsiChar = 'ToolBar/AddButton';

//////////////////////////////////////////////////////////////////////////
// Remove button
// WPARAM = (HANDLE) hButton;
// LPARAM = 0;
  MS_TB_REMOVEBUTTON:PAnsiChar = 'ToolBar/RemoveButton';

//////////////////////////////////////////////////////////////////////////
// SetState
// WPARAM = (HANDLE) hButton;
// LPARAM = one of below TBST_ states 
// LRESULT= old state
  TBST_PUSHED   = 1;
  TBST_RELEASED = 0;
  MS_TB_SETBUTTONSTATE:PAnsiChar = 'ToolBar/SetButtonState';

//////////////////////////////////////////////////////////////////////////
// SetStatebyId
// WPARAM = (char *) szButtonID;
// LPARAM = one of below TBST_ states 
// LRESULT= old state
  MS_TB_SETBUTTONSTATEBYID:PAnsiChar = 'ToolBar/SetButtonStateId';
//////////////////////////////////////////////////////////////////////////
// GetState
// WPARAM = (HANLDE) hButton;
// LPARAM = 0
// LRESULT= current state
  MS_TB_GETBUTTONSTATE:PAnsiChar = 'ToolBar/GetButtonState';

//////////////////////////////////////////////////////////////////////////
// GetState
// WPARAM = (char *) szButtonID;
// LPARAM = 0
// LRESULT= current state
  MS_TB_GETBUTTONSTATEBYID:PAnsiChar = 'ToolBar/GetButtonStateId';

{$ENDIF}
