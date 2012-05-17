VersionInfo, v.1.4.3.4
   by Luca "Hrk" Santarelli, Cristian Libotean
   hrk@users.sf.net, eblis102@yahoo.com
   Thanks go to: StormLord, Cyreve, Strickz, Wintime98
   All of them helped me somehow.

- INFO -

Hi :-)

This plugin is born from a feature request on the Open Discussion forum.
Its purpose is to display some informations related to Miranda and to the OS
so that you can copy/paste them in a bug report.

Remember: submitting a bug report DOES NOT mean that it will be fixed.
You need to check the bug report because developers may need your cooperation,
testing debug builds or making other tests.

The displayed informations are:
§ Date/Time of report
§ System cpu type
§ OS name and version (major, minor, build, additional data)
§ Installed RAM
§ Miranda version, being it or not a nightly and/or unicode.
§ List of enabled plugins (filename, short name, version and last modified date)
§ List of disabled plugins (filename, short name, version and last modified date). Optional.


- USE -

!!! For bugs and/or suggestions please email me, eblis102@yahoo.com (please don't spam hrk as
he's not working on this anymore).

Click on the button in the main menu or the one in the options... and you're done.
You can choose where to debug: text file, message box (no copy/paste), dialog
box (copy/paste), OutputDebugString(), Clipboard directly. You can also check to print
to clipboard while printing to any of the other options (except clipboard, of course :) )


- CHANGES -

+ : new feature
* : changed
! : bufgix
- : feature removed or disabled because of pending bugs

v. 1.4.3.4 - 2009/11/13
	+ x64 support (not tested !!)

v.1.4.3.3 - 2008/02/15
	! Plugins that do not have the extension .dll will not be listed anymore

v.1.4.3.2 - 2008/01/31
	* Report plugins without UUIDs in Miranda 0.8 #9 or later as Unloadable instead of Inactive

v.1.4.3.1 - 2008/01/30
	* When in service mode enable showing of disabled plugins if 'forum style formatting' is disabled, regardless if the option to show disabled plugins in the options is checked or not.
	! Ignore case when checking dll name.

v.1.4.3.0 - 2008/01/29
	* Show messages using a popup if a popup plugin is available
	+ Added service mode functionality - requires miranda 0.8 #9 or later.
	WARNING: Please rename the plugin back to svc_vi.dll if you're using Updater to update your plugins.

v.1.4.2.7 - 2008/01/25
	! Fixed crash when displaying information for unloadable plugins

v.1.4.2.6 - 2008/01/24
	* Changed beta versions server.

v.1.4.2.5 - 2007/10/24
	+ Added profile path to VersionInfo report.

v.1.4.2.4 - 2007/09/26
	* Show warning message when user tries to activate 'Show plugin UUIDs' option.

v.1.4.2.3 - 2007/07/25
	! Fixed time zone offset calculation.
	! Fixed negative time zones display.

v.1.4.2.2 - 2007/07/24
	+ Added time zone information for profile creation date and language pack modified date.

v.1.4.2.1 - 2007/06/22
	* Show menu item is always checked (to disable use the menu editor).
	* Enabled unicode flag since plugin is 2in1.

v.1.4.2.0 - 2007/06/22
	* Show UUIDs checkbox is now visible only when 'show expert options' is checked.
	* Added checkbox to enable/disable showing installed languages - visible only when 'show expert options' is checked.

v.1.4.1.5 - 2007/06/21
	* Removed installed languages info from report (still available using the hidden setting 'ShowInstalledLanguages')

v 1.4.1.4 - 2007/05/11
	! Show more than 2g of memory if available.

v. 1.4.1.3 - 2007/03/27
	* Fix for database plugins returning non standard profile path (dbxSA).

v. 1.4.1.2 - 2007/03/22
	! Fix Unicode plugin detection.

v. 1.4.1.1 - 2007/03/07
	+ Added icon to output dialog window.
	* Make 'Customize using folders plugin' translatable.

v. 1.4.1.0 - 2007/03/07
	* Changed plugin UUID ( {2f174488-489c-4fe1-940d-596cf0f35e65} ).
	+ Added VERSIONINFO interface.

v. 1.4.0.4 - 2007/03/06
	+ Option to show UUID info.
	+ Added VI UUID.
	+ Added hidden setting to change the character shown if plugin has a UUID defined (UUIDCharMark).
	! Do not enable Apply when first entering the options.

v. 1.4.0.3 - 2007/02/12
	+ Read plugin Unicode aware flag. Only shown if plugin name doesn't contain 'unicode' or '2in1'.

v. 1.4.0.2 - 2007/01/31
	* Changed beta URL.

