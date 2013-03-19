Remove Personal Settings plugin
-------------------------------

WARNING: THIS PLUGIN DELETE SETTINGS FROM MIRANDA DB AND FROM THE HD. THIS CAN'T BE UNDONE. IT CAN DELETE ALL FILES IN YOUR COMPUTER (ALTOUGHT IT NEVER HAPPENED TO ME :P ). USE IT AT YOUR OWN RISK.

This is a plugin that removes personal settings from database.

It can remove all users, remove some files and directories from miranda dir (but only if these aren't beeing used), call services and disable plugins. To do that it is based in a configuration .ini file. If you need some especial configuration (and probabily you will need, because I just put some settings there, but I bet I forgot some), just edit the file RemovePersonalSettings.ini

The idea for using this plugin is to allow sending a copy of you miranda with your actual configuration, but without your personal settings. To use it, you should:
1) Copy the hole miranda dir to other location
2) Enable the plugin (if it isn't enabled)
4) Set global status to offline
3) Run the menu option "Remove personal settings..."
4) Exit miranda
4) Run DBTOOL.EXE to compact sweeped database file
5) Pack and send to your friend

I made this based on the discussion in the thread:
http://forums.miranda-im.org/showthread.php?t=5224. I asked the source from nullbie and made my version of the plugin. 

To report bugs/make suggestions, go to the forum thread: http://forums.miranda-im.org/showthread.php?t=5451
