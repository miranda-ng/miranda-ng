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
 * (C) 2005-2015 by silvercircle _at_ gmail _dot_ com and contributors
 *
 * clist_ng skin helper classes and functions. No low level Gfx here, just
 * managing the skin data structures...
 *
 * drawing is mostly in Gfx
 *
 * TODO implement settings override system. Allow user to save modified
 * skin settings to a separate override file and use these modified settings
 * when loading the skin.
 *
 * TODO implement logic to determine whether a skin has changed to optimize
 * loading performance (e.g. no need to import fonts and color settings when
 *
 * TODO implement replacement for the old clist_nicer style "per contact" skin
 * items. Allow colorization of contact rows on a per user base. The settings
 * could be added to the "Contact list settings" dialog.
 */

#ifndef __SKIN_H_
#define __SKIN_H_

#define ID_EXTBKOFFLINE			0
#define ID_EXTBKONLINE			1
#define ID_EXTBKAWAY			2
#define ID_EXTBKDND				3
#define ID_EXTBKNA				4
#define ID_EXTBKOCCUPIED		5
#define ID_EXTBKFREECHAT		6
#define ID_EXTBKINVISIBLE		7
#define ID_EXTBKONTHEPHONE		8
#define ID_EXTBKOUTTOLUNCH		9

#define ID_EXTBKIDLE            10
#define ID_EXTBKEXPANDEDGROUP   11
#define ID_EXTBKCOLLAPSEDDGROUP 12
#define ID_EXTBKEMPTYGROUPS     13
#define ID_EXTBKFIRSTITEM       14
#define ID_EXTBKSINGLEITEM      15
#define ID_EXTBKLASTITEM        16


#define ID_EXTBKFIRSTITEM_NG    17
#define ID_EXTBKSINGLEITEM_NG   18
#define ID_EXTBKLASTITEM_NG     19

#define ID_EXTBKEVEN_CNTCTPOS   20
#define ID_EXTBKODD_CNTCTPOS    21

#define ID_EXTBKSELECTION       22
#define ID_EXTBKHOTTRACK        23
#define ID_EXTBKFRAMETITLE      24
#define ID_EXTBKEVTAREA         25
#define ID_EXTBKSTATUSBAR       26
#define ID_EXTBKBUTTONSPRESSED  27
#define ID_EXTBKBUTTONSNPRESSED 28
#define ID_EXTBKBUTTONSMOUSEOVER 	29
#define ID_EXTBKSTATUSFLOATER	    30
#define ID_EXTBKOWNEDFRAMEBORDER    31
#define ID_EXTBKOWNEDFRAMEBORDERTB  32
#define ID_EXTBKAVATARFRAME         33
#define ID_EXTBKAVATARFRAMEOFFLINE  34
#define ID_EXTBK_LAST_D             34

#define ID_EXTBKSEPARATOR           40200

void extbk_import(char *file, HWND hwndDlg);
TStatusItem *GetProtocolStatusItem(const char *szProto);
void SaveNonStatusItemsSettings(HWND hwndDlg);

#define SKIN_DB_MODULE "CLNgSkin"					// database module for clng skinning

/*
 * flags for status item rendering
 */
#define AGG_USE_SOLID_RENDERER 1				// item has no gradient -> use a faster solid renderer
#define AGG_USE_GRADIENT_X_RENDERER 2			// item has horizontal gradient -> use agg::gradient_x function
#define AGG_USE_GRADIENT_Y_RENDERER 4			// item has vertical gradient -> use agg::gradient_y function
#define S_ITEM_SKIP_UNDERLAY 8					// skip all gradient underlays and render the image item only
#define S_ITEM_TEXT_AERO_GLOW 16				// render text with aero glow instead of solid color
#define S_ITEM_IMAGE_ONLY	  32				// this item cannot be used with gradients (only for image skinning)
#define S_ITEM_SKIP_IMAGE 64					// skip all gradient underlays and render the image item only

/*
 * this structure describes all relevant settings that are needed
 * for a skin except the skin items themselves. Usually, they are loaded
 * from the skin definition (.cng), but the user can override them.
 * Most of these settings are available on the option pages.
 *
 * These settings are copied to the database under ClNGSkin and have a m prefix.
 * (e.g. cLeftFramed member name = "mLeftFramed" in the db.
 */
struct TSkinMetrics {
	BYTE		cLeft, cTop, cRight, cBottom;								// these are the currently _effective_ values and are calculated
																			// from the other margins, never saved anywhere, only exist at runtime
	BYTE		cLeftFramed, cTopFramed, cRightFramed, cBottomFramed;		// margins when using a window frame
	BYTE		cLeftSkinned, cTopSkinned, cRightSkinned, cBottomSkinned;	// margins when using a window skin
	BYTE		cLeftButtonset, cTopButtonset, cRightButtonset, cBottomButtonset;	// additional margins for the button sets
	BYTE		bWindowStyle;												// border style (none, toolbar or normal title bar
	BYTE		cButtonHeight;												// height of the bottom buttons
	BYTE		bSBarHeight;												// status bar height adjustment (for skinned mode only)
	DWORD		dwTopOffset, dwBottomOffset;								// cTop + additional paddings (rare)
	BYTE		cFakeCaption;												// for frameless skin mode - artificial caption bar height, will be added to top aligned buttons
																			// which do not have the title bar attribute
	BYTE		cFakeLeftBorder;
	BYTE		cFakeRightBorder;
	BYTE		cFakeBtmBorder;
	bool		fHaveFrame;													// quick boolean, true if clist window has a frame
	bool		fHaveAeroSkin;												// true if using the background skin in aero mode
	bool		fHaveColorkey;												// true if using a background skin in non-aero mode and
																			// we need a colorkey to get transparent areas
	DWORD		cCornerRadius;												// corner radius used for gradient items
};

/*
 * settings for a skin (everything else with the exception of geometry information
 *
 * Database module: ClNGSkin and values have a s prefix.
 * e.gl member crColorKey = sColorKey in the db.
 */
struct TSkinSettings {
	COLORREF	crColorKey;													// the colorkey for LWA_COLORKEY
	bool		fTransparentFrames;											// make owned frames transparent
	COLORREF	crFramesColorKey;											// background color which should be made transparent in frames
	BYTE		bFramesAlpha;												// constant alpha for translucent frames
	bool		fUseAero;													// may be used to disable aero regardless of operating system state
	bool		fHaveAeroBG;												// we have a valid background item for aero mode
	wchar_t		wszSkinBaseFolder[MAX_PATH];								// internal use only - base directory of the currently loaded skin
	wchar_t		wszSkinName[MAX_PATH];										// internal use only - base name of the skin (the .cng file without extension)
};
/*
 * the Skin class has helper functions to manage skin items and stores the
 * items (image items, button items and status items
 */
class Skin
{
public:
	static void				Unload								();
	static void 			freeImageItem						(TImageItem *item);
	static void 			exportToFile						(const char *file);
	static void 			exportSettingsToFile				(const char *file);

	static void 			renderNamedImageItem				(const char *szItemname, RECT* rc, const HDC hdc);
	static void 			colorizeGlyphByName					(const char* szGlyphName, const COLORREF clr, float h, float s, float v);
	static void				Activate							();
	static void				setAeroMargins						();
	static void				updateAeroState						();
	static void				setupAGGItemContext					(TStatusItem* item);

	static int 				ID_EXTBK_LAST;
	static TStatusItem*		statusItems;
	static TImageItem*		imageItems;
	static TImageItem*		glyphItem;

	static TSkinMetrics		metrics;
	static TSkinSettings	settings;
};

/*
 * the skin loader class is a helper for reading and writing skin definition files
 */
class SkinLoader
{
public:
	SkinLoader(const wchar_t* wszFilename);
	~SkinLoader() {};

public:
	bool							isValid						() const { return(m_isValid); }
	HRESULT 						Load						();
	DWORD 							readInt						(const char* szSection, const char* szValue, DWORD dwDefault = 0);
	HRESULT 						loadItems					();
	void 							readButtonItem				(const char *itemName, const char *file);
	void 							readImageItem				(const char *itemname, const char *szFileName);
	void 							createImageItem				(TImageItem *item, const char *fileName, HDC hdc);
	void 							loadFonts					();
	void 							loadBaseItems				();
	void 							readItem					(TStatusItem *this_item, const char *szItem);

	static UINT 					nextButtonID;

private:
	bool							m_isValid;
	char							m_szFilename[MAX_PATH];
	wchar_t							m_wszFilename[MAX_PATH];
};

#endif /* __SKIN_H_ */
