SmileyAdd plugin for Miranda NG
=====================================================================
A plugin that adds smileys to the standard message dialog. 
Features:
- Emoticons in all message dialogs (SRMM, TabSRMM, Scriver and others), 
  IEView logs, History++ (History++ v.1.5.1.2 or later required) 
  logs static and dynamic images for smileys in pretty much any format (png, gif, jpg, ico, etc.) 
  as well as images packaged in DLL 
- Animated gif emoticons and provide animation for all message logs 
- Emoticons animation in a smiley selector window 
- Emoticons on contact list, in Popup+ popups, Tipper YM tooltips, etc. 
- Smiley packs per protocol 
- Smiley pack per transport (gateway between protocols) 
- Smiley packs per contact 
- Graphical emoticons in the message input area, thus it will allow you to see these images while you are typing a message 
- MSN Custom Emoticons (smileys) 
- Supports plain text and regular expression smiley codes 
- Supports sending ICQ tZers, Nudges and/or call any Miranda NG service from selector window.
- Unicode emoticons
- Translation support

Author: Boris Krasnovskiy, previous: Rein-Peter de Boer (peacow), bid, 
Email:  borkra@miranda-im.org

Emoticon Images: Angeli-Ka
Regular expression library: Jeff Stuart
Hashing algorithm: Austin Appleby

Date: June, 2012
Current version: 0.2.3.17

Licensing terms could be found below.


Installation Instructions
=========================
Extract the zip file in the Miranda NG home directory. This will create a directory
'Smileys' with the default smiley pack. And the smileyadd.dll will go in the
plugins directory. 
Or you can copy this by hand from the archive. Note: to make the default work,
the default smiley pack HAS to be in the 'Smileys' directory.

Smiley Packs
================
SmileyAdd supports *.msl, *.asl and *.xep Smiley Packs.
For syntax of in *.msl or *.asl files check the example file "smileyadd_example_libfile.msl".

Note SmileyAdd provide better validation for malformed Regular Expressions then nConvers. 
As a result some Smiley Packs working under nConvers will give errors under SmileyAdd. 
Although if these errors are fixed resulting Smiley Pack will work correctly under both plugins.

Pre-packaged Smiley Packs could be found here:
http://nightly.miranda.im/

Any Smiley Pack from SmileyAdd, IEView or nConvers sections is usable with SmileyAdd

Per Contact Smiley Packs
========================
At first you need to create custom smiley category. To do that type the name of the new category 
into the text input field in Smiley Category part of option dialog and press "+".
Now you can select this new Smiley Category for the contact. You need to right click on the 
contact select "Assign Smiley Category" and then select from the menu appeared category you want. 
To delete custom Smiley Category select in in the list in option dialog and press "-".

Limitations
===========
Non Unicode version does not support DBCS characters in smiley codes and file names. 
DBCS character sets are Chinese, Japanese, Korean, etc. 
It's recommended to use Unicode version in this case.

For true Unicode operation of Unicode SmileyAdd Unicode Smiley Packs required

SmileyAdd will work only with Miranda NG 0.92 or later. 
Unicode SmileyAdd will only work with Unicode Miranda NG and 
Windows XP or later.

GIF with "Disposal method" "To Previous" not supported

When SmileyAdd is used with IEView only smiley packs with each smiley stored 
in separate image file (phg, gif, ico, jpg, etc.) could be used

Bugs
====
No guarantees, but probably there are some bugs. If you find something, and would 
like to see this fixed then post on the Miranda NG forum as detailed as possible report 
on the bug (version numbers, OS, how to reproduce it) and I will get back on that.


Source
======
Source is published on: http://trac.mysvn.ru/ghazan/myranda/browser/trunk


Developers
==========
There is also some smiley support for plugins... There is something remotely 
resembling documentation in "m_smileyadd.h". Contact me if you have questions. 
At this moment you can: 
    - replace smileys in a RichEdit with only supplying the RichEdit handle
    - retrieve an icon by supplying the emoticon text, i.e. ":)"
    - parse text and retrieve icons for each emoticon found
    - show a smiley select window
    - contact me if you need more support (see the developer.txt also)



