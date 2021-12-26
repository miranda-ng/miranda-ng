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

#include "stdafx.h"

#include <stdio.h>
#include <fstream>
#include <sstream>

// PopupSkin
PopupSkin::PopupSkin(LPCTSTR aName)
{
	for (int i = 0; i < 32; i++)
		m_flag_names[i] = nullptr;
	m_elements = nullptr;
	m_name = aName ? mir_wstrdup(aName) : nullptr;
}

PopupSkin::~PopupSkin()
{
	if (m_name) mir_free(m_name);
	for (int i = 0; i < 32; i++)
		mir_free(m_flag_names[i]);
	freeSkin(m_elements);
}

SIZE PopupSkin::measureAction(HDC hdc, POPUPACTION *act) const
{
	SIZE sz = { 0 };
	if (!(PopupOptions.actions & ACT_ENABLE))
		return sz;

	if (PopupOptions.actions & ACT_LARGE)
		sz.cx = sz.cy = 32;
	else
		sz.cx = sz.cy = 16;

	if (PopupOptions.actions & ACT_TEXT) {
		char *name = strchr(act->lpzTitle, '/');
		if (!name) name = act->lpzTitle;
		else ++name;

		SIZE szText, szSpace;
		LPTSTR wname = mir_a2u(name);
		wchar_t *str = TranslateW(wname);
		GetTextExtentPoint32(hdc, str, (int)mir_wstrlen(str), &szText);
		mir_free(wname);
		GetTextExtentPoint32(hdc, L" ", 1, &szSpace);

		sz.cy = max(sz.cy, szText.cy);
		sz.cx += szSpace.cx;
		sz.cx += szText.cx;
		sz.cy += 2;
	}
	else {
		sz.cx += 4;
		sz.cy += 4;
	}

	return sz;
}

SIZE PopupSkin::measureActionBar(HDC hdc, PopupWnd2 *wnd) const
{
	SIZE sz = { 0 };
	HFONT hFntSave = (HFONT)SelectObject(hdc, fonts.action);
	for (int i = 0; i < wnd->getActionCount(); ++i) {
		SIZE szAction = measureAction(hdc, &wnd->getActions()[i].actionA);

		if (PopupOptions.actions & ACT_TEXT) {
			sz.cx = max(sz.cx, szAction.cx);
			sz.cy += szAction.cy;
		}
		else {
			sz.cx += szAction.cx;
			sz.cy = max(sz.cy, szAction.cy);
		}
	}
	SelectObject(hdc, hFntSave);
	return sz;
}

void PopupSkin::drawAction(MyBitmap *bmp, POPUPACTION *act, int x, int y, bool hover) const
{
	if (!(PopupOptions.actions & ACT_ENABLE))
		return;

	bmp->DrawIcon(act->lchIcon,
		(PopupOptions.actions & ACT_TEXT) ? x : (x + 2),
		y + 2,
		(PopupOptions.actions & ACT_LARGE) ? 32 : 16,
		(PopupOptions.actions & ACT_LARGE) ? 32 : 16);

	if (PopupOptions.actions & ACT_TEXT) {
		char *name = strchr(act->lpzTitle, '/');
		if (!name) name = act->lpzTitle;
		else ++name;

		SIZE szText, szSpace;

		HFONT hFntSave = (HFONT)SelectObject(bmp->getDC(), hover ? fonts.actionHover : fonts.action);
		SetTextColor(bmp->getDC(), hover ? fonts.clActionHover : fonts.clAction);
		SetBkMode(bmp->getDC(), TRANSPARENT);

		GetTextExtentPoint32(bmp->getDC(), L" ", 1, &szSpace);

		LPTSTR wname = mir_a2u(name);
		wchar_t *str = TranslateW(wname);
		GetTextExtentPoint32(bmp->getDC(), str, (int)mir_wstrlen(str), &szText);
		bmp->Draw_Text(str,
			(PopupOptions.actions & ACT_LARGE) ? (x + szSpace.cx + 32) : (x + szSpace.cx + 16),
			max(y + 2, y + 2 + (((PopupOptions.actions & ACT_LARGE) ? 32 : 16) - szText.cy) / 2));
		mir_free(wname);

		SelectObject(bmp->getDC(), hFntSave);
	}
	else {
		if (hover) {
			RECT rc;
			rc.left = x;
			rc.top = y;
			rc.right = x + ((PopupOptions.actions & ACT_LARGE) ? 32 : 16) + 4;
			rc.bottom = y + ((PopupOptions.actions & ACT_LARGE) ? 32 : 16) + 4;
			bmp->saveAlpha(rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top);
			HBRUSH hbr = CreateSolidBrush(fonts.clActionHover);
			FrameRect(bmp->getDC(), &rc, hbr);
			DeleteObject(hbr);
			bmp->restoreAlpha(rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top);
		}
	}
}

void PopupSkin::drawActionBar(MyBitmap *bmp, PopupWnd2 *wnd, int x, int y) const
{
	for (int i = 0; i < wnd->getActionCount(); ++i) {
		SIZE szAction = measureAction(bmp->getDC(), &wnd->getActions()[i].actionA);
		drawAction(bmp, &wnd->getActions()[i].actionA, x, y, wnd->getActions()[i].hover);

		wnd->getActions()[i].rc.left = x;
		wnd->getActions()[i].rc.top = y;
		wnd->getActions()[i].rc.right = x + szAction.cx;
		wnd->getActions()[i].rc.bottom = y + szAction.cy;

		if (PopupOptions.actions & ACT_TEXT)
			y += szAction.cy;
		else
			x += szAction.cx;
	}
}

