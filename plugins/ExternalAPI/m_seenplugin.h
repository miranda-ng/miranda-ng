#pragma once

/*
* Get last seen string
* WPARAM = hContact
* LPARAM = NULL
* returns a WCHAR* - containg the parsed last seen time. This need to be free using miranda free.
*/

#define MS_LASTSEEN_GET "Lastseen/Get"