Thanks to:
==========
- Angeli-Ka    For excellent swmiley pack
- Jeff Stuart  For excelent regular expression library
- MatriX       For helping out with the icons and other stuff!
- Pixador      For some bug tracking :)
- Wesslén      For even more enhancing the standard message dialog and 
               fine tuning my subclass routines.. :)
- Various forum people tracking bugs for me
- Miranda NG crew for Miranda NG... :)
- Everyone pointing out bugs to me!
- Creators of all the smiley packs for SmileyAdd :)             


ChangeLog
=========

0.2.3.17
---------
 Added support for Omegle protocol

0.2.3.16
---------
 Added WLM 2011 smileys
 Added ability to disable smiley for contact
 Miranda 0.9 compatibility
 Improved support for accounts

0.2.3.15
---------
 Prevented ANSI SmileyAdd from loading on Unicode Miranda
 Fixed duplicate smiley categories
 Fixed active account determination

0.2.3.14
---------
 Fixed crash with smiley scaling
 Added x64 portability
 Fixed crashes with broken web packs
 Fixed memory leaks with bogus custom smileys
 Added Folders plugin support for web packs

0.2.3.13
---------
 Fixed one more crash

0.2.3.12
---------
 Fixed crash with bogus custom smileys
 Removed plugin smiley categories from per-contact menu
 Allow plugin smiley category in Use one for all mode

0.2.3.11
---------
 Added abilty to define and use user defined smiley categories in all modes
 Added optional high quality image scaling for icon and png smileys (GDI+ required)
 Added ability to specify minimum smiley height

0.2.3.10
---------
 Fixed smiley jumping in some cases with smileys of different sizes in one line   

0.2.3.9
---------
 Added support for zoom
 Fixed text/smileys in input area on options change
 Smarter operation (do not touch input area if no need) 

0.2.3.8
---------
 Fixed custom smileys with Metacontacts
 Fixed memory leak

0.2.3.7
---------
 Performance improvements

0.2.3.5
---------
 Fixed super fast animation
 Fixed window background draw

0.2.3.4
---------
 Use GDI+ to display TIFF smileys
 Unloading smileys after 3 min of non-display

0.2.3.3
---------
 Added protection for bogus frame numbers in smiley pack
 Web packs downloading in background now
 Redraw Message windows when packs reloaded through API

0.2.3.2
---------
 Fixed error message with text-only "smileys"
 Fixed crash on exit
 Fixed smiley display in Tipper YM
 Non gif smileys now handled through advimg if possible (no GDI+ required)
 
0.2.3.1
---------
 Large reduction in GDI resources and memory usage
 Added irc url to exclusions from parsing
 Added support for animated xep smiley packs
 Fixed Apply button behavior in Options dialog

0.2.3.0
---------
 Fixed text typing in input area
 Fixed drawing of selected animated smileys
 Added skipping timestamp during smiley parsing
 
 Reduction of memory used with animated smileys in message dialog and pre-selector

0.2.2.28
---------
 Fixed typing in the input area
 Fixed Metacontacts protocol detection

0.2.2.27
---------
 Fixed Popup+ operation

0.2.2.26
---------
 Fixes for Max custom smiley size option
 Fixed animated smiley drawing issues
 Fixed input area smiley parsing

0.2.2.25
---------
 Fixed text replacement smiley
 Added ability to specify max size for custom smiley

0.2.2.24
---------
 Added text smiley capability
 Fixed Metacontacts and per contact smiley pack
 Removed internal path conversion logic (should fix crashes)
 Lots of internal changes

0.2.2.23
---------
 Fixed crash
 Fixed hidden smileys
 Fixed assigning contact smileys when no protocol pack defined
 Renamed default menu item
 
0.2.2.22
---------
 Redesigned smiley pack selection menu 
 Reduced plugin size

