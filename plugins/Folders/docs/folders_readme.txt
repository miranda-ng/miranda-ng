Custom profile folders v.0.1.6.0
Copyright © 2005-2010 Cristian Libotean

This plugin allows users to select the folders where plugins save their data.
It's a service plugin, it adds no functionality on it's own.
See the included m_folders.h for help on how to make use of these services in your own plugins.
Don't forget to click apply to allow the plugin to create the folder structure.
When you click apply it will try to delete the old path (all the empty folders) and
then create the new structure.
!!! Make sure you don't have any of those folders open in explorer or any other program.
The plugin also allows environment variables to be used. Environment variables
are expanded before any miranda variables are expanded so if you have, for example, %profile_path%
defined as a system variable it will use that value instead expanding to miranda's profile path.

Changes:

+ : new feature
* : changed
! : bufgix
- : feature removed or disabled because of pending bugs

version 0.1.6.0 - 2012/03/22
	! Fixed m_folders.h declarations in unicode mode - Thanks Freak_TheMighty

version 0.1.5.8 - 2011/08/23
	* Patch from Kirill Volinsky - updated m_folders.h header file

version 0.1.5.7 - 2011/08/23
	+ made x64 version updater aware

version 0.1.5.6 - 2011/08/22
	* Patch from Kirill Volinsky - use mir_free() instead of calling the services manually

version 0.1.5.5 - 2011/05/16
	* Patch from Kirill Volinsky - added mir_a2u mir_u2a support

version 0.1.5.4 - 2011/04/14
	* Patch from Kirill Volinsky - Fixed Folders debug service (it's for testing purposes)

Version 0.1.5.3 - 2010/11/01
	* Applied patch from IG_Vasilich - unicode dialogs

Version 0.1.5.2 - 2010/11/01
	! Attempt to fix crash with Utils_ReplaceVarsW function.

Version 0.1.5.1 - 2010/08/02
	! Fixed path retrieval - the path will no longer contain any trailing \ (as the help file suggests)
	! In previous versions Folders plugin did not remove spaces and \ although the help file said it did, but now they are removed.

Version 0.1.5.0 - 2010/05/19
	+ Added support for Variables plugin.
	+ Added support for Miranda core variables.
	* Environment variables are no longer expanded by Folders plugin, instead it relies on the core to expand these variables.
	+ The fields defined in Variables plugin are the first to get parsed, then the core variables and only after all these have been expanded will
	Folders plugin expand the fields it defines itself (if they still need to be expanded) (Variables plugin -> Core -> Folders -> result).

Version 0.1.4.5 - 2009/11/13
	+ x64 support - thanks to Nightwish for patch

Version 0.1.4.4 - 2008/05/14
	! Only try to delete folders if they're not reparse points

Version 0.1.4.3 - 2008/01/24
	* Changed beta versions server.

Version 0.1.4.2 - 2007/06/26
	! Fixes for the help text.

Version 0.1.4.1 - 2007/06/24
	! Translation support for help text.
	* Enabled unicode flag since plugin is 2in1.

Version 0.1.4.0 - 2007/04/23
	*Translation support for help text.
	+Expand environment variables. Environment variables are expanded before any miranda variables.
	So if you have, for example, %profile_path% defined as a system variable it will expand to that value
	instead	of expanding to miranda's profile path.

Version 0.1.3.0 - 2007/04/11
	+Section names and item names are now translateable. Plugins should NOT translate the strings themselves, they should let Folders plugin do it.

Version 0.1.2.1 - 2007/03/27
	*Fix for database plugins returning non standard profile path (dbxSA).

Version 0.1.2.0 - 2007/03/06
	+Added UUID ( {2f129563-2c7d-4a9a-b948-97dfcc0afdd7} )
	+Added FOLDERS interface.

Version 0.1.1.1 - 2007/01/31
	*Changed beta URL.

Version 0.1.1.0 - 2007/01/28
	!In case of empty format string use miranda's main path instead.

Version 0.1.0.7 - 2007/01/11
	!Show sections and items as unicode.

Version 0.1.0.6 - 2007/01/07
	+New version resource file.
	*Options cleanup. :)
	+New extended helper function.
	*Don't show old folder if it isn't empty.

Version 0.1.0.5 - 2006/11/26
	+Added dll version info.
	*Changed controls tab order.

Version 0.1.0.4 - 2006/11/07
	*Select first child item when a section is selected.

Version 0.1.0.3 - 2006/10/23
	!Unicows needed on Windows 98.
	*Don't show explorer windows when folders are changed.

Version 0.1.0.2 - 2006/09/10
	!Rebased the dll (0x2F000000)
	+Updater support (beta versions)

Version 0.1.0.1 - 2006/08/15
	*Modified m_folders.h so it compiles with C.
	+Modified helper functions so they use PATH_TOABSOLUTE when folders plugin is not installed. This should make the helper functions always return an absolute path.

