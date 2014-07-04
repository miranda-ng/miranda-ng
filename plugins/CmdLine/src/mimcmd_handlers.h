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

#ifndef M_MIMCMD_HANDLERS_H
#define M_MIMCMD_HANDLERS_H

#include "mimcmd_data.h"

//aditional services
#define MS_EXCHANGE_CHECKEMAIL "Exchange/CheckEmail"
#define MS_YAMN_FORCECHECK  "YAMN/Service/ForceCheck"

extern PCommand GetCommand(char *command);

extern HANDLE heServerBufferFull;
extern PLUGININFOEX pluginInfo;

void HandleCommand(PCommand command, TArgument *argv, int argc, PReply reply);

#endif //M_MIMCMD_HANDLERS_H