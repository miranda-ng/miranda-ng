About
-----
New "Notes & Reminders" Plugin

Version 0.0.5.0
Updated by Georg Fischer for Miranda IM 0.9.2+ (may work on earlier versions
possibly down to 0.7.0 but not tested) and VS2008, based on code:

by Joe @ Whale (Joe Kucera) jokusoftware@gmail.com
Originally by d00mEr (Lubomir Ivanov) d00mEr@dir.bg
for Miranda ICQ 0.1.2.1+written with Visual C++ 6.0IMPORTANT: When upgrading to 0.0.5.0 (from 0.0.4.5-), it's recommended to make a backup of           the Miranda DB file, because the format for notes and reminder DB data has           changed. Allthough the new version should be able to read the old format, the           old plugin version will not be able to read the new format, in case you need           to downgrade plugin version again.
Description
-----------
This plugin allows user to create Sticky Notes, to
store some important data in a well visible place :)
New features allow to create reminders. This is a
note that will show up on specified date and time.
The notes are completely customizable.

Features
--------
+ Sticky Notes (each Note is limited to 16000 chars)
+ Reminders (each Reminder Text is limited to 16000 chars)
+ Reminder time input displays a preset list with times. For the current date (if date is not
  changed) it displays presets for 5, 10, 15, 30 minutes and then every full and half hour. When
  the time input is in current date mode, you can also enter a delta time by first entering a +.
  For example +5 would specify in 5 minutes, +1:15 would specify in 1 hour and 15 minutes.
+ Allow change of fonts and colors
+ Allow setting default size for Notes
+ Notes always stay on top of all windows (unless option is disabled)
+ Show and Hide all notes
+ Show or Hide notes at startup.
+ Bring all notes to front (for notes that are not on top and are hidden behind other windows).
+ Hot Keys for New Note, New Reminder, Show/Hide notes and Bring All Notes to Front
+ Manage Reminder list.
+ Manage Notes list.
+ Quick delete all notes and/or reminders
+ Transparent Notes (on Windows 2000 or newer)
+ Language translations (see langpack_N&R.txt for details)
+ Select between 3 custom sounds to play On Reminder
+ Reminder alert sound can optionally play repeatedly at specified interval until user responds
  to alert (makes it less likely to miss important reminders)
+ Font Colors
+ Reminder notification via E-mail or SMS.

To Do:
------
- Verify langpack for changes and new features introduced in 0.0.5.0
- User configurable icons through the Miranda icon configuration functionality
- Icons for : Fix/Move the note
- Text formatting support for notes
- "best fit" feature - by pressing one button I would adjust the size of the note automatically
- double clicking a note's title-bar scrolls up the note (to be just the title bar).
- import/export of all notes/alarms
- Reminder (daily, weekly or monthly) without a specified time? i.e. when you start miranda...
- Trigger on event (remind me when is user online)
- Unicode support (Use Japanese texts was garbled)
- Easier way to delete notes (like Ctrl-D or ctrl-shift-d)

Installation
------------
Just copy the dll into Miranda's plugin subdirectory.

Translation
-----------
see N&R-langpack.txt for details

License
-------
Copyright (C) 2002 Lubomir Ivanov
Copyright (C) 2005 Joe Kucera

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.

For more information, e-mail jokusoftware@gmail.com
                             d00mEr@dir.bg

Changes
-------
0.0.5.1
-------
! BugFix: Notes window init issue that could result in scrollbar being shown when it shouldn't.
! BugFix: Notes window init issue that could result in scrollbar position not being restored correctly
          for notes that are suposed to be scrolled to top
+ Feature: Option to specify a custom program to use when opening links in notes. By default links
           are opened using the system default web browser.
+ Feature: Set custom font on individual notes.

-------
0.0.5.0
-------
* (Temporary) Developer: Georg Fischer, I just updated it to work properly with the latest Miranda 
  version and added some features and polish. I don't know if I'll do much more work aside from immediate
  bug fixes, at least on any regular basis.
! BugFix: Custom frame drawing for notes to look nicer but also avoid issues especially with Aero.
! BugFix: Changed window type/parent for notes so that they don't show up on Windows alt-tab list
          and other issues.
