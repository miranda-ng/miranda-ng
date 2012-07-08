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

void HandleStatusCommand(PCommand command, TArgument *argv, int argc, PReply reply);
void HandleAwayMsgCommand(PCommand command, TArgument *argv, int argc, PReply reply);
void HandlePopupsCommand(PCommand command, TArgument *argv, int argc, PReply reply);
void HandleSoundsCommand(PCommand command, TArgument *argv, int argc, PReply reply);
void HandleClistCommand(PCommand command, TArgument *argv, int argc, PReply reply);
void HandleQuitCommand(PCommand command, TArgument *argv, int argc, PReply reply);
void HandleExchangeCommand(PCommand command, TArgument *argv, int argc, PReply reply);
void HandleYAMNCommand(PCommand command, TArgument *argv, int argc, PReply reply);
void HandleCallServiceCommand(PCommand command, TArgument *argv, int argc, PReply reply);
void HandleMessageCommand(PCommand command, TArgument *argv, int argc, PReply reply);
void HandleDatabaseCommand(PCommand command, TArgument *argv, int argc, PReply reply);
void HandleProxyCommand(PCommand command, TArgument *argv, int argc, PReply reply);
void HandleContactsCommand(PCommand command, TArgument *argv, int argc, PReply reply);
void HandleHistoryCommand(PCommand command, TArgument *argv, int argc, PReply reply);
void HandleVersionCommand(PCommand command, TArgument *argv, int argc, PReply reply);
void HandleSetNicknameCommand(PCommand command, TArgument *argv, int argc, PReply reply);
void HandleIgnoreCommand(PCommand command, TArgument *argv, int argc, PReply reply);

#endif //M_MIMCMD_HANDLERS_H