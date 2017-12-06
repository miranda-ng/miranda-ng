/*

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

*/

// Settings values
#define KEYBDMODULE "keybdnotify"
#define FLASH_SAMETIME   0
#define FLASH_INTURN     1
#define FLASH_INSEQUENCE 2
#define FLASH_CUSTOM     3
#define FLASH_TRILLIAN   4
#define SEQ_LEFT2RIGHT   0
#define SEQ_RIGHT2LEFT   1
#define SEQ_LIKEKIT      2
#define ACTIVE_MIRANDA   0
#define ACTIVE_WINDOWS   1
// Until settings map
#define UNTIL_NBLINKS    1
#define UNTIL_REATTENDED 2
#define UNTIL_EVENTSOPEN 4
#define UNTIL_CONDITIONS 8

// Status map
#define MAP_ONLINE       1
#define MAP_AWAY         2
#define MAP_NA           4
#define MAP_OCCUPIED     8
#define MAP_DND          16
#define MAP_FREECHAT     32
#define MAP_INVISIBLE    64
#define MAP_ONTHEPHONE   128
#define MAP_OUTTOLUNCH   256
#define MAP_OFFLINE      512

// Default settings
#define DEF_SETTING_ONMSG       1   // 1: Yes, 0: No
#define DEF_SETTING_ONURL       1   // 1: Yes, 0: No
#define DEF_SETTING_ONFILE      1   // 1: Yes, 0: No
#define DEF_SETTING_OTHER       1   // 1: Yes, 0: No
#define DEF_SETTING_FSCREEN     1   // 1: Yes, 0: No
#define DEF_SETTING_SSAVER      1   // 1: Yes, 0: No
#define DEF_SETTING_LOCKED      1   // 1: Yes, 0: No
#define DEF_SETTING_PROCS       0   // 1: Yes, 0: No
#define DEF_SETTING_ACTIVE      1   // 1: Yes, 0: No
#define DEF_SETTING_IFMSGOPEN   1   // 1: Yes, 0: No
#define DEF_SETTING_IFMSGNOTTOP 0   // 1: Yes, 0: No
#define DEF_SETTING_IFMSGOLDER  0   // 1: Yes, 0: No
#define DEF_SETTING_SECSOLDER   10  // Seconds
#define DEF_SETTING_FLASHUNTIL  2   // 1 = After x blinks, 2 = Until Miranda/Windows become active, 4 = Events are opened, 8 = Until end of 'notify when' conditions
#define DEF_SETTING_NBLINKS     4   // Seconds
#define DEF_SETTING_MIRORWIN    0   // 0-1, 0 = Miranda becomes active, 1 = Windows becomes active
#define DEF_SETTING_STATUS      1023 // 1023 = All
#define DEF_SETTING_CHECKTIME   0   // Minutes, 0 = don't check
#define DEF_SETTING_FLASHCAPS   1   // 1: Yes, 0: No
#define DEF_SETTING_FLASHNUM    1   // 1: Yes, 0: No
#define DEF_SETTING_FLASHSCROLL 1   // 1: Yes, 0: No
#define DEF_SETTING_FLASHEFFECT 0   // 0-3, 0 = All together, 1 = In turn, 2 = In sequence, 3 = Custom Theme, 4 = Trillian-like sequences
#define DEF_SETTING_SEQORDER    0   // 0-2, 0 = left to right, 1 = right to left, 2 = left <-> right
#define DEF_SETTING_CUSTOMTHEME 0   // Theme number
#define DEF_SETTING_LEDSMSG     2   // 2: Num Lock
#define DEF_SETTING_LEDSFILE    4   // 2: Caps Lock
#define DEF_SETTING_LEDSURL     1   // 2: Scroll Lock
#define DEF_SETTING_LEDSOTHER   7   // 2: Num Lock + Caps Lock + Scroll Lock
#define DEF_SETTING_STARTDELAY  2   // Seconds
#define DEF_SETTING_FLASHSPEED  3   // 0-5, 0 = really slow, 5 = really fast
#define DEF_SETTING_KEYPRESSES  0   // 1: Yes, 0: No
#define DEF_SETTING_OVERRIDE    0   // 1: Yes, 0: No
#define DEF_SETTING_TESTNUM     2   // Number of sequences to test before stopping
#define DEF_SETTING_TESTSECS    2   // Seconds to test before stopping
#define DEF_SETTING_PROTOCOL    1   // 1: Yes, 0: No
#define DEF_SETTING_XSTATUS     1   // 1: Yes, 0: No
