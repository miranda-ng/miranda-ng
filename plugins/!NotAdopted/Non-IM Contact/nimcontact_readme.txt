Non-IM Contact
--------------

Non-IM Contact allows you to add "contacts" that can act as shortcuts to other programs, or links to web pages.
The contacts name can be read from a text file (includes any ASCII file).
(This plugin is a combination of Favorites and Text Reader plugins both made by me)

Why would you use this plugin?
Well... lets say you are lazy and want to be able to load up the c:\ drive by double clicking a contact in your CList. So what you do to set this up is create a new contact (from the main menu), give the contact a name, then in the "Link and CList settings" tab, type c:\ into the link field.
This is the very simplest way of using NimC.
Now lets take this contact 1 step further.
Lets say you have a script that you have scheduled to run every 30min or so that prints the available disk space into a text file.
This script for example. http://www.coolmon.org/extensions/extension.php?id=154 prints something like this 
"
Last Updated: 5/17/2004 7:04:13 PM

C: 
Fixed : NTFS
25.35gb
6.13gb
19.5%
80.5%
[++========]
"
into a txt file. So, what can we do with information?
Lets set our contact to display as its nick the available disk space on the drive (i.e it would look like "C:\ 25.35gb free" )
all we need to do is load the file in the "files" tab then type something like this into the "display Name" field "file(0)wholeline(line(2))\ file(0)wholeline(line(4)) free".
This may look cryptic but it really is very simple !!! (see Explanation of Variables below)


Detailed Explanation of Non-IM Contact Settings
--------------------------------------------------

"Contacts Display Info" tab 
-> "Contacts display name" this is the only compulsary field.. this is what the contact will display as their nick, more info below how to make it kewl :) 
-> "Contacts tooltip" well this isnt really their tooltip becuase to make it easier for me, the info u type here after being replaced with info from the files gets copied into the user "notes" field in their "user info" display, so u can use this is mtooltip with the %notes variable (dont know about other tooltip plugins) 

"Link and CList settings" tab
-> "Link" the file to load/ program to run/ web page to open etc, when u double click the contact 
-> "program parameters" any parameters for the program 
-> "status icon" the contacts status icon. (See below)

"Timer settings" 
-> "use timer", if this is set then the contact can be updated using the timer 
-> the blank text field... the number of intervals between checking... the intervals are set in the options... default of 1 second)

"Copy Contact"
this dialog is explained below..

"Files" 
here is a list of all the available files (and eventually web pages) that u can use in all the contacts. the %fn ID box shows the text to type to use the file (explained below) 
the big text box shows the contents and line numbers of the file, and the drop list has a list of availablew files. 

Options window...
-> "ignore global status change" with this set, nimc will only change status if manually set (using the status bar icon, or status menu)
-> "aways as another status" changes the way away status works. if set then only contacts with away set as their icon will be displayed when the protocol chnages to away, and timer wont be turned off.. otherwise away acts the same as online but with the timer off)
-> "disable timer" disables the timer, no web pages or contacts will be updated if this is checked
-> "timer interval" this is the global timer interval. the individual contact timers are a multiple of this timer. so if this is set to 15 and a contacts individual timer is set to 3, then that contact will be updated every 45 seconds. (same for web pages)

String Maker window...
-> "string to test" you can type anything here, when you press Test String this string will get parsed and the result will be displayed in the other edit box
-> the row of varables is used to tell you which variable you are in.


Explanation of Variables
-------------------------

NEW VARIABLES IN 0.5
--------------------
lastchecked(file(X)) <- displays the time and date the file was last modified (im 2 lazy to make it editable, so live with it.. its DD/MM/YY HH:MM) (X is the file number)

for the next 5 the "" marks are compulsary... if they arnt there it wont work as expected!
the following are all done after the string has been complety replaced, and compare(..) is done the very last....

compare("A","B","X","Y") <- what this does is after all the strings have been replaced it compares A and B, if they are the same then X will get inserted into the final string, otherwise Y will be inserted. (this is the last variable that is checked so u can put file(x)start(.....)...) inside it and it will work the way u want it to. )

save("A","B") <- this saves whatever B is to a db variable named A in the contact's "NIM_Contact" module

