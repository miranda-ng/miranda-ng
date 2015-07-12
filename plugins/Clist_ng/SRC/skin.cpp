/*
 * astyle --force-indent=tab=4 --brackets=linux --indent-switches
 *		  --pad=oper --one-line=keep-blocks  --unpad=paren
 *
 * Miranda IM: the free IM client for Microsoft* Windows*
 *
 * Copyright 2000-2010 Miranda ICQ/IM project,
 * all portions of this codebase are copyrighted to the people
 * listed in contributors.txt.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * you should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * part of clist_ng plugin for Miranda.
 *
 * (C) 2005-2010 by silvercircle _at_ gmail _dot_ com and contributors
 *
 * $Id: skin.cpp 138 2010-11-01 10:51:15Z silvercircle $
 *
 * implement skin management, loading, applying etc.
 *
 * about database values:
 *
 * various skin-related settings may be written to the databse. the module
 * name is CLNgSkin and the value names are the same as variable names in the
 * TSkinSettings and TSkinMetrics data structures with the following
 * prefixes:
 *
 * s - a value belonging to TSkinSettings
 * m - a value belonging to TSkinMetrics
 * g - a global value belonging to the Skin class - e.gl gCurrentSkin
 * holds the name of the currently active skin.
 *
 */


#include <commonheaders.h>

TStatusItem*	Skin::statusItems = 0;
TImageItem*		Skin::imageItems = 0;
TImageItem*		Skin::glyphItem = 0;
TSkinMetrics	Skin::metrics = {0};
TSkinSettings   Skin::settings = {0};

int 			Skin::ID_EXTBK_LAST = ID_EXTBK_LAST_D;

UINT 			SkinLoader::nextButtonID = IDC_TBFIRSTUID;



/*
 * TODO Refactor
 */
extern struct 	CluiTopButton top_buttons[];
extern TStatusItem DefaultStatusItems[ID_EXTBK_LAST_D + 1];

HBRUSH g_CLUISkinnedBkColor = 0;
COLORREF g_CLUISkinnedBkColorRGB = 0;


void Skin::setAeroMargins()
{
	if(cfg::isAero) {
		MARGINS m = {-1};
		Api::pfnDwmExtendFrameIntoClientArea(pcli->hwndContactList, &m);
	}
	else {
		MARGINS m = {0};
		Api::pfnDwmExtendFrameIntoClientArea(pcli->hwndContactList, &m);
	}
}

void Skin::updateAeroState()
{
	cfg::isAero = (settings.fHaveAeroBG && settings.fUseAero && Api::sysState.isAero) ? true : false;
}

