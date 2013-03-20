Buddy Pounce
-------------

Buddy Pounce allows you to send a message to a contact automatically when they change status (i.e from offline) if you are expecting to be away form the computer.
This bassically adds "Offline Messaging" to protocols that dont have it (i.e MSN)

HOW To use it.
---------------
Right click the contact you want to send the message to. choose the Buddy Pounce item.
If you havnt edited the options yet you will be at the "simple" window. by default any message you type here will be sent to the contact when they change status. (dont worry the default behaviour can be changed in the options).
Pressing the "advanced >>" button will open the advanced window. Here you have full power of when to send the message.
Firstly, make sure the contact you want to send to is selected in the drop list at the top right.
next type your message.
Now the fun begins.....

The Settings Explained...
- "send if My Status Is..." - selecting this will open up a window with 10 check boxes. basically the message will only be sent if your status matches any checked statuses here.
- "Send if they change status to..." - bassicaly the same as above, cept it will only send if they changed to a checked status
----------- the next 3 options can all be set to 0 to ignore them
- "Reuse Pounce" - if this is set to anything but 0 the messsage will be sent the next X times the contact changes to the specified status
- "Give Up Delay" - will not try to send the message if the contact doesnt change to the specified status after X days
- "Confirmation Window" - with this set, a window will popup asking you if you want to send the message. It will automatically be sent in X seconds if you dont press cancel. if its set to 1024 then it will never send unless you press OK (sort of useless then...)

(the advanced dialog has 2 buttons... delete pounce which deletes the pounce ! :O, and load deafults which overwrights any settings with the default ones from the options.)
the simple window uses the default options to send the message... 

Explanation of options window
-----------------------------
"Use Advanced Dialog" - if this is checked then the advanced window will be automatically opened
"Show Delivery Messages" - if this is checked a window will poup saying the message was un/successfully sent to the contact.
"Default Settings" - excatly the same as the settings as explained above... any values set here will be used as the values for the "simple" window, and will be automatically set when u load the buddy pounce windw.


FOR DEVELOPERS:
---------------

in the of-chance you want to setup a pounce in your own plugins i have added 2 service funcitons you can use.
call both of these like CallService(SERVICE, (WPARAM)hContact, (LPARAM)message)
where hContact is the HANDLE to a valid contact, and message is a pointer to a string to use as the message
"BuddyPounce/AddSimplePounce" - which sets a new pounce using the message u supply and the defalt settings
"BuddyPounce/AddToPounce" - which adds your text to an exsisnting pounce message, or call the above service if 1 doesnt exsist.
it doesnt really matter which u call, but to check if a message exsists check if the string (hContact, "buddypounce", "PounceMsg") exsists..
if you want to setup an "advanced" pounce you will have to setup the other settings manually... email me @ jdgordy@gmail.com for info...)

Translation:
------------
the following strings can be translated....
Send If My Status Is...
Send If They Change Status to...
Reuse Pounce
Give Up delay
Confirmation Window
Send a File
Send If My Status Is
Any
Online
NA
Occupied
DND
Free For Chat
Invisible
On The Phone
Out To Lunch
Send If Their Status changes
From Offline
To Online
To NA
To Occupied
To DND
To Free For Chat
To Invisible
To On The Phone
To Out To Lunch
The Message    (%d Characters)   **** make sure u keep the %d in this 1
Pounce being sent to %s in %d seconds   **** and the %s and %d in the same order in this 1

the dialogs also need to be translated but im falling asleep and want to relase the plugin...


Change-log:
-----------
0.3
- completly re-written