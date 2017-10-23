

       Tooltip notify plugin for Miranda IM
       ____________________________________

                   Version 0.6.0.45


About
_____

This Miranda IM plugin popups a small tooltip above system tray area
when a contact changes his status. It's intended for and tested with ICQ 
protocol mainly, so it can miswork with other protocols.



Changelog
_________

Version 0.6.0.45
	- Individual font settings for events

Version 0.5.0.42
	- Added idle notifications
	- Using font services

Version 0.4.0.36
	- Fixed crash with too long contact names

Version 0.4.0.32
	- Next generation plugin interface support
	- Correct unicode/ansi flag in plugin description

Version 0.4.0.29
	- per-contact notifications ability
	- ansi/unicode module check on corresponding os
	- more unicode support
	- internal module name changed from 'ttntfmod' to 'Tooltip Notify'
	  (all settings migrate automatically)
	- fixed potential crash while reading font settings
	- fixed memory leak

Version 0.3.0.15
	- fixed one major unexpected crash

Version 0.3.0.14
	- unicode support
	- fixed minor bug related to settings reading
	- fixed potential crash with the tooltip moving
	- other minor fixes

Version 0.2.0.2
	- obey 'suppress online notification' setting
	- sound plays only if the event notification is checked
	- minor stability fixes
	- translation file update

Version 0.2.0.1
	- stability fixes

Version 0.2.0.0
	- some controls' text fields are enlarged (translators requests)
	- win9x misworking fixed (not tested)
	- new abilities:
		- proto name prefixing
	- enabled back
		- work in conjunction with 'suppress online notification'

Version 0.1.0.0
	- the whole code rewritten in cpp
	- new abilities:
		- suppress notifications upon connect
		- typing notification
	- disabled
		- work in conjunction with 'suppress online notification'

Version 0.0.6.0
	- rewritten code of the tooltip window
	- tooltip dragging behavior changed (now with usual left-click)
	- new options:
		- double click on tooltip action
		- font and color customization
	- some minor fixes/changes

Version 0.0.5.0
	- sound support for the contact events
	- added a button to run the tooltip within the options dialog to test it
	- some minor changes

Version 0.0.4.3
	- protocols selection bug fixed

Version 0.0.4.2
	- translucency options
	- translation support

Version 0.0.4.0
	- ability to select protocols to react to
	- ability of custom tooltip placement

Version 0.0.3.1
	- a little translations added

Version 0.0.3.0
	- options moved to events->tooltip notify
	- options added:
		- allow tooltip duration adjusting
		- conjunctive work with 'Suppress online notification'
	- fixed showing a tooltip when protocol goes offline
	- plugin size reduced

Version 0.0.2.4
	- some internal fixes


Copyright
_________

Copyright (C) 2004-2008 Gneedah software (perf@mail333.com)



DISCLAIMER OF WARRANTY 
--------------------------------------------------------------------

This software is distributed "AS IS" without any warranties.
USE IT AT YOU OWN RISK.