0.2.2.21
---------
 Fixed problem with ANSI SmileyAdd and smileys with '?'
 Improvements for custom smileys

0.2.2.20
---------
 Fixed sending ICQ tZers
 Added ability to send Nudges through Nudge plugin
 Slight reduction in resources used

0.2.2.19
---------
 Added ability to add service calls to smiley pack 
 (allows adding ICQ tZers, Nudges, etc. to smiley selection window) 

0.2.2.18
---------
 Fixed crash with Miranda 0.7 and Jabber transports 
 Reduced amount of error notifications in option dialog

0.2.2.17
---------
 Fixed extention of downloaded smiley
 Fixed crash when loaded without initialization

0.2.2.16
---------
 Fixed https type urls 
 Fixed crash on exit

0.2.2.15
---------
 Added ability to provide http url as smiley path

0.2.2.14
---------
 Added support for not adding custom smileys to outgoing messages
 Added support for Transports
 Fixed crash with language packs

0.2.2.13
---------
 Fixed custom category name in option dialog
 Fixed smiley definitions in smiley pack

0.2.2.12
---------
 Fixed Ctrl-Up/Down operation in chat
 Rewrote smiley pack file specification
 
0.2.2.11
---------
 Fixed crash with \b regex clause
 Fixed crash with Miranda 0.8 build #8
 Fixed smiley pack definitions
 Increased size of smiley prelookup in input area
 Changed screenshot

0.2.2.10
---------
 Added new default smiley pack(s) by Angeli-Ka (has default smiley for most protocols) 
 Added auto selection of default smiley pack for protocol based on protocol type
 Fixed problems with Tab key and input area lockup
 Fixed smiley quoting in TabSRMM
 Fixed decoding special chars (double quote and spaces) in insert text and tool text

0.2.2.9
---------
 Fixed Tab expansion in Scriver chat
 Fixed input area smileys after changing options
 Fixed selection preservation after copy

0.2.2.8
---------
 Fixed smiley animation with Popup+ opening message window
 Fixed Tab expansion in Chat
 Fixed alpha transparent icon creation
 Fixed deleting custom smileys after Miranda loaded
 Fixed resource leak with custom smileys

0.2.2.7
---------
 Fixed inability to type after choosing smiley through selector in Win98
 Fixed crash with Miranda 0.8 #10
 Fixed loosing smileys after deleting an account

0.2.2.6
---------
 Fixed positioning of controls in option dialog
 Put smiley selector window computation back

0.2.2.5
---------
 Added support for Miranda 0.8 Accounts
 Reduced resource utilization with png smileys
 Fixed few crashes in obscure conditions

0.2.2.4
---------
 Added option to disable custom smileys
 Fixed custom smiley operation with SRMM
 Added Updater plugin Beta site

0.2.2.3
---------
 Fixed freezing after cut in input area
 Fixed smiley selector covered by status bar
 Slight typing performance improvements

0.2.2.2
---------
 Fixed freezing with History++ pseudo edit

0.2.2.1
---------
 Fixed lazy animation in selector window
 Fixed input area freeze with no smiley pack defined
 Fixed no scroll bar with preset selector window size
 Fixed custom smiley parsing when no smiley pack defined

0.2.2.0
---------
 Added MSN custom smiley support
 Fixed Scriver All tabs message send
 Redesigned animation in selection window (for better or worse)

0.2.1.10
---------
 Fixed cut & paste issue
 
0.2.1.9
---------
 Fix for the crash on some computers

0.2.1.8
---------
 Restored History++ compatibility

0.2.1.7
---------
 Fixed operation with input area smileys disabled 

0.2.1.6
---------
 Added correct display of selected animated smileys
 Added ability to cut&paste animated smileys
 Large number of fixes for cut&paste and drawing issues

0.2.1.5
---------
 Fixed input area smileys with surround spaces
 Fixed smiley background color

0.2.1.4
---------
 Fixed input area smiley replacement

