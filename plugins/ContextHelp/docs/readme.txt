
Help Plugin 0.2.1.2
------------------------------------------------------------------------
                                      Plugin for Miranda IM 0.6 and +

	Plugin Info: ----------------------------
	| Version:      0.2.1.2
	| Filename:     help.dll
	| Authors:      R. Hughes, H. Herkenrath (hrathh at users.sourceforge.net)
	| Description:  Provides context sensitive help in all of the Miranda IM dialog boxes.

	Contents: -------------------------------
	| Features, Requirements, Usage, Installation,
	| Bugs and Wishes, Version History, 
	| Translation, License and Copyright

Features
----------------------
	+ Display help tooltips on request for every component in a window
	+ Tooltips always occur below the element to show help for
	+ All help texts are stored in a Help Pack file ('helppack_<language>.txt')
	+ Allows to switch installed Help Packs easily
	+ Displays all information about the Help Packs in one place
	+ Shows which Help Pack includes support for what plugins
	+ Auto-Updates for all installed Help Packs
	+ Wizard helping you to download and install your native language
	+ Right-click on a help pack entry to be able to uninstall it
	+ Help API file for developers to control the help in their dialogs
	+ Full Unicode support
	+ Automatic installation of all files, just unzip into Plugins directory
	+ Supported plugins: Database Editor, Country Flags

Requirements
----------------------
	-> Miranda IM 0.6+:
	Miranda IM is needed in version 0.6 or greater.

Usage
----------------------
	Provides support for context-sensitive help in all of Miranda's dialog boxes. 
	Right click, press F1 or use the ? button in the title bar.

	When a dialog supports context sensitive help, you can right click
	on every control element to get more information about it.
	Dialogs that don't have a minimize or maximize box in their caption do also show
	an additional button with a '?'-symbol in their window caption.
	The F1 key is also recognized as a context help call in dialogs.

	The used Help Pack file can be selected here:
	'Options' -> 'Customize' -> 'Help'

Installation
----------------------
	Find 'miranda32.exe' on your computer.

	Just copy all the contents of the zip-file as they are into the 'Plugins' directory
	in the Miranda IM folder.
	The Help Plugin will detect the files and move them into the appropriate
	directories on it's first run.
	
	You can also do all the installation by hand, if you want to:

	Main Plugin: Copy the file 'help.dll' into the 'Plugins' directory
	in the Miranda IM folder. 
	The file 'helppack_english.txt' needs to copied into the main directory
	of Miranda IM.

	The Unicode version of the plugin will only work on Windows NT/2000/XP,
	Windows Server 2000, Windows Vista or later with an installed
	Unicode version of Miranda IM.
	To use it on Windows 95/98/Me, please download the ANSI version of the plugin.
	  
	Documentation: The txt-files should be moved along with the SDK-zip into the
	'Docs' directory in the Miranda IM folder.

	That's it!

Bugs and Wishes
----------------------
	Feel free to mail me your wishes about 'Language Pack Manager' and tell
	me all the bugs you may find.
	My email address is: hrathh at users.sourceforge.net

