***************************************
*  Webview Protocol Plugin by Bumper1 *
***************************************



About
-----

A plugin for Miranda IM which adds web pages as contacts to your contact list and can 
display text and/or changes within those pages in a window, as well as issue different types of
alerts when changes occur.

You need special status icons for this protocol, these icons are available here:
http://www.miranda-im.org/download/details.php?action=viewfile&id=1274
-------
WARNING
----------

On a large site (approx ~50kbytes) with "clean up display" option enabled the processor
may approach 100% until the tags are removed and text formatting is complete.
To minimise this only select a section of the web page to be displayed (the larger
the selection the longer it will take to remove the tags and format the text).


Installation:

Copy the dll file to your plugins folder like any other plugin, but if you want the ping fuction to work, copy psite.bat to the same folder as miranda32.exe

Features
--------
Creates contacts on your Contact list representing web pages. A dialog displaying
the text on that web page can be accessed through the contact menu of the contact
that represents that web page.

You can add more sites to the contact list using the Webview -> Add Contact
item on the Main menu.
You can update your Webview contacts all at once using the Webview -> Update All Webview
Sites item on the Main menu.
You can enable/disable timed updates of your contacts using the Webview -> Auto Update
Enabled item on the Main menu.

If you have the main menu items disabled and you have the TopToolbar plugin loaded Webview
will load three buttons on the toolbar for Adding contacts, updating all Webview site and
for enabling auto update.


There are various options which you can set from each individual site/Webview contact on your
contact list; you can set the name of the contact, the site URL,  start string, end string, logging
options, clean up of the display (removal of excess whitespace, tags and conversion of HTML
character codes to displayable characters), and the action to take when double clicking on a
contact (either open that site in a browser window or display site in the Webview data display
window). You can adjust all these settings from the Contact Options item on each contact menu.
The amount of white space removed can be set by the user for each contact.

There is also an option to "Display whole web page"; if you use this option then it will
display the whole web page in the data display window. 
If you use the option "Display between Start and End strings then you need to use
unique strings on the web page to set the part of the page you want to display.

The clean up display option stops tags being displayed in the data window.
It also removes extra white space to help tidy up the display. The amount
of white space removed can be set from options.

You can also set various alerts on a contact by contact basis. You can set an alert to be
issued if a certain string is present, if anything on the whole web page (or in the displayed 
portion of the web page) changes or you can have an alert issued only of a specific 
part of the web page changes.

You can also set the method by which you want to be alerted on a contact by contact basis.
You can set the alert to issue a popup using the PopUp plugin, open the data display
window, display the alert using the OSD plugin or log to file.



You can set the update interval, suppression of error messages, disabling of the main menu item
(hiding main menu item requires a reboot for changes to be seen), whether to update data on
Miranda startup, hiding/showing of the protocol icon on the status bar, updating data for a specfic
site when its data display window is opened and text/background color all from the main
Miranda Webview options. You can also set the text attributes. 
You can set an option to save the position of individual data display windows.

