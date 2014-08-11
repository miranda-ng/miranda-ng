--------------------------------
| Facebook Protocol RM 0.2.4.0 |
|        for Miranda NG        |
|          (21.4.2014)         |
--------------------------------

Autor: Robyer
  E-mail: robyer@seznam.cz
  Jabber: robyer@jabbim.cz
  ICQ: 372317536
  Web: http://www.robyer.cz
 
Info: 
 - This plugin is based on Facebook Protocol (author jarvis) version 0.1.3.3 (open source).
 - His version you can find on: http://code.google.com/p/eternityplugins/

--------------------------------
   Information about statuses
--------------------------------
 - Online = connected to fb, chat is online
 - Invisible = connected to fb, chat is offline, but you can still see other people online, receive messages, newsfeeds and notifications
 - Offline = disconnected
 
--------------------------------
      Hidden settings
--------------------------------
"TimeoutsLimit" (Byte) - Errors limit (default 3) after which fb disconnects
"DisableLogout" (Byte) - 1 = Disable logout procedure, 0 = default
"PollRate" (Byte) - Waiting time between buddy list and newsfeed parsing.
"Locale" (String) - Get facebook errors in specific language, "en_US", "cs_CZ", etc.
"UseLocalTimestampUnread" (Byte) - 1 = Use local timestamp for offline (unread) messages
"KeepUnread" (Byte) - 1 = Don't mark messages as read on server (works globally or per contact)
"NaseemsSpamMode" (Byte) - 1 = Don't add contacts when we send message to them from other instances, add them only when they reply
"NameAsNick" (Byte) - 0 = don't use real name as nickname, use nickname if possible (default is 1)
"EnableChat" (Byte) - 0 = don't use multi user chat at all (default is 1)

--------------------------------
       Version history
