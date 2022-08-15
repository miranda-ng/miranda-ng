/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org)
Copyright (c) 2000-04 Miranda ICQ/IM project,
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

#include "stdafx.h"

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR GetAvatarBitmap(WPARAM hContact, LPARAM)
{
	if (hContact == 0 || g_shutDown)
		return 0;

	hContact = GetContactThatHaveTheAvatar(hContact);

	// Get the node
	CacheNode *node = FindAvatarInCache(hContact, true);
	if (node == nullptr || !node->bLoaded)
		return (INT_PTR)GetProtoDefaultAvatar(hContact);
	return (INT_PTR)node;
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR ProtectAvatar(WPARAM hContact, LPARAM lParam)
{
	uint8_t was_locked = db_get_b(hContact, "ContactPhoto", "Locked", 0);

	if (was_locked == (uint8_t)lParam)      // no need for redundant lockings...
		return 0;

	if (hContact) {
		if (!was_locked)
			MakePathRelative(hContact);
		db_set_b(hContact, "ContactPhoto", "Locked", lParam ? 1 : 0);
		if (lParam == 0)
			MakePathRelative(hContact);
		ChangeAvatar(hContact, true);
	}
	return 0;
}

/*
 * set an avatar (service function)
 * if lParam == NULL, a open file dialog will be opened, otherwise, lParam is taken as a FULL
 * image filename (will be checked for existance, though)
 */

struct OpenFileSubclassData
{
	uint8_t *locking_request;
	uint8_t setView;
};

UINT_PTR CALLBACK OpenFileSubclass(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	OpenFileSubclassData *data = (OpenFileSubclassData *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwnd);
		{
			OPENFILENAME *ofn = (OPENFILENAME *)lParam;

			data = (OpenFileSubclassData *)malloc(sizeof(OpenFileSubclassData));
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)data);
			data->locking_request = (uint8_t *)ofn->lCustData;
			data->setView = TRUE;
			CheckDlgButton(hwnd, IDC_PROTECTAVATAR, *(data->locking_request) ? BST_CHECKED : BST_UNCHECKED);
		}
		break;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_PROTECTAVATAR)
			*(data->locking_request) = IsDlgButtonChecked(hwnd, IDC_PROTECTAVATAR) ? TRUE : FALSE;
		break;

	case WM_NOTIFY:
		if (data->setView) {
			HWND hwndParent = GetParent(hwnd);
			HWND hwndLv = FindWindowEx(hwndParent, nullptr, L"SHELLDLL_DefView", nullptr);
			if (hwndLv != nullptr) {
				SendMessage(hwndLv, WM_COMMAND, SHVIEW_THUMBNAIL, 0);
				data->setView = FALSE;
			}
		}
		break;

	case WM_NCDESTROY:
		free((OpenFileSubclassData *)data);
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)0);
		break;
	}

	return FALSE;
}

