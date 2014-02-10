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

#ifndef H_WWI_DATE_UTILS_H
#define H_WWI_DATE_UTILS_H

#include "errno.h"

#define SAVE_MODE_STANDARD  0
#define SAVE_MODE_PROTOCOL  1
#define SAVE_MODE_MBIRTHDAY 2

#define DOB_UNKNOWN							100
#define DOB_USERINFO						101
#define DOB_MBIRTHDAY						102
#define DOB_PROTOCOL						103
#define DOB_BIRTHDAYREMINDER                104
#define DOB_MICQBIRTHDAY                    105

#define SAVE_MODE_DELETEALL 100

time_t Today();
bool IsLeapYear(int year);
bool IsDOBValid(int year, int month, int day);

unsigned int DaysToBirthday(time_t now, int ctYear, int ctMonth, int ctDay);
int DaysAfterBirthday(time_t now, int ctYear, int ctMonth, int ctDay);
int GetContactAge(MCONTACT hContact);
int GetContactDOB(MCONTACT hContact, int &year, int &month, int &day);
char GetContactGender(MCONTACT hContact);

int SaveBirthday(MCONTACT hContact, int year, int month, int day, int mode);

#endif //H_WWI_DATE_UTILS_H