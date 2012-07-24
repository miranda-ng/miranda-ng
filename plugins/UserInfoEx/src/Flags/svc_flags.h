/*
UserinfoEx plugin for Miranda IM

Copyright:
� 2006-2010 DeathAxe, Yasnovidyashii, Merlin, K. Romanov, Kreol

part of this code based on:
Miranda IM Country Flags Plugin Copyright �2006-2007 H. Herkenrath

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

===============================================================================

File name      : $HeadURL: https://userinfoex.googlecode.com/svn/trunk/Flags/svc_flags.h $
Revision       : $Revision: 187 $
Last change on : $Date: 2010-09-08 16:05:54 +0400 (Ср, 08 сен 2010) $
Last change by : $Author: ing.u.horn $

===============================================================================
*/
#ifndef  _UINFOEX_FLAGS_H_INCLUDED_
#define  _UINFOEX_FLAGS_H_INCLUDED_

#define EXTRAIMAGE_REFRESHDELAY		100		/* time for which setting changes are buffered */
#define STATUSICON_REFRESHDELAY		100		/* time for which setting changes are buffered */

typedef struct _FLAGSOPTIONS 
{
	BYTE	bShowExtraImgFlag;
	BYTE	bUseUnknownFlag;
	BYTE	idExtraColumn;
//	BYTE	bUseIpToCountry;
	BYTE	bShowStatusIconFlag;
} FLAGSOPTIONS, *LPFLAGSOPTIONS;

extern int nCountriesCount;
extern struct CountryListEntry *countries;
extern FLAGSOPTIONS	gFlagsOpts;

class MsgWndData {
	public:
		HANDLE	m_hContact;
		HWND	m_hwnd;
		int		m_contryID;
		
		MsgWndData(HWND hwnd, HANDLE hContact);
		~MsgWndData();

		void FlagsIconSet();
		void FlagsIconUnset();
		void FlagsIconUpdate() {
			gFlagsOpts.bShowStatusIconFlag ? FlagsIconSet():FlagsIconUnset();
		};
		void ContryIDchange(int ID) {
			m_contryID = ID; FlagsIconUpdate();
		};
};

class IconList {
	public:
		int				m_ID;
		HANDLE			m_hIcon;		//register
		HANDLE			m_hImage;		//return value from MS_CLIST_EXTRA_ADD_ICON	   -INVALID_HANDLE_VALUE;		//preset
		BYTE			m_TypeFlag;
		StatusIconData	m_StatusIconData;

		IconList(StatusIconData* sid);
//		IconList(HWND hwnd, HANDLE hContact);
		~IconList();

};

typedef void (CALLBACK *BUFFEREDPROC)(LPARAM lParam);
#ifdef _DEBUG
	void _CallFunctionBuffered(BUFFEREDPROC pfnBuffProc,const char *pszProcName,LPARAM lParam,BOOL fAccumulateSameParam,UINT uElapse);
	#define CallFunctionBuffered(proc,param,acc,elapse) _CallFunctionBuffered(proc,#proc,param,acc,elapse)
#else
	void _CallFunctionBuffered(BUFFEREDPROC pfnBuffProc,LPARAM lParam,BOOL fAccumulateSameParam,UINT uElapse);
	#define CallFunctionBuffered(proc,param,acc,elapse) _CallFunctionBuffered(proc,param,acc,elapse)
#endif

void EnsureExtraImages();
VOID SvcFlagsEnableExtraIcons(BOOLEAN bEnable, BOOLEAN bUpdateDB);
void CALLBACK UpdateStatusIcons(LPARAM lParam);

void SvcFlagsLoadModule();
void SvcFlagsOnModulesLoaded();
void SvcFlagsUnloadModule();

#endif /* _UINFOEX_FLAGS_H_INCLUDED_ */