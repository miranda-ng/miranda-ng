#ifndef __MESSAGES_H
#define __MESSAGES_H

#define WM_YAMN WM_APP+0x2800  // (0xA800 in fact)

enum
{
	WM_YAMN_CHANGETIME = WM_YAMN,

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
