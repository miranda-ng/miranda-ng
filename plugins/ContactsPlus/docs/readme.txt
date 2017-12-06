About
-----

Send and receive contacts ui
Version 1.5.2.0
by Joe Kucera aka Jokusoftware
Original Code by Todor Totev aka Dominus Procellarum
for use with Miranda IM 0.3.3+
Best for Miranda IM 0.4+


Description
-----------

This plugin allows you to send and receive contacts.
It is protocol independent, although currently the only
protocol supporting contacts transfer is ICQ. 
 

Installation
------------

For the contactsp.zip zip archive - 
  Extract contacts.dll to Miranda plugin folder  - 
  usually this is C:\Program Files\Miranda\Plugins.
  All other files are not mandatory and can go to Miranda folder. 


Source
------

The source code for this plugin can be downloaded from Miranda IM
website http://addons.miranda-im.org.


License
-------

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

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
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

For more information, e-mail jokusoftware@miranda-im.org


Version history
---------------
1.0.11.0 
	initial release - working ui, but M is still crashing 
1.0.12.0
  20-may night build of M - underlying code is OK, so i can continue this plugin
  remove inverse logic in MirandaPluginInfo()
  removed #include "win2k.h" for cleaner build
  attempt to show more user info on receive contacts
1.0.13.0
  cleaned main.cpp
  added icon to dialogue windows
1.0.14.0
  Nick / UIN swapped upon showing received contacts /silly me/
  bigger size of nick string allowed to accomodate non-english nicks /cyrrilic letters requires 2 times more bytes/
  added icon for menu item
1.0.15.0
  list columns default size is more convinient
  save restore window position / size
1.0.16.0
  receive dlg is resizable one /thanks to WTL there was virtually no code to write/
1.0.17.0
  add button is enabled only when at least one item in the list view is checked-in
1.0.18.0
  more translate()-friendly
  at last contacts are permanently added to DB
1.0.19.0
  receive window is now being put on top
  lots of big-fixes 
1.1.0.0
  source beautify
  receive user iface and internal dependancies re-design
  now only contacts which details are requested are being added to the DB
  std::vector is used instead of std::deque for 10KB smaller dll
  this should be the official release build
  you can get detailed info about contacts being sent to you
  more bug fixes
1.2.0.0
  bugfix - contacts are always being added outside any group /forget actually writing to the DB/
  sending contacts is now a Miranda service, accessible via m_contacts.h
  implemented contactlist event, no more popup windows
  NOTE: the langpack is modified, because my english is at least questionable and some items are now obsolete

1.4.0.0 (New developer: Joe @ Whale, jokusoftware@miranda-im.org)
  now fully protocol independent
  made sending with full acknowledgement (needs miranda 0.3.4alpha) & improved Send Dialog
  now sent contacts are added to history (can be viewed by e.g. History++)
  improved Receive Dialog - added popup & some convenient functions, display more info (like ICQ)
  many fixes
1.4.1.3
  fixed multipacket sending (if 15 contacts, do not try to send empty packet)
  optimisations (reimplemented many structures, size cut by 20kB)
  when contact gets deleted close his send window
  redesigned Send dialog - more nice & effective (show only contacts of the same protocol + many more)
  grouplist on receive dialog now gets updated
1.4.1.7
  redesigned and improved Received dialog (get rid of WTL & MFC)
  empty groups in send dialog are hidden according to clui setting
  fixed multi-packet error handling
  fixed crashes on exit (thanks to Ghazan)
  small fixes
1.4.1.8
  fixed received dialog not showed on some systems
  other small fixes
1.4.2.0
  fixed memory leak on any event received (thx to Ghazan)
  fixed other small memory leaks
  popup menu on Received dialog gets translated now
  added DBE3++ Known Modules support
1.5.0.0
  many internal optimisations
  added Unicode 2in1 support
  added support for Unicode ready DB contacts events
  added Updater plug-in support
1.5.1.0
  added support for per-contact contacts API capabilities
  improved compatibility with unicode database API
  added support for new clist groups API
  several internal fixes & improvements
1.5.2.0
  fixed possible crash during contacts send (with older Miranda IM)
  fixed compatibility with the new Account API introduced in Miranda IM 0.8


Translation
-----------

Contacts plugin can be translated via the Miranda language files.
The required strings are provided in langpack_contacts.txt file.
