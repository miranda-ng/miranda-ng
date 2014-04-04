#include "common.h"
//#include <gdiplus.h>
//#pragma comment(lib, "GdiPlus.lib")

INT_PTR CALLBACK CSteamProto::GuardProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//HDC hdc;
	//HBITMAP hBitmap, cBitmap;
	GuardParam *guard = (GuardParam*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	switch (message)
	{
	case WM_INITDIALOG:
		TranslateDialogDefault(hwnd);
		{
			guard = (GuardParam*)lParam;
			SetWindowLongPtr(hwnd, GWLP_USERDATA, lParam);
			// load steam icon
			char iconName[100];
			mir_snprintf(iconName, SIZEOF(iconName), "%s_%s", MODULE, "main");
			SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)Skin_GetIcon(iconName, 16));
			SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)Skin_GetIcon(iconName, 32));
		}
		Utils_RestoreWindowPositionNoSize(hwnd, 0, "STEAM", "GuardWindow");
		return TRUE;

	case WM_CLOSE:
		Skin_ReleaseIcon((HICON)SendMessage(hwnd, WM_SETICON, ICON_BIG, 0));
		Skin_ReleaseIcon((HICON)SendMessage(hwnd, WM_SETICON, ICON_SMALL, 0));
		Utils_SaveWindowPosition(hwnd, NULL, "STEAM", "GuardWindow");
		EndDialog(hwnd, 0);
		break;

	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case IDC_GETDOMAIN:
			CallService(MS_UTILS_OPENURL, 0, (LPARAM)guard->domain);
			SetFocus(GetDlgItem(hwnd, IDC_TEXT));
			break;

		case IDOK:
			GetDlgItemTextA(hwnd, IDC_TEXT, guard->code, sizeof(guard->code));
			EndDialog(hwnd, IDOK);
			break;

		case IDCANCEL:
			EndDialog(hwnd, IDCANCEL);
			break;
		}
	}
		break;
	}

	return FALSE;
}

INT_PTR CALLBACK CSteamProto::CaptchaProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//HDC hdc;
	//HBITMAP hBitmap, cBitmap;
	CaptchaParam *captcha = (CaptchaParam*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	switch (message)
	{
	case WM_INITDIALOG:
		TranslateDialogDefault(hwnd);
		{
			captcha = (CaptchaParam*)lParam;
			SetWindowLongPtr(hwnd, GWLP_USERDATA, lParam);

			//HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, captcha->size);
			//LPVOID pImage = GlobalLock(hMem);
			//memcpy(pImage, captcha->data, captcha->size);
			//GlobalUnlock(hMem);

			//IStream* pStream = NULL;
			//if (CreateStreamOnHGlobal(hMem, FALSE, &pStream) == S_OK)
			//{
			//	HANDLE hBmp = CreateBitmap(260, 40, 1, 24, captcha->data);
			//	//Gdiplus::Bitmap *pBitmap = Gdiplus::Bitmap::FromStream(pStream);
			//	SendDlgItemMessage(hwnd, IDC_CAPTCHA, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)pBitmap);
			//	delete pBitmap;
			//	pStream->Release();
			//}

			//HANDLE hBmp = CreateBitmap(260, 40, 1, 24, captcha->data);

			/*tagBITMAPFILEHEADER bfh = *(tagBITMAPFILEHEADER*)captcha->data;
			tagBITMAPINFOHEADER bih = *(tagBITMAPINFOHEADER*)(captcha->data + sizeof(tagBITMAPFILEHEADER));
			RGBQUAD rgb = *(RGBQUAD*)(captcha->data + sizeof(tagBITMAPFILEHEADER) + sizeof(tagBITMAPINFOHEADER));

			BITMAPINFO bi;
			bi.bmiColors[0] = rgb;
			bi.bmiHeader = bih;

			char* pPixels = ((char*)captcha->data + bfh.bfOffBits);

			char* ppvBits;
			hBitmap = CreateDIBSection(NULL, &bi, DIB_RGB_COLORS, (void**)&ppvBits, NULL, 0);
			SetDIBits(NULL, hBitmap, 0, bih.biHeight, pPixels, &bi, DIB_RGB_COLORS);
			GetObject(hBitmap, sizeof(BITMAP), &cBitmap);*/
		}
		return TRUE;

	case WM_CLOSE:
		EndDialog(hwnd, 0);
		break;

		//case WM_PAINT:
		//	{
		//		PAINTSTRUCT ps;
		//		HDC hdc = BeginPaint(hwnd, &ps);
		//		HBITMAP hBitmap = CreateBitmap(260, 40, 1, 24, captcha->data);
		//		if(hBitmap != 0)
		//		{
		//			HDC hdcMem = CreateCompatibleDC(hdc);
		//			SelectObject(hdcMem, hBitmap);
		//			BitBlt(hdc, 10, 10, 260, 40, hdcMem, 0, 0, SRCCOPY);
		//			DeleteDC(hdcMem);
		//		}
		//		EndPaint(hwnd, &ps);
		//	}
		//	return 0;

	//case WM_DRAWITEM:
	//	{
	//		LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT)lParam;

	//		if (dis->CtlType == ODT_BUTTON && dis->CtlID == IDC_CAPTCHA)
	//		{
	//			FI_INTERFACE *fei = 0;

	//			FIBITMAP *fb = fei->FI_CreateDIBFromHBITMAP(hbm);
	//			FIBITMAP *fbResized = fei->FI_Rescale(fb, newWidth, newHeight, FILTER_BICUBIC);
	//			HBITMAP hbmResized = fei->FI_CreateHBITMAPFromDIB(fbResized);
	//			fei->FI_Unload(fb);
	//			fei->FI_Unload(fbResized);

	//			HBITMAP hbmTempOld;
	//			HDC hdcTemp = CreateCompatibleDC(r->hTargetDC);
	//			hbmTempOld = (HBITMAP)SelectObject(hdcTemp, hbmResized);

	//			GdiAlphaBlend(
	//				r->hTargetDC, r->rcDraw.left + leftoffset, r->rcDraw.top + topoffset, newWidth, newHeight,
	//				hdcTemp, 0, 0, newWidth, newHeight, bf);

	//			SelectObject(hdcTemp, hbmTempOld);
	//			DeleteObject(hbmResized);
	//			DeleteDC(hdcTemp);

	//			/*AVATARDRAWREQUEST avdrq = {0};
	//			avdrq.cbSize = sizeof(avdrq);
	//			avdrq.hTargetDC = dis->hDC;
	//			avdrq.dwFlags |= AVDRQ_PROTOPICT;
	//			avdrq.szProto = g_selectedProto;
	//			GetClientRect(GetDlgItem(hwndDlg, IDC_PROTOPIC), &avdrq.rcDraw);
	//			CallService(MS_AV_DRAWAVATAR, 0, (LPARAM)&avdrq);*/
	//		}
	//		return TRUE;
	//	}

	//case WM_PAINT:
	//	{
	//		PAINTSTRUCT ps;
	//		HDC hdc = BeginPaint(hwnd, &ps);
	//		//260x40
	//		Image image(L"Image.png");
	//		// Draw the original source image.
	//		Graphics graphics(hdc);
	//		graphics.DrawImage(&image, 10, 10);
	//		EndPaint(hwnd, &ps);
	//	}
	//	return 0;

	case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
			case IDOK:
				GetDlgItemTextA(hwnd, IDC_TEXT, captcha->text, sizeof(captcha->text));
				EndDialog(hwnd, IDOK);
				break;

			case IDCANCEL:
				EndDialog(hwnd, IDCANCEL);
				break;
			}
		}
		break;
	}

	return FALSE;
}

