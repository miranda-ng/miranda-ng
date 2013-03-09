/*
Miranda SmileyAdd Plugin
Copyright (C) 2006 - 2011 Boris Krasnovskiy

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

#ifndef anim_h
#define anim_h

class Animate
{
private:

	ImageBase  *m_img;
	SmileyType *m_sml;

	HDC			m_hdcMem;
	HBRUSH		m_hbr;

	RECT		m_cliprect;

	unsigned	m_nFramePosition;
	int			m_offset;
	int			m_counter;
	bool		m_running;
	bool		m_sel;
	bool		m_clip;

	void DrawFrame(HDC hdc);

public:

	Animate(SmileyType *sml, RECT &rect, HDC hdcMem, HBRUSH hbr, bool clip);
	Animate(const Animate& an); 
	~Animate();

	void Draw(HDC hdc);

	void StartAnimation(void);
	void SetOffset(int off, int wsize);
	void SetSel(int x, int y);

	void ProcessTimerTick(HWND hwnd);
};

class AnimatedPack
{
private:
	OBJLIST<Animate> m_AniList;

	HWND m_hwnd;
	int  m_wsize;

	HBRUSH	m_hbr;
	HBITMAP m_hBmp;
	HDC		m_hdcMem;
	HBITMAP	m_hOld;

	static unsigned CALLBACK AnimateThreadFunc ( void* arg );

public:
	AnimatedPack(HWND hwnd, int wsize, SIZE& sel, COLORREF bkg);
	~AnimatedPack();

	void Add(SmileyType *sml, RECT rect, bool clip);
	void Draw(HDC hdc);
	void SetOffset(int off);
	void SetSel(RECT& rect);

	void ProcessTimerTick(HWND hwnd);
};

#endif
