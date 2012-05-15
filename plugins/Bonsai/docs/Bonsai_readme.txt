Bonsai plugin v.0.0.2.5
Copyright © 2006-2008 Cristian Libotean

This plugin lets you rearrange the options tree by changing the options 
groups and titles that plugin options are displayed under.  Now you can
move those stubborn plugin options to create the structure you want in options.

NOTE: This plugin works by using a hack, and you should backup your 
profile before installing the plugin.

***** IMPORTANT DISCLAIMER *****

Before using this plugin you should be aware that editing the groups 
and titles of plugin option pages can make it extremely difficult for 
other users to provide you with support, as they will not be able to help 
you navigate the options tree if you have edited it to suit your 
tastes.  By using this plugin you implicitly agree that any changes you make 
are carried out AT YOUR OWN RISK and you are SOLELY RESPONSIBLE for the 
consequences of any changes you make.

Please also note that this plugin may in some instances be incompatible 
with language packs, as by its very nature it can change the text to be 
translated.


Requirements
============
The plugin requires an Unicode aware system, so if you're on Windows 98 
you need Microsoft Layer for Unicode, UNICOWS.  This can be downloaded 
at the URL below:

http://www.microsoft.com/globaldev/handson/dev/mslu_announce.mspx



How to Use
==========
Change the text in group and title to the group and title you wish to 
use.  For example, to move a plugin to Plugins > MyPlugin, set the Group 
to Plugins and the Title to MyPlugin, then press Save.

To make an options page the root of a group, set the group text to an 
empty string.

To hide a plugin page completely change the title text to an empty 
string.

You can reset all plugins to their default group and title by holding down control
and clicking on the Reset button.


Changes
=======
+ : new feature
* : changed
! : bufgix
- : feature removed or disabled because of pending bugs

version 0.0.2.5 - 2011/08/23
	+ made x64 version updater aware

version 0.0.2.4 - 2009/11/16
	+ x64 support (not tested !!)

version 0.0.2.3 - 2008/01/24
	* Changed beta versions server.

version 0.0.2.2 - 2007/04/24
	* forgot some debug code in :)

version 0.0.2.1 - 2007/04/16
	+ Rebased dll (0x2F080000)

version 0.0.2.0 - 2007/03/06
	+ Added UUID ( {2fe47261-7f38-45f4-8fac-b87e974a1350} )
	+ Added BONSAI interface.

version 0.0.1.5 - 2007/02/12
	+ Added unicode flag.

version 0.0.1.4 - 2007/01/31
	* Changed beta URL.

version 0.0.1.3 - 2007/01/12
	! Fix for ANSI miranda versions.

version 0.0.1.2 - 2007/01/07
	+ New version resource file.
	* Requires at least Miranda 0.6.

version 0.0.1.1 - 2006/11/26
	+ Added dll version info.
	* Changed controls tab order.

version 0.0.1.0 - 2006/11/10
	! Fixed possible crash for plugins without a group.
	+ Added icon for hidden option entries.

version 0.0.0.8 - 2006/10/27
	+ Added refresh icon when settings get changed.
	* Restate the question when resetting all plugins to their default group and title.
	
version 0.0.0.7 - 2006/10/26
	! Fix for Miranda 0.6 tab titles on Windows 98

version 0.0.0.6 - 2006/10/25
	! Fix for plugins using the new options page structure.

version 0.0.0.5 - 2006/10/24
	+ Added icons to the list of menu entries

version 0.0.0.4 - 2006/09/29
	* Use Bonsai module to store settings

version 0.0.0.3 - 2006/09/29
	! Fix for Windows 98 (UNICOWS still required).
	! Old plugins kept their default group and title.
	* Show plugin name in lowercase on Windows 98.

version 0.0.0.2 - 2006/09/27
	* Support for oooold plugins which use a really old structure for the option pages.
	+ Major rewrite, most strings are now kept internally as Unicode.
		This means that you need an Unicode aware system to use the plugin, if you're on Windows 98 you need UNICOWS, Microsoft Layer for Unicode.
	* Changed plugin name, now it's Bonsai (thanks to the people on #miranda who suggested and voted the name :) )
	* Disable the edit windows if no item is selected.
	* Clicking Reset while holding down Ctrl will reset all option pages to their default values.
	* Ask to save changes when selecting a new item without saving first.


version 0.0.0.1 - 2006/09/26
	+ First release