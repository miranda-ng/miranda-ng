
Language Pack Manager 1.0.2.2
------------------------------------------------------------------------
                                      Plugin for Miranda IM 0.6 and +

	Plugin Info: ----------------------------
	| Version:      1.0.2.2
	| Filename:     langman.dll
	| Author:       H. Herkenrath (hrathh at users.sourceforge.net)
	| Description:  Helps you manage Language Packs of different languages.

	Contents: -------------------------------
	| Features, Requirements, Usage, Installation,
	| Bugs and Wishes, Version History, 
	| Translation, License and Copyright

Features
----------------------
	+ Allows to switch installed Language Packs easily
	+ Displays all information about the Language Packs in one place
	+ Shows which Language Pack includes support for what plugins
	+ Right-click on a language pack entry to be able to uninstall it
	+ Auto-Updates for all installed Language Packs
	+ Wizard helping you to download and install your native language
	+ Unicode support
	+ Automatic installation of all files, just unzip into Plugins directory
	+ Supported plugins: Database Editor, Country Flags

Requirements
----------------------
	-> Miranda IM 0.6+:
	Miranda IM is needed in version 0.6 or greater.

Usage
----------------------
	The Language Packs can be managed here:
	'Options' -> 'Customize' -> 'Language'

	When you do a right-click on a specific language pack, you are able to
	uninstall that file if it is no longer needed. 

Installation
----------------------
	Find 'miranda32.exe' on your computer.

	Just copy all the contents of the zip-file as they are into the 'Plugins'
	subdirectory in the Miranda IM folder.
	'Language Pack Manager' will detect the files and move them into the appropriate
	directories on it's first run.
	
	You can also do all the installation by hand, if you want to:

	Main Plugin: Copy the file 'langman.dll' into the 'Plugins' subdirectory
	in the Miranda IM folder. 

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
	1.0.2.2 - try to fix radio buttons on Win9x
	        - try to fix anything else
                - fixed langpack download
	        - fixed selected flag icon
	1.0.2.1 - added flag icons using flags.dll
	        - only search for updates if a langpack is present
	        - better displays incompatible locales
	        - added v0.8 support
	        - minor improvements
	1.0.2.0 - fixed langpack removal in context menu
	        - fixed automatic installaton
	        - updated to use new miranda headers
	        - auto-cleanup of langpack info
	        - support for WinXP style radio buttons in list
	        - moved options to 'Customize' section
	        - shows langpack filenames in expert mode
	        - auto-updates for language packs
	        - download wizard to install your native language
	        - other improvements
	1.0.1.1 - minor fixes and adjustments
	1.0.1.0 - minor fixes
	        - right-click on a language pack entry to uninstall a file
	        - enlarged not-included list a bit
	        - del key is shortcut for context menu item 'remove'
	1.0.0.0 - Initial release

Translation
----------------------
	Translation strings for language pack maintainers can be found
	in 'LangMan-Translation.txt' of the SDK package.

License and Copyright
----------------------
	'Language Pack Manager' is released under the terms of the
	GNU General Public License.
	See 'LangMan-License.txt' for more details.

	Copyright (C) 2005-2007 by H. Herkenrath. All rights reserved.
	
	The included unzip code for ZIP-files is
	Copyright (C) 1995-1998 Jean-Loup Gailly and Mark Adler.
	It is based on a modification by
	Copyright (C) 1998 Gilles Vollant, Lucian Wischik.

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
