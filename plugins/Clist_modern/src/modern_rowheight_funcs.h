/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (�) 2012-15 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-08 Miranda ICQ/IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

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

Created by Pescuma
*/

#pragma once

#ifndef __ROWHEIGHT_FUNCS_H__
# define __ROWHEIGHT_FUNCS_H__

#include "modern_clc.h"

#define ROW_SPACE_BEETWEEN_LINES 2
#define ICON_HEIGHT 16
#define ICON_WIDTH	16



BOOL RowHeights_Initialize(ClcData *dat);
void RowHeights_Free(ClcData *dat);
void RowHeights_Clear(ClcData *dat);

BOOL RowHeights_Alloc(ClcData *dat, int size);

// Calc and store max row height
int RowHeights_GetMaxRowHeight(ClcData *dat, HWND hwnd);

// Calc and store row height
int RowHeights_GetRowHeight(ClcData *dat, HWND hwnd, ClcContact *contact, int item);

// Calc and store row height for all itens in the list
void RowHeights_CalcRowHeights(ClcData *dat, HWND hwnd);

// Calc item top Y (using stored data)
int cliGetRowTopY(ClcData *dat, int item);

// Calc item bottom Y (using stored data)
int cliGetRowBottomY(ClcData *dat, int item);

// Calc total height of rows (using stored data)
int cliGetRowTotalHeight(ClcData *dat);

// Return the line that pos_y is at or -1 (using stored data). Y start at 0
int cliRowHitTest(ClcData *dat, int pos_y);

// Returns the height of the chosen row
int cliGetRowHeight(ClcData *dat, int item);

#endif // __ROWHEIGHT_FUNCS_H__
