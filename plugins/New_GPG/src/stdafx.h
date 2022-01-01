// Copyright © 2010-22 sss
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#ifndef COMMONHEADERS_H
#define COMMONHEADERS_H

#pragma warning(disable:4512 4267 4127)

#define WIN32_LEAN_AND_MEAN
#define _SCL_SECURE_NO_WARNINGS

#include <io.h>

// windows
#include <windows.h>
#include <commdlg.h>
#include <shlobj.h>
#include <shlwapi.h>

// c++
#include <map>
using std::map;
#include <list>
using std::list;
#include <string>
using std::string;
using std::wstring;
#include <fstream>
using std::wfstream;
using std::fstream;

// boost
#include <boost/nondet_random.hpp>
#include <boost/random/variate_generator.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/date_time.hpp>
#include <boost/iostreams/stream.hpp>

// boost process
#include <boost/process.hpp>
#include <boost/process/windows.hpp>

// miranda
#include <newpluginapi.h>
#include <m_contacts.h>
#include <m_database.h>
#include <m_options.h>
#include <m_langpack.h>
#include <m_skin.h>
#include <m_jabber.h>
#include <m_message.h>
#include <m_clist.h>
#include <m_cluiframes.h>
#include <m_icolib.h>
#include <m_extraicons.h>
#include <m_gui.h>

#include <m_metacontacts.h>

struct CMPlugin : public PLUGIN<CMPlugin>
{
	CMOption<bool> bJabberAPI, bPresenceSigning, bFileTransfers, bAutoExchange, bSameAction, bAppendTags, bStripTags, bDebugLog, bSendErrorMessages;
	HANDLE hCLIcon = nullptr;
	HGENMENU hToggleEncryption = nullptr, hSendKey = nullptr;

	CMPlugin();

	int Load() override;
	int Unload() override;
};

// internal
#include "resource.h"
#include "version.h"
#include "constants.h"
#include "log.h"
#include "utilities.h"
#include "gpg_wrapper.h"
#include "jabber_account.h"
#include "metacontacts.h"
#include "ui.h"
#include "options.h"
#include "globals.h"

#endif