void PopupSkin::measure(HDC hdc, PopupWnd2 *wnd, int maxw, POPUPOPTIONS *options) const
{
	if (!m_elements)
		return;

	SKINELEMENT *head = nullptr;

	int STYLE_SZ_CLOCK = 0;
	if (!this->useInternalClock())
		for (head = m_elements; head; head = head->next)
			if ((head->type & ST_TYPEMASK) == ST_CLOCK)
				break;

	if (head && head->myBmp) { // layerd clock
		SIZE szNew;
		szNew.cx = head->clocksize[CLOCK_LEFT] + head->clocksize[CLOCK_RIGHT];
		szNew.cy = head->myBmp->getHeight();
		for (wchar_t *p = wnd->getTime(); *p; p++) {
			if (*p == ':')
				szNew.cx += head->clocksize[CLOCK_SEPARATOR];
			else if ((*p >= '0') && (*p <= '9'))
				szNew.cx += head->clocksize[CLOCK_DIGITS + *p - '0'];
		}
		wnd->getArgs()->add("clock.width", szNew.cx);
		wnd->getArgs()->add("clock.height", szNew.cy);
		STYLE_SZ_CLOCK = szNew.cx;
	}
	else { // normal clock
		HFONT hfnSave = (HFONT)SelectObject(hdc, fonts.clock);
		SIZE sz; GetTextExtentPoint32(hdc, wnd->getTime(), (int)mir_wstrlen(wnd->getTime()), &sz);
		SelectObject(hdc, hfnSave);
		wnd->getArgs()->add("clock.width", sz.cx + 2 * STYLE_SZ_GAP);
		wnd->getArgs()->add("clock.height", sz.cy);
		STYLE_SZ_CLOCK = sz.cx + 2 * STYLE_SZ_GAP;
	}

	wnd->getArgs()->clear();
	wnd->getArgs()->add("options.avatarsize", PopupOptions.avatarSize);
	wnd->getArgs()->add("window.width", maxw);
	wnd->getArgs()->add("window.maxwidth", maxw);

	for (int i = 0; i < 32; ++i) {
		char buf[10];
		mir_snprintf(buf, "opt%d", i);
		wnd->getArgs()->add(buf, (m_flags&(1L << i)) ? 1 : 0);
	}

	SIZE sz = { 0, 0 };
	head = m_elements;
	while (head) {
		if ((m_flags & head->flag_mask) != (head->flags & head->flag_mask)) {
			head = head->next;
			continue;
		}

		SIZE szNew = { 0, 0 };
		switch (head->type & ST_TYPEMASK) {
		case ST_TEXT:
			{
				int tmp = head->fw.eval(wnd->getArgs());
				// this is used to measure and layout text
				wnd->getRenderInfo()->textw = tmp ? tmp : (maxw - head->fx.eval(wnd->getArgs()));
				szNew.cx = wnd->getRenderInfo()->textw;
				if (wnd->isTextEmpty())
					szNew.cx = szNew.cy = 0;
				else {
					HFONT hFntSave = (HFONT)SelectObject(hdc, fonts.text);
					switch (wnd->getTextType()) {
					case PopupWnd2::TT_UNICODE:
						{
							RECT rc; SetRect(&rc, 0, 0, szNew.cx, 0);
							DrawTextEx(hdc, wnd->getText(), (int)mir_wstrlen(wnd->getText()), &rc,
								DT_CALCRECT | DT_EXPANDTABS | DT_LEFT | DT_NOPREFIX | DT_TOP | DT_WORDBREAK/*|DT_RTLREADING*/, nullptr);
							szNew.cx = rc.right;
							szNew.cy = rc.bottom;
						}
						break;
					case PopupWnd2::TT_MTEXT:
						MTextMeasure(hdc, &szNew, wnd->getTextM());
						break;
					}
					SelectObject(hdc, hFntSave);
				}

				wnd->getRenderInfo()->texth = szNew.cy;

				SIZE szActions = measureActionBar(hdc, wnd);
				wnd->getRenderInfo()->actw = szActions.cx;
				if (szActions.cy) {
					szNew.cx = max(szNew.cx, szActions.cx);
					szNew.cy += szActions.cy;
					szNew.cy += 3;
				}

				wnd->getRenderInfo()->realtextw = szNew.cx;

				if (szNew.cx > maxw - head->fx.eval(wnd->getArgs()))
					szNew.cx = maxw - head->fx.eval(wnd->getArgs());
				wnd->getArgs()->add("text.width", szNew.cx);
				wnd->getArgs()->add("text.height", szNew.cy);
			}
			break;

		case ST_TITLE:
			{
				int tmp = head->fw.eval(wnd->getArgs());
				// this is used to measure and layout text
				wnd->getRenderInfo()->titlew = tmp ? tmp : (maxw - head->fx.eval(wnd->getArgs()) - STYLE_SZ_CLOCK);
				szNew.cx = wnd->getRenderInfo()->titlew;
				HFONT hFntSave = (HFONT)SelectObject(hdc, fonts.title);
				switch (wnd->getTextType()) {
				case PopupWnd2::TT_UNICODE:
					{
						RECT rc; SetRect(&rc, 0, 0, szNew.cx, 0);
						DrawTextEx(hdc, wnd->getTitle(), (int)mir_wstrlen(wnd->getTitle()), &rc,
							DT_CALCRECT | DT_EXPANDTABS | DT_LEFT | DT_NOPREFIX | DT_TOP | DT_WORDBREAK/*|DT_RTLREADING*/, nullptr);
						szNew.cx = rc.right;
						szNew.cy = rc.bottom;
					}
					break;
				case PopupWnd2::TT_MTEXT:
					MTextMeasure(hdc, &szNew, wnd->getTitleM());
					break;
				}

				SelectObject(hdc, hFntSave);
				if (szNew.cx > maxw - head->fx.eval(wnd->getArgs()) - STYLE_SZ_CLOCK)
					szNew.cx = maxw - head->fx.eval(wnd->getArgs()) - STYLE_SZ_CLOCK;
				szNew.cx += STYLE_SZ_CLOCK;
				wnd->getArgs()->add("title.width", szNew.cx);
				wnd->getArgs()->add("title.height", szNew.cy);
			}
			break;

		case ST_ICON:
			szNew.cx = szNew.cy = 16;
			wnd->getArgs()->add("icon.width", 16);
			wnd->getArgs()->add("icon.height", 16);
			break;

		case ST_BITMAP:
			GetBmpSize(head->hbm, &szNew);
			break;

		case ST_MYBITMAP:
			szNew.cx = head->myBmp->getWidth();
			szNew.cy = head->myBmp->getHeight();
			break;

		case ST_AVATAR:
			wnd->getRenderInfo()->hasAvatar = false;
			szNew.cx = szNew.cy = 0;
			if (wnd->getAvatar()) {
				szNew.cx = wnd->getAvatar()->getWidth();
				szNew.cy = wnd->getAvatar()->getHeight();
				wnd->getArgs()->add("avatarbitmap.width", max(1, szNew.cx));
				wnd->getArgs()->add("avatarbitmap.height", max(1, szNew.cy));
				wnd->getRenderInfo()->hasAvatar = true;
			}
			break;

		case ST_CLOCK:
			if (head && head->myBmp) {
				wchar_t *p = wnd->getTime();
				if (*p != 0) {
					szNew.cx = head->clocksize[CLOCK_LEFT] + head->clocksize[CLOCK_RIGHT];
					szNew.cy = head->myBmp->getHeight();
					while (*p) {
						if (*p == ':')
							szNew.cx += head->clocksize[CLOCK_SEPARATOR];
						else if ((*p >= '0') && (*p <= '9'))
							szNew.cx += head->clocksize[CLOCK_DIGITS + *p - '0'];
						p++;
					}
				}
				else szNew.cx = szNew.cy = 0;

				wnd->getArgs()->add("clock.width", szNew.cx);
				wnd->getArgs()->add("clock.height", szNew.cy);
			}
			else {
				HFONT hfnSave = (HFONT)SelectObject(hdc, fonts.clock);
				SIZE szClock;
				GetTextExtentPoint32(hdc, wnd->getTime(), (int)mir_wstrlen(wnd->getTime()), &szClock);
				SelectObject(hdc, hfnSave);
				wnd->getArgs()->add("clock.width", szClock.cx + 2 * STYLE_SZ_GAP);
				wnd->getArgs()->add("clock.height", szClock.cy);
			}
			break;
		}

		if (head->proportional && szNew.cx && szNew.cy && (((head->fw.eval(wnd->getArgs()) > 0) && !head->fh.eval(wnd->getArgs())) || (!head->fw.eval(wnd->getArgs()) && (head->fh.eval(wnd->getArgs()) > 0)))) {
			if (head->fw.eval(wnd->getArgs())) {
				szNew.cy = szNew.cy * head->fw.eval(wnd->getArgs()) / szNew.cx;
				szNew.cx = head->fw.eval(wnd->getArgs());
			}
			else {
				szNew.cx = szNew.cx * head->fh.eval(wnd->getArgs()) / szNew.cy;
				szNew.cy = head->fh.eval(wnd->getArgs());
			}
		}

		if ((head->type & ST_TYPEMASK) == ST_AVATAR) {
			wnd->getArgs()->add("avatar.width", szNew.cx);
			wnd->getArgs()->add("avatar.height", szNew.cy);

			szNew.cy += 2;
		}

		if (!head->fw.eval(wnd->getArgs()))
			szNew.cx += head->fx.eval(wnd->getArgs());
		else
			if (head->fw.eval(wnd->getArgs()) > 0)
				szNew.cx = head->fx.eval(wnd->getArgs()) + head->fw.eval(wnd->getArgs());
			else
				szNew.cx = 0;

		if (!head->fh.eval(wnd->getArgs()))
			szNew.cy += head->fy.eval(wnd->getArgs());
		else
			if (head->fh.eval(wnd->getArgs()) > 0)
				szNew.cy = head->fy.eval(wnd->getArgs()) + head->fh.eval(wnd->getArgs());
			else
				szNew.cy = 0;

		if (head->fx.eval(wnd->getArgs()) >= 0 && szNew.cx > sz.cx) sz.cx = szNew.cx;
		if (head->fy.eval(wnd->getArgs()) >= 0 && szNew.cy > sz.cy) sz.cy = szNew.cy;

		head = head->next;
	}

	sz.cx += m_right_gap;
	sz.cy += m_bottom_gap;

	int tmp;

	if (tmp = m_fw.eval(wnd->getArgs())) sz.cx = tmp;
	if (tmp = m_fh.eval(wnd->getArgs())) sz.cy = tmp;

	if (options->DynamicResize) {
		if (options->UseMinimumWidth && sz.cx < options->MinimumWidth)
			sz.cx = options->MinimumWidth;
	}
	else sz.cx = options->MaximumWidth;

	wnd->getArgs()->add("window.width", sz.cx);
	wnd->getArgs()->add("window.height", sz.cy);
	wnd->setSize(sz);
}

