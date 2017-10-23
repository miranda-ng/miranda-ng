Database Editor ++
-------------------

Database Editor ++ allows you to easily modify your database settings (which is a good and bad thing..). Be carefull editing values that you dont know what they are used for... you couldnt couse Miranda IM to not work if you change the wrong setting...
YOU HAVE BEEN WARNED...

A summary of what DBE++ can do.. (each will be explained below)
- Edit every database setting (not history)
- export / import your entire (or partial) database to/from a txt file
- Remove an entire module from the database
- Find and replace a value in string variables..
- Find text (numbers or letters) in any module, setting name or value
- Keep a list of variables you need to watch (usefull for developers)
- display modules which are not currently being used (if the plugin that uses that module isnt loaded)
- clone a contact
- copy a module to a contact
- rename module and settings
- change a variable type
- show popups when a certain setting changes
- show encrypted strings decrypted (as well as encrypt/decrypt any string variable)
and more....

Editing Variables
-----------------
The easiest way to edit or rename a setting is to dowuble click the setting's icon, a new window will popup and you can edit it there...
well, i lie.. the easiest way to edit/rename the settings is a bit more complicated :)
if you click twice the setting name a edit window should popup where you can rename the setting... pressing escape will cancel it, everything else will accpet your changes without confirmation.
Same goes for the setting value (2nd column) except you can do nifty stuff here which needs to be explained...
just typing a number will keep the vairable type the same and change the value to your new number (it does not check if your new number is larger that the type's largest possible number).
Typeing anything other than numbers will convert the variable to a string with your new data as the value.
Typeing a number into a string variable will keep the variable as a string.

You can force a variable to a type by prepending the value with a letter. 
'b' or 'B' to force a BYTE variable.
'w' or 'W' to force a WORD variable.
'd' or 'D' to force a DWORD variable.
If your new data starts with a ' or " the setting will be converted to a string, so if you actually want to start the value with a ' you need to put 2 ' marks at the start!..
unless a number starts with 0x or 0X the number is assumed to be decimal, with a 0x/0X it is hexidecimal.

Exporting / Importing
---------------------
To export the entire database go Actions > Export / Import > Export entire database (in the DBE++ window), you can choose any file extension you want, it just saves as a plaintext file, so .txt is logical :)
To export only a certain module or contact right click it and go export..

Importing is a bit more complex...
there are two ways to import settings, either useing the import window, or just importing an entire file.
Using the import window lets you choose exactly which modules/settings to import into the selected contact.
Importing an entire file will start importing all the settings into the selected contact (the Settings contact if you do it from the actions menu item).

Every time a line starting with "CONTACT:" is read a new contact is created and the following modules/settings get imported into that contact... there is one exception tho. if the line is "CONTACT: Settings" then the following modules/settings get imported into the Settings contact ( a new contact is not created..)

Remove an entire module from the database
----------------------------------------------
To remove a module from the database use the actions > Delete module menu item. If you have alot of modules this window can take a few seconds to finish setting up. when it finishes loading all the modules just choose the one u want to delete and press ok. there is no confirmation here so be careful... Also its a good idea to run dbtool after doing this as you may have lots of wasted space in the DB...

Find and replace a value in string variables..
----------------------------------------------
Actions > find > find and replace...
This searches every string value in the DB and will replace the find text with the new text.. it does not replace text in value names, and there is no confirmation...

Find text (numbers or letters) in any module, setting name or value
----------------------------------------------------------
Actions > find > find
This window lets you search the entire database for any text (numbers included..) in module names, setting names and setting values...
Double clicking any of the found items will cause the main DBE++ window to select that item...

Keep a list of variables you need to watch (AKA watch list)
-------------------------------------------
if there is a variable you want to keep an eye on (testing something?) then right click the setting in the list and go "watch". (you can also watch entire modules by right clicking them). To then see the watch list use the watches > view all menu item. A new window will apear with all the watched settings. double clicking any of them will find the item in the main window. If you have chosen to be notified with popups then any watched setting that changes will create a popup (even if the DBE++ and watch list windows are not open)
Currently there is no way to save the list after a restart...

display modules which are not currently being used (AKA known modules list)
---------------------------------------------------------------------------
If a module has the yellow folder icon it means that the module is currently being used, e.g the DBEditorpp module will always be yellow because its being used by this plugin... but the Chat module will only be yellow if you have the chat plugin enabled and installed... If you dont care whether a module is being used or not then u can turn this feature off in the options...
Unfortuantly, the only way to know which modules are being used is by other plugins letting DBE++ know... so untill more plugins get updated to use this feature you can add module names to the edit box in options > plugins > Database Editor++ (changeing this requires a restart to take effect), OR you can use an mbot script to add modules "on the fly" (read below)
Plugin developers please read below how to add your modules...

Renameing modules
-----------------
Select the module you want to rename and press f2 or click the name again and you can type a new name, pressing esc will cancel the change, anything else will accept the change with no confirmation.

Other Stuff
----------
Everything else should be self-explanatory or easily done by context menu items....


ADDING YOUR MODULES TO THE KNOWN MODULES LIST
------------------------------------------------
If your plugin only uses ONE module then you can use the "DBEditorpp/RegisterSingleModule" Service function like this:
CallService("DBEditorpp/RegisterSingleModule",(WPARAM)"modName",0);
If you have more than one module then use the "DBEditorpp/RegisterModule" with the WPARAM char** and LPARAM int being the number of modules to register..
e.g
{
   char *mods[4] = {"module1","Module2","Module3"};
   CallService("DBEditorpp/RegisterModule",(WPARAM)mods,(LPARAM)3);
}
REMEMBER TO DO THIS IN OR AFTER ME_SYSTEM_MODULESLOADED HAS BEEN TRIGGERED....

DOING IT WITH MSP (formerly mBot)..
If you want to add modules "on-the-fly" you can add them with an mbot script like this:
<?php
mb_SysCallService('DBEditorpp/RegisterSingleModule',"modulename",0);
?>
then refresh the module tree and the module will be known..

Translation
-----------
ive added a translation.txt file in the archive... apart from the text in the windows i tihnk it has everything...

Changelog
---------
3.1
- added icon manager support coz you people are annoying... btw huge thanx to Angeli-ka for the icons
- fixed a bug in the find and replace code...
- added a fix to disallow inline setting editing on win98 and winME to stop crashes
- big thanx to Std for pointing out (and fixing) some stupid bugs in the export/import code
- menu should be translatable now
- added a bit more room in the options dialog for tranlsators
- added in bio's code changes which seems to have fixed the popup module crash problem... (hopefully) (THANX BIO)
- the old "KnownModules" module is now used again... any modules in that are added
- still trying to work out how to not save the window size if it was maximised...
- added a "add module to known list" menu item 
- modules added in the options page can now have a space in their name (put a \ before the space for it to work)
- fixed the options page a bit
- now asks to overwrite files when you export...
- no more contact proto icons in the contact list
- fixed various translation problems

3.0
- 99% recode and now it all works nicely :)

1.0->2.0
- old stuff.... not relevant anymore

------------------------------

(c) 2005 Jonathan Gordon (jdgordy@gmail.com)

contact me on ICQ, MSN, Email or Telepathy... just make sure you tell me who you are or youll get blocked :)
ICQ: 98791178
MSN: jonnog@hotmail.com
Jabber: jdgordy@jabber.zim.net.au

