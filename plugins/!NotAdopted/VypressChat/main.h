/*
 * Miranda-IM Vypress Chat/quickChat plugins
 * Copyright (C) Saulius Menkevicius
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * $Id: main.h,v 1.19 2005/03/08 16:53:20 bobas Exp $
 */

#include "miranda.h"
#include "version.h"

#ifdef VYPRESSCHAT
# define VQCHAT_PROTO		"VYPRESSCHAT"
# define VQCHAT_PROTO_NAME	"Vypress Chat"
# define VQCHAT_PROTO_DLL	"vypresschat.dll"
#endif

#ifdef QUICKCHAT
# define VQCHAT_PROTO		"QUICKCHAT"
# define VQCHAT_PROTO_NAME	"QuickChat"
# define VQCHAT_PROTO_DLL	"quickchat.dll"
#endif

#ifdef VYPRESSCHAT
# define VQCHAT_VQP_PROTO	VQP_PROTOCOL_VYPRESSCHAT
# define VQCHAT_VQP_DEF_PORT	8167
# define VQCHAT_VQP_DEF_PROTO_OPT 0
# define VQCHAT_VQP_SWVERSION	VQP_MAKE_SWVERSION(1, 93)
# define VQCHAT_VQP_DEF_COLOR	0
# define VQCHAT_VQP_COMPAT_CODEPAGE VQP_CODEPAGE_UTF8
# define VQCHAT_UNDEF_SWVERSION	VQP_MAKE_SWVERSION(1, 50)
#endif

#ifdef QUICKCHAT
# define VQCHAT_VQP_PROTO	VQP_PROTOCOL_QUICKCHAT
# define VQCHAT_VQP_DEF_PORT	8167
# define VQCHAT_VQP_DEF_PROTO_OPT 0
# define VQCHAT_VQP_SWVERSION	VQP_MAKE_SWVERSION(1, 5)
# define VQCHAT_VQP_DEF_COLOR	0
# define VQCHAT_VQP_COMPAT_CODEPAGE VQP_CODEPAGE_LOCALE
# define VQCHAT_UNDEF_SWVERSION	VQP_MAKE_SWVERSION(1, 5)
#endif

#define VQCHAT_VQP_DEF_MULTICAST 0xe3000002
#define VQCHAT_VQP_DEF_SCOPE	7
#define VQCHAT_VQP_SWPLATFORM	"Windows"
#define VQCHAT_VQP_SWNAME	"Miranda IM, http://www.miranda-im.org/"
#define VQCHAT_MAIN_CHANNEL	"Main"
#define VQCHAT_UNDEF_ACTIVE	VQP_ACTIVE_ACTIVE

#define VQCHAT_MIN_REFRESH_TIMEOUT 1
#define VQCHAT_MAX_REFRESH_TIMEOUT 254
#define VQCHAT_DEF_REFRESH_TIMEOUT 15

extern HINSTANCE	g_hDllInstance;
extern HANDLE		g_hMainThread;
extern PLUGINLINK * pluginLink;

/* debuging allocator
 * (watch for "memwatch.log" in miranda's dir !!)
 */
#ifdef MEMWATCH
# include "contrib/memwatch.h"
#endif

/* debug traps, messages, asserts
 */
#ifndef NDEBUG
# define DEBUG_TRAP() asm volatile ("int $3");
# define DEBUG_MSG(...) \
	do {	char message[256], formatted[192];		\
		sprintf(formatted, __VA_ARGS__);		\
		sprintf(message, "(%s:%s():%d): %s",			\
			__FILE__, __FUNCTION__, __LINE__, formatted);	\
		PUShowMessage(message, SM_NOTIFY);			\
	} while(0);
		
#else
# define DEBUG_TRAP()
# define DEBUG_MSG(...)
#endif

#define VALIDPTR(p) ((unsigned long)(p) >= 0x1000)

#define ASSERT_RETURNIFFAIL(assert) \
	if(! (assert)) {	\
		DEBUG_TRAP();	\
		return; 	\
	}
#define ASSERT_RETURNVALIFFAIL(assert, val) \
	if(! (assert)) {	\
		DEBUG_TRAP();	\
		return (val);	\
	}

