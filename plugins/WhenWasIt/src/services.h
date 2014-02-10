/*
WhenWasIt (birthday reminder) plugin for Miranda IM

Copyright © 2006 Cristian Libotean

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

#ifndef M_WWI_SERVICES_H
#define M_WWI_SERVICES_H

#define MS_WWI_CHECK_BIRTHDAYS "WhenWasIt/Birthdays/Check"
#define MS_WWI_LIST_SHOW "WhenWasIt/List/Show"
#define MS_WWI_ADD_BIRTHDAY "WhenWasIt/Birthdays/Add"
#define MS_WWI_REFRESH_USERDETAILS "WhenWasIt/Birthdays/RefreshDetails"
#define MS_WWI_IMPORT_BIRTHDAYS "WhenWasIt/Birthdays/Import"
#define MS_WWI_EXPORT_BIRTHDAYS "WhenWasIt/Birthdays/Export"

#define REFRESH_DETAILS_DELAY 3000


#define BIRTHDAY_EXTENSION ".bdaysW"
#define BIRTHDAYS_EXPORT_FORMAT "%s@%S : %02d/%02d/%04d\n"
#define NOTFOUND_FORMAT LPGEN("Could not find UID '%s [%S]' in current database, skipping")


extern int bShouldCheckBirthdays;
extern int bBirthdayFound;

int InitServices();
int DestroyServices();

int NotifyContactBirthday(MCONTACT hContact, time_t now, int daysInAdvance);
int NotifyMissedContactBirthday(MCONTACT hContact, time_t now, int daysAfter);

int DoExport(TCHAR *fileName);
int DoImport(TCHAR *fileName);

INT_PTR CheckBirthdaysService(WPARAM wParam, LPARAM lParam);
INT_PTR ShowListService(WPARAM wParam, LPARAM lParam);
INT_PTR AddBirthdayService(WPARAM wParam, LPARAM lParam);
INT_PTR RefreshUserDetailsService(WPARAM wParam, LPARAM lParam);
INT_PTR ImportBirthdaysService(WPARAM wParam, LPARAM lParam);
INT_PTR ExportBirthdaysService(WPARAM wParam, LPARAM lParam);

#endif //M_EXCHANGE_SERVICES_H