/*
Miranda Crash Dumper Plugin
Copyright (C) 2008 - 2012 Boris Krasnovskiy All Rights Reserved

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

#ifndef M_CRASHDUMPER_H
#define M_CRASHDUMPER_H

/*Returns a string containing the versioninfo post
wParam - (BOOL) suppress forum style formatting. If true the post won't have forum style formatting even if the option is checked in miranda's options.
lParam - (char **) Pointer to a string that receives the info. Memory is allocated using miranda's version of malloc() and you need to use miranda's version of free() on it.
Returns 0 on success.

how to use:
{
//...
	char *data;
	if (GetInfoService(TRUE, (LPARAM) &data) == 0)
		{//success
		}
	return 0;
}

*/
#define MS_CRASHDUMPER_GETINFO "CrashDmp/GetInfo"

#endif //M_CRASHDUMPER_H
