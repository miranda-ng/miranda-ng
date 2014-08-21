IEView History Viewer v.0.0.1.4
  by Cristian "Eblis" Libotean
	copyright © 2005-2007 Cristian Libotean

This plugin requires IEView to work, it might even crash if IEView is not available.
Make sure you have it enabled before reporting crashes !!!
    
You can search the logs by pressing Ctrl + F while the log window is selected.
You can copy text by selecting it and then right click, Copy.

Changes:

+ : new feature
* : changed
! : bufgix
- : feature removed or disabled because of pending bugs

v. 0.0.1.4 - 2007/04/16
	* Rebased dll (0x2F040000 ... previous address was 0x2F020000).

v. 0.0.1.3 - 2007/03/09
	* Changed icon (icon provided by Faith Healer)

v. 0.0.1.2 - 2007/03/09
	* Use the correct interface ID for history plugins (MIID_UIHISTORY).

v. 0.0.1.1 - 2007/03/08
	+ Added Unicode flag.

v. 0.0.1.0 - 2007/03/06
	+ Added UUID ( {2f093b88-f389-44f1-9e2a-37c29194203a} Unicode, {2f997250-bc2f-46f0-a33e-65f06283be5d} Ansi )
	+ Added HISTORY interface.

v. 0.0.0.13 - 2007/01/12
	! Don't use data after being free()'d

v. 0.0.0.12 - 2006/12/07
	* Changed controls tab order.
	+ Added version info.

v. 0.0.0.11 - 2006/09/14
	! Fixed search in unicode version.
	* Always use ANSI strings for menu items.
	* Use themes for tabbed dialogs if they are available.
	
v. 0.0.0.10 - 2006/06/13
	* Fix for Windows 2000 and date time controls.

v. 0.0.0.9 - 2006/05/17
	* Destroy services the correct way.
	
v. 0.0.0.8 - 2006/03/23
	+ Added 'Load in background' - in alpha stanges, doesn't work correctly with all event types (might show garbled text) (disabled).
	
v. 0.0.0.7 - 2006/02/1
	+ Rebased the dll (0x2F020000)
	* Modified plugin to work with new ieview.
	* Removed unneeded text from statusbar.

v. 0.0.0.6 - 2006/01/11
	+ Added min width and height for the history window.
	* The contact list window is no longer the parent of the history window :)
	(using the list in m_utils.h - thanks nullbie)
	+ Added status bar.
	
v. 0.0.0.5 - 2006/01/09

	! Changed options root to "Message Sessions" from "Message sessions".
	* Miranda's clist window is now the parent of the history window.
	! Allow updater to update the unicode version too.
	* Use the custom handle of the contact in the window title (if available).
	
v. 0.0.0.4 - 2006/01/06
	+ Added search function. You can search for a given text
	or for a given date and/or time. If an event is found
	the current page will be centered on it but it won't be selected as
	this can't be done. If you want to find it in the current page
	you can press Ctrl + F like before (this way the text *will* become
	selected). Some notes on searching:
			- search is disabled if all events are loaded.
			- you can either search for text or a date/time, not both.
			- you can't start a search from a given entry, they always begin
			with the first (or last) entry, depending on the direction of the search.
			- if you make a modification to the string or date/time the search
			will restart, it won't continue.
			- seconds aren't taken into account when searching for a given time.
	* Check to see if contact is RTL or LTR (only tabsrmm supported)
		(the setting in options acts as the default text direction which is
		overriden by the per-contact one).
	* Counting of events loaded in page now begins at 1 (not 0 as before)
	* Minor changes to prevent memory leaks.
	
v. 0.0.0.3 - 2006/01/03
	! Fixed some issues with the pages.
	+ Added option to show the last page of the history first.
	+ Added RTL support.
	
v. 0.0.0.2 - 2006/01/03
	+ You can now choose to either load all events or only a certain number
	per page.
	+ Added border around ieview control.
	+ Allow resize of main window.	
	+ Added icon to main window.

v. 0.0.0.1 - 2005/12/30
	First build, released on miranda's forums.
	
	
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
