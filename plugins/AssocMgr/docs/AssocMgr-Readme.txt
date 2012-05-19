
File Association Manager 0.1.1.0
------------------------------------------------------------------------
                                      Plugin for Miranda IM 0.6 and +

	Plugin Info: ----------------------------
	| Version:     0.1.1.0
	| Filename:    assocmgr.dll
	| Author:      H. Herkenrath (hrathh at users.sourceforge.net)
	| Description: Handles file types and URLs like aim,
	|              ymsgr, xmpp, wpmsg, gg, tlen.

	Contents: -------------------------------
	| Features, Requirements, Usage, Installation, Browsers,
	| Bugs and Wishes, To-Do List, Version History,
	| Thanks, Translation, License and Copyright

Features
----------------------
	+ Allows file types and URLs to be opened using Miranda IM and plugins
	+ If Miranda IM is is not running when an associated file or URL is opened
	  it gets invoked automatically
	+ You can add contacts easily from websites and forums
	+ Adds an option to start Miranda IM automatically with your Computer
	+ Offers the possibility to associate the file types only while running
	+ Provides register services for other plugins (very easy to use)
	+ Supports an unlimited amount of URLs or file types to be registered
	+ Full Unicode support
	+ All registry keys are cleanly generated and can completely be removed
	+ Keeps a backup of overwritten file extensions and restores it
	+ Backup gets restored only if it is still valid
	+ Auto-cleanup for old database settings of uninstalled plugins
	+ Automatic installation of all files, just unzip into Plugins directory
	+ Supported plugins: Database Editor++

Requirements
----------------------
	-> Miranda IM 0.6+:
	Miranda IM is needed in version 0.6 or later.

Usage
----------------------
	The registered associations of 'File Association Manager' can be
	configured here: 'Options' -> 'Services' -> 'Associations'
	You need to have other plugins installed that actually
	offer URL protocols or file type associations.
	This plugin only offers the registration service, it does not
	add any file associations on its own. 

Installation
----------------------
	Find 'miranda32.exe' on your computer.

	Just copy all the contents of the zip-file as they are into the 'Plugins' directory
	in the Miranda IM folder.
	'File Association Manager' will detect the files and move them into the appropriate
	directories on it's first run.
	
	You can also do all the installation by hand, if you want to:

	Main Plugin: Copy the file 'assocmgr.dll' into the 'Plugins' subdirectory
	in the Miranda IM folder. 

	The Unicode version of the plugin will only work on Windows NT/2000/XP,
	Windows Server 2003, Windows Vista or later with an installed
	Unicode version of Miranda IM.
	To use it on Windows 95/98/Me, please download the ANSI version of the plugin.
	  
	Documentation: The txt-files should be moved along with the SDK-zip into the
	'Docs' directory in the Miranda IM folder. 

	That's it!

Bugs and Wishes
----------------------
	Feel free to mail me your wishes about 'File Association Manager' and tell
	me all the bugs you may find.
	My email address is: hrathh at users.sourceforge.net

To-Do List (random ideas)
----------------------
	- draw a better mirandafile icon...anyone?
	- outsource UrlDecode() function into MS_NETLIB_URLDECODE (MS_NETLIB_URLENCODE already exits)
	  add nice wrapper Netlib_UrlDecode() as this needs to be called in each urlparser service (substrings)
	- test using different browsers
	- remove demo purpose code (after some time)
	- dreaming of an installer.dll plugin that registers on
	  mir-files to unzip them...anyone?

Version History
----------------------
	0.1.1.0 - keeps a backup of overwritten file extensions and restores it
	        - backup only is restored if it is still valid
	        - auto-cleanup for old database settings of uninstalled plugins
	        - workaround for WinXP bug causing an scrolling glitch
	          on ListView controls with groups
	        - added FTDF_ISSHORTCUT flag for file types
	        - list gets sorted using langpack locale
			- fixes for Win9x/NT4
	        - minor improvements
	0.1.0.3 - really fixed crash on options
	        - minor fixes
	0.1.0.2 - fixed crash on options
	0.1.0.1 - added v0.8 support
	        - minor improvements
	0.1.0.0 - Initial release

Thanks
----------------------
	* To Jacek_FH who wrote the ProtoLink plugin which first
	  summed up all link handling, but was not extensible.
	* To egoDust who wrote the unfinished icqwm plugin to
	  handle icq-files
	* To rainwater for the aim links part of the AIM TOC protocol
	* To Sergey Gershovich (Jazzy) who wrote the Autorun plugin

Translation
----------------------
	Translation strings for language pack maintainers can be found
	in 'AssocMgr-Translation.txt' of the SDK package.

License and Copyright
----------------------
	'File Association Manager'" is released under the terms of the
	GNU General Public License.
	See "AssocMgr-License.txt" for more details.

	'File Association Manager' is copyright 2005-2007 by H. Herkenrath.

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

	THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS
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
