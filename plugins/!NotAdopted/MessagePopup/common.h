/*

MessagePopup - replacer of MessageBox'es

Copyright 2004 Denis Stanishevskiy

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
#include"AggressiveOptimize.h"
#include <windows.h>

#include "../../SDK/headers_c/newpluginapi.h"

#include "../../SDK/headers_c/m_system.h"
#include "../../SDK/headers_c/m_database.h"
#include "../../SDK/headers_c/m_langpack.h"
#include "../../SDK/headers_c/m_clist.h"
#include "../../SDK/headers_c/m_options.h"
#include "../../SDK/headers_c/m_utils.h"
#include "../../SDK/headers_c/m_popup.h"

#include "options.h"
#include "resource.h"

#define SERVICENAME "MessagePopup"

struct MSGBOXOPTIONS
{
	COLORREF FG[4];
	COLORREF BG[4];
	int Timeout[4];
	BOOL Sound;
};

extern MSGBOXOPTIONS options;
extern MSGBOXOPTIONS optionsDefault;

void LoadConfig();

#ifdef __cplusplus
extern "C" {
#endif 

int __declspec(dllexport) Load( PLUGINLINK *link );
int __declspec(dllexport) Unload( void );
__declspec(dllexport) PLUGININFO *MirandaPluginInfo( DWORD dwVersion );

#ifdef __cplusplus
}
#endif /* __cplusplus */