void PopupSkin::display(MyBitmap *bmp, PopupWnd2 *wnd, POPUPOPTIONS *options, uint32_t drawFlags) const
{
	if (!m_elements) return;

	HDC hdc = bmp->getDC();
	SKINELEMENT *head = nullptr;

	int STYLE_SZ_CLOCK = 0;
	if (!this->useInternalClock())
		for (head = m_elements; head; head = head->next)
			if ((head->type & ST_TYPEMASK) == ST_CLOCK)
				break;

	if (head && head->myBmp) {
		SIZE szNew;
		szNew.cx = head->clocksize[CLOCK_LEFT] + head->clocksize[CLOCK_RIGHT];
		szNew.cy = head->myBmp->getHeight();
		for (wchar_t *p = wnd->getTime(); *p; p++) {
			if (*p == ':')
				szNew.cx += head->clocksize[CLOCK_SEPARATOR];
			else if ((*p >= '0') && (*p <= '9'))
				szNew.cx += head->clocksize[CLOCK_DIGITS + *p - '0'];
		}
		STYLE_SZ_CLOCK = szNew.cx;
	}
	else {
		HFONT hfnSave = (HFONT)SelectObject(hdc, fonts.clock);
		SIZE sz; GetTextExtentPoint32(hdc, wnd->getTime(), (int)mir_wstrlen(wnd->getTime()), &sz);
		SelectObject(hdc, hfnSave);
		STYLE_SZ_CLOCK = sz.cx + 2 * STYLE_SZ_GAP;
	}

	head = m_elements;

	SIZE sz;
	SIZE szWnd = wnd->getSize();
	SIZE pos;
	bool actionsRendered = false;
	while (head) {
		if ((head->type & ST_TYPEMASK) == ST_AVATAR) {
			if (wnd->getAvatar()->isAnimated() && !(drawFlags & DF_ANIMATE)) {
				head = head->next;
				continue;
			}
			if (!wnd->getAvatar()->isAnimated() && !(drawFlags & DF_STATIC)) {
				head = head->next;
				continue;
			}
		}
		else if ((head->type & ST_TYPEMASK) == ST_TEXT) {
			if (!wnd->getActionCount() && !(drawFlags & DF_STATIC)) {
				head = head->next;
				continue;
			}
		}
		else if (!(drawFlags & DF_STATIC)) {
			head = head->next;
			continue;
		}

		if ((head->type & ST_BADPOS) || ((m_flags & head->flag_mask) != (head->flags & head->flag_mask))) {
			head = head->next;
			continue;
		}

		switch (head->type & ST_TYPEMASK) {
		case ST_TEXT:
			if (head->textColor != (COLORREF)0xffffffff)
				SetTextColor(hdc, head->textColor);
			else
				SetTextColor(hdc, wnd->getTextColor());
			{
				POINT pt;
				pt.x = head->fx.eval(wnd->getArgs());
				pt.y = head->fy.eval(wnd->getArgs());
				sz.cx = wnd->getRenderInfo()->textw;
				sz.cy = 1000;

				SetRect(&wnd->getRenderInfo()->textRect, pt.x, pt.y,
					pt.x + wnd->getRenderInfo()->realtextw,
					pt.y + wnd->getRenderInfo()->texth);

				if ((drawFlags&DF_STATIC) && !wnd->isTextEmpty()) {
					HFONT hFntSave = (HFONT)SelectObject(hdc, fonts.text);
					bmp->saveAlpha();
					switch (wnd->getTextType()) {
					case PopupWnd2::TT_UNICODE:
						{
							RECT rc; SetRect(&rc, pt.x, pt.y, pt.x + sz.cx, pt.y + sz.cy);
							DrawTextEx(hdc, wnd->getText(), (int)mir_wstrlen(wnd->getText()), &rc,
								DT_EXPANDTABS | DT_LEFT | DT_NOPREFIX | DT_TOP | DT_WORDBREAK/*|DT_RTLREADING*/, nullptr);
						}
						break;
					case PopupWnd2::TT_MTEXT:
						MTextDisplay(hdc, pt, sz, wnd->getTextM());
						break;
					}
					SelectObject(hdc, hFntSave);
					bmp->restoreAlpha();
				}

				if (!actionsRendered && (drawFlags&DF_ANIMATE)) {
					int textAreaWidth = head->fw.eval(wnd->getArgs());
					if (textAreaWidth <= 0) textAreaWidth = wnd->getRenderInfo()->realtextw;

					drawActionBar(bmp, wnd,
						g_plugin.getByte("CenterActions", 0) ?
						(pt.x + (textAreaWidth - wnd->getRenderInfo()->actw) / 2) :
						(PopupOptions.actions & ACT_RIGHTICONS) ?
						(pt.x + textAreaWidth - wnd->getRenderInfo()->actw) :
						// else
						pt.x,
						pt.y + wnd->getRenderInfo()->texth + 3);

					actionsRendered = true;
				}
			}
			break;

		case ST_TITLE:
			if (head->textColor != (COLORREF)0xffffffff)
				SetTextColor(hdc, head->textColor);
			else
				SetTextColor(hdc, wnd->getTitleColor());
			{
				bmp->saveAlpha();
				POINT pt;
				pt.x = head->fx.eval(wnd->getArgs());
				pt.y = head->fy.eval(wnd->getArgs());
				sz.cx = wnd->getRenderInfo()->titlew;
				sz.cy = 1000;

				switch (wnd->getTextType()) {
				case PopupWnd2::TT_UNICODE:
					{
						HFONT hFntSave = (HFONT)SelectObject(hdc, fonts.title);
						RECT rc; SetRect(&rc, pt.x, pt.y, pt.x + sz.cx, pt.y + sz.cy);
						DrawTextEx(hdc, wnd->getTitle(), (int)mir_wstrlen(wnd->getTitle()), &rc,
							DT_EXPANDTABS | DT_LEFT | DT_NOPREFIX | DT_TOP | DT_WORDBREAK/*|DT_RTLREADING*/, nullptr);
						SelectObject(hdc, hFntSave);
					}
					break;
				case PopupWnd2::TT_MTEXT:
					HFONT hFntSave = (HFONT)SelectObject(hdc, fonts.title);
					MTextDisplay(hdc, pt, sz, wnd->getTitleM());
					SelectObject(hdc, hFntSave);
					break;
				}

				bmp->restoreAlpha();
			}
			break;

		case ST_ICON:
			bmp->DrawIcon(wnd->getIcon(), head->fx.eval(wnd->getArgs()), head->fy.eval(wnd->getArgs()), 16, 16);
			break;

		case ST_MYBITMAP:
			sz.cx = head->myBmp->getWidth();
			sz.cy = head->myBmp->getHeight();

			if (head->proportional && sz.cx && sz.cy && (((head->fw.eval(wnd->getArgs()) > 0) && !head->fh.eval(wnd->getArgs())) || (!head->fw.eval(wnd->getArgs()) && (head->fh.eval(wnd->getArgs()) > 0)))) {
				if (head->fw.eval(wnd->getArgs())) {
					sz.cy = sz.cy * head->fw.eval(wnd->getArgs()) / sz.cx;
					sz.cx = head->fw.eval(wnd->getArgs());
				}
				else {
					sz.cx = sz.cx * head->fh.eval(wnd->getArgs()) / sz.cy;
					sz.cy = head->fh.eval(wnd->getArgs());
				}
			}
			else {
				if (head->fh.eval(wnd->getArgs()) > 0)
					sz.cy = head->fh.eval(wnd->getArgs());
				else if (head->fh.eval(wnd->getArgs()) < 0)
					sz.cy = szWnd.cy - head->fy.eval(wnd->getArgs()) + head->fh.eval(wnd->getArgs()) + 1;

				if (head->fw.eval(wnd->getArgs()) > 0)
					sz.cx = head->fw.eval(wnd->getArgs());
				else if (head->fw.eval(wnd->getArgs()) < 0)
					sz.cx = szWnd.cx - head->fx.eval(wnd->getArgs()) + head->fw.eval(wnd->getArgs()) + 1;
			}

			bool vars;
			pos.cx = ((head->fx.eval(wnd->getArgs(), &vars) < 0) && !vars) ? szWnd.cx + head->fx.eval(wnd->getArgs()) : head->fx.eval(wnd->getArgs());
			pos.cy = ((head->fy.eval(wnd->getArgs(), &vars) < 0) && !vars) ? szWnd.cy + head->fy.eval(wnd->getArgs()) : head->fy.eval(wnd->getArgs());

			if (head->type & ST_MONO) {
				COLORREF back = wnd->getBackColor();
				if (head->type & ST_BLEND)
					bmp->BlendColorized(head->myBmp, pos.cx, pos.cy, sz.cx, sz.cy, back);
				else
					bmp->DrawColorized(head->myBmp, pos.cx, pos.cy, sz.cx, sz.cy, back);
			}
			else {
				if (head->type & ST_BLEND)
					bmp->Blend(head->myBmp, pos.cx, pos.cy, sz.cx, sz.cy);
				else
					bmp->Draw(head->myBmp, pos.cx, pos.cy, sz.cx, sz.cy);
			}
			break;

		case ST_AVATAR:
			if (wnd->getAvatar()) {
				sz.cx = wnd->getAvatar()->getWidth();
				sz.cy = wnd->getAvatar()->getHeight();

				if (head->proportional && sz.cx && sz.cy && (((head->fw.eval(wnd->getArgs()) > 0) && !head->fh.eval(wnd->getArgs())) || (!head->fw.eval(wnd->getArgs()) && (head->fh.eval(wnd->getArgs()) > 0)))) {
					if (head->fw.eval(wnd->getArgs())) {
						sz.cy = sz.cy * head->fw.eval(wnd->getArgs()) / sz.cx;
						sz.cx = head->fw.eval(wnd->getArgs());
					}
					else {
						sz.cx = sz.cx * head->fh.eval(wnd->getArgs()) / sz.cy;
						sz.cy = head->fh.eval(wnd->getArgs());
					}
				}
				else {
					if (head->fh.eval(wnd->getArgs()) > 0)
						sz.cy = head->fh.eval(wnd->getArgs());
					else if (head->fh.eval(wnd->getArgs()) < 0)
						sz.cy = szWnd.cy - head->fy.eval(wnd->getArgs()) + head->fh.eval(wnd->getArgs());

					if (head->fw.eval(wnd->getArgs()) > 0)
						sz.cx = head->fw.eval(wnd->getArgs());
					else if (head->fw.eval(wnd->getArgs()) < 0)
						sz.cx = szWnd.cx - head->fx.eval(wnd->getArgs()) + head->fw.eval(wnd->getArgs());
				}

				pos.cx = head->fx.eval(wnd->getArgs()) < 0 ? szWnd.cx + head->fx.eval(wnd->getArgs()) - sz.cx : head->fx.eval(wnd->getArgs());
				pos.cy = head->fy.eval(wnd->getArgs()) < 0 ? szWnd.cy + head->fy.eval(wnd->getArgs()) - sz.cy : head->fy.eval(wnd->getArgs());

				wnd->getAvatar()->draw(bmp, pos.cx, pos.cy, sz.cx, sz.cy, options);
			}
			break;

		case ST_CLOCK:
			if (options->DisplayTime) {
				COLORREF back = wnd->getBackColor();
				int x = head->fx.eval(wnd->getArgs());
				int y = head->fy.eval(wnd->getArgs());
				if (head->myBmp) {
					int sy = head->myBmp->getHeight();
					if (head->type & ST_MONO)
						bmp->BlendPartColorized(head->myBmp, head->clockstart[CLOCK_LEFT], 0, head->clocksize[CLOCK_LEFT], sy, x, y, head->clocksize[CLOCK_LEFT], sy, back);
					else
						bmp->BlendPart(head->myBmp, head->clockstart[CLOCK_LEFT], 0, head->clocksize[CLOCK_LEFT], sy, x, y, head->clocksize[CLOCK_LEFT], sy);

					x += head->clocksize[CLOCK_LEFT];
					for (wchar_t *p = wnd->getTime(); *p; p++) {
						int clock_idx = -1;
						if (*p == ':')
							clock_idx = CLOCK_SEPARATOR;
						else if ((*p >= '0') && (*p <= '9'))
							clock_idx = CLOCK_DIGITS + *p - '0';

						if (clock_idx >= 0) {
							if (head->type & ST_MONO)
								bmp->BlendPartColorized(head->myBmp, head->clockstart[clock_idx], 0, head->clocksize[clock_idx], sy, x, y, head->clocksize[clock_idx], sy, back);
							else
								bmp->BlendPart(head->myBmp, head->clockstart[clock_idx], 0, head->clocksize[clock_idx], sy, x, y, head->clocksize[clock_idx], sy);
							x += head->clocksize[clock_idx];
						}
					}
					if (head->type & ST_MONO)
						bmp->BlendPartColorized(head->myBmp, head->clockstart[CLOCK_RIGHT], 0, head->clocksize[CLOCK_RIGHT], sy, x, y, head->clocksize[CLOCK_RIGHT], sy, back);
					else
						bmp->BlendPart(head->myBmp, head->clockstart[CLOCK_RIGHT], 0, head->clocksize[CLOCK_RIGHT], sy, x, y, head->clocksize[CLOCK_RIGHT], sy);
					x += head->clocksize[CLOCK_RIGHT];
				}
				else {
					if (head->textColor != (COLORREF)0xffffffff)
						SetTextColor(hdc, head->textColor);
					else
						SetTextColor(hdc, wnd->getClockColor());

					HFONT hfnSave = (HFONT)SelectObject(bmp->getDC(), fonts.clock);
					GetTextExtentPoint32(bmp->getDC(), wnd->getTime(), (int)mir_wstrlen(wnd->getTime()), &sz);
					bmp->Draw_Text(wnd->getTime(), x, y);
					SelectObject(bmp->getDC(), hfnSave);
				}
				break;
			}
		}
		head = head->next;
	}
}