INT_PTR CALLBACK CSteamProto::MainOptionsProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	CSteamProto *proto = (CSteamProto*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	switch (message)
	{
	case WM_INITDIALOG:
		TranslateDialogDefault(hwnd);
		{
			proto = (CSteamProto*)lParam;
			SetWindowLongPtr(hwnd, GWLP_USERDATA, lParam);

			ptrW username(proto->getWStringA("Username"));
			SetDlgItemText(hwnd, IDC_USERNAME, username);

			ptrA password(proto->getStringA("Password"));
			SetDlgItemTextA(hwnd, IDC_PASSWORD, password);

			ptrW groupName(proto->getWStringA(NULL, "DefaultGroup"));
			SetDlgItemText(hwnd, IDC_GROUP, groupName);
			SendDlgItemMessage(hwnd, IDC_GROUP, EM_LIMITTEXT, 64, 0);

			if (proto->IsOnline())
			{
				EnableWindow(GetDlgItem(hwnd, IDC_USERNAME), FALSE);
				EnableWindow(GetDlgItem(hwnd, IDC_PASSWORD), FALSE);
				EnableWindow(GetDlgItem(hwnd, IDC_GROUP), FALSE);
			}
		}
		return TRUE;

	case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
			case IDC_USERNAME:
				{
					if ((HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus())) return 0;
					proto->delSetting("SteamID");
					wchar_t username[128];
					GetDlgItemText(hwnd, IDC_USERNAME, username, SIZEOF(username));
					SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
				}
				break;

			case IDC_PASSWORD:
				{
					if ((HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus())) return 0;
					proto->delSetting("TokenSecret");
					char password[128];
					GetDlgItemTextA(hwnd, IDC_PASSWORD, password, SIZEOF(password));
					SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
				}
				break;

			case IDC_GROUP:
				{
					if ((HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus()))
						return 0;
					SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
				}
				break;
			}
		}
		break;

	case WM_NOTIFY:
		if (reinterpret_cast<NMHDR*>(lParam)->code == PSN_APPLY)
		{
			if (!proto->IsOnline())
			{
				wchar_t username[128];
				GetDlgItemText(hwnd, IDC_USERNAME, username, SIZEOF(username));
				proto->setWString("Username", username);

				char password[128];
				GetDlgItemTextA(hwnd, IDC_PASSWORD, password, SIZEOF(password));
				proto->setString("Password", password);

				wchar_t groupName[128];
				GetDlgItemText(hwnd, IDC_GROUP, groupName, SIZEOF(groupName));
				if (lstrlen(groupName) > 0)
				{
					proto->setWString(NULL, "DefaultGroup", groupName);
					Clist_CreateGroup(0, groupName);
				}
				else
					proto->delSetting(NULL, "DefaultGroup");
			}

			return TRUE;
		}
		break;
	}

	return FALSE;
}