0.2.1.3
---------
 Added support for "mutating" smileys in input area
 Added correct handling of cut and copy operations 
 Fixed selection in input area during smiley replacement

0.2.1.2
---------
  Fixed URL detection
  Fixed isolated smiley replacing in input area

0.2.1.1
---------
  Fixed non animated smiley packs
  Fixed sending on "Send" button with Input area smileys
  Fixed TabSRMM saved message

0.2.1.0
---------
  Added support for input area smileys
  Added support for Metacontacts with SRMM
  Fixed empty row/col in selection window
  
0.2.0.8
---------
  Fixes for History++ animation
  Fixes for smiley positioning and scaling

0.2.0.7
---------
  Added History++ animation
  Fixed memory leaks

0.2.0.6
---------
  Changed horizontal smiley scaling
  Fixed TabSRMM animation
  
0.2.0.5
---------
  Fixed smiley jump when scrolling

0.2.0.4
---------
  Fixed tooltips for close smileys
  Fixed crash on exit

0.2.0.3
---------
  Added smiley tooltips

0.2.0.2
---------
  Fixed animated smileys positioning problems

0.2.0.0
---------
  Animated smileys in RichEdit support

0.1.12.17
---------
  Fixed xep parameters display
  Added service to reload smiley packs

0.1.12.16
---------
  Fixed XEP file parsing

0.1.12.15
---------
  Fixed crash in ANSI version with non ASCII regex smileys

0.1.12.14
---------
  Fixed problem when parsing some xep files
  Fixed an error message
  Added new Miranda interface support

0.1.12.13
---------
  Fixed bogus error message in option dialog
  Fixed translation issues
  Fixed services destruction

0.1.12.12
---------
  Fixed few resource leaks
  Minor code improvements

0.1.12.11
---------
  Added ability to select smiley by typing row and column
  Restored Windows 95 compatibility
  Changed xep smiley pack parsing  
  Fixed protocol name translation
  Removed popup support due to non displayed errors in some cases
  Moved reporting broken smileys in a pack to Network Log 
  Fix for BoundsChecker (thanks ghazan)

0.1.12.10
---------
  Fixed smiley duplication
  Added URL and file path exclusion logic for smiley parsing

0.1.12.9
--------
  Fixed full path to relative path conversion logic
  Improved smiley scaling algorithm

0.1.12.8
--------
  Added support for Spell Checker plugin
  Fixed a crash in some rear cases

0.1.12.7
--------
  Fixed regular expressions with non-English alphabets
  Attempt to fix double size smileys with History++

0.1.12.6
--------
  Fixed crash caused by other plugins registering smiley category

0.1.12.5
--------
  Fixed non-Unicode SmileyAdd and DBCS character sets
  Fixed scale to height option with History++
  Disabled "Assign Smiley Category" menu if per protocol smileys disabled 
  Added support for (?imsxr-imsxr) clause in regular expressions

0.1.12.4
--------
  Fixed random crash
  Fixed ability to disable smiley category
  Added support for new xep smiley packs
  Added ability to specify size of smiley selection window in smiley pack
  Added ability to specify tooltip text for regular expression smileys
  Changed to use popup plugin for error messages 

0.1.12.3
--------
  Fixed 100% CPU with custom smiley categories defined
  Fixed contact specific smileys and Specify Smiley pack for each protocol
  Naming fixes
  Changed default button smiley icon

0.1.12.2
--------
  Fixed IEView smileys

0.1.12.1
--------
  Fixed crash on removal of custom smiley categories
  Fixed some backwards compatibilty issues
  Added ability to assign protocol smileys for the contact
  Added message dialog redraw on changing contact smileys

0.1.12.0
--------
  Added per contact smiley packs
  Improvements in error notifications
  Removed deprecated API from header file

0.1.11.15
---------
  Modifier support for regular expressions
  Performance improvements in animated preview window closing

0.1.11.14
---------
  Fixed missing last smiley code (no smiley insert)

0.1.11.13
---------
  Added IcoLib support
  Moved Options into "Customize" group
  More dll size reductions