v. 1.4.0.1 - 2007/01/28
	+ Added InternetExplorer version

v. 1.4.0.0 - 2007/01/28
	+ Added Windows shell information.
	! Fix for profile creation date and size on Windows 98

v. 1.3.2.2 - 2007/01/07
	+ New version resource file.
	+ Added OS languages info: {User UI language/}System UI language | Current user locale/System locale [available system languages]. User UI language is not available for Windows 95, 98, Me and NT.
	+ Folders support.
	! Close thread handle.

v. 1.3.2.1 - 2006/12/07
	* Changed controls tab order.

v. 1.3.2.0 - 2006/11/26
	* Reorganised the post a bit.
	+ Added empty blockquote style.
	! Fixed <u><b> version number highlighting.
	+ Added dll version info.

v. 1.3.1.1 - 2006/11/15
	* "Ask every time", not "Ask everytime" :)
	+ Added administrator privileges information.
	+ Added "Save to file" button to dialog.
	* Disabled the "Upload" button.

v. 1.3.1.0 - 2006/11/1
	+ Added multiple CPUs info.
	+ Added WOW64 info.
	! Don't get plugin information twice.
	* Moved main menu entry near Help.
	* Default output location changed to dialog.

v. 1.3.0.4 - 2006/10/16
	* Made langpack retrieval more robust.
	+ Show langpack info.
	* No more dependency on winsock2 (removed upload feature for now, support isn't available anyway)

v. 1.3.0.1 - 2006/09/25
	+ Updater support (beta versions)
	! Enable the Apply button when combobox items are changed.
	+ Added quote, size and bold comboboxes to ask dialog.
	* Sections now use the same highlighting method as the plugins.
	* Changed text displayed when plugins don't want to load with current Miranda version.

v. 1.3.0.0 - 2006/08/08
	* Use relative paths. The file path in the options will show as absolute but it's saved in the database as relative - uses the services in m_utils.h.
	+ New versioninfo icon - thanks Faith Healer.
	+ Option to automatically upload VI post to vi.cass.cz :) - currently there's no support on the site for it
	+ Show plugins that fail to load for unknown reasons.
	* Use mir_snprintf where possible.
	+ Added checkbox for forum style formatting in ask dialog.
	* Changed plugin section to "Services"
	
v. 1.2.0.4 - 2006/07/26
	* 'Attempt to find unloadable plugins' is checked by default now and is grayed out on Win 98.
	+ Added check for Windows 2003 R2.

v. 1.2.0.3 - 2006/06/01
	+ Added a service to get the VersionInfo post as a string.
	
v. 1.2.0.2 - 2006/05/20
	! Fixed a bug when notifing of errors - thanks for the patch Peter (UnregistereD).
	
v. 1.2.0.1 - 2006/05/17
	* People reported they don't have PF_NX_ENABLED defined so i created a workaround for those who want to build VI from sources.
	! Destroy services the proper way.
	
v. 1.2.0.0  - 2006/05/01
	+ Added DEP information.
	* Show message if CPU doesn't recognize cpuid extension.
	
v. 1.0.1.19 - 2006/03/23
	! Fixed "Do it now" being disabled when first entering the options window.
	
v. 1.0.1.18 - 2006/03/09
	+ Added a new size option for phpBB forums.
	+ Added a new highlight option (bold and underline).
	+ Added a new option to suppress header and footer information - will make the post a bit smaller.
	! Fixed a bug when information was copied to clipboard.
	
v. 1.0.1.17 - 2006/01/01
	+ Added check to find out plugin information for plugins that refuse to load.
	! Fixed a new line issue (happened when Unloadable plugins entry was visible).
	
v. 1.0.1.16 - 2006/01/31
	* Removed the static edge from the dialog buttons.
	+ Added check for plugins that refuse to load.
	* Updated the translation strings.
	
