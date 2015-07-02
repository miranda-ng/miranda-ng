#ifndef __ROWHEIGHT_FUNCS_H__
# define __ROWHEIGHT_FUNCS_H__

#define ROW_SPACE_BEETWEEN_LINES 0
#define ICON_HEIGHT 16

class RowHeight
{
public:
	static BOOL Alloc(ClcData *dat, int size);
	static BOOL	Init(ClcData *dat);
	static void	Free(ClcData *dat);
	static void	Clear(ClcData *dat);

	// Calc and store max row height
	static int getMaxRowHeight(ClcData *dat, const HWND hwnd);

	// Calc and store row height
	static int getRowHeight(ClcData *dat, ClcContact* contact, int item, DWORD style)
	{
		if (!Alloc(dat, item + 1))
			return -1;

		int height = dat->fontInfo[GetBasicFontID(contact)].fontHeight;

		if (!dat->bisEmbedded) {
			if (contact->bSecondLine != MULTIROW_NEVER && contact->bSecondLine != MULTIROW_IFSPACE && contact->type == CLCIT_CONTACT) {
				if ((contact->bSecondLine == MULTIROW_ALWAYS || ((cfg::dat.dwFlags & CLUI_FRAME_SHOWSTATUSMSG && contact->bSecondLine == MULTIROW_IFNEEDED) && (contact->xStatus > 0 || contact->pExtra->bStatusMsgValid > STATUSMSG_XSTATUSID))))
					height += (dat->fontInfo[FONTID_STATUS].fontHeight + cfg::dat.avatarPadding);
			}

			// Avatar size
			if (contact->cFlags & ECF_AVATAR && contact->type == CLCIT_CONTACT && contact->ace != NULL && !(contact->ace->dwFlags & AVS_HIDEONCLIST))
				height = max(height, cfg::dat.avatarSize + cfg::dat.avatarPadding);
		}

		// Checkbox size
		if ((style & CLS_CHECKBOXES && contact->type == CLCIT_CONTACT) ||
			(style & CLS_GROUPCHECKBOXES && contact->type == CLCIT_GROUP) ||
			(contact->type == CLCIT_INFO && contact->flags & CLCIIF_CHECKBOX)) {
			height = max(height, dat->checkboxSize);
		}

		//height += 2 * dat->row_border;
		// Min size
		height = max(height, contact->type == CLCIT_GROUP ? dat->group_row_height : dat->min_row_heigh);
		height += cfg::dat.bRowSpacing;

		dat->row_heights[item] = height;
		//contact->iRowHeight = item;

		return height;
	}

	// Calc and store row height for all itens in the list
	static void				calcRowHeights		(ClcData *dat, HWND hwnd);

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
};

#endif // __ROWHEIGHT_FUNCS_H__
