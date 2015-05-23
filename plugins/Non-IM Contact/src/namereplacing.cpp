#include "commonheaders.h"

int readFileIntoArray(int fileNumber, char *FileContents[])
{
	char dbSetting[20], temp[MAX_STRING_LENGTH];
	mir_snprintf(dbSetting, SIZEOF(dbSetting), "fn%d", fileNumber);

	char *szVar = db_get_sa(NULL, MODNAME, dbSetting);
	if (szVar == NULL)
		return 0;

	char tszFileName[MAX_PATH];
	if (!strncmp("http://", szVar, 7) || !strncmp("https://", szVar, 7))
		mir_snprintf(tszFileName, SIZEOF(tszFileName), "%s\\plugins\\fn%d.html", getMimDir(temp), fileNumber);
	else
		mir_strncpy(tszFileName, szVar, SIZEOF(tszFileName));
	mir_free(szVar);

	FILE* file = fopen(tszFileName, "r");
	if (file == NULL)
		return 0;

	// read the file into the FileContents array
	// free this array before stringReplacer() returns
	int i;
	for (i = 0; fgets(temp, MAX_STRING_LENGTH - 1, file); i++) {
		if (temp[mir_strlen(temp) - 1] == '\n')
			temp[mir_strlen(temp) - 1] = '\0';
		else temp[mir_strlen(temp)] = '\0';

		FileContents[i] = (char*)malloc(mir_strlen(temp) + 1);
		if (FileContents[i] == NULL) break;
		mir_strcpy(FileContents[i], temp);
	}
	fclose(file);
	return i;
}

int getNumber(const char* line)
{
	int i;
	return sscanf(line, "%d", &i) == 1 ? i : -1;
}

int findWordInString(const char* line, const char* string, int* lengthOfWord, int flag) /* flag = 0 %from, flag = 1 %until */
{
	unsigned int i, j = 0;
	char word[64] = "", OpenDivider[8], CloseDivider[8];
	strncpy(OpenDivider, "(\"", sizeof(OpenDivider));
	strncpy(CloseDivider, "\")", sizeof(CloseDivider));
	/* get the word we r looking for */
	if (!strncmp(string, OpenDivider, mir_strlen(OpenDivider))) {
		for (i = 2; strncmp(&string[i], CloseDivider, mir_strlen(CloseDivider)); i++) {
			word[j] = string[i];
			word[++j] = '\0';
		}
	}
	i = 0;
	*lengthOfWord = (int)(mir_strlen(word) + mir_strlen(CloseDivider) + mir_strlen(OpenDivider));
	/* find the word in the line */
	while (i < (mir_strlen(line) - mir_strlen(word))) {
		if (!strncmp(&line[i], word, mir_strlen(word))) {
			if (!flag) return i + (int)mir_strlen(word); /* the next char after the word */
			else return i; /* the char before the word */
		}
		i++;
	}
	return -1;
}

int findLine(char* FileContents[], const char* string, int linesInFile, int startLine, int *positionInOldString)
{
	char tmp[5];
	int i = getNumber(&string[*positionInOldString]);

	// check if blank
	if (string[*positionInOldString] == ')')
		return startLine;

	// check if its a number
	if (i != -1) {
		*positionInOldString += (int)mir_strlen(_itoa(i, tmp, 10)) - 1;
		return i;
	}

	// lastline
	if (!strncmp(&string[*positionInOldString], "lastline(", mir_strlen("lastline("))) {
		*positionInOldString += (int)mir_strlen("lastline(");
		i = getNumber(&string[*positionInOldString]);
		if (i != -1) {
			*positionInOldString += (int)mir_strlen(_itoa(i, tmp, 10));
			return linesInFile - (i + 1);
		}

		(*positionInOldString)++;
		return (linesInFile - 1);
	}

	// string
	if (string[*positionInOldString] == '\"') {
		char string2Find[256];
		int j = 0;
		// get the word to find
		for (i = (*positionInOldString + 1); strncmp(&string[i], "\")", 2); i++) {
			string2Find[j] = string[i];
			string2Find[++j] = '\0';
		}

		// find the word
		for (j = startLine; j < linesInFile; j++) {
			if (strstr(FileContents[j], string2Find)) {
				i = j;
				break;
			}
			i = -1;
		}
		*positionInOldString += (int)(mir_strlen(string2Find) + mir_strlen("\"\")"));
		if (i == -1) return i;
		// allow for a +- after the word to go up or down lines
		if (string[*positionInOldString] == '+') {
			*positionInOldString += 1;
			j = getNumber(&string[*positionInOldString]);
			if (j != -1) {
				*positionInOldString += (int)mir_strlen(_itoa(j, tmp, 10)) - 2;
				return i + j;
			}
		}
		else if (string[*positionInOldString] == '-') {
			*positionInOldString += 1;
			j = getNumber(&string[*positionInOldString]);
			if (j != -1) {
				*positionInOldString += (int)mir_strlen(_itoa(j, tmp, 10)) - 2;
				return i - j;
			}
		}
		else {
			*positionInOldString -= 2;
			return i;
		}
	}
	return -1;
}

