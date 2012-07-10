/* 
Copyright (C) 2008 Ricardo Pescuma Domenecci

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


#ifndef __COMMONS_H__
# define __COMMONS_H__


#define OEMRESOURCE 
#define _WIN32_WINNT 0x0400
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <time.h>
#include <commctrl.h>


// Disable "...truncated to '255' characters in the debug information" warnings
#pragma warning(disable: 4786)

#include <vector>
#include <string>
using namespace std;


// Miranda headers
#define MIRANDA_VER 0x0800
#include <win2k.h>
#include <newpluginapi.h>
#include <m_system.h>
#include <m_system_cpp.h>
#include <m_protocols.h>
#include <m_protosvc.h>
#include <m_clist.h>
#include <m_contacts.h>
#include <m_langpack.h>
#include <m_database.h>
#include <m_options.h>
#include <m_utils.h>
#include <m_updater.h>
#include <m_metacontacts.h>
#include <m_popup.h>
#include <m_history.h>
#include <m_message.h>
#include <m_folders.h>
#include <m_icolib.h>
#include <m_imgsrvc.h>
#include <m_netlib.h>
#include <m_fontservice.h>

#include <mir_memory.h>
#include <mir_options.h>
#include <mir_icons.h>
#include <mir_buffer.h>
#include <utf8_helpers.h>

#include "resource.h"
#include "m_skins.h"
#include "m_skins_cpp.h"
#include "options.h"
#include "MirandaSkinnedDialog.h"
#include "MirandaField.h"
#include "MirandaTextField.h"
#include "MirandaIconField.h"
#include "MirandaImageField.h"


#define MODULE_NAME		"Skins"

#define DEFAULT_SKIN_NAME "Default"
#define SKIN_EXTENSION "msk"


// Global Variables
extern HINSTANCE hInst;
extern PLUGINLINK *pluginLink;
extern FI_INTERFACE *fei;
extern HANDLE hChangedEvent;

#define MAX_REGS(_A_) ( sizeof(_A_) / sizeof(_A_[0]) )
#define MIR_FREE(_X_) if (_X_ != NULL) { mir_free(_X_); _X_ = NULL; }


extern TCHAR skinsFolder[1024];

extern std::vector<MirandaSkinnedDialog *> dlgs;

void getSkinnedDialogFilename(std::tstring &ret, const TCHAR *skin, const char *dialogName);
void getAvaiableSkins(std::vector<std::tstring> &skins, MirandaSkinnedDialog *dlg = NULL);


// See if a protocol service exists
static __inline int ProtoServiceExists(const char *szModule,const char *szService)
{
	char str[MAXMODULELABELLENGTH];
	strcpy(str,szModule);
	strcat(str,szService);
	return ServiceExists(str);
}



#endif // __COMMONS_H__
