/*
Copyright (C) 2005 Ricardo Pescuma Domenecci

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


#ifndef __MIR_DBLISTS_H__
# define __MIR_DBLISTS_H__

#include <windows.h>
#include <newpluginapi.h>
#include <m_system.h>



extern struct LIST_INTERFACE list_interface;


// Need to be called on ME_SYSTEM_MODULESLOADED
void init_list_interface();


#define List_Create(x,y)		(list_interface.List_Create(x,y))
#define List_Destroy(x)			(list_interface.List_Destroy(x))
#define List_Find(x,y)			(list_interface.List_Find(x,y))
#define List_GetIndex(x,y,z)	(list_interface.List_GetIndex(x,y,z))
#define List_Insert(x,y,z)		(list_interface.List_Insert(x,y,z))
#define List_Remove(x,y)		(list_interface.List_Remove(x,y))
#define List_IndexOf(x,y)		(list_interface.List_IndexOf(x,y))


BOOL List_HasItens( SortedList* p_list );

void List_DestroyFreeContents( SortedList* );
int List_RemoveByValue( SortedList*, void* );
int List_RemoveByValueFreeContents( SortedList*, void* );

int List_Append( SortedList*, void* );
int List_InsertOrdered( SortedList*, void* );

// Theese work on the end of the list
void List_Push( SortedList* p_list, void* p_value );
void* List_Pop( SortedList* p_list );
void* List_Peek( SortedList* p_list );



#endif // __MIR_DBLISTS_H__
