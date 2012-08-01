Avatar History plugin
-----------------------------------

CAUTION: THIS IS AN BETA STAGE PLUGIN. IT CAN DO VERY BAD THINGS. USE AT YOUR OWN RISK.

This plugins keep a history of contact avatars. It stores the files on the disk and uses miranda history to keep track of then.

This plugin is really made by MattJ, I'm only doing some changes to allow better handling of the avatars.

If you want to import the old history to new format, you can use the Avatar History Importer plugin by TioDuke. It will import old avatars to new format, allowing to keep only one copy of each avatar (but new avatars may appear duplicated, because the hash function used by the importer may not be the same as the one used by the protocol). It can be downloaded at:
Avatar History Importer Ansi Dll: http://pescuma.mirandaim.ru/miranda/avh_imp.zip
Avatar History Importer Unicode Dll: http://pescuma.mirandaim.ru/miranda/avh_impW.zip
Avatar History Importer Source: http://pescuma.mirandaim.ru/miranda/avh_imp_src.zip

To user the importer, you should:
1. Backup your profile and the old avatar history folder
2. Install the new version of Avatar History
3. Start miranda with all protocols offline
4. Setup Avatar History options (some are better if set once and not changed anymore). You will need 'Show expert options' checkbox enabled
5. Finish miranda and install Avatar History Importer
6. Start miranda with all protocols offline
7. A messagebox will appear, telling that the importer will run. Wait until the importer ended messagebox.
8. Enjoy ;)

Now all avatars are stored in per protocol folders. This is done to allow avoiding duplicated avatars. (If you enable 'Keep all in same folder' there will be only one folder for all protocols - this may help in case you have more than one instance of the same proto). The relation avatar <-> contact and the avatar history dialog is created from info from the contact history (and not using the files in HD). Since some people like to see the images in contacts folders, there is the option 'Also create per contact folders with shortcuts'. This option will create the contact folders (as previous version) and will create shortcuts inside it to the avatar stored in the protocol folder (if 2 avatars are the same, there will be 2 shortcuts but only one avatar in disk). Importer will use this option too when importing, but you must set it before importing everything.

This plugin needs Avatar Service and Miranda 0.7 to be able to work. To log events to history it needs History Events plugin.

To report bugs/make suggestions, go to the forum thread: http://forums.miranda-im.org/showthread.php?t=9118