bool PopupSkin::onMouseMove(PopupWnd2 *wnd, int x, int y) const
{
	POINT pt;
	pt.x = x;
	pt.y = y;

	bool res = false;
	bool hovered = false;

	for (int i = 0; i < wnd->getActionCount(); ++i) {
		bool hover = PtInRect(&wnd->getActions()[i].rc, pt) ? true : false;
		hovered |= hover;
		if (wnd->getActions()[i].hover != hover) {
			wnd->getActions()[i].hover = hover;
			res = true;
		}
	}

	SetCursor(LoadCursor(nullptr, hovered ? IDC_HAND : IDC_ARROW));

	return res;
}

bool PopupSkin::onMouseLeave(PopupWnd2 *wnd) const
{
	bool res = false;

	for (int i = 0; i < wnd->getActionCount(); ++i) {
		if (wnd->getActions()[i].hover) {
			wnd->getActions()[i].hover = false;
			res = true;
		}
	}

	SetCursor(LoadCursor(nullptr, IDC_ARROW));

	return res;
}

void PopupSkin::loadOptions(std::wistream &f)
{
	wchar_t *buf = new wchar_t[1024];
	while (!f.eof()) {
		f >> buf;
		if (*buf == '#') {
			f.ignore(1024, '\n');
			continue;
		}
		if (!mir_wstrcmp(buf, L"option")) {
			int id, val;
			f >> id >> val;
			f.getline(buf, 1024);
			id--;
			if (m_flag_names[id])
				mir_free(m_flag_names[id]);
			wchar_t *p = buf;
			while (isspace(*p))
				p++;
			wchar_t *q = p + mir_wstrlen(p) - 1;
			while ((q >= p) && isspace(*q))
				*q-- = 0;
			m_flag_names[id] = mir_u2a(p);
			if (val)
				m_flags |= 1 << id;
			else
				m_flags &= ~(1 << id);
		}
		else if (!mir_wstrcmp(buf, L"end"))
			break;
	}
	delete[] buf;
}

