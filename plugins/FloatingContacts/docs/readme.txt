******************
Floating Contacts
******************

Description
===========
This plugin is further development of Floating contacts plugin v1.0.0.7 from Iavor Vajarov (thx).

New features
************
- Showing a tooltip when mouse's hovering over a floating contact's window.
- Support MUID inteface of Miranda v0.8.0.0 and higher.
- Unicode aware.
- True alpha channel support on windows 2000+.
- "Bring to front" feature allows bring to front floating windows every x seconds. 
	With a lot of window may get backside effect, in this case set greater interval.
- And various other features.

Requirements
************
- Miranda v0.6.0.0 or higher.
- Tipper or mToolTip plugin for tooltip show.

Changelog
=========

+   Relative coordinates.

---1.0.2.2---
!   Fixed gdi leak.
!   Fixed possible crash.
!   Fixed drawing 8-bit status icons.
!   Show floating contacts on startup when "Hide when contact list is shown" option is checked.

---1.0.2.1---
!   Wrong detection of hiding CList behind display border.
*   Changed "Mouse In" highlight (must resolve some problem).
!   Fixed "Single click" feature.
!   Fixed memory leak.

----1.0.2.0---
+   True alpha channel support on windows 2000+.
!   Minor fixed.
*   Converted to C++.

----1.0.1.1---
+   Added "Dim idle contacts" feature.
!   Fixed menu.
*   Updated icons (added 256 colors icons).
*   Changed painting. 
*   Changed moving.

----1.0.1.0---
+   Unicode aware.
+   Added russian langpack.
*   Updated langpack.
!   Fixed showing of nicks, containing "&".
+   Added feature "Single click interface".
+   Added hotkeys for "Show/Hide all thumbs" and "Hide when contact list is showing".
!   Fixed refresh icon.
+   Added "Hide when contact list is showing" feature.

----1.0.0.9---
!   Fixed show of floating contact's windows if check "Hide all contacts".
+   Added "Bring to front" feature. The feature provide keeping floating contact's windows on top. 
*   Unified plugin name in service, db setting and db contacts settings.
	To keep your old settings just rename module settings branch "Floating contacts" in database to "FloatingContacts" (use DBE++).
!   Fixed font size.
*   Reformed options.
-   Deleted support Miranda version lower then 0.6.0.0.
+   Added MUID.
*   New icons.

----1.0.0.8---
!   Fixed coordinate floating contact's window when it is dragged.
!   Fixed name change.
+   Added showing of tooltip when mouse's hovering over a floating contact's window (requires Tipper or mToolTip plugin).

Symbols used in changelog
--------------
+ : new feature
* : changed
! : bufgix
- : feature removed or disabled because of pending bugs
--------------

Author
======
Created by Iavor Vajarov ( ivajarov <at> code <dot> bg ).
Further development by Kosh&chka ( ell-6 <at> ya <dot> ru ).

