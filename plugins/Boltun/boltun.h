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

//Should use different keys in UNICODE and ANSI builds, because usage of plugin
//with old (another) configs will cause crashes.
#ifdef _UNICODE
#define BOLTUN_KEY  "BoltunW"
#else
#define BOLTUN_KEY  "Boltun"
#endif

//Service names
#define SERV_CONTACT_AUTO_CHAT "Boltun/ContactAutoChat"
#define SERV_CONTACT_NOT_TO_CHAT "Boltun/ContactNotToChat"
#define SERV_CONTACT_START_CHATTING "Boltun/ContactStartChatting"

//Database keys
#define DB_CONTACT_BOLTUN_NOT_TO_CHAT "BoltunNotToChat"
#define DB_CONTACT_BOLTUN_AUTO_CHAT "BoltunAutoChat"
#define DB_CONTACT_WARNED "Warned"

//Plugin group in settings
#define BOLTUN_GROUP "Message sessions"

//Filename depends on UNICODE
#ifdef UNICODE
#define DEFAULT_MIND_FILE _T("boltun.mindw")
#else
#define DEFAULT_MIND_FILE _T("boltun.mind")
#endif

//===============================================
// These are strings for translation:
//===============================================

//Plugin name
#define BOLTUN_NAME  "Boltun"

#define PLUGIN_DESCRIPTION "Boltun, the chat bot in the russian language."

#define MIND_FILE_DESC _T("Mind Files")
#define ALL_FILES_DESC _T("All Files")

//UI strings
#define BOLTUN_AUTO_CHAT   "Boltun/Auto Chat"
#define BOLTUN_NOT_TO_CHAT "Boltun/Not to Chat"
#define BOLTUN_START_CHATTING "Boltun/Start Chatting"

#define DEFAULT_WARN_TEXT _T("Hello. I'm Boltun! I'll talk to you, while my owner is away. Please write without mistakes!")

// Error messages
#define BOLTUN_ERROR         _T("Boltun Error")
#define FAILED_TO_LOAD_BASE  _T("Failed to load base of remarks. Error at line %d of %s. (Or few lines before).")
#define FAILED_TO_SAVE_BASE  _T("Failed to save base of remarks to %s")

//Settings tab names
#define TAB_GENERAL "General Settings"
#define TAB_ENGINE "Engine Settings"

#endif /*_BOLTUN_H*/
