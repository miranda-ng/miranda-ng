CmdLine plugin v.0.0.4.1
Copyright © 2007-2011 Cristian Libotean

This plugin lets you control Miranda from the command line.
It does this using an additional executable file that's used to pass command line arguments to Miranda.
This plugin will not work correctly when multiple instances of Miranda and the plugin are running at
the same time. Do not run more than once instance of the plugin at the same time!

The executable file (MimCmd.exe) can be placed anywhere, it doesn't have to be inside Miranda's main folder.

Changes
=======
+ : new feature
* : changed
! : bufgix
- : feature removed or disabled because of pending bugs

v. 0.0.4.1 - 2011/08/22
	+ made x64 version updater aware

v. 0.0.4.0 - 2011/07/25
	+ Added 'mimcmd contacts open' command which opens all the message windows for contacts which have unread messages
	+ Added 'mimcmd ignore' command which can ignore or remove the ignore flags for specified contacts
	+ Added 'mimcmd setnickname' command which can change the owner's nickname on given protocols

v. 0.0.3.0 - 2010/11/01
	+ Added per account information to status command. The 'mimcmd status' will now list the global status as well as per account status.
	* Code rewrite to take advantage of accounts instead of protocols - expect problems :)
	! The code should now work with account names instead of internal protocol names.
	! The code no longer converts the account name (former protocol name) to uppercase.

v. 0.0.2.20 - 2009/11/16
	+ x64 support (not tested !!)

v. 0.0.2.12 - 2009/05/28
	! Fixed quit command
	! Possible fix for crash in history command

v. 0.0.2.11 - 2009/05/18
	! Issuing buffer full events did not work correctly (the buffer was reset before Miranda had a chance to process it)
	! Showing history did not work correctly for a big number of entries.
	! Crash when encountering a NULL contact id.

v. 0.0.2.10 - 2009/05/18
	+ Added contact id to 'contact list' command.
	+ Added id: search parameter to contact command.

v. 0.0.2.9 - 2008/05/12
	! Only set away message for protocols that support it.

v. 0.0.2.8 - 2008/01/24
	* Increased timeout when sending message from 4 seconds to 15 seconds.
	* Changed beta versions server.

v. 0.0.2.7 - 2007/11/15
	+ Added quit wait command. If this command is used the command will return only when CmdLine plugin is unloaded by Miranda.
	    Warning: The status will be reported as an error, "Miranda has been closed or an error has occured while waiting for the result, could not process request."

v. 0.0.2.6 - 2007/09/27
	! Fix for history unread command truncating messages ( :) )
	+ Added status:<status> parameter to contacts command.

v. 0.0.2.5 - 2007/09/27
	! Fix for history command truncating messages.
	+ Added version command - mimcmd -v (works even if miranda is not started).

v. 0.0.2.4 - 2007/08/09
	+ Do not allow more than 1 instance of CmdLine to run at the same time.
	* Changed the order of the parameters for message command (now the message parameter is the last one).
	* Increased parameter maximum size from 256 characters to 512.

v. 0.0.2.3 - 2007/08/09
	! Forgot to update version number.

v. 0.0.2.2 - 2007/07/31
	+ Added command to open the message window for specified contacts.
	* Contacts list command will also show the contact handle.

v. 0.0.2.1 - 2007/05/30
	+ Added version command.

v. 0.0.2.0 - 2007/05/28
	! Successfully sent messages are now stored in the database.
	+ Added contacts search function (keywords can match either the contact name or the protocol).
	+ Added unread events command.
	+ Added view history command.

v. 0.0.1.3 - 2007/05/13
	! Status change now works correctly when KeepStatus is installed.

v. 0.0.1.2 - 2007/04/23
	! Allow usage information to be translated.

v. 0.0.1.1 - 2007/04/21
	* If connection with Miranda could not be established try to read language pack file in current folder too.
	! Convert strings to OEM before printing them (fixes translations for non latin based alphabets).

v. 0.0.1.0 - 2007/04/19
	+ Added translation support for command line utility.
	+ Added database command (you can write, read and delete settings).
	+ Added proxy command. Currently it will *not* change proxy servers for all netlib users as there is no way
	to get that list (yet). It will only change protocol settings (client to client connnections, updater, help and
	others will not be changed). Requires a Miranda restart before taking effect.

v. 0.0.0.4 - 2007/04/16
	+ Rebased dll (0x2F0B0000)
	
v. 0.0.0.3 - 2007/04/04
	! Status was not set correctly when protocol name was specified.

v. 0.0.0.2 - 2007/04/02
	+ Added yamn command.
	+ Added message command.

v. 0.0.0.1 - 2007/03/30
	+ First release.
	
	
