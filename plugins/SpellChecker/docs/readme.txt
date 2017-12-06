Spell Checker plugin
--------------------

This is a spell checker for message windows. It also allow to configure replacements to auto-correct words.

It uses Hunspell to do the dirty work. Hunspell is the spell checker used by OpenOffice, so it should have a good range of dictionaries. The dictionaries: each is a couple of files with the name beeing the language and the extensions .dic and .aff. Both need to be inside the dir <Miranda Path>\Dictionaries (of a custom folder if folders plugin is installed). You can download them at: http://wiki.services.openoffice.org/wiki/Dictionaries .
PS: This path is read only at startup, so changing it needs a restart of miranda.

Note that this plugin does NOT work with SRMM, because it doesn't have a rich edit in the input field. I've done a moded version, but it is old and I'm not sure if it still works:
Ansi: http://pescuma.org/miranda/srmm.zip
Unicode: http://pescuma.org/miranda/srmmW.zip
Patch: http://pescuma.org/miranda/srmm.spellchecker.patch

It has an options page to set the default dictionary and some other options. It is at Message Sessions/Spell Checker .

For other plugins, it works throught providing 3 serices for them to interact:
- one to add handling of a rich edit control
- one to remove it
- one to append options needed to a popup menu and show it
These plugins have to call this services in other for the magic to happen. So, things like when the popup menu is show are handled by the calling plugin.

The flags: it supports showing flags to represent the dicts. This is done to have an visual way to represent the dict. To load the flags, the following steps are done:
1. Try to load from file Icons\flags.dll - in this case an icon resource must exist with same name as language (for ex: pt_BR). If this is found, the icon is also added to IconLib to allow further customizing.
2. Load unknown flag. It can be customized thorught IconLib
Currently there are 2 sets of flags, one from Angeli-Ka and one from the famfamfam.com site:
- famfamfam's icons as .ico: http://pescuma.org/miranda/flags-famfamfam.zip (note that there are a lot of files inside this zip with wrong names. It happens because I don't know which languages they represent - and if they represent a language or not. So, if you think some file name must change, please tell me)
- famfamfam's icons as .dll: http://pescuma.org/miranda/flags-dll-famfamfam.zip
- Angeli-Ka's icons as .ico: http://pescuma.org/miranda/flags-angelika.zip
- Angeli-Ka's icons as .ico with language names: http://pescuma.org/miranda/flags-angelika-name.zip
- Angeli-Ka's icons as .dll: http://pescuma.org/miranda/flags-dll-angelika.zip

Many thanks to the Hunspell team and to Vladimir Vainer that made an initial version of the plugin. And thanks to the famfamfam.com site for the icons I'm using for the flags.

Needs Miranda 0.8

To report bugs/make suggestions, go to the forum thread: http://forums.miranda-im.org/showthread.php?t=11555