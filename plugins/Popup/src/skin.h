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

#ifndef __skin_h__
#define __skin_h__

#include <stdio.h>
#include <fstream>

class MyBitmap;
class PopupWnd2;

class PopupSkin
{
public:
	enum
	{
		ST_TYPEMASK = 0x07,
		ST_NOTHING	= 0x00,
		ST_ICON		= 0x01,
		ST_TEXT		= 0x02,
		ST_TITLE	= 0x03,
		ST_BITMAP	= 0x04,
		ST_MYBITMAP = 0x05,
		ST_AVATAR   = 0x06,
		ST_CLOCK    = 0x07,

		ST_STRETCH	= 0x08,
		ST_MONO		= 0x10,
		ST_BLEND	= 0x20,
		ST_BADPOS	= 0x40
	};

	enum
	{
		DF_STATIC	= 0x01,
		DF_ANIMATE	= 0x02,
		DF_ALL		= 0xff
	};

	enum
	{
		 // left, separator, digits, am/pm, right
		CLOCK_LEFT = 0,
		CLOCK_SEPARATOR = 1,
		CLOCK_DIGITS = 2,
		CLOCK_AM = 12,
		CLOCK_PM = 13,
		CLOCK_RIGHT = 14,
		CLOCK_ITEMS = 1+1+10+2+1
	};

	struct	SKINELEMENT
	{
		int type;
		union
		{
			HICON hic;
			HBITMAP hbm;
			MyBitmap *myBmp;
			HFONT hfn;
		};
		unsigned long flags;
		unsigned long flag_mask;
		Formula fx, fy, fw, fh;
		int clocksize[CLOCK_ITEMS];
		int clockstart[CLOCK_ITEMS];
		int proportional;
		COLORREF textColor;
		SKINELEMENT *next;
	};	

	struct	RenderInfo
	{
		bool hasAvatar;
		int titlew, textw;
		int realtextw, texth;
		int actw;
		RECT textRect;
	};

private:
	LPTSTR	m_name;
	int		m_bottom_gap, m_right_gap;
	int		m_legacy_region_opacity, m_shadow_region_opacity;
	int		m_popup_version;
	bool	m_internalClock;
	Formula					m_fw, m_fh;
	SKINELEMENT				*m_elements;
	char					*m_flag_names[32];
	mutable unsigned long	m_flags;

	void	loadOptions(std::wistream &f);
	SKINELEMENT	*loadObject(std::wistream &f);
	void	loadSkin(std::wistream &f);
	void	loadSkin(LPCTSTR fn);
	void	loadSkin(LPCTSTR lpName, LPCTSTR lpType);

	void	freeSkin(SKINELEMENT *head);

	SIZE	measureAction(HDC hdc, POPUPACTION *act) const;
	SIZE	measureActionBar(HDC hdc, PopupWnd2 *wnd) const;
	void	drawAction(MyBitmap *bmp, POPUPACTION *act, int x, int y, bool hover) const;
	void	drawActionBar(MyBitmap *bmp, PopupWnd2 *wnd, int x, int y) const;

public:
	PopupSkin(LPCTSTR aName = 0);
	~PopupSkin();

	void	measure(HDC hdc, PopupWnd2 *wnd, int maxw, POPUPOPTIONS *options) const;
	void	display(MyBitmap *bmp, PopupWnd2 *wnd, int maxw, POPUPOPTIONS *options, DWORD drawFlags=DF_ALL) const;
	bool	onMouseMove(PopupWnd2 *wnd, int x, int y) const;
	bool	onMouseLeave(PopupWnd2 *wnd) const;

	bool	load(LPCTSTR dir); // load skin from current directory

	SKINELEMENT		*getSubSkin()						{ return m_elements; }
	int				getBottomGap()				const	{ return m_bottom_gap; }
	int				getRightGap()				const	{ return m_right_gap; }
	int				useInternalClock()			const	{ return m_internalClock; }
	int				getLegacyRegionOpacity()	const	{ return m_legacy_region_opacity; }
	int				getShadowRegionOpacity()	const	{ return m_shadow_region_opacity; }
	bool			isCompatible()				const	{ return (DWORD) m_popup_version <= (DWORD) pluginInfoEx.version; }

	const LPTSTR	getName()					const	{ return m_name; }

	const char		*getFlagName(int id)		const	{ return m_flag_names[id-1]; }
	bool			getFlag(int id)				const	{ return (m_flags & (1 << (id-1))) != 0; }
	void			setFlag(int id, bool val)	const
	{
		if (val)
			m_flags |= 1 << (id-1);
		else
			m_flags &= ~(1 << (id-1));
	}

	void			saveOpts()					const;
	void			loadOpts()					const;
};

class Skins
{
public:
	struct		SKINLIST
	{
		PopupSkin	*skin;
		LPTSTR		dir;
		LPTSTR		name;
		SKINLIST *next;
	};

private:
	SKINLIST	*m_skins;

public:
	Skins();
	~Skins();

	bool			load(LPCTSTR dir);
	const PopupSkin	*getSkin(LPCTSTR name);

	const SKINLIST	*getSkinList()				const	{ return m_skins; }

	void			loadActiveSkin();
	void			freeAllButActive();
};

extern Skins skins;

#endif
