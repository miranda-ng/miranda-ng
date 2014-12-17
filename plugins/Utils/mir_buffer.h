/*
Copyright (C) 2005-2009 Ricardo Pescuma Domenecci

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.
*/


#ifndef __MIR_BUFFER_H__
# define __MIR_BUFFER_H__

#include <windows.h>

#include <m_variables.h>

template<class T>
static inline size_t __blen(const T *str)
{
	return 0;
}

template<>
static inline size_t __blen<char>(const char *str)
{
	return strlen(str);
}

template<>
static inline size_t __blen<wchar_t>(const wchar_t *str)
{
	return mir_wstrlen(str);
}

template<class T>
static inline T * __bTranslate(const T *str)
{
	return 0;
}

template<>
static inline char * __bTranslate<char>(const char *str)
{
	return Translate(str);
}

template<>
static inline wchar_t * __bTranslate<wchar_t>(const wchar_t *str)
{
	return TranslateW(str);
}


template<class O, class D>
static void __bcopy(D *dest, const O *orig, size_t len)
{
}

template<>
static void __bcopy(char *dest, const char *orig, size_t len)
{
	strncpy(dest, orig, len);
}

template<>
static void __bcopy(WCHAR *dest, const WCHAR *orig, size_t len)
{
	wcsncpy(dest, orig, len);
}

template<>
static void __bcopy(WCHAR *dest, const char *orig, size_t len)
{
	MultiByteToWideChar(CallService("LangPack/GetCodePage", 0, 0), 0, orig, (int)len, dest, (int)len);
}

template<>
static void __bcopy(char *dest, const WCHAR *orig, size_t len)
{
	WideCharToMultiByte(CallService("LangPack/GetCodePage", 0, 0), 0, orig, (int)len, dest, (int)len, NULL, NULL);
}



template<class T>
class Buffer
{
	public:
		size_t len;
		T *str;

		Buffer() : str(NULL), size(0), len(0)
		{
			alloc(1);
			pack();
		}

		Buffer(T in) : str(NULL), size(0), len(0)
		{
			if (in == NULL)
			{
				alloc(1);
				pack();
			}
			else
			{
				str = in;
				size = len = __blen(str);
			}
		}

		~Buffer()
		{
			free();
		}

		void pack()
		{
			if (str != NULL)
				memset(&str[len], 0, sizeof(str[len]));
		}

		void alloc(size_t total)
		{
			if (total > size)
			{
				size = total + 256 - total % 256;
				if (str == NULL)
					str = (T *) mir_alloc(size * sizeof(T));
				else
					str = (T *) mir_realloc(str, size * sizeof(T));
			}
		}

		void free()
		{
			if (str != NULL)
			{
				mir_free(str);
				str = NULL;
				len = size = 0;
			}
		}

		void clear()
		{
			len = 0;
			pack();
		}

		void append(T app)
		{
			alloc(len + 1 + 1);

			str[len] = app;
			len++;
			pack();
		}

		void appendn(size_t n, T app)
		{
			alloc(len + n + 1);

			for (; n > 0; n--)
			{
				str[len] = app;
				len++;
			}
			pack();
		}

		void append(const char *app, size_t appLen = -1)
		{
			if (app == NULL)
				return;
			if (appLen == -1)
				appLen = __blen(app);

			size_t total = len + appLen + 1;
			alloc(total);

			__bcopy(&str[len], app, appLen);
			len += appLen;
			pack();
		}

		void append(const WCHAR *app, size_t appLen = -1)
		{
			if (app == NULL)
				return;
			if (appLen == -1)
				appLen = __blen(app);

			size_t total = len + appLen + 1;
			alloc(total);

			__bcopy(&str[len], app, appLen);
			len += appLen;
			pack();
		}

		void append(const Buffer<char> &app)
		{
			if (app.str == NULL)
				return;
			size_t appLen = app.len;

			size_t total = len + appLen + 1;
			alloc(total);

			__bcopy(&str[len], app.str, appLen);
			len += appLen;
			pack();
		}

		void append(const Buffer<WCHAR> &app)
		{
			size_t appLen = app.len;

			size_t total = len + appLen + 1;
			alloc(total);

			__bcopy(&str[len], app.str	, appLen);
			len += appLen;
			pack();
		}

		void appendPrintf(const T *app, ...)
		{
			size_t total = len + 512;
			alloc(total);

			va_list arg;
			va_start(arg, app);
			total = __bvsnprintf<T>(&str[len], size - len - 1, app, arg); //!!!!!!!!!!!!
			if (total < 0)
				total = size - len - 1;
			len += total;
			pack();
		}

		void insert(size_t pos, T *app, size_t appLen = -1)
		{
			if (pos > len)
				pos = len;
			if (pos < 0)
				pos = 0;

			if (appLen == -1)
				appLen = __blen(app);

			alloc(len + appLen + 1);

			if (pos < len)
				memmove(&str[pos + appLen], &str[pos], (len - pos) * sizeof(T));
			memmove(&str[pos], app, appLen * sizeof(T));

			len += appLen;
			pack();
		}

		void replace(size_t start, size_t end, T *app, size_t appLen = -1)
		{
			if (start > len)
				start = len;
			if (start < 0)
				start = 0;

			if (end > len)
				end = len;
			if (end < start)
				end = start;

			if (appLen == -1)
				appLen = __blen(app);

			size_t oldLen = end - start;
			if (oldLen < appLen)
				alloc(len + appLen - oldLen + 1);

			if (end < len && oldLen != appLen)
				memmove(&str[start + appLen], &str[end], (len - end) * sizeof(T));
			memmove(&str[start], app, appLen * sizeof(T));

			len += appLen - oldLen;
			pack();
		}