load("A") <- this loads the setting A from the contact and inserts it into the string.

saveN("A","B","C","D") <- this is the same as save(...) except it lets you save to the setting contact in the db. A is module, B is setting, C is value, D is type (0 or b = byte, 1 or w = word, 2 or d = dword, 3 or s = string) (not 100% sure this is safe or useful.. but i dunno. an examlpe use of this might be to wright a quick script that spits out the name of a picture (from a defined list) and it will set your clist background pic to it... ?)

loadN("A","B") <- same as load(...) except it loads from the settings contact from the module A and setting B

REMEMBER... THE "" MARKS ARE NESCASSERY
------------------------------------------

from version 0.4 the variables have all been changed.. this new system is slightly worse but allows much better control over what u can display...

firstly the easiest variable.... filename(X) <- displays the filename of the Xth file in the files tab

now the hard variables...
each string replace thingy must look losmething like this...
file(X)start(line(...)...)end(line(...)...) or file(X)wholeline(line(...))
confused? good :)

file(X) <- choose which file to read from
start(...) and/or end(...) must follow. You can use both, or only 1 of them. If you use both start MUST be before end.. (obvious???).
OR you can follow file() with wholeline(line(...)), which as u can guess will display the entire line.

So, how does start(...) and end() work?
... Like this...
start(line(...)X) where X is either a number, or s string in "" marks, or the csv() variable. (we will get to the line() part in a minute).
   - If X a number then it will start or finish copying from the Xth character in the line (remember, 0 is the frst char).
   - If X is a string inside "" marks, then the line will be searched for the string and either start or stop copying depening if you are in start() or end().
   - If X is csv(seperatorY) then the line is expected a comma seperated variable list (or similar). the seperator must either be "tab" (for tab seperated list), "space" (for space seperated list), or any SINGLE character (e.g , or ;). and Y is which variable to start or finish copying from
simple isnt it? *head falls off in fit of laughter*

Now ill explain the line(...) var and then show some examples
line(...) is very similar to start() and end() with some slight changes...
line(X) where X is a number, string in "" marks, or lastline(...)
   - If X is a number then the rest of the start() or end() or wholeline() will use the Xth line of the file (remember 0 is the first line)
   - If X is a string it will search through the file (starting from the first line if in start() or wholeline() OR the line used by start() if in end() and will use the first line with the string if it is found.
      - You can add a +Y or -Y after the closing ) to go Y lines up (- sign) or down (+ sign) from the line found (e.g  ...line("hello")+5... will find the word hello and use the 5ht line after it)
   - If X is lastline(Y) then it will use the line Y lines up from the last line of the file. (e.g 0 or blank is the very last line of the file, 1 is the 2nd last line etc...)

and thats it....  so lets c some examples...
.
filename(0)		<- will display the filename of the 0th file

file(0)wholeline(line(0))	<- will display the whole first line of the 0th file

file(0)wholeline(line("hello")-1))	 <- the wholeline above the first occurance of \"hello\" in the file

file(0)start(line(lastline(1))csv(tab2))end(line(lastline())csv(tab4))	 <- starts at the 2nd last line of the file, from the 2nd tab variable, untill the 4th tab variable in the last line (in the 0th file)

file(0)start(line("hello")+1"zzzz")end(line(6)17))	<- starts from the first occurance of zzzz in the line after the first occurance of hello, untill the 17th character in the 6th line (starting from line 0) of the 0th file.

i plan on adding regexp ability and some other vars (tell me what u want...)
EVERY EDITABLE TEXT FIELD (EXCEPT GROUP FIELD) CAN USE THESE STRINGS 

The Copy Contact Dialog
-----------------------
Since the oriognal text reader (plugin this is based on, no longer available..) i wanted to be able to copy a contact easily, so this is how you do it.
The Edit box in this window lets you automatically change any string in the new contact. i.e this lets you change all occurances of %fn2 to %fn3 in the contacts settings (i.e their name, tooltip, etc).
the way you do is like this. if you want to change fn2 to fn3 you type fn2,fn3 in the box. ONE PER LINE... and make sure you dont forget the , or miranda may crash, and WILL KILL YOUR DB IF IT DOES.
You can do a maximum of 15 of the replaces.

