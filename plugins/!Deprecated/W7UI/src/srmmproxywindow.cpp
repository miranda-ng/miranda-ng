#include "headers.h"

extern int g_eventSlotMessage;
extern ITaskbarList3 *g_pTaskbarList;

///////////////////////////////////////////////////////////////////////////////
// srmm processing

CSrmmProxyWindow::CSrmmProxyWindow(HANDLE hContact, HWND hwndWindow, HWND hwndParent)
{
	m_hContact = hContact;
	m_hwndWindow = hwndWindow;
	m_hwndParent = hwndParent;

	m_hbmpPreview = NULL;
	m_refreshPreview = true;

	m_bActive = false;
	m_bUnread = false;
	m_bTyping = false;

	UpdateIcon();
	SetWindowText(hwnd(), (TCHAR *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)m_hContact, GCDNF_TCHAR));
	SetTimer(1, 1000);

	SetEventHook(ME_DB_EVENT_ADDED, &CSrmmProxyWindow::OnDbEventAdded);
	SetEventHook(ME_DB_CONTACT_SETTINGCHANGED, &CSrmmProxyWindow::OnDbSettingChanged);
	SetEventHook(ME_AV_AVATARCHANGED, &CSrmmProxyWindow::OnAvatarChanged);
	SetEventHook(ME_PROTO_CONTACTISTYPING, &CSrmmProxyWindow::OnContactTyping);

	AddButton(LoadSkinnedIcon(SKINICON_OTHER_USERDETAILS), TranslateT("User Details"), 1, THBF_DISMISSONCLICK);
	AddButton(LoadSkinnedIcon(SKINICON_OTHER_HISTORY), TranslateT("History"), 2, THBF_DISMISSONCLICK);
	AddButton(LoadSkinnedIcon(SKINICON_EVENT_FILE), TranslateT("File"), 3, THBF_DISMISSONCLICK);
	UpdateButtons(g_pTaskbarList);
}

CSrmmProxyWindow::~CSrmmProxyWindow()
{
	DeleteObject(m_hbmpPreview);
	KillObjectEventHooks(this);
}


void CSrmmProxyWindow::Refresh()
{
	InvalidateThumbnail();
	m_refreshPreview = true;
}

int __cdecl CSrmmProxyWindow::OnDbEventAdded(WPARAM wParam, LPARAM lParam)
{
	if ((HANDLE)wParam == m_hContact)
	{
		Refresh();

		if (!m_bUnread && !IsActive())
		{
			DBEVENTINFO dbei = {0};
			dbei.cbSize = sizeof(dbei);
			if (!CallService(MS_DB_EVENT_GET, (WPARAM)lParam, (LPARAM)&dbei))
				if (dbei.eventType == EVENTTYPE_MESSAGE && !(dbei.flags & (DBEF_READ|DBEF_SENT)))
				{
					g_clistProxyWnd->Flash();
					g_clistProxyWnd->AddOverlayEvent(g_eventSlotMessage);
					m_bUnread = true;
					UpdateIcon();
				}
		}
	}
	return 0;
}

int __cdecl CSrmmProxyWindow::OnDbSettingChanged(WPARAM wParam, LPARAM lParam)
{
	if ((HANDLE)wParam == m_hContact)
	{
		UpdateIcon();
		Refresh();
	}
	return 0;
}

int __cdecl CSrmmProxyWindow::OnAvatarChanged(WPARAM wParam, LPARAM lParam)
{
	if ((HANDLE)wParam == m_hContact)
		Refresh();
	return 0;
}

int __cdecl CSrmmProxyWindow::OnContactTyping(WPARAM wParam, LPARAM lParam)
{
	if ((HANDLE)wParam == m_hContact)
	{
		m_bTyping = lParam ? true : false;
		UpdateIcon();
	}
	return 0;
}

HANDLE CSrmmProxyWindow::SetEventHook(char *evt, int (__cdecl CSrmmProxyWindow::*fn)(WPARAM, LPARAM))
{
	return HookEventObj(evt, *(MIRANDAHOOKOBJ *)&fn, this);
}

void CSrmmProxyWindow::OnTabActive()
{
	m_bActive = true;
	if (m_bUnread)
	{
		g_clistProxyWnd->RemoveOverlayEvent(g_eventSlotMessage);
		m_bUnread = false;
		UpdateIcon();
	}
}

