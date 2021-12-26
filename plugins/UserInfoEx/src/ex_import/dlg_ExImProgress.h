/*
UserinfoEx plugin for Miranda IM

Copyright:
© 2006-2010 DeathAxe, Yasnovidyashii, Merlin, K. Romanov, Kreol

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

#ifndef _DLG_EXIMPROGRESS_H_
#define _DLG_EXIMPROGRESS_H_

#pragma once

class CProgress
{
	HWND	_hDlg;
	uint32_t	_dwStartTime;

	uint8_t	Update();

public:
	CProgress();
	~CProgress();

	void Hide();

	void SetContactCount(uint32_t numContacts);
	void SetSettingsCount(uint32_t numSettings);

	uint8_t UpdateContact(LPCTSTR pszFormat, ...);
	uint8_t UpdateSetting(LPCTSTR pszFormat, ...);
};

#endif /* _DLG_EXIMPROGRESS_H_ */
