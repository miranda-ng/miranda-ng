/*
UserinfoEx plugin for Miranda IM

Copyright:
© 2006-2010 DeathAxe, Yasnovidyashii, Merlin, K. Romanov, Kreol

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

/**
 * system & local includes:
 **/

#include "..\commonheaders.h"

/***********************************************************************************************************
 * exporting stuff
 ***********************************************************************************************************/

/**
 * name:	ExportModule
 * desc:	write all settings from a database module to file
 * param:	hContact	- handle of contact the module is owned from
 *			pszModule	- name of the module to save
 *			file		- file to write the settings to
 * return	nothing
 **/
static void ExportModule(MCONTACT hContact, LPCSTR pszModule, FILE* file)
{
	DB::CEnumList	Settings;

	if (!Settings.EnumSettings(hContact, pszModule))
	{
		DBVARIANT dbv;
		LPSTR here;
		WORD j;
		int i;
		LPSTR pszSetting;
		//char tmp[32];

		// print the module header..
		fprintf(file, "\n[%s]\n", pszModule);

		for (i = 0; i < Settings.getCount(); i++)
		{
			pszSetting = Settings[i];

			if (!DB::Setting::GetAsIs(hContact, pszModule, pszSetting, &dbv))
			{
				switch (dbv.type) 
				{
					case DBVT_BYTE:
						{
							fprintf(file, "%s=b%u\n", pszSetting, dbv.bVal);
						}
						break;

					case DBVT_WORD:
						{
							fprintf(file, "%s=w%u\n", pszSetting, dbv.wVal);
						}
						break;

					case DBVT_DWORD:
						{
							fprintf(file, "%s=d%u\n", pszSetting, dbv.dVal);
						}
						break;

					case DBVT_ASCIIZ:
					case DBVT_UTF8:
						{
							for (here = dbv.pszVal; here && *here; here++) 
							{
								switch (*here) {
									// convert \r to STX
									case '\r':
										*here = 2;
										break;

									// convert \n to ETX
									case '\n':
										*here = 3;
								}
							}
							if (dbv.type == DBVT_UTF8) 
								fprintf(file, "%s=u%s\n", pszSetting, dbv.pszVal);
							else
								fprintf(file, "%s=s%s\n", pszSetting, dbv.pszVal);
						}
						break;

					case DBVT_BLOB:
						{
							fprintf(file, "%s=n", pszSetting);
							for (j = 0; j < dbv.cpbVal; j++)
							{
								fprintf(file, "%02X ", (BYTE)dbv.pbVal[j]);
							}
							fputc('\n', file);
						}
						break;
				}
				db_free(&dbv);
			}
		}
	}
}

/**
 * name:	ExportContact
 * desc:	Exports a certain contact to an ini file.
 * param:	hContact	- contact to export or -1 to export all contacts
 *			pModules	- module to export, NULL to export all modules of a contact
 *			file		- ini file to write the contact to
 **/
static BYTE ExportContact(MCONTACT hContact, DB::CEnumList* pModules, FILE* file)
{
	CExImContactBase vcc;

	if (pModules) 
	{
		if ((vcc = hContact) >= NULL)
		{
			int i;
			LPSTR p;

			vcc.toIni(file, pModules->getCount()-1);

			for (i = 0; i < pModules->getCount(); i++)
			{
				p = (*pModules)[i];

				/*Filter/
				if (mir_stricmp(p, "Protocol"))*/
				{
					ExportModule(hContact, p, file);
				}
			}
			return TRUE;
		}
	}
	return FALSE;
}

/**
 * name:	SvcExImINI_Export
 * desc:	Exports a certain contact or all contacts to an ini file.
 * param:	hContact	- contact to export or -1 to export all contacts
 *			pszFileName	- ini-filename to write the contact to
 **/
int SvcExImINI_Export(lpExImParam ExImContact, LPCSTR pszFileName)
{
	FILE* file;
	errno_t err;
	DB::CEnumList Modules;
	SYSTEMTIME now;
	MCONTACT hContact;

	if (!DlgExImModules_SelectModulesToExport(ExImContact, &Modules, NULL))
	{
		if ((err = fopen_s(&file, pszFileName, "wt")) != NULL)
		{
			MsgErr(NULL, 
				LPGENT("The ini-file \"%s\"\nfor saving contact information could not be opened."),
				pszFileName);
			return 1;
		}
		
		SetCursor(LoadCursor(NULL, IDC_WAIT));

		// write header
		GetLocalTime(&now);
		fprintf(file, 
			";DATE = %04d-%02d-%02d %02d:%02d:%02d\n\n",
			now.wYear, now.wMonth, now.wDay, now.wHour, now.wMinute, now.wSecond
		);

		if (Modules.getCount() == 0)
		{
			Modules.EnumModules();
		}
		
		//	hContact == -1 export entire db.
		if (ExImContact->Typ != EXIM_CONTACT)
		{
			// Owner
			ExportContact(NULL, &Modules, file);
			fprintf(file, "\n\n");
			// Contacts
			for (hContact = db_find_first(); hContact != NULL; hContact = db_find_next(hContact))
			{
				ExportContact(hContact, &Modules, file);
				fprintf(file, "\n\n");
			}
		}
		// export only one contact
		else 
		{
			ExportContact(ExImContact->hContact, &Modules, file);
		}

		fclose(file);
		SetCursor(LoadCursor(NULL, IDC_ARROW));
	}
	return 0;
}