0.1.11.12
---------
  Fixed crash with malformed smiley packs

0.1.11.11
---------
  New Regular expression engine for smaller size
  Button Smiley behaivior changed 
  (now if Button Smiley explicitly not specified in smiley pack SmileyAdd built-in icon is used (the same as TabSRMM))
  Fixed crash in selector window with no visible smileys
  Fixed option dialog for smiley pack with no visible smileys

0.1.11.10
---------
  Attempt to fix high CPU usage on/after smiley selection on some computers

0.1.11.9
--------
  Attempt to fix desktop redraw issue
  Fixed high cpu usage

0.1.11.8
--------
  Attempt to fix desktop redraw issue
  Fixed random crash

0.1.11.7
--------
  Attempt to fix desktop redraw issue

0.1.11.6
--------
  Animation antilockup logic
  Less invasive window subclassing 

0.1.11.5
--------
  Animation performance improvemnets 
  Animation antilockup logic

0.1.11.4
--------
  Fixed crash with hidden animated smiley
  Fixed smiley corruption in scale to text mode

0.1.11.3
--------
  Fixed crash

0.1.11.2
--------
  Fixed improper delay on animated GIFs
  Fixed png smiley corruption
  Fixed smiley corruption with EMF
  Fixed smileys do not animate after scroll up 

0.1.11.1
--------
  Fixed smileys do not animate sometimes in selector window
  Added mouse wheel scrolling in selector window
  Animation performance improvements
  Fixed smiley no show with TabSRMM

0.1.11.0
--------
  Added ability to show animated smileys in selector window
  Added API to insert EMF into RichEdit (allows "true" transparency)
  Fixed crash on exit with Win98
  Fixed error reporting for not found smileys 

0.1.10.9
--------
  Fixed smiley selector window with large number of smileys
  Do not allow smiley selector window to cover more then 1/4 of the screen area

0.1.10.8
--------
  Fixed grid lines in smiley selector

0.1.10.7
--------
  Fixed issue with smiley replacement when "Replace only isolated smileys" selected 
  Added grid lines to IEView smiley selector 

0.1.10.6
--------
  Fixed smiley replacement at the beginning of the line

0.1.10.5
--------
  More IEView like selector window (in IEView mode) clipping, scroll bar
  Fixed non-Unicode SmileyAdd with DBCS character sets (Chinese, Japanese, Korean, etc.)

0.1.10.4
--------
  Fixed insertion code when regular expressions used in msl/asl smiley pack
  IEView style selection window now selectable option (default SmileyAdd one)
  Fixed preview window update on option change in Option dialog
  Added ability to select gif frame to display as static smiley 

0.1.10.3
--------
  Added Regular Expression support to msl/asl Smiley Packs
  Attempt to correct message log repainting problems

0.1.10.2
--------
  More IEView like selector window
  Attempt to correct message log repainting problems

0.1.10.1
--------
  Fixed Preview window for xep and asl smiley packs
  Attempt to fix "jumping"

0.1.10.0
--------
  Added batch parsing API (to support IEView)
  Fixed checkbox operation in Option Dialog
  Added support for SelectionSize in asl/msl packs

0.1.9.6
--------
  Added ability to paste smiley codes from message log
  (This feature works only if RichEdit v3 or later is used)
  Perfomance improvements in smiley drawing
  Fixed "Surround Smiley with spaces" checkbox in Option dialog

0.1.9.5
--------
  Fixed Smiley Pack information display in Option dialog 

0.1.9.4
--------
  Fixed inadvertent smiley pack change in Option dialog
  Fixed random crash in Message Dialog smiley parsing
  Perfomance improvements in smiley drawing
  Support for Unicode file names in Unicode version

0.1.9.3
--------
  Fixed crash on Disable Message Dialog Support
  Fixed resource leak
  Added GETINFO2 API where deleting Smiley handle is user responsibility
  (GETINFO API should not be used any longer)
  Added protection against multiple SmileyAdd instances execution

