/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org)
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

#include "commonheaders.h"

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR GetAvatarBitmap(WPARAM hContact, LPARAM lParam)
{
	if (hContact == 0 || g_shutDown || fei == NULL)
		return 0;

	hContact = GetContactThatHaveTheAvatar(hContact);

	// Get the node
	CacheNode *node = FindAvatarInCache(hContact, TRUE);
	if (node == NULL || !node->loaded)
		return (INT_PTR)GetProtoDefaultAvatar(hContact);
	return (INT_PTR)node;
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR ProtectAvatar(WPARAM hContact, LPARAM lParam)
{
	BYTE was_locked = db_get_b(hContact, "ContactPhoto", "Locked", 0);

	if (fei == NULL || was_locked == (BYTE)lParam)      // no need for redundant lockings...
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
	BYTE *locking_request;
	BYTE setView;
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
			data->locking_request = (BYTE *)ofn->lCustData;
			data->setView = TRUE;
			CheckDlgButton(hwnd, IDC_PROTECTAVATAR, *(data->locking_request));
		}
		break;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_PROTECTAVATAR)
			*(data->locking_request) = IsDlgButtonChecked(hwnd, IDC_PROTECTAVATAR) ? TRUE : FALSE;
		break;

	case WM_NOTIFY:
		if (data->setView) {
			HWND hwndParent = GetParent(hwnd);
			HWND hwndLv = FindWindowEx(hwndParent, NULL, _T("SHELLDLL_DefView"), NULL);
			if (hwndLv != NULL) {
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

static INT_PTR avSetAvatar(MCONTACT hContact, TCHAR *tszPath)
{
	TCHAR FileName[MAX_PATH];
	TCHAR *szFinalName;
	BYTE locking_request;

	if (hContact == NULL || fei == NULL)
		return 0;

	int is_locked = db_get_b(hContact, "ContactPhoto", "Locked", 0);

	if (tszPath == NULL) {
		OPENFILENAME ofn = { 0 };
		TCHAR filter[256];

		filter[0] = '\0';
		CallService(MS_UTILS_GETBITMAPFILTERSTRINGST, SIZEOF(filter), (LPARAM)filter);

		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = 0;
		ofn.lpstrFile = FileName;
		ofn.lpstrFilter = filter;
		ofn.nMaxFile = MAX_PATH;
		ofn.nMaxFileTitle = MAX_PATH;
		ofn.Flags = OFN_FILEMUSTEXIST | OFN_ENABLETEMPLATE | OFN_EXPLORER | OFN_ENABLESIZING | OFN_ENABLEHOOK;
		ofn.lpstrInitialDir = _T(".");
		*FileName = '\0';
		ofn.lpstrDefExt = _T("");
		ofn.hInstance = g_hInst;
		ofn.lpTemplateName = MAKEINTRESOURCE(IDD_OPENSUBCLASS);
		ofn.lpfnHook = OpenFileSubclass;
		locking_request = is_locked;
		ofn.lCustData = (LPARAM)&locking_request;
		if (!GetOpenFileName(&ofn))
			return 0;
		
		szFinalName = FileName;
		is_locked = locking_request ? 1 : is_locked;
	}
	else szFinalName = tszPath;

	// filename is now set, check it and perform all needed action
	HANDLE hFile = CreateFile(szFinalName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return 0;

	// file exists...
	CloseHandle(hFile);

	TCHAR szBackupName[MAX_PATH];
	PathToRelativeT(szFinalName, szBackupName, g_szDataPath);
	db_set_ts(hContact, "ContactPhoto", "Backup", szBackupName);

	db_set_b(hContact, "ContactPhoto", "Locked", is_locked);
	db_set_ts(hContact, "ContactPhoto", "File", szFinalName);
	MakePathRelative(hContact, szFinalName);

	// Fix cache
	ChangeAvatar(hContact, true);
	return 0;
}

INT_PTR SetAvatar(WPARAM wParam, LPARAM lParam)
{
	return avSetAvatar(wParam, _A2T((const char*)lParam));
}

INT_PTR SetAvatarW(WPARAM wParam, LPARAM lParam)
{
	return avSetAvatar(wParam, (TCHAR*)lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////
// see if is possible to set the avatar for the expecified protocol
 
static INT_PTR CanSetMyAvatar(WPARAM wParam, LPARAM lParam)
{
	char *protocol = (char *)wParam;
	if (protocol == NULL || fei == NULL)
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
	if (protocol != NULL) {
		if (ProtoServiceExists(protocol, PS_SETMYAVATAR))
			ret = SaveAvatar(protocol, NULL);
		else
			ret = -3;

		if (ret == 0) {
			// Has global avatar?
			DBVARIANT dbv = { 0 };
			if (!db_get_ts(NULL, AVS_MODULE, "GlobalUserAvatarFile", &dbv)) {
				db_free(&dbv);
				db_set_b(NULL, AVS_MODULE, "GlobalUserAvatarNotConsistent", 1);
				DeleteGlobalUserAvatar();
			}
		}
	}
	else {
		PROTOACCOUNT **accs;
		int i, count;

		ProtoEnumAccounts(&count, &accs);
		for (i = 0; i < count; i++) {
			if (!ProtoServiceExists(accs[i]->szModuleName, PS_SETMYAVATAR))
				continue;

			if (!Proto_IsAvatarsEnabled(accs[i]->szModuleName))
				continue;

			// Found a protocol
			int retTmp = SaveAvatar(accs[i]->szModuleName, NULL);
			if (retTmp != 0)
				ret = retTmp;
		}

		DeleteGlobalUserAvatar();

		if (ret)
			db_set_b(NULL, AVS_MODULE, "GlobalUserAvatarNotConsistent", 1);
		else
			db_set_b(NULL, AVS_MODULE, "GlobalUserAvatarNotConsistent", 0);
	}

	SetIgnoreNotify(protocol, FALSE);

	ReportMyAvatarChanged(WPARAM((protocol == NULL) ? "" : protocol), 0);
	return ret;
}

static void FilterGetStrings(CMString &filter, BOOL xml, BOOL swf)
{
	filter.AppendFormat(_T("%s (*.bmp;*.jpg;*.gif;*.png"), TranslateT("All Files"));
	if (swf) filter.Append(_T(";*.swf"));
	if (xml) filter.Append(_T(";*.xml"));
	
	filter.AppendFormat(_T(")%c*.BMP;*.RLE;*.JPG;*.JPEG;*.GIF;*.PNG"), 0);
	if (swf) filter.Append(_T(";*.SWF"));
	if (xml) filter.Append(_T(";*.XML"));

	filter.AppendFormat(_T("%s (*.bmp;*.rle)%c*.BMP;*.RLE%c"), TranslateT("Windows Bitmaps"), 0, 0);
	filter.AppendFormat(_T("%s (*.jpg;*.jpeg)%c*.JPG;*.JPEG%c"), TranslateT("JPEG Bitmaps"), 0, 0);
	filter.AppendFormat(_T("%s (*.gif)%c*.GIF%c"), TranslateT("GIF Bitmaps"), 0, 0);
	filter.AppendFormat(_T("%s (*.png)%c*.PNG%c"), TranslateT("PNG Bitmaps"), 0, 0);

	if (swf)
		filter.AppendFormat(_T("%s (*.swf)%c*.SWF%c"), TranslateT("Flash Animations"), 0, 0);

	if (xml)
		filter.AppendFormat(_T("%s (*.xml)%c*.XML%c"), TranslateT("XML Files"), 0, 0);
	
	filter.AppendChar(0);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Callback to set thumbnaill view to open dialog

static UINT_PTR CALLBACK SetMyAvatarHookProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
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

		SetWindowText(GetDlgItem(hwnd, IDC_MAKE_SQUARE), TranslateT("Make the avatar square"));
		SetWindowText(GetDlgItem(hwnd, IDC_GROW), TranslateT("Grow avatar to fit max allowed protocol size"));

		CheckDlgButton(hwnd, IDC_MAKE_SQUARE, data->square ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwnd, IDC_GROW, data->grow ? BST_CHECKED : BST_UNCHECKED);

		if (data->protocol != NULL && (Proto_AvatarImageProportion(data->protocol) & PIP_SQUARE))
			EnableWindow(GetDlgItem(hwnd, IDC_MAKE_SQUARE), FALSE);
		break;

	case WM_NOTIFY:
		data = (SetMyAvatarHookData *)ofn->lCustData;
		if (data->thumbnail) {
			HWND hwndParent = GetParent(hwnd);
			HWND hwndLv = FindWindowEx(hwndParent, NULL, _T("SHELLDLL_DefView"), NULL);
			if (hwndLv != NULL) {
				SendMessage(hwndLv, WM_COMMAND, SHVIEW_THUMBNAIL, 0);
				data->thumbnail = FALSE;
			}
		}
		break;

	case WM_DESTROY:
		data = (SetMyAvatarHookData *)ofn->lCustData;
		data->square = IsDlgButtonChecked(hwnd, IDC_MAKE_SQUARE);
		data->grow = IsDlgButtonChecked(hwnd, IDC_GROW);

		hwndSetMyAvatar = NULL;
		break;
	}

	return 0;
}

struct SaveProtocolData
{
	DWORD max_size;
	TCHAR image_file_name[MAX_PATH];
	BOOL saved;
	BOOL need_smaller_size;
	int width;
	int height;
	TCHAR temp_file[MAX_PATH];
	HBITMAP hBmpProto;
};

void SaveImage(SaveProtocolData &d, char *protocol, int format)
{
	if (!Proto_IsAvatarFormatSupported(protocol, format))
		return;

	mir_sntprintf(d.image_file_name, SIZEOF(d.image_file_name), _T("%s%s"), d.temp_file, ProtoGetAvatarExtension(format));
	if (BmpFilterSaveBitmapT(d.hBmpProto, d.image_file_name, format == PA_FORMAT_JPEG ? JPEG_QUALITYSUPERB : 0))
		return;

	if (d.max_size != 0 && GetFileSize(d.image_file_name) > d.max_size) {
		DeleteFile(d.image_file_name);

		if (format == PA_FORMAT_JPEG) {
			// Try with lower quality
			if (!BmpFilterSaveBitmapT(d.hBmpProto, d.image_file_name, JPEG_QUALITYGOOD)) {
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

static int SetProtoMyAvatar(char *protocol, HBITMAP hBmp, TCHAR *originalFilename, int originalFormat, BOOL square, BOOL grow)
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

	d.max_size = (DWORD)Proto_GetAvatarMaxFileSize(protocol);

	Proto_GetAvatarMaxSize(protocol, &d.width, &d.height);
	int orig_width = d.width;
	int orig_height = d.height;

	if (Proto_AvatarImageProportion(protocol) & PIP_SQUARE)
		square = TRUE;

	// Try to save until a valid image is found or we give up
	int num_tries = 0;
	do {
		// Lets do it
		ResizeBitmap rb;
		rb.size = sizeof(ResizeBitmap);
		rb.hBmp = hBmp;
		rb.max_height = d.height;
		rb.max_width = d.width;
		rb.fit = (grow ? 0 : RESIZEBITMAP_FLAG_DONT_GROW)
			| (square ? RESIZEBITMAP_MAKE_SQUARE : RESIZEBITMAP_KEEP_PROPORTIONS);

		d.hBmpProto = (HBITMAP)BmpFilterResizeBitmap((WPARAM)&rb, 0);

		if (d.hBmpProto == NULL) {
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
				 || GetTempFileName(d.temp_file, _T("mir_av_"), 0, d.temp_file) == 0) {
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
	}
		while (!d.saved && d.need_smaller_size && num_tries < 4);

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

static int InternalSetMyAvatar(char *protocol, TCHAR *szFinalName, SetMyAvatarHookData &data, BOOL allAcceptXML, BOOL allAcceptSWF)
{
	HANDLE hFile = 0;

	int format = ProtoGetAvatarFormat(szFinalName);
	if (format == PA_FORMAT_UNKNOWN || (hFile = CreateFile(szFinalName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
		return -3;

	CloseHandle(hFile);

	// file exists...

	HBITMAP hBmp = NULL;

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
		hBmp = (HBITMAP)CallService(MS_IMG_LOAD, (WPARAM)szFinalName, IMGL_TCHAR);
		if (hBmp == NULL)
			return -4;
	}

	SetIgnoreNotify(protocol, TRUE);

	int ret = 0;
	if (protocol != NULL) {
		ret = SetProtoMyAvatar(protocol, hBmp, szFinalName, format, data.square, data.grow);
		if (ret == 0) {
			DeleteGlobalUserAvatar();
			db_set_b(NULL, AVS_MODULE, "GlobalUserAvatarNotConsistent", 1);
		}
	}
	else {
		int count;
		PROTOACCOUNT **accs;
		ProtoEnumAccounts(&count, &accs);
		for (int i = 0; i < count; i++) {
			if (!ProtoServiceExists(accs[i]->szModuleName, PS_SETMYAVATAR))
				continue;

			if (!Proto_IsAvatarsEnabled(accs[i]->szModuleName))
				continue;

			int retTmp = SetProtoMyAvatar(accs[i]->szModuleName, hBmp, szFinalName, format, data.square, data.grow);
			if (retTmp != 0)
				ret = retTmp;
		}

		DeleteGlobalUserAvatar();

		if (ret)
			db_set_b(NULL, AVS_MODULE, "GlobalUserAvatarNotConsistent", 1);
		else {
			// Copy avatar file to store as global one
			TCHAR globalFile[1024];
			BOOL saved = TRUE;
			if (FoldersGetCustomPathT(hGlobalAvatarFolder, globalFile, SIZEOF(globalFile), _T(""))) {
				mir_sntprintf(globalFile, SIZEOF(globalFile), _T("%s%s"), g_szDataPath, _T("GlobalAvatar"));
				CreateDirectory(globalFile, NULL);
			}

			TCHAR *ext = _tcsrchr(szFinalName, _T('.')); // Can't be NULL here
			if (format == PA_FORMAT_XML || format == PA_FORMAT_SWF) {
				mir_sntprintf(globalFile, SIZEOF(globalFile), _T("%s\\my_global_avatar%s"), globalFile, ext);
				CopyFile(szFinalName, globalFile, FALSE);
			}
			else {
				// Resize (to avoid too big avatars)
				ResizeBitmap rb = { 0 };
				rb.size = sizeof(ResizeBitmap);
				rb.hBmp = hBmp;
				rb.max_height = 300;
				rb.max_width = 300;
				rb.fit = (data.grow ? 0 : RESIZEBITMAP_FLAG_DONT_GROW)
					| (data.square ? RESIZEBITMAP_MAKE_SQUARE : RESIZEBITMAP_KEEP_PROPORTIONS);

				HBITMAP hBmpTmp = (HBITMAP)BmpFilterResizeBitmap((WPARAM)&rb, 0);

				// Check if need to resize
				if (hBmpTmp == hBmp || hBmpTmp == NULL) {
					// Use original image
					mir_sntprintf(globalFile, SIZEOF(globalFile), _T("%s\\my_global_avatar%s"), globalFile, ext);
					CopyFile(szFinalName, globalFile, FALSE);
				}
				else {
					// Save as PNG
					mir_sntprintf(globalFile, SIZEOF(globalFile), _T("%s\\my_global_avatar.png"), globalFile);
					if (BmpFilterSaveBitmap((WPARAM)hBmpTmp, (LPARAM)globalFile))
						saved = FALSE;

					DeleteObject(hBmpTmp);
				}
			}

			if (saved) {
				TCHAR relFile[1024];
				if (PathToRelativeT(globalFile, relFile, g_szDataPath))
					db_set_ts(NULL, AVS_MODULE, "GlobalUserAvatarFile", relFile);
				else
					db_set_ts(NULL, AVS_MODULE, "GlobalUserAvatarFile", globalFile);

				db_set_b(NULL, AVS_MODULE, "GlobalUserAvatarNotConsistent", 0);
			}
			else db_set_b(NULL, AVS_MODULE, "GlobalUserAvatarNotConsistent", 1);
		}
	}

	DeleteObject(hBmp);

	SetIgnoreNotify(protocol, FALSE);

	ReportMyAvatarChanged(WPARAM((protocol == NULL) ? "" : protocol), 0);
	return ret;
}

INT_PTR avSetMyAvatar(char* protocol, TCHAR* tszPath)
{
	TCHAR FileName[MAX_PATH];
	TCHAR *szFinalName = NULL;
	BOOL allAcceptXML;
	BOOL allAcceptSWF;

	// Protocol allow seting of avatar?
	if (protocol != NULL && !CanSetMyAvatar((WPARAM)protocol, 0))
		return -1;

	if (tszPath == NULL && hwndSetMyAvatar != 0) {
		SetForegroundWindow(hwndSetMyAvatar);
		SetFocus(hwndSetMyAvatar);
		ShowWindow(hwndSetMyAvatar, SW_SHOW);
		return -2;
	}

	SetMyAvatarHookData data = { 0 };

	// Check for XML and SWF
	if (protocol == NULL) {
		allAcceptXML = TRUE;
		allAcceptSWF = TRUE;

		int count;
		PROTOACCOUNT **accs;
		ProtoEnumAccounts(&count, &accs);
		for (int i = 0; i < count; i++) {
			if (!ProtoServiceExists(accs[i]->szModuleName, PS_SETMYAVATAR))
				continue;

			if (!Proto_IsAvatarsEnabled(accs[i]->szModuleName))
				continue;

			allAcceptXML = allAcceptXML && Proto_IsAvatarFormatSupported(accs[i]->szModuleName, PA_FORMAT_XML);
			allAcceptSWF = allAcceptSWF && Proto_IsAvatarFormatSupported(accs[i]->szModuleName, PA_FORMAT_SWF);
		}

		data.square = db_get_b(0, AVS_MODULE, "SetAllwaysMakeSquare", 0);
	}
	else {
		allAcceptXML = Proto_IsAvatarFormatSupported(protocol, PA_FORMAT_XML);
		allAcceptSWF = Proto_IsAvatarFormatSupported(protocol, PA_FORMAT_SWF);

		data.protocol = protocol;
		data.square = (Proto_AvatarImageProportion(protocol) & PIP_SQUARE)
			|| db_get_b(0, AVS_MODULE, "SetAllwaysMakeSquare", 0);
	}

	if (tszPath == NULL) {
		data.protocol = protocol;

		CMString filter;
		FilterGetStrings(filter, allAcceptXML, allAcceptSWF);

		TCHAR inipath[1024];
		FoldersGetCustomPathT(hMyAvatarsFolder, inipath, SIZEOF(inipath), _T("."));

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
		ofn.lpstrDefExt = _T("");
		ofn.hInstance = g_hInst;

		TCHAR title[256];
		if (protocol == NULL)
			mir_sntprintf(title, SIZEOF(title), TranslateT("Set My Avatar"));
		else {
			TCHAR* prototmp = mir_a2t(protocol);
			mir_sntprintf(title, SIZEOF(title), TranslateT("Set My Avatar for %s"), prototmp);
			mir_free(prototmp);
		}
		ofn.lpstrTitle = title;

		if (!GetOpenFileName(&ofn))
			return 1;

		szFinalName = FileName;
	}
	else szFinalName = (TCHAR*)tszPath;

	// filename is now set, check it and perform all needed action
	if (szFinalName[0] == '\0')
		return InternalRemoveMyAvatar(protocol);

	return InternalSetMyAvatar(protocol, szFinalName, data, allAcceptXML, allAcceptSWF);
}

static INT_PTR SetMyAvatar(WPARAM wParam, LPARAM lParam)
{
	return avSetMyAvatar((char*)wParam, _A2T((const char*)lParam));
}

static INT_PTR SetMyAvatarW(WPARAM wParam, LPARAM lParam)
{
	return avSetMyAvatar((char*)wParam, (TCHAR*)lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK DlgProcAvatarOptions(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

static INT_PTR ContactOptions(WPARAM wParam, LPARAM lParam)
{
	if (wParam)
		CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_AVATAROPTIONS), 0, DlgProcAvatarOptions, (LPARAM)wParam);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR DrawAvatarPicture(WPARAM wParam, LPARAM lParam)
{
	AVATARCACHEENTRY *ace = NULL;

	AVATARDRAWREQUEST *r = (AVATARDRAWREQUEST*)lParam;
	if (fei == NULL || r == NULL || IsBadReadPtr((void *)r, sizeof(AVATARDRAWREQUEST)))
		return 0;

	if (r->cbSize != sizeof(AVATARDRAWREQUEST))
		return 0;

	if (r->dwFlags & AVDRQ_PROTOPICT) {
		if (r->szProto == NULL)
			return 0;

		for (int i = 0; i < g_ProtoPictures.getCount(); i++) {
			protoPicCacheEntry& p = g_ProtoPictures[i];
			if (!lstrcmpA(p.szProtoname, r->szProto) && lstrlenA(r->szProto) == lstrlenA(p.szProtoname) && p.hbmPic != 0) {
				ace = (AVATARCACHEENTRY *)&g_ProtoPictures[i];
				break;
			}
		}
	}
	else if (r->dwFlags & AVDRQ_OWNPIC) {
		if (r->szProto == NULL)
			return 0;

		if (r->szProto[0] == '\0' && db_get_b(NULL, AVS_MODULE, "GlobalUserAvatarNotConsistent", 1))
			return -1;

		ace = (AVATARCACHEENTRY *)GetMyAvatar(0, (LPARAM)r->szProto);
	}
	else ace = (AVATARCACHEENTRY *)GetAvatarBitmap((WPARAM)r->hContact, 0);

	if (ace && (!(r->dwFlags & AVDRQ_RESPECTHIDDEN) || !(ace->dwFlags & AVS_HIDEONCLIST))) {
		ace->t_lastAccess = time(NULL);

		if (ace->bmHeight == 0 || ace->bmWidth == 0 || ace->hbmPic == 0)
			return 0;

		InternalDrawAvatar(r, ace->hbmPic, ace->bmWidth, ace->bmHeight, ace->dwFlags);
		return 1;
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR GetMyAvatar(WPARAM wParam, LPARAM lParam)
{
	if (wParam || g_shutDown || fei == NULL)
		return 0;

	char *szProto = (char *)lParam;
	if (lParam == 0 || IsBadReadPtr(szProto, 4))
		return 0;

	for (int i = 0; i < g_MyAvatars.getCount(); i++)
		if (!lstrcmpA(szProto, g_MyAvatars[i].szProtoname) && g_MyAvatars[i].hbmPic != 0)
			return (INT_PTR)&g_MyAvatars[i];

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static void ReloadMyAvatar(LPVOID lpParam)
{
	char *szProto = (char *)lpParam;

	mir_sleep(500);
	for (int i = 0; !g_shutDown && i < g_MyAvatars.getCount(); i++) {
		char *myAvatarProto = g_MyAvatars[i].szProtoname;

		if (szProto[0] == 0) {
			// Notify to all possibles
			if (lstrcmpA(myAvatarProto, szProto)) {
				if (!ProtoServiceExists(myAvatarProto, PS_SETMYAVATAR))
					continue;
				if (!Proto_IsAvatarsEnabled(myAvatarProto))
					continue;
			}

		}
		else if (lstrcmpA(myAvatarProto, szProto))
			continue;

		if (g_MyAvatars[i].hbmPic)
			DeleteObject(g_MyAvatars[i].hbmPic);

		if (CreateAvatarInCache((MCONTACT)-1, &g_MyAvatars[i], myAvatarProto) != -1)
			NotifyEventHooks(hMyAvatarChanged, (WPARAM)myAvatarProto, (LPARAM)&g_MyAvatars[i]);
		else
			NotifyEventHooks(hMyAvatarChanged, (WPARAM)myAvatarProto, 0);
	}

	free(lpParam);
}

INT_PTR ReportMyAvatarChanged(WPARAM wParam, LPARAM lParam)
{
	const char *proto = (const char*)wParam;
	if (proto == NULL)
		return -1;

	for (int i = 0; i < g_MyAvatars.getCount(); i++) {
		if (g_MyAvatars[i].dwFlags & AVS_IGNORENOTIFY)
			continue;

		if (!lstrcmpA(g_MyAvatars[i].szProtoname, proto)) {
			LPVOID lpParam = (void *)malloc(lstrlenA(g_MyAvatars[i].szProtoname) + 2);
			strcpy((char *)lpParam, g_MyAvatars[i].szProtoname);
			mir_forkthread(ReloadMyAvatar, lpParam);
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
	CreateServiceFunction(MS_AV_SETAVATAR, SetAvatar);
	CreateServiceFunction(MS_AV_SETAVATARW, SetAvatarW);
	CreateServiceFunction(MS_AV_SETMYAVATAR, SetMyAvatar);
	CreateServiceFunction(MS_AV_SETMYAVATARW, SetMyAvatarW);
	CreateServiceFunction(MS_AV_CANSETMYAVATAR, CanSetMyAvatar);
	CreateServiceFunction(MS_AV_CONTACTOPTIONS, ContactOptions);
	CreateServiceFunction(MS_AV_DRAWAVATAR, DrawAvatarPicture);
	CreateServiceFunction(MS_AV_GETMYAVATAR, GetMyAvatar);
	CreateServiceFunction(MS_AV_REPORTMYAVATARCHANGED, ReportMyAvatarChanged);

	CreateServiceFunction(MS_AV_LOADBITMAP32, BmpFilterLoadBitmap32);
	CreateServiceFunction(MS_AV_SAVEBITMAP, BmpFilterSaveBitmap);
	CreateServiceFunction(MS_AV_CANSAVEBITMAP, BmpFilterCanSaveBitmap);
	CreateServiceFunction(MS_AV_RESIZEBITMAP, BmpFilterResizeBitmap);
}
