#ifndef __history_h__
#define __history_h__

#define EVENTTYPE_STATUSCHANGE 25368

enum
{
	UM_LOADCONTACT = WM_USER+1,

	UM_REBUILDLIST,
	UM_FILTERLIST,
	UM_REDRAWLIST,
	UM_REDRAWLISTH,
	UM_UPDATEICONS,

	UM_ADDEVENT,
	UM_ADDEVENTFILTER,
	UM_REMOVEEVENT,

	UM_SELECTED,

	UM_GETEVENTCOUNT,
	UM_GETEVENT,
	UM_GETEVENTTEXT,
	UM_GETEVENTCONTACT,
	UM_GETEVENTHANDLE,
//	UM_GETEVENTDATA,

	UM_JUMP2TIME
};

extern HANDLE hNewstoryWindows;

void InitHistory();
void FreeHistory();

int svcShowNewstory(WPARAM wParam, LPARAM lParam);
int svcShowSystemNewstory(WPARAM wParam, LPARAM lParam);

#endif // __history_h__