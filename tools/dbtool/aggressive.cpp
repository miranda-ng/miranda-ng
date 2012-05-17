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
#include "dbtool.h"

#define BLOCKSIZE  65536

extern DWORD sourceFileSize;
extern DWORD spaceProcessed;
extern DWORD sp;
static DWORD ofsCurrent;

int WorkAggressive(int firstTime)
{
	int blockBytes,i;
	BYTE *buf;

	if(firstTime) {
		if(!opts.bAggressive) return ERROR_NO_MORE_ITEMS;
		AddToStatus(STATUS_MESSAGE,TranslateT("Performing aggressive pass"));
		ofsCurrent=0;
		spaceProcessed=0;
		sp=0;
	}
	blockBytes=min(BLOCKSIZE+3,(int)(sourceFileSize-ofsCurrent));
	if(blockBytes<=0) return ERROR_NO_MORE_ITEMS;
	buf = opts.pFile+ofsCurrent;
	blockBytes-=3;
	for(i=0;i<blockBytes;i++) {
		if(buf[i]) {
			if((*(PDWORD)&buf[i]&0x00FFFFFF)!=0xDECADE)
				AddToStatus(STATUS_WARNING,TranslateT("Aggressive: random junk at %08X: skipping"),ofsCurrent+i);
			else {
				//TODO: give user the option of placing manually
				AddToStatus(STATUS_ERROR,TranslateT("Aggressive: unlinked data at %08X: can't automatically place"),ofsCurrent+i);
			}
			for(;i<blockBytes;i++)
				if(buf[i]==0) {i--; break;}
		}
	}
	ofsCurrent+=BLOCKSIZE;
	spaceProcessed=ofsCurrent;
	return ERROR_SUCCESS;
}