void CSrmmProxyWindow::OnTabInactive()
{
	m_bActive = false;
}

bool CSrmmProxyWindow::IsActive()
{
	for (HWND hwnd = GetFocus(); hwnd; hwnd = GetParent(hwnd))
		if (hwnd == m_hwndWindow)
			return m_bActive = true;
	return m_bActive = false;
}

void CSrmmProxyWindow::UpdateIcon()
{
	HICON hIcon;
	if (m_bTyping)
	{
		hIcon = LoadSkinnedIcon(SKINICON_OTHER_TYPING);
	} else
	if (m_bUnread)
	{
		hIcon = LoadSkinnedIcon(SKINICON_EVENT_MESSAGE);
	} else
	{
		char *szProto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)m_hContact, 0);
		if (!szProto) return;
		int iStatus = DBGetContactSettingWord(m_hContact, szProto, "Status", ID_STATUS_OFFLINE);
		hIcon = (HICON)LoadSkinnedProtoIcon(szProto, iStatus);
	}

	SendMessage(hwnd(), WM_SETICON, ICON_BIG, (LPARAM)hIcon);
	SendMessage(hwnd(), WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
}

void CSrmmProxyWindow::OnActivate(HWND hwndFrom)
{
	CallService(MS_CLIST_CONTACTDOUBLECLICKED, (WPARAM)m_hContact, 0);
}

void CSrmmProxyWindow::OnToolbar(int id, INT_PTR data)
{
	POINT pt; GetCursorPos(&pt);
	HMENU hMenu = NULL;
	HWND hwndClui = (HWND)CallService(MS_CLUI_GETHWND, 0, 0);

	switch (data)
	{
		case 1:
			CallService(MS_USERINFO_SHOWDIALOG, (WPARAM)m_hContact, 0);
			break;

		case 2:
			CallService(MS_HISTORY_SHOWCONTACTHISTORY, (WPARAM)m_hContact, 0);
			break;

		case 3:
			CallService(MS_FILE_SENDFILE, (WPARAM)m_hContact, 0);
			break;
	}
}

