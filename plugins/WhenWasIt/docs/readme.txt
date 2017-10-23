WhenWasIt birthday reminder plugin v.0.3.3.4
Copyright © 2006-2011 Cristian Libotean
Thanks go to Angeli-Ka for the icons.

This plugin notifies you of upcoming birthdays using different notification methods.
It can show a popup of the contact with the upcoming birthday and can also add a clist extra icon.

You can also set a contacts birthday using the contact menu item. Checking "mBirthday compatibility" will force
the plugin to store it's birthday settings in the same place as mBirthday plugin. When the change dialog first appears
it will be checked if a birthday was found in the database (and the date will be set accordingly). If no birthday is found the
date time control will be unchecked. To delete a birthday that's currently in the database open up the Add/change
window, uncheck the date time control and click Ok.

!!! Clicking the Ok button when the date time control is unchecked will delete any birthdays from the database (if any).
To close the dialog without saving the changes use the close button (X).

Changes:

+ : new feature
* : changed
! : bufgix
- : feature removed or disabled because of pending bugs

version 0.3.3.4 - 2011/09/22
	+ Patch from Tobias Weimer - addded ability to save and restore window position

version 0.3.3.3 - 2011/09/05
	+ Patch from Tobias Weimer - added hotkey support

version 0.3.3.2 - 2011/08/23
	* Do not notify about missed birthdays if notify after is set to 0 days even if Miranda hasn't been started for a long time (longer than the interval)

version 0.3.3.1 - 2011/08/23
	+ made x64 version updater aware

version 0.3.3.0 - 2011/08/22
	* Patch from Kirill Volinsky - use mir_free() helper function instead of calling the services manually.
	* Changed the way header files are included.

version 0.3.2.9 - 2011/05/16
	* Patch from Kirill Volinsky - TranslateT() is not needed when initializing dialogs.

version 0.3.2.8 - 2011/04/14
	* Patch from Kirill Volinsky - TranslateT() is not needed when initializing dialogs.
	! Fixed version number.

version 0.3.2.7 - 2011/03/29
	* Patch from Kirill Volinsky - use mir_sntprintf in ImportBirthdaysService()

version 0.3.2.6 - 2011/03/29
	* Patch from Kirill Volinsky - filter translation in open file dialog

version 0.3.2.5 - 2010/05/19
	! Fix for extra icons dissapearing with Clist Modern and ExtraIcons plugin when changing options

version 0.3.2.4 - 2010/04/13
	* Version bumb due to Addons site failure

version 0.3.2.3 - 2010/04/13
	! Open dialog in background enabled only if Dialog notification is checked.

version 0.3.2.2 - 2010/04/12
	* More work with resource files :)

version 0.3.2.1 - 2010/04/12
	* Enlarged options window

version 0.3.2.0 - 2010/04/12
	+ Added mICQBirthday support

version 0.3.1.1 - 2009/12/17
	! Do not translate Main Menu items, let the core translate them.

version 0.3.1.0 - 2009/11/16
	* Extra icons name changes
	+ Support for ExtraIcons service plugin
	+ Added option to open upcoming birthdays list in background
	+ x64 support (not tested !!)

version 0.3.0.4 - 2008/11/12
	! Fix bug reported by George, related to erroneous DTB reporting due to errno not being reset.

version 0.3.0.3 - 2008/04/14
	! Fix notifying missed birthdays from a long time ago

version 0.3.0.2 - 2008/03/31
	+ Added options to get notified about missed birthdays

version 0.3.0.1 - 2008/01/24
	* Changed beta versions server.

version 0.3.0.0 - 2007/11/15
	+ Added option not to show notifications for hidden or ignored contacts
	* Refactoring - WhenWasIt should be a lot less demading on the database

version 0.2.2.3 - 2007/05/09
	! Close the upcoming birthdays dialog on miranda exit.

version 0.2.2.2 - 2007/05/08
	! Fixed options dialog issues.

version 0.2.2.1 - 2007/05/08
	* Changed text for timeout dialog.
	+ Sort dialog entries by DTB.
	! Timeout would only initiate the first time the dialog was shown.

version 0.2.2.0 - 2007/05/07
	+ Added dialog notification option.

version 0.2.1.2 - 2007/04/16
	* Allow days in advance to be set to 0 (only notifies of birthdays happening on that day).

version 0.2.1.1 - 2007/04/04
	* Message window will be opened in a separate thread.

