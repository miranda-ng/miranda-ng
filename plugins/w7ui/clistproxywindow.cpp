#include "headers.h"

CClistProxyWindow *g_clistProxyWnd = 0;
extern ITaskbarList3 *g_pTaskbarList;

CClistProxyWindow::CClistProxyWindow() : m_overlayEvents(5)
{
	g_clistProxyWnd = this;

	m_activeOverlay = 0;
	m_overlayIcon = 0;
	m_overlayText = 0;

	SetWindowText(hwnd(), _T("Miranda IM"));
	SendMessage(hwnd(), WM_SETICON, ICON_BIG, (LPARAM)LoadSkinnedIcon(SKINICON_OTHER_MIRANDA));
	SendMessage(hwnd(), WM_SETICON, ICON_SMALL, (LPARAM)LoadSkinnedIcon(SKINICON_OTHER_MIRANDA));

	SetEventHook(ME_CLIST_STATUSMODECHANGE, &CClistProxyWindow::OnStatusModeChanged);
	SetEventHook(ME_AV_MYAVATARCHANGED, &CClistProxyWindow::OnAvatarChanged);
	SetEventHook(ME_SYSTEM_MODULESLOADED, &CClistProxyWindow::OnModulesLoaded);
	SetEventHook(ME_PROTO_ACCLISTCHANGED, &CClistProxyWindow::OnAccListChanged);

	SetTimer(1, 500);
}

CClistProxyWindow::~CClistProxyWindow()
{
	KillObjectEventHooks(this);
}

int __cdecl CClistProxyWindow::OnStatusModeChanged(WPARAM wParam, LPARAM lParam)
{
	Update();
	return 0;
}
int __cdecl CClistProxyWindow::OnAvatarChanged(WPARAM wParam, LPARAM lParam)
{
	Update();
	return 0;
}
int __cdecl CClistProxyWindow::OnModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	Update();
	return 0;
}
int __cdecl CClistProxyWindow::OnAccListChanged(WPARAM wParam, LPARAM lParam)
{
	Update();
	return 0;
}

HANDLE CClistProxyWindow::SetEventHook(char *evt, int (__cdecl CClistProxyWindow::*fn)(WPARAM, LPARAM))
{
	return HookEventObj(evt, *(MIRANDAHOOKOBJ *)&fn, this);
}

void CClistProxyWindow::Flash()
{
	FlashWindow(hwnd(), TRUE);
}

void CClistProxyWindow::SetOverlayIcon(HICON hIcon)
{
	m_overlayIcon = hIcon;
	m_overlayIconHandle = 0;
}

void CClistProxyWindow::SetOverlayIconHandle(HANDLE hIcolibIcon)
{
	m_overlayIcon = 0;
	m_overlayIconHandle = hIcolibIcon;
}

void CClistProxyWindow::AddOverlayEvent(int idx)
{
	m_overlayEvents[idx].Push();
}

void CClistProxyWindow::RemoveOverlayEvent(int idx)
{
	m_overlayEvents[idx].Pop();
}

int CClistProxyWindow::AllocateOverlayEvent(HANDLE hIcolibIcon)
{
	m_overlayEvents.insert(new COverlayEventSlot(hIcolibIcon, _T("")), m_overlayEvents.getCount());
	return m_overlayEvents.getCount() - 1;
}

void CClistProxyWindow::Update()
{
	AddButton(LoadSkinnedIcon(SKINICON_OTHER_MIRANDA), TranslateT("Menu"), -1);

	int nAccounts = 0;
	PROTOACCOUNT **accounts;
	ProtoEnumAccounts(&nAccounts, &accounts);
	for (int i = 0; i < nAccounts; ++i)
	{
		if (!accounts[i]->bIsEnabled || !accounts[i]->bIsVisible || !accounts[i]->ppro || !(accounts[i]->ppro->GetCaps(PFLAGNUM_1) & PF1_IM))
			continue;

		HICON hIcon = LoadSkinnedProtoIcon(accounts[i]->szModuleName, CallProtoService(accounts[i]->szModuleName, PS_GETSTATUS, 0, 0));
		AddButton(hIcon, accounts[i]->tszAccountName, (INT_PTR)accounts[i]->tszAccountName);
	}
	UpdateButtons(g_pTaskbarList);

	InvalidateThumbnail();
}

