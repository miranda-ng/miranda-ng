#include "commonheaders.h"

int readFileIntoArray(int fileNumber, char *FileContents[])
{
	int i =0;
	FILE* file;
	char FileName[MAX_PATH], temp[MAX_STRING_LENGTH];
	wsprintf(FileName,"fn%d",fileNumber);
	DBGetContactSettingString(NULL,modname, FileName, FileName);

	if (!strncmp("http://", FileName, strlen("http://")))
		wsprintf(FileName, "%s\\plugins\\fn%d.html",getMimDir(temp),fileNumber);

	file = fopen(FileName, "r");
	if (file == NULL) return 0;
	// read the file into the FileContents array
	// free this array before stringReplacer() returns
	while (fgets(temp, MAX_STRING_LENGTH-1,file))
	{
		if(temp[strlen(temp)-1]=='\n') 
			temp[strlen(temp)-1]='\0';
		else temp[strlen(temp)]='\0';

		FileContents[i] = (char*)malloc(strlen(temp)+1);
		if (FileContents[i] == NULL) return i;
		strcpy(FileContents[i], temp);
		i++;
	}
	fclose(file);
	return i;
}

int getNumber(const char* line)
{
	int i;
	if (sscanf(line, "%d", &i) == 1) return i; else return -1;
}
	
int findWordInString(const char* line, const char* string, int* lengthOfWord, int flag) /* flag = 0 %from, flag = 1 %until */
{
	unsigned int i, j=0;
	char word[64]="", OpenDivider[8], CloseDivider[8];
	strcpy(OpenDivider, Translate("(\""));
	strcpy(CloseDivider, Translate("\")"));
	/* get the word we r looking for */
	if (!strncmp(string, OpenDivider, strlen(OpenDivider)))
	{
		for (i=2; strncmp(&string[i], CloseDivider, strlen(CloseDivider)); i++)
		{
			word[j] = string[i];
			word[++j] = '\0';
		}
	}
	i=0;
	*lengthOfWord = strlen(word)+strlen(CloseDivider)+strlen(OpenDivider);
	/* find the word in the line */
	while (i < (strlen(line) - strlen(word) ) )
	{
		if (!strncmp(&line[i], word, strlen(word)))
		{
			if (!flag) return i + strlen(word); /* the next char after the word */
			else return i; /* the char before the word */
		}
		i++;
	}
	return -1;
}

int findLine(char* FileContents[], const char* string, int linesInFile,int startLine, int *positionInOldString)
{
	int i;
	char tmp[5];
	i = getNumber(&string[*positionInOldString]);
	// check if blank
	if (string[*positionInOldString] == ')')
		return startLine;
	// check if its a number
	if ( i != -1)
	{
		*positionInOldString += strlen(itoa(i,tmp,10)) - 1; 
		return i;
	}
	// lastline
	else if (!strncmp(&string[*positionInOldString], Translate("lastline("), strlen(Translate("lastline("))))
	{
		*positionInOldString += strlen(Translate("lastline("));
		i = getNumber(&string[*positionInOldString]);
		if ( i != -1)
		{
			*positionInOldString += strlen(itoa(i,tmp,10)); 
			return linesInFile - (i+1);
		}
		else 
		{
			*positionInOldString ++; 
			return (linesInFile - 1);
		}
	}
	// string
	else if (string[*positionInOldString] == '\"')
	{
		char string2Find[256];
		int j=0;
		// get the word to find
		for (i=(*positionInOldString+1); strncmp(&string[i], "\")", 2); i++)
		{
			string2Find[j] = string[i];
			string2Find[++j] = '\0';
		}
		// find the word
		for (j=startLine; j<linesInFile;j++)
		{
			if (strstr(FileContents[j], string2Find))
			{
				i = j;
				break;
			}
			i=-1;
		}
		*positionInOldString += strlen(string2Find) + strlen(Translate("\"\")"));
		if (i==-1) return i;
		// allow for a +- after the word to go up or down lines
		if (string[*positionInOldString] == '+')
		{
			*positionInOldString += 1;
			j = getNumber(&string[*positionInOldString]);
			if (j != -1)
			{
				*positionInOldString += strlen(itoa(j,tmp,10))-2;
				return i+j;
			}
		}
		else if (string[*positionInOldString] == '-')
		{
			*positionInOldString+=1;
			j = getNumber(&string[*positionInOldString]);
			if (j != -1)
			{
				*positionInOldString += strlen(itoa(j,tmp,10))-2;
				return i-j;
			}
		}
		else 
		{
			*positionInOldString -= 2;
			return i;
		}
	}
/*	else if (!strncmp(&string[*positionInOldString] , Translate("regexp(\""), strlen(Translate("regexp(\""))))
	{
		char string2Find[256];
		int j=0;
		// get the word to find
		for (i=(*positionInOldString+1); strncmp(&string[i], "\")", 2); i++)
		{
			string2Find[j] = string[i];
			string2Find[++j] = '\0';
		}
		*positionInOldString += strlen(Translate("regexp(\"")) + strlen(string2Find) +2;
	}
*/	return -1;
}