int findChar(char* FileContents[], const char* string, int linesInFile, int startLine, int *positionInOldString, int startChar, int startEnd) // 0=start, 1=end for startEnd
{
	char tmp[5];
	int i = getNumber(&string[*positionInOldString]);
	// check if its a number
	if (i != -1) {
		*positionInOldString += (int)mir_strlen(_itoa(i, tmp, 10)) - 1;
		return i;
	}

	// string
	if (string[*positionInOldString] == '\"') {
		char string2Find[256];
		unsigned int j = 0;
		// get the word to find
		for (i = (*positionInOldString + 1); strncmp(&string[i], "\")", 2); i++) {
			string2Find[j] = string[i];
			string2Find[++j] = '\0';
		}
		// find the word
		for (j = 0; j < mir_strlen(FileContents[startLine]); j++)
			if (!strncmp(&FileContents[startLine][j], string2Find, mir_strlen(string2Find)))
				break;

		if (j == mir_strlen(FileContents[startLine]))
			return -1;

		*positionInOldString += (int)mir_strlen(string2Find) + 1;
		return (startEnd) ? j : j + (int)mir_strlen(string2Find);
	}

	// csv(
	if (!strncmp(&string[*positionInOldString], "csv(", mir_strlen("csv("))) {
		char seperator;
		int j = 0, k = startChar;
		*positionInOldString += (int)mir_strlen("csv(");
		if (!strncmp(&string[*positionInOldString], "tab", 3)) {
			*positionInOldString += 3;
			seperator = '\t';
		}
		else if (!strncmp(&string[*positionInOldString], "space", 5)) {
			*positionInOldString += 5;
			seperator = ' ';
		}
		else {
			seperator = string[*positionInOldString];
			*positionInOldString += 1;
		}
		i = getNumber(&string[*positionInOldString]);
		if (i == -1) return -1;
		*positionInOldString += (int)mir_strlen(_itoa(i, tmp, 10));
		while (j < i) {
			if (FileContents[startLine][k] == '\0') break;
			if (FileContents[startLine][k] == seperator)
				j++;
			k++;
		}
		return k;
	}
	return -1;
}

// do the compare("A","B","X","Y")
void checkStringForcompare(char *str)
{
	if (!strstr(str, "compare(\"")) return;
	char *A, *B, *X, *Y, *newStr = (char*)malloc(mir_strlen(str)), *copyOfStr = _strdup(str);
	unsigned int i, j = 0, s = (int)mir_strlen(str);
	newStr[0] = '\0';
	for (i = 0; i < s; i++) {
		if (!strncmp(&str[i], "compare(\"", mir_strlen("compare(\""))) {
			i += (int)mir_strlen("compare(\"");
			A = strtok(&copyOfStr[i], "\",\"");
			B = strtok(NULL, "\",\"");
			X = strtok(NULL, "\",\"");
			Y = strtok(NULL, ",\")");
			j = Y - &copyOfStr[i] + (int)mir_strlen(Y) + 1;
			if (A && B && X && Y) {
				if (!mir_strcmp(A, B))
					mir_strcat(newStr, X);
				else mir_strcat(newStr, Y);
			}
			else strncat(newStr, &str[i], j);
			i += j;
		}
		else strncat(newStr, &str[i], 1);
	}
	mir_strcpy(str, newStr);
	free(newStr);
	free(copyOfStr);
}

