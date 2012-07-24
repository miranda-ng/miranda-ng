/* 
Copyright (C) 2009-2010 Ricardo Pescuma Domenecci

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

#ifndef __ARDIALOG_H__
# define __ARDIALOG_H__


typedef void (*AutoReplaceDialogCallback)(BOOL canceled, Dictionary *dict, 
										  const TCHAR *find, const TCHAR *replace, BOOL useVariables, 
										  const TCHAR *original_find, void *param);

BOOL ShowAutoReplaceDialog(HWND parent, BOOL modal, 
						   Dictionary *dict, const TCHAR *find, const TCHAR *replace, BOOL useVariables,
						   BOOL findReadOnly, AutoReplaceDialogCallback callback, void *param);


#endif // __ARDIALOG_H__
