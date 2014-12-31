/*

'File Association Manager'-Plugin for
Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org),
Copyright (C) 2005-2007 H. Herkenrath

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program (AssocMgr-License.txt); if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef M_ASSOCMGR_H__
#define M_ASSOCMGR_H__

/* Add a new file type   v0.1.0.0+
Add a new file type to be registered with Windows.
You probably want to call this event when
ME_SYSTEM_MODULESLOADED is fired.
 wParam=0
 lParam=(LPARAM)(FILETYPEDESC*)ftd
Returns 0 on success, nonzero otherwise.
*/
#define MS_ASSOCMGR_ADDNEWFILETYPE  "AssocMgr/AddNewFileType"

typedef struct {
	int cbSize;                      // set to sizeof(FILETYPEDESC), in bytes
	union {
	   const char *pszDescription;   // description for options dialog and in registry.
	   const TCHAR *ptszDescription; // please Translate(), use singular form here.
	   const WCHAR *pwszDescription;
	};
	HINSTANCE hInstance;             // instance of the calling module and where the icon
	                                 // resource is located.
	                                 // always be sure you set this to your own hInstance even if
	                                 // you use the generic default icon

	UINT nIconResID;                 // resource id of an icon to use for the file type.
	                                 // this icon should contain icons of all sizes and color depths
	                                 // needed by Windows.
	                                 // set this to 0 to use the generic 'miranda file' icon
	                                 // provided by assocmgr.

	const char *pszService;          // service to call when a file is opened
	                                 // this service will be called with lParam set to
	                                 // the file name being opened including path.
	                                 // it can be assumed that the provided file name
	                                 // is always the long path name.
	                                 // return zero on suceess, nonzero on error.
	                                 // Note: set this to NULL to pass the file name as
	                                 // commandline argument to miranda32.exe (db file).

	DWORD flags;                     // see FTDF_* flags below

	const char *pszFileExt;          // file extension, e.g. ".ext"
	                                 // first character must be a dot, assumed to be all lower case.
	                                 // may only consist of ascii characters.

	const char *pszMimeType;         // MIME type of the file, e.g. "application/x-icq"
	                                 // may only consist of ascii characters.
	union {
	   const char *pszVerbDesc;      // description for the open verb e.g. "&Install".
	   const TCHAR *ptszVerbDesc;    // set this to NULL to use the default description.
	   const WCHAR *pwszVerbDesc;    // include an ampersand (&) character for a mnemonic key.
	};                               // please Translate().
} FILETYPEDESC;

#define FTDF_UNICODE          0x0001  // pszDescription and pszVerbDesc in struct are Unicode.
                                      // the specified service is called with Unicode parameters.

#define FTDF_DEFAULTDISABLED  0x0002  // file type is not registered by default, it needs to be
                                      // enabled explicitly on the options page.

#define FTDF_BROWSERAUTOOPEN  0x0004  // tells the browser to download and open the file directly
                                      // without prompt (currently IE and Opera6+) - be careful!
                                      // use only in conjunction with pszMimeType set.
                                      // this tells Windows that open can be safely invoked for
                                      // downloaded files.
                                      // Note that this flag may create a security risk,
                                      // because downloaded files could contain malicious content.
                                      // you need to protect against such an exploit.

#define FTDF_ISTEXT           0x0008  // tells Windows that this file can be opened
                                      // as a text file using e.g Notepad.
                                      // only has an effect on Windows XP and higher.

#define FTDF_ISSHORTCUT       0x0010  // file type behaves as shortcut, this means a
                                      // small overlay arrow is applied and the extension is never shown

#if defined(_UNICODE)
   #define FTDF_TCHAR  FTDF_UNICODE   // strings in struct are WCHAR*, service accepts WCHAR*
#else
   #define FTDF_TCHAR  0              // strings in struct are char*, service accepts char*
#endif

#if !defined(ASSOCMGR_NOHELPERFUNCTIONS)
__inline static int AssocMgr_AddNewFileType(const char *ext,const char *mime,const char *desc,const char *verb,HINSTANCE hinst,UINT iconid,const char *service,DWORD flags)
{
	FILETYPEDESC ftd;
	ftd.cbSize=sizeof(FILETYPEDESC);
	ftd.pszFileExt=ext;
	ftd.pszMimeType=mime;
	ftd.pszDescription=desc;
	ftd.pszVerbDesc=verb;
	ftd.hInstance=hinst;
	ftd.nIconResID=iconid;
	ftd.pszService=service;
	ftd.flags=flags&~FTDF_UNICODE;
	return CallService(MS_ASSOCMGR_ADDNEWFILETYPE,0,(LPARAM)&ftd);
}
__inline static int AssocMgr_AddNewFileTypeW(const char *ext,const char *mime,const WCHAR *desc,const WCHAR *verb,HINSTANCE hinst,UINT iconid,const char *service,DWORD flags)
{
	FILETYPEDESC ftd;
	ftd.cbSize=sizeof(FILETYPEDESC);
	ftd.pszFileExt=ext;
	ftd.pszMimeType=mime;
	ftd.pwszDescription=desc;
	ftd.pwszVerbDesc=verb;
	ftd.hInstance=hinst;
	ftd.nIconResID=iconid;
	ftd.pszService=service;
	ftd.flags=flags|FTDF_UNICODE;
	return CallService(MS_ASSOCMGR_ADDNEWFILETYPE,0,(LPARAM)&ftd);
}
#if defined(_UNICODE)
   #define AssocMgr_AddNewFileTypeT  AssocMgr_AddNewFileTypeW
