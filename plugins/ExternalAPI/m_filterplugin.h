#ifndef __M_FILTERPLUGIN_H
#define __M_FILTERPLUGIN_H

#include <windows.h>
#include "m_mails.h"	//for mail definition

//
//================================== IMPORTED FUNCTIONS ==================================
//

#ifndef YAMN_STANDARDFCN
typedef DWORD (WINAPI *YAMN_STANDARDFCN)(LPVOID);
#endif
typedef DWORD (WINAPI *YAMN_FILTERMAILFCN)(HACCOUNT,DWORD,HYAMNMAIL,DWORD);

typedef struct CFilterImportFcn
{
//If changes are made in this structure, version is changed. 
//So then YAMN does not initialize your structure, if version does not match.
#define	YAMN_FILTERIMPORTFCNVERSION	2

//Function is called to get info from mail and mark mail as spam or not...
	YAMN_FILTERMAILFCN		FilterMailFcnPtr;

//Function is called when application exits. Plugin should unload
	YAMN_STANDARDFCN		UnLoadFcn;
} YAMN_FILTERIMPORTFCN, *PYAMN_FILTERIMPORTFCN;

//
//================================== FILTER PLUGIN REGISTRATION STRUCTURES ==================================
//

typedef struct CFilterPluginRegistration
{
#define	YAMN_FILTERREGISTRATIONVERSION	2
//Name of plugin
//this member CANNOT be NULL. Just write here description, i.e. "PopFile filter plugin for YAMN"
	char *Name;

//The version of plugin. CANNOT be NULL.
	char *Ver;

//Plugin copyright
//Write here your copyright if you want (or NULL)
	char *Copyright;

//Plugin description. Can be NULL.
	char *Description;

//Your contact (email). Can be NULL.
	char *Email;

//The web page. Can be NULL.
	char *WWW;
} YAMN_FILTERREGISTRATION, *PYAMN_FILTERREGISTRATION;

typedef struct CYAMNFilterPlugin
{
//Importance of plugin. Mails are filtered in the way, that filter with smallest importance number
//filters and marks mails first and the filter using the highest number marks mails the last. It means,
//that number with highest number is the most important, because it can set or clear flags as it wants,
//if another plugin set some flag, plugin with higher number can clear it.
	DWORD Importance;

//All needed other info from plugin
	PYAMN_FILTERREGISTRATION PluginInfo;

//Imported functions
	PYAMN_FILTERIMPORTFCN FilterFcn;
} YAMN_FILTERPLUGIN, *PYAMN_FILTERPLUGIN, *HYAMNFILTERPLUGIN;

typedef struct CFilterPluginQueue
{
	HYAMNFILTERPLUGIN Plugin;
	struct CFilterPluginQueue *Next;
} YAMN_FILTERPLUGINQUEUE,*PYAMN_FILTERPLUGINQUEUE;

//
//================================== YAMN SERVICES FOR PROTOCOL PLUGIN ==================================
//

//RegisterFilterPlugin Service
//Registers filter plugin 
//WPARAM- pointer to YAMN_FILTERREGISTRATION structure. Plugin must not delete this structure from memory.
//LPARAM- version of YAMN_FILTERREGISTRATION structure (use YAMN_PROTOREGISTRATIONVERSION definition)
//returns handle to plugin (HYAMNFILTERPLUGIN), if registration failed (plugin not registered) returns NULL
//You need next to call SetFilterPluginFcnImportFcn to have your plugin cooperated with YAMN.
#define	MS_YAMN_REGISTERFILTERPLUGIN		"YAMN/Service/RegisterFilterPlugin"

//UnregisterFilterPlugin Service
//Unregisters filter plugin
//WPARAM- (HYAMNFILTERPLUGIN) plugin handle
//LPARAM- any value
//returns nonzero if success
#define	MS_YAMN_UNREGISTERFILTERPLUGIN		"YAMN/Service/UnregisterFilterPlugin"

//
//================================== FUNCTIONS DEFINITIONS ========================================
//

typedef int (WINAPI *YAMN_SETFILTERPLUGINFCNIMPORTFCN)(HYAMNFILTERPLUGIN Plugin,DWORD Importance,PYAMN_FILTERIMPORTFCN YAMNFilterFcn,DWORD YAMNFilterFcnVer);

//
//================================== QUICK FUNCTION CALL DEFINITIONS ========================================
//

//These are defininitions for YAMN exported functions. Your plugin can use them.
//pYAMNFcn is global variable, it is pointer to your structure containing YAMN functions.
//It is something similar like pluginLink variable in Miranda plugin. If you use
//this name of variable, you have already defined these functions and you can use them.
//It's similar to Miranda's CreateService function.

//How to use YAMN functions:
//Create a structure containing pointer to functions you want to use in your plugin
//This structure can look something like this:
//
//	struct
//	{
//		YAMN_SETFILTERPLUGINFCNIMPORTFCN	SetFilterPluginFcnImportFcn;
//	} *pYAMNFcn;
//
//then you have to fill this structure with pointers... If you use Miranda services, you will do it like this
//
//	pYAMNFcn->SetFilterPluginFcnImportFcn=(YAMN_SETFILTERPLUGINFCNIMPORTFCN)CallService(MS_YAMN_GETFCNPTR,(WPARAM)YAMN_SETFILTERPLUGINFCNIMPORTID,0);
//
//If you do not use Miranda services, call service MS_YAMN_GETFCNPTR directly. The address to the MS_YAMN_GETFCNPTR is sent to you in LoadFilter function:
//
//	pYAMNFcn->SetFilterPluginFcnImportFcn=(YAMN_SETFILTERPLUGINFCNIMPORTFCN)YAMN_GetFcnPtr((WPARAM)YAMN_SETFILTERPLUGINFCNIMPORTID,0);
//
//and in your plugin just simply use e.g.:
//
//	SetFilterPluginFcnImport(...);
//

#define	YAMN_SETFILTERPLUGINFCNIMPORTID		"YAMN/SetFilterPluginFcnImport"

#define SetFilterPluginFcnImport(a,b,c,d)	pYAMNFcn->SetFilterPluginFcnImportFcn(a,b,c,d)

#endif
