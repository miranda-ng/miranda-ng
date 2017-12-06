#ifndef __MESSAGES_H
#define __MESSAGES_H

//#include "list.h"

// structure for chained list of handles (window handles, account handles, whatever)
struct WndHandles
{
	HANDLE Handle;

	struct WndHandles *Next;
};

#define WM_YAMN		WM_APP+0x2800		//(0xA800 in fact)
enum
{
	WM_YAMN_CHANGEHOTKEY=WM_YAMN,
	WM_YAMN_CHANGETIME,

//ChangeStatus message
//WPARAM- (HACCOUNT) Account whose status is changed
//LPARAM- new status of account
	WM_YAMN_CHANGESTATUS,

//StopAccount message
//WPARAM- (HACCOUNT) Account, which should stop its work and finish immidiatelly
	WM_YAMN_STOPACCOUNT,

//Account content changed
	WM_YAMN_CHANGECONTENT,

	WM_YAMN_UPDATEMAILS,

	WM_YAMN_NOTIFYICON,

	WM_YAMN_CHANGESTATUSOPTION,

	WM_YAMN_SHOWSELECTED,
};

#endif
