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

#ifndef _SMILEYROUTINES_
#define _SMILEYROUTINES_

struct ReplaceSmileyType
{
	CHARRANGE loc;
	SmileyType *sml;
	SmileyCType *smlc;
	bool ldspace;
	bool trspace;
};

// Queue to store smileys found
typedef SMOBJLIST<ReplaceSmileyType> SmileysQueueType;

void LookupAllSmileys(SmileyPackType *smileyPack, SmileyPackCType *smileyCPack, const wchar_t *lpstrText, SmileysQueueType &smllist, bool firstOnly);
void ReplaceSmileys(HWND hwnd, SmileyPackType *smp, SmileyPackCType *smcp, const CHARRANGE &sel, bool useHidden, bool ignoreLast, bool unFreeze, bool fireView = 0);
void ReplaceSmileysWithText(HWND hwnd, CHARRANGE &sel, bool keepFrozen);
SmileyType* FindButtonSmiley(SmileyPackType *smp);

#endif