0.1.9.2 
--------
  Fixed Smiley Pack info display for Unicode version
  A lot of robustness updates for Unicode version
  Added option to scale all smileys in selector window
  Fixed translation file
  Error messages in Unicode 
  Fixed crash with nonexistent xep smiley pack 

0.1.9.0
--------
  Fixed xep smiley replacement (longer smiley have precedence over shorter ones)
  Fixed button smiley for xep smileys (works now by the same rule as msl one)
  Added unloading GDI+ when it's no longer needed
  Improved smiley parsing performance by at least 20%
  Final resolution for SmileyAdd related flickering
  Updates for Unicode version
  Added support for Unicode (UTF8 and UTF16) msl, asl and xep files
  Added Unicode version for ParseText API

0.1.8.8
--------
  Added workaround for crashing ISee being reported as SmileyAdd crash
  Smiley Preview Window now shows smileys in it's original size (window might get pretty big) 
   
0.1.8.7
--------
  Fixed unicode conversion problem 
  (now Windows language for non-Unicode programs always used) 

0.1.8.6
--------
  Improved performance for smiley parsing
  Fixed flickering durig smiley replacement
  Fixed for rare crashes in non message log window

0.1.8.4
--------
  Improved performance for smiley parsing
  Added workarounds for 32bit_Icons and '&' problems in xep smileys

0.1.8.3
--------
  Improved performance for msl smiley parsing
  Fixed selection restore in message log

0.1.8.2
--------
  Fixed Message Log position restore after smiley insertion
  Fixed cursor flickering durig smiley replacement
  Improved performance on smiley replacement
  Fixed smiley replacement issues for xep smiley pack
  Added workaround for non compliant XML node name

0.1.8.1
--------
  Fixed smiley replacement at the end of the line
  A lot of error checking and notifications for xep Smiley Pack
  Some perfomance improvements

0.1.8.0
--------
  Added support for nConvers (xep) Smiley Packs
  Fixed few potential problems with unicode
  
0.1.7.6
--------
  Added support for environment variables in the path
  Fixed SmileyAdd operation with RSS plugin
  Utilizing Unicode hooks for Unicode SRMMs
  Added line number display for broken smiley
  Added translation capability for few error messages
  Fixed loading Smiley Pack once when used by multiple smiley categories 
  Fixed default Smiley Pack for RegisterCategory API
  Fixed Option Dialog icon for category registered with RegisterCategory API
  Fixed crash when call RegisterCategory API at power-up
  Changed ParseText API to support iterative operation
  Fixed icon retrieval by ParseText API

0.1.7.5
--------
  Fixed smiley detection and replacement issues

0.1.7.4
--------
  Fixed rare smiley detection and replacement issues
  Fixed "Use one for all" checkbox handling in Option Dialog  

0.1.7.3
--------
  Fixed rare smiley detection and replacement issues 

0.1.7.2
--------
  Fixed smiley replacement with no button bar
  Fixed images in Option Dialog for systems with Commmon Controls 5 or earlier 
  Added color selection in Option Dialog for smiley selector window 
  Spelling fixes in Register Category API

0.1.7.1
--------
  Fixed crash with massage dialog with no contact
  Fixed crash with PopUp++
  Attempt to fix Win9x icon size problem in Option Dialog
  Much reduced GDI resource utilization
  Added API to register a Smiley Category
  Many changes to API to Find Smiley in Text

0.1.7.0
--------
  Added support for non icon smileys (png, gif, etc.) 
     GDI+ required for this feature to work
  Added support for IEView .asl Smiley Packs
  Added error notification on missing smileys from the pack
  Better handling for relative paths in Smiley Pack
  Added API to find smileys in a text string (pescuma request for use in modern contact list)

0.1.6.2
--------
  Fixed URL corruption regression
  
0.1.6.1
--------
  Fixed Option Dialog for systems with many protocols
  Surround smiley with spaces option now make sure that any smiley   
      detected have spaces on both sides

  At this point I am not aware of any problems with SmileyAdd. 
  If you encounter some you want to report it together with detailed steps to reproduce it 
  and software versions (Miranda, plugins, OS, etc.) you are using.

