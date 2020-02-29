#pragma once

enum MessageReadData_TimeType
{
	MRD_TYPE_DELIVERED = 1,
	MRD_TYPE_READ = 2
};

/*
* Update messagestate read time
* WPARAM = hContact
* LPARAM = MessageReadData_TimeType
* returns 0
*/

#define MS_MESSAGESTATE_UPDATE "MessageState/Update"