Version 0.1.0.0 - 2006/5/17
	!Fixed helper functions (now they return 0 on error instead of 1). Also, the register functions how return a HANDLE, like they should.
	!Destroy services the proper way.

Version 0.0.0.8 - 2006/3/27
	!Made MS_FOLDERS_GET_SIZE service unicode aware.
	*Changed the way the helper functions behave - they should now work even when encountering old versions of folders plugin.
	+Added new TCHAR helper functions.
	+Preview window updates on every key press.
	
Version 0.0.0.7 - 2006/3/24
	!Fixed stupid include file in m_folders.h
	*Preview window is always unicode aware ... might cause problems on 98 systems.
	+Added helper functions.
	+Added unicode support - alpha stages.
	+Added TCHAR variables to structures.
	+Fixed a bug with register path service - i wasn't following my own doc

Version 0.0.0.6 - 2006/2/14:
	+Changed the api (again). Unicode paths are now possible. The unicode part doesn't work yet and maybe
	it never will - there are no *unicode* services to get the current profile path.
	*Removed support for API from version 0.0.0.3. Only 0.0.0.6 API works now ...
	+Added an event ME_FOLDERS_PATH_CHANGED.
	+If the old directory isn't empty when changing a path then 2 explorer
	windows will be opened, one for the old path and one for the new one,
	so the user can copy the data.

Version 0.0.0.5 - 2006/1/19:
	+Rebased the dll (0x2F010000)
	+Added the changes made by pescuma

Version 0.0.0.4 - 2005/12/19:
	*Api changes (sorry). Every entry now belongs
	to a section. The old api (from version 0.0.0.3)
	still works and folders registered with the old 
	api will be put in section "Unknown".
	+Plugin will try to delete old path structure when
	a new one is entered.
	+Plugin will try to create the new path structure if
	it doesn't exist.
	*Minor cosmetic fixes

Version 0.0.0.3 - 2005/12/17:
	*Path string is trimmed now.
	*Now it removes the last \ from the input
	edit (if there was one)
	*Added more information about variables.

Version 0.0.0.2 - 2005/12/17:
	*Changed the api, no loger requires defines for common folders.
	+Plugins can now register their folders using the new api.
	+Added options page to allow users to modify the folders.
	
	This plugin still needs testing; i've done some testing
	myself but i'm not sure it won't bork the results on some
	weird cases. Please report any bugs you find on miranda's forum.
	
version 0.0.0.1 - 2005/12/16:
	First release ...




Translation strings (updated for version 0.1.4.2)

"Don't forget to click on \\i Apply\\i0  to save the changes. If you don't then the changes won't\\par be saved to the database, they will only be valid for this session.\n"
"\\ul\\b Variable string\\ulnone\\b0\t\t\\ul\\b What it expands to:\\ulnone\\b0\n"
"%miranda_path%\t Expands to your miranda path (e.g: c:\\\\program files\\\\miranda im).\n"
"%profile_path%\t\t Expands to your profile path - the value found in mirandaboot.ini,\n"
"\t\t\t\\ul ProfileDir\\ulnone  section (usually inside miranda's folder).\n"
"%current_profile%\t Expands to your current profile name without the extenstion.\n"
"\t\t\t (e.g.\\b default\\b0  if your your profile is \\i default.dat\\i0 ).\n"
"\n"
"\n"
"\\ul\\b Environment variables\n"
"\\ulnone\\b0 The plugin can also expand environment variables; the variables are specified like in any other\\par program that can use environment variables, i.e. %<env variable>%.\n"
"\\b Note\\b0 : Environment variables are expanded before any Miranda variables. So if you have, for\\par example, %profile_path% defined as a system variable then it will be expanded to that value\\par instead of expanding to Miranda's profile path.\n"
"\n"
"\\ul\\b Examples:\n"
"\\ulnone\\b0 If the value for the ProfileDir inside \\i mirandaboot.ini\\i0 , \\ul ProfileDir\\ulnone  section is '.\\\\profiles', current\\par profile is 'default.dat' and miranda's path is 'c:\\\\program files\\\\miranda im' then:\n"
"\\b %miranda_path%\\b0  \t\t\t will expand to 'c:\\\\program files\\\\miranda im'\n"
"\\b %profile_path%\\b0  \t\t\t will expand to 'c:\\\\program files\\\\miranda im\\\\profiles'\n"
"\\b %current_profile%\\b0\t\t\t will expand to 'default'\n"
"\\b %temp%\\b0\t\t\t\t will expand to the temp folder of the current user.\n"
"\\b %profile_path%\\\\%current_profile%\\b0  will expand to 'c:\\\\program files\\\\miranda im\\\\profiles\\\\default'\n"
"\\b %miranda_path%\\\\plugins\\\\config\\b0\t will expand to 'c:\\\\program files\\\\miranda im\\\\plugins\\\\config'\n"
"\\b '   %miranda_path%\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\     '\\b0\t will expand to 'c:\\\\program files\\\\miranda im'\n"
"notice that the spaces at the beginning and the end of the string are trimmed, as well as the last \\\\\n"



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
