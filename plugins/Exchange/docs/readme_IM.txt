Exchange mail notifier plugin v.0.0.3.3
Copyright © 2006-2009 Cristian Libotean, Attila Vajda

This plugin notifies you if you have unread emails on an exchange server.

You need to have an Exchange server present that accepts SMTP connections. You need to have Outlook
installed (mapi dll hell :) ) and a recent miranda version ( > 0.4).
You enter your email account, your password (you can leave this blank if you want - if you don't
enter it exchange will ask for your password and you'll have an option to remember it). Enter the
Exchange server ip or hostname and the port which accepts SMTP connections. The port is not needed
for the actual connection but it is needed in order to check that the server is available - if the server
is blocking the port you won't get notified as the plugin will think it can't connect to the server.

Changes:

+ : new feature
* : changed
! : bufgix
- : feature removed or disabled because of pending bugs

v. 0.0.3.3 - 2009/06/05
	+ Added support for core variables for user name field.

v. 0.0.3.2 - 2009/01/28
	* Added unicode aware flag
	! Manually initiated checks were treated the same as automatic checks

v. 0.0.3.1 - 2008/01/24
	* Changed beta versions server.

v. 0.0.3.0 - 2007/08/30
	* Check mailbox in separate thread.

v. 0.0.2.0 - 2007/03/06
	+ Added UUID ( {2fd0df15-7098-41ce-aa92-ff621806e38b} ).
	+ Added EXCHANGE interface.

v. 0.0.1.12 - 2007/02/06
	! Fixed a crash that occured sometimes when moving emails in Outlook and then trying to mark them as read with the plugin.

v. 0.0.1.11 - 2007/01/31
	* Changed beta URL.

v. 0.0.1.10 - 2007/01/07
	+ New version resource file.

v. 0.0.1.9 - 2006/11/26
	+ Added dll version info.
	* Changed controls tab order.

v. 0.0.1.8 - 2006/10/20
	! Fixed maximum number of automatic connection attempts.

v. 0.0.1.7 - 2006/10/02
	+ Added how to make it work info to readme file.
	+ Option to disable server port check.
	! Don't try to connect if server not configured.
	* Removed dependency on msvcr80 (file size doubled :( )

v. 0.0.1.6 - 2006/09/26
	! Miranda will exit a lot sooner :)
	* Changed popup title to "Exchange notifier"

v. 0.0.1.4 - 2006/09/26
	+ Updater support (beta versions)
	! In certain circumstances plugin reported 0 emails.
	* Some other things i don't know :)

v. 0.0.1.3 - 2006/09/19
	! Don't show "cannot connect" popup too often

v. 0.0.1.2 - 2006/09/14
	+ Rebased dll (0x2F010000).
	+ Option to reconnect every x minutes (logout and then login again - without this Exchange server might block the connection after a while).
	* Check for email after Miranda starts.
	! Fixed bug which made the plugin lose its connection with the Exchange server and always report 0 emails.

v. 0.0.1.1 - 2006/09/01
	+ Added option to disable automatic checking.
	! Miranda won't hang on exit anymore.

v. 0.0.1.0 - 2006/08/23
	* Use miranda pre shutdown event to disconnect from exchange server.
	+ Added icon to emails dialog.
	* Emails dialog is only created when needed.
	! Marking events as read and then clicking the popup again would crash Miranda.
	* Disable the popups checkbox if no popup module is present.
	! Apply button is now disabled when options dialog is created.
	* Don't show emails window if no emails are available.

v. 0.0.0.9 - 2006/06/01
	* Some minor changes.
	
v. 0.0.0.8 - 2006/05/xx
	+ Internal build.
	
v. 0.0.0.7 - 2006/05/18
	+ First release on miranda's File Listing.
	

Translation strings - last update v0.0.1.8:
[Check exchange mailbox]
;Options
[Plugins]
[Exchange notify]
;Options dialog
[Username:]
[Password:]
[Server:]
[Connection]
[Preferences]
[Show email notifications using popup module]
[Check server before trying to connect]
[Try to connect]
[times before giving up.]
;Unread emails window
[Unread emails]
[Mark as read]
[Close]
[Entry ID]
[Subject]
[Sender]
;Popups/message boxes
[You have %d unread emails ...]
[You have one unread email ...]
[Exchange email]
[Do you want to see the email headers?]
[Server is not configured ...]
[Server not available]
[Cannot connect to Exchange server ...]
[Connecting to Exchange ...]
[Forcefully reconnecting to Exchange server ...]



	
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
