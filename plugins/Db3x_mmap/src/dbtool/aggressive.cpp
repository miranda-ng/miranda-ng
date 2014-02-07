/*
Miranda Database Tool
Copyright (C) 2001-2005  Richard Hughes

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
#include "..\commonheaders.h"

#define BLOCKSIZE  65536

int CDb3Mmap::WorkAggressive(int firstTime)
{
	if (firstTime) {
		if (!cb->bAggressive)
			return ERROR_NO_MORE_ITEMS;

		cb->pfnAddLogMessage(STATUS_MESSAGE, TranslateT("Performing aggressive pass"));
		ofsAggrCur = 0;
		cb->spaceProcessed = 0;
	}

	int blockBytes = min(BLOCKSIZE + 3, (int)(sourceFileSize - ofsAggrCur));
	if (blockBytes <= 0)
		return ERROR_NO_MORE_ITEMS;

	BYTE *buf = m_pDbCache + ofsAggrCur;
	blockBytes -= 3;
	for (int i = 0; i < blockBytes; i++) {
		if (buf[i]) {
			if ((*(PDWORD)&buf[i] & 0x00FFFFFF) != 0xDECADE)
				cb->pfnAddLogMessage(STATUS_WARNING, TranslateT("Aggressive: random junk at %08X: skipping"), ofsAggrCur + i);
			else
				//TODO: give user the option of placing manually
				cb->pfnAddLogMessage(STATUS_ERROR, TranslateT("Aggressive: unlinked data at %08X: can't automatically place"), ofsAggrCur + i);

			for (; i < blockBytes; i++)
				if (buf[i] == 0) { i--; break; }
		}
	}
	ofsAggrCur += BLOCKSIZE;
	cb->spaceProcessed = ofsAggrCur;
	return ERROR_SUCCESS;
}