press Copy Non-IM Contact to copy with the replaced strings.

To export the single contact to a txt file press export contact. (you cant import contacts yet, but ill add that soon..ish)

ToolTip stuff
--------------
OK.. if you want to actually use the "ToolTip" field then you'll need either mToolTip or another tooltip plugin.
NimC copies the string (after having all the variables replaced) from the "ToolTip" field into the Notes field in the contacts User Details window. in mToolTip use the variable %notes to display this.
NimC also copies the program (or link) that the contact points to when you double click it as the contacts First Name field in the contacts User Details window. in mToolTip use the variable %first to display this.


Contact Status Icon
-------------------
I have included all available statuses so you can seperate contacts by thier status icons, and so you can quickly differentiate between different "types" of contacts (by this i mean, you can set all web links to one icon, and all local folders to another, etc.)
BUT...
- ONLINE icon - this contact will always be displayed as online unless the plugin's status is set to offline
- AWAY icon - same as online (unless u choose to use away as any other status, i.e contacts wont be online if not set to away)
- all other icons means only display this contact if the plugins' status is set to that status or set to away or online.

Plugin Status's
---------------
If the plugins' status is:
- ONLINE - all contacts are displayed and the timer is ON (so names get updates)
- AWAY - all contacts are displayed and the timer is OFF (so names dont get updates ... unless u choose to use away as any other status)
- OFFLINE - all contacts are displayed as offline and the timer is OFF (so names dont get updates)
- other - only contacts set to online, away or the set status are shown as not offline and the timer is ON (so names get updates)


Last Minute Stuff To Remember
-----------------------------
- If you hold CTRL down when you double click a contact it will open the contacts settings (this also happens if no link is set)
- the file(X) variable is "global" in that all contacts with file(X) will be reading the same file. this also means that if you remove the file(X) file from the list, then all other files will get pushed up and any contact with file(X) set will be reading the new file, not the one you were expecting.

IMPORTANT NOTES ON WEB PAGES:
-----------------------------
- in the files tab you can enter urls into the URL box, it must be the full url.. i.e http://www.miranda-im.org/ (nothing happens if the url doesnt work)
- web pages can then be used as any of file.
- the files are downloaded into the \plugins dir with the filename fnX.html where X is the %fn number in the plugin. im lazy and want to get this release out so the files arent deleted when u remove the link yet. (soz)


Todo list
---------
- fix any mistakes with the string replacing (i dont know if there are any :) )
- add a program schedular to execute the scripts before updating the contacts' names
- suggestions?

Change-log
-----------
0.1
- Initial public release.. pretty much everything works

0.1.1
- fixed some stupid bugs (the timer actually works now, and it wont crash if you stuff up the string to replace (hopefully))
- made it ignore global status change

0.2
- added heaps of code to hopefully eliminate most of the crashes
  - files tab wont crash anymore if the file is too long
  - wont crash if you try to make a string too long (hard-coded max to 10000 chars which should be big enough)
- added support for web pages