bool PopupSkin::load(LPCTSTR dir)
{
	for (int i = 0; i < 32; i++) {
		if (m_flag_names[i]) {
			mir_free(m_flag_names[i]);
			m_flag_names[i] = nullptr;
		}
	}
	m_flags = 0;

	if (!wcsncmp(L"res:", dir, 4)) // resource
		loadSkin(dir + 4, L"Skin");
	else { // filesystem
		// skin info
		wchar_t dir_save[1024];
		GetCurrentDirectory(1024, dir_save);
		SetCurrentDirectory(dir);

		std::wifstream theFile;
		theFile.open("popupskin.config", std::wios::in);
		if (theFile) {
			loadOptions(theFile);
			theFile.close();
		}

		WIN32_FIND_DATA ffd;
		HANDLE hFind = FindFirstFile(L"*.popupskin", &ffd);
		while (hFind != INVALID_HANDLE_VALUE) {
			loadSkin(ffd.cFileName);
			if (!FindNextFile(hFind, &ffd))
				break;
		}
		FindClose(hFind);

		SetCurrentDirectory(dir_save);
	}

	loadOpts();

	return true;
}

void PopupSkin::loadSkin(std::wistream &f)
{
	m_bottom_gap = m_right_gap = 0;
	m_legacy_region_opacity = 64;
	m_shadow_region_opacity = 1;
	m_popup_version = 0;
	m_internalClock = true;

	SKINELEMENT *head = new SKINELEMENT;
	m_elements = head;
	head->type = ST_NOTHING;
	head->next = nullptr;

	while (!f.eof()) {
		wchar_t buf[1024];
		f >> buf;

		if (!*buf)
			continue;

		if (*buf == '#' || *buf == ';') {
			f.ignore(1024, '\n');
			continue;
		}

		if (!mir_wstrcmp(buf, L"popup-version")) {
			f >> m_popup_version;
			m_popup_version = PLUGIN_MAKE_VERSION((m_popup_version / 1000000) % 100, (m_popup_version / 10000) % 100, (m_popup_version / 100) % 100, (m_popup_version / 1) % 100);
		}
		else if (!mir_wstrcmp(buf, L"padding-right")) {
			f >> m_right_gap;
		}
		else if (!mir_wstrcmp(buf, L"padding-bottom")) {
			f >> m_bottom_gap;
		}
		else if (!mir_wstrcmp(buf, L"shadow-region-opacity")) {
			f >> m_shadow_region_opacity;
		}
		else if (!mir_wstrcmp(buf, L"legacy-region-opacity")) {
			f >> m_legacy_region_opacity;
		}
		else if (!mir_wstrcmp(buf, L"w")) {
			f.getline(buf, 1024);
			m_fw.set(buf);
		}
		else if (!mir_wstrcmp(buf, L"h")) {
			f.getline(buf, 1024);
			m_fh.set(buf);
		}
		else if (!mir_wstrcmp(buf, L"object")) {
			head->next = loadObject(f);
			if (head->next && ((head->next->type & ST_TYPEMASK) == ST_CLOCK))
				m_internalClock = false;
			head = head->next;
			head->next = nullptr;
		}
		else if (!mir_wstrcmp(buf, L"options")) {
			loadOptions(f);
		}
	}

	head = m_elements;
	m_elements = m_elements->next;
	delete head;
}

