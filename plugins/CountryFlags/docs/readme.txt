
Country Flags 0.1.0.3
------------------------------------------------------------------------
                                      Plugin for Miranda IM 0.7 and +

	Plugin Info: ----------------------------
	| Version:     0.1.0.3
	| Filename:    flags.dll
	| Author:      H. Herkenrath (hrathh at users.sourceforge.net)
	| Description: Service offering misc country utilities as
	|              flag icons and a IP-to-Country database.

	Contents: -------------------------------
	| Features, Requirements, Usage, Installation,
	| Bugs and Wishes, Version History, License and Copyright

Features
----------------------
	+ Provides country utility services for other plugins (Flag Icons, IP-To-Country)
	+ Shows country flags as extra images on contact list
	+ Shows country flags as status icons on message windows
	+ Flag icon can be hidden if no country can be determined
	+ Flag icon is automatically refreshed when user details are updated
	+ Unicode compatible	
	+ Automatic installation of all files, just unzip into Plugins directory

Requirements
----------------------
	-> Miranda IM 0.7+:
	Miranda IM is needed in version 0.7 or greater.

Usage
----------------------
	Country Flags Provider does not add much functionality by itself.
	It offers services for other plugins to use.

	To show country flags on your contact list or
	on your message window open the options and go here:
	'Contact List' -> 'Country Flags'
	
	The country flag icons can be viewed here:
	'Customize' -> 'Icons'

Installation
----------------------
	Find 'miranda32.exe' on your computer.

	Just copy all the contents of the zip-file as they are into the 'Plugins' directory
	in the Miranda IM folder.
	'Country Flags' will detect the files and move them into the appropriate
	directories on it's first run.
	
	You can also do all the installation by hand, if you want to:

	Main Plugin: Copy the file 'flags.dll' into the 'Plugins' directory
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
	Feel free to mail me your wishes about 'Language Pack Manager' and tell
	me all the bugs you may find.
	My email address is: hrathh at users.sourceforge.net

Version History
----------------------
	0.1.0.3 - really fixed those nasty crashes
		- lightning-like speed due to buffered IP-database access
	        - option to turn-off usage of ip-to-country database 
	0.1.0.2 - another try to fix those crashes
	        - refreshs icons on user details update
	        - optionally shows nothing if no country can be determined for a contact
	0.1.0.1 - try to fix those extraimg crashes
	        - fixed: implemented extra image column selector properly
	        - fixed: heavy crash on Unicode startup
	          (search on garbage might cross page-boundary)
	        - rewritten flags as status icons
	        - reimplemented country lookup
	        - added map-file to SDK package
	0.1.0.0 - Initial release

License and Copyright
----------------------
	'Country Flags' is released under the terms of the
	GNU General Public License.
	See 'Flags-License.txt' for more details.

	Copyright (c) 2006-2007 by H. Herkenrath. All rights reserved.

	This service includes the IP-to-Country Database
	provided by WebHosting.info (http://www.webhosting.info),
	available from http://ip-to-country.webhosting.info.
	Copyright (c) 2003 Direct Information Pvt. Ltd. All rights Reserved.
	Included Version: 05-03-2007, will be updated regularly.

	The included Huffman encodig algorithm is
	Copyright (c) 2003-2006 Marcus Geelnard.

	The included default flag icons are public domain.
	Flag Icons - http://www.famfamfam.com (created by Mark James)

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
