{$IFNDEF M_NOTIFY}
{$DEFINE M_NOTIFY}

{** Miranda Notify Dispatcher ************************************************
Notify Dispatcher provides common interface to different notification plugins
like osd, popup, ticker etc.
******************************************************************************}

const
{ Options UI event and service. The same as for miranda options }
  ME_NOTIFY_OPT_INITIALISE:PAnsiChar = 'Notify/Opt/Initialise';
  MS_NOTIFY_OPT_ADDPAGE:PAnsiChar    = 'Notify/Opt/AddPage';

type
  PMNOTIFYACTIONINFO = ^TMNOTIFYACTIONINFO;
  TMNOTIFYACTIONINFO = record
    icon   :HICON;
    name   :array [0..MAXMODULELABELLENGTH-1] of AnsiChar;
    service:array [0..MAXMODULELABELLENGTH-1] of AnsiChar;
    cookie :dword;
  end;

// Just like miranda pluginLink... This should work faster then services,
// we need some reactivity in notifications.
type
  PMNOTIFYLINK = ^TMNOTIFYLINK;
  TMNOTIFYLINK = record
    // Create a new notification type
    Register:function(name:PAnsiChar;icon:HICON):THANDLE;cdecl;

    // Create a new notification object
    Create:function(atype:THANDLE):THANDLE;cdecl;

    // Check is handle is a valid notification object
    IsValid:function(notify:THANDLE):integer;cdecl;

    // Set/get information about object, or type defaults
    _Set:function(notifyORtype:THANDLE;name:PAnsiChar;val:TDBVARIANT):integer;cdecl;
    _Get:function(notifyORtype:THANDLE;name:PAnsiChar;val:PDBVARIANT):integer;cdecl;

    // Set/get actions
    AddAction :function(notifyORtype:THANDLE;icon:HICON;name:PAnsiChar;service:PAnsiChar;cookie:dword):integer;cdecl;
    GetActions:function(notifyORtype:THANDLE;actions:PMNOTIFYACTIONINFO):integer;cdecl;

    // Increment/decrement refer count of notification object. Unreferred objects are destroyed
    AddRef :function(notify:THANDLE):integer;cdecl;
    Release:function(notify:THANDLE):integer;cdecl;

    // Notify user
    Show  :procedure(notify:THANDLE);cdecl;
    Update:procedure(notify:THANDLE);cdecl;
    Remove:procedure(notify:THANDLE);cdecl;
  end;

const
// Get the MNOTIFYLINK struct
// result = (LRESULT)(MNOTIFYLINK* )notifyLink
  MS_NOTIFY_GETLINK:PAnsiChar = 'Notify/GetLink';

// Hook this to process corresponding actions
  ME_NOTIFY_SHOW  :PAnsiChar = 'Notify/Show';
  ME_NOTIFY_UPDATE:PAnsiChar = 'Notify/Update';
  ME_NOTIFY_REMOVE:PAnsiChar = 'Notify/Remove';

var
  notifyLink:PMNOTIFYLINK;

const
// Common options for Get/Set actions
  NFOPT_TYPENAME  = 'General/TypeName';
  NFOPT_ICON      = 'General/Icon';
  NFOPT_CONTACT   = 'General/Contact';
  NFOPT_EVENT     = 'General/Event';
  NFOPT_TEXT      = 'General/Text';
  NFOPT_TEXTW     = 'General/TextW';
  NFOPT_TITLE     = 'General/Title';
  NFOPT_TITLEW    = 'General/TitleW';
  NFOPT_BACKCOLOR = 'General/BackColor';
  NFOPT_TEXTCOLOR = 'General/TextColor';
  NFOPT_TIMEOUT   = 'General/Timeout';

{$ENDIF}
