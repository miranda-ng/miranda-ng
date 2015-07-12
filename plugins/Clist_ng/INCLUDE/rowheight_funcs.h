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
* $Id: rowheight_funcs.h 133 2010-09-30 06:27:18Z silvercircle $
*
* dynamic row height calculation for contact row. heavily used when
* painting a clc control with dynamic row heights.
*
*/

#ifndef __ROWHEIGHT_FUNCS_H__
#define __ROWHEIGHT_FUNCS_H__

#define ROW_SPACE_BEETWEEN_LINES 0
#define ICON_HEIGHT 16

class RowHeight {

public:
	static BOOL				Alloc(ClcData *dat, int size);
	static BOOL				Init(ClcData *dat);
	static void				Free(ClcData *dat);
	static void				Clear(ClcData *dat);

	// Calc and store max row height
	static int 				getMaxRowHeight(ClcData *dat, const HWND hwnd);

	// Calc and store row height
	static inline int getRowHeight(ClcData *dat, ClcContact* contact, int item, DWORD style, CLCPaintHelper* ph)
	{
		int 	height = 0;
		BYTE	secondLine;
		BOOL	fAvatar;

		if (!Alloc(dat, item + 1))
			return -1;

	    height = dat->fontInfo[GetBasicFontID(contact)].fontHeight;

	    ph->fAvatar = ph->fSecondLine = false;

	    if(!dat->bisEmbedded) {
	    	secondLine = contact->bSecondLineLocal;
	    	if(255 == secondLine)
	    		secondLine = ph->dsp ? (ph->dsp->bSecondLine == 0 ? contact->bSecondLine : (ph->dsp->bSecondLine == 1 ? MULTIROW_ALWAYS : MULTIROW_NEVER)) : contact->bSecondLine;
	        if(secondLine != MULTIROW_NEVER && contact->type == CLCIT_CONTACT) {
	            if ((secondLine == MULTIROW_ALWAYS || ((cfg::dat.dwFlags & CLUI_FRAME_SHOWSTATUSMSG && secondLine == MULTIROW_IFNEEDED) && (contact->xStatus > 0 || cfg::eCache[contact->extraCacheEntry].bStatusMsgValid > STATUSMSG_XSTATUSID)))) {
	                height += (dat->fontInfo[FONTID_STATUS].fontHeight + cfg::dat.avatarPadding);
	                ph->fSecondLine = true;
	            }
	        }

	        // Avatar size
	        if(!(contact->dwDFlags & (ECF_HIDEAVATAR | ECF_FORCEAVATAR))) {					// no per contact show/hide, consider dsp override
	        	if(ph->dsp)
	        		fAvatar = (ph->dsp->bAvatar == 0 ? contact->cFlags & ECF_AVATAR : (ph->dsp->bAvatar == -1 ? FALSE : TRUE));
	        	else
	        		fAvatar = contact->cFlags & ECF_AVATAR;
		        if (fAvatar && contact->type == CLCIT_CONTACT && contact->ace != NULL) {
		            height = max(height, cfg::dat.avatarSize + cfg::dat.avatarPadding);
		            ph->fAvatar = true;
		        }
	        }
	        else {
		        if (contact->cFlags & ECF_AVATAR && contact->type == CLCIT_CONTACT && contact->ace != NULL) {
		            height = max(height, cfg::dat.avatarSize + cfg::dat.avatarPadding);
		            ph->fAvatar = true;
		        }
	        }
	    }

	    // Checkbox size
	    if((style&CLS_CHECKBOXES && contact->type==CLCIT_CONTACT) ||
	        (style&CLS_GROUPCHECKBOXES && contact->type==CLCIT_GROUP) ||
	        (contact->type==CLCIT_INFO && contact->flags&CLCIIF_CHECKBOX))
	    {
	        height = max(height, dat->checkboxSize);
	    }

	    //height += 2 * dat->row_border;
	    // Min size
	    height = max(height, contact->type == CLCIT_GROUP ? dat->group_row_height : dat->min_row_heigh);
	    height += cfg::dat.bRowSpacing;

		dat->row_heights[item] = height;

		return height;
	}

	// Calc and store row height for all itens in the list
	static void				calcRowHeights		(ClcData *dat, HWND hwnd, CLCPaintHelper* ph);

	// Calc item top Y (using stored data)
	static int 				getItemTopY			(ClcData *dat, int item);

	// Calc item bottom Y (using stored data)
	static int 				getItemBottomY		(ClcData *dat, int item);

	// Calc total height of rows (using stored data)
	static int 				getTotalHeight		(ClcData *dat);

	// Return the line that pos_y is at or -1 (using stored data). Y start at 0
	static int 				hitTest				(ClcData *dat, int pos_y);

	// Returns the height of the chosen row
	static int 				getHeight			(ClcData *dat, int item);

	// returns the height for a floating contact
	static int 				getFloatingRowHeight(const ClcData *dat, HWND hwnd, ClcContact *contact, DWORD dwFlags);
};

#endif // __ROWHEIGHT_FUNCS_H__
