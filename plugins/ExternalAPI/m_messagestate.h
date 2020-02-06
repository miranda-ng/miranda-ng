#pragma once

enum MessageReadData_TimeType
{
	MRD_TYPE_READTIME,
	MRD_TYPE_MESSAGETIME
};

struct MessageReadData
{
	DWORD dw_lastTime;
	int iTimeType;
	MessageReadData(DWORD lastTime, int type) : dw_lastTime(lastTime), iTimeType(type) {}
};

/*
* Update messagestate read time
* WPARAM = hContact
* LPARAM = (LPARAM)(MessageReadData*)
* returns 0
*/

#define MS_MESSAGESTATE_UPDATE "MessageState/Update"