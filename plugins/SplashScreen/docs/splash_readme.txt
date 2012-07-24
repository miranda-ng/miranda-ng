Splash Screen Plugin for Miranda IM
Version 0.1.2.2
Copyright: 2004-2007 nullbie, 2005-2010 Thief
Addons page: http://nightly.miranda.im/
Beta site: http://nightly.miranda.im/
Forum thread: 

REQUIREMENTS:
advaimg.dll
Get it from nigtlies: http://nightly.miranda.im/

NOTES:
Splashes should be in PNG format (alpha channel supported) or BMP (no alfa)
Sounds should be not broken .wav files
To enable plugin after disabling it in options you must edit plugins/splash.ini (or mirandaboot.ini) and set
"Active" param to 1 or enable splash display at options page.

Thanks to:
nullbie, who gave me the sources and for helping with development
Faith Healer for ideas and feedback
foxter.ru for first splashes design (I really like them)
Miranda project for the greatest IM client ever :)

Version history:

0.1.2.2 beta
[-] Crash fix by George Hazan

0.1.2.1 beta
[-] Fixed: wrong absolute path was used for preview
[-] Fixed: preview function completely stops working when closing splash by right click (thx m0003r)
[-] Enabled and fixed tab navigation in options
[*] Updated langpack

0.1.2.0 beta
[!] Switched to advaimg.dll for processing pngs, png2dib is not used anymore
[+] Plugin API changes: added uuid (C64CC8E0-CF03-474A-8B11-8BD4565CC) and MIID_SPLASHSCREEN interface (91CB1E8D-C33C-43C0-BDD8-6725B070B3E0)
[+] Added an option to show Miranda version on splash (more options to be implemented)
[+] Added environment variables support (%PROFILEPATH%, %APPDATA%, etc.)
[+] Added Ini param to point to config file (aware of environment variables support, thus makes possible to use different ini-es for different profiles)
[*] Removed mp3 support (was too buggy, will reimplement eventually)
[*] When random feature is enabled file name is omitted from path when selecting a new file

0.1.1.7 beta
[+] Experimental mp3 playback support
[-] Fix for long standing (from 0.1.1.x) 24-bit images loading issue (thx nullbie)
[-] Fix for thread hang
[-] Hopefully fixed "splash not closing" problem on some configurations
[*] Random feature searches only for valid extensions - bmp and png
[*] Redesigned options page a bit
[*] Langpack changes
[*] Alot of other changes, reorganizations and fixes

0.1.1.6 beta
[-] Fixed "Display time" option. Now should be really working. If you want splash to be closed after Miranda fully loaded set it to 0
[-] Fixed: sometimes splash wasn't autoclosed with TimeToShow = 0
[*] Display time option can accept values up to 20000 mseconds
[*] Dinamically linked build is removed from archive to avoid cluttering of miranda root folder when using Updater.
[*] Code cleanup

0.1.1.5 beta
[+] Updater support for betas (don't forget to enable "Use beta?" option in Updater settings)
[+] Dinamically linked build (you'll need MSVCR71.dll to run it)
[-] Another workaraund for crash when opening options due to regression
[-] Fix for the "Play sound" check
[*] A small change to options page
[*] Langpack addition

0.1.1.4 beta
[+] Option to scan a folder and select random file for display
[+] Added a service (see m_splash.h for details) (thx MiCHi for inspiration)
[-] Fix: the Apply button was enabled every time options were opened with SplashScreen page
[*] Added third state to PlaySound checkbox in options. Means not to follow global sound state
[*] Reduced dll size due to switch to VS2003
[-] Some other minor fixes

0.1.1.2
[*] ini file removed from archive, it will be created automatically on startup if not exists
[-] Version bump in order to fix wrong updater notifications in previous version (sorry for that)
[!] Known issue: won't load non 32-bit splash image

0.1.1.1
[-] A little fix for options page

0.1.1.0
[!] Switched to png2dib (v0.1.3.x or later) for png processing (Faith Healer wins ;P). Imgdecoder no longer needed.
[-] Fixed inability to use absolute path(s)
[+] Options to control Display time and Fade in/out (somewhat temporary solution)
[*] Translation additions.

0.1.0.6 (not published)
[+] When sounds disabled in miranda disable it in splash screen (thx +eRRikon for suggestion)

0.1.0.5
[*] Renamed dll into AdvSlashScreen to increase loading priority on NTFS file systems
[+] New ini option: TimeToShow, defines time (in msecs) how long splash should stay on screen 
[+] Added ability to play sound
[+] URL to File Listing page of plugin
[+] Can use mirandaboot.ini as config (if splash.ini not found)
[*] Little cleanup: splash.ini goes into /plugins, /splash into root dir (default locations)
[-] Changing fading setings at options page directly affect preview
[*] Now fadeouts when closed via leftclick

0.1.0.4
[!] First release on FL
[+] Options to enable fade in/out 
[*] Splash preview disappears after 2 secs (thx nullbie)
[*] OpenFile dialog now uses last used folder as initial
[+] DisableAfterStartup param in ini - allows to disable splash after first run, later it can be activated again in options
[+] Option to control plugin activity

0.1.0.3
[+] Preview button in options (rightclick to hide splash)

0.1.0.2
[+] Options page added
[*] Using config instead of hardly coded values
[-] Workaround for crash when plug is disabled and opening options dialog

0.1.0.1
[+] Win 9x "alfa channel" trick
[+] Rightclick dismisses splash

0.1.0.0
[!] First beta release

CONTACT:
send your comments, suggestions, bugreports and feature requests here: turyak@gmail.com
or PM me on Miranda forums.

This plugin is free software; you can redistribute it and/or
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
