{$IFNDEF M_YAMN}
{$DEFINE M_YAMN}

//
//================================== VARIABLES STRUCT ========================================
//

{$IFNDEF MIRANDASERVICE}
type
  MIRANDASERVICE = function(WPARAM,LPARAM:int):int; cdecl;
{$ENDIF}

const
 YAMN_VARIABLESVERSION = 3;

type
  PYAMN_VARIABLES = ^TYAMN_VARIABLES;
  TYAMN_VARIABLES = record
    hInst:HINSTANCE;
    MessageWnds:THANDLE;
    NewMailAccountWnd:THANDLE;
    Shutdown:int;
  end;
  CYAMNVariables = TYAMN_VARIABLES;

//
//================================== EXPORTED FUNCTIONS STRUCT ===============================
//
type
  CExportedFunctions = record
    ID:PAnsiChar;
    Ptr:pointer;
  end;

type
  CExportedServices = record
    ID:PAnsiChar;
    Ptr:MIRANDASERVICE;
  end;

//
//================================== YAMN EVENTS ==================================
//

const
//UninstallPlugin Event
//Sent when user wants to uninstall YAMN and all its plugins
  ME_YAMN_UNINSTALLPLUGINS = 'YAMN/MirandaEvents/UninstallPlugins';

//NewMail Event
//Notifies you about new mail
//no arguments now (Developers, send mail, which params would you like to have, but note there's problem that
//params are 32b numbers. When it is pointer to some data, these data should persist while every plugin read them and
//after that they can be removed from memory. So it is problem)
  ME_YAMN_NEWMAIL = 'YAMN/MirandaEvents/NewMail';

//
//================================== YAMN SERVICES ==================================
//

//GetFcnPtr Service
//Your plugin can co-operate with YAMN in 2 ways: with Miranda services and with YAMN exported functions
//Some commands are written in services, some are functions. The advantage of function calling instead of
//service calling is, that your code is more clear and it is faster than service calling (smaller, FASTER,
//easier- it is slogan of Miranda, isn't it ?). Miranda service has only 2 parameters, that can be
//disadvantage too.
//In every way, it is discutable which functions should be exported or if they should be implemented as
//services. And if YAMN should export some functions etc. Functions not used very often are now implemented
//as Miranda services.
//
//This service gets pointer to YAMN function. Then you can use function directly. In m_?????.h files you have
//definitions of some functions, with definitions of structure variable, so you can use functions very
//clearly, just look to header file.
//WPARAM- function ID. It is string representating function you need to get pointer (e.g. YAMN_WRITEWAITID)
//LPARAM- not used now, but set it to 0
//returns pointer to YAMN function or NULL when functions does not exist
  MS_YAMN_GETFCNPTR = 'YAMN/Service/GetFcn';

//GetVariables Service
//Ask YAMN for pointer to CYAMNVariables structure.
//WPARAM- YAMN_VARIABLESVERSION
//LPARAM- any value
//returns pointer to YAMN_VARIABLES or NULL when version of structure does not match
  MS_YAMN_GETVARIABLES = 'YAMN/Service/GetVar';

//ForceCheck Service
//Check mail on accounts
//WPARAM- not used
//LPARAM- not used
  MS_YAMN_FORCECHECK = 'YAMN/Service/ForceCheck';

//AccountCheck Service
//Check mail on individual account
//WPARAM- HACCOUNT
//LPARAM- not used
  MS_YAMN_ACCOUNTCHECK = 'YAMN/Service/AccountCheck';

//Contact List Context Menu Click
//wParam=(WPARAM)hContact
//lParam=0
//
//Event is fired when there is a double click on a CList contact,
//it is upto the caller to check for the protocol & status
//of the TMCONTACT, it's not done for you anymore since it didn't make
//sense to store all this information in memory, etc.
  MS_YAMN_CLISTCONTEXT = 'YAMN/Service/ClistContactContextMenu';

//Contact List Context Menu Click for application
//wParam=(WPARAM)hContact
//lParam=0
//
//Event is fired when there is a double click on a CList contact,
//it is upto the caller to check for the protocol & status
//of the TMCONTACT, it's not done for you anymore since it didn't make
//sense to store all this information in memory, etc.
  MS_YAMN_CLISTCONTEXTAPP = 'YAMN/Service/ClistContactContextMenuApp';

//Contact List Double Click
//wParam=(WPARAM)hContact
//lParam=0
//
//Event is fired when there is a double click on a CList contact,
//it is upto the caller to check for the protocol & status
//of the TMCONTACT, it's not done for you anymore since it didn't make
//sense to store all this information in memory, etc.
  MS_YAMN_CLISTDBLCLICK = 'YAMN/Service/ClistContactDoubleclicked';

//FilterMail Service
//Ask YAMN to process mail filtering. YAMN calls filter plugins to mark mail as spam etc... Warning! Leave all
//read or write access to mail as this function waits for write-access to mail!
//WPARAM- (HACCOUNT) account to which mail belongs
//LPARAM- (HYAMNMAIL) mail to filter
  MS_YAMN_FILTERMAIL = 'YAMN/Service/FilterMail';

//MailBrowser Service
//runs mail browser window (or tray icon only or popups only)
//WPARAM- pointer to YAMN_MAILBROWSERPARAM structure, data to mailbrowser. You do not need to fill ThreadRunningEV event member.
//LPARAM- YAMN_MAILBROWSERPARAM structure version param. Use YAMN_MAILBROWSERVERSION definition.
//returns zero if failed, nonzero if succeed
  MS_YAMN_MAILBROWSER = 'YAMN/Service/RunMailBrowser';

//NoNewMail Service
//runs no new mail procedure (shows popups e.g.)
//WPARAM- pointer to YAMN_NONEWMAILPARAM structure, data to no new mail procedure. You do not need to fill ThreadRunningEV event member.
//LPARAM- YAMN_NONEWMAILPARAM structure version param. Use YAMN_NONEWMAILVERSION definition.
//returns zero if failed, nonzero if succeed
  MS_YAMN_NONEWMAILPROC = 'YAMN/Service/NoNewMailProc';

//BadConnection Service
//runs bad connection window
//WPARAM- pointer to YAMN_BADCONNECTIONPARAM structure, data to mailbrowser. You do not need to fill ThreadRunningEV event member.
//LPARAM- YAMN_BADCONNECTIONPARAM structure version param. Use YAMN_BADCONNECTIONVERSION definition.
//returns zero if failed, nonzero if succeed
  MS_YAMN_BADCONNECTION = 'YAMN/Service/BadConnection';

{$ENDIF}