		void replaceAll(T find, T replace)
		{
			for(size_t i = 0; i < len; i++)
				if (str[len] == find)
					str[len] = replace;
			pack();
		}

		void translate()
		{
			if (str == NULL || len == 0)
				return;

			str[len] = 0;
			T *tmp = __bTranslate(str);
			len = __blen(tmp);
			alloc(len + 1);
			memmove(str, tmp, len * sizeof(T));
			pack();
		}

		void reverse()
		{
			for(size_t i = 0; i < len/2; i++)
			{
				T tmp = str[i];
				str[i] = str[len-i-1];
				str[len-i-1] = tmp;
			}
		}

		T *appender(size_t appLen)
		{
			alloc(len + appLen + 1);
			T *ret = &str[len];
			len += appLen;
			return ret;
		}

		T *lock(size_t maxSize)
		{
			alloc(len + maxSize + 1);
			return &str[len];
		}

		void release()
		{
			len += max(__blen(&str[len]), size - len - 1);
		}

		T *detach()
		{
			T *ret = str;
			str = NULL;
			len = 0;
			return ret;
		}

		void trimRight()
		{
			if (str == NULL)
				return;

			int e;
			for(e = len-1; e >= 0 && (str[e] == (T)' '
									  || str[e] == (T)'\t'
									  || str[e] == (T)'\r'
									  || str[e] == (T)'\n'); e--) ;
			len = e+1;
			pack();
		}

		void trimLeft()
		{
			if (str == NULL)
				return;

			int s;
			for(s = 0; str[s] == (T)' '
					   || str[s] == (T)'\t'
					   || str[s] == (T)'\r'
					   || str[s] == (T)'\n'; s++) ;
			if (s > 0)
			{
				memmove(str, &str[s], (len - s) * sizeof(T));
				len -= s;
			}
			pack();
		}

		void trim()
		{
			trimRight();
			trimLeft();
		}

		Buffer<T>& operator+=(const char *txt)
		{
			append(txt);
			return *this;
		}

		Buffer<T>& operator+=(const WCHAR *txt)
		{
			append(txt);
			return *this;
		}

		Buffer<T>& operator+=(const Buffer<T> &txt)
		{
			append(txt);
			return *this;
		}

		Buffer<T>& operator=(const char *txt)
		{
			clear();
			append(txt);
			return *this;
		}

		Buffer<T>& operator=(const WCHAR *txt)
		{
			clear();
			append(txt);
			return *this;
		}

		Buffer<T>& operator=(const Buffer<T> &txt)
		{
			clear();
			append(txt);
			return *this;
		}


	private:
		size_t size;
};


static void ReplaceVars(Buffer<TCHAR> *buffer, MCONTACT hContact, TCHAR **variables, int numVariables)
{
	if (buffer->len < 3)
		return;

	if (numVariables < 0)
		return;

	for(size_t i = buffer->len - 1; i > 0; i--)
	{
		if (buffer->str[i] == _T('%'))
		{
			// Find previous
			size_t j;
			for(j = i - 1; j > 0 && ((buffer->str[j] >= _T('a') && buffer->str[j] <= _T('z'))
								    || (buffer->str[j] >= _T('A') && buffer->str[j] <= _T('Z'))
									|| buffer->str[j] == _T('-')
									|| buffer->str[j] == _T('_')); j--) ;

			if (buffer->str[j] == _T('%'))
			{
				size_t foundLen = i - j + 1;
				if (foundLen == 9 && _tcsncmp(&buffer->str[j], _T("%contact%"), 9) == 0)
				{
					buffer->replace(j, i + 1, (TCHAR *) CallService(MS_CLIST_GETCONTACTDISPLAYNAME, hContact, GCDNF_TCHAR));
				}
				else if (foundLen == 6 && _tcsncmp(&buffer->str[j], _T("%date%"), 6) == 0)
				{
					TCHAR tmp[128];
					DBTIMETOSTRINGT tst = {0};
					tst.szFormat = _T("d s");
					tst.szDest = tmp;
					tst.cbDest = 128;
					CallService(MS_DB_TIME_TIMESTAMPTOSTRINGT, (WPARAM) time(NULL), (LPARAM) &tst);
					buffer->replace(j, i + 1, tmp);
				}
				else
				{
					for(int k = 0; k < numVariables; k += 2)
					{
						size_t len = mir_tstrlen(variables[k]);
						if (foundLen == len + 2 && _tcsncmp(&buffer->str[j]+1, variables[k], len) == 0)
						{
							buffer->replace(j, i + 1, variables[k + 1]);
							break;
						}
					}
				}
			}

			i = j;
			if (i == 0)
				break;
		}
		else if (buffer->str[i] == _T('\\') && i+1 <= buffer->len-1 && buffer->str[i+1] == _T('n'))
		{
			buffer->str[i] = _T('\r');
			buffer->str[i+1] = _T('\n');
		}
	}
}


static void ReplaceTemplate(Buffer<TCHAR> *out, MCONTACT hContact, TCHAR *templ, TCHAR **vars, int numVars)
{

	if (ServiceExists(MS_VARS_FORMATSTRING))
	{
		TCHAR *tmp = variables_parse_ex(templ, NULL, hContact, vars, numVars);
		if (tmp != NULL)
		{
			out->append(tmp);
			mir_free(tmp);
			out->pack();
			return;
		}
	}

	out->append(templ);
	ReplaceVars(out, hContact, vars, numVars);
	out->pack();
}


#endif // __MIR_BUFFER_H__