! BugFix: Cleaned up custom window drawing for notes windows, to hopefully work more solidly.
! BugFix: Fixed pasting into notes window, so it doesn't paste text formatting (as formatting
          is not currently supported and would be lost next time Miranda starts).
! BugFix: Removed reminder reoccurrence dialog options, it would just confuse the user because
          there is no support for that functionality in the code.
! BugFix: Time handling reworked, which hopefully also fixes reported issue with reminders triggering at
          wrong time (an hour early or later). Special handling for daylight saving (summer time) begin
          and end was also added, which in some cases could lead to some issues on old Windows versions
          which have outdated being and end dates/times.
! BugFix: Potential crash bug with reminder list window and empty reminder note.
! BugFix: Transparency slider (in options) now uses correct direction, previously 100% transparent was
          actually 100% Opaque. Also clamped the internal max value to avoid 100% transparent windows.
! BugFix: Creating note from a reminder could result in a crash or corrupted data if note was deleted
          before the reminder.
! BugFix: A few other unlisted bug/issue fixes.
+ Change: Changed DB format to be more flexible and easily allow adding features in the future without
          breaking existing notes and reminders.
+ Change: Changed "Show notes at startup" to "Hide notes at startup" because that better matches
          actual behavior.
+ Change: Changed the default menu item order to have New Note and New Reminder first as they're the
          most commonly used commands.
+ Optimization: changed list iterations in various placed to directly iterate linked lists instead
                of using index based for-loops.
+ Optimization: reminder list sorted by trigger time, which reduces update function to a single
                check 99.99+% of the time.
+ Feature: Hotkeys, fonts and colors now configurable through the common config interface.
+ Feature: Notes save/restore their current scroll position.
+ Feature: Notes title bar is configurable to select prefered date and time formats or disable them.
+ Feature: Buttons in notes title bar can be disabled (all functions are also available in the context menu).
+ Feature: Bring All Notes to Front command, brings all visible notes in front of other windows
           (without changing the on-top state).
+ Feature: Tweaked notes window context menu and added a couple of commands.
+ Feature: Option to make "Add Reminder" button in New Reminder dialog close the dialog (previously
           it was required to click Close after Add Reminder to get rid of the dialog).
+ Feature: Reminder alerts now generate a Miranda system event, meaning it will blink the tray icon
           which the user has to click in order to open the reminder dialog. Previously the reminder
           dialog opened automatically which could interfere with the application currently being used.
+ Feature: Delete confirmation dialogs for all notes and reminder delete actions, to avoid accidental
           deletion.
+ Feature: Reworked reminder date and time input controls. In particularily the time edit control works
           more like it did in ICQ aswell as allowing user to enter custom values.
+ Feature: Per-reminder option to have alert sound repeat (until reminder event is acknowledged by
           clicking the tray icon). Repeat interval can be selected between 5 and 60 seconds.
+ Feature: Set custom background and text color on individual notes.
+ Feature: Set custom title/caption on individual notes. (max 63 characters)
+ Feature: Select between 3 sound presets or disable sound completely for individual reminders.
+ Feature: Edit/Update reminders.
+ Feature: Update reminder message in reminder notify dialog, so that the text can be updated
           before chosing to remind again.
+ Feature: Notes list dialog.
+ Feature: Notes/Reminders list dialogs resizable with persistent window and column sizes.

-------
0.0.4.5
-------
* New Developer: Joe @ Whale, jokusoftware@gmail.com, I took over the development
   of this great plug-in, got sources of 0.0.4.0 from original author (newer were
   lost), hopfully managed to add features of succeeding versions.
! BugFix: Hopefully fixed all crash issues & memory leaks
! BugFix: Fixed random reminder issue
+ Optimisation: minimised use of memory alloc/free (much faster loading)
+ Feature: reminder sound now configurable thru default Skin/Sounds module (EVents/Sounds)

-------
0.0.4.2
-------
! BugFix: Reminder -> SMS Notify now works.
+ Option: Show vertical scrollbar in Notes.
+ Feature: Create Note from content of Reminder Notify.
+ Feature: Icons for Delete Note, Hide Note

-------
0.0.4.1
-------
! BugFix: URL in Note problem fixed
! Bugfix: Closing Reminder dialog will now mean "Remind me again"
          You must press "Dismiss" to remove reminder
