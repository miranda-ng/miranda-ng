/*
UserinfoEx plugin for Miranda IM

Copyright:
� 2006-2010 DeathAxe, Yasnovidyashii, Merlin, K. Romanov, Kreol

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

File name      : $HeadURL: https://userinfoex.googlecode.com/svn/trunk/svc_avatar.cpp $
Revision       : $Revision: 187 $
Last change on : $Date: 2010-09-08 16:05:54 +0400 (Ср, 08 сен 2010) $
Last change by : $Author: ing.u.horn $

===============================================================================
*/

#include "commonheaders.h"
#include "m_protocols.h"
#include "m_png.h"
#include "m_avatars.h"

namespace NServices 
{
	namespace NAvatar 
	{

		static HANDLE ghChangedHook = NULL;

/*
		int SaveBitmapAsAvatar(HBITMAP hBitmap, LPCSTR szFileName) 
		{
			BITMAPINFO* bmi;
			HDC hdc;
			HBITMAP hOldBitmap;
			BITMAPINFOHEADER* pDib;
			BYTE* pDibBits;
			long dwPngSize = 0;
			DIB2PNG convertor;
			FILE* out;

			// file exists?
			out = fopen(szFileName, "rb");
			if (out != NULL) {
				fclose(out);
				return ERROR_SUCCESS;
			}	

			if (!ServiceExists(MS_DIB2PNG)) {
				MsgErr(NULL, TranslateT("Your png2dib.dll is either obsolete or damaged."));
				return 1;
			}

			hdc = CreateCompatibleDC(NULL);
			hOldBitmap = (HBITMAP)SelectObject(hdc, hBitmap);

			bmi = (BITMAPINFO*)_alloca(sizeof(BITMAPINFO) + sizeof(RGBQUAD) * 256);
			memset(bmi, 0, sizeof(BITMAPINFO));
			bmi->bmiHeader.biSize = 0x28;
			if (GetDIBits(hdc, hBitmap, 0, 96, NULL, bmi, DIB_RGB_COLORS) == 0) {
				return 2;
			}

			pDib = (BITMAPINFOHEADER*)GlobalAlloc(LPTR, sizeof(BITMAPINFO) + sizeof(RGBQUAD) * 256 + bmi->bmiHeader.biSizeImage);
			if (pDib == NULL)
				return 3;

			memcpy(pDib, bmi, sizeof(BITMAPINFO) + sizeof(RGBQUAD) * 256);
			pDibBits = ((BYTE*)pDib) + sizeof(BITMAPINFO) + sizeof(RGBQUAD) * 256;

			GetDIBits(hdc, hBitmap, 0, pDib->biHeight, pDibBits, (BITMAPINFO*)pDib, DIB_RGB_COLORS);
			SelectObject(hdc, hOldBitmap);
			DeleteDC(hdc);

			convertor.pbmi = (BITMAPINFO*)pDib;
			convertor.pDiData = pDibBits;
			convertor.pResult = NULL;
			convertor.pResultLen = &dwPngSize;
			if (!CallService(MS_DIB2PNG, 0, (LPARAM)&convertor)) {
				GlobalFree(pDib);
				return 2;
			}

			convertor.pResult = (PBYTE)mir_alloc(dwPngSize);
			CallService(MS_DIB2PNG, 0, (LPARAM)&convertor);

			GlobalFree(pDib);
				
			out = fopen(szFileName, "wb");
			if (out != NULL) {
				fwrite(convertor.pResult, dwPngSize, 1, out);
				fclose(out);
			}	

			mir_free(convertor.pResult);

			return ERROR_SUCCESS;
		}
*/

		static INT GetContactAvatarFileName(LPCTSTR zodiac, LPSTR szFileName, INT cchFileName)
		{
			if (!CallService(MS_DB_GETPROFILEPATH, (WPARAM)cchFileName, (LPARAM)szFileName)) 
			{
				size_t len = mir_strlen(szFileName);
		#ifdef _UNICODE
				CHAR tmp[64];

				if (WideCharToMultiByte(CP_ACP, 0, zodiac, 64, tmp, SIZEOF(tmp),0,0) > 0) 
				{
					mir_snprintf(szFileName + len, cchFileName - len, "\\avatars\\%s.png", tmp);
				}
		#else
				mir_snprintf(szFileName + len, cchFileName - len, "\\avatars\\%s.png", zodiac);
		#endif
				return !PathFileExistsA(szFileName);
			}
			return 1;
		}

		/**
		 *
		 *
		 **/
		static VOID SetZodiacAvatar(HANDLE hContact) 
		{
			MAnnivDate mtb;

			// try to load birthday for contact
			if (!mtb.DBGetBirthDate(hContact))
			{
				MZodiac zodiac;
				//ICONINFO iinfo;
				CHAR szFileName[MAX_PATH];
				
				// get zodiac for birthday
				zodiac = mtb.Zodiac();

				if (!GetContactAvatarFileName(zodiac.pszName, szFileName, SIZEOF(szFileName))) 
				{
					// extract the bitmap from the icon
					//GetIconInfo(zodiac.hIcon, &iinfo);

					// save the bitmap to a file used as avatar later
					//if (!SaveBitmapAsAvatar(iinfo.hbmColor, szFileName)) 
					{
						if (!CallService(MS_AV_SETAVATAR, (WPARAM)hContact, (LPARAM)szFileName))
						{
							DB::Setting::WriteByte(hContact, "ContactPhoto", "IsZodiac", 1);
						}
					}
				}
			}
		}