void Skin::Activate()
{
	CLUI::applyBorderStyle();
	CLUI::configureGeometry(1);

	ShowWindow(pcli->hwndContactList, SW_SHOWNORMAL);
    SetWindowPos(pcli->hwndContactList, 0, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
    SendMessageW(pcli->hwndContactList, WM_SIZE, 0, 0);
    updateAeroState();
    setAeroMargins();
	CLUI::Redraw();
}

/**
 * free a single image item, release DC, delete bitmap
 * @param item		TImageItem* the item to free
 */
void Skin::freeImageItem(TImageItem *item)
{
    if(item) {
    	if(!(item->dwFlags & IMAGE_GLYPH)) {
    		SelectObject(item->hdc, item->hbmOld);
    		DeleteObject(item->hbm);
    		DeleteDC(item->hdc);
    	}
    	if(item->fillBrush)
    		DeleteObject(item->fillBrush);
    }
}

void Skin::colorizeGlyphByName(const char* szGlyphName, const COLORREF clr, float h, float s, float v)
{
	if(szGlyphName) {
		TImageItem *_t = imageItems;
		while(_t) {
			if(!strcmp(_t->szName, szGlyphName)) {
				Gfx::colorizeGlyph(_t, clr, h, s, v);
				break;
			}
			_t = _t->nextItem;
		}
	}
}

/**
 * render a image item with the given name to the target rectangle in the target
 * device context.
 *
 * @param szItemname	image item name (must include the preceeding $ or @)
 * @param rc			target rectangle
 * @param hdc			target dc
 */
void Skin::renderNamedImageItem(const char *szItemname, RECT* rc, const HDC hdc)
{
	if(szItemname) {
		TImageItem *_t = imageItems;
		while(_t) {
			if(_t->szName && !strcmp(_t->szName, szItemname)) {
				Gfx::renderImageItem(hdc, _t, rc);
				break;
			}
			_t = _t->nextItem;
		}
	}
}
/**
 * free all skin items
 * a) image items
 * b) global image items (glyph, CLUI background)
 * c) button items
 */
void Skin::Unload()
{
    TImageItem *pItem = imageItems, *pNextItem;
    TButtonItem *pbItem = CLUI::buttonItems, *pbNextItem;

	while(pItem) {
        freeImageItem(pItem);
        pNextItem = pItem->nextItem;
        free(pItem);
        pItem = pNextItem;
    }
	imageItems = NULL;
    while(pbItem) {
        DestroyWindow(pbItem->hWnd);
        pbNextItem = pbItem->nextItem;
        free(pbItem);
        pbItem = pbNextItem;
    }
    CLUI::buttonItems = 0;

    if(CLUI::bgImageItem) {
        freeImageItem(CLUI::bgImageItem);
        free(CLUI::bgImageItem);
    }
    CLUI::bgImageItem = 0;

    if(CLUI::bgImageItem_nonAero) {
        freeImageItem(CLUI::bgImageItem_nonAero);
        free(CLUI::bgImageItem_nonAero);
    }
    CLUI::bgImageItem_nonAero = 0;

    if(CLUI::bgClientItem) {
        freeImageItem(CLUI::bgClientItem);
        free(CLUI::bgClientItem);
    }
    CLUI::bgClientItem = 0;

    if(glyphItem) {
        freeImageItem(glyphItem);
        free(glyphItem);
    }
    glyphItem = NULL;

    if(statusItems) {
    	TStatusItem* 	item;
    	for(int i = 0; i <= ID_EXTBK_LAST; i++) {
    		item = &statusItems[i];

    		if(item->span_allocator) {
				delete item->gradient_renderer_x;
				delete item->gradient_renderer_y;
				delete item->solid_renderer;
				delete item->span_gradient_x;
				delete item->span_gradient_y;
				delete item->gradient_trans;
				delete item->gradient_func_x;
				delete item->gradient_func_y;
				delete item->color_array;
				delete item->span_interpolator;
				delete item->span_allocator;
				delete item->pixfmt;
				delete item->rbase;
				delete item->rect;
    		}
    	}
    	free(statusItems);
    	statusItems = 0;
    	ID_EXTBK_LAST = ID_EXTBK_LAST_D;
    }
}

/**
 * setup the items AGG rendering pipeline - set color values for the gradient
 * and the rectangle shape (corners)
 *
 * @param item		TStatusItem* - a skin item
 */
void Skin::setupAGGItemContext(TStatusItem* item)
{
	BYTE	r = GetRValue(item->COLOR);
	BYTE	g = GetGValue(item->COLOR);
	BYTE	b = GetBValue(item->COLOR);

	BYTE	r2 = GetRValue(item->COLOR2);
	BYTE	g2 = GetGValue(item->COLOR2);
	BYTE	b2 = GetBValue(item->COLOR2);

	if(0 == item->span_allocator)					// do not setup for image items only (these do not have AGG objects).
		return;

	item->dwFlags &= ~(AGG_USE_GRADIENT_X_RENDERER | AGG_USE_GRADIENT_Y_RENDERER | AGG_USE_SOLID_RENDERER);

	/*
	 * alpha values as read from the skin definition are in PERCENT (0-100)
	 */
	if(item->GRADIENT & GRADIENT_ACTIVE) {
		//agg::rgba8 r1 = agg::rgba8(r, g, b, percent_to_byte(item->ALPHA));
		//agg::rgba8 r2 = agg::rgba8(r2, g2, b2, percent_to_byte(item->ALPHA2));

		agg::rgba8 r1(r, g, b, percent_to_byte(item->ALPHA));
		agg::rgba8 r2(r2, g2, b2, percent_to_byte(item->ALPHA2));

		if(item->GRADIENT & GRADIENT_LR || item->GRADIENT & GRADIENT_TB) {
			AGGContext::fill_color_array(*(item->color_array), r1, r2);
			item->dwFlags |= (item->GRADIENT & GRADIENT_LR ? AGG_USE_GRADIENT_X_RENDERER : AGG_USE_GRADIENT_Y_RENDERER);
		}
		else {
			item->dwFlags |= (item->GRADIENT & GRADIENT_RL ? AGG_USE_GRADIENT_X_RENDERER : AGG_USE_GRADIENT_Y_RENDERER);
			AGGContext::fill_color_array(*(item->color_array), r2, r1);
		}
	}
	else {
		/*
		 * if no gradient is defined for this item, use a solid renderer
		 * it's faster.
		 */
		item->solid_renderer->color(agg::rgba8(r, g, b, percent_to_byte(item->ALPHA)));
		item->dwFlags |= AGG_USE_SOLID_RENDERER;
	}

	/*
	 * we can setup the corner radius values for our shape here. When rendering, we use
	 * item->rect->rect() to set the dimensions of the rectangle only.
	 *
	 * TODO make radius customizable
	 */

	item->rect->radius(0, 0, 0, 0, 0, 0, 0, 0);
	if(item->CORNER & CORNER_ACTIVE) {
		double x1 = 0, y1 = 0, x2 = 0, y2 = 0, x3 = 0, y3 = 0, x4 = 0, y4 = 0;

		if(item->CORNER & CORNER_TL)
			x1 = y1 = Skin::metrics.cCornerRadius;
		if(item->CORNER & CORNER_TR)
			x2 = y2 = Skin::metrics.cCornerRadius;
		if(item->CORNER & CORNER_BL)
			x4 = y4 = Skin::metrics.cCornerRadius;
		if(item->CORNER & CORNER_BR)
			x3 = y3 = Skin::metrics.cCornerRadius;

		item->rect->radius(x1, y1, x2, y2, x3, y3, x4, y4);
	}
}

/**
 * export skin-relevant database settings to the corresponding skin settings file
 * @param file
 */
void Skin::exportSettingsToFile(const char *file)
{
	// TODO needs work

	/*
	int i = 0;
	DWORD data;
	char  cPrefix, szBuf[30];
	*/
}
/**
 * exports all skin-relevant customization to a file
 * @param file	file name (INI format)
 */
void Skin::exportToFile(const char *file)
{
    int 		n;
    char 		szSection[255];
    char 		szKey[255];
    DBVARIANT 	dbv = {0};
    DWORD 		data;
    char*		szSectionName = 0, *p = 0;

    for (n = 0; n <= ID_EXTBK_LAST; n++) {
        if (statusItems[n].statusID != ID_EXTBKSEPARATOR) {
        	szSectionName = p = statusItems[n].szName;
        	if('{' == szSectionName[0])
        		p += 3;

        	Utils::writeProfile(p, "ColorHigh", statusItems[n].COLOR, file);
            Utils::writeProfile(p, "ColorLow", statusItems[n].COLOR2, file);
            Utils::writeProfile(p, "Textcolor", statusItems[n].TEXTCOLOR, file);
            Utils::writeProfile(p, "Ignored", statusItems[n].IGNORED, file);
            Utils::writeProfile(p, "Left", statusItems[n].MARGIN_LEFT, file);
            Utils::writeProfile(p, "Top", statusItems[n].MARGIN_TOP, file);
            Utils::writeProfile(p, "Right", statusItems[n].MARGIN_RIGHT, file);
            Utils::writeProfile(p, "Bottom", statusItems[n].MARGIN_BOTTOM, file);
            Utils::writeProfile(p, "Alpha", statusItems[n].ALPHA, file);
            Utils::writeProfile(p, "Alpha2", statusItems[n].ALPHA2, file);
            Utils::writeProfile(p, "Corner", statusItems[n].CORNER, file);
            Utils::writeProfile(p, "Gradient", statusItems[n].GRADIENT, file);
            Utils::writeProfile(p, "Flags", statusItems[n].dwFlags, file);
        }
    }
    for(n = 0; n <= FONTID_LAST; n++) {
        mir_snprintf(szSection, 255, "Font%d", n);

        mir_snprintf(szKey, 255, "Font%dName", n);
        if(!cfg::getString(NULL, "CLC", szKey, &dbv)) {
            Utils::writeProfile(szSection, "Name", dbv.pszVal, file);
            mir_free(dbv.pszVal);
        }
        mir_snprintf(szKey, 255, "Font%dSize", n);
        data = (DWORD)cfg::getByte("CLC", szKey, 8);
        Utils::writeProfile(szSection, "Size", data, file);

        mir_snprintf(szKey, 255, "Font%dSty", n);
        data = (DWORD)cfg::getByte("CLC", szKey, 8);
        Utils::writeProfile(szSection, "Style", data, file);

        mir_snprintf(szKey, 255, "Font%dSet", n);
        data = (DWORD)cfg::getByte("CLC", szKey, 8);
        Utils::writeProfile(szSection, "Set", data, file);

        mir_snprintf(szKey, 255, "Font%dCol", n);
        data = cfg::getDword("CLC", szKey, 8);
        Utils::writeProfile(szSection, "Color", data, file);

        mir_snprintf(szKey, 255, "Font%dFlags", n);
        data = (DWORD)cfg::getDword("CLC", szKey, 8);
        Utils::writeProfile(szSection, "Flags", data, file);

    }
}

static TStatusItem default_item =  {
	"{--Contact--}", 0,
     CLCDEFAULT_GRADIENT, CLCDEFAULT_CORNER,
     CLCDEFAULT_COLOR, CLCDEFAULT_COLOR2, CLCDEFAULT_COLOR2_TRANSPARENT, -1,
     CLCDEFAULT_ALPHA, CLCDEFAULT_MRGN_LEFT, CLCDEFAULT_MRGN_TOP, CLCDEFAULT_MRGN_RIGHT,
     CLCDEFAULT_MRGN_BOTTOM, CLCDEFAULT_IGNORE
};

void SkinLoader::readItem(TStatusItem *this_item, const char *szItem)
{
    TStatusItem *defaults = &default_item;
    DWORD		tmpflags;

    this_item->ALPHA = (int)GetPrivateProfileIntA(szItem, "Alpha", defaults->ALPHA, m_szFilename);
    this_item->ALPHA = min(this_item->ALPHA, 100);
    this_item->ALPHA2 = (int)GetPrivateProfileIntA(szItem, "Alpha2", defaults->ALPHA, m_szFilename);
    this_item->ALPHA2 = min(this_item->ALPHA2, 100);
    this_item->COLOR = GetPrivateProfileIntA(szItem, "ColorHigh", 0, m_szFilename);
    this_item->COLOR2 = GetPrivateProfileIntA(szItem, "ColorLow", 0, m_szFilename);

    this_item->CORNER = GetPrivateProfileIntA(szItem, "Corner", 0, m_szFilename);

    this_item->GRADIENT = GetPrivateProfileIntA(szItem, "Gradient", 0, m_szFilename);

    this_item->MARGIN_LEFT = GetPrivateProfileIntA(szItem, "Left", defaults->MARGIN_LEFT, m_szFilename);
    this_item->MARGIN_RIGHT = GetPrivateProfileIntA(szItem, "Right", defaults->MARGIN_RIGHT, m_szFilename);
    this_item->MARGIN_TOP = GetPrivateProfileIntA(szItem, "Top", defaults->MARGIN_TOP, m_szFilename);
    this_item->MARGIN_BOTTOM = GetPrivateProfileIntA(szItem, "Bottom", defaults->MARGIN_BOTTOM, m_szFilename);

    this_item->TEXTCOLOR = GetPrivateProfileIntA(szItem, "Textcolor", 0, m_szFilename);
    this_item->IGNORED = GetPrivateProfileIntA(szItem, "Ignored", 0, m_szFilename);
    tmpflags = GetPrivateProfileIntA(szItem, "Flags", 0, m_szFilename);
    this_item->dwFlags |= tmpflags;
}

void SkinLoader::loadBaseItems()
{
	int protoCount = 	0, i, n;
	PROTOACCOUNT**		accs = 0;
	char*				p = 0;

	Proto_EnumAccounts(&protoCount, &accs);

	if(Skin::statusItems) {
		free(Skin::statusItems);
		Skin::statusItems = 0;
		Skin::ID_EXTBK_LAST = ID_EXTBK_LAST_D;
	}
	Skin::statusItems = (TStatusItem *)malloc(sizeof(TStatusItem) * ((Skin::ID_EXTBK_LAST) + protoCount + 2));
	CopyMemory(Skin::statusItems, DefaultStatusItems, sizeof(DefaultStatusItems));

	for(i = 0; i < protoCount; i++) {
		Skin::ID_EXTBK_LAST++;
		CopyMemory(&Skin::statusItems[Skin::ID_EXTBK_LAST], &Skin::statusItems[0], sizeof(TStatusItem));
		if(i == 0) {
			lstrcpynA(Skin::statusItems[Skin::ID_EXTBK_LAST].szName, "{-}", 30);
			strncat(Skin::statusItems[Skin::ID_EXTBK_LAST].szName, accs[i]->szModuleName, 30);
		}
		else
			lstrcpynA(Skin::statusItems[Skin::ID_EXTBK_LAST].szName, accs[i]->szModuleName, 30);
		Skin::statusItems[Skin::ID_EXTBK_LAST].statusID = Skin::ID_EXTBK_LAST;
	}
	for (n = 0; n <= Skin::ID_EXTBK_LAST; n++) {
		if (Skin::statusItems[n].statusID != ID_EXTBKSEPARATOR) {
			Skin::statusItems[n].imageItem = 0;
			p = Skin::statusItems[n].szName;
			if(*p == '{')
				p += 3;

			TStatusItem* item = &Skin::statusItems[n];

			readItem(item, p);

			if(!(item->dwFlags & S_ITEM_IMAGE_ONLY)) {
				item->span_allocator	= new span_allocator_t;
				item->color_array 		= new agg::pod_auto_array<agg::rgba8, 256>();
				item->gradient_func_x 	= new agg::gradient_x();
				item->gradient_func_y 	= new agg::gradient_y();
				item->gradient_trans 	= new agg::trans_affine();
				item->span_interpolator = new agg::span_interpolator_linear<>(*(item->gradient_trans));
				item->span_gradient_x 	= new span_gradient_x_t(*(item->span_interpolator),
						*(item->gradient_func_x), *(item->color_array), 0, 200);

				item->span_gradient_y 	= new span_gradient_y_t(*(item->span_interpolator),
						*(item->gradient_func_y), *(item->color_array), 0, 200);

				item->pixfmt 			= new agg::pixfmt_bgra32(); //*(item->rbuf));
				item->rbase				= new agg::renderer_base<agg::pixfmt_bgra32>(); //*(item->pixfmt));
				item->gradient_renderer_x = new agg::renderer_scanline_aa<agg::renderer_base<agg::pixfmt_bgra32>, span_allocator_t, span_gradient_x_t>(*(item->rbase),
						*(item->span_allocator), *(item->span_gradient_x));

				item->gradient_renderer_y = new agg::renderer_scanline_aa<agg::renderer_base<agg::pixfmt_bgra32>, span_allocator_t, span_gradient_y_t>(*(item->rbase),
						*(item->span_allocator), *(item->span_gradient_y));

				item->solid_renderer 	= new agg::renderer_scanline_aa_solid<agg::renderer_base<agg::pixfmt_bgra32> >(*(item->rbase));

				item->rect				= new agg::rounded_rect(0, 0, 0, 0, 0);

				Skin::setupAGGItemContext(item);
			}
			else
				item->span_allocator = 0;
		}
	}
}

/**
 * read font definitions from INI style file and write them to the
 * database.
 *
 * uses m_szFilename to read from...
 */
void SkinLoader::loadFonts()
{
    int n;
    char buffer[255];
    char szKey[255], szSection[255];
    DWORD data;

	for(n = 0; n <= FONTID_LAST; n++) {
		mir_snprintf(szSection, 255, "Font%d", n);

		mir_snprintf(szKey, 255, "Font%dName", n);
		GetPrivateProfileStringA(szSection, "Name", "Arial", buffer, sizeof(buffer), m_szFilename);
		cfg::writeString(NULL, "CLC", szKey, buffer);

		mir_snprintf(szKey, 255, "Font%dSize", n);
		data = readInt(szSection, "Size", 10);
		cfg::writeByte("CLC", szKey, (BYTE)data);

		mir_snprintf(szKey, 255, "Font%dSty", n);
		data = readInt(szSection, "Style", 0);
		cfg::writeByte("CLC", szKey, (BYTE)data);

		mir_snprintf(szKey, 255, "Font%dSet", n);
		data = readInt(szSection, "Set", 1);
		cfg::writeByte("CLC", szKey, (BYTE)data);

		mir_snprintf(szKey, 255, "Font%dCol", n);
		data = readInt(szSection, "Color", 0x00);
		cfg::writeDword("CLC", szKey, data);

		mir_snprintf(szKey, 255, "Font%dFlags", n);
		data = readInt(szSection, "Flags", 0);
		cfg::writeDword("CLC", szKey, (WORD)data);
	}
}

/**
 * load a skin with the given name.
 * @param wszFilename:	full path and file name of the skin
 *  					definition file (.cng format)
 *
 * after constructing, check ::isValid() before using the Load()
 * method.
 */
SkinLoader::SkinLoader(const wchar_t* wszFilename)
{
	wchar_t		wszBase[MAX_PATH], wszRelPath[MAX_PATH];
	m_isValid = false;
	wchar_t		wszDrive[_MAX_DRIVE], wszDir[_MAX_DIR], wszFile[_MAX_FNAME];

	Skin::settings.wszSkinBaseFolder[0] = Skin::settings.wszSkinName[0] = 0;

	mir_sntprintf(wszBase, MAX_PATH, L"%s%s", cfg::szProfileDir, L"skin\\clng\\base\\base.cng");
	if(0 == wszFilename) {
		if(PathFileExists(wszBase)) {
			wcsncpy(m_wszFilename, wszBase, MAX_PATH);
			m_wszFilename[MAX_PATH - 1] = 0;
			WideCharToMultiByte(CP_ACP, 0, wszBase, MAX_PATH, m_szFilename, MAX_PATH, 0, 0);
			m_isValid = true;
		}
	}
	else {
		if(PathFileExists(wszFilename)) {
			WideCharToMultiByte(CP_ACP, 0, wszFilename, MAX_PATH, m_szFilename, MAX_PATH, 0, 0);
			wcsncpy(m_wszFilename, wszFilename, MAX_PATH);
			m_wszFilename[MAX_PATH - 1] = 0;
		}
		else {
			WideCharToMultiByte(CP_ACP, 0, wszBase, MAX_PATH, m_szFilename, MAX_PATH, 0, 0);
			wcsncpy(m_wszFilename, wszBase, MAX_PATH);
			m_wszFilename[MAX_PATH - 1] = 0;
		}
		m_isValid = true;
	}
	Utils::pathToRelative(m_wszFilename, wszRelPath, cfg::szProfileDir);
	cfg::writeTString(0, SKIN_DB_MODULE, "gCurrentSkin", wszRelPath);
	_wsplitpath(m_wszFilename, wszDrive, wszDir, wszFile, 0);
	mir_sntprintf(Skin::settings.wszSkinBaseFolder, MAX_PATH, L"%s%s", wszDrive, wszDir);
	mir_sntprintf(Skin::settings.wszSkinName, MAX_PATH, L"%s", wszFile);
}

/**
 * read a setting from the skin definition file
 * the filename is in m_szFilename and was set in the ctor
 *
 * @param szSection		INI section
 * @param szValue		key name
 * @param dwDefault		default when the entry is not found
 * @return				DWORD containing the read value (or the default)
 */
DWORD SkinLoader::readInt(const char* szSection, const char* szValue, DWORD dwDefault)
{
	if(m_isValid)
		return(GetPrivateProfileIntA(szSection, szValue, dwDefault, m_szFilename));
	else
		return(0);
}

HRESULT SkinLoader::Load()
{
	/*
	 * load metrics and other skin data
	 */

	Skin::metrics.cLeftSkinned = readInt("FramelessMetrics", "CLeft", 0);
	Skin::metrics.cRightSkinned = readInt("FramelessMetrics", "CRight", 0);
	Skin::metrics.cTopSkinned = readInt("FramelessMetrics", "CTop", 0);
	Skin::metrics.cBottomSkinned = readInt("FramelessMetrics", "CBottom", 0);

	Skin::metrics.cLeftFramed = readInt("Metrics", "CLeft", 0);
	Skin::metrics.cRightFramed = readInt("Metrics", "CRight", 0);
	Skin::metrics.cTopFramed = readInt("Metrics", "CTop", 0);
	Skin::metrics.cBottomFramed = readInt("Metrics", "CBottom", 0);

	Skin::metrics.bWindowStyle = readInt("Settings", "DefaultWindowStyle", SETTING_WINDOWSTYLE_NOBORDER);
	Skin::metrics.cButtonHeight = readInt("Settings", "ButtonHeight", BUTTON_HEIGHT_D);
	Skin::metrics.cFakeCaption = readInt("FramelessMetrics", "CaptionHeight", 0);
	Skin::metrics.cFakeLeftBorder = readInt("FramelessMetrics", "LeftBorderWidth", 0);
	Skin::metrics.cFakeRightBorder = readInt("FramelessMetrics", "RightBorderWidth", 0);
	Skin::metrics.cFakeBtmBorder = readInt("FramelessMetrics", "BottomBorderWidth", 0);

	Skin::metrics.cTopButtonset = readInt("Buttonset", "Top", 0);
	Skin::metrics.cLeftButtonset = readInt("Buttonset", "Left", 0);
	Skin::metrics.cRightButtonset = readInt("Buttonset", "Right", 0);
	Skin::metrics.cBottomButtonset = readInt("Buttonset", "Bottom", 0);

	Skin::metrics.cCornerRadius = readInt("Settings", "CornerRadius", 5);
	Skin::settings.fUseAero = cfg::getByte(SKIN_DB_MODULE, "sfUseAero", 1) ? true : false;

	/*
	 * read the base items and fill the structure
	 */
	loadBaseItems();

	/*
	 * load all other skin elements (images, buttons, icons)
	 */
	if(!SUCCEEDED(loadItems()))
		return(-S_FALSE);

	/*
	 * TODO read font settings only when necessary (when skin has changed)
	 */

	loadFonts();
	cfg::FS_RegisterFonts();

	/*
	 * validations (check image items, border styles and make sure everything makes
	 * sense). Protect against skin definition errors
	 */
	Skin::settings.fHaveAeroBG = CLUI::bgImageItem ? true : false;
	if(Skin::metrics.bWindowStyle == SETTING_WINDOWSTYLE_NOBORDER && (0 == CLUI::bgImageItem || 0 == CLUI::bgImageItem_nonAero))
		Skin::metrics.bWindowStyle = SETTING_WINDOWSTYLE_DEFAULT;

	return(S_OK);
}

HRESULT SkinLoader::loadItems()
{
    char*		szSections = 0, *szFileName;
    char*		p;

    /*
     * TODO rewrite the skin loading in wchar_t manner
     */

    if(!PathFileExistsA(m_szFilename) || !m_isValid)
        return(-S_FALSE);

	szFileName = m_szFilename;

    szSections = reinterpret_cast<char *>(malloc(3002));
    ZeroMemory(szSections, 3002);
    p = szSections;
    GetPrivateProfileSectionNamesA(szSections, 3000, szFileName);

    szSections[3001] = szSections[3000] = 0;
    p = szSections;
    while(lstrlenA(p) > 1) {
		if(p[0] == '$' || p[0] == '@')
            readImageItem(p, szFileName);
        p += (lstrlenA(p) + 1);
    }
    nextButtonID = IDC_TBFIRSTUID;
    p = szSections;
    while(lstrlenA(p) > 1) {
        if(p[0] == '!')
            readButtonItem(p, szFileName);
        p += (lstrlenA(p) + 1);
    }
    if(pcli && pcli->hwndContactList)
        CLUI::setButtonStates(pcli->hwndContactList);
    free(szSections);

    if(CLUI::bgImageItem)
        cfg::dat.dwFlags &= ~CLUI_FRAME_CLISTSUNKEN;

    return(S_OK);
}

void SkinLoader::readImageItem(const char *itemname, const char *szFileName)
{
    TImageItem tmpItem, *newItem = NULL;
    char buffer[512], szItemNr[30];
    char szFinalName[MAX_PATH];
    HDC hdc = GetDC(pcli->hwndContactList);
    int i, n;
    BOOL alloced = FALSE;
    char szDrive[MAX_PATH], szPath[MAX_PATH];

    ZeroMemory(&tmpItem, sizeof(TImageItem));
    GetPrivateProfileStringA(itemname, "Glyph", "None", buffer, 500, szFileName);
    if(strcmp(buffer, "None")) {
        sscanf(buffer, "%d,%d,%d,%d", &tmpItem.glyphMetrics[0], &tmpItem.glyphMetrics[1],
               &tmpItem.glyphMetrics[2], &tmpItem.glyphMetrics[3]);
        if(tmpItem.glyphMetrics[2] > tmpItem.glyphMetrics[0] && tmpItem.glyphMetrics[3] > tmpItem.glyphMetrics[1]) {
            tmpItem.dwFlags |= IMAGE_GLYPH;
            tmpItem.glyphMetrics[2] = (tmpItem.glyphMetrics[2] - tmpItem.glyphMetrics[0]) + 1;
            tmpItem.glyphMetrics[3] = (tmpItem.glyphMetrics[3] - tmpItem.glyphMetrics[1]) + 1;
            goto done_with_glyph;
        }
    }
    GetPrivateProfileStringA(itemname, "Image", "None", buffer, 500, szFileName);
    if(strcmp(buffer, "None")) {

done_with_glyph:

        strncpy(tmpItem.szName, &itemname[0], sizeof(tmpItem.szName));
        tmpItem.szName[sizeof(tmpItem.szName) - 1] = 0;
        _splitpath(szFileName, szDrive, szPath, NULL, NULL);
        mir_snprintf(szFinalName, MAX_PATH, "%s\\%s\\%s", szDrive, szPath, buffer);
        tmpItem.alpha = GetPrivateProfileIntA(itemname, "Alpha", 100, szFileName);
        tmpItem.alpha = min(tmpItem.alpha, 100);
        tmpItem.alpha = (BYTE)((FLOAT)(((FLOAT) tmpItem.alpha) / 100) * 255);
        tmpItem.bf.SourceConstantAlpha = tmpItem.alpha;
        tmpItem.bLeft = GetPrivateProfileIntA(itemname, "Left", 0, szFileName);
        tmpItem.bRight = GetPrivateProfileIntA(itemname, "Right", 0, szFileName);
        tmpItem.bTop = GetPrivateProfileIntA(itemname, "Top", 0, szFileName);
        tmpItem.bBottom = GetPrivateProfileIntA(itemname, "Bottom", 0, szFileName);
        if(tmpItem.dwFlags & IMAGE_GLYPH) {
            tmpItem.width = tmpItem.glyphMetrics[2];
            tmpItem.height = tmpItem.glyphMetrics[3];
            tmpItem.inner_height = tmpItem.glyphMetrics[3] - tmpItem.bTop - tmpItem.bBottom;
            tmpItem.inner_width = tmpItem.glyphMetrics[2] - tmpItem.bRight - tmpItem.bLeft;

            if(tmpItem.bTop && tmpItem.bBottom && tmpItem.bLeft && tmpItem.bRight)
                tmpItem.dwFlags |= IMAGE_FLAG_DIVIDED;
            tmpItem.bf.BlendFlags = 0;
            tmpItem.bf.BlendOp = AC_SRC_OVER;
            tmpItem.bf.AlphaFormat = 0;
            tmpItem.dwFlags |= IMAGE_PERPIXEL_ALPHA;
            tmpItem.bf.AlphaFormat = AC_SRC_ALPHA;
            if(tmpItem.inner_height <= 0 || tmpItem.inner_width <= 0) {
                ReleaseDC(pcli->hwndContactList, hdc);
                return;
            }
        }
        GetPrivateProfileStringA(itemname, "Fillcolor", "None", buffer, 500, szFileName);
        if(strcmp(buffer, "None")) {
            COLORREF fillColor = Utils::hexStringToLong(buffer);
            tmpItem.fillBrush = CreateSolidBrush(fillColor);
            tmpItem.dwFlags |= IMAGE_FILLSOLID;
        }
        else
            tmpItem.fillBrush = 0;

        GetPrivateProfileStringA(itemname, "Stretch", "None", buffer, 500, szFileName);
        if(buffer[0] == 'B' || buffer[0] == 'b')
            tmpItem.bStretch = IMAGE_STRETCH_B;
        else if(buffer[0] == 'h' || buffer[0] == 'H')
            tmpItem.bStretch = IMAGE_STRETCH_V;
        else if(buffer[0] == 'w' || buffer[0] == 'W')
            tmpItem.bStretch = IMAGE_STRETCH_H;
        tmpItem.hbm = 0;

        if(!_stricmp(itemname, "$glyphs")) {
            createImageItem(&tmpItem, szFinalName, hdc);
            if(tmpItem.hbm) {
                newItem = reinterpret_cast<TImageItem *>(malloc(sizeof(TImageItem)));
                ZeroMemory(newItem, sizeof(TImageItem));
                *newItem = tmpItem;
                Skin::glyphItem = newItem;
            }
            goto imgread_done;
        }
        if(itemname[0] == '@') {
            if(!(tmpItem.dwFlags & IMAGE_GLYPH))
                createImageItem(&tmpItem, szFinalName, hdc);
            if(tmpItem.hbm || tmpItem.dwFlags & IMAGE_GLYPH) {

                newItem = reinterpret_cast<TImageItem *>(malloc(sizeof(TImageItem)));
                ZeroMemory(newItem, sizeof(TImageItem));
                *newItem = tmpItem;

                if(Skin::imageItems == NULL)
                	Skin::imageItems = newItem;
                else {
                    TImageItem *pItem = Skin::imageItems;

                    while(pItem->nextItem != 0)
                        pItem = pItem->nextItem;
                    pItem->nextItem = newItem;
                }
            }
            goto imgread_done;
        }
        for(n = 0;;n++) {
            mir_snprintf(szItemNr, 30, "Item%d", n);
            GetPrivateProfileStringA(itemname, szItemNr, "None", buffer, 500, szFileName);
            if(!strcmp(buffer, "None"))
                break;
            if(!stricmp(buffer, "CLUI") || !stricmp(buffer, "CLUIAero") || !stricmp(buffer, "CLUIClient")) {
                if(!(tmpItem.dwFlags & IMAGE_GLYPH))
                    createImageItem(&tmpItem, szFinalName, hdc);
                if(tmpItem.hbm || tmpItem.dwFlags & IMAGE_GLYPH) {
                    COLORREF clr;

                    newItem = reinterpret_cast<TImageItem *>(malloc(sizeof(TImageItem)));
                    ZeroMemory(newItem, sizeof(TImageItem));
                    *newItem = tmpItem;

                    if(!stricmp(buffer, "CLUIAero"))
                    	CLUI::bgImageItem = newItem;
                    else if(!stricmp(buffer, "CLUIClient"))
                    	CLUI::bgClientItem = newItem;
                    else {
                        GetPrivateProfileStringA(itemname, "Colorkey", "e5e5e5", buffer, 500, szFileName);
                        clr = Utils::hexStringToLong(buffer);
                        cfg::dat.colorkey = clr;
                        cfg::writeDword("CLUI", "ColorKey", clr);
                        if(g_CLUISkinnedBkColor)
                            DeleteObject(g_CLUISkinnedBkColor);
                        g_CLUISkinnedBkColor = CreateSolidBrush(clr);
                        g_CLUISkinnedBkColorRGB = clr;
                    	CLUI::bgImageItem_nonAero = newItem;
                    }
                }
                continue;
            }
            for(i = 0; i <= Skin::ID_EXTBK_LAST; i++) {
                if(!_stricmp(Skin::statusItems[i].szName[0] == '{' ? &Skin::statusItems[i].szName[3] : Skin::statusItems[i].szName, buffer)) {
                    if(!alloced) {
                        if(!(tmpItem.dwFlags & IMAGE_GLYPH))
                            createImageItem(&tmpItem, szFinalName, hdc);
                        if(tmpItem.hbm || tmpItem.dwFlags & IMAGE_GLYPH) {
                            newItem = reinterpret_cast<TImageItem *>(malloc(sizeof(TImageItem)));
                            ZeroMemory(newItem, sizeof(TImageItem));
                            *newItem = tmpItem;
                            Skin::statusItems[i].imageItem = newItem;
                            if(Skin::imageItems == NULL)
                            	Skin::imageItems = newItem;
                            else {
                                TImageItem *pItem = Skin::imageItems;

                                while(pItem->nextItem != 0)
                                    pItem = pItem->nextItem;
                                pItem->nextItem = newItem;
                            }
                            alloced = TRUE;
                        }
                    }
                    else if(newItem != NULL)
                    	Skin::statusItems[i].imageItem = newItem;
                }
            }
        }
    }
imgread_done:
    ReleaseDC(pcli->hwndContactList, hdc);

}

void SkinLoader::readButtonItem(const char *itemName, const char *file)
{
    TButtonItem tmpItem, *newItem;
    char szBuffer[1024];
    TImageItem *imgItem = Skin::imageItems;

    ZeroMemory(&tmpItem, sizeof(tmpItem));
    mir_snprintf(tmpItem.szName, sizeof(tmpItem.szName), "%s", &itemName[1]);
    tmpItem.width = GetPrivateProfileIntA(itemName, "Width", 16, file);
    tmpItem.height = GetPrivateProfileIntA(itemName, "Height", 16, file);
    tmpItem.xOff = GetPrivateProfileIntA(itemName, "xoff", 0, file);
    tmpItem.yOff = GetPrivateProfileIntA(itemName, "yoff", 0, file);

    tmpItem.dwFlags |= GetPrivateProfileIntA(itemName, "toggle", 0, file) ? BUTTON_ISTOGGLE : 0;
    tmpItem.dwFlags |= (GetPrivateProfileIntA(itemName, "FakeTitleButton", 0, file) ? (BUTTON_FRAMELESS_ONLY | BUTTON_FAKE_CAPTIONBUTTON) : 0);

    GetPrivateProfileStringA(itemName, "Pressed", "None", szBuffer, 1000, file);
    if(!_stricmp(szBuffer, "default"))
        tmpItem.imgPressed = Skin::statusItems[ID_EXTBKBUTTONSPRESSED].imageItem;
    else {
        while(imgItem) {
            if(!_stricmp(&imgItem->szName[1], szBuffer)) {
                tmpItem.imgPressed = imgItem;
                break;
            }
            imgItem = imgItem->nextItem;
        }
    }

    imgItem = Skin::imageItems;
    GetPrivateProfileStringA(itemName, "Normal", "None", szBuffer, 1000, file);
    if(!_stricmp(szBuffer, "default"))
        tmpItem.imgNormal = Skin::statusItems[ID_EXTBKBUTTONSNPRESSED].imageItem;
    else {
        while(imgItem) {
            if(!_stricmp(&imgItem->szName[1], szBuffer)) {
                tmpItem.imgNormal = imgItem;
                break;
            }
            imgItem = imgItem->nextItem;
        }
    }

    imgItem = Skin::imageItems;
    GetPrivateProfileStringA(itemName, "Hover", "None", szBuffer, 1000, file);
    if(!_stricmp(szBuffer, "default"))
        tmpItem.imgHover = Skin::statusItems[ID_EXTBKBUTTONSMOUSEOVER].imageItem;
    else {
        while(imgItem) {
            if(!_stricmp(&imgItem->szName[1], szBuffer)) {
                tmpItem.imgHover = imgItem;
                break;
            }
            imgItem = imgItem->nextItem;
        }
    }
    GetPrivateProfileStringA(itemName, "Align", "lt", szBuffer, 1000, file);
    if(lstrlenA(szBuffer) == 2) {
    	if(szBuffer[0] =='r' || szBuffer[0] == 'R')
    		tmpItem.dwFlags |= BUTTON_HALIGN_R;
    	if(szBuffer[1] == 'B' || szBuffer[1] == 'B')
    		tmpItem.dwFlags |= BUTTON_VALIGN_B;
    }
    GetPrivateProfileStringA(itemName, "NormalGlyph", "0, 0, 0, 0", szBuffer, 1000, file);
    sscanf(szBuffer, "%d,%d,%d,%d", &tmpItem.normalGlyphMetrics[0], &tmpItem.normalGlyphMetrics[1],
           &tmpItem.normalGlyphMetrics[2], &tmpItem.normalGlyphMetrics[3]);
    tmpItem.normalGlyphMetrics[2] = (tmpItem.normalGlyphMetrics[2] - tmpItem.normalGlyphMetrics[0]) + 1;
    tmpItem.normalGlyphMetrics[3] = (tmpItem.normalGlyphMetrics[3] - tmpItem.normalGlyphMetrics[1]) + 1;

    GetPrivateProfileStringA(itemName, "PressedGlyph", "0, 0, 0, 0", szBuffer, 1000, file);
    sscanf(szBuffer, "%d,%d,%d,%d", &tmpItem.pressedGlyphMetrics[0], &tmpItem.pressedGlyphMetrics[1],
           &tmpItem.pressedGlyphMetrics[2], &tmpItem.pressedGlyphMetrics[3]);
    tmpItem.pressedGlyphMetrics[2] = (tmpItem.pressedGlyphMetrics[2] - tmpItem.pressedGlyphMetrics[0]) + 1;
    tmpItem.pressedGlyphMetrics[3] = (tmpItem.pressedGlyphMetrics[3] - tmpItem.pressedGlyphMetrics[1]) + 1;


    GetPrivateProfileStringA(itemName, "HoverGlyph", "0, 0, 0, 0", szBuffer, 1000, file);
    sscanf(szBuffer, "%d,%d,%d,%d", &tmpItem.hoverGlyphMetrics[0], &tmpItem.hoverGlyphMetrics[1],
           &tmpItem.hoverGlyphMetrics[2], &tmpItem.hoverGlyphMetrics[3]);
    tmpItem.hoverGlyphMetrics[2] = (tmpItem.hoverGlyphMetrics[2] - tmpItem.hoverGlyphMetrics[0]) + 1;
    tmpItem.hoverGlyphMetrics[3] = (tmpItem.hoverGlyphMetrics[3] - tmpItem.hoverGlyphMetrics[1]) + 1;

    tmpItem.uId = IDC_TBFIRSTUID - 1;

    GetPrivateProfileStringA(itemName, "Action", "Custom", szBuffer, 1000, file);
    if(!_stricmp(szBuffer, "service")) {
        tmpItem.szService[0] = 0;
        GetPrivateProfileStringA(itemName, "Service", "None", szBuffer, 1000, file);
        if(_stricmp(szBuffer, "None")) {
            mir_snprintf(tmpItem.szService, 256, "%s", szBuffer);
            tmpItem.dwFlags |= BUTTON_ISSERVICE;
            tmpItem.uId = nextButtonID++;
        }
    }
    else if(!_stricmp(szBuffer, "protoservice")) {
        tmpItem.szService[0] = 0;
        GetPrivateProfileStringA(itemName, "Service", "None", szBuffer, 1000, file);
        if(_stricmp(szBuffer, "None")) {
            mir_snprintf(tmpItem.szService, 256, "%s", szBuffer);
            tmpItem.dwFlags |= BUTTON_ISPROTOSERVICE;
            tmpItem.uId = nextButtonID++;
        }
    }
    else if(!_stricmp(szBuffer, "database")) {
        int n;

        GetPrivateProfileStringA(itemName, "Module", "None", szBuffer, 1000, file);
        if(_stricmp(szBuffer, "None"))
            mir_snprintf(tmpItem.szModule, 256, "%s", szBuffer);
        GetPrivateProfileStringA(itemName, "Setting", "None", szBuffer, 1000, file);
        if(_stricmp(szBuffer, "None"))
            mir_snprintf(tmpItem.szSetting, 256, "%s", szBuffer);
        if(GetPrivateProfileIntA(itemName, "contact", 0, file) != 0)
           tmpItem.dwFlags |= BUTTON_DBACTIONONCONTACT;

        for(n = 0; n <= 1; n++) {
            char szKey[20];
            BYTE *pValue;

            strcpy(szKey, n == 0 ? "dbonpush" : "dbonrelease");
            pValue = (n == 0 ? tmpItem.bValuePush : tmpItem.bValueRelease);

            GetPrivateProfileStringA(itemName, szKey, "None", szBuffer, 1000, file);
            switch(szBuffer[0]) {
                case 'b':
                {
                    BYTE value = (BYTE)atol(&szBuffer[1]);
                    pValue[0] = value;
                    tmpItem.type = DBVT_BYTE;
                    break;
                }
                case 'w':
                {
                    WORD value = (WORD)atol(&szBuffer[1]);
                    *((WORD *)&pValue[0]) = value;
                    tmpItem.type = DBVT_WORD;
                    break;
                }
                case 'd':
                {
                    DWORD value = (DWORD)atol(&szBuffer[1]);
                    *((DWORD *)&pValue[0]) = value;
                    tmpItem.type = DBVT_DWORD;
                    break;
                }
                case 's':
                {
                    mir_snprintf((char *)pValue, 256, &szBuffer[1]);
                    tmpItem.type = DBVT_ASCIIZ;
                    break;
                }
            }
        }
        if(tmpItem.szModule[0] && tmpItem.szSetting[0]) {
            tmpItem.dwFlags |= BUTTON_ISDBACTION;
            if(tmpItem.szModule[0] == '$' && (tmpItem.szModule[1] == 'c' || tmpItem.szModule[1] == 'C'))
                tmpItem.dwFlags |= BUTTON_ISCONTACTDBACTION;
            tmpItem.uId = nextButtonID++;
        }
    }
    else if(_stricmp(szBuffer, "Custom")) {
        int i = 0;

        while(top_buttons[i].id) {
            if(!_stricmp(top_buttons[i].szIcoLibIcon, szBuffer)) {
                tmpItem.uId = top_buttons[i].id;
                tmpItem.dwFlags |= BUTTON_ISINTERNAL;
                break;
            }
            i++;
        }
    }
    GetPrivateProfileStringA(itemName, "PassContact", "None", szBuffer, 1000, file);
    if(_stricmp(szBuffer, "None")) {
        if(szBuffer[0] == 'w' || szBuffer[0] == 'W')
            tmpItem.dwFlags |= BUTTON_PASSHCONTACTW;
        else if(szBuffer[0] == 'l' || szBuffer[0] == 'L')
            tmpItem.dwFlags |= BUTTON_PASSHCONTACTL;
    }

    GetPrivateProfileStringA(itemName, "Tip", "None", szBuffer, 1000, file);
    if(strcmp(szBuffer, "None")) {
        MultiByteToWideChar(cfg::dat.langPackCP, 0, szBuffer, -1, tmpItem.szTip, 256);
        tmpItem.szTip[255] = 0;
    }
    else
        tmpItem.szTip[0] = 0;

    // create it

    newItem = (TButtonItem *)malloc(sizeof(TButtonItem));
    ZeroMemory(newItem, sizeof(TButtonItem));
    if(CLUI::buttonItems == NULL) {
        CLUI::buttonItems = newItem;
        *newItem = tmpItem;
        newItem->nextItem = 0;
    }
    else {
        TButtonItem *curItem = CLUI::buttonItems;
        while(curItem->nextItem)
            curItem = curItem->nextItem;
        *newItem = tmpItem;
        newItem->nextItem = 0;
        curItem->nextItem = newItem;
    }
    newItem->hWnd = CreateWindowEx(0, _T("CLCButtonClass"), _T(""), BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD | WS_TABSTOP, 0, 0, 5, 5, pcli->hwndContactList, (HMENU)newItem->uId, g_hInst, NULL);

    SendMessage(newItem->hWnd, BM_SETBTNITEM, 0, (LPARAM)newItem);
    SendMessage(newItem->hWnd, BUTTONSETASFLATBTN, 0, 0);
    SendMessage(newItem->hWnd, BUTTONSETASFLATBTN + 10, 0, 0);
    if(newItem->dwFlags & BUTTON_ISTOGGLE)
        SendMessage(newItem->hWnd, BUTTONSETASPUSHBTN, 0, 0);

    if(newItem->szTip[0])
        SendMessage(newItem->hWnd, BUTTONADDTOOLTIP, (WPARAM)newItem->szTip, 0);
    return;
}

void SkinLoader::createImageItem(TImageItem *item, const char *fileName, HDC hdc)
{
    HBITMAP hbm = Gfx::loadPNG(fileName);
    BITMAP bm;

    if(hbm) {
        item->hbm = hbm;
        item->bf.BlendFlags = 0;
        item->bf.BlendOp = AC_SRC_OVER;
        item->bf.AlphaFormat = 0;

        GetObject(hbm, sizeof(bm), &bm);
        if(bm.bmBitsPixel == 32) {
            Gfx::preMultiply(hbm, 1);
            item->dwFlags |= IMAGE_PERPIXEL_ALPHA;
            item->bf.AlphaFormat = AC_SRC_ALPHA;
        }
        item->width = bm.bmWidth;
        item->height = bm.bmHeight;
        item->inner_height = item->height - item->bTop - item->bBottom;
        item->inner_width = item->width - item->bLeft - item->bRight;
        if(item->bTop && item->bBottom && item->bLeft && item->bRight) {
            item->dwFlags |= IMAGE_FLAG_DIVIDED;
            if(item->inner_height <= 0 || item->inner_width <= 0) {
                DeleteObject(hbm);
                item->hbm = 0;
                return;
            }
        }
        item->hdc = CreateCompatibleDC(hdc);
        item->hbmOld = reinterpret_cast<HBITMAP>(SelectObject(item->hdc, item->hbm));
    }
}