int findChar(char* FileContents[], const char* string, int linesInFile,int startLine, int *positionInOldString, int startChar, int startEnd) // 0=start, 1=end for startEnd
{
	int i;
	char tmp[5];
	i = getNumber(&string[*positionInOldString]);
	// check if its a number
	if ( i != -1)
	{
		*positionInOldString += strlen(itoa(i,tmp,10)) - 1; 
		return i;
	}
	// string
	else if (string[*positionInOldString] == '\"')
	{
		char string2Find[256];
		unsigned int j=0;
		// get the word to find
		for (i=(*positionInOldString+1); strncmp(&string[i], "\")", 2); i++)
		{
			string2Find[j] = string[i];
			string2Find[++j] = '\0';
		}
		// find the word
		for (j=0;j<strlen(FileContents[startLine]);j++)
		{
			if (!strncmp(&FileContents[startLine][j], string2Find, strlen(string2Find)))
				break;
		}
		if (j==strlen(FileContents[startLine])) return -1;
		else 
		{
			*positionInOldString += strlen(string2Find)+1;
			if (startEnd) return j;
			else return j+strlen(string2Find);
		}
	}
	// csv(
	else if (!strncmp(&string[*positionInOldString], Translate("csv("), strlen(Translate("csv("))))
	{
		char seperator;
		int j=0, k=startChar;
		*positionInOldString += strlen(Translate("csv("));
		if (!strncmp(&string[*positionInOldString], "tab",3))
		{
			*positionInOldString += 3;
			seperator = '\t';
		}
		else if (!strncmp(&string[*positionInOldString], "space",5))
		{
			*positionInOldString += 5;
			seperator = ' ';
		}
		else
		{
			seperator =string[*positionInOldString];
			*positionInOldString += 1;
		}
		i = getNumber(&string[*positionInOldString]);
		if ( i == -1) return -1;
		*positionInOldString += strlen(itoa(i,tmp,10));
		while (j<i)
		{
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
	char *A,*B, *X, *Y , *newStr = (char*)malloc(strlen(str)), *copyOfStr = strdup(str);
	unsigned int i, j=0, s=strlen(str);
	newStr[0] = '\0';
	if (!strstr(str,Translate("compare(\""))) return;
	for (i=0; i<s; i++)
	{
		if (!strncmp(&str[i], Translate("compare(\""), strlen(Translate("compare(\""))))
		{
			i += strlen(Translate("compare(\""));
			A = strtok(&copyOfStr[i], "\",\"");
			B = strtok(NULL, "\",\"");
			X = strtok(NULL, "\",\"");
			Y = strtok(NULL, ",\")");
			j = Y - &copyOfStr[i] + strlen(Y)+1;
			if (A && B && X && Y)
			{
				if (!strcmp(A,B))
					strcat(newStr, X);
				else strcat(newStr, Y);
			}
			else strncat(newStr, &str[i], j);
			i += j;
		}
		else 
		{
			strncat(newStr, &str[i], 1);
		}
	}
	strcpy(str, newStr);
	free(newStr);
	free(copyOfStr);
}

// do save("A","B") A is DBVar name, B is value
void checkStringForSave(HANDLE hContact, char* str)
{
	char *A,*B,*newStr = (char*)malloc(strlen(str)),*copyOfStr = strdup(str);
	unsigned int i, j=0, s=strlen(str);
	newStr[0] = '\0';
	if (!strstr(str,Translate("save(\""))) return;
	for (i=0; i<s; i++)
	{
		if (!strncmp(&str[i], Translate("save(\""), strlen(Translate("save(\""))))
		{
			i += strlen(Translate("save(\""));
			A = strtok(&copyOfStr[i], "\",\"");
			B = strtok(NULL, ",\")");
			j = B - &copyOfStr[i] + strlen(B)+1;
			if (A && B)
			{
				DBWriteContactSettingString(hContact,modname, A, B);
			}
			else strncat(newStr, &str[i], j);
			i += j;
		}
		else 
		{
			strncat(newStr, &str[i], 1);
		}
	}
	strcpy(str, newStr);
	free(newStr);
	free(copyOfStr);
}

// do load("A") A is DBVar name
void checkStringForLoad(HANDLE hContact, char* str)
{
	char *A,*newStr = (char*)malloc(strlen(str)),*copyOfStr = strdup(str);
	unsigned int i, j=0, s=strlen(str);
	newStr[0] = '\0';
	if (!strstr(str,Translate("load(\""))) return;
	for (i=0; i<s; i++)
	{
		if (!strncmp(&str[i], Translate("load(\""), strlen(Translate("load(\""))))
		{
			i += strlen(Translate("load(\""));
			A = strtok(&copyOfStr[i], "\")");
			j = A - &copyOfStr[i] + strlen(A)+1;
			if (A)
			{
				DBVARIANT dbv;
				if (!DBGetContactSetting(hContact,modname, A, &dbv))
				{
					if (dbv.type == DBVT_ASCIIZ)
						strcat(newStr, dbv.pszVal);
					DBFreeVariant(&dbv);
				}
			}
			else strncat(newStr, &str[i], j);
			i += j;
		}
		else 
		{
			strncat(newStr, &str[i], 1);
		}
	}
	strcpy(str, newStr);
	free(newStr);
	free(copyOfStr);
}

// do saveN("A","B","C","D") A is module, B is setting, c is value, D is type 0/b 1/w 2/d 3/s
void checkStringForSaveN(char* str)
{
	char *A,*B,*C,*D,*newStr = (char*)malloc(strlen(str)),*copyOfStr = strdup(str);
	unsigned int i, j=0, s=strlen(str);
	newStr[0] = '\0';
	if (!strstr(str,Translate("saveN(\""))) return;
	for (i=0; i<s; i++)
	{
		if (!strncmp(&str[i], Translate("saveN(\""), strlen(Translate("saveN(\""))))
		{
			i += strlen(Translate("saveN(\""));
			A = strtok(&copyOfStr[i], "\",\"");
			B = strtok(NULL, ",\"");
			C = strtok(NULL, ",\"");
			D = strtok(NULL, ",\")");
			j = D - &copyOfStr[i] + strlen(D)+1;
			if (A && B && C && D)
			{
				switch (D[0])
				{
					case '0':
					case 'b':
						DBWriteContactSettingByte(NULL, A, B, (BYTE)atoi(C));
					break;
					case '1':
					case 'w':
						DBWriteContactSettingWord(NULL, A, B, (WORD)atoi(C));
					break;
					case '2':
					case 'd':
						DBWriteContactSettingDword(NULL, A, B, (DWORD)atoi(C));
					break;
					case '3':
					case 's':
						DBWriteContactSettingString(NULL, A, B, C);
					break;
				}
			}
			else strncat(newStr, &str[i], j);
			i += j;
		}
		else 
		{
			strncat(newStr, &str[i], 1);
		}
	}
	strcpy(str, newStr);
	free(newStr);
	free(copyOfStr);
}

// do loadN("A","B") A is module, B is setting
void checkStringForLoadN(char* str)
{
	char *A,*B,*newStr = (char*)malloc(strlen(str)),*copyOfStr = strdup(str), temp[32];
	unsigned int i, j=0, s=strlen(str);
	newStr[0] = '\0';
	if (!strstr(str,Translate("loadN(\""))) return;
	for (i=0; i<s; i++)
	{
		if (!strncmp(&str[i], Translate("loadN(\""), strlen(Translate("loadN(\""))))
		{
			i += strlen(Translate("loadN(\""));
			A = strtok(&copyOfStr[i], "\",\"");
			B = strtok(NULL, ",\")");
			j = B - &copyOfStr[i] + strlen(B)+1;
			if (A && B)
			{
				DBVARIANT dbv;
				if (!DBGetContactSetting(NULL,A, B, &dbv))
				{	
					switch (dbv.type)
					{
						case DBVT_BYTE:
							strcat(newStr,itoa(dbv.bVal,temp,10));
						break;
						case DBVT_WORD:
							strcat(newStr,itoa(dbv.wVal,temp,10));
						break;
						case DBVT_DWORD:
							strcat(newStr,itoa(dbv.dVal,temp,10));
						break;
						case DBVT_ASCIIZ:
							strcat(newStr, dbv.pszVal);
						break;
					}
					DBFreeVariant(&dbv);
				}
			}
			else strncat(newStr, &str[i], j);
			i += j;
		}
		else 
		{
			strncat(newStr, &str[i], 1);
		}
	}
	strcpy(str, newStr);
	free(newStr);
	free(copyOfStr);
}

BOOL GetLastWriteTime(HANDLE hFile, LPTSTR lpszString)
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
    wsprintf(lpszString, TEXT("%02d/%02d/%d  %02d:%02d"),
        stLocal.wDay, stLocal.wMonth, stLocal.wYear,
        stLocal.wHour, stLocal.wMinute);

    return TRUE;
}

// do lastchecked(file(X)) returns amount of chars to add to str pointer
int lastChecked(char* newStr, const char* str)
{
	if (!strncmp(str, Translate("lastchecked(file("), strlen(Translate("lastchecked(file("))))
	{
		int file;
		HANDLE hFile;
		char FileName[MAX_PATH], temp[MAX_PATH];
		sscanf(&str[strlen(Translate("lastchecked(file("))], "%d", &file);
		_snprintf(FileName, MAX_PATH, "fn%d", file);
		DBGetContactSettingString(NULL,modname, FileName, FileName);

		if (!strncmp("http://", FileName, strlen("http://")) || !strncmp("https://", FileName, strlen("https://")))
			wsprintf(FileName, "%s\\plugins\\fn%d.html",getMimDir(temp),file);
		hFile= CreateFile(FileName, 0 , FILE_SHARE_READ , 0 ,OPEN_ALWAYS , FILE_ATTRIBUTE_NORMAL , NULL );
		if (hFile == INVALID_HANDLE_VALUE) return 0;
		if (GetLastWriteTime(hFile, FileName))
		{
			strcat(newStr, FileName);
			_snprintf(FileName, MAX_PATH, "%s%d))",Translate("lastchecked(file("), file);
			return strlen(FileName);
		}
		return 0;
	}
	return 0;
}


// do icon(x) 0=offline, 1=online, 10=lunch
void checkIcon(HANDLE hContact, char* string)
{
	char* str = strstr(string,"icon(");
	if (str)
	{
		int icon = getNumber(str+5);
		if (icon >=0)
		{
			DBWriteContactSettingWord(hContact, modname, "Status", (WORD)(ID_STATUS_OFFLINE+icon));
		}
	}
}

int stringReplacer(const char* oldString, char* newString, HANDLE hContact)
{
	char var_file[8];
	int tempInt;
	int startLine = 0, endLine = 0, startChar=0, endChar = 0, wholeLine=-1, linesInFile;
	unsigned int positionInOldString = 0;
	char *fileContents[MAXLINES] = {NULL}, tempString[MAX_STRING_LENGTH];
	
//	setup the variable names
	strcpy(newString, "");
	strcpy(var_file, Translate("file("));


	while ((positionInOldString < strlen(oldString)) && (oldString[positionInOldString] != '\0'))
	{
		// load the file... must be first
		if (!strncmp(&oldString[positionInOldString], var_file, strlen(var_file)))
		{
			positionInOldString += strlen(var_file);
			// check if its a number
			tempInt = getNumber(&oldString[positionInOldString]);
			if (tempInt == -1)
			{
				// not a number so check vars..
				// there are none yet
				return ERROR_NO_FILE;
			}
			// read the file
			linesInFile = readFileIntoArray(tempInt, fileContents);
			if (linesInFile == 0) return ERROR_NO_FILE;
			positionInOldString += strlen(itoa(tempInt, tempString,10)) + 1; // +1 for the closing )
		
			// wholeline()
			if (!strncmp(&oldString[positionInOldString], Translate("wholeline(line("), strlen(Translate("wholeline(line("))))
			{
				positionInOldString += strlen(Translate("wholeline(line("));
				tempInt = findLine(fileContents,oldString, linesInFile, startLine,&positionInOldString);
				if (tempInt == -1|| !fileContents[tempInt])
					return ERROR_NO_LINE_AFTER_VAR_F;
				else wholeLine =tempInt;
				positionInOldString += 3; // add 2 for the )) for wholeline(line())
			}
			
			if (!strncmp(&oldString[positionInOldString], Translate("start("), strlen(Translate("start("))))
			{
				positionInOldString += strlen(Translate("start(line("));
				tempInt = findLine(fileContents,oldString, linesInFile, startLine,&positionInOldString);
				if (tempInt == -1 || !fileContents[tempInt])
					return ERROR_NO_LINE_AFTER_VAR_F;
				else 
				{
					positionInOldString+=2;
					startLine = tempInt;
					if (!endChar ) endChar = strlen(fileContents[startLine]);
					tempInt = findChar(fileContents,oldString, linesInFile, startLine,&positionInOldString, startChar,0);
					if (tempInt == -1)
						return ERROR_NO_LINE_AFTER_VAR_F;
					else startChar = tempInt;
				}
				positionInOldString += 2; // add 2 for the )) for start(line())
			}
			if (!strncmp(&oldString[positionInOldString], Translate("end("), strlen(Translate("end("))))
			{
				positionInOldString += strlen(Translate("end(line("));
				tempInt = findLine(fileContents,oldString, linesInFile, startLine,&positionInOldString);
				if (tempInt == -1 || !fileContents[tempInt])
					return ERROR_NO_LINE_AFTER_VAR_F;
				else 
				{
					positionInOldString+=2;
					endLine = tempInt;
					tempInt = findChar(fileContents,oldString, linesInFile, startLine,&positionInOldString, startChar,1);
					if (tempInt == -1)
						return ERROR_NO_LINE_AFTER_VAR_F;
					else endChar = tempInt;
				}
				positionInOldString += 2; // add 2 for the )) for end(line())
			}
			// check for both start() and end() otherwise, only copying 1 line
			if (!strstr(oldString, Translate("start("))) startLine = endLine;
			if (!strstr(oldString, Translate("end("))) endLine = startLine;
			// after all the options copy the line across and add 2 to positionInOldString for the file(print(....))
			if (wholeLine >= 0) strcat(newString, fileContents[wholeLine]);
			else
			{
				// only copying from 1 line
				if (startLine == endLine)
					strncat(newString, &fileContents[startLine][startChar], endChar - startChar);
				else
				{
					int i;
					// copy the whole first line from startChar
					strcat(newString, &fileContents[startLine][startChar]);
					// copy the middle lines across
					for (i=(startLine+1);i<endLine;i++)
					{
						strcat(newString, fileContents[i]);
					}
					// copy the last line untill endChar
					strncat(newString, fileContents[endLine], endChar);
				}
			}
		}
		// filename()
		else if (!strncmp(&oldString[positionInOldString], Translate("filename("), strlen(Translate("filename("))))
		{
			positionInOldString += strlen(Translate("filename("));
			tempInt = getNumber(&oldString[positionInOldString]);
			if (tempInt == -1)
			{
				return ERROR_NO_FILE;
			}
			else 
			{
				wsprintf(tempString, "fn%d", tempInt);
				if (DBGetContactSettingString(NULL, modname,tempString, tempString))
					strcat(newString, tempString);
				else return ERROR_NO_FILE;
				positionInOldString += strlen(itoa(tempInt, tempString, 10))+1;
			}
		}
		// lastchecked(file(X))
		else if (!strncmp(&oldString[positionInOldString], Translate("lastchecked(file("), strlen(Translate("lastchecked(file("))))
		{
			positionInOldString += lastChecked(newString, &oldString[positionInOldString]);
		}
		else
		{
			strncat(newString, &oldString[positionInOldString], 1);
			positionInOldString++;
		}
	}
	// free the file strings
	for (tempInt=0; (fileContents[tempInt] != NULL) && (tempInt<MAXLINES); tempInt++)
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
	int i=0;
	while (string[i] != '\0')
	{
		if (  (string[i] == '\t') || (string[i] == '\r') || (string[i] == '\n') )
			string[i] = ' ';
		i++;
	}
}
void WriteSetting(HANDLE hContact, char* module1, char* setting1 , char* module2, char* setting2)
{
	char text[MAX_STRING_LENGTH], newString[MAX_STRING_LENGTH];
	int error = 0, status = GetLCStatus(0,0);
	if (DBGetContactSettingString(hContact, module1, setting1, text))
	{
		switch (stringReplacer(text, newString, hContact))
		{
			case ERROR_NO_LINE_AFTER_VAR_F:
				wsprintf(newString, Translate("%s - ERROR: no line specified or line not found (in %s)"),text, setting1);
				error = 1;
			break;
			case ERROR_LINE_NOT_READ:
				wsprintf(newString, Translate("%s - ERROR: file couldnt be opened (in %s)"), text, setting1);
				error = 1;
			break;
			case ERROR_NO_FILE:
				wsprintf(newString, Translate("%s - ERROR: no file specified in settings (in %s)"), text, setting1);
				error = 1;
			break;
			default:
				error = 0;
			break;
		}
		// strip the tab and new lines from all except the tooltip
		if (!error && strcmp(setting1, "ToolTip")) stripWhiteSpace(newString);
		DBWriteContactSettingString(hContact, module2, setting2, newString);
	}
	else DBWriteContactSettingString(hContact, module2, setting2, "");
	if (!error)
	{
		if ( (status == ID_STATUS_ONLINE) || (status == ID_STATUS_AWAY) || 
			 (status == DBGetContactSettingWord(hContact, modname, "Icon", ID_STATUS_ONLINE) ) ||
			 DBGetContactSettingByte(hContact, modname ,"AlwaysVisible", 0)
		   )
			DBWriteContactSettingWord(hContact, modname, "Status", (WORD)DBGetContactSettingWord(hContact, modname, "Icon", ID_STATUS_ONLINE));
		else DBWriteContactSettingWord(hContact, modname, "Status", ID_STATUS_OFFLINE);
	}
	else DBWriteContactSettingWord(hContact, modname, "Status", ID_STATUS_OFFLINE);
}
                      
void replaceAllStrings(HANDLE hContact)
{
	char tmp1[256], tmp2[256], tmp3[256];
	WriteSetting(hContact, modname, "Name", modname, "Nick");
	WriteSetting(hContact, modname, "ProgramString", modname, "Program");
	WriteSetting(hContact, modname, "ProgramParamsString", modname, "ProgramParams");
	/* tooltips*/
	WriteSetting(hContact, modname, "ToolTip", "UserInfo", "MyNotes");
	if (DBGetContactSettingString(hContact, modname, "Program", tmp1) && DBGetContactSettingString(hContact, modname, "ProgramParams", tmp2))
	{
		wsprintf(tmp3, "%s %s", tmp1, tmp2);
		DBWriteContactSettingString(hContact, "UserInfo", "FirstName", tmp3);
	}
	else if (DBGetContactSettingString(hContact, modname, "Program", tmp1))
	{
		DBWriteContactSettingString(hContact, "UserInfo", "FirstName", tmp1);
	}
	else DBWriteContactSettingString(hContact, "UserInfo", "FirstName", "");
}

