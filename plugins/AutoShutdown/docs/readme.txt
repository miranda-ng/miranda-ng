
AutoShutdown 1.4.0.2
------------------------------------------------------------------------
                                      Plugin for Miranda IM 0.7 and +

	Plugin Info: ----------------------------
	| Version:     1.4.0.2
	| Filename:    shutdown.dll
	| Author:      H. Herkenrath (hrathh at users.sourceforge.net)
	| Description: Adds the possibility to shutdown Miranda IM
	|              or the computer after at a specified time.

	Contents: -------------------------------
	| Features, Requirements, Usage, Installation,
	| Bugs and Wishes, To-Do List, Version History,
	| Thanks, Translation, License and Copyright

Features
----------------------
	+ Shutdown Miranda IM or your computer after a specified time,
	  when receiving a special message or when all file transfers are completed
	+ Offers the following shutdown possibilities:
	  Logoff, Restart, Shutdown, Standby mode, Hibernate mode, Lock workstation,
	  Hang up dialup connections, Close Miranda IM, Set Miranda offine
	+ The available shutdown possibilities are only shown if they
	  are supported on your system
	+ The available shutdown events are only shown if you have 
	  protocols or plugins installed that support the features
	+ Displays a confirmation dialog which is shown for 30 seconds
	  to cancel the process
	+ Plays a sound on confirmation dialog
	+ Displays the countdown in a special window below contact list
	  (this feature needs a MultiWindow enabled Contact List plugin)
	+ Plays a sound when the confirmation dialog is displayed
	+ No data gets lost on shutdown especially no unread messages
	+ Allows you to pause the shutdown countdown
	+ If Miranda is closed while automatic shutdown is running it will
	  show a dialog on next start of Miranda IM where it can be selected
	  to start automatic shutdown again
	+ Provides shutdown services for other plugins
	+ Full Unicode support
	+ Automatic installation of all files, just unzip into Plugins directory
	+ Supported plugins/tools:
	  MultiWindow Contact Lists (frames and hotkeys), Hotkey Plugins,
	  Magnetic Windows, Snapping Windows, TopToolBar, AutoReplacer,
	  Trigger Plugin (http://www.pboon.nl/projects.htm), HDD Info,
	  Weather Protocol

Requirements
----------------------
	-> Miranda IM 0.7+:
	Miranda IM is needed in version 0.7 or later.

	-> Optional: Weather Protocol 0.3.3.17+:
	If you would like to use the thunderstorm shutdown feature,
	you need to have Weather Protocol installed, version 0.3.3.17 or later.

	-> Optional: HDD Info 0.1.2.0+:
	If you would like to use the harddrive overheat shutdown feature,
	you need to have the HDD Info plugin installed, version 0.1.2.0 or later.

	-> Optional: Trigger Plugin 0.2.0.69+:
	If you would like to use the Trigger feature,
	you need to have the Trigger plugin installed, version 0.2.0.69 or later.

Usage
----------------------
	AutoShutdown can be activated via the main menu:
	'Main Menu' -> 'Automatic Shutdown'

	The options to use 'Hibernate mode' and 'Lock workstation'
	are only available if your system supports it.
	All other options are only available if they are enabled on your system.

	Enabling 'Hibernate mode' on Windows ME/2000 and later:
	'Control Panel' -> 'Power Options' -> 'Hibernate'
	To enable the Hibernate feature go to the Control Panel,
	double-click Power Options, and then click the Hibernate tab.
	Click to select the Enable Hibernate Support check box.

Installation
----------------------
	Find 'miranda32.exe' on your computer.

	Just copy all the contents of the zip-file as they are into the 'Plugins'
	subdirectory in the Miranda IM folder.
	'AutoShutdown' will detect the files and move them into the appropriate
	directories on it's first run.
	
	You can also do all the installation by hand, if you want to:

	Main Plugin: Copy the file 'shutdown.dll' into the 'Plugins' subdirectory
	in the Miranda IM folder. 

	The Unicode version of the plugin will only work on Windows NT/2000/XP,
	Windows Server 2000, Windows Vista or later with an installed
	Unicode version of Miranda IM.
	To use it on Windows 95/98/Me, please download the ANSI version of the plugin.
	  
	Documentation: The txt-files should be moved along with the SDK-zip into the
	'Docs' directory in the Miranda IM folder. 

	Sounds: The wav-files should be moved into the 'Sounds'
	directory. They will get recognized automatically and be added to the sounds list.

	That's it!

Bugs and Wishes
----------------------
	Feel free to mail me your wishes about 'AutoShutdown' and tell
	me all the bugs you may find:  hrathh at users.sourceforge.net

To-Do List (random ideas)
----------------------
	? Design some icons for the shutdown types (shutdown, reboot, etc.)...anyone?
	? Ddd possibility to shutdown on status change of specific users
	? Support Alarms Plugin API (m_alarms.h):
	  This would replace countdown frame, showing the shutdown countdown
	  on the Alarms frame instead (don't know if this is a good idea, not yet possible)
	? Show system tray icon when automatic shutdown is enabled (not needed, bad looking)
	? Add possibility to set Miranda to a specific status instead of offline
	  (really shutdown related?)
	? Start a specified application on event (really shutdown related?) 

Version History
----------------------
	1.4.0.2 - minor fixes
	1.4.0.1 - ensure correct threshold values for cpuusage
	        - uses langpack locale for combobox sorting
	        - fixed: fontservice items were registered too early
	        - fixed: unicode problem with tray menu item
	        - fixed: settings dialog did not show up with parent
	        - added v0.8 support
	        - minor other improvements
	1.4.0.0 - Added possibility to shutdown when computer finishes
	          a busy task (high cpu usage)
	        - Support for AutoReplacer in message editbox
	        - Fixed minor issue with reactivating on miranda startup
	        - Really fixed shutdown on message in unicode build 
	        - Updated new miranda headers, use of mir_forkthread
	        - Fixed automatic installation routine, now works again
	        - Bug-Fix: countdown hours (and higher) were converted wrongly
	        - Fixes for Win9x and WinNT4
	        - Adjustments for hung apps
	        - Tweak: support for ENDSESSION_CLOSEAPP in Windows Vista
	        - Code reorganization
	        - Minor Vista and robustness tweaks
	        - Improved FontService and IcoLib support (core built-in)
	1.3.1.1 - Tweaks for blind users
	        - Minor speed-up for frame drawing when resized
	        - Cleanups
	        - workaround for frames not drawing when previously hidden
	1.3.1.0 - Added FontService support
	        - Added HDD Info overheat shutdown
	        - Updated Trigger plugin support (0.2)
	        - Updated TopToolBar support to latest version (0.7.3)
	        - Fixed 'lock workstation' on WinNT4
	        - Marquee on progress	
	        - Minor fixes and improvements
	        - Minor updates for new Miranda
	1.3.0.8 - Bug-Fix: Check for unicode core was wrong
	        - Some other minor changes
	1.3.0.7 - Improvements to OKTOEXIT handling
	        - Bug-Fix: Daylight saving time was ignored in time conversion
	1.3.0.6 - Reduced ANSI file size (corrected compiler settings)
	        - Minor internal tweaks
	        - Bug-Fix: Confirmation countdown in options was not displayed correctly
	        - Included debug symbols (PDB) into SDK package
	        - Adjusted db settings
	1.3.0.5 - Minor internal tweaks
	        - Fixed crash on start when Trigger plugin installed
	1.3.0.4 - Minor improvements and some internal changes to the countdown frame
	        - Finally fully fixed the dialup shutdown problem
	        - Added: Now also adds Close Miranda and Set Miranda offline as
	          actions for the trigger plugin (was requested)
	1.3.0.3 - Bug-Fix: Another try to fix the 'hangup dialup connections' freeze
	1.3.0.2 - Bug-Fix: 'hang up dialup connection' freezed Miranda sometimes
	1.3.0.1 - Fix: On some rare situations Miranda was not shutdown correctly
	        - Fix: Frame was not shown correctly on clist_modern
	        - Fixed mw_clist hotkey
	        - Some minor improvements
	        - Added shutdown event
	        - Fixed minor workstation locked issue
	        - Fixed 'sec' not shown correctly on frame (bug in WinAPI)
	        - Made 'hang up dialup connections' code more robust
	        - Bug-Fix: countdown values were not saved
	        - Fixed: shutdown dates in past were sometimes not recognized correctly
	1.3.0.0 - Make use of FORCEIFHUNG flag instead of FORCE whenever it is possible
	        - Shutdown at [hh:mm] and [yy-mm-dd] instead of Shutdown only at [hh:mm]
	          (also making use of calendar common control)
	        - New shutdown type: set status to offline
	        - Make use of ME_IDLE_CHANGED to shutdown on idle
	        - Added 'Thunder Shutdown' functionality
	        - Now making use of marquee mode of progressbar of time countdown is to
	          long to be displayed 
	        - Added: Hotkey support for toggling automatic shutdown on/off
	        - Added: IcoLib support
	        - Updated: DBEditor++ support
	        - Added: Support for Trigger plugin (shutdown actions)
	        - Added: Unicode support
	        - Added: Functionality of Thunder Shutdown plugin (see option page)
	        - Removed: AgressiveOptimize.h to make it work on all systems
	        - Improved: New service functions
	        - Added: Shows countdown frame in normal window when frames are not available
	        - Added: Hotkey support for hotkey services (toggle)
	        - Improved: Now makes use of StrFromTimeInterval, BroadcastSystemMessage
	        - Improved: Made file transfer shutdown logic more robust
	        - Imroved dialogs look and feel
	        - Cleaned up and revisited code
	1.2.0.4 - Changed time control to use windows default control
	        - Fix: The icon on the countdown frame was sqeezed a bit
	        - Improved shutdown dialog layout
	        - Some small string improvements/changes (see Shutdown-Translation.txt')
	        - Some other minor code changes and improvements
	1.2.0.3 - 'Shutdown on file transfer' and 'Shutdown on message receival'
	          are now only enabled if plugins and protocols are installed that
	          support file transfer and/or instant messaging features
	        - Some other minor changes
	1.2.0.2 - Fix: Shutdown on message receival only worked when
	          message dialog was closed (Thanks to Hurricane and Foo)
	        - Fix: Sound option dialog sometimes crashed
	          when selecting a different countdown sound file (Thanks to Rex)
	        - Minor string improvements (see 'Shutdown-Translation.txt')
	        - Some small internal improvements
	1.2.0.1 - Fix: TopToolBar button showed wrong behaviour if
	          dialog was already opened
	1.2.0.0 - Add: New shutdown type 'Hang up dialup connections'
	        - Fix: Combo box did not remember last option correctly
	        - Some other small fixes/changes/improvents
	1.1.3.1 - Fix: Tabulator did not work correctly on settings dialog
	        - Fix: Some problems with the 'last exit remembering'
	        - Fix: 'Cancel' button was not default button on shutdown dialog
	        - Some other minor fixes/improvements 
	1.1.3.0 - Improved: Sending of WM_ENDSESSION
	        - Add: If Miranda is closed and automatic shutdown is running
	          it will remember it and shows on next Miranda start a message box
	          where the user can select to start automatic shutdown again
	        - Add: When no option is selected it does no longer allow the
	          user to click on OK
	        - Fix: Time input control sometimes got hidden when editing hours
	        - Some other minor improvements/fixes
	1.1.2.0 - Fix: The shutdown on file transfer completion feature
	          did not always work correctly, especially with ICQ file transfers
	        - Fix: If the default sound file isn't installed it doesn't
	          anylonger give out the windows default sound by default
	        - Add: If Miranda is closed and automatic shutdown is running
	          it will remember it and start on next Miranda start again
	1.1.1.1 - Fix: Shutdown on specific time sometimes did not work correctly
	          and crashed Miranda on opening the 30s confirmation window
	1.1.1.0 - Fix: Default time was sometimes not detected correctly
	        - Improved: Shutdown/Restart is now faster due to improved
	          sending of WM_ENDSESSION
	        - Improved: When the countdown is paused it shows a blinking text
	        - Change: Context menu can now also be opened via left click
	        - Change: Shutdown on message receival now checks for
	          if the message contains the specified text instead of checking for
	          exactly the same content
	        - Change: 'Cancel Countdown' in context menu now only stops the countdown
	          instead of stopping all shutdown causes
	        - Change: When using shutdown at time feature the countdown display will
	          show the time instead of the countdown (Suggested by Rootgar)
	        - Change: The context menu now show a different text instead of being
	          checked when the countdown is paused
	        - Added/Changed some strings (see 'Shutdown-Translation.txt')
	        - Some other minor changes/improvements
	1.1.0.0 - Add: Possibility to shut down when all file transfers are finished
	        - Add: Possibility to shut down on message receival
	        - Add: WM_ENDSESSION now gets send to all applications manually on
	          shutdown/restart to prevent data loss
	        - Add: Now containing default sound file boundled with zip
	        - Add: Possibility to pause/unpause the countdown via right click on it
	        - Fix: Shutdown/Restart was not shown on Windows 2000/XP
	        - Fix: Updated link on plugin options page
	        - Fix: Countdown got updated one second after reactivating clist
	        - Change: Sound now gets played repeatedly on 30s shutdown dialog
	        - Change: Now 'Cancel' is the default button on 30s shutdown dialog 
	        - Improved/Added some strings (please see 'Shutdown-Translation.txt')
	        - Some other minor improvements
	1.0.0.0 - Initial release

Thanks
----------------------
	* To Corsario (Angelo Luiz Tartari) for the first idea of a shutdown plugin
	* To Dennys for the first idea of rebooting via message ('dReboot' plugin)
	* To noname for the thunderstorm shutdown feature
	* To Marek 'tusz' Tusiewicz for the sound file and testing
	* To Rootgar and Tigerix for their bug reports and suggestions

Translation
----------------------
	Translation strings can be found in 'Shutdown-Translation.txt'.

License and Copyright
----------------------
	'AutoShutdown' is released under the terms of the GNU General Public License.
	See 'Shutdown-License.txt' for more details.

	Copyright (c) 2004-2007 by H. Herkenrath. All rights reserved.

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
