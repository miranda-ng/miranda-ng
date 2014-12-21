/*
Popup Plus plugin for Miranda IM

Copyright	© 2002 Luca Santarelli,
© 2004-2007 Victor Pavlychko
© 2010 MPK
© 2010 Merlin_de

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

#include "headers.h"

PopupAvatar *PopupAvatar::create(MCONTACT hContact)
{
	if (hContact)
	{
		if (ServiceExists(MS_AV_GETAVATARBITMAP))
		{
			avatarCacheEntry *av = (avatarCacheEntry *)CallService(MS_AV_GETAVATARBITMAP, hContact, 0);
			//			MessageBox(NULL, _T("00"), _T(MODULNAME_LONG), MB_OK);
			if (av && (_tcslen(av->szFilename) > 4))
			{
				//				MessageBox(NULL, _T("01"), _T(MODULNAME_LONG), MB_OK);
				if (!_tcsicmp(av->szFilename + _tcslen(av->szFilename) - 4, _T(".gif")))
				{
					//					MessageBox(NULL, _T("02"), _T(MODULNAME_LONG), MB_OK);
					if (db_get_b(NULL, MODULNAME, "EnableGifAnimation", 1) && GDIPlus_IsAnimatedGIF(av->szFilename))
					{
						//						MessageBox(NULL, _T("03"), _T(MODULNAME_LONG), MB_OK);
						return new GifAvatar(hContact);
					}
				}
			}
		}
	}

	return new SimpleAvatar((HANDLE)hContact);
}
