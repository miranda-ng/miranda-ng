/*
Miranda SmileyAdd Plugin
Copyright (C) 2005 - 2011 Boris Krasnovskiy
Copyright (C) 2003 - 2004 Rein-Peter de Boer

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

#ifndef SMILEYADD_SMLTOOL_H_
#define SMILEYADD_SMLTOOL_H_

class SmileyPackType;

struct SmileyToolWindowParam
{
	SmileyPackType* pSmileyPack;
	int xPosition;
	int yPosition;
	int direction;
	HWND hWndTarget;
	HWND hWndParent;
	UINT targetMessage;
	WPARAM targetWParam;
	MCONTACT hContact;
};


void __cdecl SmileyToolThread(void *arg);

#ifndef min
#define min(A, B) ((A) < (B) ? (A) : (B)) 
#endif

#endif // SMILEYADD_SMLTOOL_H_
