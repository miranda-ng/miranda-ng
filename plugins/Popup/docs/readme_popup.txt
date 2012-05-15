PopUp plugin, v.1.0.1.9
   by Luca "Hrk" Santarelli
   hrk@users.sf.net

- IMPORTANT -

Developers, read m_popup.h. It's both in the source and in the binary package.


- SUPPORTED PLUGINS -

PLEASE, read here!

PluginName		Minimum Required Version
NewStatusNotify		1.1.1.5
Winamp Notify		1.5
NewEventNotify		x.x.x.x
WhoIsReading[...]Notify		x.x.x.x
EmotIcon		x.x.x.x
YAHOO protocol		x.x.x.x
modified MSN protocol		x.x.x.x
Weather		x.x.x.x
NotifyAnything	 x.x.x.x
...	x.x.x.x

- LATEST CHANGE -

1.0.1.9 New: plugins can choose a custom timeout or no timeout at all (using
        the new POPUPDATAEX structure and MS_POPUP_ADDPOPUPEX function).
        New: PopUp plugin can be totally disabled depending on status (thanks
        to MatriX's help).
        Fix: Delphi/Pascal headers (m_popup_inc) had a mistake which caused
        Access Violations. It has now be fixed. :-)
        Change: "General Options" has been moved to "PopUps" page.
        Fix: some sentences were missing in the langpack (andreas)
        New: added support for Miranda Installer/Wassup by Tornado.


- KNOWN BUGs -

There are NO known bugs.
Oh well... except for the multithreading issue ("Concurrent PopUps").
If you get strange crashes with "Concurrent PopUps" on, disable it.


- INFORMATIONS (you can skip this, but you can read it too!) -

Hi :-)

This plugin is the son of NewStatusNotify.
You can use it even if you don't use NewStatusNotify, and probably you're here
because some other plugin has "PopUp Plugin" as requirement. :-)

This lets you have all the popups at the same place, being them status
notifications, previewed messages, WinAmp song titles or whatever the plugin
you installed does. :-)

Should I feel that way, I'll make it so that you can have popups of different
plugins in different places but... do you really need it?


- USAGE -

Open the options page and make your choices. You can choose:
-> where to have the popup Window. Either one of the corners or the center of
   the screen, but in this case the windows will appear cascaded (like the
   old SplitMsgDialog Plugin or StatusFloater) instead of spreading.
   You can spread them vertically or horizontally.

-> Display hour and minute in the popup (thanks to the author of StatusFloater
   for the cool idea!). You can have an "AM/PM" style too.

-> Choose which style the popup should have. Border, size, second line...

-> Set the interval for the popup windows to live (;-)).

-> Choose the font used in the popups, and if you wish the name to be in bold
   or not. Oh, you can choose the colour too.

-> Color of the background too, obviously.

-> And the default Windows' color too, obviously :-)

-> Choose to have a fixed size popup, or a dynamically sizing popup: this means
   that if the contact shown has a small nick and status text, the popup will be
   small. If the contact shown has a long nick or long status text, the popup
   will be large enough for the text not to be cut/clipped! :-)
   No more clipped text! [except for "Latin"/"Serif" fonts like MS Sans Serif]

-> If you enabled the resizing of PopUps, but sometimes they're too small for
    your tastes, you can choose to have a "Minimum width", which will avoid too
    small PopUps. Same goes with "Maximum width".

-> Plugin works with "Large Fonts" systems and "Other..." systems too. It has
   problems with "Serif" family fonts :-( (they get cut sometimes) but...

-> ... you can nevertheless enlarge the PopUps by a choosen % value, avoiding
   this annoying thing. :) [suggested is 105%, but read FAQ]

-> Transparency. Should I add more? ;-) "Just slide the slider!"

-> Animations, Fade in, Fade out! :-)


- ROADMAP - (or: gee, what's next?)

* Anything you think useful. :-)


- TRANSLATION -

The strings you can translate are in a separate file, made for people who don't
use a langpack file. Just copy what you need. :)


- CHANGES -