version 0.2.1.0 - 2007/03/07
	+ Added UUID ( {2ff96c84-b0b5-470e-bbf9-907b9f3f5d2f} Unicode, {2f369a01-211a-4f13-be3c-bf4f8c62cace} Ansi )
	+ Added BIRTHDAYNOTIFY interface.

version 0.2.0.11 - 2007/02/12
	+ Added Unicode flag.

version 0.2.0.10 - 2007/01/31
	* Changed beta URL.

version 0.2.0.9 - 2007/01/26
	+ Added custom popup delay for birthdays that occur today

version 0.2.0.8 - 2007/01/07
	+ New version resource file.
	! Close thread handle.
	* Use new m_popup.h header.
	
version 0.2.0.7 - 2006/11/26
	+ Added dll version info.
	* Changed controls tab order
	! Disable left and right popup click actions if popups notification is disabled.

version 0.2.0.6 - 2006/10/30
	* Increased the delay between update requests to 3 seconds
	+ Configurable delay between update requests using a "hidden" variable called "UpdateDelay" created in WhenWasIt module.

version 0.2.0.5 - 2006/10/02
	! Close birthday list window on exit.
	* Use window list in m_utils.h for add/change birthday windows.

version 0.2.0.4 - 2006/09/25
	+ Rebased dll (0x2F400000)
	* Use big icons.
	! Kill timers on exit.
	* Updater support (beta versions).

version 0.2.0.3 - 2006/09/05
	* Always use ANSI strings for menu items.

version 0.2.0.2 - 2006/08/24
	! Refresh clist icons when settings get changed.
	* Use WhenWasIt popup colors for notification popups.
	* Use unicode text for the menu items.

version 0.2.0.1 - 2006/08/17
	+ Birthday list window automatically refreshes when a birthday is changed.
	+ Added shortcut key 'b' to add/change birthday menu item.

version 0.2.0.0 - 2006/08/13
	! Text inside comboboxes is now translateable.
	+ Group checkboxes in options dialog will now resize to correct size depending on translation string.
	* Running dbtool on the profile won't produce so many empty settings messages.
	+ Birthday list window now shows the module where birthday info was found.
	+ Unicode version. Translation strings should be created with care, %s %S behave differently depending if build is ANSI or UNICODE - DO NOT MIX !!!
	Warning! Please do not mix Unicode and Ansi exported birthday files. You should use the same version (Ansi/Unicode) of WhenWasIt that was used to export the info.

version 0.1.2.2 - 2006/08/04
	+ Option to only check once a day

version 0.1.2.1 - 2006/08/03
	+ Added tooltip to add/change birthday dialog to show the current location of the birthday in the database.
	* Color coded add/change birthday dialog - yellow - mBirthday, red - Protocol module, purple - Birthday Notifier, green - UserInfo.
	* Minor cosmetic changes in options dialog.
	* Also use contact protocol when exporting/importing - you cannot import old .bdays files, you need to use the new export method.
	* Compare contact IDs without regard to case.
	You cannot import/export birthdays for contacts that don't have their protocol module loaded!

version 0.1.2.0 - 2006/07/25
	+ Added option to customize left and right mouse click actions on popups.
	+ Added import/export birthdays features. Export will *NOT* export info for contacts that don't have their protocol plugin loaded. Imported birthdays will be stored in the default module selected in options.
	+ Added contacts count in birthday list window.
	* Changed icons so the number on the ballons is easier to see (thanks Angeli-Ka).

version 0.1.1.3 - 2006/07/04
	! Users that have birthday today were shown last.
	* Case insensitive comparison in birthday list.

version 0.1.1.2 - 2006/07/03
	* Use CallServiceSync() ... thanks Imme.
	* Disable popups notifications if no popups plugin is loaded.
	* Changed default colors.
	
version 0.1.1.1 - 2006/07/03
	* Left clicking the popup shouldn't crash anymore (using an event to call the send message service in the main thread).
	* No need to restart miranda for the menu icons to change.
	
version 0.1.1.0 - 2006/07/01
	+ Added option to set the default module where the plugin will save birthday info.
	+ Added birthday near and birthday today sound.
	+ Added popup preview button - age and days to birthday are generated randomly :).
	! Destroying add/change birthday window over and over again (stack overflow on win 98).

version 0.1.0.2 - 2006/06/30
	*Ooops, fogot create the release build.
	
