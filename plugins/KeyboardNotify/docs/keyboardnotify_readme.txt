		-=====================================-
		 Keyboard Notify plugin for Miranda-IM
		-=====================================-


Copyright (C) 2002,2003	Martin Öberg
Copyright (C) 2004	Std
Copyright (C) 2005-2006	TioDuke (tioduke@yahoo.ca)


Description
-----------
This plugin for Miranda-IM notifies user of specified events (as incoming messages, incoming files, incoming URLs or other events). This plugin is based on the original one by Martin Öberg (aka strickz) and Std's modifications (mainly the idea of using direct port handling using a driver).

It has many options allowing:
  a) To select on which events to react
  b) Under which conditions (eg: fullscreen mode, ScreenSaver running, workstation locked)
  c) To act only if the protocol receiving the event is under specified status
  d) For message events you can choose to be notified if the message window is open or not
  e) A notification feature allowing to be notified of pending events (unopened events) after specified period of time
  f) To select method for stopping the blinking (after x secs, if Miranda is re-attended, if Windows is re-attended, if all notified events are opened or when the notify conditions end)
  g) And several flashing options: select leds to blink, effects (all at the same time, in turn, in sequence - left to right, right to left and left to right and to left again - and custom, preview button

It was designed to be flexible and performing different tasks. It can be configured to act like the original one and has several functions from other Miranda's keyboard notifier plugins.

It also provides services to allow third party plugins use its notifier capabilities and supports Trigger plugin.

---------------------------------------------------------------------------------------------------------------------------------------------------------------------
IMPORTANT NOTE FOR USB KEYBOARD'S USERS: The plugin will only work if you choose to emulate keypresses (which is not the nicest solution but is the only way I found)
---------------------------------------------------------------------------------------------------------------------------------------------------------------------


Installation
------------
Just the typical Miranda way: drop keyboardnotify.dll into your plugins folder, restart Miranda and configure the options as you like... voilà! done! NO OTHER FILES ARE REQUIRED.


Options
-------
Options page Options->Plugins->Keyboard Flash. Tabbed: Protocols, Rules (when), Flashing (how), Themes and Ignore.

Here there are some explanations on features on the Options' Page.

Protocols tab:
Protocols to check: check/uncheck the protocols you want this plugin to work with.

Rules tab:
Events to react on:
   - Incoming messages: check this if you want to be notified on incoming message events
   - Incoming files: check this if you want to be notified on incoming file events
   - Incoming URLs: check this if you want to be notified on incoming URL events
   - Everything else: check this if you want to be notified on incoming events that are not messages, files or URLs (like authorization requests or you-were-added events).
Message event only:
   - Blink if message window is open: check it if you want to be notified of incoming messages even if that contact's message window is open, if you uncheck it you will only be notified of incoming messages if the window is closed
   - ... and not in foreground: check this option if you want to be notified if the message window is open and that window is not focused
   - Only if last is xx seconds old: if you check this option the plugin will not notify you of new messages until xx seconds have passed after the last message for that contact was sent/received
Notify when:
   - Full screen mode: check this if you want to be notified when you are on full screen mode (screen saver excluded form the full screen classification, although it is a special type of full screen application)
   - Screen saver is running: check this if you want to be notified while screen saver is running
   - Workstation is locked (2000/XP): check this if you want to be notified when your workstation is locked (only for Windows NT systems)
   - Defined programs are running: check this one if you want to be notified when certain programs are running. To define those programs you have to click over the [...] button and enter the .exe filenames (including the .exe suffix).
   - All other situations: check this if you want to be notified when neither you are in full screen mode, nor the screen saver is running and nor the workstation is locked.
   Note on 'notify when': to be notified on ANY condition (always) you will have to check them all.
Notify if status is:
   - Online, Away, NA, etc: check each status for which you want notification to be turn on. Checking all means on all statuses.
   Note on 'notify if status is ...': The plugin will check these options against the event's protocol status, this means that if you choose to be notified only when you are online and at a certain moment you are online in one protocol but away in another, then you will be notified on events for the first one only.
Flash until:
   - nn seconds: if this option is selected leds will flash for the amount of seconds you have specified
   - Miranda/Windows is re-attended: if this option is selected flashing will stop when either the mouse or a key is pressed on any of Miranda/Windows's windows
   - Events are opened: if this option is selected flashing will stop when there will be no more pending events (for the selected protocols, of course)
   - End of 'notify when' conditions: if this option is selected the flashing will stop when all situations checked under 'notify when' (and also the status conditions) ceased to be true.
   Note on 'End of notify when conditions': this means that if you select ALL the 'notify when' situations and all the statuses (e.g. always) flashing will not stop (you may still stop it manually by pressing the Pause/Break key).
   This option is particluarly usuful if you want to be notified only when you are in full screen mode (for example) and you want the flashing to stop when you change to non full screen mode. The same applies to all the other situations.
Pending Events:
   - Remind me every xx minutes: this option will make the plugin check for unopen events every xx minutes and start notifying if any is found and the 'notify when' and the status conditions are met. If you don't want this option, just specify 0 minutes and it will be disabled.
   This option is usuful if you, for example, select to be notified on workstation locked and then to stop the flashing using 'end of notify conditions'. If you do not open the event that generated the flashing (e.g. leaving the event pending) then when you lock your computer back the flashing will begin again.
   Note that if on your messaging plugin you have enabled the option to pop up the message window, then the reminder won't alert you of messages (as they will be promptly opened) but it will still work for files/URLs/others.

Flashing tab:
Keys to flash:
   - Num Lock: check this if you want the Num Lock led to blink
   - Caps Lock: check this if you want the Caps Lock led to blink
   - Scroll Lock: check this if you want the Scroll Lock led to blink.
   Note on 'keys to flash': if you don't choose any you will see nothing happen.
Flashing effects:
   - All at the same time: if you select this one the leds will blink at the same time
   - In turn: if you select this one the leds will blink this way: Num+Scroll first and then Caps if the three of them are selected in 'keys to flash', if not they will blink alternately
   - In sequence: if you select this one leds will blink one at the time in sequence; the order of the sequence depends on:
        - left->right: from left to right
        - right->left: from right to left
        - left<->right: from left to right and then right to left (producing the infamous KITT effect)
   - Custom theme: if you select this one you will have to select from the combo box one of the themes you have created in the "Themes" tab. For more information refer to the 'Themes tabs' explanation
   - Accordingly to events' count: if you select this option the plugin will flash the num lock led as many times as pending message events there will be, the same for caps lock and file events, scroll lock and urls events and the three at the same time and other events.
     Since v.1.5.3.2 you can use the [...] button to open the dialog that allows you to change the default leds assign to each event.
   Note on 'flashing effects': effects will take into consideration the 'keys to flash' options, which means that only the checked leds will blink. In the case of custom flashing sequences (themes), the plugin will show no led turned on if you specify one led (or a combination of leds) that is not selected in 'keys to flash'.
Wait before starting flashing:
   - nn seconds: specify the amount of time for the plugin to wait before starting flashing (0 means do not wait).
Speed: You may select between 5 different speeds that go from really slow to really fast.
Preview button: This button will allow you to test your flashing settings. Since v.1.5.3.3 it is an on/off button (thank you tweety for the definition), which means that it will flash when it is on and stop when it is off.
Note on preview button: the preview button will show nothing at all when you select the "Accordingly to events' count' flashing effect: as most probably there will be no pending event it will just turn off your leds (it has really nothing to show).
Emulate keypresses: if you check this option the plugin will act as if keystrokes were sent from the keyboard, which means that some nasty things may happen, as toggling on and off capital letters or being kicked from Windows' screensaver. It is meant only for people having trouble with the other method (ie people using USB keyboards).

Themes tab:
Create/Edit theme:
   - In the 'Theme' combo box you type the name of the new theme you want to add or you select from the dropdown list the one you want to modify or delete.
   - In the text box you specify a flashing sequence in the form of an expression in a special 'flashing language' created by me (to take a look at the specification of this 'language' please refer to next section: 'Custom order language').
   - Test button: use this one to see how the sequence in the text box will actualy look in action.
   Note on the test button: this button and the preview button act differently (and don't expect the 'Preview' to act like this one, as its function is completely different). First, this one will show you how the sequence will look without taking into consideration options in the 'flashing tab' (like 'keys to flash', 'wait delay', but it will use the 'speed' setting). This test button will finish playing the sequence after showing it twice and at least for 2 seconds (both conditions). This behaviour, I think, will work fine for almost any user but if you think you need to change that (say, making it flash for 3 seconds or 4 times) there is a way if you change two settings stored in the DB using DBEditor++: you have to change the values of "testnum" and/or "testsecs", both of the 'byte' type (the first one rules the x times condition and the second the x seconds one). I did not put these options in the Option page just because I don't like the idea of a control to control another control (superstition?); I find it confusing.
    - Add/Update/Delete: these buttons will either add a new theme, update and delete an existing one. 'Add' will be enabled only if you type a new theme name in the theme combo box. 'Update' will be enabled if you have modified the flashing sequence for an existing theme. 'Delete' will be enabled if the theme currently selected is an existing one (e.g., not a new one that has not been added yet). After pressing any of these buttons the 'Apply' button will be automatically enabled. This means that changes will be saved in the database only after activating the 'Apply' button.
Import/Export:
    - Export: it will export the themes stored in the DB to a text file with the .knt extension (keyboard Notify Theme).
    - Import: it will load into the option page the themes specified in a text file with the .knt extension (the file, obviously, has the same format that the export one). Themes will be stored only after clicking on the 'Aply' button (just like the add/update/delete buttons).
    - Override existing: check this option if you want existing themes (it checks by theme name) to be overwritten when you import them. If you don't check this option existing themes will not be overwritten.
Note on Import/Export: I packed my own themes file to show you the file format (mythemes.knt). Most of the themes are the creation of Sir_Qwerty (thank you very much *g*), the boring ones are mine.


Custom order 'language'
-----------------------
I designed a special 'flashing language' in order to allow users to provide the plugin with the flashing sequence. Here is an explanation of which symbols it uses and what they mean:
1: turn on Num Lock led
2: turn on Caps Lock led
3: turn on Scroll Lock led
0: turn all leds off
[...]: turn on at the same time the leds specified inside the brackets (1, 2, 3)
This is the complete list of symbols. Any other will be ignored by the plugin parser (and cleaned from the text box).
And here you have some examples of how the predefined effects should look in this 'language':
All at the same time: [123]0
In turn: [13]2
In sequence (left->right): 123
In sequence (right->left): 321
In sequence (left<->right) 1232
Turn on all leds without blinking: [123]
Sliding lights: [12]30[23]10
Heartbeat (created by sir_qwerty): 00000000000000000000000000000000303
Note: remember that when the sequence is finished the plugin will go back to the beginning until it has to stop due to 'until when' settings.


  Description
  -----------
  This plugin for Miranda-IM notifies user of specified events (as incoming messages,
  incoming files, incoming URLs or other events). This plugin is based on the original one
  by Martin Öberg (aka strickz) and Std's modifications (mainly the idea of using direct
  port handling using a driver).
  It has many options allowing:
  a) To select on which events to react
  b) Under which conditions (eg: fullscreen mode, ScreenSaver running, workstation locked)
  c) To act only if the protocol receiving the event is under specified status
  d) For message events you can choose to be notified if the message window is open or not
  e) A notification feature allowing to be notified of pending events (unopen events)
  after specified period of time
  f) To select method for stopping the blinking (after x secs, if Miranda is re-attended,
  if Windows is re-attended, if all notified events are opened or when the notify conditions
  end)
  g) And several flashing options: select leds to blink, effects (all at the same time,
  in turn, in sequence and like KITT!), preview button
  It was designed to be flexible and performing several different tasks. It can be
  configured to act like the original one and has several functions from other Miranda's
  keyboard notifier plugins.
  It also provides a service to allow third party plugins use its notifier abilities.

  Options
  -------
  Options page Options->Plugins->Keyboard Flash. Tabbed: Protocols, Rules (when), Flashing (how), Themes and Ignore.

  Thanks
  ------
  - Pete for the numerous patches he sent, actively helping to improve the code and
  functionality
  - UnregistereD for great help in solving problem with Windows activity detection
  - Slacktarn, Sir_qwerty and Tweety for giving great help with ideas (most of the new
  features included in this plugin were suggested by them) and testing
  - The authors of AAA, PopUp+, KeyScrollNotify, original KeyboardNotify, Neweventnotify,
  IEView, NGEventNotify for part of their code used in this plugin.
  - Vampik fot the InTurn flashing option
  - Miranda IM developers for this amazing program
  - all other people from Miranda community

  History
  -------
  1.5.7.7:
    [!] Added support for Miranda 0.8.x.x.
  1.5.7.6:
    [!] Fixed bug in Ignore module.
  1.5.7.5:
    [!] Updated TriggerPlugin support for latest version.
  1.5.7.4:
    [*] Updated screenshot
	[*] Minor code cleaning
  1.5.7.3:
    [+] Added xstatus support
  1.5.7.2:
    [+] Added per contact Ignore options
  1.5.7.1:
    [!] Fix in Options for themes under WinXP+ (really essential feature)
  1.5.7.0:
    [+] Added support for Trigger plugin
  1.5.6.3:
    [-] Removed device presence validation: it is not needed now that the plugin works on USB (thanks Nick, aka Aestetic)
    [+] Added a new service to the API for 'normalizing' a custom flashing sequence string
    [-] Simplified the API (the extended version of the start blink service is no longer needed).
  1.5.6.2:
    [!] Fixed problem while trying to detect if message window is in foreground.
  1.5.6.1:
    [!] Fixed bug with keypress emulation and "until Windows is re-attended" option.
  1.5.6.0:
    [+] Option to emulate keypresses (for the USB people)
    [*] Changed the emergency key (to make it stop with PAUSE instead of SCROLL LOCK key).
  1.5.5.4:
    [*] Improved ListView control handling
    [*] Changed the default values (for the sake of new users).
  1.5.5.3:
    [*] More code optimization.
  1.5.5.2:
    [+] Support for Update plugin.
  1.5.5.1:
    [!] Minor source fixes.
  1.5.5.0:
    [+] New 'notify when' option: while defined programs are running (just like gamerstatus)
    [+] Extended the API to add two new services to disable and re-enable keyboard notifications (for use by bosskey plugin).
  1.5.4.4:
    [!] Fixed (hopefully) problems with some system configurations (with PS2 keyboards) where the KeyboardClass0 device was not the apropriate one (thanks pete!)
    [+] Extended the plugin API (requested bt tweety).
  1.5.4.3:
    [!] Fixed some compatibility issues with nconvers++ (thank you donatas for your help).
  1.5.4.2:
    [!] Fixed problem with Windows' activity detection under Win9X when using other plugins that do the same.
    [!] Fixed crash caused by incoming authorisation requests when metacontacts was enabled.
  1.5.4.1:
    [!] Some corrections on third party plugins events handling (now they are more assimilated to the 'other events')
    [*] Some code cleaning
    [!] Fixed problem with first message in Metacontacts recognition while checking for pending events (thank you again NirG)
  1.5.4.0:
    [+] New plugin API (thank you CriS for your ideas and great help)
    [!] Added Offline status to status check list (thank you Slaktarn for finding it).
  1.5.3.4:
    [!] Fixed Metacontacts recognition in checking and counting of pending events (thank you NirG for finding the problem)
    [!] Fixed problems with multiple instances of the plugin running (thank you tweety for reporting and testing).
  1.5.3.3:
    [!] Changed behaviour of Preview button to make it independent of the rules' options.
  1.5.3.2:
    [+] New dialog to asign leds to specific events for the trillian-like sequences.
  1.5.3.1:
    [!] Fixed bug of loosing any other until setting when selecting 'Events are opened'.
  1.5.3.0:
    [+] Applied pete's patches (thank you very much for your great work)
        - Use of GetLastInputInfo when possible for detecting Windows' activity
        - Made Windows' mouse hooks also aware of mouse clicking
        - Made Miranda re-attended option react on windows restoring and ignoring mouse hovering an unfocused window
        - New option for message events to avoid blinking if message window is focused
        - Made the plugin handle metacontact's special issues
    [!] Use of the new message API for windows detection when possible
    [+] New message event option to check last message timestamp (requested by D46MD)
    [+] Possibility of choosing more than one flash until option at the same time
    [+] New flashing effect to make the leds blink accordingly to number of events
    [+] Possibility of selecting/unselecting protocols (requested by tweety, usuful to avoid flashing on some protocols as rss)
  1.5.2.2:
    [!] scriver's message window detection (thanks D46MD for your great help)
    [!] corrected 'flash until' checking accordingly to pete's patch (thank you)
  1.5.2.1:
    [!] nconvers++'s message window detection
    [!] checked window detection for srmm, scriver, sramm and srmm_mod
  1.5.2.0:
    [+] Custom theme support
    [-] Custom order history
  1.5.1.0:
    [+] Custom order effect
    [+] Custom order history
  1.5.0.0:
    [+] Drivers aren't needed anymore
    [+] Status selection option
    [+] Miranda/Windows activity detection (thank you Peter Boon)
    [+] 'Opened events' stop method
    [+] x seconds stop method
    [+] Hooking database event for detecting incoming events
    [+] Notifier option for pending events
    [+] Checkbox for enabling disabling open messages notification
    [+] In sequence and KIT flashing effects
    [+] Preview button
    [+] Tabbed options
    [!] Several corrections/improvements in options page
    [!] Not selected leds will preserve their original state
  1.4.1.0: (by me and Vampik)
    [+] Notify on fullscreen, screensaver, worksation locked
    [!] Try to improve Win98 keysimulation routines
    [+] Added InTurn effect (thank you Vampik)
    [!] Corrected speed of blinking (thank you Vampik)
  1.4.0.0: (by Std, unreleased)
    [+] Added direct port handling using PortTalk.sys driver
  1.3.0.0: (by strickz)
    This is strickz' final release. It still uses keypress simulation. It was nice (thanks *g*)

TODO:
-----
    - I consider the plugin feature finished. :-D

License
-------
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

=====================================================================