0.2.0.1
- fixed it so it returns to the previous status when you start miranda
- fixed it so only status changes to Non-IM contacts protocol change the status
- fixed it so that you can change the timer for the web pages in the files tab (change the number and press apply)
- did a big plug on the overclockers australia forums :) (http://www.overclockers.com.au)

0.2.0.2
- removed the startup status thing to hopefully finally fix the status changing bugs.. if you want to have it start with a status use the startupstatus plugin
- fixed it so nconvers doesnt popup its msg dialoge when you double click a Non-IM Contact

0.3
- added multi-threading support so it doesnt freeze miranda while it waits for the websites to download. (thans Matrix)
- added ability to import/export/copy contacts

0.3.1
- sorry, didnt do enouhg testing again and the fiiles tab vanished...

0.3.2.1
- added %csv variable
- fixed status changing bug (hopefully the last 1)
- fixed timer so it works again
- fixed the contact copying
- fixed a crash when you want to show the tooltip for the contact
- got the menu items where i want them

0.4
- changed the vairable system
- added multi-threading so miranda doesnt freeze while it updates web sites (thanx Matrix and noname)
- removed contact importing coz i wanted to release this build but importing needed a rewrite and i cbf...
- added a String Maker window to help make strings to get replaced.. (with a kewl display that tells youn which variable you are in..)
- timer _MAY_ be a bit screwy if we pages dont update fast enough, but shouldnt...
- added options dialog
- fixed status changing bug (hopefully the last 1.. again) 

0.4.0.1
-fixed the timer..

0.4.1.0
- fixed a bug where the files tab would crash if more than 10 files were loaded. (now wont crash untill 1000 files are loaded)
- added the "NIM_Contact/DoubleClick" service for other devs if they want to simulate a double click on a non-im contact. (wparam=hcontact, lparam=0)
- added some error-checking to the string replacing code so it doesnt crash.. 

0.5
- added 6 new variables..
- fixed some minor stuff
- new stanslation strings...

0.5.0.1
- fixed bug where you couldnt add new contacts if u are running the mwclist.dll plugin
- fixed the netlib code so its not naughty...
Translation strings
--------------------
im pretty sure i havn't set up all the strings as translatable, so if i miss any, email or msg me on the forums and ill fix it.
this is the lot....

New strings for 0.5
[lastchecked(file(]
[loadN(\"]
[load(\"]
[saveN(\"]
[save[(\"]
{compare(\"]



[Select Folder]
[New Non-IM Contact]
[File couldn't be opened]
[line(%-3d) = | %s]
[Non-IM Contacts]
[&Non-IM Contact]
[&Add Non-IM Contact]
[&View/Edit Files]
[&Export all Non-IM Contacts]
[&Import Non-IM Contacts] <- will be back in the next release...
[&String Maker]
[E&dit Contact Settings]
[(\"]
[\")]
[lastline(]
[csv(]
[file(]
[wholeline(line(]
[start(]
[start(line(]
[end(]
[end(line(]
[filename(]
[%s - ERROR: no line specified or line not found (in %s)]
[%s - ERROR: file couldnt be opened (in %s)]
[%s - ERROR: no file specified in settings (in %s)]
[String replacing variables....\r\nThe following are all the valid variables that can be used. Refer to the readme for a proper explanation.\r\n\r\n]
[file(X)\t\t<- specifies the file to read from\r\nMUST be followed by either start() or end() or wholeline()\r\n]
[filename(X)\t\t<- copyies the filename of file X.\r\n]
[start(...)\t\t<-specifies where to start copying from.\r\n]
[end(...)\t\t<-specifies where to stop copying.\r\n]
[wholeline(line(...))\t<-specifies a whole line to copy\r\n\r\n]
[start() and end() explained\r\n.........................\r\n]
[MUST start with line() followed by a number or a string inside \" marks, OR csv(seperatorX) variable\r\n]
[The number specifies which character in the line to start/end copying.\r\nThe string specifies a string in the line to start/end copying.\r\n]
[csv(seperatorX) explained...\r\nseperator is either \"tab\" or \"space\" or any SINGLE character.\r\nX is the Xth seperator to pass before copying, (or to stop before)\r\n\r\n]
[Lastly the line(...) variable...\r\n]
[Inside the brackets must be either a number (to specify the line number), or a string inside \" marks (to use the line with that string), or lastline(X).\r\nthe X in lastline is the Xth line above the last line. i.e lastline(1) will use the 2nd last line of the file.\r\n]
[If searching for a line with u may put a + or - X after the closing ) i.e line(\"some words\")+3 to go 3 lines after the line with \"some words\".\r\n\r\n]
[Some Expamples...\r\n\r\n]
[filename(0)     <- will display the filename of the 0th file\r\nfile(0)wholeline(line(0))) <- will display the whole first line of the 0th file\r\nfile(0)wholeline(line(\"hello\")-1))) <- the wholeline above the first occurance of \"hello\" in the file\r\nfile(0)start(line(lastline(1))csv(tab2))end(line(lastline())csv(tab4))) <- starts at the 2nd last line of the file, from the 2nd tab variable, untill the 4th tab variable in the last line (in the 0th file)\r\nfile(0)start(line(\"hello\")+1\"zzzz\")end(line(6)17)) <- starts from the first occurance of zzzz in the line after the first occurance of hello, untill the 17th character in the 6th line (starting from line 0) of the 0th file.\r\n]
