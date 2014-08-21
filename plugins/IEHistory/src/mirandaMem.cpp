/*
IEView history viewer plugin for Miranda IM

Copyright © 2005-2006 Cristian Libotean

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

#include "mirandaMem.h"

void *(*MirandaMalloc)(size_t size) = NULL;
void *(*MirandaRealloc)(void *data, size_t newSize) = NULL; 
void (*MirandaFree) (void *data) = NULL;

void InitializeMirandaMemFunctions()
{
	MM_INTERFACE mmi = {0};
	mmi.cbSize = sizeof(MM_INTERFACE);
	CallService(MS_SYSTEM_GET_MMI, 0, (LPARAM) &mmi);
	MirandaMalloc = mmi.mmi_malloc;
	MirandaRealloc = mmi.mmi_realloc;
	MirandaFree = mmi.mmi_free;	
}

void DestroyMirandaMemFunctions()
{
	MirandaMalloc = NULL;
	MirandaRealloc = NULL;
	MirandaFree = NULL;
}