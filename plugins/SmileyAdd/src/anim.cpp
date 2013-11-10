/*
Miranda SmileyAdd Plugin
Copyright (C) 2006 - 2011 Boris Krasnovskiy All Rights Reserved

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "general.h"

Animate::Animate(SmileyType *sml, RECT& rect, HDC hdcMem, HBRUSH hbr, bool clip) :
	m_sml(sml),
	m_img(NULL),
	m_nFramePosition(0), m_sel(false), m_clip(clip),
	m_offset(0), m_running(false),
	m_hdcMem(hdcMem), m_hbr(hbr)
{
	m_cliprect = rect;
}


Animate::~Animate()
{
	if (m_img)
		m_img->Release();
}


void Animate::StartAnimation(void)
{
	m_img = m_sml->CreateCachedImage();

	if (m_img && m_img->IsAnimated()) {
		m_img->SelectFrame(m_nFramePosition);
		long frtm = m_img->GetFrameDelay();
		m_counter = frtm / 10 + ((frtm % 10) >= 5);
	}
}


void Animate::ProcessTimerTick(HWND hwnd)
{
	if (m_running && m_img->IsAnimated() && --m_counter <= 0) {
		m_nFramePosition = m_img->SelectNextFrame(m_nFramePosition);

		long frtm = m_img->GetFrameDelay();
		m_counter = frtm / 10 + ((frtm % 10) >= 5);

		HDC hdc = GetDC(hwnd);
		DrawFrame(hdc);
		ReleaseDC(hwnd, hdc);
	}
}


void Animate::DrawFrame(HDC hdc)
{
	long width  = m_cliprect.right  - m_cliprect.left;
	long height = m_cliprect.bottom - m_cliprect.top;

	RECT frc = { 0, 0, width, height };
	FillRect(m_hdcMem, &frc, m_hbr);

	m_img->Draw(m_hdcMem, frc, m_clip);

	BitBlt(hdc, m_cliprect.left, m_cliprect.top, width, height, m_hdcMem, 0, 0, SRCCOPY);

	if (m_sel)
		DrawFocusRect(hdc, &m_cliprect);
}


void Animate::Draw(HDC hdc) 
{ 
	if (m_running) {
		m_img->Draw(hdc, m_cliprect, m_clip);

		if (m_sel)
			DrawFocusRect(hdc, &m_cliprect);
	}
}


void Animate::SetOffset(int off, int wsize) 
{ 
	const int dy = m_offset - off;

	m_cliprect.top += dy;
	m_cliprect.bottom += dy;

	m_offset = off; 

	m_running = m_cliprect.top >= 0 && m_cliprect.top < wsize;
	if (m_running) {
		if (m_img == NULL) {
			StartAnimation();
			if (m_img == NULL)
				m_running = false;
		}
	}
	else {
		if (m_img) m_img->Release();
		m_img = NULL;
	}
}


void Animate::SetSel(int x, int y)
{
	m_sel = x >= m_cliprect.left && x < m_cliprect.right && y >= m_cliprect.top && y < m_cliprect.bottom;
}


AnimatedPack::AnimatedPack(HWND hwnd, int wsize, SIZE& sel, COLORREF bkg)
	: m_AniList(40), m_hwnd(hwnd), m_wsize(wsize) 
{
	HDC hdc = GetDC(hwnd);

	m_hBmp = CreateCompatibleBitmap(hdc, sel.cx, sel.cy);
	m_hdcMem = CreateCompatibleDC(hdc);
	m_hOld = (HBITMAP)SelectObject(m_hdcMem, m_hBmp);
	m_hbr = CreateSolidBrush(bkg);

	ReleaseDC(hwnd, hdc);
}


AnimatedPack::~AnimatedPack()
{
	DeleteObject(m_hbr);
	SelectObject(m_hdcMem, m_hOld);    
	DeleteObject(m_hBmp);	
	DeleteDC(m_hdcMem);
}


void AnimatedPack::Add(SmileyType *sml, RECT rect, bool clip)
{
	m_AniList.insert(new Animate(sml, rect, m_hdcMem, m_hbr, clip));
}


void AnimatedPack::Draw(HDC hdc)
{
	for (int i=0; i < m_AniList.getCount(); i++) 
		m_AniList[i].Draw(hdc);
}


void AnimatedPack::SetOffset(int off)
{
	for (int i=0; i < m_AniList.getCount(); i++) 
		m_AniList[i].SetOffset(off, m_wsize);
}


void AnimatedPack::SetSel(RECT& rect)
{
	for (int i=0; i < m_AniList.getCount(); i++) 
		m_AniList[i].SetSel(rect.left, rect.top);
}

void AnimatedPack::ProcessTimerTick(HWND hwnd)
{
	for (int i=0; i < m_AniList.getCount(); i++) 
		m_AniList[i].ProcessTimerTick(hwnd);
}
