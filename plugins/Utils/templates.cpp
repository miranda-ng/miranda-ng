#include "templates.h"

#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>

extern "C"
{
#include <newpluginapi.h>
#include <time.h>
#include <win2k.h>
#include <m_system.h>
#include <m_plugins.h>
#include <m_options.h>
#include <m_langpack.h>
#include <m_database.h>
#include <m_utils.h>
#include <m_protocols.h>

#include <m_notify.h>

#include "../utils/mir_memory.h"
}


const char *defaultVariables[] = { "%title%", "%text%" };
const WCHAR *defaultVariablesW[] = { L"%title%", L"%text%" };

const char *defaultVariableDescriptions[] = { "Notification Title", "Notification Text" };
const WCHAR *defaultVariableDescriptionsW[] = { L"Notification Title", L"Notification Text" };


#define MAX_REGS(_A_) ( sizeof(_A_) / sizeof(_A_[0]) )


#define DATA_BLOCK 128


// ASCII VERSIONS ///////////////////////////////////////////////////////////////////////

struct StringHelper
{
	char *text;
	size_t allocated;
	size_t used;
};


int CopyData(StringHelper *str, const char *text, size_t len)
{
	if (len == 0)
		return 0;

	if (text == NULL)
		return 0;

	size_t totalSize = str->used + len + 1;

	if (totalSize > str->allocated)
	{
		totalSize += DATA_BLOCK - (totalSize % DATA_BLOCK);

		if (str->text != NULL)
		{
			char *tmp = (char *) mir_realloc(str->text, sizeof(char) * totalSize);

			if (tmp == NULL)
			{
				mir_free(str->text);
				return -1;
			}

			str->text = tmp;
		}
		else
		{
			str->text = (char *) mir_alloc(sizeof(char) * totalSize);

			if (str->text == NULL)
			{
				return -2;
			}
		}

		str->allocated = totalSize;
	}

	memmove(&str->text[str->used], text, sizeof(char) * len);
	str->used += len;
	str->text[str->used] = '\0';
	
	return 0;
}


char * ParseText(const char *text, 
				 const char **variables, size_t variablesSize, 
				 const char **data, size_t dataSize) 
{
	size_t length = strlen(text);
	size_t nextPos = 0;
	StringHelper ret = {0};

	// length - 1 because a % in last char will be a % and point
	for (size_t i = 0 ; i < length - 1 ; i++)
	{
		if (text[i] == '%')
		{
			bool found = false;

			if (CopyData(&ret, &text[nextPos], i - nextPos))
				return NULL;

			if (text[i + 1] == '%')
			{
				if (CopyData(&ret, "%", 1))
					return NULL;

				i++;

				found = true;
			}
			else
			{
				size_t size = min(variablesSize, dataSize);

				// See if can find it
				for(size_t j = 0 ; j < size ; j++)
				{
					size_t vlen = strlen(variables[j]);

					if (strnicmp(&text[i], variables[j], vlen) == 0)
					{
						if (CopyData(&ret, data[j], strlen(data[j])))
							return NULL;

						i += vlen - 1;

						found = true;

						break;
					}
				}
			}


			if (found)
				nextPos = i + 1;
			else
				nextPos = i;
		}
	}

	if (nextPos < length)
		if (CopyData(&ret, &text[nextPos], length - nextPos))
			return NULL;

	return ret.text;
}


// UNICODE VERSIONS /////////////////////////////////////////////////////////////////////

struct StringHelperW
{
	WCHAR *text;
	size_t allocated;
	size_t used;
};


int CopyDataW(StringHelperW *str, const WCHAR *text, size_t len)
{
	if (len == 0)
		return 0;

	if (text == NULL)
		return 0;

	size_t totalSize = str->used + len + 1;

	if (totalSize > str->allocated)
	{
		totalSize += DATA_BLOCK - (totalSize % DATA_BLOCK);

		if (str->text != NULL)
		{
			WCHAR *tmp = (WCHAR *) mir_realloc(str->text, sizeof(WCHAR) * totalSize);

			if (tmp == NULL)
			{
				mir_free(str->text);
				return -1;
			}

			str->text = tmp;
		}
		else
		{
			str->text = (WCHAR *) mir_alloc(sizeof(WCHAR) * totalSize);

			if (str->text == NULL)
			{
				return -2;
			}
		}

		str->allocated = totalSize;
	}

	memmove(&str->text[str->used], text, sizeof(WCHAR) * len);
	str->used += len;
	str->text[str->used] = '\0';
	
	return 0;
}


