/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2008 Miranda ICQ/IM project, 
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

#ifndef M_PLUGINS_H__
#define M_PLUGINS_H__

/*
	Undocumented: Do NOT use.
	Version: 0.3.4.1+ (2004/10/04)
*/
#define DBPE_DONE 1
#define DBPE_CONT 0
#define DBPE_HALT (-1)
typedef struct PLUGIN_DB_ENUM {
	int cbSize;
	// 0 = continue, 1 = found, -1 = stop now
	int (*pfnEnumCallback) ( char * pluginname, /*DATABASELINK*/ void * link, LPARAM lParam);
	LPARAM lParam;
} PLUGIN_DB_ENUM;
#define MS_PLUGINS_ENUMDBPLUGINS "Plugins/DbEnumerate"


//#define DEFMOD_PROTOCOLICQ  1     //removed from v0.3 alpha
//#define DEFMOD_PROTOCOLMSN  2	    //removed from v0.1.2.0+
#define DEFMOD_UIFINDADD      3
#define DEFMOD_UIUSERINFO     4
#define DEFMOD_SRMESSAGE      5
#define DEFMOD_SRURL          6
#define DEFMOD_SREMAIL        7
#define DEFMOD_SRAUTH         8
#define DEFMOD_SRFILE         9
#define DEFMOD_UIHELP         10
#define DEFMOD_UIHISTORY      11
//#define DEFMOD_RNDCHECKUPD  12    //removed from v0.3.1 alpha
//#define DEFMOD_RNDICQIMPORT 13	//removed from v0.3 alpha
#define DEFMOD_RNDAUTOAWAY    14
#define DEFMOD_RNDUSERONLINE  15
//#define DEFMOD_RNDCRYPT     16    // v0.1.0.1-v0.1.2.0
#define DEFMOD_SRAWAY         17	// v0.1.0.1+
#define DEFMOD_RNDIGNORE      18	// v0.1.0.1+
#define DEFMOD_UIVISIBILITY   19    // v0.1.1.0+, options page only
#define DEFMOD_UICLUI         20    // v0.1.1.0+
//#define DEFMOD_UIPLUGINOPTS 21	// removed from 0.4.0.1
//#define DEFMOD_PROTOCOLNETLIB 22	// removed from 0.8.0.5
#define DEFMOD_RNDIDLE        23    // v0.3.4a+
#define DEFMOD_CLISTALL       24	// v0.3.4a+ (2004/09/28)
#define DEFMOD_DB             25    // v0.3.4.3+ (2004/10/11)
#define DEFMOD_FONTSERVICE    26    // v0.7.0+ (2006/11/17)
#define DEFMOD_UPDATENOTIFY   27
#define DEFMOD_SSL            28    // v0.8.0+
#define DEFMOD_HIGHEST        28

//plugins/getdisabledefaultarray
//gets an array of the modules that the plugins report they want to replace
//wParam=lParam=0
//returns a pointer to an array of INT_PTR, with elements 1 or 0 indexed by the
//DEFMOD_ constants. 1 to signify that the default module shouldn't be loaded.
//this is primarily for use by the core's module initialiser, but could also
//be used by modules that are doing naughty things that are very
//feature-dependent
#define MS_PLUGINS_GETDISABLEDEFAULTARRAY	   "Plugins/GetDisableDefaultArray" 

#endif // M_PLUGINS_H__


