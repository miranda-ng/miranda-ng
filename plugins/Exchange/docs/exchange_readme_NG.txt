Exchange mail notifier plugin v.0.1.3.4
Copyright © 2006-2009 Cristian Libotean, Attila Vajda, MaKaR

This plugin notifies you if you have unread emails on an exchange server.

This miranda NG version is based on Miranda IM exchange plugin (0.0.3.3). 
Base sources dowloaded from (http://eblis.tla.ro/projects/miranda/Exchange/Exchange src 0.0.3.3.zip)

You need to have an Exchange server present that accepts SMTP connections. You need to have Outlook
installed (mapi dll hell :) ) and a recent miranda NG version.
You enter your email account, your password (you can leave this blank if you want - if you don't
enter it exchange will ask for your password and you'll have an option to remember it). Enter the
Exchange server ip or hostname and the port which accepts SMTP connections. The port is not needed
for the actual connection but it is needed in order to check that the server is available - if the server
is blocking the port you won't get notified as the plugin will think it can't connect to the server.

For 64bit miranda both windows and office must be 64bit (not tested)
Changes:

+ : new feature
* : changed
! : bufgix
- : feature removed or disabled because of pending bugs

v. 0.1.3.4 - 2013/05/29
	* Ported to Miranda NG
	* Unicode support


	
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