// do save("A","B") A is DBVar name, B is value
void checkStringForSave(MCONTACT hContact, char* str)
{
	if (!strstr(str, "save(\"")) return;
	char *A, *B, *newStr = (char*)malloc(mir_strlen(str)), *copyOfStr = _strdup(str);
	unsigned int i, j = 0, s = (int)mir_strlen(str);
	newStr[0] = '\0';
	for (i = 0; i < s; i++) {
		if (!strncmp(&str[i], "save(\"", mir_strlen("save(\""))) {
			i += (int)mir_strlen("save(\"");
			A = strtok(&copyOfStr[i], "\",\"");
			B = strtok(NULL, ",\")");
			j = B - &copyOfStr[i] + (int)mir_strlen(B) + 1;
			if (A && B)
				db_set_s(hContact, MODNAME, A, B);

			else strncat(newStr, &str[i], j);
			i += j;
		}
		else strncat(newStr, &str[i], 1);
	}
	mir_strcpy(str, newStr);
	free(newStr);
	free(copyOfStr);
}

// do load("A") A is DBVar name
void checkStringForLoad(MCONTACT hContact, char* str)
{
	if (!strstr(str, "load(\"")) return;
	char *A, *newStr = (char*)malloc(mir_strlen(str)), *copyOfStr = _strdup(str);
	unsigned int i, j = 0, s = (int)mir_strlen(str);
	newStr[0] = '\0';
	for (i = 0; i < s; i++) {
		if (!strncmp(&str[i], "load(\"", mir_strlen("load(\""))) {
			i += (int)mir_strlen("load(\"");
			A = strtok(&copyOfStr[i], "\")");
			j = A - &copyOfStr[i] + (int)mir_strlen(A) + 1;
			if (A) {
				DBVARIANT dbv;
				if (!db_get_s(hContact, MODNAME, A, &dbv)) {
					mir_strcat(newStr, dbv.pszVal);
					db_free(&dbv);
				}
			}
			else strncat(newStr, &str[i], j);
			i += j;
		}
		else strncat(newStr, &str[i], 1);
	}
	mir_strcpy(str, newStr);
	free(newStr);
	free(copyOfStr);
}

// do saveN("A","B","C","D") A is module, B is setting, c is value, D is type 0/b 1/w 2/d 3/s
void checkStringForSaveN(char* str)
{
	if (!strstr(str, "saveN(\"")) return;
	char *A, *B, *C, *D, *newStr = (char*)malloc(mir_strlen(str)), *copyOfStr = _strdup(str);
	unsigned int i, j = 0, s = (int)mir_strlen(str);
	newStr[0] = '\0';
	for (i = 0; i < s; i++) {
		if (!strncmp(&str[i], "saveN(\"", mir_strlen("saveN(\""))) {
			i += (int)mir_strlen("saveN(\"");
			A = strtok(&copyOfStr[i], "\",\"");
			B = strtok(NULL, ",\"");
			C = strtok(NULL, ",\"");
			D = strtok(NULL, ",\")");
			j = D - &copyOfStr[i] + (int)mir_strlen(D) + 1;
			if (A && B && C && D) {
				switch (D[0]) {
				case '0':
				case 'b':
					db_set_b(NULL, A, B, (BYTE)atoi(C));
					break;
				case '1':
				case 'w':
					db_set_w(NULL, A, B, (WORD)atoi(C));
					break;
				case '2':
				case 'd':
					db_set_dw(NULL, A, B, (DWORD)atoi(C));
					break;
				case '3':
				case 's':
					db_set_s(NULL, A, B, C);
					break;
				}
			}
			else strncat(newStr, &str[i], j);
			i += j;
		}
		else strncat(newStr, &str[i], 1);
	}
	mir_strcpy(str, newStr);
	free(newStr);
	free(copyOfStr);
}