******Translateable strings****** (updated for version 0.0.2.6)
[Available commands: ]
Miranda has been closed or an error has occured while waiting for the result, could not process request.
%s usage:\n
%s <command> [<param> [, <param> ..]]\n
This will will tell Miranda to run the specified command. The commands can have zero, one or more parameters. Use '%s help' to get a list of possible commands.\n
No command can have more than %d parameters\n
Could not create connection with Miranda or could not retrieve list of known commands.\n
Unknown command '%s'.\n
Change protocol status either globally or per protocol.\nUsage: status <status> [<protocol>].\nPossible values for <status> are: offline, online, away, dnd, na, occupied, freechat, invisible, onthephone, outtolunch.\n<Protocol> is the name of the protocol. If it's not specified then the command will issue a global status change.
Change away message either globally or per protocol.\nUsage: awaymsg <message> [<protocol>].\n<Message> is the new away message.\n<Protocol> is an optional parameter specifying the protocol to set the away message for. If not specified then the away message will be set globally.
Change extended status either globally or per protocol.\nUsage xstatus <status> [<protocol>].\n<XStatus> is the new extended status to set. Possible values are:...\n<Protocol> is an optional parameter specifying the protocol for which extended status is set. If not specified then extended status for all protocols will be changed.\nNOTE: Not all protocols support extended status.
Disables or enables popups display.\nUsage popups (disable | enable | toggle).\nThe command will either enable or disable popups display.
Disables or enables sounds.\nUsage: sounds (disable | enable | toggle).\nThe command will either disable or enable sounds.
Hides or shows the contact list window.\nUsage: clist (show | hide | toggle).\nThe command will either show or hide the contact list window.
Closes Miranda.\nUsage: quit.
Provides help on other commands.\nUsage: help [<command>].\nThe command will print help information for other commands. If run without any parameters it will print available commands.
Notifies Exchange plugin to check for email.\nUsage: exchange check
Notifies YAMN plugin to check for email.\nUsage: yamn check.
Calls a Miranda service.\nUsage: callservice <service> (d|s)<wParam> (d|s)<lParam>. The command will call Miranda <service> service using wParam and lParam as arguments; the first letter of the paramater must be either 'd' if the parameter is a decimal number or 's' if the parameter is a string. Be careful when you use this function as you can only pass numbers and strings as data.\nNOTE:If you pass invalid data to a service Miranda might crash.
Sends a message to the specified contact(s).\nUsage: message <contact> [<contact> [<contact> [...]]] <message>. The command will send <message> to the specified contact(s) - at least one contact must be specified - all parameters except the last one are considered recipients.\n<Contact> has the following format:<name>[:<protocol>]. <Name> is the contact display name or unique ID and <protocol> is an optional parameter representing the protocol of the contact (useful in case there is more than one contact with the same name).\nNOTE:The message string cannot exceed 512 characters.
Allows you to manage database settings.\nUsage:\n db set <module> <name> (b|i|d|s|w)<value>\n  db delete <module> <name>\n  db get <module> <name>.\nThe command can set a database entry to the specified value (if the entry does not exist it will be created) as well as read or delete a specified database entry. <Module> is the name of the module where the key should be located, <name> is the name of the key and <value> is the value to be written. A character must be placed before <value> in order to specify what kind of data to write: b - byte, i - integer (word), d - double word, s - string, w - wide string.
Configures proxy settings either globally or per protocol.\nUsage: proxy (global|<protocol>) <setting> [<params>].\n  <setting> is one of the following settings:\n    status (disable | enable | toggle).\n    server <proxy type> <server> <port>
Allows you to search/list contacts or open a message windows for specified contacts.\nUsage:\n contacts list [<keyword> [protocol:<protocol>] [status:<status>] [<keyword> [...]]]. The command will search all contacts and display the ones matching the search criteria. To search for a specific protocol use the keyword 'protocol:<protocol>'. To search for contacts that have a certain status use 'status:<status>'.\n contacts open [<keyword> [protocol:<protocol>] [status:<status>] [<keyword> [...]]]. The command will open a message window for all contacts that match the search criteria. To search for a specific protocol use the keyword 'protocol:<protocol>'. To search for contacts that have a certain status use 'status:<status>'.
Shows history or unread messages for a contact.\nUsage:\n history <command> <contact>.\n <command> is one of the following commands:\n    unread - show unread messages for that contact.\n    show <contact> <start> <finish> - show history from event number <start> to <finish>. If any number is negative it is interpreted as a relative index from the last event number + 1 (so the last event for a contact is -1).
Shows version information for Miranda and CmdLine plugin. If VersionInfo plugin is installed it will use its report instead.\nUsage:\n version. The command will print Miranda's and CmdLine's version numbers or, if VersionInfo plugin is installed, it will show VersionInfo's report.

Popups are currently enabled.
Popups are currently disabled.
Popups were enabled successfully.
Popups could not be enabled.
Popups were disabled successfully.
Popups could not be disabled.
Sounds are currently enabled.
Sounds are currently disabled.
Sounds were enabled successfully.
Sounds could not be enabled.
Sounds were disabled successfully.
Sounds could not be disabled.
Contact list is currectly shown.
Contact list is currently hidden.
Contact list was shown successfully.
Contact list could not be shown.
Contact list was hidden successfully.
Contact list could not be hidden.
Command '%s' is not currently supported.
Wrong number of parameters for command '%s'.
Unknown parameter '%s' for command '%s'.
Current status: %s.
Changed global status to '%s' (previous status was '%s').
Changed '%s' status to '%s' (previous status was '%s').
'%s' doesn't seem to be a valid protocol.
Failed to change status for protocol '%s' to '%s'.
Successfully set '%s' status message to '%s' (status is '%s').
Changed '%s' status message to '%s' (status is '%s').
'%s' doesn't seem to be a valid protocol.
Failed to change status message for protocol '%s' to '%s' (status is '%s').
Issued check email command to Exchange plugin.
Exchange plugin is not running.
Issued check email command to YAMN plugin.
YAMN plugin is not running.
CallService call successful: service '%s' returned %d.
Invalid parameter '%s' passed to CallService command.
Service '%s' does not exist.
Message sent to '%s'.
Could not send message to '%s'.
Setting '%s/%s' deleted.
Wrote '%s:%s' to database entry '%s/%s'.
'%s/%s' - %s.
Could not retrieve setting '%s/%s': %s.
Setting '%s/%s' was not found.
'%s' proxy was disabled.
'%s' proxy was enabled.
%s proxy server: %s %s:%d.
%s proxy set to %s %s:%d.
%s The port or the proxy type parameter is invalid.
'%s' doesn't seem to be a valid protocol.
Could not find contact handle for contact '%s'.
No unread messages found.
%s:%s - %d unread events.
Contact '%s' has '%d' events in history.