v. 1.0.1.15 - 2005/10/17
	+ Added some hidden beautification settings. You need to use dbeditor to add these settings to
		VersionInfo. All settings are strings and they should either be bbcodes or html codes. (do a default
		output for an example).	Check the changes in version 1.0.1.14 to see how you can activate this output.
		~ BeautifyHorizLine - code for a horizontal line 
				(default: <hr />)
		~ BeautifyBlockStart - code for a blockquote (start code) that separates the hw and miranda settings from the plugins 
				(default: <blockquote>)
		~ BeautifyBlockEnd - code for a blockquote (end code) that separates the hw and miranda settings from the plugins
				(default: </blockquote>)
		~ BeautifyActiveHeaderBegin - code for the font and size of the active header text (start code)
				(default: <b><font size="-1" color="DarkGreen">)
		~ BeautifyActiveHeaderEnd - code for the font and size of the active header text (end code)
				(default: </font></b>)
		~ BeautifyInactiveHeaderBegin - code for the font and size of the inactive header text (start code)
				(default: <b><font size="-1" color="DarkRed">)
		~ BeautifyInactiveHeaderEnd - code for the font and size of the inactive header text (end code)
				(default: </font></b>)
		~ BeautifyUnloadableHeaderBegin - code for the font and size of the unloadable header text (start code)
				(default: <b><font size="-1"><font color="Red">)
		~ BeautifyUnloadableHeaderEnd - code for the font and size of the unloadable header text (end code)
				(default: </font></b>)
		~ BeautifyPluginsBegin - code for the font and size of plugins (start code)
				(default: <font size="-2" color="black">)
		~ BeautifyPluginsEnd  - code for the font and size of plugins (end code)
				(default: </font>)
		How it works:
		$starting info [Miranda IM ... Miranda's homepage ... Miranda tools]
		{Horizontal line}{block start}Report generated at: $date
		$hardware, os and miranda settings
		{block end}{horizontal line}{active header start}Active Plugins (#):{active header end}
		{plugins start}
		$plugins
		{plugins end}{horizontal line}{inactive header start}Inactive Plugins (#):{inactive header end}
		{plugins start}
		$plugins
		{plugins end}{horizontal line}{unloadable header start}Unloadable Plugins (#):{unloadable header end}
		{plugins start}
		$plugins
		{plugins end}{horizontal line}
		$end info [End of report ...]

v. 1.0.1.14 - 2005/10/16
	- Removed the logging functions used when attempting to find an unloadable plugin.
	+ Added hidden option to beautify the output. You need to use dbeditor and add a new
		byte value to VersionInfo called Beautify and set it to 1 for the setting to take effect.
		Forum style formatting must be OFF; although forum style is unchecked it will get the
		highlighting information from the appropriate checkbox so be sure to set it 
		accordingly (usually to <b /> or <u />. The formatting is hardcoded at the moment
		(it's set to html tags), i might add some strings in the database for it later on.
	+ Added option to select how to highlight version number; currently you can
		select from bold or underline.
		
v. 1.0.1.13 - 2005/10/14
	+ Added option to ask where to print the output.
	
v. 1.0.1.12 - 2005/10/06
	* Changed the way versioninfo scans for enabled/disabled plugins.
		Now it should be able to detect even plugin that were reported as inactive before (like aim toc2 and clist_nicerW+)
	+ Added profile size and creation date.
	+ Added information about missing statically linked dll files. Use with care, might crash miranda on some systems.
		This function might not work (correctly) on windows 98 and below. If you check this option a file named 
		'versioninfo.log' will appear in miranda's folder; in case of a crash please post this info in the forums
		(thread Plugins->Versioninfo format style) using [code][/code] bbcodes.
	  
v. 1.0.1.11 - 2005/09/24
	+ Added option to print version number in bold or not.
	+ Added [code] bbcode and empty size field (if the forum doesn't strip other bbcodes from inside a [code])
	
v. 1.0.1.10 - 2005/09/25
	! Fixed issue that prevented the apply button to be enabled when changing the filename.
	+ Added options to select forum bbcodes.
	
v. 1.0.1.9 - 2005/09/22 :)
	+ Added a new CPU check. Might be unstable and can't detect very old processors (like pentium mmx)
	* Changed hidden settings strings to QuoteBegin, QuoteEnd, SizeBegin, SizeEnd.
	! Fixed updater issue - older versions should be detected as well (thanks sje). Plugin should
          comply with updater rules from now on.

v. 1.0.1.8 - 2005/09/22
	* Changed month number to month short name (english locale)
	+ Added "hidden" settings for 'quote' and 'size' strings (only valid when using forum style formatting).
		You need to use dbeditor++ to add these.
		"QuoteHeader" - String value containing the starting quote string.
                                You can use this if you want to change the "[quote]" string to "[code]" for example.
                                If the setting is not present it reverts to "[quote]"
		"SizeHeader" - String value containing the starting size string.
                               You should use this if the forum software doesn't recognize "[size=1]" and needs
                               something like "[size=1px]". If the setting is not present it reverts to "[size=1]".
		"QuoteFooter" - String value containing the ending quote string. If the setting is not present it reverts to "[/quote]"
		"SizeFooter" - String value containing the ending size string. If the setting is not present it reverts to "[/size]"
		!If you change the header for one of these don't forget to change the footer as well (and vice versa).
	+ When using forum style formatting the version number will be bold.
	+ Added active and inactive plugins count (it can't hurt :) )

v. 1.0.1.7 - 2005/09/21
	! Fixed double enter issue
	! Fixed unicode detection problem


1.0.1.6 - 2005/09/11
	* Changed miranda's homepage from 'http://miranda-icq.sourceforge.net/' to 'miranda-im.org'
	* Changed miranda tools link.
	* Changed some strings (date and time related).
	+ Added check for unicode core.
	+ Added miranda path to versioninfo output.
	+ Added plugin's last modified date to versioninfo output.
	+ Added plugin's short name to versioninfo output.
	+ Added os check for windows 2003.
	+ Added option to show information window in taskbar
	+ Added option to always copy to clipboard
	+ Added CPU info
	Should still work with amd 64 (unsure)

1.0.1.5 * Works on AMD64 now.
        * Fixed some typos in the text and URLs.
        * Changed some default settings.

1.0.1.4 * Fixed the crashes when a .dll file was found but was not a plugin.
        * Filesize somehow increased to 80KB... don't ask me why.
        + Added support for MirandaInstaller/Wassup by Tornado.

1.0.1.3 * No change at all, but it now works for everyone. Don't ask me how.

1.0.1.2 * Fixed NT detection.
         * Fixed click on X button in dialogbox.

1.0.1.1 * Changed the option page UI to reflect Miranda's one and to have a
        more intuitive usage.
        * Size has increased, but there's not anymore the need for that
        external DLL. :-)

1.0.1.0 + Added "DialogBox" as output (Cyreve, Stormlord, myself)
        + Added "OutputDebugString()" as output
        + Added "Clipboard" as output
        + Added the missing button in the options
        + Added a cool icon
        + Added OS name (Wintime98, myself)
        + Added Miranda build time (Wintime98)
        + Added free disk space on Miranda partition (Stormlord, myself)
        * Changed "Debug to:" to "Output to:" because of its meaning

1.0.0.0 First release.
        DialogBox is not yet selectable.
        Developers, read the REBASE information.


- TRANSLATION -

The strings you can translate are these:

for version 1.4.2.4
;Plugin description:
;[Collects and prints information related to Miranda, the plugins and the OS.]
;
;Option dialog
;[Plugins]
;[Version Information]
;[Output to:]
;[Text file]
;[MessageBox()]
;[DialogBox]
;[Show window in taskbar]
;[OutputDebugString()]
;[Clipboard]
;[Upload to site]
;[Ask every time]
;[Also copy info to clipboard]
;[Forum style formatting]
;[Highlight version number using]
;[Show disabled plugins too]
;[Show plugin UUIDs]
;[Attempt to find unloadable plugins (doesn't work on Windows 98)]
;[Suppress header information]
;[Add a menu item to the main Miranda Menu]
;[You will need to restart Miranda to add/remove the menu item.]
;[Do it now]
;[Upload site settings]
;[Username]
;[Password]
;[Customize using folders plugin]
;[Are you sure you want to enable this option ?\nPlease only enable this option if you really know what you're doing and what the option is for or if someone asked you to do it.]
;[Show plugin UUIDs ?]
;
;Dialog box
;[Upload]
;[Close]
;[Copy text]
;
;Miscellanea
;[If you are going to use this report to submit a bug, remember to check the website for questions or help the developers may need.\r\nIf you don't check your bug report and give feedback, it will not be fixed!]
;[Information successfully written to file: \"%s\".]
;[Error during the creation of file \"%s\". Disk may be full or write protected.]

;[Ok, something went wrong in the \"%s\" setting. Report back the following values:\nFacility: %X\nError code: %X\nLine number: %d]

;[The clipboard is not available, retry.]
;[Information successfully copied into clipboard.]
;[Miranda Version Information]

- DISCLAIMER -

This plugin works just fine on my machine, it should work just fine on yours
without conflicting with other plugins. Should you have any trouble, write me
at hrk@users.sf.net where "sf" must be changed to "sourceforge".
Anyway, if you are a smart programmer, give a look at the code and tell me
the changes you'd make. If I like them, I'll put them inside. :-)

This plugin is released under the GPL license, I'm too lazy to copy it, though.
Anyway, if you do have Miranda (and you should, otherwise this plugin is
pretty useless) you already have a file called GPL.txt with tis license.
Being GPLed you are free to modify or change the source code (you can find it
here: http://nortiq.com/miranda/ and look for the source section) but you
cannot sell it.
As I already wrote: if you do modify it, notify me, I don't see a good reason
not to share the improvements with the Miranda community. :-)

Yes, I have made a quite long disclaimer, I can save the file now. :-)
