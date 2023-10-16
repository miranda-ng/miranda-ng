#ifndef	__M_YAMN_H
#define	__M_YAMN_H

#include <windows.h>

//
//================================== YAMN SERVICES ==================================
//

//ForceCheck Service
//Check mail on accounts
//WPARAM- not used
//LPARAM- not used
#define MS_YAMN_FORCECHECK		"YAMN/Service/ForceCheck"

//Contact List Context Menu Click
//wParam=(WPARAM)hContact
//lParam=0
//
//Event is fired when there is a double click on a CList contact,
//it is upto the caller to check for the protocol & status
//of the MCONTACT, it's not done for you anymore since it didn't make
//sense to store all this information in memory, etc.
#define MS_YAMN_CLISTCONTEXT	"YAMN/Service/ClistContactContextMenu"

//Contact List Context Menu Click for application
//wParam=(WPARAM)hContact
//lParam=0
//
//Event is fired when there is a double click on a CList contact,
//it is upto the caller to check for the protocol & status
//of the MCONTACT, it's not done for you anymore since it didn't make
//sense to store all this information in memory, etc.
#define MS_YAMN_CLISTCONTEXTAPP	"YAMN/Service/ClistContactContextMenuApp"

//Contact List Double Click
//wParam=(WPARAM)hContact
//lParam=0
//
//Event is fired when there is a double click on a CList contact,
//it is upto the caller to check for the protocol & status
//of the MCONTACT, it's not done for you anymore since it didn't make
//sense to store all this information in memory, etc.
#define MS_YAMN_CLISTDBLCLICK	"YAMN/Service/ClistContactDoubleclicked"

//NoNewMail Service
//runs no new mail procedure (shows popups e.g.)
//WPARAM- pointer to YAMN_NONEWMAILPARAM structure, data to no new mail procedure. You do not need to fill ThreadRunningEV event member.
//LPARAM- YAMN_NONEWMAILPARAM structure version param. Use YAMN_NONEWMAILVERSION definition.
//returns zero if failed, nonzero if succeed
#define MS_YAMN_NONEWMAILPROC		"YAMN/Service/NoNewMailProc"

#endif