WCHAR * ParseTextW(const WCHAR *text, 
				   const WCHAR **variables, size_t variablesSize, 
				   const WCHAR **data, size_t dataSize) 
{
	size_t length = lstrlenW(text);
	size_t nextPos = 0;
	StringHelperW ret = {0};

	// length - 1 because a % in last char will be a % and point
	for (size_t i = 0 ; i < length - 1 ; i++)
	{
		if (text[i] == L'%')
		{
			bool found = false;

			if (CopyDataW(&ret, &text[nextPos], i - nextPos))
				return NULL;

			if (text[i + 1] == L'%')
			{
				if (CopyDataW(&ret, L"%", 1))
					return NULL;

				i++;

				found = true;
			}
			else
			{
				size_t size = min(variablesSize, dataSize);

				// See if can find it
				for(size_t j = 0 ; j < size ; j++)
				{
					size_t vlen = lstrlenW(variables[j]);

					if (_wcsnicmp(&text[i], variables[j], vlen) == 0)
					{
						if (CopyDataW(&ret, data[j], lstrlenW(data[j])))
							return NULL;

						i += vlen - 1;

						found = true;

						break;
					}
				}
			}

			if (found)
				nextPos = i + 1;
			else
				nextPos = i;
		}
	}

	if (nextPos < length)
		if (CopyDataW(&ret, &text[nextPos], length - nextPos))
			return NULL;

	return ret.text;
}


// INTERFACE ////////////////////////////////////////////////////////////////////////////

// In future maybe pre-parse here
void MNotifySetTemplate(HANDLE notifyORtype, const char *name, const char *value)
{
	MNotifySetString(notifyORtype, name, value);
}
void MNotifySetWTemplate(HANDLE notifyORtype, const char *name, const WCHAR *value)
{
	MNotifySetWString(notifyORtype, name, value);
}

// Well, why not?
const char *MNotifyGetTemplate(HANDLE notifyORtype, const char *name, const char *defValue)
{
	return MNotifyGetString(notifyORtype, name, defValue);
}
const WCHAR *MNotifyGetWTemplate(HANDLE notifyORtype, const char *name, const WCHAR *defValue)
{
	return MNotifyGetWString(notifyORtype, name, defValue);
}

// You must free the return with mir_sys_free
char *MNotifyGetParsedTemplate(HANDLE notifyORtype, const char *name, const char *defValue)
{
	const char *temp = MNotifyGetString(notifyORtype, name, defValue);

	if (MNotifyHasVariables(notifyORtype))
	{
		const char ** vars = (const char **) MNotifyGetDWord(notifyORtype, NFOPT_VARIABLES_STRS, NULL);
		size_t varsSize = (size_t) MNotifyGetDWord(notifyORtype, NFOPT_VARIABLES_SIZE, 0);

		const char ** data = (const char **) MNotifyGetDWord(notifyORtype, NFOPT_DATA_STRS, NULL);
		size_t dataSize = (size_t) MNotifyGetDWord(notifyORtype, NFOPT_DATA_SIZE, 0);

		return ParseText(temp, vars, varsSize, data, dataSize);
	}
	else
	{
		// Default values
		const char * data[MAX_REGS(defaultVariables)];
		data[0] = MNotifyGetString(notifyORtype, NFOPT_TITLE, NULL);
		data[1] = MNotifyGetString(notifyORtype, NFOPT_TEXT, NULL);

		return ParseText(temp, defaultVariables, MAX_REGS(defaultVariables), data, MAX_REGS(defaultVariables));
	}
}
WCHAR *MNotifyGetWParsedTemplate(HANDLE notifyORtype, const char *name, const WCHAR *defValue)
{
	const WCHAR *temp = MNotifyGetWString(notifyORtype, name, defValue);

	if (MNotifyHasWVariables(notifyORtype))
	{
		const WCHAR ** vars = (const WCHAR **) MNotifyGetDWord(notifyORtype, NFOPT_VARIABLES_STRSW, NULL);
		size_t varsSize = (size_t) MNotifyGetDWord(notifyORtype, NFOPT_VARIABLES_SIZE, 0);

		const WCHAR ** data = (const WCHAR **) MNotifyGetDWord(notifyORtype, NFOPT_DATA_STRSW, NULL);
		size_t dataSize = (size_t) MNotifyGetDWord(notifyORtype, NFOPT_DATA_SIZE, 0);

		return ParseTextW(temp, vars, varsSize, data, dataSize);
	}
	else
	{
		// Default values
		const WCHAR * data[MAX_REGS(defaultVariablesW)];
		data[0] = MNotifyGetWString(notifyORtype, NFOPT_TITLEW, NULL);
		data[1] = MNotifyGetWString(notifyORtype, NFOPT_TEXTW, NULL);

		return ParseTextW(temp, defaultVariablesW, MAX_REGS(defaultVariablesW), data, MAX_REGS(defaultVariablesW));
	}
}


