//***********************************************************
//	Copyright © 2003-2008 Alexander S. Kiselev, Valentin Pavlyuchenko
//
//	This file is part of Boltun.
//
//    Boltun is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 2 of the License, or
//    (at your option) any later version.
//
//    Boltun is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//	  along with Boltun. If not, see <http://www.gnu.org/licenses/>.
//
//***********************************************************

#ifndef _BOLTUN_H
#define _BOLTUN_H

#include <windows.h>
#include <time.h>
#include <fstream>
#include <assert.h>

#include <string>
#include <list>

#include "newpluginapi.h"
#include "m_clist.h"
#include "m_system_cpp.h"
#include "m_database.h"
#include "m_options.h"
#include "m_langpack.h"

#include "resource.h"
#include "version.h"
#include "config.h"
#include "actionQueue.h"
#include "Engine/TalkEngine.h"
#include "Engine/WordsList.h"
#include "Engine/UnrecentChooser.h"
#include "Engine/MyCodeCvt.h"
#include "Engine/Mind.h"

#define BOLTUN_KEY  "Boltun"

//Service names
#define SERV_CONTACT_AUTO_CHAT "Boltun/ContactAutoChat"
#define SERV_CONTACT_NOT_TO_CHAT "Boltun/ContactNotToChat"
#define SERV_CONTACT_START_CHATTING "Boltun/ContactStartChatting"

//Database keys
#define DB_CONTACT_BOLTUN_NOT_TO_CHAT "BoltunNotToChat"
#define DB_CONTACT_BOLTUN_AUTO_CHAT "BoltunAutoChat"
#define DB_CONTACT_WARNED "Warned"

//Plugin group in settings
#define BOLTUN_GROUP LPGEN("Message sessions")

//Filename depends on UNICODE
#define DEFAULT_MIND_FILE _T("boltun.mindw")

//===============================================
// These are strings for translation:
//===============================================

//Plugin name
#define BOLTUN_NAME  "Boltun"

#define MIND_FILE_DESC LPGENT("Mind Files")
#define ALL_FILES_DESC LPGENT("All Files")

//UI strings
#define BOLTUN_AUTO_CHAT   LPGEN("Boltun/Auto Chat")
#define BOLTUN_NOT_TO_CHAT LPGEN("Boltun/Not to Chat")
#define BOLTUN_START_CHATTING LPGEN("Boltun/Start Chatting")

#define DEFAULT_WARN_TEXT LPGENT("Hello. I'm Boltun! I'll talk to you, while my owner is away. Please write without mistakes!")

// Error messages
#define BOLTUN_ERROR         LPGENT("Boltun Error")
#define FAILED_TO_LOAD_BASE  LPGENT("Failed to load base of remarks. Error at line %d of %s. (Or few lines before).")
#define FAILED_TO_SAVE_BASE  LPGENT("Failed to save base of remarks to %s")

//Settings tab names
#define TAB_GENERAL LPGEN("General Settings")
#define TAB_ENGINE LPGEN("Engine Settings")

#endif /*_BOLTUN_H*/