version 0.1.0.1 - 2006/06/30
	+ Added option to save birthday info in the protocol module. This ensures that User Details module can read the date of birth correctly.
	There are now 3 options on where to save the date of birth. Default location is "UserInfo module"; if you want to have mBirthday compatibility
	you should select "mBirthday module" and if you want the default User Details miranda module to be able to read the date of birth you should select "Protocol module".
	The value in the protocol module will be overwritten if you select "Protocol module" from the dropdown list.
	+ Added tooltip in the Add birthday window explaining what the combo box entries mean.
	+ Right clicking the popup dismissed it, left clicking will open the message window.

version 0.1.0.0 - 2006/06/27
	+ Made show popups when no birthdays are near optional.
	+ Added service to refresh user details on all contacts. Will go through all contacts in your list (regardless of protocol) and update their user details, one every 2 seconds (to avoid flooding).

version 0.0.0.9 - 2006/06/26
	+ Added popup if no birthdays are near.

version 0.0.0.8 - 2006/06/25:
	+ Added Birthday reminder compatibility (Birthday reminder color inside add/change birthday dialog is pink).

version 0.0.0.7 - 2006/06/22:
	+ Option to show upcoming age (age the contact will have on 31 dec) or current age (the age of the contact today).

version 0.0.0.6 - 2006/06/17:
	* Added custom text for contacts that have birthday today.
	
version 0.0.0.5 - 2006/06/14:
	! Use user's local time to compute age and DTB, not GMT time.
	* Removed unnecessary icon. Now the add birthdaw dialog uses the add birthday icon, and the birthday list dialog uses birthday list icon.
	+ Option to not show popups for subcontacts of metacontact.
	+ Fix for windows 2000 and date time control.

version 0.0.0.4 - 2006/06/13:
	+ Added sound notification.
	+ Added descending sort.
	* When sorting by DTB or Age show N/A last.
	! Ooops, age calculation was good before.
	* Messagebox with error when add birthday dialog can't be created.
	! Fixed annoying popups appearing everytime icons were changed.

version 0.0.0.3 - 2006/06/12:
	+ Color coded background in Add/change birthday dialog. Red means the value is stored in the protocol module (won't be 
	overwritten, a new entry in either UserInfo or mBirthday module will be created instead). Pink means the value is stored
	in mBirthday module and will be overwritten if you select "mBirthday compatibility" (otherwise it will be moved to module UserInfo).
	Normal color (gray) means it was found in UserInfo module - clicking "mBirthday compatibility" will move it to mBirthday module.
	+ Added TopToolBar button.
	* Show contact name in Add/change birthday dialog.
	+ Remember birthday list position.
	+ Double clicking a contact in the birthday list will open the add birthday dialog.
	+ Remember sort column.
	* Added a bit more room for translations.
	! Fixed age calculation.
	* Check for birthdays on miranda start (~20 seconds delay).

version 0.0.0.2 - 2006/06/11:
	+ Support for contacts that have their birthday in the protocol module.
	! Fixed some possible clist_modern issues.
	
version 0.0.0.1 - 2006/06/10:
	First release ...


Translation string - last update v0.2.0.1:
Please be careful when translating these as there are a lot of escape characters in them, don't forget to add those and don't change the order :)
[%s has birthday in %d days.]
[%s has birthday tomorrow.]	
[%s has birthday today.]
[Birthday - %s]
[He]
[She]
[He/She]
[%s\n%s will be %d years old.]
[%s\n%s just turned %d.]
[Protocol]
[Contact]
[DTB]
[Birthday]
[Age]
[Set birthday for %s:]
[N/A]
[WhenWasIt]
[No upcoming birthdays.]
[Starting to refresh user details]
[Done refreshing user details]
[Add/change user &birthday]
[Birthdays (When Was It)]
[Check for birthdays]
[Birthday list]
[Refresh user details]
[Import birthdays]
[Export birthdays]
[Please select a file to import birthdays from ...]
[Please select a file to export birthdays to ...]
[Exporting birthdays to file: %s]
[Done exporting birthdays]
[Importing birthdays from file: %s]
[Done importing birthdays]
[Could not find UID '%s [%s]' in current database, skipping] - ANSI version - please don't mix
[Could not find UID '%s [%S]' in current database, skipping] - UNICODE version - please don't mix
[%s protocol] - ANSI version - please don't mix
[%S protocol] - UNICODE version - please don't mix
[Please select the module where you want the date of birth to be saved.\r\n\"UserInfo\" is the default location.\r\nUse \"Protocol module\" to make the data visible in User Details.\n\"mBirthday module\" uses the same module as mBirthday plugin.]
[Birthday list (%d)]
	
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