// do loadN("A","B") A is module, B is setting
void checkStringForLoadN(char* str)
{
	if (!strstr(str, "loadN(\"")) return;
	char *newStr = (char*)malloc(mir_strlen(str)), *copyOfStr = _strdup(str), temp[32];
	unsigned int i, j = 0, s = (int)mir_strlen(str);
	newStr[0] = '\0';
	for (i = 0; i < s; i++) {
		if (!strncmp(&str[i], "loadN(\"", mir_strlen("loadN(\""))) {
			i += (int)mir_strlen("loadN(\"");
			char *A = strtok(&copyOfStr[i], "\",\"");
			char *B = strtok(NULL, ",\")");
			if (A && B) {
				j = B - &copyOfStr[i] + (int)mir_strlen(B) + 1;
				DBVARIANT dbv;
				if (!db_get(NULL, A, B, &dbv)) {
					switch (dbv.type) {
					case DBVT_BYTE:
						mir_strcat(newStr, _itoa(dbv.bVal, temp, 10));
						break;
					case DBVT_WORD:
						mir_strcat(newStr, _itoa(dbv.wVal, temp, 10));
						break;
					case DBVT_DWORD:
						mir_strcat(newStr, _itoa(dbv.dVal, temp, 10));
						break;
					case DBVT_ASCIIZ:
						mir_strcat(newStr, dbv.pszVal);
						break;
					}
					db_free(&dbv);
				}
			}
			else strncat(newStr, &str[i], i);
			i += j;
		}
		else strncat(newStr, &str[i], 1);
	}
	mir_strcpy(str, newStr);
	free(newStr);
	free(copyOfStr);
}

BOOL GetLastWriteTime(HANDLE hFile, LPSTR lpszString)
{
	FILETIME ftCreate, ftAccess, ftWrite;
	SYSTEMTIME stUTC, stLocal;

	// Retrieve the file times for the file.
	if (!GetFileTime(hFile, &ftCreate, &ftAccess, &ftWrite))
		return FALSE;

	// Convert the last-write time to local time.
	FileTimeToSystemTime(&ftWrite, &stUTC);
	SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);

	// Build a string showing the date and time.
	wsprintfA(lpszString, "%02d/%02d/%d  %02d:%02d",
		stLocal.wDay, stLocal.wMonth, stLocal.wYear,
		stLocal.wHour, stLocal.wMinute); //!!!!!!!!!!!!!!!

	return TRUE;
}