! BugFix: Strange behaviour to add reminders + 1 hour.. (Now uses Localtime)
! BugFix: Missing Langpack strings

-------
0.0.4.0
-------
+ Totaly revriten everything!!! Moved to Microsoft (R) Visual C ++ 6.0
+ Added TopToolbar buttons for "New Reminder" and "New Note"
+ Notify via SMS to E-mail gateway (Only for Reminders)

-------
0.0.3.1
-------
+ Sorry, i've forgot to remove debug :( That's why the plugin crashes
  Now i've removed it :) Also i've relocated plugin to new base addres, so
  there must not be a crash on start or exit (btw if you use any other
  Delphi plugins, please contact autor to relocate them to a different
  base addres than standart $4000000, or there will be a crash !!

0.0.3.0
-------
+ Reocurrence of Reminders (Daily, Weekly, Monthly)
+ Fixed bug with crash :(

0.0.2.8
-------
+ Changed DB structure again (fixed bug with Notes & Reminders size greater
  than 4096 bytes). Now every Note & Reminder are stored into separate DB
  setting, but every Note or Reminder is still limited to 4000 bytes :( sorry
  for that. It is recomended you to clear all notes & Reminders to avoid errors :)
+ Implemented Purge function to free unused Notes & Reminders data from DB :)
+ Removed 'Empty' sign when a Note or Reminder have no text :)
+ Is the "Reminder On Top" bug still there? It works fine for me ?!? :)
  (I've tested on Win ME, 2K Pro & XP)

0.0.2.7
-------
+ Fixed bug with Reminders not showing on top of all windows

0.0.2.6
-------
+ "Remind me again in:" now includes any date and(or) time in the future,
  not just (5,10,15...etc min.).
+ Changed Name in Options dialog.

0.0.2.5
-------
+ Togle On-Top with the pin icon (Icon reflects status)
+ Change Font effects & colors (For Caption & Note body)
+ A "View Reminders" button in the "Add Reminder" box.
+ Rearanged TAB order :)

0.0.2.4
-------
+ Custom Sound to play On Reminder

0.0.2.3
-------
[Warning!!! New Format of DB data! Incompatible with old versions]
[It is recomended to delete all Notes and Reminders              ]

+ Fixed bug - not showing corectly "Remind Again In :" Combo Box
+ Fixed bug - Trying to delete from empty list of reminders causes
  error message.
+ Fixed bug - Problem when using Unicode characters
+ Other minor bug (or not bug) fixes :)

0.0.2.2
-------
+ Switched to Delphi 6 (smaller code)
+ There where so many feature requests for language translations :)
  So i decided to implement this feature :)
+ I think finally fixed the on-top bug ?!?

0.0.2.1
-------
+ Fixed bug on 9X platforms (Didn't draw notes correct) :) Sorry, i forgot
  to initialize length of an structure :)
+ Changed Name :)
+ Added transparency of Notes (Win 2K & XP)

0.0.2.0
-------
+ Reminder function implemented :)
+ Changed version to 2.0 :)

0.0.1.6
-------
+ Fixed structure in DB to store new features (Visible,On Top).
  (Now DB Settings of plugin are incompatible with old versions,
  so if you update , you will loose all notes).
+ Added Popup menu items for new features (Visible, On Top).
+ Minor changes in code to improve preformance. Smaller code :)

0.0.1.5
-------
+ Replaced Hot Keys with global Hot Keys.
+ Added option to change these Hot Keys.
+ Added Popup menu to Notes (Cut,Copy,Paste).
+ Added option to set default size of notes on create new.
+ Basic Implementation of reminders.

0.0.1.4
-------
+ Now saving Notes data on every change to avoid loosing notes if crash.
+ Fixed some bugs with window placement.
+ Added options for staying Always On Top.
+ Added Hot Keys to menu.

0.0.1.3
-------
+ Fixed bug with notes that have no text.
+ Added functions to Show/Hide Notes.
+ Added some icons :)

0.0.1.2
-------
+ Added Options dialog, to alow change of color and fonts for notes.

0.0.1.1
-------
+ Changed to save Notes data into Miranda database insted
  of Windows registry.

0.0.1.0
-------
+ First release of this plugin.