/***********************************************************************************************************
 * importing stuff
 ***********************************************************************************************************/

LPSTR strnrchr(LPSTR string, int ch, DWORD len)
{
	LPSTR start = (LPSTR)string;

	string += len;		/* find end of string */
						/* search towards front */
	while (--string != start && *string != (CHAR)ch);
		if (*string == (CHAR)ch)		/* char found ? */
			return ((LPSTR)string);
	return(NULL);
}

/**
 * name:	ImportreadLine
 * desc:	read exactly one line into a buffer and return its pointer. Size of buffer is managed.
 * param:	file	- pointer to a file
 *			string	- the string to write the read line to
 * return:	pointer to the buffer on success or NULL on error
 **/
static DWORD ImportreadLine(FILE* file, LPSTR &str)
{
	CHAR c;
	DWORD l = 0;
	BYTE bComment = 0;

	str[0] = 0;
	while (!feof(file)) {
		switch (c = fgetc(file)) {
			case EOF:
				// reading error
				if (ferror(file)) {
					MIR_FREE(str);
					return 0;
				}
				// end of line & file
				return l;

			case '\r':
			case '\n':
				// ignore empty lines
				if (l == 0) { 
					bComment = 0;
					continue;
				}
				return l;
			
			case ';':
				// found a comment line
				bComment |= l == 0;
			case '\t':
			case ' ':
				// ignore space and tab at the beginning of the line
				if (l == 0) break;
			
			default:
				if (!bComment) {
					str = mir_strncat_c(str, c);
					l++;
				}
				break;
		}
	}
	return 0;
}

/**
 * name:	ImportFindContact
 * desc:	This function decodes the given line, which is already identified to be a contact line.
 *			The resulting information is matcht to the given hContact if it isn't NULL.
 *			Otherwise all existing contacts are matched.
 * param:	hContact	- handle to contact to match or NULL to match all existing
 *			pszBuf		- pointer to the buffer holding the string of the current line in the ini.-file
 *			cchBuf		- character count of the buffer
 * return:	handle to the contact that matches the information or NULL if no match
 **/
static MCONTACT ImportFindContact(MCONTACT hContact, LPSTR &strBuf, BYTE bCanCreate)
{
	CExImContactBase vcc;

	vcc.fromIni(strBuf);
	if (vcc.handle() != INVALID_CONTACT_ID) {
		//if (vcc.isHandle(hContact))
		//	return hContact;
		return vcc.handle();
	}
	else if (bCanCreate)
		return vcc.toDB();

	return vcc.handle();
}

/**
 * name:	ImportSetting
 * desc:	This function writes a line identified as a setting to the database
 * param:	hContact	- handle to contact to match or NULL to match all existing
 *			pszModule	- module to write the setting to
 *			strLine		- string with the setting and its value to write to db
 * return:	0 if writing was ok, 1 otherwise
 **/
int ImportSetting(MCONTACT hContact, LPCSTR pszModule, LPSTR &strLine)
{
	DBVARIANT dbv;
	LPSTR end, value;
	size_t numLines = 0;
	size_t brk;
	LPSTR pszLine = strLine;

	// check Module and filter "Protocol"
	if (!pszModule || !*pszModule || mir_strncmp(pszModule,"Protocol",8) == 0)
		return 1;
	if ((end = value = mir_strchr(pszLine, '=')) == NULL)
		return 1;

	// truncate setting string if it has spaces at the end
	do {
		if (end == pszLine)
			return 1;
		*(end--) = 0;
	}
		while (*end == '\t' || *end == ' ' || *end < 27);

	// skip spaces from the beginning of the value
	do {
		value++;
		// if the value is empty, delete it from db
		if (*value == '\0')
			return db_unset(hContact, pszModule, pszLine);
	} while (*value == '\t' || *value == ' ');

	// decode database type and value
	switch (*(value++)) {
		case 'b':
		case 'B':
			if (brk = strspn(value, "0123456789-"))
				*(value + brk) = 0;
			dbv.type = DBVT_BYTE;
			dbv.bVal = (BYTE)atoi(value);
			break;
		case 'w':
		case 'W':
			if (brk = strspn(value, "0123456789-"))
				*(value + brk) = 0;
			dbv.type = DBVT_WORD;
			dbv.wVal = (WORD)atoi(value);
			break;
		case 'd':
		case 'D':
			if (brk = strspn(value, "0123456789-"))
				*(value + brk) = 0;
			dbv.type = DBVT_DWORD;
			dbv.dVal = (DWORD)_atoi64(value);
			break;
		case 's':
		case 'S':
		case 'u':
		case 'U':
			for (end = value; end && *end; end++) {
				switch (*end) {
					// convert STX back to \r
					case 2:
						*end = '\r';
						break;
					// convert ETX back to \n
					case 3:
						*end = '\n';
						break;
				}
			}
			switch (*(value - 1)) {
				case 's':
				case 'S':
					dbv.type = DBVT_ASCIIZ;
					dbv.pszVal = value;
					break;
				case 'u':
				case 'U':
					dbv.type = DBVT_UTF8;
					dbv.pszVal = value;
					break;
			}
			break;
		case 'n':
		case 'N':
		{
			PBYTE dest;
			dbv.type = DBVT_BLOB;
			dbv.cpbVal = (WORD)mir_strlen(value) / 3;
			dbv.pbVal = (PBYTE)value;
			for ( dest = dbv.pbVal, value = strtok(value, " ");
					value && *value;
					value = strtok(NULL, " "))
				*(dest++) = (BYTE)strtol(value, NULL, 16);
			*dest = 0;
			break;
		}
		default:
			dbv.type = DBVT_DELETED;
			//return 1;
	}
	return db_set(hContact, pszModule, pszLine, &dbv);
}