#else
   #define AssocMgr_AddNewFileTypeT  AssocMgr_AddNewFileType
#endif
#endif

/* Remove a file type   v0.1.0.0+
Remove a file type registered previously using
MS_ASSOCMGR_ADDNEWFILETYPE.
This removes all settings in database and in registry
associated with the file type.
 wParam=0
 lParam=(WPARAM)(char*)pszFileExt
Returns 0 on success, nonzero otherwise.
*/
#define MS_ASSOCMGR_REMOVEFILETYPE  "AssocMgr/RemoveFileType"

/* Add a new url protocol type   v0.1.0.0+
Add a new url type to be registered with Windows.
You probably want to call this event when
ME_SYSTEM_MODULESLOADED is fired.
 wParam=0
 lParam=(LPARAM)(URLTYPEDESC*)utd
Returns 0 on success, nonzero otherwise.
*/
#define MS_ASSOCMGR_ADDNEWURLTYPE  "AssocMgr/AddNewUrlType"

typedef struct {
	int cbSize;                      // set to sizeof(URLTYPEDESC), in bytes
	union {
	   const char *pszDescription;   // description for options dialog and in registry.
	   const TCHAR *ptszDescription; // please Translate(), use singular form here.
	   const WCHAR *pwszDescription;
	};
	HINSTANCE hInstance;             // instance of the calling module and where the icon
	                                 // resource is located.
	                                 // always be sure you set this to your own hInstance even if
	                                 // you use the generic default icon

	UINT nIconResID;                 // resource id of an icon to use for the url type.
	                                 // only a small one (16x16) is needed by Windows,
	                                 // e.g. proto icon as used in Miranda.
	                                 // set this to 0 to use the default miranda icon.

	const char *pszService;          // service to call when a url is opened (can't be NULL)
	                                 // this service will be called with lParam set to
	                                 // the url being opened including the prefix.
	                                 // return zero on suceess, nonzero on error.

	DWORD flags;                     // see UTDF_* flags below

	const char *pszProtoPrefix;      // protocol prefix, e.g. "http:"
	                                 // last character must be a colon, assumed to be all lower case.
	                                 // may only consist of ascii characters.
} URLTYPEDESC;

#define UTDF_UNICODE          0x0001  // pszDescription in struct is Unicode.
                                      // the specified service is called with Unicode parameters.

#define UTDF_DEFAULTDISABLED  0x0002  // url type is not registered by default, it needs to be
                                      // enabled explicitly on the options page.
#if defined(_UNICODE)
   #define UTDF_TCHAR  UTDF_UNICODE   // strings in struct are WCHAR*, service accepts WCHAR*
#else
   #define UTDF_TCHAR  0              // strings in struct are char*, service accepts char*
#endif

#if !defined(ASSOCMGR_NOHELPERFUNCTIONS)
static int __inline AssocMgr_AddNewUrlType(const char *prefix,const char *desc,HINSTANCE hinst,UINT iconid,const char *service,DWORD flags)
{
	URLTYPEDESC utd;
	utd.cbSize=sizeof(URLTYPEDESC);
	utd.pszProtoPrefix=prefix;
	utd.pszDescription=desc;
	utd.hInstance=hinst;
	utd.nIconResID=iconid;
	utd.pszService=service;
	utd.flags=flags&~UTDF_UNICODE;
	return CallService(MS_ASSOCMGR_ADDNEWURLTYPE,0,(LPARAM)&utd);
}
static int __inline AssocMgr_AddNewUrlTypeW(const char *prefix,const WCHAR *desc,HINSTANCE hinst,UINT iconid,const char *service,DWORD flags)
{
	URLTYPEDESC utd;
	utd.cbSize=sizeof(URLTYPEDESC);
	utd.pszProtoPrefix=prefix;
	utd.pwszDescription=desc;
	utd.hInstance=hinst;
	utd.nIconResID=iconid;
	utd.pszService=service;
	utd.flags=flags|UTDF_UNICODE;
	return CallService(MS_ASSOCMGR_ADDNEWURLTYPE,0,(LPARAM)&utd);
}
#if defined(_UNICODE)
   #define AssocMgr_AddNewUrlTypeT  AssocMgr_AddNewUrlTypeW
#else
   #define AssocMgr_AddNewUrlTypeT  AssocMgr_AddNewUrlType
#endif
#endif

/* Remove an url protocol type   v0.1.0.0+
Remove an url registered previously using
MS_ASSOCMGR_ADDNEWURLTYPE.
This removes all settings in database and in registry
associated with the url type.
 wParam=0
 lParam=(WPARAM)(char*)pszProtoPrefix
Returns 0 on success, nonzero otherwise.
*/
#define MS_ASSOCMGR_REMOVEURLTYPE  "AssocMgr/RemoveUrlType"

/* utility which should be moved as service into m_netlib.h (MS_NETLIB_URLENCODE already exists) */
#if defined(MoveMemory) && defined(lstrlen)
static __inline char *Netlib_UrlDecode(char *str)
{
	char *psz=str;
	for (;*psz;++psz)
		switch(*psz) {
			case '+':
				*psz=' ';
				break;
			case '%':
				if (!psz[1] || !psz[2]) break;
				MoveMemory(psz,&psz[1],2);
				psz[2]=0;
				*psz=(char)strtol(psz,NULL,16);
				MoveMemory(&psz[1],&psz[3],lstrlenA(&psz[3])+1);
				break;
		}
	return str;
}
#endif

#ifndef ASSOCMGR_NOSETTINGS
#define SETTING_ONLYWHILERUNNING_DEFAULT  0
#endif

#endif // M_ASSOCMGR_H