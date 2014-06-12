/*

Miranda Scripting Plugin for Miranda NG
Copyright 2004-2006 Piotr Pawluczuk (www.pawluczuk.info)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
/*
Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-03 Miranda ICQ/IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef M_SCRIPT_H__
#define M_SCRIPT_H__ 1

#define MBOT_GETCPARAM(cp) ((void*)((unsigned char*)cp + 24))

typedef enum {MBOT_NUMBER=0,MBOT_STRING=1,MBOT_VOID=2}MBOT_TYPE;

#ifndef _LIBPHP_H__
enum SVAR_TYPE{SV_NULL,SV_LONG=1,SV_WORD=2,SV_DOUBLE=3,
	SV_STRING=4,SV_LPOINTER=5,SV_ARRAY=11};
#endif

typedef struct{
	int	cbSize;	//Set to sizeof();
	char* pszOutput;
	char* pszResult;
}MBOT_RESULT;

struct MBOT_VPARAM{
	void* data;
	long  length; /*length of data, used for strings, and binary strings*/
	long  type;
};

/*
MBOT function prototypes;
*/
typedef int		(*MBOT_RegisterFunction)(const char* name,void* fptr,long lp,
	MBOT_TYPE rval,MBOT_TYPE p1,MBOT_TYPE p2,MBOT_TYPE p3,MBOT_TYPE p4);
typedef	int		(*MBOT_UnregisterFunction)(const char* name);

typedef void*	(*MBOT_Malloc)(long amount);
typedef char*	(*MBOT_Strdup)(const char* str);
typedef void	(*MBOT_Free)(void* ptr);
typedef void	(*MBOT_FreeResult)(MBOT_RESULT* ptr);

typedef int		(*MBOT_GetVar)(const char* name,void** value,SVAR_TYPE* cType);//returns TRUE if var exists;
typedef int		(*MBOT_DelVar)(const char* name);
typedef int		(*MBOT_SetVar)(const char* name,void* value,SVAR_TYPE cType);
typedef int		(*MBOT_NewVar)(const char* name,void* value,SVAR_TYPE cType,char locked);

/*******************************************************************
 * PARAMETERS:
	pszScript	- source code of the script; ***DO NOT USE PHP TAGS*** <?php ?>
	pResult		- if set, you'll get the struct filled with the result;
	NOTE! Remember to release the struct with "fp_freeresult"!;
	pCParam		- custom parameter, for your own use; use MBOT_GETCPARAM(cparam) to get the pointer;
	nCPType		- its type {MBOT_NUMBER,MBOT_STRING,MBOT_VOID} if str or num, php will be able to get it too

 *EXAMPLE:
	fpExecuteFile("mbot/scripts/my.php","fct1",&mbr,"parameter",
					MBOT_STRING,"suf","hello",100,25.6);
 *******************************************************************/
typedef int		(*MBOT_ExecuteScript)(const char* pszScript,MBOT_RESULT* pResult,
									  void* pCParam,MBOT_TYPE nCPType);

/*******************************************************************
 * PARAMETERS:
	pszPathname	- relative or absolute path of the php file;
	pszFcnName	- function name can be NULL (whole file will be executed then);
	pResult		- if set, you'll get the struct filled with the result;
	NOTE! Remember to release the struct with "fp_freeresult"!;
	pCParam		- custom parameter, for your own use; use MBOT_GETCPARAM(cparam) to get the pointer;
	nCPType		- its type {MBOT_NUMBER,MBOT_STRING,MBOT_VOID} if str or num, php will be able to get it too
	pszPTypes	- a string containing pformats s-string,u-long,l-long,d-long,q-double,f-float,x-hex long, v - MBOT_VPARAM*
	...			- values;

 *EXAMPLE:
	fpExecuteScript("mbot/scripts/my.php","fct1",&mbr,"parameter",
					MBOT_STRING,"suf","hello",100,25.6);
 *******************************************************************/
typedef int		(*MBOT_ExecuteFile)(const char* pszPathname,const char* pszFcnName,MBOT_RESULT* pResult,
									void* pCParam,MBOT_TYPE nCPType,const char* pszPTypes,...);

typedef struct{
	int	cbSize; //sizeof(MBOT_LPHP);
	const char* pszMBotVersion;
	//execution
	MBOT_ExecuteScript	fpExecuteScript;
	MBOT_ExecuteFile	fpExecuteFile;
	//functions
	MBOT_RegisterFunction	fpRegister;
	MBOT_UnregisterFunction	fpUnregister;
	//variables
	MBOT_NewVar	fpNewVar;
	MBOT_SetVar	fpSetVar;
	MBOT_GetVar	fpGetVar;
	MBOT_DelVar	fpDelVar;
	//memory
	MBOT_Strdup	fp_strdup;
	MBOT_Malloc	fp_malloc;
	MBOT_Free	fp_free;
	MBOT_FreeResult fp_freeresult;
}MBOT_FUNCTIONS;

/*MS_MBOT_GET_FUNCTIONS
lParam = wParam = 0;
returns a pointer to const MBOT_FUNCTIONS structure;
*/
#define MS_MBOT_GET_FCN_TABLE "MBot/GetFcnTable"
#endif //M_SCRIPT_H__