You can set the error messages to be displayed using the PopUp
plugin (or the OSD plugin if the Popup service isn't available).
If you don't select errors to be displayed with the Popup plugin (or the OSD plugin), or neither
the Popup or OSD plugins are loaded then the error is displayed using a balloon tip (if the OS
supports it).

When plugin is first installed the window updates once an hour.
The timer can be set in options, setting timer to 0 disables updates
( plugin will still do one update on start up even if timer=0).

The "Update Data" button on the data window will re-download date from the specific website
that data window uses.
The "Stick to the front" button on the data window will allow or stop the display window 
staying on top of other windows.
The "Find" button on the data window will allow you to find a word or phrase in the data
window.

You can copy, cut and delete text from the data display window as well
as copy all, select all and clear all text by a drop down menu
when you click with your right mouse key on the edit area of the data
display window.

The web page being available, down or lagging, server error returned or alert issued will all
change the status icon on the contact list for the specific contact that even happened to.
Online, offline, error and alert all have their own icon which can be set in Miranda's Icon 
options.

N/A status icon means the site is offline or lagging.
Away status icon means the server returned an error.
Online status icon means site is working normally.
Offline status icon means the protocol has been manually put offline.
Occupied status icon means an alert has been issued for that site.
DND status icon means plugin is downloading or processing data for that site.

You can set the sound to be used when there is an alert in the Sounds section of Miranda
Options.

In PopUp options you can set the popup time out, various options for selecting the color of the
popup and select left and right click options.

You can view the complete source code for a web page if you set the Contact Options 
to display the whole web page, untick Clean Up Display then update (download)
the data for that Webview contact.


*****
Here follow the essential settings you need to have 
right to get a result.
*****

There is an option in the contact options to put in the URL of the 
website you wish to display contents of.
Note: Netlib doesn't support redirection, make sure you dont use
plugin to monitor a site that does this because it will not work.

There are two search string options which are used to find data on the 
web page. The first is the Start string, this is a string or sequences of tags and words in the source
code of the web page where you wish to begin displaying data on your data window.
The second is the End string, this is a string or sequences of tags and words in the source code of
the web page where you wish to stop displaying data on your data window.
Note: To find these display strings you should know how to look at the source code of 
the web page you are looking at. If you don't wish to use a browser to go find out what the
source code looks the you can use the search strings like this:
Start:<html 
End:</html> 

or 
Start:<HTML
End:</HTML>

If you do not want to have to use start and end strings you can select the option
to display the whole site then the plugin will display the contents of the entire
web page.

There is a button to the Contact Options dialog to copy the display
Start and End strings to the alert Start and End strings and set
the event type to "Alert When A Specific Part of Web Page Changes".
*****


Requirements
------------
 - Miranda IM 0.7++
(Popup plugin recommended
 but not essential)

Popup plugin is used for displaying error messages, but OSD plugin can also be used.


To Do
--------
-Unicode support!!!
-need to review the plugin documentation and see what needs to be added or removed from it.
-window option to load cache into window and have it processed?
-create webview group on contactlist and automatically add sites to it?
-menu item to close all webview windows?



Changelog
----------
0.0.1.0
-Use SETI@Home 1.3.0.0 sources as 
  base code for Webview
-Change icons
-rename and remove various options
 and DBKeys
-Lets set some HTML headers to keep
 some sites happy
-Successfully displayed selected data from
 a site using search strings
0.0.2.0
-Removed some old data window functions
-Prevent users using an end string, thats present
 in the data BEFORE the search string.
-Created a dialog for displaying data,
 discontinue using MessageBox()
-Don't open another dialog if one is
 already open
-Update dialog automatically after download
-Clear data in dialog only if new data exists
-Display complete page if no search strings 
 set or if not present
-Added back "stick to front" and "hide/show" features
 to code.
-Treat a destroyed window the same as a hidden
 one in options and menus
-Added back right-click menu and automove
 to code(automove a little buggy)
-Add hyperlink to dialog
-Use dllname in titlebar
-Add resize code
-Added Multiwindow code(buggy)
0.0.3.0
-Back out displaying complete page if no search strings 
 set or if not present (results unpredictable)
-Update hyperlink  when changed in options
-Back out code to allow automove to be used if
 dragged with titlebar.Caused plugin to consume 
 all system resources when first minimized then attempted
 to restore/maximise/close.
-Make sure correct url value present before
 opening it using the hyperlink
-Removed option and key for disabling automove,
 user can just drag with the titlebar
-Removed all options and keys related to
 saving to file, not really needed,
 data can be copied and pasted from 
 data window.  
-Removed option for clearing old data.
 Code now does this automatically if 
 needed
-Removed common dialog library
-#include <richedit.h>
0.0.4.0
-Got rid of the old data structures
-Positioned the close button a little better
-Check to see that Miranda version is at least
 0.3.2.0 before loading plugin
-Added richedit control to dialog
-Hotkeys working again
-Backout Multiwindow code, much too buggy
-Intermittant crash on startup.Cause seems 
 to be that popup service isn't threadsafe.
 Don't call from thread, use a timer to call
 an error function that will check for presence
 of errors
-Unregister then reset hotkeys after dialog is
 created
-Got rid of some old unused variables
-rebase to 0x2d900000
-Use modeless dialog instead of
 MessageBox()
-Make sure there isn't more than one error 
 dialog with same message created (assuming
 user closes all the error messages)
0.0.5.0
-If the update interval is set to zero
 don't activate the error reporting timer on
 startup
-Only do update on startup if interval is not
 zero
-If the update interval is changed to a value
 other than zero then restart the error reporting
 timer
-Added advanced options to control displaying text
 before or after the search strings
-Got change background color option working
-Error check is now run one minute after update
-Added a button in advanced options to check 
 for errors
-Make data dialog read only
-Error timer not restarting with correct interval
-Moved Load() to main.c
-Added support for PluginUninstaller
-Renamed license and readme to make 
 easier to uninstall
-Got "use windows colors" option
 working
0.0.6.0
-Replaced the right click menu with a button
 on the data display window
-Use main plugin icon for button
-Improved code for setting background
 color
-Got text color working
-Change memory usage depending
 on data download size
-Write downloaded data to the
 database(maybe just a temporary
 measure)
-Clear display before write new data
 to dialog
-Don't allocate/free memory or use
 download arrays if download unsuccessful
-URL not showing on dialog after reboot
-Initialise dialog with web data stored in 
 database
-Cleaned up an icon
-Removed writing data to database
-Only free memory when new data available
 and in Unload() so all functions can
 access data
0.0.7.0
-Wrote basic data save to file function
-Readded common dialog library and restored 
 the saving functions to similar level to
 that in SETI@Home plugin
-Changed the advanced search options a little
 to avoid confusion.
-got basic tag erasing code in place
-Fixed crash in tag erasing code
-Got tag with attributes erasing working
-Support filtering both upper and lower
 case tags
-Remove excess whitespace
-Improvments to background color code
-small changes to text coloring code
-Got rid of "Use windows colors" option
-Put in an option to enable filtering
 of tags and whitespace
-Fixed crash while removing whitespace 
 if search strings were not found/set
-Choice to enable using popup plugin or not 
 even if popup service exists
-Fixed some error dialog bugs
-Fixed automove, now works better
 with titlebar, removed the easy move
 feature for now
-Don't save win size, position etc. when
 window max'ed or min'ed, also don't
 automove()
-Save width of window
-Fixed bug where the saved window size/position
 wasn't being used
-Save new window size/position after resizing
-Holding down ctrl key while moving 
 data dialog disables automove
-Load old NETLIBHTTPREQUEST struct
 if Miranda version less than 0.3.3
-Remove some old unneeded variables
0.0.8.0
-Warning on startup if Miranda version less than
 0.3.2
-Disable "Use Popup Plugin" option if popup
 service isn't loaded
-Other small changes to Advanced Options
-Resistered the webview protocol and
 added basic services
-Allow return to be allowed in the richedit
 control with out closing the dialog
-Add maximise and minimise buttons to data
 dialog
-Got rid of horizontal scroll bar on data
 dialog, the richedit control automatically
 wordwraps
-Added contact to clist, make it name itself to
 the same as the dllname
-Don't add contact to list if a contact already
 exists
-Changed the minutes setting to spin control
0.0.9.0
-Change main menu items dynamically
-Changed options dialog to make visually
 better.Removed some options, changed others
-Use balloon tip for error messages
-Removed "use popup plugin option".
 Plugin now uses the following 3 methods for 
 displaying errors(in decreasing priority)
 *Balloon tip(if OS supports)
 *popup plugin(if service loaded)
 *Error dialog(if the neither balloon tip
  or popup can be used)
-If server is down but previously gave back
 a server error then make sure the server error
 is cleared before reporting that the server is down.
-Control contact status by global status and protocol
 status
-Don't download data if current protocol status
 is offline
-Change contact status depending on download
 *Download success = online
 *server offline   = n/a
 *server error     = away
-Added item to contact menu to show/hide
 data dialog
-Treat <a> same as other tags for now
-first public alpha release
0.0.10.0
-Remove support for 0.3.2 NetLib
-Don't load plugin if Miranda version
 less than 0.3.3
-Remove hbrush, not used anymore
-Fixed intermittant crash when contact
 already exists
-minor changes to options
-added items to contact for update,stick to
 front and open web page.
-New icons for show/hide window om menu menu.
-Dynamically change contact menu item text
 and icons
-Some changes to drop down menu
-URLs now open webpage when clicked
-Added new Unstick icon
-Removed advanced search options.Search
 strings are now always included in displayed
 data (prior to tag filtering)
-Added Webview Group to clist to hold
 contacts generated by multiple instances
 of the plugin
0.0.11.0
-make spin control work in the correct direction
-Moved options to Network group
-Dynamtically change text on data dialog
 menu items
-Remove stick to front and update data Main menu
 items
-Stop sending Content-Length header to sites
-Moved save to file options from advanced
 options dialog to options dialog
-Removed check for errors option
-Made logging to file an advanced option
-Added multiple monitor support
-Destroy data window when ME_SYSTEM_PRESHUTDOWN
 event occurs
-reduce disk I/O by storing colors,
 window height and window position
 in global variables.Variables only
 read/written to DB on start/exit.
-added copy/copy all/select all menu to right click
0.0.12.0
-Added size grip to data dialog
-Added separate stick/unstick button to data dialog
-Removed stick/unstick item from drop down menu
-Made buttons into flat buttons
-Make WM_CLOSE hide the data window instead of 
 destroying it.
-Removed Update and Stick/Unstick contact menu
 items
-Moved main menu item position to the same place 
 as other protocols
-Removed advanced option dialog for now
-Updates to README
0.0.13.0
-Double clicking on contact opens web page in browser
-Added some comments to menu items. Some menu items 
 will be removed in a future release, and other new
 items will be added(new items are disabled for now)
-New icons for Add user and Contact options menu items
 and new icon for drop down menu button on data window
-Added some more tags to the filter
-Change (Latin, Numeric and some punctuation) HTML
 character codes into characters.
-Change data window caption to the URL.
-Change contact name to the URL.
0.0.14.0
-Added icon to data window titlebar
-Filter out some more tags
-Added update button to data window
-Removed drop down menu button (no
 need for anymore, all of the functions
 of it can be accessed from the data window)
-Added Clear All, Paste, Cut and Delete
 to right click menu
-Fixed a bug where double clicking on any
 contact from any protocol opened up
 a page in a browser.. opps
0.0.15.0
-Added tooltips to buttons
-added more tags to filter
-Added contact options dialog (disabled
 for now)
-Crash when tag filter sometimes tries to write
 outside of the array (hopefully fixed).
-Crash with some sites crash if free 
 szInfo after tags are filtered 
 out(hopefully fixed)
-Crash (possibly related to the crashes above),
 no idea what causes it really but fixed by 
 a delay midway in the tag filtering function
0.0.16.0
-Added some more tags to filter
-Opps..was dependant on other plugins 
 to load richedit into memory
-Removed options in Miranda options
 for hiding and sticking window to the
 front(no longer needed because soon
 will be moving to contact based options)
0.0.17.0
-Enabled the contact optons dialog
-Removed Url, start string and start string
 options from plugin options to the contact 
 options dialog
-Store url, start string and start string
 in the contact settings not in the plugin
 module in the database.
-reduced some crash risks
-Added option to enable updating data on
 Miranda startup
0.0.18.0
-possible crash when freeing memory in unload,
 free in pre-shutdown instead (possibly isn't
 the solution)
-Enable the add contact menu item.
-Some other changes to usage of malloc
 and free
-Small delay added before downloading data 
 on startup, also reduced a delay else
 where in the code
-Added code to check multiple sites
-Some more code to try and avoid crashes
-Change individual contact status to indicate
 site status
-Remove hotkeys (possibly temporarily)
-Move save to file options to contact options
0.0.19.0
-More stability improvements
-Stick to front option working again
-Disable file save options in add contact dialog
-Code cleanup
0.0.20.0
-URL sometimes corrupted
-Got background and text color change options 
 working
-Removed feature where timer set to zero stopped you
 downloading data manually
-Clear all data windows when changing text/background 
 color (possibly a temporary measure)
-More code cleanup
0.0.21.0
-User can choose either to open web page 
 in a browser or opening the data window
 as the action taken when double clicking
 on a webview contact
-Option to update data when data window 
 opened 
-Removed old show/hide contact menu item
 and replaced with a new open/close menu item
 with a new icon
-New add contact icon and show more icons
 on dialogs
-Better handling of protocol/global status
-Other minor fixes
-Added a new contact option to add
 contact/site name.
-When adding a new site,if you leave the site name 
 blank then the site name will automatically be
 the same as the sites URL
0.0.22.0
-Make an option to hide icon on status bar
-show the icon again if there is a crash
-Make the titlebar text of the contacts
 data window the same as the name of the
 contact
-only look for end string further down
 the page from the start string
-Remove garbage from end of displayed
 text
0.0.23.0
-Got the options on the data display window
 working properly again
-Fixed a bug where double clicking on a webview
 contact sometimes opened a message window.
-Cleaned up code
0.0.24.0
-Allow user to set separate double click action 
 for each webview contact from the contact options.
-Updated icons.
-Faster (and more stable) method for copying web data
 into memory used.
-Use Save As dialog instead of Open 
 to select log file.
-Added a search button to Webview data window.
 Search is pretty basic as yet but it shows you
 the approximate area of the window where the search
 keyword was found.
-Use monospaced font in data window because its easier
 to read text in window. Also made font bigger for the same
 reason.
-Status messages displayed on status bar of data window.
-Code to remove more unsupported symbol codes and other
 unwanted data from the data window when the clean
 up display option is selected.
-Updates to the README.
0.0.25.0
-The Find button now can find and highlight the 
 exact position of the requested word in the 
 data window.
-The Find button's text search is no longer case
 sensitive.
-Status bar on data display window now tells you
 the time the last update of data occured from
 that window.
-You can now set the exact amount of whitespace
 to filter out of the data window display through
 a trackerbar control in Miranda options.
-Font face and size as well as attributes bold, italic
 and underline can now be set from Miranda options.
-Some stability fixes and slightly better support for 
 large sites.
-Added more panes to the status bar.
-Status bar on data display window now tells you
 the number of bytes of data displayed in that 
 window and the number of bytes downloaded.
-Fixed a bug which caused some of the text not to
 be formatted correctly with larger sites.
-Some other minor changes.
0.0.26.0
-Removed some spaces left in the formatted text by the
 character code to symbol function.
-Compiled code with some optimisations.
-Added options to Miranda options to select
 the method of tag filtering; accurate, fast
 or dynamic filtering.
-Moved Display Clean-up options to the contact
 options dialog so you can set different options
 for each site.
-Moved options back to Plugins group.
-Some bug fixes for the Add Contact/Contact
 options window.
-Fixed bug where part of the code regarded
 a error reply from the server as a download
 success.
-Status messages for "Server down or lagging",
 "Server replied with an error code" and "Download
 successful now about to process data" added to
 status bar messages on the data display window.
-Some more tags added to tag filter
-More changes to the README.
0.0.27.0
-Make sure the first letter of the plugin name
 in options is always uppercase.
-Error messages now show for each individual site
 using either balloon message on the system tray
 icon or using popup plugin if available. The
 popup messages are better and that option is 
 recommended.
-New Contact Options button added to the data
 display window which replaces the Options menu item
 on the contact menu.
-Some changes to code that removes the parts of the webpage
 that are supposed to remain hidden.
-There is only one method of filtering now; the fast
 filter. All options for choosing filter method removed.
-Added Alert button to data display window.
-There are two alert methods used for now; popup
 plugin and sound file. There is one alert event 
 for now; string present in downloaded data.
-Small problem with plugin's response to global
 status changes fixed.
0.0.28.0
-Added some more translatable strings.
-Fixed some bugs in and added some improvements
 to the Alert Options dialog.
-Log to file was removed from Contact Options
 and is now an Alert option.
0.0.29.0
-Made the plugin information in the plugin listing
 more descriptive.
-Fixed bug with hyperlink on data display window by
 making control a button.
-Added cancel and Apply buttons to Add Webview Site,
 Contact Options and Alert Options windows. 
-If the user does not supply a name when creating
 a new webview contact then the plugin no longer just
 uses the URL of the site as a name but requests
 that the user supply a name. Using a URL as a
 name just looked too messy.
-When an alert is issued the date and time of the alert
 is appended to the contact name.
-The contact name in Contact Options is the contact name without
 any alert time and date appended to it.
-Fixed bug which where the presence of an ampersand in the
 url caused letters to be underlined or missing in the
 text of the url button on the data display window.
-Removed the sound alert. Now you can instead set
 a sound be played when an alert occurs from the 
 same Sounds group in options that other plugins
 use.
-Disabled the apply button on the add contact window.
-Added more translatable strings including error 
 messages, menu items and status bar messages.
-Fixed bug where the description of the amount
 of white space removal was not showing in the 
 Add Webview Site window.
-Other minor changes.
0.0.30.0
-Set the titlebar text of the display data window
 to contact name without any alert time and date
 appended to it.
-Added event-type "Alert When The Web Page Change".
 This produces a user-defined alert when the the
 contents of a page change from what was on the page
 when it was last checked. This option creates
 a file on the users harddrive for every contact
 this option is set for.
-Update URL displayed on the data display window 
 if that URL is changed in Contact Options.
-Fixed bug which caused buttons on data display
 window not to work if URL was changed.
-Use PUShowMessage to generate any popup error
 messages.
-Contact Options now requests that the user supply a name
 for a contact if one is not already supplied.
-Contact Options and Add Webview Site windows now check
 to see if there are any invalid symbols in the contact name
 (this is needed to make sure the file name is valid for the 
 cache file).
-Change the titlebar text of the data display window when
 the contact name is changed and the changes are applied (
 used to work only if user pressed "Ok").
-Made some more of the text on different windows titlebar's
 translatable.Also made some changes to the code to allow
 the translation of some of the dynamic strings which occur
 on the data display windows status bar and in alert popups.
-Added an alert type to open the data display window when
 a user defined event occurs.
-Fixed bug which caused the icon of the (Disable) Stick to the 
 Front button always to show the Disable icon when the
 window was first opened by double clicking on a contact.
0.0.31.0
-Make sure the time and date is not appended to the contact
 name on contact list UNLESS an alert has been issued.
-Miranda Translator tool was not translating some strings 
 correctly even though they were translatable. Made changes
 to the code to help the Translator extract the correct strings.
-Make the appending of the time of the last alert to the contact
 list name a contact option.
-Added a Popup option dialog to the PopUps group in Miranda 
 options so users can set the color of the Alert popups separately
 from the color of the data display window. The user can also set
 the popup delay from this option dialog.
-Added an option to Alert Options to allow the user to define whether
 the date is added to the contact name as a suffix or a prefix.
-increased thread safety for popups.
-Added webview to the "KnownModules" list.
-User can now control whether the time added to the contact name 
 is in 24 hour or 12 hour format.
0.0.32.0
-Made an install script for Miranda Installer.
-Added an item to the Main menu to allow the user to update
 all Webview sites at once.
-Cleaned up indents in source code.
-Fixed bug in the the code which removes unsupported character
 code from the display.
-Fixed bug in code which caused the plugin not to clean up the display
 if one or more of the Start or End strings was not present on 
 the web page even if plugin didn't need to check for these strings.
-Separated the sources out into smaller files.
-There was no way of adding a new webview site if the user had disabled
 the main menu items. Added options to take place of the main menu items
 in the main Miranda options dialog and made sure these options are not 
 enabled unless the main menu is disabled.
-Right clicking on a popup now opens the web site in a browser window.
-Added "Only Update Window on Alert" to main Miranda options to prevent
 contents of the data display window being overwritten if there
 is not an alert.
-Changed the automove function so it no longer moves the data display
 window back on screen (if the user moves it off of it) but just
 makes sure the position will be on screen the next time it is created.
-If Miranda crashed on exit then the window colors were not being 
 saved; save colors when the color options are changed to prevent this
 happening.
-Added a main menu item to enable or disable automatic updates of webview
 sites.
-Increased time interval range from 0-99 minutes to 0-999 minutes.
-Fixed some bugs in the the Popup options.
-Added option to use the same colors in the alert popup as is 
 used in the data display window.
-User can now set the left and right click popup actions from
 popup options. User can set the actions to open data display window,
 open web page or dismiss popup.
0.1.0.0
-First Beta release
-Make sure the first letter of the Webview menu title on 
 the Main menu is uppercase.
-Use the Miranda Find/Add Contacts dialog to add webview 
 sites to the contact list.
-Use the URL to create a name for the contact.
-Make sure the update button on the data display window
 always can update the text in the window even if the 
 "Only Update Window on Alert" option ticked.
-Fixed bug which caused an invalid window position to be written
 to the database if data display window was minimised when
 Miranda exited.
-Make sure the site name in Contact options and on the data 
 display window's titlebar is the same as the contact name
 on the contact list unless it has a date added to it.
-If Miranda crashed on exit then the window position was not being 
 saved; save position when the window is closed to prevent this
 happening.
-On slower systems data display window was seen in a different
 area of the screen when opened then it was seen to move to
 the correct position on the screen. Put fixes in place to correct this.
-Added an event type to test if a specific area of a webpage has changed
-Fixed some bugs in the Alert Options dialog.
-Fixed bug which caused update button to become disabled if the 
 user clicked the button when that site was already being downloaded.
-Check for alerts BEFORE filtering cleaning up the text.
-Added the option to display alerts using the OSD plugin.
-Fixed a bug that caused the plugin to falsely report that
 the monitored website had changed.
-Make sure the Apply buttons on the Contact Options and the
 Alert Options windows is disabled unless there is a change
 made to the options.
-If either the display or alert start/end strings aren't present on
 on a site issue an alert.
-Start/End string not present and invalid search parameter errors
 now set the contact to Away status (just like server error codes).
-Added a button to the Contact Options dialog to copy the display
 Start and End strings to the alert Start and End strings and set
 the event type to "Alert When A Specific Part of Web Page Changes".
-If a server replies with an error code display the exact error code
 returned.
-Find/Add dialog was truncating URL's.
-Added more compact Contact Options and Alert Options dialogs
-Made the comboboxes in the Alert Options dialog translatable.
-Some improvements to the code that fixes the underlined text
 bug on the url button.
-Disable both the Contact Options and the Alert Options button
 when then either then Contact Options or the Alert Options
 button is opened (need to do this to avoid a bug).
-Allow user to have more than one contact with the same URL
 if those contacts have different names. If more than one
 contact has the same name and URL then issue a warning
 when using buttons on those contacts data display window.
-Allow user to add contacts with the same URL and an existing
 Webview contact.
-Append a random number to the contact name when it is created
 to avoid creation of Webview contacts with the same name if
 they are using the same URL.
-Added a status bar message to tell the user that a site is currently
 downloading if they try to update another site manually.
-Error now display using either PopUp plugin or the OSD plugin
 if the "display errors using PopUp plugin" option is selected.
 The plugin tries to use the PopUp plugin first but if that service
 is not available then it uses the OSD plugin (if plugin is loaded).
-Removed some unneeded preshutdown code.
-Removed support for Plugin Uninstaller (that plugin will not work in
 Miranda 0.4 anyway).
-Removed main Webview option control which was used to add new Webview 
 contacts (the user can easily do this through the main menu item or
 by the Find/Add dialog if the main menu item is disabled).
-Updates to the README.
0.1.1.0
-Some improvement to the code that converts HTML character entities to 
 symbols.
-Remember contact previous status on startup.
-Stop the User Details dialog from constantly displaying "...Updating...".
--Button in the Contact Options dialog to copy the Start and End strings
  was not setting the event type to "Alert When A Specific Part of Web Page Changes".
-If "Update window only on Alert" option was ticked statusbar was always displaying
 "processing data, please wait" if all the sites were updated at the same time. Corrected
 and added another status message.
-Make sure all existing Webview contacts are set to online status the first time this
 version of Webview is loaded.
-Added a control to the main options to allow the user to set a delay of up to 120
 seconds before Webview updates on start-up.
-Make sure the timed updates cannot occur until the update on start up is finished.
-Don't log to file twice if both types of start/end strings are not present and there is an
 alert to indicate this.
-Automatically delete the cache file (if one exists) when deleting a Webview contact
 or when alerts are disabled for that contact.
-Webview now knows when a contact is renamed on the contact list.
-Rename the cache file (if one exists) when a contact is renamed.
-Added the option to save the position of individual data display windows.
-Added some delays to make the plugin slightly less CPU usage intensive
 during data download and processing.
-Added an advanced option to disable the download protection code if
 the user wishes to do this. This has the advantage of being able to 
 download more than one site at the same time but it also potentially
 could cause corruption of data.
-Fixed a bug in the main Webview options.
-Display time/date in the logs and on the status bar in the 
 users own language.
-Bug causing last letter in display to be sometimes repeated.
 This isn't a complete fix but it should stop the worst cases.
-Added option to Contact Options so user can choose to log
 data to file in its raw state or with all the tags removed.
-Added a new status (DND) to indicate the plugin is downloading or processing
 data for a particular site.
-Filter out (most) CSS attributes.
-When part of the web page has changed, change the color of the text 
 and background (and select the text) in the part of the page you are 
 monitoring so that the user can find this portion of the page easier 
 within the larger body of the displayed text.
-Made some more space for translation strings in options.
-Fixed some bugs in the EraseBlock function which causes
 the plugin to hang while processing data.
-Data window too small when it opened the first time. Made
 window bigger.
-Use SkinAddNewSoundEx instead of SkinAddNewSound.
-Prevent Miranda versions less than 0.4.0.0 from loading plugin.
-Cleaned up code.
-Added TopToolBar buttons for adding Webview sites,updating
 all Webview sites and disabling/enabling auto update.
-Only load TopToolBar buttons if Main Menu items are disabled.
-0.1.3.1
-added "prettier" icon resources.
-Removed" Add Webview Site from main menu.
-Added code to get ride of <style> </style>.
-removed TopToolbar support
-Added Contact Options and Contact Alert Options to the contact menu.
-Moved from PLUGININFO to PLUGININFOEX structure
-Moved Webview's options from "Plugins" to "Network" in Miranda's options page.
-Improved language pack support.
-Added menu item to open cache folder.
-New icon for Update All  main menu item and Open Web Page contact menu item.
-Added main menu item to mark all sites read
-Added contact menu item to update site data.
-Added Contact Menu item to ping the web site to test if there is any issue with connectivity.
-0.1.3.2
-Fixed an old bug where a DB value was getting written to any contact which was double clicked
 regardless of its protocol.
-The URL of a Webview contact is now also the contact's "Web Page" in user details.
-Stop displaying unneeded status icons on the status bar.
-Removed BBCode from popups to make compatible with YAPP.
-The progress of data processing is shown on the data window.
-Added contact menu item to stop the data processing on a site.
-0.1.3.3
-Stopped translating some clist items that should not be translated except by the core.
-Add random number to contacts name when contact is created, if its the same as the 
 name of another webview contact on the list.
-When updating all sites, don't wait for one site to finish updating before updating
 the next.
-The contact's StatusMsg now reports the site updating, offline, online, alert issued and site errors.
-Removed an old part of the data window that is no longer needed.
-Added a button to the data window to allow the user to stop data processing.
-Text in data window updates during data processing.
-0.1.3.4
-Backed out a change in the code which was causing false alerts. Contacts have to 
 wait for other sites to stop updating before updating themselves.
-Smaller cache size.
-0.1.3.5
-Destroy services on exit.
-Popup message when all sites have been updated.
-If Miranda crashes, the status icon no longer reappears on the statusbar bar, if
 the user had originally chosen to hide it.
-0.1.3.6
-Smaller dll size.
-Swapped the stick and unstick icons.
-Added option to Main Options to display site data in a Popup when there is an alert.
-Show error message if psite.bat doesn't exist and user is attempting to ping a site.
-Show error message if cache folder doesn't exist and user is attempting to open it.
-0.1.3.7
-Fixed a potential crash when updating contacts.
-If user renames a contact to a name containing a forbidden character,
 there is an error popup to inform them and the character is converted to a "_" 
 and a random number appended to the end of the name.
 -0.1.3.8
 -The "Update on Window Open" option has been changed to "Load Page on Window Open".
This change means when the option is selected, the window will first of all attempt to 
load information into the window from the cache, and only if the cache file doesn't exist
will it attempt to download the web page and display it. 
-Mark site as read when window is opened and window is set to load data on open.
-0.1.3.9
-Prevent a crash when "Add Contact" is used instead of "Find/Add Contact", and also when
  "Find/Add Contact" is used incorrectly.
- Opening a site in a browser window using double click, contact menu, the url button
  on the data window or clicking on popups, all set the contacts status to online/read.
-Added a counter to the Webview main menu, which shows how many minutes to the next update.
-Changed the "Permanent" and "From Popup Plugin" options in popup options, from radio
 buttons to buttons.
-0.1.3.10
-Made separate Unicode and ANSI versions of plugin.
-Removed some old unneeded code.
 
 









*********************
Copyright
---------
Copyright (C) 2011 Vincent Joyce

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



