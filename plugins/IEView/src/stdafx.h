/*

IEView Plugin for Miranda IM
Copyright (C) 2005-2010  Piotr Piastucki

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

#ifndef IEVIEW_COMMON_H
#define IEVIEW_COMMON_H

#define _CRT_NONSTDC_NO_DEPRECATE

#include <windows.h>
#include <commctrl.h>
#include <time.h>
#include <process.h>
#include <shlguid.h>
#include <mshtml.h>

#include <msapi/comptr.h>

#include <io.h>
#include <fcntl.h>

#include <newpluginapi.h>
#include <m_avatars.h>
#include <m_chat_int.h>
#include <m_clistint.h>
#include <m_contacts.h>
#include <m_database.h>
#include <m_icolib.h>
#include <m_ieview.h>
#include <m_langpack.h>
#include <m_message.h>
#include <m_metacontacts.h>
#include <m_netlib.h>
#include <m_options.h>
#include <m_protosvc.h>
#include <m_skin.h>
#include <m_smileyadd.h>
#include <m_srmm_int.h>
#include <m_timezones.h>

#include "IEView.h"
#include "Utils.h"
#include "ieview_services.h"
#include "Options.h"
#include "version.h"
#include "HistoryHTMLBuilder.h"
#include "HTMLBuilder.h"
#include "resource.h"
#include "ScriverHTMLBuilder.h"
#include "TabSRMMHTMLBuilder.h"
#include "TemplateHTMLBuilder.h"
#include "Template.h"
#include "TextToken.h"
#include "external_funcs.h"

#define MODULENAME "IEVIEW"

struct CMPlugin : public PLUGIN<CMPlugin>
{
	CMPlugin();

	HANDLE hLogger;

	int Load() override;
	int Unload() override;
};

CSrmmLogWindow *logBuilder(CMsgDialog &pDlg);

extern IEView *debugView;
extern char *workingDirUtf8;
extern HANDLE hHookOptionsChanged;
extern IconItem iconList[];

class BSTR_PTR
{
	BSTR _data;
public:
	BSTR_PTR(const wchar_t *src) : _data(SysAllocString(src)) {}
	~BSTR_PTR() { SysFreeString(_data); }
	size_t length() { return SysStringLen(_data); }
	operator BSTR () { return _data; }
};

#endif