BOOL MNotifyHasVariables(HANDLE notifyORtype)
{
	return MNotifyGetDWord(notifyORtype, NFOPT_VARIABLES_STRS, NULL) != NULL && 
			MNotifyGetDWord(notifyORtype, NFOPT_VARIABLES_SIZE, 0) != 0;
}

BOOL MNotifyHasWVariables(HANDLE notifyORtype)
{
	return MNotifyGetDWord(notifyORtype, NFOPT_VARIABLES_STRSW, NULL) != NULL && 
			MNotifyGetDWord(notifyORtype, NFOPT_VARIABLES_SIZE, 0) != 0;
}




void MNotifyShowVariables(HANDLE notifyORtype)
{
	StringHelper ret = {0};

	const char ** vars;
	size_t varsSize;
	const char ** descs;
	size_t descsSize;

	if (MNotifyHasVariables(notifyORtype))
	{
		vars = (const char **) MNotifyGetDWord(notifyORtype, NFOPT_VARIABLES_STRS, NULL);
		varsSize = (size_t) MNotifyGetDWord(notifyORtype, NFOPT_VARIABLES_SIZE, 0);

		descs = (const char **) MNotifyGetDWord(notifyORtype, NFOPT_VARIABLES_DESCRIPTIONS_STRS, NULL);
		descsSize = (size_t) MNotifyGetDWord(notifyORtype, NFOPT_VARIABLES_DESCRIPTIONS_SIZE, 0);
	}
	else
	{
		// Default values
		vars = defaultVariables;
		varsSize = MAX_REGS(defaultVariables);

		descs = defaultVariableDescriptions;
		descsSize = MAX_REGS(defaultVariableDescriptions);
	}

	for(size_t i = 0 ; i < varsSize ; i++)
	{
		if (vars[i] != NULL)
		{
			if (CopyData(&ret, vars[i], strlen(vars[i])))
				return;

			if (i < descsSize && descs[i] != NULL && descs[i] != L'\0')
			{
				if (CopyData(&ret, " -> ", 4))
					return;
				if (CopyData(&ret, descs[i], strlen(descs[i])))
					return;
			}

			if (CopyData(&ret, "\r\n", 2))
				return;
		}
	}

	MessageBoxA(NULL, ret.text, "Variables", MB_OK);

	mir_free(ret.text);
}

void MNotifyShowWVariables(HANDLE notifyORtype)
{
	StringHelperW ret = {0};

	const WCHAR ** vars;
	size_t varsSize;
	const WCHAR ** descs;
	size_t descsSize;

	if (MNotifyHasWVariables(notifyORtype))
	{
		vars = (const WCHAR **) MNotifyGetDWord(notifyORtype, NFOPT_VARIABLES_STRSW, NULL);
		varsSize = (size_t) MNotifyGetDWord(notifyORtype, NFOPT_VARIABLES_SIZE, 0);

		descs = (const WCHAR **) MNotifyGetDWord(notifyORtype, NFOPT_VARIABLES_DESCRIPTIONS_STRSW, NULL);
		descsSize = (size_t) MNotifyGetDWord(notifyORtype, NFOPT_VARIABLES_DESCRIPTIONS_SIZE, 0);
	}
	else
	{
		// Default values
		vars = defaultVariablesW;
		varsSize = MAX_REGS(defaultVariablesW);

		descs = defaultVariableDescriptionsW;
		descsSize = MAX_REGS(defaultVariableDescriptionsW);
	}

	for(size_t i = 0 ; i < varsSize ; i++)
	{
		if (vars[i] != NULL)
		{
			if (CopyDataW(&ret, vars[i], lstrlenW(vars[i])))
				return;

			if (i < descsSize && descs[i] != NULL && descs[i] != L'\0')
			{
				if (CopyDataW(&ret, L" -> ", 3))
					return;
				if (CopyDataW(&ret, descs[i], lstrlenW(descs[i])))
					return;
			}

			if (CopyDataW(&ret, L"\r\n", 2))
				return;
		}
	}

	MessageBoxW(NULL, ret.text, L"Variables", MB_OK);

	mir_free(ret.text);
}


