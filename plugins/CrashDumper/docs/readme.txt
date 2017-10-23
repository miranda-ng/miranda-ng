Crash Dumper plugin for Miranda NG
=====================================================================
Plugin creates usable Crash Report and Version Information for Miranda NG

When Miranda crashes plugin ptrovides: 
- text representation of crash report 
- MiniDump  representation of crash report

On demand plugin provides Version Information report
On demand uploads VersionInfo report to www.miranda-vi.org 

VersionInfo report includes:

- OS Version information
- Internet Explorer version information
- Miranda version information with build date
- Miranda profile used
- Loaded Miranda plugin list with versions
- Loaded language pack version
- Loaded weather ini files
- Created protocols accounts 

Files stored in {Miranda Profile}\CrashLog directory

Text version of crash report includes:

- Guess for plugin responsible for the crash (it printed in the report)
- Stack trace for exception (with function names, source file names 
  and line numbers, if possible)
- Version Information (see above) 
- Loaded modules (dlls) list
- Plugin lists all information you need to submit crash report for Miranda
- Loaded weather ini files

Plugin works with all current Operating Systems.

It could be downloaded here:
Unicode: http://nightly.miranda.im/x32/crashdumper.zip
x64:     http://nightly.miranda.im/x64/crashdumper.zip
All:     http://nightly.miranda.im/

Author: Boris Krasnovskiy (borkra)

Licensing terms could be found below.

Change Log
==========

0.0.4.20
--------
Added Windows 8 detection

0.0.4.19
--------
Improved reporting for MS RTL failures
Fixes for translation

0.0.4.18
--------
Renamed Hotkeys, added Hotkey to open message window
Added support for 32x32 icons
Added proper display of Miranda 0.9 profile path

0.0.4.17
--------
Removed option to ignore crashes that do not cause Miranda to close
Improved protocol reporting
Fixed TopToolbar buttons no show
Added proper handling of Crash Dumper late exit in Miranda 0.9

0.0.4.16
--------
Added icon pack reporting
Fixed toolbar button for storing to file

0.0.4.14
--------
Fixed options dialog
Changed bug reporting message to popup 

0.0.4.13
--------
Changed option to notify only about crashes that stop Miranda
Fixed bug reporting menu

0.0.4.12
--------
Improved accounts reporting
Fixed error messages about inability to store crash report
Added option to disable crash reporting function
Added coping VI into clipboard on Report Bug menu item

0.0.4.11
--------
Allow creating crash report with dbghelp.dll v5.0 
(this adds support for creating crash reports on Windows 95 with extra dbghelp.dll 
and on Windows 2000 with built-in dbghelp.dll)

0.0.4.10
--------
Fixed bbcodes for Weather ini
x64 portability
Windows 95 compatibility
Fixed crash in service mode

0.0.4.9
--------
Printing weather only if weather plugin present
Changed alternative date format
Fixed langpack display when non ANSI chars used in the description
Fixed crash in debug version
Fixed langpack display when locale id is bogus
Changed stored file format to UTF-8
Added opening generated version info file on popup click

0.0.4.8
--------
Changed upload to miranda-vi.org to unicode
Added Windows 7 & Windows Server 2008 R2 detection 
Added Weather ini files reporting
Changed Accounts reporting to list enabled and disabled accounts
Changed VI display font to Courier New

0.0.4.7
--------
Made message boxes the topmost window
Added Protocols and Accounts printout 

0.0.4.6
--------
Attempt to reduce no-report conditions

0.0.4.5
--------
Added icons to menu items
Added storing crash report in clipboard only by request
Added base address display to VI with DLLs
Fixed buttons in VI view when show VI with DLL

0.0.4.4
--------
Added menu item to open crash log folder in explorer
Added menu item to open miranda-vi.org in web browser
Added automatic login into miranda-vi when open in web browser through Miranda
Added option to display date and time in "traditional" format for all reports

0.0.4.3
--------
Removed Trap all exceptions option

0.0.4.2
--------
Added caution note to Trap All Exceptions option 
Restored compatibility of Crash Dumper Unicode with dbghelp.dll distributed with XP
Disabled on startup Trap All Exceptions option on pre Vista OSes

0.0.4.1
--------
Fixed crash in Trap all possible exceptions mode

