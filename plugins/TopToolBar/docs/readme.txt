About
-----

TopToolBar
Version 0.7.3.1
by 
Bethoven (bethoven@mailgate.ru)

Note: This plugin required MultiWindow or clist_mw plugin.

Description
-----------
TopToolBar adds buttons in top frame for fast access 
to miranda/plugins functions.

Features
--------

-Predefined buttons(Hide/Show OfflineUsers,Hide/Show Groups,On/Off Sounds)
-Services for plugins to add/remove/modify new buttons.


WhatsNew
--------
11-04-2008 v0.7.3.1
        - ???
17-09-2005 v0.7.3.0
        -Fixed "new separator" bug.
		-Less flicks on resize.
		-Removed default bitmaps.

11-09-2005 v0.7.2.0
		-Removed default bitmaps.

11-09-2005 v0.7.1.0
		-Fixed drawing of buttons when Themes enabled.
		-New TTBButtonV2 struct for adding buttons (icon member)

26-06-2005 v0.7.0.0
        -Supporting for miranda buton class
		-Supporting for icolib(use with miranda buton class)
		-Moved background options to Customize

26-05-2005 v0.6.0.12
        -Fixed show online users button.

16-12-2004 v0.6.0.11
        -User defined width and height of buttons
		-Support loading of icons.

23-09-2003 v0.6.0.9
        -Set all bitmaps by selecting directory.

09-09-2003 v0.6.0.7
        -Fixed some bugs that may cause crash.

05-05-2003 v0.6
	-Added 2 new internal buttons Minimize and Find/Add contacts.
	-Autowraping mode.
	-Added separators.
	-Added Launch buttons.

31-03-2003 v0.5.0.1
	-Added background configuration 
	-Added Button enable/ordering. 
	-You may change button bitmaps. 
	-Added two new internal buttons (MainMenu and Options).

22-03-2003 v0.4.0.1
	-Fixed runtime button removing.
	-Fixed ownerdraw borders for buttons.

11-01-2003 v0.4
	-(dev) Changed service names to sandart look MS_ (thanks hrk).
	-(dev) Added border drawing.
	-Added tooltips for buttons.
	

13-12-2002 v0.3
	-Added services for add/remove additional buttons from plugins.
	  (see m_toptoolbar.h for description)
	-Added button to quick enable/disable sounds.
	-All buttons are synchronized. So if you check hide groups 
	  from menu, button track this.
	

Bitmap Names 
----------
TopToolBar search for bitmaps in this pattern.
Selecteddir/ButtonName_DN.bmp
Selecteddir/ButtonName_UP.bmp
Where ButtonName is non localised name of button.
This chars in name '/','\','*','?','<','>',':','|','"' replaced with '_'


Names for standart buttons:

"Find_Add Contacts_DN.bmp"
"Find_Add Contacts_UP.bmp"
"Groups On_Off_DN.bmp"
"Groups On_Off_UP.bmp"
"Minimize Button_DN.bmp"
"Minimize Button_UP.bmp"
"Show Main Menu_DN.bmp"
"Show Main Menu_UP.bmp"
"Show only Online Users_DN.bmp"
"Show only Online Users_UP.bmp"
"Show Options Page_DN.bmp"
"Show Options Page_UP.bmp"
"Sounds Enable_Disable_DN.bmp"
"Sounds Enable_Disable_UP.bmp"


Installation
------------

Copy the TopToolBar.dll into Miranda's plugin subdirectory.


Translation
-----------
[Hide Offline Users]
[Show All Users]
[Show Groups]
[Hide Groups]
[Disable Sounds]
[Enable Sounds]




Development
-----------
My radix 		27d,0x1B (BET)
My base 		0x1b000000

Quicksearch 	base    0x1b000000
MultiWindow 	base	0x1b100000
TopToolBar  	base	0x1b200000
SystemStatus	base	0x1b300000


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


