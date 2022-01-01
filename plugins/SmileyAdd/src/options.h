/*
Miranda NG SmileyAdd Plugin
Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org)
Copyright (C) 2005-11 Boris Krasnovskiy All Rights Reserved
Copyright (C) 2003-04 Rein-Peter de Boer

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

#ifndef SMILEYADD_OPTIONS_H_
#define SMILEYADD_OPTIONS_H_

int SmileysOptionsInitialize(WPARAM wParam, LPARAM lParam);

class OptionsType
{
public:
	bool EnforceSpaces;
	bool ScaleToTextheight;
	bool UseOneForAll;
	bool UsePhysProto;
	bool SurroundSmileyWithSpaces;
	bool ScaleAllSmileys;
	bool IEViewStyle;
	bool AnimateSel;
	bool AnimateDlg;
	bool InputSmileys;
	bool DCursorSmiley;
	bool HQScaling;
	bool HorizontalSorting;
	unsigned SelWndBkgClr;
	unsigned MaxCustomSmileySize;
    unsigned MinSmileySize;

	void Load(void);
	void Save(void);
	static void ReadPackFileName(CMStringW &filename, const CMStringW &name, const CMStringW &defaultFilename);
	static void ReadCustomCategories(CMStringW &cats);
	static void ReadContactCategory(MCONTACT hContact, CMStringW &cats);
	static void WritePackFileName(const CMStringW &filename, const CMStringW &name);
	static void WriteCustomCategories(const CMStringW &cats);
	static void WriteContactCategory(MCONTACT hContact, const CMStringW &cats);
};

extern OptionsType opt;

#endif // SMILEYADD_OPTIONS_H_