// do lastchecked(file(X)) returns amount of chars to add to str pointer
int lastChecked(char *newStr, const char *str)
{
	char *szPattern = "lastchecked(file(";
	size_t cbPattern = mir_strlen(szPattern);

	if (!strncmp(str, szPattern, cbPattern)) {
		int file;
		char tszFileName[MAX_PATH], temp[MAX_PATH], szSetting[20];
		sscanf(&str[cbPattern], "%d", &file);
		mir_snprintf(szSetting, SIZEOF(szSetting), "fn%d", file);

		char *szVar = db_get_sa(NULL, MODNAME, szSetting);
		if (szVar == NULL)
			return 0;

		if (!strncmp("http://", szVar, 7) || !strncmp("https://", szVar, 8))
			mir_snprintf(tszFileName, SIZEOF(tszFileName), "%s\\plugins\\fn%d.html", getMimDir(temp), file);
		else
			mir_strncpy(tszFileName, szVar, SIZEOF(tszFileName));
		mir_free(szVar);

		HANDLE hFile = CreateFileA(tszFileName, 0, FILE_SHARE_READ, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
			return 0;

		if (GetLastWriteTime(hFile, tszFileName)) {
			CloseHandle(hFile);
			mir_strcat(newStr, tszFileName);
			mir_snprintf(tszFileName, SIZEOF(tszFileName), "%s%d))", szPattern, file);
			return (int)mir_strlen(tszFileName);
		}
		CloseHandle(hFile);
	}
	return 0;
}


// do icon(x) 0=offline, 1=online, 10=lunch
void checkIcon(MCONTACT hContact, char* string)
{
	char* str = strstr(string, "icon(");
	if (str) {
		int icon = getNumber(str + 5);
		if (icon >= 0)
			db_set_w(hContact, MODNAME, "Status", (WORD)(ID_STATUS_OFFLINE + icon));
	}
}

int stringReplacer(const char* oldString, char* newString, MCONTACT hContact)
{
	char var_file[8];
	int tempInt;
	int startLine = 0, endLine = 0, startChar = 0, endChar = 0, wholeLine = -1, linesInFile;
	int positionInOldString = 0;
	char *fileContents[MAXLINES] = { NULL }, tempString[MAX_STRING_LENGTH];

	//	setup the variable names
	strncpy(newString, "", sizeof(newString));
	strncpy(var_file, "file(", sizeof(var_file));

	while ((positionInOldString < (int)mir_strlen(oldString)) && (oldString[positionInOldString] != '\0')) {
		// load the file... must be first
		if (!strncmp(&oldString[positionInOldString], var_file, mir_strlen(var_file))) {
			positionInOldString += (int)mir_strlen(var_file);
			// check if its a number
			tempInt = getNumber(&oldString[positionInOldString]);
			if (tempInt == -1) {
				// not a number so check vars..
				// there are none yet
				return ERROR_NO_FILE;
			}
			// read the file
			linesInFile = readFileIntoArray(tempInt, fileContents);
			if (linesInFile == 0)
				return ERROR_NO_FILE;
			positionInOldString += (int)mir_strlen(_itoa(tempInt, tempString, 10)) + 1; // +1 for the closing )

			// wholeline()
			if (!strncmp(&oldString[positionInOldString], "wholeline(line(", mir_strlen("wholeline(line("))) {
				positionInOldString += (int)mir_strlen("wholeline(line(");
				tempInt = findLine(fileContents, oldString, linesInFile, startLine, &positionInOldString);
				if (tempInt == -1 || !fileContents[tempInt])
					return ERROR_NO_LINE_AFTER_VAR_F;
				wholeLine = tempInt;
				positionInOldString += 3; // add 2 for the )) for wholeline(line())
			}

			if (!strncmp(&oldString[positionInOldString], "start(", mir_strlen("start("))) {
				positionInOldString += (int)mir_strlen("start(line(");
				tempInt = findLine(fileContents, oldString, linesInFile, startLine, &positionInOldString);
				if (tempInt == -1 || !fileContents[tempInt])
					return ERROR_NO_LINE_AFTER_VAR_F;
				else {
					positionInOldString += 2;
					startLine = tempInt;
					if (!endChar)
						endChar = (int)mir_strlen(fileContents[startLine]);
					tempInt = findChar(fileContents, oldString, linesInFile, startLine, &positionInOldString, startChar, 0);
					if (tempInt == -1)
						return ERROR_NO_LINE_AFTER_VAR_F;
					startChar = tempInt;
				}
				positionInOldString += 2; // add 2 for the )) for start(line())
			}
			if (!strncmp(&oldString[positionInOldString], "end(", mir_strlen("end("))) {
				positionInOldString += (int)mir_strlen("end(line(");
				tempInt = findLine(fileContents, oldString, linesInFile, startLine, &positionInOldString);
				if (tempInt == -1 || !fileContents[tempInt])
					return ERROR_NO_LINE_AFTER_VAR_F;

				positionInOldString += 2;
				endLine = tempInt;
				tempInt = findChar(fileContents, oldString, linesInFile, startLine, &positionInOldString, startChar, 1);
				if (tempInt == -1)
					return ERROR_NO_LINE_AFTER_VAR_F;
				endChar = tempInt;
				positionInOldString += 2; // add 2 for the )) for end(line())
			}
			// check for both start() and end() otherwise, only copying 1 line
			if (!strstr(oldString, "start(")) startLine = endLine;
			if (!strstr(oldString, "end(")) endLine = startLine;
			// after all the options copy the line across and add 2 to positionInOldString for the file(print(....))
			if (wholeLine >= 0) mir_strcat(newString, fileContents[wholeLine]);
			else {
				// only copying from 1 line
				if (startLine == endLine)
					strncat(newString, &fileContents[startLine][startChar], endChar - startChar);
				else {
					int i;
					// copy the whole first line from startChar
					mir_strcat(newString, &fileContents[startLine][startChar]);
					// copy the middle lines across
					for (i = (startLine + 1); i < endLine; i++) {
						mir_strcat(newString, fileContents[i]);
					}
					// copy the last line untill endChar
					strncat(newString, fileContents[endLine], endChar);
				}
			}
		}
		// filename()
		else if (!strncmp(&oldString[positionInOldString], "filename(", mir_strlen("filename("))) {
			positionInOldString += (int)mir_strlen("filename(");
			tempInt = getNumber(&oldString[positionInOldString]);
			if (tempInt == -1) {
				return ERROR_NO_FILE;
			}
			else {
				mir_snprintf(tempString, SIZEOF(tempString), "fn%d", tempInt);
				if (db_get_static(NULL, MODNAME, tempString, tempString, SIZEOF(tempString)))
					mir_strcat(newString, tempString);
				else return ERROR_NO_FILE;
				positionInOldString += (int)mir_strlen(_itoa(tempInt, tempString, 10)) + 1;
			}
		}
		// lastchecked(file(X))
		else if (!strncmp(&oldString[positionInOldString], "lastchecked(file(", mir_strlen("lastchecked(file("))) {
			positionInOldString += lastChecked(newString, &oldString[positionInOldString]);
		}
		else {
			strncat(newString, &oldString[positionInOldString], 1);
			positionInOldString++;
		}
	}
	// free the file strings
	for (tempInt = 0; (fileContents[tempInt] != NULL) && (tempInt < MAXLINES); tempInt++)
		free(fileContents[tempInt]);



	// check for load("A","B")
	checkStringForLoad(hContact, newString);
	// and loadN(...)
	checkStringForLoadN(newString);
	// check for compare("A","B","X","Y")
	checkStringForcompare(newString);
	// check for save("A","B")
	checkStringForSave(hContact, newString);
	// and saveN(...)
	checkStringForSaveN(newString);
	return 1;
}

void stripWhiteSpace(char* string)
{
	int i = 0;
	while (string[i] != '\0') {
		if ((string[i] == '\t') || (string[i] == '\r') || (string[i] == '\n'))
			string[i] = ' ';
		i++;
	}
}
void WriteSetting(MCONTACT hContact, char* module1, char* setting1, char* module2, char* setting2)
{
	char text[MAX_STRING_LENGTH], newString[MAX_STRING_LENGTH];
	int error = 0, status = GetLCStatus(0, 0);
	if (db_get_static(hContact, module1, setting1, text, SIZEOF(text))) {
		switch (stringReplacer(text, newString, hContact)) {
		case ERROR_NO_LINE_AFTER_VAR_F:
			mir_snprintf(newString, SIZEOF(newString), Translate("%s - ERROR: no line specified or line not found (in %s)"), text, setting1);
			error = 1;
			break;
		case ERROR_LINE_NOT_READ:
			mir_snprintf(newString, SIZEOF(newString), Translate("%s - ERROR: file couldn't be opened (in %s)"), text, setting1);
			error = 1;
			break;
		case ERROR_NO_FILE:
			mir_snprintf(newString, SIZEOF(newString), Translate("%s - ERROR: no file specified in settings (in %s)"), text, setting1);
			error = 1;
			break;
		default:
			error = 0;
			break;
		}
		// strip the tab and new lines from all except the tooltip
		if (!error && mir_strcmp(setting1, "ToolTip")) stripWhiteSpace(newString);
		db_set_s(hContact, module2, setting2, newString);
	}
	else db_set_s(hContact, module2, setting2, "");
	
	if (!error) {
		if ((status == ID_STATUS_ONLINE) || (status == ID_STATUS_AWAY) ||
			 (status == db_get_w(hContact, MODNAME, "Icon", ID_STATUS_ONLINE)) ||
			 db_get_b(hContact, MODNAME, "AlwaysVisible", 0))
			db_set_w(hContact, MODNAME, "Status", (WORD)db_get_w(hContact, MODNAME, "Icon", ID_STATUS_ONLINE));
		else
			db_set_w(hContact, MODNAME, "Status", ID_STATUS_OFFLINE);
	}
	else db_set_w(hContact, MODNAME, "Status", ID_STATUS_OFFLINE);
}

void replaceAllStrings(MCONTACT hContact)
{
	char tmp1[256], tmp2[256], tmp3[256];
	WriteSetting(hContact, MODNAME, "Name", MODNAME, "Nick");
	WriteSetting(hContact, MODNAME, "ProgramString", MODNAME, "Program");
	WriteSetting(hContact, MODNAME, "ProgramParamsString", MODNAME, "ProgramParams");
	/* tooltips*/
	WriteSetting(hContact, MODNAME, "ToolTip", "UserInfo", "MyNotes");
	if (db_get_static(hContact, MODNAME, "Program", tmp1, SIZEOF(tmp1)) && db_get_static(hContact, MODNAME, "ProgramParams", tmp2, SIZEOF(tmp2))) {
		mir_snprintf(tmp3, SIZEOF(tmp3), "%s %s", tmp1, tmp2);
		db_set_s(hContact, "UserInfo", "FirstName", tmp3);
	}
	else if (db_get_static(hContact, MODNAME, "Program", tmp1, SIZEOF(tmp1))) {
		db_set_s(hContact, "UserInfo", "FirstName", tmp1);
	}
	else db_set_s(hContact, "UserInfo", "FirstName", "");
}