void CClistProxyWindow::OnActivate(HWND hwndFrom)
{
	CallService(MS_CLIST_SHOWHIDE, 0, 0);

	HWND hwndClui = (HWND)CallService(MS_CLUI_GETHWND, 0, 0);
	if (hwndFrom != hwndClui && !IsIconic(hwndClui) && IsWindowVisible(hwndClui))
		SetForegroundWindow(hwndClui);
	else
		SetForegroundWindow(NULL);
}

void CClistProxyWindow::OnToolbar(int id, INT_PTR data)
{
	POINT pt; GetCursorPos(&pt);
	HMENU hMenu = NULL;
	HWND hwndClui = (HWND)CallService(MS_CLUI_GETHWND, 0, 0);

	switch (data)
	{
		case -1:
		{
			hMenu = (HMENU)CallService(MS_CLIST_MENUBUILDMAIN, 0, 0);
			break;
		}

		default:
		{
			hMenu = (HMENU)CallService(MS_CLIST_MENUGETSTATUS, 0, 0);
			int nItems = GetMenuItemCount(hMenu);
			for (int i = 0; i < nItems; ++i)
			{
				TCHAR buf[128];
				MENUITEMINFO mii = {0};
				mii.cbSize = sizeof(mii);
				mii.fMask = MIIM_STRING|MIIM_SUBMENU;
				mii.dwTypeData = buf;
				mii.cch = SIZEOF(buf);
				GetMenuItemInfo(hMenu, i, TRUE, &mii);

				if (mii.hSubMenu && !lstrcmp(mii.dwTypeData, (TCHAR*)data))
				{
					hMenu = mii.hSubMenu;
					break;
				}
			}
		}
	}

	SetForegroundWindow(hwndClui);
	TrackPopupMenu(hMenu, 0, pt.x, pt.y, 0, hwndClui, NULL);
}

