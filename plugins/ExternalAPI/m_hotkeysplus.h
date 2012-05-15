#define MS_HOTKEYSPLUS_ADDKEY "HotkeysPlus/Add"

/*
This service registers hotkey for 
WPARAM - service to perform
LPARAM - decription of the service
Returned values:
	 0 - success,
	 1 - hotkey for this function is already existing,
	 2 - the service, that you try to register the hotkey for, not exists
*/

#define MS_HOTKEYSPLUS_EXISTKEY "HotkeysPlus/Exist"
/*
This service checks whether hotkey  for service (WPARAM) exists
LPARAM - not used
Returned values:
	 0 - failed,
	 1 - the hotkey for this function exists,
	 2 - the service not exists
*/