1.0.1.9 New: plugins can choose a custom timeout or no timeout at all (using
        the new POPUPDATAEX structure and MS_POPUP_ADDPOPUPEX function).
        Fix: Delphi/Pascal headers (m_popup_inc) had a mistake which caused
        Access Violations. It has now be fixed. :-)
        Change: "General Options" has been moved to "PopUps" page.
        Fix: some sentences were missing in the langpack (andreas)
        New: added support for Miranda Installer/Wassup by Tornado.

1.0.1.8 Fix: PopUps still running when Miranda is closed will now be cleanly
         destroyed and memory freed. (This could prevent some crashes and could
         just do nothing...)
         New: Custom fade in and out delays.
         Change: transparency percentage acts now like Miranda's one (thus
         showing opacity and not transparency).
         Fix: maximum width and minimum width were loaded wrongly.

1.0.1.7 Fix: maximum and minimum width were saved wrongly.
         Source will not be released due to lazyness. :)

1.0.1.6 Fix: position, style and spreading option bug. Configure them again,
         the old values had to be deleted.

1.0.1.5 Fix: bug of wrong resize of popups when AM/PM is enabled.
         Source for will not be released (I won't ever forget this bugfix...)

1.0.1.4 New: multiline popups. :-)
        Change: reordered general options dialog.
        Fix: threads weren't killed if too much popups were created.
        Change: PopUpList_Show() consumes less memory.
        Change/Fix: made PUShowMessage() work better with single lined popups.
        Change: translators, something changed. :)
1.0.1.3 Fix: concurrent popups should work fine. They do here. :)
        Fix: removed some debug code left in 1.0.1.2

1.0.1.2 New: added a menu item to enable/disable every popup depending on PopUp
        plugin. Note that this will not affect other plugins, but will only
        disable *popups*. If you don't want to get status notification (as
        example) and you have NewStatusNotify, you have to disable
        NewStatusNotify too or you'll see no popups but you will hear the
        sounds!
        New: MS_POPUP_QUERY service for developers. You can enable/disable
        every popup with it.
        New: added an advanced option to enable/disable Concurrent PopUps. Open
        FAQ.txt and read the documentation. :-)
        Fix: some crashes should be fixed even if "Concurrent PopUps" is on.
        Fix: some names weren't shown due to a bug in CreateWindow() API. It
        has been fixed thanks to Vassily Goissa (vgoissa).
        New: added a m_popup.inc for Delphi developers. Thanks to egodust.

1.0.1.1 Fix: thanks to Tornado and BGMonster a memory leak has been removed.

1.0.1.0 New/Fix: while working on multiline popups, when the text will be two
        lines (or more) long, a "..." will be appended to the first line, and
        the popup will be rightly sized. Just try it with NewEventNotify or
        NewStatusNotify.
        Fix: MultiMonitor was broken in latest release, it now works again. [If
        you need UNICODE support, recompile the plugin with #define UNICODE]

1.0.0.9 Fix: plugin will now load under WinNT and Win95. Problem was related to MultiMonitor options.
        Fix: MultiMonitor options will now be enabled only if there is a multimonitor setup. :-)
        Fix: "RegOpenKeyEx()" error fixed in NT platforms (9x weren't affected).
        Fix: custom maximum width and minimum width weren't properly saved.
        New: PUChangeText() resizes plugin (if dinamic resize is enabled). 
        This means you'll get a resizing popup with NewStatusNotify and
        automatic read of away messages.
        Change: many small changes, don't ask me to remember them :-)

1.0.0.8 Bug fix release.
        Fix: \r\n are not displayed correctly (which means not displayed!!)
        Fix: threads not being killed :-)
        Fix: Use Windows Colours wasn't working outisde of preview.
        Fix: Default colours weren't saved on Miranda close.
        Fix: custom maximum width wasn't saved on Miranda close.

