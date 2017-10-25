BuddyExpectator+ plugin v0.1.2.3
This plugin notifies about contacts who has returned after an absence of long period.

Features:
 - Supports Popups module
 - You may specify a sound for its' event
 - Logs 'last seen' statistics (use User Details dialog)
 - Uses built-in 'Ignore' list

Version history:

0.1.2.3
- added extraicons for MissYou via hidden key (byte)bMissYouIcon - icon slot number
- added MissYou options to UserInfo page
- some fixes

0.1.2.2
- key BuddyExpectator/LastStatus is written to contact's module
- fixes to unload code

0.1.2.1
- bugfixes

0.1.2.0
- added an option to hide inactive contacts
- added an option to set custom absence perion for a contact
- addded support for Popup+ actions
- removed ignore online check
- changed icons (thanks to a0x and induction)
- implemended IcoLib API for menu icons (requires Miranda 0.7)
- implemented new plugin API
- implemented Updater support for beta versions

0.0.0.7-0.0.1.2 (by Thief)
- percontact notification period setting
- IcoLib support
- implemented "Miss You" feature
- more popups colors options
- disabled notifications for non-IM protocols

0.0.0.3-0.0.0.6 (by sje)
- "long time no see" feature 
- first time online notification 
- storing contact creation date in db 
- other fixes including memleaks clearing

0.0.0.1-0.0.0.2
- initial version by Anar Ibragimoff

Why
'Status notifyer'-like plugins are preety good... while your contactlist contains just 5, 10, 20 buddies.
Mine includes over 100 persons (family, friends, colleagues, messmates...). And every moment somebody goes offline, 
others come back... So my left side of screen is always flickering with green/red popups ;-) 
Sure, I can turn them off... But I want to be notified when somebody comes back after a vacation, business 
trip, maternity leave ;-)
I found 'Contacts Notifier', but it's really inconveniently to watch for all buddies and their activity... and setup 
a notifyer for everyone who has gone...
So I have thought out to notify just about contacts who were offline for a long time. 
In other words, it works like 'Last seen' plugin, but with notification ;-) 

(c) 2007-2008, Alexander Turyak (turyak@gmail.com)
(c) 2006, Scott Ellis (mail@scottellis.com.au)
(c) 2005, Anar Ibragimoff, Minsk, Belarus (ai91@mail.ru)