INT_PTR SetAvatar(WPARAM hContact, LPARAM lParam)
{
	if (hContact == NULL)
		return 0;

	int is_locked = db_get_b(hContact, "ContactPhoto", "Locked", 0);

	wchar_t *tszPath = (wchar_t*)lParam, *szFinalName;
	if (tszPath == nullptr) {
		wchar_t filter[256];
		Bitmap_GetFilter(filter, _countof(filter));

		wchar_t FileName[MAX_PATH];
		*FileName = '\0';

		OPENFILENAME ofn = { 0 };
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = nullptr;
		ofn.lpstrFile = FileName;
		ofn.lpstrFilter = filter;
		ofn.nMaxFile = MAX_PATH;
		ofn.nMaxFileTitle = MAX_PATH;
		ofn.Flags = OFN_FILEMUSTEXIST | OFN_ENABLETEMPLATE | OFN_EXPLORER | OFN_ENABLESIZING | OFN_ENABLEHOOK;
		ofn.lpstrInitialDir = L".";
		ofn.lpstrDefExt = L"";
		ofn.hInstance = g_plugin.getInst();
		ofn.lpTemplateName = MAKEINTRESOURCE(IDD_OPENSUBCLASS);
		ofn.lpfnHook = OpenFileSubclass;
		uint8_t locking_request = is_locked;
		ofn.lCustData = (LPARAM)&locking_request;
		if (!GetOpenFileName(&ofn))
			return 0;

		szFinalName = FileName;
		is_locked = locking_request ? 1 : is_locked;
	}
	else szFinalName = tszPath;

	// filename is now set, check it and perform all needed action
	if (_waccess(szFinalName, 4) == -1)
		return 0;

	// file exists...
	wchar_t szBackupName[MAX_PATH];
	PathToRelativeW(szFinalName, szBackupName, g_szDataPath);
	db_set_ws(hContact, "ContactPhoto", "Backup", szBackupName);

	db_set_b(hContact, "ContactPhoto", "Locked", is_locked);
	db_set_ws(hContact, "ContactPhoto", "File", szFinalName);
	MakePathRelative(hContact, szFinalName);

	// Fix cache
	ChangeAvatar(hContact, false, false, ProtoGetAvatarFileFormat(szFinalName));
	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////
// see if is possible to set the avatar for the expecified protocol

static INT_PTR CanSetMyAvatar(WPARAM wParam, LPARAM)
{
	char *protocol = (char *)wParam;
	if (protocol == nullptr)
		return 0;

	return ProtoServiceExists(protocol, PS_SETMYAVATAR);
}

/////////////////////////////////////////////////////////////////////////////////////////
// set an avatar for a protocol(service function)
// if lParam == NULL, a open file dialog will be opened, otherwise, lParam is taken as a FULL
// image filename (will be checked for existance, though)

static int InternalRemoveMyAvatar(char *protocol)
{
	SetIgnoreNotify(protocol, TRUE);

	// Remove avatar
	int ret = 0;
	if (protocol != nullptr) {
		if (ProtoServiceExists(protocol, PS_SETMYAVATAR))
			ret = SaveAvatar(protocol, nullptr);
		else
			ret = -3;

		if (ret == 0) {
			// Has global avatar?
			DBVARIANT dbv = { 0 };
			if (!g_plugin.getWString("GlobalUserAvatarFile", &dbv)) {
				db_free(&dbv);
				g_plugin.setByte("GlobalUserAvatarNotConsistent", 1);
				DeleteGlobalUserAvatar();
			}
		}
	}
	else {
		for (auto &it : Accounts()) {
			if (!ProtoServiceExists(it->szModuleName, PS_SETMYAVATAR))
				continue;

			if (!Proto_IsAvatarsEnabled(it->szModuleName))
				continue;

			// Found a protocol
			int retTmp = SaveAvatar(it->szModuleName, nullptr);
			if (retTmp != 0)
				ret = retTmp;
		}

		DeleteGlobalUserAvatar();

		if (ret)
			g_plugin.setByte("GlobalUserAvatarNotConsistent", 1);
		else
			g_plugin.setByte("GlobalUserAvatarNotConsistent", 0);
	}

	SetIgnoreNotify(protocol, FALSE);

	ReportMyAvatarChanged(WPARAM((protocol == nullptr) ? "" : protocol), 0);
	return ret;
}

static void FilterGetStrings(CMStringW &filter, BOOL xml, BOOL swf)
{
	filter.AppendFormat(L"%s (*.bmp;*.jpg;*.gif;*.png", TranslateT("All files"));
	if (swf) filter.Append(L";*.swf");
	if (xml) filter.Append(L";*.xml");

	filter.AppendFormat(L")%c*.BMP;*.RLE;*.JPG;*.JPEG;*.GIF;*.PNG", 0);
	if (swf) filter.Append(L";*.SWF");
	if (xml) filter.Append(L";*.XML");
	filter.AppendChar(0);

	filter.AppendFormat(L"%s (*.bmp;*.rle)%c*.BMP;*.RLE%c", TranslateT("Windows bitmaps"), 0, 0);
	filter.AppendFormat(L"%s (*.jpg;*.jpeg)%c*.JPG;*.JPEG%c", TranslateT("JPEG bitmaps"), 0, 0);
	filter.AppendFormat(L"%s (*.gif)%c*.GIF%c", TranslateT("GIF bitmaps"), 0, 0);
	filter.AppendFormat(L"%s (*.png)%c*.PNG%c", TranslateT("PNG bitmaps"), 0, 0);

	if (swf)
		filter.AppendFormat(L"%s (*.swf)%c*.SWF%c", TranslateT("Flash animations"), 0, 0);

	if (xml)
		filter.AppendFormat(L"%s (*.xml)%c*.XML%c", TranslateT("XML files"), 0, 0);

	filter.AppendChar(0);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Callback to set thumbnaill view to open dialog

static UINT_PTR CALLBACK SetMyAvatarHookProc(HWND hwnd, UINT msg, WPARAM, LPARAM lParam)
{
	OPENFILENAME *ofn = (OPENFILENAME *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	SetMyAvatarHookData *data;

	switch (msg) {
	case WM_INITDIALOG:
		hwndSetMyAvatar = hwnd;

		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)lParam);
		ofn = (OPENFILENAME *)lParam;

		data = (SetMyAvatarHookData *)ofn->lCustData;
		data->thumbnail = TRUE;

		SetDlgItemText(hwnd, IDC_MAKE_SQUARE, TranslateT("Make the avatar square"));
		SetDlgItemText(hwnd, IDC_GROW, TranslateT("Grow avatar to fit max allowed protocol size"));

		CheckDlgButton(hwnd, IDC_MAKE_SQUARE, data->square ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwnd, IDC_GROW, data->grow ? BST_CHECKED : BST_UNCHECKED);

		if (data->protocol != nullptr && (Proto_AvatarImageProportion(data->protocol) & PIP_SQUARE))
			EnableWindow(GetDlgItem(hwnd, IDC_MAKE_SQUARE), FALSE);
		break;

	case WM_NOTIFY:
		data = (SetMyAvatarHookData *)ofn->lCustData;
		if (data->thumbnail) {
			HWND hwndParent = GetParent(hwnd);
			HWND hwndLv = FindWindowEx(hwndParent, nullptr, L"SHELLDLL_DefView", nullptr);
			if (hwndLv != nullptr) {
				SendMessage(hwndLv, WM_COMMAND, SHVIEW_THUMBNAIL, 0);
				data->thumbnail = FALSE;
			}
		}
		break;

	case WM_DESTROY:
		data = (SetMyAvatarHookData *)ofn->lCustData;
		data->square = IsDlgButtonChecked(hwnd, IDC_MAKE_SQUARE);
		data->grow = IsDlgButtonChecked(hwnd, IDC_GROW);

		hwndSetMyAvatar = nullptr;
		break;
	}

	return 0;
}

struct SaveProtocolData
{
	uint32_t max_size;
	wchar_t image_file_name[MAX_PATH];
	BOOL saved;
	BOOL need_smaller_size;
	int width;
	int height;
	wchar_t temp_file[MAX_PATH];
	HBITMAP hBmpProto;
};

void SaveImage(SaveProtocolData &d, char *protocol, int format)
{
	if (!Proto_IsAvatarFormatSupported(protocol, format))
		return;

	mir_snwprintf(d.image_file_name, L"%s%s", d.temp_file, ProtoGetAvatarExtension(format));
	if (BmpFilterSaveBitmap(d.hBmpProto, d.image_file_name, format == PA_FORMAT_JPEG ? JPEG_QUALITYSUPERB : 0))
		return;

	if (d.max_size != 0 && GetFileSize(d.image_file_name) > d.max_size) {
		DeleteFile(d.image_file_name);

		if (format == PA_FORMAT_JPEG) {
			// Try with lower quality
			if (!BmpFilterSaveBitmap(d.hBmpProto, d.image_file_name, JPEG_QUALITYGOOD)) {
				if (GetFileSize(d.image_file_name) > d.max_size) {
					DeleteFile(d.image_file_name);
					d.need_smaller_size = TRUE;
				}
				else d.saved = TRUE;
			}
		}
		else d.need_smaller_size = TRUE;
	}
	else d.saved = TRUE;
}

static int SetProtoMyAvatar(char *protocol, HBITMAP hBmp, wchar_t *originalFilename, int originalFormat, BOOL square, BOOL grow)
{
	if (!ProtoServiceExists(protocol, PS_SETMYAVATAR))
		return -1;

	// If is swf or xml, just set it

	if (originalFormat == PA_FORMAT_SWF) {
		if (!Proto_IsAvatarFormatSupported(protocol, PA_FORMAT_SWF))
			return -1;

		return SaveAvatar(protocol, originalFilename);
	}

	if (originalFormat == PA_FORMAT_XML) {
		if (!Proto_IsAvatarFormatSupported(protocol, PA_FORMAT_XML))
			return -1;

		return SaveAvatar(protocol, originalFilename);
	}

	// Get protocol info
	SaveProtocolData d = { 0 };

	d.max_size = (uint32_t)Proto_GetAvatarMaxFileSize(protocol);

	Proto_GetAvatarMaxSize(protocol, &d.width, &d.height);
	int orig_width = d.width;
	int orig_height = d.height;

	if (Proto_AvatarImageProportion(protocol) & PIP_SQUARE)
		square = TRUE;

	// Try to save until a valid image is found or we give up
	int num_tries = 0;
	do {
		// Lets do it
		int fit = (grow ? 0 : RESIZEBITMAP_FLAG_DONT_GROW) | (square ? RESIZEBITMAP_MAKE_SQUARE : RESIZEBITMAP_KEEP_PROPORTIONS);
		d.hBmpProto = Image_Resize(hBmp, fit, d.width, d.height);

		if (d.hBmpProto == nullptr) {
			if (d.temp_file[0] != '\0')
				DeleteFile(d.temp_file);
			return -1;
		}

		// Check if can use original image
		if (d.hBmpProto == hBmp
			&& Proto_IsAvatarFormatSupported(protocol, originalFormat)
			&& (d.max_size == 0 || GetFileSize(originalFilename) < d.max_size)) {
			if (d.temp_file[0] != '\0')
				DeleteFile(d.temp_file);

			// Use original image
			return SaveAvatar(protocol, originalFilename);
		}

		// Create a temporary file (if was not created already)
		if (d.temp_file[0] == '\0') {
			d.temp_file[0] = '\0';
			if (GetTempPath(MAX_PATH, d.temp_file) == 0
				|| GetTempFileName(d.temp_file, L"mir_av_", 0, d.temp_file) == 0) {
				DeleteObject(d.hBmpProto);
				return -1;
			}
		}

		// Which format?

		// First try to use original format
		if (originalFormat != PA_FORMAT_BMP)
			SaveImage(d, protocol, originalFormat);

		if (!d.saved && originalFormat != PA_FORMAT_PNG)
			SaveImage(d, protocol, PA_FORMAT_PNG);

		if (!d.saved && originalFormat != PA_FORMAT_JPEG)
			SaveImage(d, protocol, PA_FORMAT_JPEG);

		if (!d.saved && originalFormat != PA_FORMAT_GIF)
			SaveImage(d, protocol, PA_FORMAT_GIF);

		if (!d.saved)
			SaveImage(d, protocol, PA_FORMAT_BMP);

		num_tries++;
		if (!d.saved && d.need_smaller_size && num_tries < 4) {
			// Cleanup
			if (d.hBmpProto != hBmp)
				DeleteObject(d.hBmpProto);

			// use a smaller size
			d.width = orig_width * (4 - num_tries) / 4;
			d.height = orig_height * (4 - num_tries) / 4;
		}
	} while (!d.saved && d.need_smaller_size && num_tries < 4);

	int ret;

	if (d.saved) {
		// Call proto service
		ret = SaveAvatar(protocol, d.image_file_name);
		DeleteFile(d.image_file_name);
	}
	else ret = -1;

	if (d.temp_file[0] != '\0')
		DeleteFile(d.temp_file);

	if (d.hBmpProto != hBmp)
		DeleteObject(d.hBmpProto);

	return ret;
}

static int InternalSetMyAvatar(char *protocol, wchar_t *szFinalName, SetMyAvatarHookData &data, BOOL allAcceptXML, BOOL allAcceptSWF)
{
	int format = ProtoGetAvatarFormat(szFinalName);
	if (format == PA_FORMAT_UNKNOWN || _waccess(szFinalName, 4) == -1)
		return -3;

	// file exists...
	HBITMAP hBmp = nullptr;

	if (format == PA_FORMAT_SWF) {
		if (!allAcceptSWF)
			return -4;
	}
	else if (format == PA_FORMAT_XML) {
		if (!allAcceptXML)
			return -4;
	}
	else {
		// Try to open if is not a flash or XML
		hBmp = Bitmap_Load(szFinalName);
		if (hBmp == nullptr)
			return -4;
	}

	SetIgnoreNotify(protocol, TRUE);

	int ret = 0;
	if (protocol != nullptr) {
		ret = SetProtoMyAvatar(protocol, hBmp, szFinalName, format, data.square, data.grow);
		if (ret == 0) {
			DeleteGlobalUserAvatar();
			g_plugin.setByte("GlobalUserAvatarNotConsistent", 1);
		}
	}
	else {
		for (auto &it : Accounts()) {
			if (!ProtoServiceExists(it->szModuleName, PS_SETMYAVATAR))
				continue;

			if (!Proto_IsAvatarsEnabled(it->szModuleName))
				continue;

			int retTmp = SetProtoMyAvatar(it->szModuleName, hBmp, szFinalName, format, data.square, data.grow);
			if (retTmp != 0)
				ret = retTmp;
		}

		DeleteGlobalUserAvatar();

		if (ret)
			g_plugin.setByte("GlobalUserAvatarNotConsistent", 1);
		else {
			// Copy avatar file to store as global one
			wchar_t globalFile[1024];
			BOOL saved = TRUE;
			if (FoldersGetCustomPathW(hGlobalAvatarFolder, globalFile, _countof(globalFile), L"")) {
				mir_snwprintf(globalFile, L"%s%s", g_szDataPath, L"GlobalAvatar");
				CreateDirectory(globalFile, nullptr);
			}

			wchar_t *ext = wcsrchr(szFinalName, '.'); // Can't be NULL here
			if (format == PA_FORMAT_XML || format == PA_FORMAT_SWF) {
				mir_snwprintf(globalFile, L"%s\\my_global_avatar%s", globalFile, ext);
				CopyFile(szFinalName, globalFile, FALSE);
			}
			else {
				// Resize (to avoid too big avatars)
				int fit = (data.grow ? 0 : RESIZEBITMAP_FLAG_DONT_GROW) | (data.square ? RESIZEBITMAP_MAKE_SQUARE : RESIZEBITMAP_KEEP_PROPORTIONS);
				HBITMAP hBmpTmp = Image_Resize(hBmp, fit, 300, 300);
				if (hBmpTmp == hBmp || hBmpTmp == nullptr) {
					// Use original image
					mir_snwprintf(globalFile, L"%s\\my_global_avatar%s", globalFile, ext);
					CopyFile(szFinalName, globalFile, FALSE);
				}
				else {
					// Save as PNG
					mir_snwprintf(globalFile, L"%s\\my_global_avatar.png", globalFile);
					if (BmpFilterSaveBitmap(hBmpTmp, globalFile, 0))
						saved = FALSE;

					DeleteObject(hBmpTmp);
				}
			}

			if (saved) {
				wchar_t relFile[1024];
				if (PathToRelativeW(globalFile, relFile, g_szDataPath))
					g_plugin.setWString("GlobalUserAvatarFile", relFile);
				else
					g_plugin.setWString("GlobalUserAvatarFile", globalFile);

				g_plugin.setByte("GlobalUserAvatarNotConsistent", 0);
			}
			else g_plugin.setByte("GlobalUserAvatarNotConsistent", 1);
		}
	}

	DeleteObject(hBmp);

	SetIgnoreNotify(protocol, FALSE);

	ReportMyAvatarChanged(WPARAM((protocol == nullptr) ? "" : protocol), 0);
	return ret;
}

INT_PTR SetMyAvatar(WPARAM wParam, LPARAM lParam)
{
	wchar_t FileName[MAX_PATH];
	wchar_t *szFinalName = nullptr;
	BOOL allAcceptXML;
	BOOL allAcceptSWF;

	// Protocol allow seting of avatar?
	char* protocol = (char*)wParam;
	if (protocol != nullptr && !CanSetMyAvatar((WPARAM)protocol, 0))
		return -1;

	wchar_t* tszPath = (wchar_t*)lParam;
	if (tszPath == nullptr && hwndSetMyAvatar != nullptr) {
		SetForegroundWindow(hwndSetMyAvatar);
		SetFocus(hwndSetMyAvatar);
		ShowWindow(hwndSetMyAvatar, SW_SHOW);
		return -2;
	}

	SetMyAvatarHookData data = {};

	// Check for XML and SWF
	if (protocol == nullptr) {
		allAcceptXML = TRUE;
		allAcceptSWF = TRUE;

		for (auto &it : Accounts()) {
			if (!ProtoServiceExists(it->szModuleName, PS_SETMYAVATAR))
				continue;

			if (!Proto_IsAvatarsEnabled(it->szModuleName))
				continue;

			allAcceptXML = allAcceptXML && Proto_IsAvatarFormatSupported(it->szModuleName, PA_FORMAT_XML);
			allAcceptSWF = allAcceptSWF && Proto_IsAvatarFormatSupported(it->szModuleName, PA_FORMAT_SWF);
		}

		data.square = g_plugin.getByte("SetAllwaysMakeSquare", 0);
	}
	else {
		allAcceptXML = Proto_IsAvatarFormatSupported(protocol, PA_FORMAT_XML);
		allAcceptSWF = Proto_IsAvatarFormatSupported(protocol, PA_FORMAT_SWF);

		data.protocol = protocol;
		data.square = (Proto_AvatarImageProportion(protocol) & PIP_SQUARE)
			|| g_plugin.getByte("SetAllwaysMakeSquare", 0);
	}

	if (tszPath == nullptr) {
		data.protocol = protocol;

		CMStringW filter;
		FilterGetStrings(filter, allAcceptXML, allAcceptSWF);

		wchar_t inipath[1024];
		FoldersGetCustomPathW(hMyAvatarsFolder, inipath, _countof(inipath), L".");

		OPENFILENAME ofn = { 0 };
		ofn.lStructSize = sizeof(ofn);
		ofn.lpstrFile = FileName;
		ofn.lpstrFilter = filter;
		ofn.nMaxFile = MAX_PATH;
		ofn.nMaxFileTitle = MAX_PATH;
		ofn.Flags = OFN_FILEMUSTEXIST | OFN_ENABLETEMPLATE | OFN_EXPLORER | OFN_ENABLESIZING | OFN_ENABLEHOOK;
		ofn.lpstrInitialDir = inipath;
		ofn.lpTemplateName = MAKEINTRESOURCE(IDD_SET_OWN_SUBCLASS);
		ofn.lpfnHook = SetMyAvatarHookProc;
		ofn.lCustData = (LPARAM)&data;

		*FileName = '\0';
		ofn.lpstrDefExt = L"";
		ofn.hInstance = g_plugin.getInst();

		wchar_t title[256];
		if (protocol == nullptr)
			mir_snwprintf(title, TranslateT("Set my avatar"));
		else {
			wchar_t* prototmp = mir_a2u(protocol);
			mir_snwprintf(title, TranslateT("Set my avatar for %s"), prototmp);
			mir_free(prototmp);
		}
		ofn.lpstrTitle = title;

		if (!GetOpenFileName(&ofn))
			return 1;

		szFinalName = FileName;
	}
	else szFinalName = (wchar_t*)tszPath;

	// filename is now set, check it and perform all needed action
	if (szFinalName[0] == '\0')
		return InternalRemoveMyAvatar(protocol);

	return InternalSetMyAvatar(protocol, szFinalName, data, allAcceptXML, allAcceptSWF);
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR DrawAvatarPicture(WPARAM, LPARAM lParam)
{
	AVATARCACHEENTRY *ace = nullptr;

	AVATARDRAWREQUEST *r = (AVATARDRAWREQUEST*)lParam;
	if (r == nullptr || IsBadReadPtr((void *)r, sizeof(AVATARDRAWREQUEST)))
		return 0;

	if (r->dwFlags & AVDRQ_PROTOPICT) {
		if (r->szProto == nullptr)
			return 0;

		for (auto &p : g_ProtoPictures) {
			if (!mir_strcmp(p->szProtoname, r->szProto) && mir_strlen(r->szProto) == mir_strlen(p->szProtoname) && p->hbmPic != nullptr) {
				ace = p;
				break;
			}
		}
	}
	else if (r->dwFlags & AVDRQ_OWNPIC) {
		if (r->szProto == nullptr)
			return 0;

		if (r->szProto[0] == '\0' && g_plugin.getByte("GlobalUserAvatarNotConsistent", 1))
			return -1;

		ace = (AVATARCACHEENTRY *)GetMyAvatar(0, (LPARAM)r->szProto);
	}
	else ace = (AVATARCACHEENTRY *)GetAvatarBitmap((WPARAM)r->hContact, 0);

	if (ace && (!(r->dwFlags & AVDRQ_RESPECTHIDDEN) || !(ace->dwFlags & AVS_HIDEONCLIST))) {
		ace->t_lastAccess = time(0);

		if (ace->bmHeight == 0 || ace->bmWidth == 0 || ace->hbmPic == nullptr)
			return 0;

		InternalDrawAvatar(r, ace->hbmPic, ace->bmWidth, ace->bmHeight, ace->dwFlags);
		return 1;
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR GetMyAvatar(WPARAM wParam, LPARAM lParam)
{
	if (wParam || g_shutDown)
		return 0;

	char *szProto = (char *)lParam;
	if (lParam == 0 || IsBadReadPtr(szProto, 4))
		return 0;

	for (auto &it : g_MyAvatars)
		if (!mir_strcmp(szProto, it->szProtoname) && it->hbmPic != nullptr)
			return (INT_PTR)it;

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static void ReloadMyAvatar(char *szProto)
{
	Thread_SetName("AVS: ReloadMyAvatar");

	mir_sleep(500);
	if (g_shutDown)
		return;

	for (auto &it : g_MyAvatars) {
		char *myAvatarProto = it->szProtoname;

		if (szProto[0] == 0) {
			// Notify to all possibles
			if (mir_strcmp(myAvatarProto, szProto)) {
				if (!ProtoServiceExists(myAvatarProto, PS_SETMYAVATAR))
					continue;
				if (!Proto_IsAvatarsEnabled(myAvatarProto))
					continue;
			}

		}
		else if (mir_strcmp(myAvatarProto, szProto))
			continue;

		if (it->hbmPic)
			DeleteObject(it->hbmPic);

		if (CreateAvatarInCache(INVALID_CONTACT_ID, it, myAvatarProto) != -1)
			NotifyEventHooks(hMyAvatarChanged, (WPARAM)myAvatarProto, (LPARAM)it);
		else
			NotifyEventHooks(hMyAvatarChanged, (WPARAM)myAvatarProto, 0);
	}

	mir_free(szProto);
}

INT_PTR ReportMyAvatarChanged(WPARAM wParam, LPARAM)
{
	const char *proto = (const char*)wParam;
	if (proto == nullptr)
		return -1;

	for (auto &it : g_MyAvatars) {
		if (it->dwFlags & AVS_IGNORENOTIFY)
			continue;

		if (!mir_strcmp(it->szProtoname, proto)) {
			mir_forkThread<char>(ReloadMyAvatar, mir_strdup(it->szProtoname));
			return 0;
		}
	}

	return -2;
}

/////////////////////////////////////////////////////////////////////////////////////////

void InitServices()
{
	CreateServiceFunction(MS_AV_GETAVATARBITMAP, GetAvatarBitmap);
	CreateServiceFunction(MS_AV_PROTECTAVATAR, ProtectAvatar);
	CreateServiceFunction(MS_AV_SETAVATARW, SetAvatar);
	CreateServiceFunction(MS_AV_SETMYAVATARW, SetMyAvatar);
	CreateServiceFunction(MS_AV_CANSETMYAVATAR, CanSetMyAvatar);
	CreateServiceFunction(MS_AV_CONTACTOPTIONS, ContactOptions);
	CreateServiceFunction(MS_AV_DRAWAVATAR, DrawAvatarPicture);
	CreateServiceFunction(MS_AV_GETMYAVATAR, GetMyAvatar);
	CreateServiceFunction(MS_AV_REPORTMYAVATARCHANGED, ReportMyAvatarChanged);
}
