#ifndef __history_control_h__
#define __history_control_h__

#define NEWSTORYLIST_CLASS "NewstoryList"

struct ADDEVENTS
{
	MCONTACT hContact;
	MEVENT hFirstEVent;
	int eventCount;
};

enum
{
	NSM_FIRST = WM_USER + 100,

	// wParam = fist item
	// lParam = last item
	// result = number of total selected items
	NSM_SELECTITEMS = NSM_FIRST,

	// wParam = fist item
	// lParam = last item
	// result = number of total selected items
	NSM_TOGGLEITEMS,

	// wParam = fist item
	// lParam = last item
	// result = number of total selected items
	// select items wParam - lParam and deselect all other
	NSM_SELECTITEMS2,

	// wParam = fist item
	// lParam = last item
	// result = number of total selected items
	NSM_DESELECTITEMS,

	// wParam = item id
	NSM_ENSUREVISIBLE,

	// wParam = x in control
	// lParam = y in control
	// result = id
	NSM_GETITEMFROMPIXEL,

	// add history for a contact
	NSM_ADDHISTORY,

	// add one or more events
	NSM_ADDEVENTS,

	// wParam = id
	NSM_SETCARET,

	// result = id
	NSM_GETCARET,

	// wParam = text
	NSM_FINDNEXT,
	NSM_FINDPREV,

	// wParam = wtext
	NSM_FINDNEXTW,
	NSM_FINDPREVW,

	//
	NSM_COPY,
	NSM_EXPORT,

	//
	NSM_GETCOUNT,

	//
	NSM_SEEKTIME,

	NSM_LAST
};

void InitNewstoryControl();
//void DestroyNewstoryControl();

#endif // __history_control_h__