		VOID DeleteAvatar(HANDLE hContact)
		{
			if (hContact && DB::Setting::GetByte(hContact, "ContactPhoto", "IsZodiac", FALSE))
			{
				//AVATARCACHEENTRY *ace;
				LPSTR szProto = DB::Contact::Proto(hContact);

				DB::Setting::Delete(hContact, "ContactPhoto", "File");
				DB::Setting::Delete(hContact, "ContactPhoto", "RFile");
				DB::Setting::Delete(hContact, "ContactPhoto", "Backup");
				DB::Setting::Delete(hContact, "ContactPhoto", "ImageHash");

				DB::Setting::WriteByte(hContact, "ContactPhoto", "IsZodiac", 0);
				
				/*
				ace = (AVATARCACHEENTRY *)CallService(MS_AV_GETMYAVATAR, NULL, (LPARAM)szProto);
				if (ace)
				{
					if (!CallService(MS_AV_SETAVATAR, (WPARAM)hContact, (LPARAM)ace->szFilename))
					{
						DB::Setting::WriteByte(hContact, "ContactPhoto", "IsZodiac", 0);
					}
				}
				*/
			}
		}


		/**
		 *
		 *
		 **/
		static INT OnAvatarChanged(HANDLE hContact, AVATARCACHEENTRY *ace)
		{
			if (hContact)
			{
				// check valid parameters
				if (ace)
				{
					if (// check for correct structure
							ace->cbSize == sizeof(AVATARCACHEENTRY) && 
							// set zodiac as avatar either if the desired avatar is invalid or a general protocol picture
							((ace->dwFlags & AVS_PROTOPIC) || !(ace->dwFlags & AVS_BITMAP_VALID)))
					{
						if (!DB::Setting::GetByte(hContact, "ContactPhoto", "IsZodiac", 0))
						{
							SetZodiacAvatar(hContact);
						}
					}
					else
					{
						DB::Setting::WriteByte(hContact, "ContactPhoto", "IsZodiac", 0);
					}
				}

				// avatar was deleted, so we can set up a zodiac avatar
				else
				{
					SetZodiacAvatar(hContact);
				}
			}
			return 0;
		}

		/**
		 *
		 *
		 **/
		VOID Enable(BOOLEAN bEnable) 
		{
			HANDLE hContact;
			DBVARIANT dbv;

			if (bEnable && !ghChangedHook) 
			{

				//walk through all the contacts stored in the DB
				for (hContact = DB::Contact::FindFirst();
					hContact != NULL;
					hContact = DB::Contact::FindNext(hContact))
				{
					// don't set if avatar is locked!
					if (!DB::Setting::GetByte(hContact, "ContactPhoto", "Locked", 0)) 
					{
						BOOLEAN bInvalidAvatar = TRUE;
						
						// the relative file is valid
						if (!DB::Setting::GetAString(hContact, "ContactPhoto", "RFile", &dbv)) 
						{
							CHAR absolute[MAX_PATH];
							absolute[0] = '\0';

							// check if file exists
							if (!CallService(MS_UTILS_PATHTOABSOLUTE, (WPARAM)dbv.pszVal, (LPARAM)absolute)) 
							{
								FILE *f = fopen(absolute, "rb");
								if (f) {
									bInvalidAvatar = FALSE;
									fclose(f);
								}
							}
							DB::Variant::Free(&dbv);
						}

						// the absolute file is valid
						if (bInvalidAvatar && !DBGetContactSetting(hContact, "ContactPhoto", "File", &dbv)) 
						{
							FILE *f = fopen(dbv.pszVal, "rb");
							if (f) {
								bInvalidAvatar = FALSE;
								fclose(f);
							}
							DB::Variant::Free(&dbv);
						}
						
						// set the zodiac as avatar
						if (bInvalidAvatar) {
							SetZodiacAvatar(hContact);
						}
					}
				}
				ghChangedHook = HookEvent(ME_AV_AVATARCHANGED, (MIRANDAHOOK) OnAvatarChanged);
			}
			else if (!bEnable && ghChangedHook) 
			{
				UnhookEvent(ghChangedHook);
				ghChangedHook = NULL;

				//walk through all the contacts stored in the DB
				for (hContact = DB::Contact::FindFirst();
						 hContact != NULL;
						 hContact = DB::Contact::FindNext(hContact))
				{
					DeleteAvatar(hContact);
				}
			}
		}


		/**
		 * name:	OnModulesLoaded
		 * desc:	initialize stuff, which require all standard modules to bee loaded
		 * params:	none
		 * return:	0
		 **/
		VOID OnModulesLoaded()
		{
			Enable(DB::Setting::GetByte(SET_ZODIAC_AVATARS, FALSE));
		}

	} /* namespace NAvatar */
} /* namespace NServices */