/**
 * name:	Import
 * desc:	This function imports an ini file
 * param:	hContact	- handle to contact to match or NULL to match all existing
 *			file		- module to write the setting to
 *			strLine		- string with the setting and its value to write to db
 * return:	0 if writing was ok, 1 otherwise
 **/
int SvcExImINI_Import(MCONTACT hContact, LPCSTR pszFileName)
{
	FILE	*file;
	MCONTACT hNewContact = INVALID_CONTACT_ID;
	DWORD	end,
			numLines				= 0;
	CHAR	szModule[MAXSETTING]	= {0};
	WORD	numContactsInFile		= 0,		// number of contacts in the inifile
			numContactsAdded		= 0;		// number of contacts, that were added to the database
	CHAR	*strBuf					= (CHAR *) mir_alloc(1);
			*strBuf					= 0;
	
	if (file = fopen(pszFileName, "rt")) {
		SetCursor(LoadCursor(NULL, IDC_WAIT));

		while (ImportreadLine(file, strBuf)) {
			numLines++;

			// contact was found and imported
			if (hContact != INVALID_CONTACT_ID && hNewContact != INVALID_CONTACT_ID)
				break;

			// importing settings is only valid vor the main menu item
			if (hContact == INVALID_CONTACT_ID) {
				if (!strncmp(strBuf, "SETTINGS:", 9)) {
					*szModule = 0;
					hNewContact = NULL;
					continue;
				}
			}

			// there are some modules of a contact (import only if contact exist)
			if (!strncmp(strBuf, "FROM CONTACT:", 13)) {
				strBuf = mir_strnerase(strBuf, 0, 13);
				while (strBuf[0] == ' ' || strBuf[0] == '\t')
					strBuf = mir_strnerase(strBuf, 0, 1);

				numContactsInFile++;
				if ((hNewContact = ImportFindContact(hContact, strBuf, FALSE)) != INVALID_CONTACT_ID)
					numContactsAdded++;
				continue;
			}

			// there is a contact to import / add
			if (!strncmp(strBuf, "CONTACT:", 8)) {
				strBuf = mir_strnerase(strBuf, 0, 8);
				while (strBuf[0] == ' ' || strBuf[0] == '\t')
					strBuf = mir_strnerase(strBuf, 0, 1);

				*szModule = 0;
				numContactsInFile++;
				if ((hNewContact = ImportFindContact(hContact, strBuf, TRUE)) != INVALID_CONTACT_ID)
					numContactsAdded++;
				continue;
			}

			// read modules and settings only for valid contacts
			if (hNewContact != INVALID_CONTACT_ID) {
				// found a module line
				if (strBuf[0] == '[' && (end = (strchr(strBuf, ']') - strBuf)) > 0) {
					mir_strncpy(szModule, &strBuf[1], end);
					continue;
				}
				// try to import a setting
				ImportSetting(hNewContact, szModule, strBuf);
			}
		} //end while
		fclose(file);
		mir_free(strBuf);
		SetCursor(LoadCursor(NULL, IDC_ARROW));

		// the contact was not found in the file
		if (numContactsInFile > 0 && !numContactsAdded) {
			MsgErr(NULL,
				LPGENT("None of the %d contacts, stored in the ini-file, match the selected contact!\nNothing will be imported"),
				numContactsInFile);
		}
		// Import complete
		else{
			MsgBox(NULL, MB_ICON_INFO, LPGENT("Import complete"), LPGENT("Some basic statistics"),
				LPGENT("Added %d of %d contacts stored in the ini-file."),
				numContactsAdded, numContactsInFile);
		}
		return 0;
	}
	MsgErr(NULL, 
		LPGENT("The ini-file \"%s\"\nfor reading contact information could not be opened."),
		pszFileName);
	return 1;
}