void CClistProxyWindow::OnRenderThumbnail(int width, int height)
{
	HBITMAP hbmp = CreateDwmBitmap(width, height);
	HDC hdc = CreateCompatibleDC(0);
	SelectObject(hdc, hbmp);

	RGBQUAD rgb0, rgb1;
	rgb0.rgbRed =  0; rgb0.rgbGreen =  0; rgb0.rgbBlue =  0;
	rgb1.rgbRed = 19; rgb1.rgbGreen = 58; rgb1.rgbBlue = 89;
	DrawGradient(hdc, 0, 0, width, height, &rgb0, &rgb1);

	HFONT hfntSave = (HFONT)SelectObject(hdc, GetStockObject(DEFAULT_GUI_FONT));
	SetTextColor(hdc, RGB(255, 255, 255));
	SetBkMode(hdc, TRANSPARENT);

	RECT rc;
	SIZE sz;
	SetRect(&rc, 5, 5, width-10, height-10);

	int avatarWidth = 0;
	int avatarHeight = 0;
	if (true)
	{
		AVATARCACHEENTRY *ace = (AVATARCACHEENTRY *)CallService(MS_AV_GETMYAVATAR, 0, (LPARAM)"");
		if (ace && (ace != (AVATARCACHEENTRY *)CALLSERVICE_NOTFOUND))
		{
			if (ace->bmWidth < width / 4)
			{
				avatarWidth = ace->bmWidth;
				avatarHeight = ace->bmHeight;
			} else
			{
				avatarWidth = width / 4;
				avatarHeight = avatarWidth * ace->bmHeight / ace->bmWidth;
			}

			AVATARDRAWREQUEST avdr = {0};
			avdr.cbSize = sizeof(avdr);
			avdr.hContact = NULL;
			avdr.hTargetDC = hdc;
			avdr.rcDraw = rc;
			avdr.rcDraw.bottom = avdr.rcDraw.top + avatarHeight;
			avdr.rcDraw.right = avdr.rcDraw.left + avatarWidth;
			avdr.dwFlags = AVDRQ_FALLBACKPROTO | AVDRQ_FORCEALPHA | AVDRQ_OWNPIC;
			avdr.szProto = "";

			avdr.alpha = 255;
			CallService(MS_AV_DRAWAVATAR, 0, (LPARAM)&avdr);

			rc.left += avatarWidth + 5;
		}
	}

	int nAccounts = 0;
	PROTOACCOUNT **accounts;
	ProtoEnumAccounts(&nAccounts, &accounts);
	for (int i = 0; i < nAccounts; ++i)
	{
		if (!accounts[i]->bIsEnabled /*|| !accounts[i]->ppro || !(accounts[i]->ppro->GetCaps(PFLAGNUM_1) & PF1_IM)*/)
			continue;

		char *proto = accounts[i]->szModuleName;

		if (true)
		{
			TCHAR name[128]; name[0] = 0;

			CONTACTINFO ci = {0};
			ci.cbSize = sizeof(ci);
			ci.hContact = NULL;
			ci.szProto = proto;
			ci.dwFlag = CNF_UNIQUEID | CNF_TCHAR;
			if (!CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM)&ci))
			{
				switch (ci.type)
				{
					case CNFT_ASCIIZ:
						mir_sntprintf(name, SIZEOF(name), _T("%s - %s"), accounts[i]->tszAccountName, ci.pszVal);
						mir_free((void *)ci.pszVal);
						break;
					case CNFT_DWORD:
						mir_sntprintf(name, SIZEOF(name), _T("%s - %u"), accounts[i]->tszAccountName, ci.dVal);
						break;
				}

			} else
			{
				lstrcpyn(name, accounts[i]->tszAccountName, SIZEOF(name));
			}

			RECT rcText = rc;
			rcText.left += 20;
			DrawText(hdc, name, -1, &rcText, DT_LEFT|DT_NOPREFIX|DT_WORDBREAK|DT_TOP|DT_SINGLELINE);
			GetTextExtentPoint32(hdc, name, lstrlen(name), &sz);
		}

		if (true)
		{
			HICON hIcon = LoadSkinnedProtoIcon(proto, CallProtoService(proto, PS_GETSTATUS, 0, 0));
			DrawIconEx(hdc, rc.left, rc.top + (sz.cy - 16) / 2, hIcon, 16, 16, 0, NULL, DI_NORMAL);
		}

		rc.top += sz.cy + 5;
	}

	SelectObject(hdc, hfntSave);

	DeleteDC(hdc);
	MakeBitmapOpaque(hbmp);
	SetThumbnail(hbmp);
	DeleteObject(hbmp);
}

void CClistProxyWindow::OnClose()
{
	CallService("CloseAction", NULL, NULL);
}

void CClistProxyWindow::OnTimer(int id)
{
	HANDLE hIcolibItem = m_overlayIconHandle;
	for (m_activeOverlay = (m_activeOverlay + 1) % (m_overlayEvents.getCount() + 1); m_activeOverlay < m_overlayEvents.getCount(); ++m_activeOverlay)
		if (m_overlayEvents[m_activeOverlay])
		{
			hIcolibItem = m_overlayEvents[m_activeOverlay].GetIcon();
			break;
		}

	HICON hIcon = hIcolibItem ? (HICON)CallService(MS_SKIN2_GETICONBYHANDLE, 0, (LPARAM)hIcolibItem) : m_overlayIcon;
	if (hIcon) g_pTaskbarList->SetOverlayIcon(hwnd(), hIcon, L"");
}
