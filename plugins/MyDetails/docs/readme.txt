My Details plugin
-----------------

What it does:
- Show your current configuration, per protocol, for avatar, nickname, status and away message
- It shows each protocol at a time, cicling throught then
- Allows to set nickname (per protocol or for all protocols) and away messages (per protocol or for all protocols - need NewAwaySystem, SimpleAway or core module)

Some comments:
1. SimpleAway does not show a dialog to set a message for all protocols. Someones it does not show the dialog (I requested it in the thread http://forums.miranda-im.org/showthread.php?p=47157).
2. For core away system, only some protocols works (probabily the same as SimpleAway). But for the ones that it works, the message in the frame is the old message. I know, it sucks... But if you use ersatz plugin this problem doesn't happen: http://pescuma.mirandaim.ru/miranda/ersatz.zip

To request support to other away system: If someone wants to use another away system, please request in its thread to add support for 2 services:
1. Get current status message for a protocol, given its name
2. Set current status message for a protocol, given its name and the message

To report bugs/make suggestions, go to the forum thread: http://forums.miranda-im.org/showthread.php?t=5643

Dependencies:
- If you want integration with clist, an frame enabled clist, such as clist_modern or clist_nicer+
- Away systens supported: Core, NewAwaySystem or SimpleAway

Todo:
- Global page
- Add custom presets
- Resize on mouse hover
- clist_modern_layered integration -> try this: http://forums.miranda-im.org/showthread.php?t=6597
- New drawing code (please, do not request things like order of items or spacing, it is in TODO list, but will take time to be made)
- Options to XStatus setup
- Icons instead of ... on mouse over
- Options to show/hide itens