void PopupSkin::loadSkin(LPCTSTR fn)
{
	if (!fn) return;

	std::wifstream theFile;
	theFile.open(fn, std::wios::in);

	if (!theFile) return;
	loadSkin(theFile);
	theFile.close();
}

void PopupSkin::loadSkin(LPCTSTR lpName, LPCTSTR lpType)
{
	HRSRC hRes = FindResource(g_plugin.getInst(), lpName, lpType);
	HRSRC hResLoad = (HRSRC)LoadResource(g_plugin.getInst(), hRes);
	char *lpResLock = (char *)LockResource(hResLoad);
	std::wistringstream stream((wchar_t*)_A2T(lpResLock));
	loadSkin(stream);
	UnlockResource(lpResLock);
	FreeResource(hRes);
}

PopupSkin::SKINELEMENT *PopupSkin::loadObject(std::wistream &f)
{
	SKINELEMENT *element = new SKINELEMENT;
	element->proportional = 0;
	element->type = ST_NOTHING | ST_BADPOS;
	element->next = nullptr;
	element->flag_mask = 0;
	element->flags = 0;
	element->myBmp = nullptr;

	while (!f.eof()) {
		wchar_t buf[1024];
		f >> buf;

		if (!*buf)
			continue;

		if (*buf == '#' || *buf == ';') {
			f.ignore(1024, '\n');
			continue;
		}

		if (!mir_wstrcmp(buf, L"type")) {
			f >> buf;
			if (!mir_wstrcmp(buf, L"icon"))
				element->type = (element->type & ~ST_TYPEMASK) | ST_ICON;
			else if (!mir_wstrcmp(buf, L"bitmap"))
				element->type = (element->type & ~ST_TYPEMASK) | ST_MYBITMAP;
			else if (!mir_wstrcmp(buf, L"text")) {
				element->type = (element->type & ~ST_TYPEMASK) | ST_TEXT;
				element->textColor = (COLORREF)0xffffffff;
				element->hfn = nullptr;
			}
			else if (!mir_wstrcmp(buf, L"title")) {
				element->type = (element->type & ~ST_TYPEMASK) | ST_TITLE;
				element->textColor = (COLORREF)0xffffffff;
				element->hfn = nullptr;
			}
			else if (!mir_wstrcmp(buf, L"avatar")) {
				element->type = (element->type & ~ST_TYPEMASK) | ST_AVATAR;
			}
			else if (!mir_wstrcmp(buf, L"clock")) {
				element->type = (element->type & ~ST_TYPEMASK) | ST_CLOCK;
				element->textColor = (COLORREF)0xffffffff;
				element->hfn = nullptr;
			}
		}
		else if (!mir_wstrcmp(buf, L"source")) {
			f >> buf;
			if (((element->type & ST_TYPEMASK) == ST_MYBITMAP) || ((element->type & ST_TYPEMASK) == ST_CLOCK))
				element->myBmp = new MyBitmap(buf);
		}
		else if (!mir_wstrcmp(buf, L"mono")) {
			element->type |= ST_MONO;
		}
		else if (!mir_wstrcmp(buf, L"layer")) {
			element->type |= ST_BLEND;
		}
		else if (!mir_wstrcmp(buf, L"proportional")) {
			f >> element->proportional;
		}
		else if (!mir_wstrcmp(buf, L"x")) {
			f.getline(buf, 1024);
			element->fx.set(buf);
			element->type &= ~ST_BADPOS;
		}
		else if (!mir_wstrcmp(buf, L"y")) {
			f.getline(buf, 1024);
			element->fy.set(buf);
			element->type &= ~ST_BADPOS;
		}
		else if (!mir_wstrcmp(buf, L"w")) {
			f.getline(buf, 1024);
			element->fw.set(buf);
		}
		else if (!mir_wstrcmp(buf, L"h")) {
			f.getline(buf, 1024);
			element->fh.set(buf);
		}
		else if (!mir_wstrcmp(buf, L"ifset")) {
			int id;
			f >> id; id--;
			element->flag_mask |= 1 << id;
			element->flags |= 1 << id;
		}
		else if (!mir_wstrcmp(buf, L"ifnotset")) {
			int id;
			f >> id; id--;
			element->flag_mask |= 1 << id;
			element->flags &= ~(1 << id);
		}
		else if (!mir_wstrcmp(buf, L"color")) {
			if (((element->type & ST_TYPEMASK) != ST_TEXT) &&
				((element->type & ST_TYPEMASK) != ST_TITLE) &&
				((element->type & ST_TYPEMASK) != ST_CLOCK)) continue;

			int r, g, b;
			f >> r >> g >> b;
			element->textColor = RGB(r, g, b);
		}
		else if (!mir_wstrcmp(buf, L"clocksize")) {
			element->clockstart[0] = 0;
			f >> element->clocksize[0];
			for (int i = 1; i < CLOCK_ITEMS; i++) {
				element->clockstart[i] = element->clockstart[i - 1] + element->clocksize[i - 1];
				f >> element->clocksize[i];
			}
		}
		else if (!mir_wstrcmp(buf, L"end")) {
			break;
		}
	}
	return element;
}

