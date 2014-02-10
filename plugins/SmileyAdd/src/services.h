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

#ifndef SMILEYADD_SERVICES_H_
#define SMILEYADD_SERVICES_H_

// service commands
INT_PTR ReplaceSmileysCommand(WPARAM wParam, LPARAM lParam);
INT_PTR GetSmileyIconCommand(WPARAM wParam, LPARAM lParam);
INT_PTR ShowSmileySelectionCommand(WPARAM wParam, LPARAM lParam);
INT_PTR GetInfoCommand(WPARAM wParam, LPARAM);
INT_PTR GetInfoCommand2(WPARAM wParam, LPARAM);
INT_PTR ParseText(WPARAM wParam, LPARAM lParam);
INT_PTR ParseTextW(WPARAM wParam, LPARAM lParam);
INT_PTR RegisterPack(WPARAM wParam, LPARAM lParam);
INT_PTR ParseTextBatch(WPARAM wParam, LPARAM lParam);
INT_PTR FreeTextBatch(WPARAM wParam, LPARAM lParam);
INT_PTR CustomCatMenu(WPARAM, LPARAM lParam);
int RebuildContactMenu(WPARAM wParam, LPARAM);
INT_PTR ReloadPack(WPARAM, LPARAM lParam);
INT_PTR LoadContactSmileys(WPARAM, LPARAM lParam);
int AccountListChanged(WPARAM wParam, LPARAM lParam);
int DbSettingChanged(WPARAM wParam, LPARAM lParam);

SmileyPackType* GetSmileyPack(const char* proto, MCONTACT hContact = NULL, SmileyPackCType** smlc = NULL);


#endif // SMILEYADD_SERVICES_H_

