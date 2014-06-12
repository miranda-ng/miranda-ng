// Copyright © 2010-2012 sss
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

#define _CRT_SECURE_NO_WARNINGS
#define _SCL_SECURE_NO_WARNINGS

//windows
#include <windows.h>
#include <shlobj.h>
#include <io.h>
#include <shlwapi.h>

//c++
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

//boost
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/nondet_random.hpp>
#include <boost/random/variate_generator.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/date_time.hpp>
#include <boost/iostreams/stream.hpp>

//boost process
#include <boost/process.hpp>

//utf8cpp
#include <utf8.h>

//miranda
#include <newpluginapi.h>
#include <m_database.h>
#include <m_options.h>
#include <m_langpack.h>
#include <m_clui.h>
#include <m_skin.h>
#include <m_protomod.h>
#include <m_jabber.h>
#include <m_icq.h>
#include <m_message.h>
#include <m_cluiframes.h>
#include <m_icolib.h>
#include <m_extraicons.h>
#include <win2k.h>

#include <m_metacontacts.h>

//internal
#include "resource.h"
#include "Version.h"
#include "constants.h"
#include "log.h"
#include "globals.h"
#include "utilities.h"
#include "main.h"
#include "gpg_wrapper.h"
#include "jabber_account.h"
#include "metacontacts.h"

#endif
