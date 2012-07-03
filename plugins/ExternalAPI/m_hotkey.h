#ifndef H_HOTKEY_H
#define H_HOTKEY_H

//hotkey struct
	typedef struct{
		BOOL isShift; //determines if hotkey uses shift
		BOOL isCtrl;  //determines if hotkey uses control
		BOOL isAlt;	//determines if hotkey uses alt
		UINT vkCode; 

	}KEYHASH,*PKEYHASH;

	//struct passed to HKS_GET_NAME_BYHASH the result of calling it will be stored in char* name
	typedef struct{
		char* name;	//pointer to buffer where the result will be stored
		size_t bufferlen; //size of the buffer
	}GETNAMEWPARAM;

	/*this below are registred miranda services and should be called by CallService */

	/* this funtion registers new hot key function, function must be registred as miranda service
	lParam = (KEYHASH*) hotKeyDefinition
	wParam = (const char*)ServiceName
	returns true on success and false if fails */
	#define HKS_REGISTERFUNCTION "mhotkeyregister"
		/* unregisters function as hot key function
	lParam = (KEYHASH*) hotKeyDefinition /search function by it's hotkey combination 
	wParam = NULL
	and if
	lParam = NULL
	wParam = (const char*) functionName //search function by it's name
	returns if function was propertly deleted returns true else returns false */
	#define HKS_UNREGISTERFUNCTION "mhotkeyunregister"
	
	/* 
	Updates hotkey to the specific function (if another function use this hot key it'll be assigned to this
	and that function will be unregistred as hotkey function) 
	wParam = (const char*) FunctoinName // name of the function you want to update
	lParam = (KEYHASH*)newHotKey

	returns: true on success and false on failure
	*/
	#define HKS_UPDATEHOTKEY "mhotkeyupdate"

	/* gets function hash by function name, the result is stored in lParam(KEYHASH*) 
	wParam = (const char*)functionName
	lParam = (KEYHASH*) - here the result is stored
	*/
	#define HKS_GET_HASH_BYNAME "mhgethashbyname"
	/*	gets functoin name by it's hash the GETNAMEPARAM struct must be filled before calling this
	it has to be passed as wParam, and there will be result stored.
	wParam=(GETNAMEPARAM*)Result
	lParam=(KEYHASH*)keyHash
	*/
	#define HKS_GET_NAME_BYHASH "mhgetnamebyhash"
	/* converts keyHash to windows hotkey 
	wParam=(DWORD*)result
	lParam=(KEYHASH*)hotKey
	*/
	#define HKS_HASH_TO_HOTKEY "mhhashtohotkey"
	/*converts windows hotkey to keyHash
	wParam=(DWORD*)hotkey
	lParam=(KEYHASH*)result
	*/
	#define MKS_HOTKEY_TO_HASH "mhhotkeytohash"

#endif