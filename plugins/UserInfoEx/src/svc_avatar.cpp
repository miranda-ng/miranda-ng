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

#include "commonheaders.h"

namespace NServices
{
	namespace NAvatar
	{
		static HANDLE ghChangedHook = NULL;

		static int GetContactAvatarFileName(LPCTSTR zodiac, LPSTR szFileName, int cchFileName)
		{
			if (!CallService(MS_DB_GETPROFILEPATH, (WPARAM)cchFileName, (LPARAM)szFileName)) {
				size_t len = mir_strlen(szFileName);

				CHAR tmp[64];
				if (WideCharToMultiByte(CP_ACP, 0, zodiac, 64, tmp, SIZEOF(tmp), 0, 0) > 0)
					mir_snprintf(szFileName + len, cchFileName - len, "\\avatars\\%s.png", tmp);

				return !PathFileExistsA(szFileName);
			}
			return 1;
		}

		static void SetZodiacAvatar(MCONTACT hContact)
		{
			MAnnivDate mtb;

			// try to load birthday for contact
			if (!mtb.DBGetBirthDate(hContact)) {
				MZodiac zodiac;
				//ICONINFO iinfo;
				CHAR szFileName[MAX_PATH];

				// get zodiac for birthday
				zodiac = mtb.Zodiac();

				if (!GetContactAvatarFileName(zodiac.pszName, szFileName, SIZEOF(szFileName))) {
					// extract the bitmap from the icon
					//GetIconInfo(zodiac.hIcon, &iinfo);

					// save the bitmap to a file used as avatar later
					if (!CallService(MS_AV_SETAVATAR, hContact, (LPARAM)szFileName))
						db_set_b(hContact, "ContactPhoto", "IsZodiac", 1);
				}
			}
		}

		void DeleteAvatar(MCONTACT hContact)
		{
			if (hContact && db_get_b(hContact, "ContactPhoto", "IsZodiac", FALSE)) {
				db_unset(hContact, "ContactPhoto", "File");
				db_unset(hContact, "ContactPhoto", "RFile");
				db_unset(hContact, "ContactPhoto", "Backup");
				db_unset(hContact, "ContactPhoto", "ImageHash");

				db_set_b(hContact, "ContactPhoto", "IsZodiac", 0);
			}
		}

		static int OnAvatarChanged(MCONTACT hContact, AVATARCACHEENTRY *ace)
		{
			if (hContact) {
				// check valid parameters
				if (ace) {
					if (// check for correct structure
						ace->cbSize == sizeof(AVATARCACHEENTRY) &&
						// set zodiac as avatar either if the desired avatar is invalid or a general protocol picture
						((ace->dwFlags & AVS_PROTOPIC) || !(ace->dwFlags & AVS_BITMAP_VALID))) {
						if (!db_get_b(hContact, "ContactPhoto", "IsZodiac", 0))
							SetZodiacAvatar(hContact);
					}
					else db_set_b(hContact, "ContactPhoto", "IsZodiac", 0);
				}

				// avatar was deleted, so we can set up a zodiac avatar
				else SetZodiacAvatar(hContact);
			}
			return 0;
		}

		void Enable(BYTE bEnable)
		{
			DBVARIANT dbv;

			if (bEnable && !ghChangedHook) {

				// walk through all the contacts stored in the DB
				for (MCONTACT hContact = db_find_first(); hContact != NULL; hContact = db_find_next(hContact)) {
					// don't set if avatar is locked!
					if (!db_get_b(hContact, "ContactPhoto", "Locked", 0)) {
						BYTE bInvalidAvatar = TRUE;

						// the relative file is valid
						if (!DB::Setting::GetAString(hContact, "ContactPhoto", "RFile", &dbv)) {
							CHAR absolute[MAX_PATH]; absolute[0] = 0;

							// check if file exists
							if (!PathToAbsolute(dbv.pszVal, absolute)) {
								FILE *f = fopen(absolute, "rb");
								if (f) {
									bInvalidAvatar = FALSE;
									fclose(f);
								}
							}
							db_free(&dbv);
						}

						// the absolute file is valid
						if (bInvalidAvatar && !db_get(hContact, "ContactPhoto", "File", &dbv)) {
							FILE *f = fopen(dbv.pszVal, "rb");
							if (f) {
								bInvalidAvatar = FALSE;
								fclose(f);
							}
							db_free(&dbv);
						}

						// set the zodiac as avatar
						if (bInvalidAvatar)
							SetZodiacAvatar(hContact);
					}
				}
				ghChangedHook = HookEvent(ME_AV_AVATARCHANGED, (MIRANDAHOOK)OnAvatarChanged);
			}
			else if (!bEnable && ghChangedHook) {
				UnhookEvent(ghChangedHook);
				ghChangedHook = NULL;

				// walk through all the contacts stored in the DB
				for (MCONTACT hContact = db_find_first(); hContact != NULL; hContact = db_find_next(hContact))
					DeleteAvatar(hContact);
			}
		}

		/**
		 * name:	OnModulesLoaded
		 * desc:	initialize stuff, which require all standard modules to bee loaded
		 * params:	none
		 * return:	0
		 **/
		void OnModulesLoaded()
		{
			Enable(db_get_b(NULL, MODNAME, SET_ZODIAC_AVATARS, FALSE));
		}

	} /* namespace NAvatar */
} /* namespace NServices */