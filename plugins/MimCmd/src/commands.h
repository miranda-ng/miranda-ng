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

#ifndef M_MIMCMD_COMMANDS_H
#define M_MIMCMD_COMMANDS_H

extern PCommand knownCommands;
extern int cKnownCommands;

typedef void (* LISTCOMMANDS)(PCommand *commands, int *count);

extern LISTCOMMANDS ListCommands;

char *GetMirandaFolder();

int ConnectToMiranda();
int DisconnectFromMiranda();
int GetKnownCommands();
int DestroyKnownCommands();

PCommand GetCommand(char *command);
PReply ParseCommand(char *argv[], int argc);
void ProcessConsoleCommand(PCommand command, char *arguments[], int count, PReply reply);

#endif