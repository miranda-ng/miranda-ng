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

===============================================================================

File name      : $HeadURL: http://svn.miranda.im/mainrepo/popup/trunk/src/formula.h $
Revision       : $Revision: 1610 $
Last change on : $Date: 2010-06-23 00:55:13 +0300 (Ð¡Ñ€, 23 Ð¸ÑŽÐ½ 2010) $
Last change by : $Author: Merlin_de $

===============================================================================
*/

#ifndef __formula_h__
#define __formula_h__

class Formula
{
public:
	class Args
	{
	private:
		struct Item
		{
			char *name;
			int value;
			Item *next;
			
			Item(char *aName, int aValue, Item *aNext): value(aValue), next(aNext) { name = aName ? mir_strdup(aName) : 0; }
			~Item() { if (name) mir_free(name); }
		};
		Item *items;
	public:
		Args():items(0){}
		~Args(){clear();}
		void	add(char *name, int value)
		{
			for (Item *p = items; p; p = p->next)
				if (!lstrcmpA(p->name,name))
				{
					p->value = value;
					return;
				}
			items = new Item(name, value, items);
		}
		int		get(char *name)
		{
			for (Item *p = items; p; p = p->next)
				if (!lstrcmpA(p->name,name))
					return p->value;
			return 0;
		}
		void	clear()
		{
			while (items)
			{
				Item *p = items->next;
				delete items;
				items = p;
			}
		}
	};

private:
	char *m_str;
	int eval_neq (char *&s, Args *args, bool *vars) const;
	int eval_sum (char *&s, Args *args, bool *vars) const;
	int eval_mul (char *&s, Args *args, bool *vars) const;
	int eval_atom(char *&s, Args *args, bool *vars) const;

public:
	Formula():m_str(mir_strdup("")){}
	Formula(char *s):m_str(mir_strdup(s)){}
	~Formula() {mir_free(m_str);}
	void	set(char *s){mir_free(m_str);m_str=mir_strdup(s);}
	int		eval(Args *args, bool *vars = 0) const;
};

#endif // __formula_h__