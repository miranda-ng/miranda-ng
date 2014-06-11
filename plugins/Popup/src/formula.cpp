/*
Popup Plus plugin for Miranda IM

Copyright	© 2002 Luca Santarelli,
			© 2004-2007 Victor Pavlychko
			© 2010 MPK
			© 2010 Merlin_de

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

#include "headers.h"

static inline bool myisspace(char ch)
{
	return (ch >= 0) && (ch <= 32);
}

int Formula::eval_neq(TCHAR *&s, Args *args, bool *vars) const
{
	int left = eval_sum(s, args, vars);
	while (*s)
	{
		if (myisspace(*s))
		{
			++s;
		} else
		if (*s == '<')
		{
			// this is needed due to side effects caused by min() macro...
			int tmp = eval_sum(++s, args, vars);
			left = min(left, tmp);
		} else
		if (*s == '>')
		{
			// this is needed due to side effects caused by max() macro...
			int tmp = eval_sum(++s, args, vars);
			left = max(left, tmp);
		} else
		{
			break;
		}
	}
	return left;
}

int Formula::eval_sum(TCHAR *&s, Args *args, bool *vars) const
{
	int left = eval_mul(s, args, vars);
	while (*s)
	{
		if (myisspace(*s))
		{
			++s;
		} else
		if (*s == '+')
		{
			left += eval_mul(++s, args, vars);
		} else
		if (*s == '-')
		{
			left -= eval_mul(++s, args, vars);
		} else
		{
			break;
		}
	}
	return left;
}

int Formula::eval_mul(TCHAR *&s, Args *args, bool *vars) const
{
	int left = eval_atom(s, args, vars);
	while (*s)
	{
		if (myisspace(*s))
		{
			++s;
		} else
		if (*s == '*')
		{
			left *= eval_atom(++s, args, vars);
		} else
		if (*s == '/')
		{
			if (int right = eval_atom(++s, args, vars))
				left /= right;
		} else
		if (*s == '%')
		{
			if (int right = eval_atom(++s, args, vars))
				left %= right;
		} else
		{
			break;
		}
	}
	return left;
}

int Formula::eval_atom(TCHAR *&s, Args *args, bool *vars) const
{
	while (*s)
	{
		if (myisspace(*s))
		{
			++s;
		} else
		if (*s == '(')
		{
			int res = eval_neq(++s, args, vars);
			if (*s == ')')
				++s;
			return res;
		} else
		if ((*s == '+') || (*s == '-'))
		{
			int sign = 1;
			if (*s == '-')
				sign = -1;
			return sign * eval_neq(++s, args, vars);
		} else
		if (*s == '!')
		{
			return !eval_neq(++s, args, vars);
		} else
		if ((*s >= '0') && (*s <= '9'))
		{
			int res = 0;
			while ((*s >= '0') && (*s <= '9'))
				res = res * 10 + *s++ - '0';
			return res;
		} else
		{
			if (!args)
				return 0;
			char buf[1024];
			char *bufptr = buf;
			while (((*s >= '0') && (*s <= '9')) || ((*s >= 'a') && (*s <= 'z')) || ((*s >= 'A') && (*s <= 'A')) || (*s == '_') || (*s == '.'))
				*bufptr++ = *s++;
			*bufptr = 0;
			int res = args->get(buf);
			if (vars) *vars = true;
			return res;
		}
	}
	return 0;
}

int Formula::eval(Args *args, bool *vars) const
{
	if (vars) *vars = false;
	TCHAR *s = m_str;
	int res = eval_neq(s, args, vars);
	return res;
}