void CSrmmProxyWindow::OnRenderThumbnail(int width, int height)
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
		AVATARCACHEENTRY *ace = (AVATARCACHEENTRY *)CallService(MS_AV_GETAVATARBITMAP, (WPARAM)m_hContact, 0);
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
			avdr.hContact = m_hContact;
			avdr.hTargetDC = hdc;
			avdr.rcDraw = rc;
			avdr.rcDraw.bottom = avdr.rcDraw.top + avatarHeight;
			avdr.rcDraw.right = avdr.rcDraw.left + avatarWidth;
			avdr.dwFlags = AVDRQ_FALLBACKPROTO | AVDRQ_FORCEALPHA;

			avdr.alpha = 255;
			CallService(MS_AV_DRAWAVATAR, 0, (LPARAM)&avdr);

			rc.left += avatarWidth + 5;
		}
	}

	char *proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)m_hContact, 0);
	
	if (true)
	{
		CONTACTINFO ci = {0};
		ci.cbSize = sizeof(ci);
		ci.hContact = m_hContact;
		ci.szProto = proto;
		ci.dwFlag = CNF_UNIQUEID | CNF_TCHAR;
		if (!CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM) & ci))
		{
			TCHAR name[128]; name[0] = 0;
			switch (ci.type)
			{
				case CNFT_ASCIIZ:
					mir_sntprintf(name, SIZEOF(name), _T("%s"), ci.pszVal);
					mir_free((void *)ci.pszVal);
					break;
				case CNFT_DWORD:
					mir_sntprintf(name, SIZEOF(name), _T("%u"), ci.dVal);
					break;
			}

			TextOut(hdc, rc.left + 20, rc.top, name, lstrlen(name));
			GetTextExtentPoint32(hdc, name, lstrlen(name), &sz);
		}
	}

	if (true)
	{
		HIMAGELIST hIml = (HIMAGELIST)CallService(MS_CLIST_GETICONSIMAGELIST, 0, 0);
		int iIcon = CallService(MS_CLIST_GETCONTACTICON, (WPARAM)m_hContact, 0);
		ImageList_Draw(hIml, iIcon, hdc, rc.left, rc.top + (sz.cy - 16) / 2, ILD_TRANSPARENT);
	}

	rc.top += sz.cy + 5;

	rc.left += 10;

	struct
	{
		TCHAR *text;
		bool out;
	} msgs[10] = {0};

	if (true)
	{
		int hMsgs = 0;
		int n = 0;
		HANDLE hEvent = (HANDLE)CallService(MS_DB_EVENT_FINDLAST, (WPARAM)m_hContact, 0);
		while (hEvent)
		{
			BYTE buf[1024];
			DBEVENTINFO dbei = {0};
			dbei.cbSize = sizeof(dbei);
			dbei.cbBlob = sizeof(buf);
			dbei.pBlob = buf;
			if (!CallService(MS_DB_EVENT_GET, (WPARAM)hEvent, (LPARAM)&dbei))
			{
				if (dbei.eventType == EVENTTYPE_MESSAGE)
				{
					msgs[n].text = DbGetEventTextT(&dbei, CP_ACP);
					msgs[n].out = dbei.flags & DBEF_SENT ? true : false;

					RECT rcCopy = rc;
					hMsgs += DrawText(hdc, msgs[n].text, -1, &rcCopy, DT_LEFT|DT_NOPREFIX|DT_WORDBREAK|DT_TOP|DT_CALCRECT);
					if (n && hMsgs > rc.bottom - rc.top)
					{
						mir_free(msgs[n].text);
						msgs[n].text = 0;
						break;
					}

					hMsgs += 3;

					if (++n >= SIZEOF(msgs)) break;
				}
			}
			hEvent = (HANDLE)CallService(MS_DB_EVENT_FINDPREV, (WPARAM)hEvent, 0);
		}
	}

	if (true)
	{
		for (int i = SIZEOF(msgs); i--; )
		{
			if (!msgs[i].text) continue;

			TCHAR szDir[] = { (msgs[i].out ? (WCHAR)0xbb : (WCHAR)0xab), 0 };
			rc.left -= 10;
			DrawText(hdc, szDir, -1, &rc, DT_LEFT|DT_NOPREFIX|DT_WORDBREAK|DT_TOP);
			rc.left += 10;

			rc.top += 3 + DrawText(hdc, msgs[i].text, -1, &rc, DT_LEFT|DT_NOPREFIX|DT_WORDBREAK|DT_TOP);
			mir_free(msgs[i].text);
		}
	}

	SelectObject(hdc, hfntSave);

	DeleteDC(hdc);
	MakeBitmapOpaque(hbmp);
	SetThumbnail(hbmp);
	DeleteObject(hbmp);
}

void CSrmmProxyWindow::OnRenderPreview()
{
	if (!m_hbmpPreview) return;

	RECT rc;
	GetWindowRect(m_hwndWindow, &rc);
	MapWindowPoints(NULL, m_hwndParent, (POINT *)&rc, 2);
	SetPreview(m_hbmpPreview, rc.left, rc.top);
}

void CSrmmProxyWindow::OnTimer(int)
{
	g_pTaskbarList->UnregisterTab(m_hwndParent);

	if (!m_refreshPreview) return;
	if (!IsWindowVisible(m_hwndWindow) || !IsWindowVisible(m_hwndParent) || IsIconic(m_hwndParent)) return;
	if (m_hbmpPreview) DeleteObject(m_hbmpPreview);

	m_refreshPreview = false;

	RECT rc;
	GetWindowRect(m_hwndWindow, &rc);

	m_hbmpPreview = CreateDwmBitmap(rc.right - rc.left, rc.bottom - rc.top);
	HDC hdc = CreateCompatibleDC(0);
	HBITMAP hbmpSave = (HBITMAP)SelectObject(hdc, m_hbmpPreview);
	PrintWindow(m_hwndWindow, hdc, PW_CLIENTONLY);
	SelectObject(hdc, hbmpSave);
	DeleteDC(hdc);

	MakeBitmapOpaque(m_hbmpPreview);

	InvalidateThumbnail();
}

void CSrmmProxyWindow::OnClose()
{	
	SendMessage(m_hwndWindow, WM_CLOSE, 1, 0);
}