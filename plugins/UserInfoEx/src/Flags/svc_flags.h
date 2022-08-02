/*
UserinfoEx plugin for Miranda IM

Copyright:
(c) 2006-2010 DeathAxe, Yasnovidyashii, Merlin, K. Romanov, Kreol

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
*/

#ifndef _UINFOEX_FLAGS_H_INCLUDED_
#define _UINFOEX_FLAGS_H_INCLUDED_

#define EXTRAIMAGE_REFRESHDELAY		100		/* time for which setting changes are buffered */
#define STATUSICON_REFRESHDELAY		100		/* time for which setting changes are buffered */

struct MsgWndData
{
	MCONTACT m_hContact;
	HWND m_hwnd;
	int m_countryID;

	MsgWndData(HWND hwnd, MCONTACT hContact);
	~MsgWndData();

	void FlagsIconSet();

	void ContryIDchange(int ID)
	{
		m_countryID = ID; FlagsIconSet();
	}
};

typedef void (CALLBACK *BUFFEREDPROC)(LPARAM lParam);
void CallFunctionBuffered(BUFFEREDPROC pfnBuffProc, LPARAM lParam, BOOL fAccumulateSameParam, UINT uElapse);

void UpdateStatusIcons();

void SvcFlagsLoadModule();
void SvcFlagsOnModulesLoaded();
void SvcFlagsUnloadModule();

#endif /* _UINFOEX_FLAGS_H_INCLUDED_ */
