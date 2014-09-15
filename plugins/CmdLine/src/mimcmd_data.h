/*
CmdLine plugin for Miranda IM

Copyright � 2007 Cristian Libotean

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

#ifndef M_MIMCMD_DATA_H
#define M_MIMCMD_DATA_H

#define COMMAND_SIZE  64
#define HELP_SIZE   1024
#define REPLY_SIZE  8096

#define MIMFOLDER_SIZE 300

#define MAX_ARGUMENTS  20
#define ARGUMENT_SIZE 512

typedef char TArgument[ARGUMENT_SIZE];

struct TReply{
	int code;
	char message[REPLY_SIZE];
	static const int cMessage = REPLY_SIZE;
};

typedef TReply *PReply;

struct TCommand{
	long ID;
	char command[COMMAND_SIZE];
	char help[HELP_SIZE];
};

typedef TCommand *PCommand;

struct TSharedData{
	char mimFolder[MIMFOLDER_SIZE];
	TCommand command;
	TArgument arguments[MAX_ARGUMENTS];
	int cArguments;
	TReply reply;
	int instances;
};

typedef TSharedData *PSharedData;

//return codes
#define MIMRES_SUCCESS          0 //command was successful
#define MIMRES_FAILURE          1 //command was not successful
#define MIMRES_NOTFOUND         2 //command not found
#define MIMRES_UNKNOWNPARAM     3 //command parameter is unknown
#define MIMRES_NOMIRANDA        4 //could not link to Miranda
#define MIMRES_WRONGPARAMSCOUNT 5 //wrong number of parameters


#define MIMCMD_UNKNOWN     -1 //unknown command
#define MIMCMD_STATUS       1
#define MIMCMD_AWAYMSG      2
#define MIMCMD_XSTATUS      3
#define MIMCMD_POPUPS       4
#define MIMCMD_SOUNDS       5
#define MIMCMD_CLIST        6
#define MIMCMD_QUIT         7
#define MIMCMD_HELP         8
#define MIMCMD_EXCHANGE     9
#define MIMCMD_YAMN        10
#define MIMCMD_CALLSERVICE 11
#define MIMCMD_MESSAGE     12
#define MIMCMD_DATABASE    13
#define MIMCMD_PROXY       14
#define MIMCMD_CONTACTS    15
#define MIMCMD_HISTORY     16
#define MIMCMD_VERSION     17
#define MIMCMD_SETNICKNAME 18
#define MIMCMD_IGNORE      19

#define MIMCMD_HELP_STATUS      LPGEN("Changes account status either globally or per account.\nUsage: status <status> [<account>].\nPossible values for <status> are: offline, online, away, dnd, na, occupied, freechat, invisible, onthephone, outtolunch.\n<Account> is the name of the account. If it's not specified then the command will issue a global status change.")
#define MIMCMD_HELP_AWAYMSG     LPGEN("Changes away message either globally or per account.\nUsage: awaymsg <message> [<account>].\n<Message> is the new away message.\n<Account> is an optional parameter specifying the account to set the away message for. If not specified then the away message will be set globally.")
#define MIMCMD_HELP_XSTATUS     LPGEN("Changes extended status either globally or per account.\nUsage: xstatus <xStatus> [<account>].\n<xStatus> is the new extended status to set. Possible values are: ...\n<Account> is an optional parameter specifying the account for which extended status is set. If not specified then extended status for all accounts will be changed.\nNOTE: Not all accounts/protocols support extended status.")
#define MIMCMD_HELP_POPUPS      LPGEN("Disables or enables popups display.\nUsage: popups (disable | enable | toggle).\nThe command will either enable or disable popups display.")
#define MIMCMD_HELP_SOUNDS      LPGEN("Disables or enables sounds.\nUsage: sounds (disable | enable | toggle).\nThe command will either disable or enable sounds.")
#define MIMCMD_HELP_CLIST       LPGEN("Hides or shows the contact list window.\nUsage: clist (show | hide | toggle).\nThe command will either show or hide the contact list window.")
#define MIMCMD_HELP_QUIT        LPGEN("Closes Miranda.\nUsage: quit [wait].\nIf wait is used then the command will return only when CmdLine plugin has been unloaded by Miranda.")
#define MIMCMD_HELP_HELP        LPGEN("Provides help on other commands.\nUsage: help [<command>].\nThe command will print help information for other commands. If run without any parameters it will print available commands.")
#define MIMCMD_HELP_EXCHANGE    LPGEN("Notifies Exchange plugin to check for email.\nUsage: exchange check.")
#define MIMCMD_HELP_YAMN        LPGEN("Notifies YAMN plugin to check for email.\nUsage: yamn check.")
#define MIMCMD_HELP_CALLSERVICE LPGEN("Calls a Miranda service.\nUsage: callservice <service> (d|s)<wParam> (d|s)<lParam>.\nThe command will call Miranda <service> service using wParam and lParam as arguments; the first letter of the paramater must be either 'd' if the parameter is a decimal number or 's' if the parameter is a string. Be careful when you use this function as you can only pass numbers and strings as data.\nNOTE: If you pass invalid data to a service, Miranda might crash.")
#define MIMCMD_HELP_MESSAGE     LPGEN("Sends a message to the specified contact(s).\nUsage: message <contact> [<contact> [<contact> [...]]] <message>.\nThe command will send <message> to the specified contact(s) - at least one contact must be specified - all parameters except the last one are considered recipients.\n<Contact> has the following format: <name>[:<account>]. <Name> is the contact display name or unique ID and <account> is an optional parameter representing the account of the contact (useful in case there is more than one contact with the same name).\nNOTE: The message string cannot exceed 512 characters. You can use \\n for new line (and \\\\n for \\n).")
#define MIMCMD_HELP_DATABASE    LPGEN("Allows you to manage database settings.\nUsage:\n  db set <module> <name> (b|i|d|s|w)<value>\n  db delete <module> <name>\n  db get <module> <name>.\nThe command can set a database entry to the specified value (if the entry does not exist, it will be created) as well as read or delete a specified database entry. <Module> is the name of the module where the key should be located, <name> is the name of the key and <value> is the value to be written. A character must be placed before <value> in order to specify what kind of data to write: b - byte, i - integer (word), d - double word, s - string, w - wide string.")
#define MIMCMD_HELP_PROXY       LPGEN("Configures proxy settings either globally or per account.\nUsage: proxy (global|<account>) <setting> [<params>].\n<Setting> is one of the following settings:\n  status (disable | enable | toggle)\n  server <proxy type> <server> <port>.")
#define MIMCMD_HELP_CONTACTS    LPGEN("Allows you to search/list contacts or open a message windows for specified contacts.\nUsage:\n  contacts list [<keyword> [account:<account>] [id:<id>] [status:<status>] [<keyword> [...]]].\nThe command will search all contacts and display the ones matching the search criteria. To search for a specific account use the keyword 'account:<account>'. To search for contacts that have a certain ID use the keyword 'id:<id>'. To search for contacts that have a certain status use 'status:<status>'.\n  contacts open [<keyword> [account:<account>] [id:<id>] [status:<status>] [<keyword> [...]]].\nThe command will open a message window for all contacts that match the search criteria. To search for a specific account use the keyword 'account:<account>'. To search for contacts that have a certain ID use the keyword 'id:<id>'. To search for contacts that have a certain status use 'status:<status>'. If no keyword is specified, the command will open a message window for all contacts that have unread messages.")
#define MIMCMD_HELP_HISTORY     LPGEN("Shows history or unread messages for a contact.\nUsage: history <command> <contact>.\n<Command> is one of the following commands:\n  unread - show unread messages for that contact.\n  show <contact> <start> <finish> - show history from event number <start> to <finish>. If any number is negative, it is interpreted as a relative index from the last event number +1 (so the last event for a contact is -1).")
#define MIMCMD_HELP_VERSION     LPGEN("Shows version information for Miranda and CmdLine plugin. If VersionInfo plugin is installed, it will use its report instead.\nUsage: version.\nThe command will print Miranda's and CmdLine's version numbers or, if VersionInfo plugin is installed, it will show VersionInfo's report.")
#define MIMCMD_HELP_SETNICKNAME LPGEN("Changes the user's nickname on the given protocol to the new name.\nUsage: setnickname <protocol> new_nickname.")
#define MIMCMD_HELP_IGNORE      LPGEN("Ignores or removes ignore flags for specific contacts.\nUsage:\n  ignore (block | unblock) <contact> [<contact> [<contact> [...]]].\nThe command will set or remove ignore flags for the specified contacts")

#define MIMMEM_REPLY       1 //data is a PReply structure

extern TCommand mimCommands[];
extern int cMimCommands;

void InitCommandData();
void DestroyCommandData();

#endif //M_MIMCMD_DATA_H