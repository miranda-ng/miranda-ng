#pragma once

/*
* Get last seen string
* WPARAM = hContact
* LPARAM = 0
* returns a LPWSTR - containg the parsed last seen time (as menu item option). This need to be free using miranda free.
*/

#define MS_LASTSEEN_GET "Lastseen/Get"