void PopupSkin::freeSkin(SKINELEMENT *head)
{
	while (head) {
		SKINELEMENT *next = head->next;

		if ((head->type & ST_TYPEMASK) == ST_BITMAP)
			DeleteObject(head->hbm);
		if ((head->type & ST_TYPEMASK) == ST_MYBITMAP)
			delete head->myBmp;
		if (((head->type & ST_TYPEMASK) == ST_CLOCK) && head->myBmp)
			delete head->myBmp;

		delete head;
		head = next;
	}
}

void PopupSkin::saveOpts() const
{
	char buf[128];
	mir_snprintf(buf, "skin.%.120S", m_name);
	g_plugin.setDword(buf, m_flags);
}

void PopupSkin::loadOpts() const
{
	char buf[128];
	mir_snprintf(buf, "skin.%.120S", m_name);
	m_flags = g_plugin.getDword(buf, m_flags);
}

// Skins
Skins skins;

Skins::Skins()
{
	m_skins = nullptr;
}

Skins::~Skins()
{
	while (m_skins) {
		SKINLIST *next = m_skins->next;
		delete m_skins->skin;
		delete[] m_skins->dir;
		mir_free(m_skins->name); // this is allocated with mir_strdup()
		delete m_skins;
		m_skins = next;
	}
}