0.0.4.0
--------
Fixed crash on VI upload
Modified unicode handling
Fixed memory leak
Added option to trap all possible exceptions
Added ability to provide crash reports for crashes in Miranda threads in 0.8 #19 and higher
Added Unicode version distribution

0.0.3.4
--------
Added storing version Information to file to top toolbar
Added storing crash report to clipboard
Attempt to resolve crashes with no report

0.0.3.3
--------
Fix for crash while generating crash report

0.0.3.2
--------
Added VersionInfo buttons to clist_modern toolbar
Fixed crash when dbghelp not installed or wrong version

0.0.3.0
--------
Added option to printout all loaded DLLs as VI
Added printout of dll version and date to dll list

0.0.2.8
--------
Fixed crash with Wine
Added MessageBox with location of Crash Report after every crash

0.0.2.7
--------
Fixed crash with debughlp.dll not available
Fixed unloadable dll diagnostics
Fixed Show VersionInfo under Win9x

0.0.2.6
--------
Fixed translation

0.0.2.5
--------
Improved diagnostics for unloadable dlls
Fixed unloading dbghelp when not needed
Fixed spelling

0.0.2.4
--------
Improved chances of getting crash report for some crashes

0.0.2.3
--------
Fixed unloadable plugin display
Improvements in notifications

0.0.2.2
--------
Added unloadable plugin display in VI
Fixed VI Show under Windows 9x

0.0.2.1
--------
Clarification in options text
Open global compare url on popup click after successful upload
Added outdated plugin indication after successful upload
Moved all db settings onto one group

0.0.2.0
--------
Fixed icolib support
Added new icons by Angeli-Ka
Added printout for profile creation date and size
Added VersionInfo uploading to www.miranda-vi.org
Removed interface to VIUploader plugin

0.0.1.4
--------
Fixed BBCode for uploaded VI

0.0.1.3
--------
Fixed BBCode formatting
Added BBCode to uploaded VI
Added country code to language pack info
Added possibility to add icons for menu items through icolib

0.0.1.2
--------
Fixed Folders support

0.0.1.1
--------
More robustness updates

0.0.1.0
--------
Fixed crashes during VersionInfo reporting
More lang pack info
Moved plugin causing crash reporting earlier

0.0.0.13
--------
More fixes to plugin sorting

0.0.0.12
--------
Made plugin sorting case insensitive

0.0.0.11
--------
Added WOW64 printout
Fixed VIUploader interface
Added explicit plugin sorting

0.0.0.10
--------
Changed printed timestamp
Merged profilename and path
Capturing only unhandled crashes

0.0.0.9
-------
Added protection against infinite exception loops
Fixed printing VersionInfo to file
Added context menu for VersionInfo window

0.0.0.8
-------
Fixed operation in service mode
Fixed inability to create crash report
Added ability to provide reports when multiple crash report plugins installed (only on XP or later)
Added notification when multiple crash reporting plugins installed
(as this causes conflict among plugins and inability to create crash reports)

0.0.0.7
-------
Changed all timestamps to ISO 8061:2004 format 
Added keeping of window sizing & position 
Changed preview window color
VersionInfo stored in profile directory by default
Added more formatting to VersionInfo
Added service mode support (dll name changed) 

0.0.0.6
-------
Fixed storing VersionInfo to file
Added ui to show VersionInfo
Added service to retrieve VersionInfo

0.0.0.5
-------
Fixed crash reports with older dbghelp.dll
Added Folders plugin support
Added more VersionInfo reporting
Added VersionInfo storage (to file and clipboard)
Improved report formatting
Added ability to get VersionInfo by hotkey

0.0.0.4
-------
Added human readable exception code
Added complete description for access violation
Updated sources to compile with older PSDK
More VersionInfo related stuff

0.0.0.3
-------
Added reporting for Internet Explorer version
Added reporting for plugin Unicode capability
Added reporting for Miranda profile path and name
Added reporting for miranda.exe build date

0.0.0.2
-------
Reduced resource utilization
Allow Unicode compilation

0.0.0.1
-------
Initial release of the plugin

Copyright and license
=====================

Software:

Copyright (C) 2008 - 2012 Boris Krasnovskiy All Rights Reserved

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