--------------------------------
0.2.?.? - ?.?.2014
 + Hidden setting "NameAsNick" to not save real name as nickname (but it's pretty useless now)
 ! Improved saving names (save only when changed)
 ! Fixed loading own name

0.2.4.0 - 21.4.2014
 + Support true invisible status (they don't see you, but you see them)
 + Don't load "unread messages" which we received already (but didn't read them yet)
 + Add support for showing unread notifications count as "unread emails" in clist
 + Add status menu item "Visit Notifications" and icons for other items
 + Hidden setting for Naseem's spam mode
 * Respect "use local timestamp" setting also for "message read" events
 * Refactor receiving multi-chat messages (it doesn't load chat participants and chat names, etc. but it will be fixed in future versions)
 * Various typing notifications changes/fixes
   + Support receiving typing notifications for multi chats (and prepare for sending it, when *srmm will be ready for that)
   * Don't switch contact to online when he is typing (he can type from invisible)
   * Send typing notifications even to offline contacts
 ! Fix not to load offline multi chat messages to single contacts
 ! Fix size of downloaded avatars (use 50x50 for smaller and 180x180 for bigger)
 ! Various other fixes (crashes, poke/newsfeed texts,...)

0.2.3.2 - 19.3.2014
 + Working manual refreshing of contact info
 + Load user info when we're receiving message from someone who isn't in clist yet (fixes contacts with our own name)

0.2.3.1 - 15.3.2014
 ! Allow more Miranda instances to be connected to same account at same time
 ! Fix showing duplicates of sent messages (at the cost of slowing down whole processing and sending only one message at a time :()
 ! Crash fix (thanks ghazan)

0.2.3.0 - 11.3.2014
 + Add option to keep messages unread and loading only inbox messages
 + Add contact menu item to open "Conversation history" on website
 * Mark messages as read when they are really read in Miranda (only for private conversations - group chats are still marked as read instantly)
 * Don't notify errors marked as "silent" by Facebook
 * Use user-defined account name in popups (thanks ghazan)
 ! Hotfix for sticky number (some FB API update)
 ! Fix loading unread (offline) messages
 ! Fixed loading sent messages (from other connected devices)
 ! Workaround to "receive" unsupported attachments in messages
 ! Fixed url to (old) mobile website (thanks Naseem)
 ! Various internal code fixes, changes and cleaning (thanks ghazan)

0.2.2.1 - 27.11.2013
 ! Fixed marking messages of some multichats as read
 ! Fixed downloading avatars of some contacts (and repeated downloading of them again and again...)
 + Load messages marked as abusive/spam

 x Getting sent messages from other browser/mobile instances still doesn't work

0.2.2.0 - 19.11.2013
 + Correctly load contacts on mobile phone that are online right now (also removed old related option)
 + Added hidden option to keep messages unread to disable marking messages as read on server
 * Multi user chat improvements:
   + Load unread messages at login
   + Mark received messages as read
   * Don't show "unseen" in statusbar
 ! Spelling corrections (thanks RMN)
 ! Fix for remembering device (no more e-mail notification on every login)
 ! Fixed searching by ID/Login
 ! Don't switch typing contacts to online when we are not online
 ! Various other fixes and code cleaning

 x Getting sent messages from other browser/mobile instances still doesn't work
 
0.2.1.0 - 1.10.2013
 + Save name into first, second and last name fields separately
 + First experimental implementation of multi user chats (thanks to nobodyreal for patch)
 * Notify more facebook errors
 * Don't use local_timestamp for unread messages (even if enabled in options) by default (use hidden setting to enable it)
 ! Duplicit contacts fixes (and hopefully it won't create duplicit contacts anymore)
 ! Fixed Poke
 ! Post status dialog accessibility fixes
 ! Various login fixes (now also passwords with not-ASCII characters are working)
 ! Fixed receiving attachements
 ! Various other fixes and improvements

0.2.0.0 - 31.7.2013
 + Ability to search friends by ID or username
 + Greatly improved Post Status dialog
   - Ability to post status with tagged friends
   - Ability to post URL attachments
   - Ability to post statuses to other contact's wall
   - Ability to post statuses to own pages (you need to enable this in options first)
 + Improved receiving messages  with attachments (files, photos, stickers)
 + Greatly improved receiving offline messages at login
   - loads up to 21 unread messages per contact
   - loads messages also from "Other" messaging tab
   - uses most effective requests with ability for future function "loading contact history from server"
 * Increased timeout value for requests from 15 to 20 seconds.
 * Switched completely to core JSON parser
 * Raised messages and status updates length limit
 * Use Popup Classes for popups
 ! Various contacts handling fixes
 ! Various login fixes (now supports enabled advanced security on Facebook)
   - it connects after approving this unknown device in browser - BUT you must logout and login in your browser for correct approval! (fb bug)
 ! Receiving offline messages fixes and improvements (gets also name of contact)
 ! Fixed not receiving sent messages from other instances 
 ! Fixed crash when using AddContactPlus
 ! Various other fixes and improvements

0.1.1.1 - 12.6.2013
 + Hotkey for Facebook posting dialog
 + Load sent messages from other devices into Miranda
 ! Fixes related to messages handling (no empty and duplicit messages anymore)

0.1.1.0 - 21.5.2013
 * Mind dialog improvements
 + Added privacy and place options into Mind dialog
 + Toolbar button for setting Mind
 
0.1.0.1 - 20.5.2013
 ! Fixed showing notifications without url
 + Added icons for notification and newsweed popups
 + Marking notifications as read with right click on popup

0.1.0.0 - 17.5.2013
 ! Group messages are not received as single messages anymore 4661
 + Showing info in statusbar when contact read your message
 + Added contact menu item to open Friendship details
 + Added contact menu item to Poke
 ! Fixed loading newsfeeds
 ! Fixed getting login error message
 + Added option to open links on another server (mobile, etc.)
 + Added option to use local time for received messages
 + Added option to send "message read" info automatically
 + Added hidden value for getting language specific errors (add in db key "Locale" (string) with locale value ("en_US", "cs_CZ" etc.))
 ! Internal changes and cleanup 

0.0.9.6
 ! Fixed getting offline messages
 + Loading some timestamp value of contacts (dword "LastActiveTS") for... I don't know.
 ! Translation fixed (thanks Basil)
 ! Fixed canceling friendship
 ! Some internal changes (thanks ghazan)

0.0.9.5 - 2.3.2013
 ! Folders plugin related fixes (thanks ghazan)
 ! Some internal changes (thanks ghazan)

0.0.9.4 - 28.8.2013;
 ! Fixed showing correct login error
 ! Some internal changes (thanks ghazan)

0.0.9.3 - 8.2.2013
 ! Fixed getting friendship requests
 ! Fixed getting notifications
 ! Fixed posting status messages
 + For posting status messages use city "Miranda NG"
 + Experimental support for Custom smileys
 ! Fixed Visit profile menu
 + Logging also into network log (if enabled logging)
 * Don't log cookies values
 * Changed invisible icon (thanks Mataes)
 ! Fixed phone icon

=== OLD CHANGES (MIRANDA IM) ===

0.0.9.3 - 12.12.2012
 ! Fixed crash when no own avatar found on webpage
 * Plugin statically linked
 
0.0.9.2 - 23.10.2012
 ! Fixed contacts' avatar changes
 ! Fixed connection with one type of required setting machine name
 ! Reworked sending messages requests (should avoid rare ban from FB :))
 ! Create default group for new contacts on login if doesn't exists yet
 ! Fix for visibility changes from other client
 * Plugin linked to C++ 2008 runtimes again

0.0.9.1 - 16.10.2012
 ! Fixed not working login due to Facebook change
 * Plugin linked to C++ 2010 runtimes

0.0.9.0 - 11.6.2012 
 + Receiving friendship requests (check every +-20 minutes)
 + Reqorked authorizations - requesting, approving, rewoking friendships
 + Support for searching and adding people
 * Changes of some strings
 * Use same GUID for 32bit and 64bit versions
 ! Receiving messages with original timestamp
 ! Fixed removing avatars from "On the phone" contacts
 ! Unhooking OnModulesLoaded (thanks Awkward)
 ! Don't send typing notificationsto contacts that are offline
 ! Fixed avatars working (thanks borkra)
 ! SetWindowLong -> SetWindowLongPtr (thanks ghazan) 
 ! Working login with approving last login from unknown device
 ! Fixed sending messages into groups
 ! More internal fixes

0.0.8.1 - 26.4.2012
 ! Fixed getting notifications on login
 ! Fixed getting unread messages on login
 ! Getting unread messages on login with right timestamp
 ! Fixed getting newsfeeds
 ! Fixed related to deleting contacts from miranda/server
 + New newsfeed type option "Applications and Games"
 + Contacts now have MirVer "Facebook" (for Fingerpring plugin)
 + Getting attachements for unread messages on login
 ! Fixed avatars in Miranda 0.10.0#3 and newer (thanks borkra)
 ! Some small fixes (thanks borkra)
 
 x Doesn't work notification about friend requests

0.0.8.0 - 14.3.2012
 # For running plugin is required Miranda 0.9.43 or newer
 # Plugin is compiled with VS2005 (Fb x86) and a VS2010 (Fb x64)
 + Added 2 types of newsfeeds: Photos and Links
 * Reworked options
 ! Fixed setting to notify different type of newsfeeds
 ! Fixed and improved parsing newsfeeds
 ! Fixed getting groupchat messages which contains %
 ! Fixed not working login
 ! Improved deleting of contacts
 + Support for Miranda's EV_PROTO_ONCONTACTDELETED events
 + Added missing GUID for x64 version and updated user-agent
 ! Some other minor fixes or improvements
 ! Fixed item 'Visit Profile' when protocol menus are moved to Main menu
 * Updated language pack file (for translators) 
 - Disabled option for closing message windows on website (temporary doesnt work)

0.0.7.0 - 19.1.2012
 + Support for group chats (EXPERIMENTAL!) - enable it in options
 ! Fixed loading contact list
 ! Fixed potential freeze.

0.0.6.1 - 6.1.2012
 + Returned option to close chat windows (on website)
 + New option to map non-standard statuses to Invisible (insetad of Online)
 + New option to load contacts, which have "On the Phone" status
 ! Fixed changing chat visibility
 ! Very long messages are no longer received duplicitely
 ! Changes and fixes related to multiuser messages and messages from people, which are not in server-list

0.0.6.0 - 18.11.2011
 * Reworked Facebook options
  + Option for use https connection also for "channel" requests
  + Option for use bigger avatars
  + Option for getting unread messages after login (EXPERIMENTAL!)
  + Option fot disconnect chat when going offline in Miranda
  - Removed option for setting User-Agent
  - Removed option for show Cookies
 * When contact is deleted, in database you can found datetime of this discovery (value "Deleted" of type DWORD)
 + Option in contact menu for delete from server
 + Option in contact menu for request friendship
 + When deleting contact is showed dialog with option to delete contact also from server
 ! Fixed not working login

0.0.5.5 - 16.11.2011
 ! Fixed contacts not going offline

0.0.5.4 - 16.11.2011 
 ! Fixed few problems with connecting
 ! Fixes for some crashes, memory leaks and communication (thanks borkra)
 @ If is your Facebook disconnecting and you have enabled HTTPS connection, disable option "Validate SSL certificates" for Facebook in Options/Networks.

0.0.5.3 - 31.10.2011
 ! Fixed issue with login for some people
 ! Fixed not receiving some messages
 ! Fixed broken getting own name
 * Faster sending messages

0.0.5.2 - 31.10.2011
 ! Fix for connecting and crashing problem.

0.0.5.1 - 30.10.2011
 ! Work-around for sending messages with url links.

0.0.5.0 - 29.10.2011
 + Notification about friends, which are back on serverlist.
 * Completely reworked avatar support.
 * Using small square avatars by default (can be changed by hidden setting "UseBigAvatars")
 ! Fixed use of hidden setting "UseBigAvatars" 
 ! Fixed setting status message.
 ! Fixed crash with MetaContacts.
 ! Fixed login for some people.
 ! Fixes related to popups on login.
 ! Fixed memory leak related to popups.
 ! Fixed getting unread messages on login (if used hidden setting "ParseUnreadMessages")
 ! Fixed login with setting Device name.
 ! Various fixes, improvements and code cleanup.
 - Removed hidden key "OldFeedsTimestamp"
 @ Thanks borkra for helping me with many things.

0.0.4.3 - 22.9.2011
 ! Fix for new facebook layout.
 ! Fix for getting contact name for new contacts.

0.0.4.2 - 15.9.2011
 ! Fixed not enabling items in status menu.
 ! Don't automatically set contact's status to Online when we got message from him.
 ! Fixed message's sending error codepage.
 + Added support for sending messages in invisible status.
 + Protocol status respects changes of chat status on website.
 + Notify concrete unread notifications after login
 + Added hidden key (ParseUnreadMessages) for getting unread messages after login << WARNING: not fully working!!!

0.0.4.1 - 13.9.2011
 ! Reverted change that made contacts not going offline.

0.0.4.0 - 12.9.2011
 * Internal changes about changing status
 - Removed support for "Away" status
 ! Fixed parsing newsfeeds
 - Removed notification about unread messages in "Invisible" status
 + In "Invisible" status are inbox messages parsed directly to messages
 + Getting gender of contacts
 + Getting all contacts from server at once (not only these, which are online right now)
 + Notification when somebody cancel our friendship (= or when disables his facebook account)

0.0.3.3 - 17.6.2011
 ! Fix for communication (getting seq number)
 ! Fixed notification with unread events after login

0.0.3.2 - 8.6.2011
 ! Mark chat messages on facebook read.
 * Disabled channel refresh notification

0.0.3.1 - 23.5.2011
 ! Fixed notify about timeout when sending message.

0.0.3.0 - 22.5.2011
 ! Fixed downloading avatars
 ! Fixed loading avatars in SRMM
 ! (Maybe) Fixed not loading for some people with miranda 0.9.17
 ! Fixed crashes and freezes when deleting account
 ! Few fixes and improvements related to login procedure
 ! Fixed parsing some newsfeeds
 ! (Hopefully) Fixed some other crashes
 ! Fix for logon crash when notify unread events.
 * Improvement in getting contact avatars (2x faster)
 * Optimized compiler settings -> 2x smaller file (thanks borkra)
 + Used persistent http connection (thanks borkra)
 + Destroy service and hook on exit (thanks FREAK_THEMIGHTY)
 + Support for per-plugin-translation (for MIM 0.10#2) (thanks FREAK_THEMIGHTY)
 + Support for EV_PROTO_ONCONTACTDELETED (for MIM 0.10#2) (thanks FREAK_THEMIGHTY)
 - Do not translate options page title, since it is the account name
 - Disabled close chat "optimalization"
 ! Fixed sending typing notifications
 ! Fixed parsing links from newsfeeds
 * Enabled sending offline messages
 ! Fixed getting errors from sending messages (+ show concrete error)
 + 5 attempts to send message before showing error message

 
0.0.2.4 - 6.3.2011
 ! Fixed duplicit messages and notifications
 * Limit for popup messages from group chats - max. one per 15 seconds
 * Optimalization for sending typing notify
 * Optimalization for closing chat windows on website
 + Notify when is possible that we didnt received some chat message(s)
 + Show popup when try to send offline message and open website for send private message when onclick

0.0.2.3 - 5.3.2011
 ! Fixed loading contact names
 ! Fixed updater support for x64 versions
 + Added 32px status icons
 + Added option for chose type of newsfeeds to notify
 * Changed user-agent names to user-friendlier
 * Enhanced parsing newsfeeds

0.0.2.2 - 2.3.2011
 + Updater support
 + Added item in contact and status menu for open contact profile on website (+ saving in db as Homepage key)
 + Notify about new private (not chat) messages in invisible
 + Automatically set https when connecting if required
 * Optimalization for downloading avatars of contacts
 ! Fix for \n in newsfeeds popups
 ! Fix for html tags in connecting error message
 ! Fix for broken sending messages

0.0.2.1 - 21.2.2011
 ! Fixes for statuses (cannot switch to Away, work in threads)
 ! Fix for loading own avatar when changed
 * Sounds are using account name (thanks FREAK_THEMIGHTY)
 ! Fixes for x64 version (thanks FREAK_THEMIGHTY)
 ! Fix for thread synchronization
 ! Fixed order of outgoing messages and notify about delivery errors
 ! Fix few things which was causing not delivering all incoming messages
 ! Fixes for internal list implementation
 * Refactoring and simplify few things
 + Added x64 version of plugin
 + Notify about new notifications after login
 ! Fixed parsing few types of newsfeeds
 + 1. stage of groupchats - notifying incomming messages

0.0.2.0 - 13.2.2011
 * Guided as new plugin Facebook RM + new readme and folder structure
 x Temporarily disabled updater support (and notification about jarvis's new "fb api")
 ! Fix for load own avatar
 ! Fix for loading newsfeeds and their better parsing
 + Added Away and Invisible statuses
 * Using away status as idle flag
 ! Fixed idle control - facebook falls into idle only when Away status

--------------------------------
      Old version history
 (Already in official version)
--------------------------------
0.0.2.0 (0.1.3.0) - 20.12.2010 (not released)
 ! Oprava zobrazování bublinového oznámení
 ! Oprava odhlašovací procedury
 ! Oprava zobrazení poètu nových zpráv
 ! Oprava správy neèinnosti  TODO
 ! Oprava naèítání stavových zpráv TODO
 ! Oprava naèítání avatarù TODO
 + Kontrola úspìšného odeslání zprávy
 * Aktualizována modifikovaná miranda32.exe na nejnovìjší verzi
 * Rozdìleno readme na èeské a anglické
1.21 - 27.11.2010
 + Notifying about received new "private" (not chat messages) messages
 + Pseudo idle management (when idle, miranda let facebook fall into his own idle)
 + Hidden key for ignoring channel timeouts (add key "DisableChannelTimeouts" (byte) with value 1)
 * Rewrited and edited few things (maybe fixed duplicit messages, maybe added some bugs, etc.)
 ! Fixed idle change of connected contacts
 ! Fixed writing time in log
1.20 - 22.11.2010
 + Option for use balloon notify instead of popups
 + Option for use https protocol (can help with some firewalls)
 + Edited miranda32.exe is added for correct work without timeouts, until somebody rewrite http communication :)
1.19 - 20.11.2010
 ! Fixed downloading and updating contact avatars
 ! Dont set "Clist\MyHandle" to contacts
 ! Fixed html tags in status messages
 ! Show error message when try to send message in offline/to offline contact (only ugly workaround)
 ! Try to fix crashes with too long text from wall post
 ! Temporary ignore timeout errors (until fixed in miranda core)
1.18 - 28.9.2010
 ! Fix for force reconnect.
 ! Dont popup contacts disconnect when self disconnect
1.17 - 23.9.2010
 ! Try to workaround duplicit messages
1.16 - 10.9.2010
 * No old news feeds after login (for old behavior create BYTE key OldFeedsTimestamp with value  1)
 ! Fix for empty news feed (empty feeds will not be notified)
1.15 - 9.9.2010
 ! Fix for setting -1 (infinity) timeout
1.14 - 9.9.2010
 ! Fixed /span> in popups
1.13 - 9.9.2010
 ! Closing fb message window (when receiving message) is in other thread
1.12 - 8.9.2010
 ! Fixed bug when last contact went offline.
1.11 - 7.9.2010
 ! Fix for duplicated wall events without link (change photo, etc.)
1.10 - 7.9.2010
 + Basic opening facebook urls on left mouse btn click
 + Added popup options (colors, timeouts,...)
 + Info about CONCRETE error message when unsuccessful login
1.9 - 4.9.2010
 + Typing notifications
1.8 - 4.9.2010
 + Option to automatically close chat windows (on website)
1.7 (0.1.2.0) - 7.8.2010
 ! Don't show Miranda's "Set status message" dialog when disabled "Set Facebook 'Whats on my mind' status through Miranda status"
 ! Fixed some memory leaks, but some can be still there...
 + Added timestamps into facebook debug log.
1.6 (0.1.1.0) - 15.6.2010
 ! Fixed (again) not working login when "Notification from new devices" enabled.
1.5 - 10.6.2010
 ! Fixed popup with new facebook api available
 ! Fixed bug with sending messages through metacontacts
 ! Fixed one small bug
1.4 - 7.6.2010 14:45
 ! Fixed GetMyAwayMsg for ansi plugins (e.g mydetails)
 + Added sound and basic FB Icon to popups
1.3 - 7.6.2010 01:00
 ! Fixed infinite "contact info refresh" "loop"
 ! Fixed bug when global status icon was set to offline when facebook was switched to offline
 ! Fixed feeds popups
1.2
 ! Login should work even if "Notification from new devices" is enabled.
1.1
 ! Fix for not working login
1.0
 + Added hidden key in database: "Folder" Facebook, key DisableStatusNotify (type Byte), value 1. (Newly added contacts will have set "Ignore status notify" flag (in Options / Events / Filter...)


