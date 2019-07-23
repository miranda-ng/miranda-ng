#pragma once
#ifndef _INC_FILTER
#define _INC_FILTER

HANDLE startFilterThread();
BOOL checkFilter(CONNECTION *head, CONNECTION *conn);

#define WM_ADD_FILTER	(WM_APP + 1)
#endif