Version History
----------------------
	0.2.1.2 - CTRL-key: copies a proposed ctl title for ctl identifier
	        - corrected user-agent 
	        - minor fixes
	0.2.1.1 - corrected exported version number
	        - temporarily deactivated the download wizard on first run
	          (as there is nothing yet to be downloaded)
	        - removed 'cache expiry' setting from options, auto-detects file change
	0.2.1.0 - added auto-updates
	        - added native language download wizard
	        - updated to use new miranda headers
	        - added automatic installation for helppack files and docs
	          (move them to correct directory)
	        - obeys win context ids when set
	        - synced with last official help.dll
	        - added v0.8 support
	0.2.0.6	- minor fix for ownerdefined window classes
	        - hyperlink cursor was incorrectly destroyed
	        - corrected <a></a> tag parsing when improperly formated
	        - hyperlinks now use correct system color (was problem with richedit)
	0.2.0.5 - fixed focus stealing due to WM_ACTIVATE recursion	
	        - adjustments for helptips about editable controls (autotip)
	        - fix for WM_HELP causing an additional WM_LBUTTONUP (fixes help about buttons)
	        - fixed issue with SHBrowseForFiles and other 'owned' windows
	        - corrected richedit offset (approx. 2px)
	        - shows endellipsis if title text is too long
	        - adjusted spinner control handling
	        - small fix for caching
	        - disallow empty title/text
	        - fixed <p></p> tag parsing
	        - removed unnecessary services of 0.2.0.0
	        - minor cleanups 
	0.2.0.4	- minor fix for unhooking
	        - pressing ctrl-c on tip copies helptip text to clipboard
	        - minor locale related fix
	0.2.0.1	- Added: optional shows autotips (see option page, experimental, feature might be dropped eventually)
	        - Fixed: discovered minor problem with threading conncurrency
	        - Modified: now also shows '?' box for minizable dialogs (especially options dialog)
	        - Minor fixes
	0.2.0.0	- New generation release
	        - Now the plugin uses a text file (helppack_*.txt) to store its data
	          The server method was defunct and nobody ever updated/stored
	          help texts. You even needed a password.
	          Now the functionality is similar to the langpacks.
	          The syntax of the helppack files is kept *very* similar to the
	          langpack ones
	        - Updated: control type detection to recgnize newer controls
	        - Replaced: netlib/networking code with file reading code
	        - Added: Help API (m_help.h) for developers
	        - Added: Full unicode support
	        - Fixed: Help support for MessageBoxes (was buggy)
	        - Added: press 'Ctrl' while showing the help tooltip to get
	          the identifier used in the helppack file
	        - Fixed: DlgId handling was not well though out
	        - Fixed: some memory leaks in low memory conditions in conjunction with realloc
	        - Fixed: some rare occuring errors were not handled
	        - Fixed: help on help tooltip is disabled (was buggy)
	        - Fixed: some unicode problems
	        - Fixed: thread safety (MS_SYSTEM_THREAD_PUSH/POP was not used)
	        - Fixed: now also supports generating IDs for IDC_STATIC(-1) controls
	        - Fixed: created some services to avoid conflicting context menus
	        - Fixed: codepage usage
	        - Replaced: uses miranda's simple memory problems catcher
	        - Verified all code, used SAME PROGRAMMING STYLE AS ORIGINAL
	        - Fixed: some minor issues with unchecked return values
	        - Changed: Moved the Cache Expiry setting to the options (was hardcoded) 
	        - Some other minor fixes and updated code
	0.1.2.2	- previous help plugin using http requests

Translation
----------------------
	Translation strings and available bad words strings can be found
	in 'Help-Translation.txt' of the SDK package.

License and Copyright
----------------------
	Help Plugin is released under the terms of the
	GNU General Public License.
	See 'Help-License.txt' for more details.

	Copyright (c) 2002 Richard Hughes, 2005-2006 Heiko Herkenrath.
	All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions
	are met:

	1. Redistributions of source code must retain the copyright
	   notice, this list of conditions and the following disclaimer.
	2. The origin of this software must not be misrepresented; you must 
	   not claim that you wrote the original software.  If you use this 
	   software in a product, an acknowledgment in the product 
	   documentation would be appreciated but is not required.
	3. Altered source versions must be plainly marked as such, and must
	   not be misrepresented as being the original software.
	4. The name of the author may not be used to endorse or promote 
	    products derived from this software without specific prior written 
	    permission.

	THIS SOFTWARE IS PROVIDED BY THE AUTHOR 'AS IS' AND ANY EXPRESS
	OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
	ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
	DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
	DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
	GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
	WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
	NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

H. Herkenrath (hrathh at users.sourceforge.net)
