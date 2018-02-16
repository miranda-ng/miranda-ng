/*
CmdLine plugin for Miranda IM

Copyright © 2007 Cristian Libotean

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

#include "stdafx.h"

TCommand mimCommands[] =
{
	{ MIMCMD_STATUS,      L"status",      MIMCMD_HELP_STATUS      },
	{ MIMCMD_AWAYMSG,     L"awaymsg",     MIMCMD_HELP_AWAYMSG     },
	{ MIMCMD_XSTATUS,     L"xstatus",     MIMCMD_HELP_XSTATUS     },
	{ MIMCMD_POPUPS,      L"popups",      MIMCMD_HELP_POPUPS      },
	{ MIMCMD_SOUNDS,      L"sounds",      MIMCMD_HELP_SOUNDS      },
	{ MIMCMD_CLIST,       L"clist",       MIMCMD_HELP_CLIST       },
	{ MIMCMD_QUIT,        L"quit",        MIMCMD_HELP_QUIT        },
	{ MIMCMD_HELP,        L"help",        MIMCMD_HELP_HELP        },
	{ MIMCMD_EXCHANGE,    L"exchange",    MIMCMD_HELP_EXCHANGE    },
	{ MIMCMD_YAMN,        L"yamn",        MIMCMD_HELP_YAMN        },
	{ MIMCMD_CALLSERVICE, L"callservice", MIMCMD_HELP_CALLSERVICE },
	{ MIMCMD_MESSAGE,     L"message",     MIMCMD_HELP_MESSAGE     },
	{ MIMCMD_DATABASE,    L"db",          MIMCMD_HELP_DATABASE    },
	{ MIMCMD_PROXY,       L"proxy",       MIMCMD_HELP_PROXY       },
	{ MIMCMD_CONTACTS,    L"contacts",    MIMCMD_HELP_CONTACTS    },
	{ MIMCMD_HISTORY,     L"history",     MIMCMD_HELP_HISTORY     },
	{ MIMCMD_VERSION,     L"version",     MIMCMD_HELP_VERSION     },
	{ MIMCMD_SETNICKNAME, L"setnickname", MIMCMD_HELP_SETNICKNAME },
	{ MIMCMD_IGNORE,      L"ignore",      MIMCMD_HELP_IGNORE      },
	{ MIMCMD_LUA,         L"lua",         MIMCMD_HELP_LUA },
};

int cMimCommands = _countof(mimCommands);