1.0.0.7 Fix: no more crashes on "Always on Top" failing (I hope).
        Change: the new translation string (the error message) has been changed
        a little, excuse me. :-) This time is final, though.
        New: option to specify custom minimum width in popups.
        New: option to specify a custom maximum width in popups (NewEventNotify
        can show very large popups!)
        New: multimonitor support. (wheee!)
        New: new service added (for developers) MS_POPUP_CHANGETEXT. :-) This
        will come in use to NewStatusNotify as soon as I code the preview of
        away messages. Oh, what a lame, I've already done it :-P Check out
        NSN 1.1.0.8
        New: new message added (for developers) UM_INITPOPUP. This is sent when
        a popup has completely been created, so your PluginData will be
        available to you.
        New: (for developers) some quick functions to avoid typing
        CallService(...) ;-)
        Fix: if the second line text is not given, but the popups are to have a
        second line, a blank one will be drawn (currently the popup shows with
        one line only)

1.0.0.6 Fix: Reorder PopUps wasn't working due to a mistake in the fixing of
        "Always on top". :-) Donwload this update because it could fix other
        bugs in the showing of PopUps.
        Change: the new translation string (the error message) has been changed
        a little, excuse me. :-)

1.0.0.5 New: (for developers) a new service function: PUShowMessage(), read
        the documentation. Oh, read it nevertheless, since I slightly modified
        the code examples. Be sure not to malloc() your POPUPDATA structure!
        Fix: Thread & Handles memory leaks are now gone. There's a 2KB leak
        sometimes, but I don't know if it's related to PopUp plugin or
        something else. (Thanks to Kenneth)
        Fix: popups are always on top for the two people who had problems too.
        :-) (thanks to wintime98)
        Change: small tweaks here and there.
        Change: GUI improvements 8-) Options are less cluttered now... (thanks
        Lynlimz)

1.0.0.4 Fix: "always on top" was not working during fade in, but only after the
        fade had completed. (Strange, it caused troubles in the old days of
        NewStatusNotify, now it just works fine :-/ bah...)
        Improvement: decreased CPU usage during fadings (in/out)... open Task
        Manager and relax ;-) (thanks to Alex Sanyukovitch for the Sleep() hint).
        Improvement: decreased memory usage and leaks, but there seems to be a
        memory leak I can't fix (read readme_popup.txt && faq_popup.txt)
        New: minimum width option to avoid too small PopUps.
        Change: buttons and checkboxes now reflect Miranda standard ones.
        Fix: Sample text font bug (it showed incorrectly sometimes).

1.0.0.3 Fixed an issue related to Winamp Notify (my fault, not his), please
        update NSN and WAN as soon as a new release is available.
        Fixed "Reorder PopUps" option not being correctly saved.

1.0.0.2 Users: MultiThreading now works! :-) PopUps are now concurrent. :-)
        Developers: you'll need to compile your plugins with the new m_popup.h
        Users: added the option to have "fixed position" popups, which means
        they will not be reordered as every one dies.
        Developers:Added a service (MS_POPUP_ISSECONDLINESHOWN)

1.0.0.1 Fixed a GDI leak (thanks to Alex Sanyukovitch)
        Removed the "MultiThreaded" checkbox. It's not yet the right time :-)

1.0.0.0 Initial release. This plugin is "son" of NewStatusNotify. Developers,
        you don't need to download the source code. Anyway, it's available.


- DISCLAIMER -

This plugin works just fine on my machine, it should work just fine on yours
withouth conflicting with other plugins. Should you have any trouble, write me
at hrk@users.sf.net where "sf" must be changed to "sourceforge".
Anyway, if you are a smart programmer, give a look at the code and tell me
the changes you'd make. If I like them, I'll put them inside giving you the
right credit! :-)

This plugin is released under the GPL license, I'm too lazy to copy it, though.
Anyway, if you do have Miranda (and you should, otherwise this plugin is
pretty useless) you already have a file called GPL.txt with tis license.
Being GPLed you are free to modify or change the source code (you can find it
here: http://nortiq.com/miranda/ and look for the source section) but you
cannot sell it.
As I already wrote: if you do modify it, notify me, I don't see a good reason
not to share the improvements with the Miranda community. :-)

Yes, I have made a quite long disclaimer, I can save the file now. :-)