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


#include "mir_dblists.h"

#include <stdio.h>

#include <newpluginapi.h>
#include <m_system.h>


void List_DestroyFreeContents( SortedList* p_list )
{
	if ( p_list == NULL )
		return;

	if ( p_list->items != NULL )
	{
		int i;
		for ( i = 0 ; i < p_list->realCount ; i++ )
		{
			if ( p_list->items[i] != NULL )
			{
				mir_free( p_list->items[i] );
			}
		}
	}

	List_Destroy( p_list );
}


int List_Append( SortedList* p_list, void* p_value )
{
	return List_Insert( p_list, p_value, p_list->realCount );
}


int List_InsertOrdered( SortedList* p_list, void* p_value )
{
	int index;

	List_GetIndex( p_list, p_value, &index );
	List_Insert( p_list, p_value, index );

	return index;
}


int List_RemoveByValue( SortedList* p_list, void* p_value )
{
	int ret = 0;

	if ( p_list->items != NULL )
	{
		int i;
		for ( i = p_list->realCount - 1 ; i >= 0 ; i-- )
		{
			if ( p_list->items[ i ] == p_value )
				ret += List_Remove( p_list, i );
		}
	}

	return ret;
}


int List_RemoveByValueFreeContents( SortedList* p_list, void* p_value )
{
	int ret = 0;

	if ( p_list->items != NULL )
	{
		int i;
		for ( i = p_list->realCount - 1 ; i >= 0 ; i-- )
		{
			if ( p_list->items[ i ] == p_value )
			{
				mir_free( p_list->items[ i ] );
				ret += List_Remove( p_list, i );
			}
		}
	}

	return ret;
}


void List_Push( SortedList* p_list, void* p_value )
{
	List_Insert( p_list, p_value, p_list->realCount );
}


void* List_Pop( SortedList* p_list )
{
	void *ret;

	if ( p_list->realCount <= 0 )
		return NULL;

	ret = p_list->items[ p_list->realCount - 1 ];
	List_Remove( p_list, p_list->realCount - 1 );

	return ret;
}


void* List_Peek( SortedList* p_list )
{
	if ( p_list->realCount <= 0 )
		return NULL;

	return p_list->items[ p_list->realCount - 1 ];
}


BOOL List_HasItens( SortedList* p_list )
{
	return p_list->realCount > 0;
}