0.1.6.0
--------
  API change, hookable event provided to notify plugins with "native"
      SmileyAdd support that options have changed, so they could redraw
  Fixed smiley selection window display location on multi-monitor systems
  Fixed protocol icon display in Option Dialog for protocols which don't 
      provide "small" icons (i.e MetaContacts)
  Increased size of controls in Option Dialog to aid translation
  Fixed string in translation file 
  Fixed changing checkbox "Specify smiley pack ..." would not enable Apply button
  Fixed translation of the protocol name could result in smiley not showing
     Caution: This fix could result in having to reselect smiley pack for protocol!

  At this point I am not aware of any problems with SmileyAdd. 
  If you encounter some you want to report it together with detailed steps to reproduce it 
  and software versions (Miranda, plugins, OS, etc.) you are using.

0.1.5.3
--------
  Option Dialog redesigned. It's hopefully more understandable now. 
  (Thanks to HikoH for lot's of suggestions)
  Unfortunatly all translations will have to change with this release.
  Changing Options in option dialog no longer require restart for them to take effect.
  
  At this point I am not aware of any problems with SmileyAdd. 
  If you encounter some you want to report it together with detailed steps to reproduce it 
  and software versions (Miranda, plugins, OS, etc.) you are using.

0.1.5.2
--------
  Fixed smiley replace in history 
  Fixed smiley selection tool sometimes open in strange location
  Fixed Option dialog filename problem when you browse for it

0.1.5.1
--------
  Fixed overwrite of "Add Contact to buddy list" button (for not listed contacts) 
  Fixed "Use One for all" radio buttons problem in Option Dialog

0.1.5.0
--------
  API change to support TabSRMM 0.9.9.96 and later (per Nightwish request)
     (the parent window for smiley selection dialog could be specified now)
  Fixed smiley scaling issue with some smiley packs on Miranda Button.

0.1.4.14
--------
  Fixed File Open Dialog for empty or broken file names

0.1.4.13
--------
  Better handling of broken msl files (Button S on message dialog instead of icon)
  Attempt to fix no File Open dialog on Win9x.
  Error messages are now using balloon notification, when it works
  Force focus to entry dialog after selecting smiley
  Code cleanup

0.1.4.12
--------
  Implemented much more robust algorithm for detecting 
     SRMM 2.x interface supporting plugins versa 
     SRMM 1.x interface supporting plugins

  This release have been tested to work with: 
	SRMM      2.4.0.0  and 1.0.4.2 (2.x and 1.x)
	TabSRMM   0.9.9.95 and 0.0.8.0 (built-in support and 1.x)
	SRAMM     0.3.2.2  (1.x)
	SRMM Mod  0.5.8.0  (1.x)
	Scriver   2.2.2.2  (built-in support)
  Single Mode SRMM 0.1 have built-in text-only SmileyAdd support and it's working as such
  
  This will fix "smiley button does not show" problems.

0.1.4.11
--------
  Fixed latest SRMM support regression
  Improved relative path support
  Smiley packs disabled in the Option dialog no longer loaded
  
0.1.4.9
-------
  Fixed handling of SRMM interface (interface changed between Miranda 3.3.1 and 4.0)
  Bottom smiley button option is not available any more for Miranda later then 3.3.1 
  Smiley packs which specified but don't exist now give error messages (at power up too)
  Non IM protocols (i.e. weather, time, etc.) are ignored now
  Fixed Option dialog translation
  Fixed Option dialog handling of missing Smiley Packs
  Fixed Option dialog Cancel/Apply functions
  Option dialog accepts relative paths now (if you type them in)
  Changing settings in Option dialog will affect all open windows on Apply
  Fixed numerous resource leaks
  A lot of general code robustness and performance updates
  Removed Windows API calls that don't exist on Win95
  MS VC++ 6 portability updates (dsp file provided)
  Fixed older TabSRMM support

  Extensive testing have been performed on plugin for this release.
  Plugin have been tested to work with Windows XP, 2000, 95, Miranda 4.0.1 and 3.3.1
     SRMM and TabSRMM

0.1.4.8
-------
  Fixed transparency with TabSRMM
  Fixed losing focus after entering smiley in SRMM
  Fixed memory leak in smiley dialog
  GCC portability updates 

0.1.4.7
-------
  Fixed crashes and option dialog.

0.1.4.6
-------
  More crash fixes.

0.1.4.5a
-------
  Fixed back merge

0.1.4.5
-------
  OLE resource leak fixed (by NightWish)
  Includes TabSRMM version (by NightWish)

0.1.4.4a
-------
  Compiled in .NET studio as before.

0.1.4.4
-------
  Multiple crash fixes with invalid files (by bidyut)

0.1.4.3
-------
  Fixed incompatibility with Jabber plugin
  Plugin support for showing a smiley selection window
  Internal changes, better memory/resource management
  Order of the smileys in the smiley pack doesn't matter anymore

0.1.3.2
-------
  Minor fixes:
    - Smiley button scales the image now correctly
    - Fixed problem with spaces and " in smiley text when used with smiley select window
    
0.1.3.1
-------
  Minor fixes:
    - Smiley selection window placement is better
    - Button is better shown when quote button is disabled
    
0.1.3.0
-------
- Works now with Miranda version 0.3.3
- Fixed 4 character smiley bug
- Smileybutton can now be on top row buttons (default)
- More than 2 consecutive smileys without spaces are now 
  recognized in 'isolated smiley' mode
- Support for spaces or " in smileys (see sample .msl file)

0.1.2.3
-------
- Fixed incompatibility (2) with message dialog in nightly build of 30-06-2003 

0.1.2.2
-------
- Fixed incompatibility with message dialog in nightly build of 30-06-2003

0.1.2.1
-------
- (Small) bug with smiley pack loading fixed

0.1.2.0
-------
- Ability for hidden smileys (not shown in select window)
- Ability for separate tooltip text in select window
- Fixed display bug in select window with large smiley packs
- Fixed bug with random smileys not being replaced (at least i hope so)
- Fixed bug with smiley replacement if some text was selected
- Fixed bug when using large smiley texts
- Fixed bug with (non) case sensitivity (now case sensitive)
- Fixed some buffer overflow bugs
- Other minor bug fixes


0.1.1.0
-------
- Added Support for smiley sizes other than 16x16
- Added SmileySelectWindow button
- Fixed (disabled) display of smileys in hyperlinks 
- Added 'scale to text height' option
- Fixed GDI leakage bugs 
- Other minor bugfixes

0.1.0.0
-------
Official 1.0 release
- Support for customizable icon packs
- Standard shipment with full OG smiley pack of MatriX
- Better smiley parsing (support for 'isolated' smileys)
- Specify smileys on a 'per protocol' basis
- Smiley replacementsupport for plugins

0.0.2.0
-------
Second alpha release with OG smileys of MatriX

0.0.1.0
-------
Initial alpha (test) release with beautyfully :) styled greenish icons.


Copyright and license
=====================

Software:

Copyright (C) 2005 - 2011 Boris Krasnovskiy All Rights Reserved

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

Emoticon Images:

Copyright (C) 2008 Angeli-Ka All Rights Reserved

Emoticon images are published under
Creative Commons Attribution-Noncommercial-No Derivative Works 3.0 license 

Under this license:

You are free:
to Share — to copy, distribute and transmit the work 

Under the following conditions:
Attribution. You must attribute the work in the manner specified by the author or licensor (but not in any way that suggests that they endorse you or your use of the work). 
Noncommercial. You may not use this work for commercial purposes. 
No Derivative Works. You may not alter, transform

The complete text of the license, translation and legal text can be found here:
http://creativecommons.org/licenses/by-nc-nd/3.0/