bool Skins::load()
{
	while (m_skins) {
		SKINLIST *next = m_skins->next;
		mir_free(m_skins->name);
		delete[] m_skins->dir;
		delete m_skins->skin;
		delete m_skins;
		m_skins = next;
	}

	SKINLIST *skin = new SKINLIST;
	skin->next = m_skins;
	m_skins = skin;
	m_skins->name = mir_wstrdup(L"* Popup Classic");
	m_skins->dir = new wchar_t[1024];
	mir_wstrcpy(m_skins->dir, L"res:classic.popupskin");
	m_skins->skin = nullptr;

	wchar_t dir[1024] = { '\0' };

	if (ServiceExists(MS_FOLDERS_GET_PATH)) {
		if (FoldersGetCustomPathW(folderId, dir, _countof(dir), nullptr) != 0)
			return false;
	}
	else {
		mir_wstrncpy(dir, VARSW(L"%miranda_path%\\skins\\popup"), _countof(dir));
		uint32_t fa = GetFileAttributes(dir);
		if ((fa == INVALID_FILE_ATTRIBUTES) || !(fa&FILE_ATTRIBUTE_DIRECTORY))
			return false;
	}

	wchar_t dir_save[1024];
	GetCurrentDirectory(1024, dir_save);
	SetCurrentDirectory(dir);

	WIN32_FIND_DATA ffd;
	HANDLE hFind = FindFirstFile(L"*.*", &ffd);
	while (hFind != INVALID_HANDLE_VALUE) {
		if ((ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && mir_wstrcmp(L".", ffd.cFileName) && mir_wstrcmp(L"..", ffd.cFileName)) {
			SetCurrentDirectory(ffd.cFileName);

			skin = new SKINLIST;
			skin->next = m_skins;
			m_skins = skin;
			m_skins->name = mir_wstrdup(ffd.cFileName);
			m_skins->dir = new wchar_t[1024];
			GetCurrentDirectory(1024, m_skins->dir);
			m_skins->skin = nullptr;

			SetCurrentDirectory(L"..");
		}

		if (!FindNextFile(hFind, &ffd))
			break;
	}
	FindClose(hFind);

	SetCurrentDirectory(dir_save);

	return true;
}

const PopupSkin *Skins::getSkin(LPCTSTR name)
{
	SKINLIST *any = nullptr;
	for (SKINLIST *p = m_skins; p; p = p->next) {
		if (!mir_wstrcmp(p->name, L"* Popup Classic") || !any)
			any = p;
		if (!mir_wstrcmpi(p->name, name)) {
			any = p;
			break;
		}
	}

	if (any && any->skin)
		return any->skin;
	if (!any)
		return nullptr;

	any->skin = new PopupSkin(any->name);
	any->skin->load(any->dir);
	return any->skin;
}

void Skins::loadActiveSkin()
{
	for (SKINLIST *p = m_skins; p; p = p->next)
		if (!mir_wstrcmpi(p->name, PopupOptions.SkinPack)) {
			if (p->skin)
				break;

			p->skin = new PopupSkin(p->name);
			p->skin->load(p->dir);
			break;
		}
}

void Skins::freeAllButActive()
{
	for (SKINLIST *p = m_skins; p; p = p->next)
		if (mir_wstrcmpi(p->name, PopupOptions.SkinPack)) {
			delete p->skin;
			p->skin = nullptr;
		}
}
